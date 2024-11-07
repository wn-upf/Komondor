/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007

 * Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>
 * Everyone is permitted to copy and distribute verbatim copies
 * of this license document, but changing it is not allowed.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * -----------------------------------------------------------------
 *
 * Author  : Sergio Barrachina-Muñoz and Francesc Wilhelmi
 * Created : 2016-12-05
 * Updated : $Date: 2017/03/20 10:32:36 $
 *           $Revision: 1.0 $
 *
 * -----------------------------------------------------------------
 */

 /**
 * pre_processor.h: this file contains functions related to the agents' operation
 *
 *  - This file contains the methods used by the pre-processor (PP) in the Machine Learning (ML) pipeline.
 * 	 In particular, this module prepares data extracted from the network simulation for applying a given ML method
 */

#include "../list_of_macros.h"

#include "../structures/node_configuration.h"
#include "../structures/performance.h"

#ifndef _AUX_PP_
#define _AUX_PP_

class PreProcessor {

	// Public items
	public:

		// General parameters
		int selected_strategy;				///> Index of the chosen action-selection strategy

		// Lists of configurations
		int *list_of_channels;				///> List of channels to be selected
		double *list_of_pd_values;			///> List of PD values to be selected
		double *list_of_tx_power_values;	///> List of Tx Power values to be selected
		int *list_of_max_bandwidth;			///> List of DCB policies to be selected

		// Actions management
		int num_arms;					///> Number of actions
		int num_arms_channel;			///> Number of channel actions
		int num_arms_sensitivity;		///> Number of sensitivity actions
		int num_arms_tx_power;			///> Number of transmission power actions
		int num_arms_max_bandwidth;		///> Number of max bandwidth actions
		int *indexes_selected_arm;			///> Indexes for each parameter that conform the current selected arm

	// Private items
	private:

	// Methods
	public:

		/************************/
		/************************/
		/*  PROCESSING METHODS  */
		/************************/
		/************************/

		/**
		* Adapt the WLAN's configuration to be passed to the ML method
		* @param "learning_mechanism" [type int]: learning method employed
		* @param "configuration" [type Configuration]: current configuration report from the AP
		* @return "processed_configuration" [type int]: configuration to be introduced directly to the ML method
		*/
		int ProcessWlanConfiguration(int learning_mechanism, Configuration configuration, bool received_from_ap) {
			int processed_configuration(0);
			// Update the current configuration according to the selected learning method
			switch(learning_mechanism) {
				/* Multi-Armed Bandits */
				case MULTI_ARMED_BANDITS:{
					// Convert the WLAN's configuration into an arm/action index value
					processed_configuration = FindActionIndexFromConfigurationBandits(configuration, indexes_selected_arm, received_from_ap);
					break;
				}
				/* Default */
				default:{
					printf("[Pre-Processor] ERROR: '%d' is not a correct learning mechanism\n", learning_mechanism);
					PrintAvailableLearningMechanisms();
					exit(EXIT_FAILURE);
					break;
				}
			}
			return processed_configuration;
		};

		/**
		* Adapt the WLAN's configuration to be passed to the ML method
		* @param "learning_mechanism" [type int]: learning method employed
		* @param "performance" [type Performance]: current performance report from the AP
		* @param "type_of_reward" [type int]: type of performance metric to be used as a reward
		* @return "processed_performance" [type double]: performance indicator to be passed to the ML method
		*/
		double ProcessWlanPerformance(Performance performance, int type_of_reward) {
			return GenerateReward(type_of_reward, performance);
		};

		/**
		* Based on the performance report of the WLAN, generate the reward to be passed to the ML method
		* @param "type_of_reward" [type int]: type of performance metric to be used as a reward
		* @param "performance" [type Performance]: current performance report from the AP
		* @return "reward" [type double]: reward to be passed to the ML method
		*/
		double GenerateReward(int type_of_reward, Performance performance) {

            double reward(0);
			// Switch to select the reward according to the metric used (rewards must be normalized)
            switch(type_of_reward){
				/* REWARD_TYPE_PACKETS_SUCCESSFUL:
				 * - The number of packets sent are taken into account
				 * - The reward must be bounded by the maximum number of data packets
				 * 	 that can be sent in each interval (e.g., packets that were sent but lost)
				 */
				case REWARD_TYPE_PACKETS_SUCCESSFUL:{
					reward = (performance.data_packets_sent-performance.data_packets_lost)/performance.data_packets_sent;
					break;
				}
				/* REWARD_TYPE_AVERAGE_THROUGHPUT:
				 * - The throughput experienced during the last period is taken into account
				 * - The reward must be bounded by the maximum throughput that would be experienced
				 * 	 (e.g., consider the data rate granted by the modulation and the total time)
				 */
				case REWARD_TYPE_AVERAGE_THROUGHPUT:{
					if (performance.max_bound_throughput == 0) {
						reward = 0;
					} else {
						reward = (double) performance.throughput/performance.max_bound_throughput;
					}
					break;
				}
				/* REWARD_TYPE_MIN_RSSI:
				 * -
				 */
				case REWARD_TYPE_MIN_RSSI:{
					reward = performance.rssi_list_per_sta[0];
					for (int i = 0; i < performance.num_stas; ++i) {
						if(reward > performance.rssi_list_per_sta[i]) reward = performance.rssi_list_per_sta[i];
					}
					break;
				}
				/* REWARD_TYPE_MAX_DELAY:
				 * -
				 */
				case REWARD_TYPE_MAX_DELAY:{
					reward = 1/performance.max_delay;
					break;
				}
				/* REWARD_TYPE_MIN_DELAY:
				 * -
				 */
				case REWARD_TYPE_MIN_DELAY:{
					reward = (1/performance.min_delay);
					break;
				}
				/* REWARD_TYPE_AVERAGE_DELAY:
				 * -
				 */
				case REWARD_TYPE_AVERAGE_DELAY:{
					reward = 1/performance.average_delay;
					break;
				}
                /* REWARD_TYPE_CHANNEL_OCCUPANCY:
                 * -
                 */
				case REWARD_TYPE_CHANNEL_OCCUPANCY:{
					reward = performance.successful_channel_occupancy;/// (SimTime() - performance.timestamp);
				    break;
				}
				/* Default */
				default:{
					printf("[Pre-Processor] ERROR: '%d' is not a correct type of performance indicator\n", type_of_reward);
					PrintAvailableRewardTypes();
					exit(EXIT_FAILURE);
					break;
				}
			}
//			printf("Reward = %f\n",reward);
			return reward;
		}

		/**
		* Process the output of the ML method and return a configuration struct
		* @param "learning_mechanism" [type int]: learning method employed
		* @param "configuration" [type Configuration]: current configuration report from the AP
		* @param "ml_output" [type int]: output of the ML method
		* @return "suggested_configuration" [type Configuration]: new configuration to be sent to the WLAN
		*/
		Configuration ProcessMLOutput(int learning_mechanism, Configuration configuration, int ml_output) {
			Configuration suggested_configuration;
			// Switch to select the reward according to the metric used (rewards must be normalized)
			switch(learning_mechanism){
				case MULTI_ARMED_BANDITS:{
					GenerateNewConfigurationBandits(&configuration, ml_output);
					break;
				}
				case RTOT_ALGORITHM:{
					GenerateNewConfigurationRtotAlg(&configuration, ml_output);
					break;
				}
				/* Default */
				default:{
					printf("[PP] ERROR: %d is not a correct type of learning mechanism. Returning the default configuration.\n", learning_mechanism);
					suggested_configuration = configuration;
					break;
				}
			}
			return configuration;
		};

		/***********************/
		/***********************/
		/*  BANDITS METHODS  */
		/***********************/
		/***********************/

		/**
		* Initialize actions
		* @return "action_array" [type *Action]: initialized array of Action structs
		*/
		Action* InitializeActions(){
			Action *action_array = new Action[num_arms];
			int *indexes_arm = new int[NUM_FEATURES_ACTIONS];
			for(int i = 0; i < num_arms; ++i) {
				index2values(indexes_arm, i, num_arms_channel,num_arms_sensitivity,
					num_arms_tx_power, num_arms_max_bandwidth);
				action_array[i].id = i;
				// Configuration
				action_array[i].channel = list_of_channels[indexes_arm[0]];
				action_array[i].cca = list_of_pd_values[indexes_arm[1]];
				action_array[i].tx_power = list_of_tx_power_values[indexes_arm[2]];
				action_array[i].max_bandwidth = list_of_max_bandwidth[indexes_arm[3]];
				// Performance
				action_array[i].instantaneous_reward = 0;
				action_array[i].times_played = 0;
				action_array[i].average_reward_since_last_cc_request = 0;
				action_array[i].times_played_since_last_cc_request = 0;
//				action_array[i].PrintAction();
			}
			return action_array;
		}

		/**
		* Encapsulate the configuration of a node to be sent
		* @param "configuration" [type Configuration]: current configuration report from the AP
		* @param "action_ix" [type int]: index of the action that corresponds to the new configuration
		* @return "new_configuration" [type Configuration]: configuration report, corresponding to the new action
		*/
		void GenerateNewConfigurationBandits(Configuration *configuration, int ml_output){
			// Find which parameters correspond to the selected arm
			index2values(indexes_selected_arm, ml_output, num_arms_channel,
				num_arms_sensitivity, num_arms_tx_power, num_arms_max_bandwidth);
			// Update each parameter in "configuration" according to the configuration provided by the MAB
			configuration->selected_primary_channel = list_of_channels[indexes_selected_arm[0]];		// Primary channel
			configuration->selected_pd = list_of_pd_values[indexes_selected_arm[1]];					// Selected PD (CCA or OBSS/PD)
			configuration->selected_tx_power = list_of_tx_power_values[indexes_selected_arm[2]];		// TX Power
			configuration->selected_max_bandwidth = list_of_max_bandwidth[indexes_selected_arm[3]];		// Max bandwidth
		}

		/**
		* Find the action index according to the global configuration report
		* @param "configuration" [type Configuration]: configuration report including all the parameters currently selected
		* @param "indexes_selected_arm" [type int*]: array containing the index of each parameter chosen by the WLAN
		* @return "action_ix" [type Configuration]: index of the action that corresponds to the input configuration
		*/
		int FindActionIndexFromConfigurationBandits(Configuration configuration, int *indexes_selected_arm, bool received_from_ap) {

		    // Find the index of each chosen parameter
			int index_channel(-1);
			int index_pd(-1);
			int index_tx_power(-1);
			int index_max_bandwidth(-1);

			// Channel
			for(int i = 0; i < num_arms_channel; i++) {
				if(configuration.selected_primary_channel == list_of_channels[i]) {
					index_channel = i;
				}
			}
			// Packet Detection (PD) threshold
			double selected_pd;

			if(received_from_ap && configuration.spatial_reuse_enabled) {
				selected_pd = configuration.non_srg_obss_pd;
			} else {
				selected_pd = configuration.selected_pd;
			}

			for(int i = 0; i < num_arms_sensitivity; i++) {
				if(selected_pd == list_of_pd_values[i]) {
					index_pd = i;
				}
			}
			// Tx Power
			for(int i = 0; i < num_arms_tx_power; i++) {
				if(configuration.selected_tx_power == list_of_tx_power_values[i]) {
					index_tx_power = i;
				}
			}
			// Max bandwidth
			for(int i = 0; i < num_arms_max_bandwidth; i++) {
				if(configuration.selected_max_bandwidth == list_of_max_bandwidth[i]) {
					index_max_bandwidth = i;
				}
			}

			//printf("index_channel = %d / index_pd = %d / index_tx_power = %d / index_max_bandwidth = %d\n",
			//		index_channel, index_pd, index_tx_power, index_max_bandwidth);

			// Update the index of each chosen parameter
			indexes_selected_arm[0] = index_channel;
			indexes_selected_arm[1] = index_pd;
			indexes_selected_arm[2] = index_tx_power;
			indexes_selected_arm[3] = index_max_bandwidth;
			// Find the action ix and return it
			int action_ix = values2index(indexes_selected_arm, num_arms_channel,
				num_arms_sensitivity, num_arms_tx_power, num_arms_max_bandwidth);
			//printf("action_ix = %d\n", action_ix);
			//PrintActionBandits(action_ix);

			return action_ix;
		}

		/******************/
		/******************/
		/*  RTOT METHODS  */
		/******************/
		/******************/

		/**
		* Encapsulate the configuration of a node to be sent
		* @param "configuration" [type Configuration]: current configuration report from the AP
		* @param "action_ix" [type int]: index of the action that corresponds to the new configuration
		* @return "new_configuration" [type Configuration]: configuration report, corresponding to the new action
		*/
		void GenerateNewConfigurationRtotAlg(Configuration *configuration, int ml_output){
			// Set configuration to the received one, and then change specific parameters
			configuration->non_srg_obss_pd = ml_output;
		}


		/*************************/
		/*************************/
		/*  PRINT/WRITE METHODS  */
		/*************************/
		/*************************/

		/**
		* Print a given action
		* @param "action_ix" [type int]: index of the action to be printed
		*/
		void PrintActionBandits(int action_ix){
			index2values(indexes_selected_arm, action_ix, num_arms_channel,
				num_arms_sensitivity, num_arms_tx_power, num_arms_max_bandwidth);
			printf("%s Action %d ([%d %d %d %d]\n", LOG_LVL2,
				action_ix, indexes_selected_arm[0], indexes_selected_arm[1], indexes_selected_arm[2], indexes_selected_arm[3]);
			printf("%s Channel: %d\n", LOG_LVL3, list_of_channels[indexes_selected_arm[0]]);
			printf("%s pd: %.2f dBm\n", LOG_LVL3,
				ConvertPower(PW_TO_DBM, list_of_pd_values[indexes_selected_arm[1]]));
			printf("%s Tx Power: %.2f dBm\n", LOG_LVL3,
				ConvertPower(PW_TO_DBM, list_of_tx_power_values[indexes_selected_arm[2]]));
			printf("%s Max bandwidth: %d\n", LOG_LVL3, list_of_max_bandwidth[indexes_selected_arm[3]]);
		}

		/**
		* Print the list of available actions
		* @param "action_ix" [type int]: index of the action to be printed
		*/
		void PrintOrWriteAvailableActions(int print_or_write, char string_device[],
			int save_logs, Logger &logger, double sim_time, int *list_of_available_actions) {

			switch(print_or_write) {
				case PRINT_LOG: {
					printf("%s List of available actions: ", string_device);
					for (int i = 0; i < num_arms; ++i) {
						printf("%d ", list_of_available_actions[i]);
					}
					printf("\n");
					break;
				}
				case WRITE_LOG: {
					LOGS(save_logs,logger.file,
						"%.15f;%s;%s;%s List of available actions: ",
						sim_time, string_device, LOG_C00, LOG_LVL2);
					for (int i = 0; i < num_arms; ++i) {
						LOGS(save_logs, logger.file, "%d ", list_of_available_actions[i]);
					}
					LOGS(save_logs,logger.file, "\n");
				}
			}

		}

		/**
		* Print the available reward types
		*/
		void PrintAvailableRewardTypes(){
			printf("%s Available types of rewards:\n%s REWARD_TYPE_PACKETS_SUCCESSFUL (%d)\n"
				"%s REWARD_TYPE_AVERAGE_THROUGHPUT (%d)\n%s REWARD_TYPE_MIN_RSSI (%d)\n"
				"%s REWARD_TYPE_MAX_DELAY (%d)\n%s REWARD_TYPE_AVERAGE_DELAY (%d)\n%s REWARD_TYPE_CHANNEL_OCCUPANCY (%d)\n",
				LOG_LVL2, LOG_LVL3, REWARD_TYPE_PACKETS_SUCCESSFUL, LOG_LVL3, REWARD_TYPE_AVERAGE_THROUGHPUT,
				LOG_LVL3, REWARD_TYPE_MIN_RSSI, LOG_LVL3, REWARD_TYPE_MAX_DELAY, LOG_LVL3, REWARD_TYPE_AVERAGE_DELAY,
				LOG_LVL3, REWARD_TYPE_CHANNEL_OCCUPANCY);
		}

		/**
		 * Print the available ML mechanisms types
		 */
		void PrintAvailableLearningMechanisms(){
			printf("%s Available types of learning mechanisms:\n", LOG_LVL2);
			printf("%s MULTI_ARMED_BANDITS (%d)\n", LOG_LVL3, MULTI_ARMED_BANDITS);
		}

		/**
		 * Write the performance of the Agent into the agent logs file
		 * @param "performance_to_write" [type Performance]: performance object to be written
		 */
		void WritePerformance(Logger &logger, double sim_time, char string_device[],
				Performance performance, int type_of_reward, double reward) {

			LOGS(TRUE, logger.file, "%.15f;%s;%s;%s Performance:\n", sim_time, string_device, LOG_C03, LOG_LVL2);
			switch(type_of_reward) {
				// Packets successful ratio
				case REWARD_TYPE_PACKETS_SUCCESSFUL:{
					LOGS(TRUE, logger.file,
						"%.15f;%s;%s;%s Packet successful ratio = %f\n", sim_time, string_device, LOG_C03, LOG_LVL3,
						(double)((performance.data_packets_sent-performance.data_packets_lost)/performance.data_packets_sent));
					break;
				}
				// Average Throughput (Mbps)
				case REWARD_TYPE_AVERAGE_THROUGHPUT:{
					LOGS(TRUE, logger.file,
						"%.15f;%s;%s;%s Average throughput = %.2f Mbps\n", sim_time, string_device,
						LOG_C03, LOG_LVL3, performance.throughput * pow(10,-6));
					LOGS(TRUE, logger.file,
						"%.15f;%s;%s;%s Average delay = %.4f s\n", sim_time, string_device,
						LOG_C03, LOG_LVL3, performance.average_delay);
					LOGS(TRUE, logger.file,
						"%.15f;%s;%s;%s Maximum delay = %.4f s\n", sim_time, string_device,
						LOG_C03, LOG_LVL3, performance.max_waiting_time);
					LOGS(TRUE, logger.file,
							"%.15f;%s;%s;%s Minimum delay = %.4f s\n", sim_time, string_device,
							LOG_C03, LOG_LVL3, performance.min_waiting_time);
//					LOGS(TRUE, logger.file,
//						"%.15f;%s;%s;%s Average delay = %.4f s\n", sim_time, string_device,
//						LOG_C03, LOG_LVL3, performance.average_delay);
//					LOGS(TRUE, logger.file,
//						"%.15f;%s;%s;%s Maximum delay = %.4f s\n", sim_time, string_device,
//						LOG_C03, LOG_LVL3, performance.max_delay);
//					LOGS(TRUE, logger.file,
//							"%.15f;%s;%s;%s Minimum delay = %.4f s\n", sim_time, string_device,
//							LOG_C03, LOG_LVL3, performance.min_delay);
					break;
				}
				// Minimum RSSI
				case REWARD_TYPE_MIN_RSSI:{
					LOGS(TRUE, logger.file,
						"%.15f;%s;%s;%s Min RSSI = %.2f dBm\n", sim_time, string_device,
						LOG_C03, LOG_LVL3, performance.rssi_list_per_sta[0]);
					break;
				}
				// Maximum delay
				case REWARD_TYPE_MAX_DELAY:{
					LOGS(TRUE, logger.file,
						"%.15f;%s;%s;%s Max delay = %.2f ms\n", sim_time, string_device,
						LOG_C03, LOG_LVL3, performance.max_delay * pow(10,-3));
					break;
				}
				// Minimum delay
				case REWARD_TYPE_MIN_DELAY:{
					LOGS(TRUE, logger.file,
						"%.15f;%s;%s;%s Min delay = %.2f ms\n", sim_time, string_device,
						LOG_C03, LOG_LVL3, performance.min_delay * pow(10,-3));
					break;
				}
				// Average delay
				case REWARD_TYPE_AVERAGE_DELAY:{
					LOGS(TRUE, logger.file,
						"%.15f;%s;%s;%s Average delay = %.2f ms\n", sim_time, string_device,
						LOG_C03, LOG_LVL3, performance.average_delay * pow(10,-3));
					break;
				}
				// Average delay
				case REWARD_TYPE_CHANNEL_OCCUPANCY:{
					LOGS(TRUE, logger.file,
						"%.15f;%s;%s;%s Successful channel successful_channel_occupancy = %.2f\n", sim_time, string_device,
						LOG_C03, LOG_LVL3, performance.successful_channel_occupancy);
					break;
				}
			}
			LOGS(TRUE, logger.file,
				"%.15f;%s;%s;%s Associated reward = %f\n", sim_time, string_device, LOG_C03, LOG_LVL3, reward);
		}

		/***********************/
		/***********************/
		/*  AUXILIARY METHODS  */
		/***********************/
		/***********************/

		/**
		* Given different lists of parameters, generate the index in each list according to the index of the joint action (which considers each parameter)
		* @param "indexes" [type int*]: array we want to fill (3 positions, one for each parameter - channel, PD, Tx power) (to be updated by this method)
		* @param "action_ix" [type int]: index of the action, which represents a combination of channel, PD and tx power
		* @param "size_channels" [type int]: size of channels possibilities
		* @param "size_pd" [type int]: size of pd possibilities
		* @param "size_tx_power" [type int]: size of Tx power possibilities
		*/
		void index2values(int *indexes, int action_ix, int size_channels, int size_pd, int size_tx_power, int size_max_bandwidth) {
			indexes[0] = (int) action_ix/(size_pd * size_tx_power * size_max_bandwidth);
			indexes[1] = (int) (action_ix - indexes[0] * (size_pd * size_tx_power * size_max_bandwidth))
					/(size_tx_power * size_max_bandwidth);
			indexes[2] = (int) (action_ix -  indexes[0] * (size_pd * size_tx_power * size_max_bandwidth)
					- indexes[1] * (size_tx_power * size_max_bandwidth))
					/(size_max_bandwidth);
			indexes[3] = action_ix % size_max_bandwidth;
		}

		/**
		* Given different indexes of actions, generate the index of the joint action (which represents a combination of each parameter)
		* @param "indexes" [type int*]: array we want to fill (3 positions, one for each parameter - channel, PD, Tx power)
		* @param "size_channels" [type int]: size of channels possibilities
		* @param "size_pd" [type int]: size of pd possibilities
		* NOTE: size of tx power elements is not necessary
		* @return  "index" [type int]: index of the action, which represents a combination of channel, pd and tx power
		*/
		int values2index(int *indexes, int size_channels, int size_pd, int size_tx_power, int size_max_bandwidth) {
			int index = indexes[0] * (size_pd * size_tx_power * size_max_bandwidth)
				+ indexes[1] * (size_tx_power * size_max_bandwidth)
				+ indexes[2] * size_max_bandwidth
				+ indexes[3];
			return index;
		}

		/**
		 * Initialize the variables used by the PP
		 */
		void InitializeVariables(){
			// Lists of modifiable parameters
			list_of_channels = new int[num_arms_channel];
			for(int i = 0; i < num_arms_channel; ++i){
				list_of_channels[i] = 0;
			}
			list_of_pd_values = new double[num_arms_sensitivity];
			for(int i = 0; i < num_arms_sensitivity; ++i){
				list_of_pd_values[i] = 0;
			}
			list_of_tx_power_values = new double[num_arms_tx_power];
			for(int i = 0; i < num_arms_tx_power; ++i){
				list_of_tx_power_values[i] = 0;
			}
			list_of_max_bandwidth = new int[num_arms_max_bandwidth];
			for(int i = 0; i < num_arms_max_bandwidth; ++i){
				list_of_max_bandwidth[i] = 0;
			}
			// Variable to keep track of the indexes belonging to each parameter's list
			indexes_selected_arm = new int[NUM_FEATURES_ACTIONS]; // 4 features considered
			for (int i = 0 ; i < NUM_FEATURES_ACTIONS; ++i) {
				indexes_selected_arm[i] = 0;
			}
		}

};

#endif
