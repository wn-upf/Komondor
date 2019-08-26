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


#include "../../list_of_macros.h"
#include "../../structures/node_configuration.h"
#include "../../structures/performance_metrics.h"

#ifndef _AUX_PP_
#define _AUX_PP_

class PreProcessor {

	// Public items
	public:

		int agent_id;						///> Identified of the agent instantiating the PP
		int num_actions;					///> Number of actions
		int type_of_reward;					///> Index indicating the type of reward

		int selected_strategy;				///> Index of the chosen action-selection strategy

		int num_actions_channel;			///> Number of channel actions
		int num_actions_sensitivity;		///> Number of sensitivity actions
		int num_actions_tx_power;			///> Number of transmission power actions
		int num_actions_dcb_policy;			///> Number of DCB policy actions

		int *list_of_channels;				///> List of channels to be selected
		double *list_of_pd_values;			///> List of PD values to be selected
		double *list_of_tx_power_values;	///> List of Tx Power values to be selected
		int *list_of_dcb_policy;			///> List of DCB policies to be selected

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
		int ProcessWlanConfiguration(int learning_mechanism, Configuration configuration) {
			int processed_configuration(0);
			// Update the current configuration according to the selected learning method
			switch(learning_mechanism) {
				/* Multi-Armed Bandits */
				case MULTI_ARMED_BANDITS:{
					// Convert the WLAN's configuration into an arm/action index value
					processed_configuration = FindActionIndexFromConfigurationBandits(configuration, indexes_selected_arm);
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
		double ProcessWlanPerformance(int learning_mechanism, Performance performance, int type_of_reward) {
			double processed_performance(0);
			// Update the current configuration according to the selected learning method
			switch(learning_mechanism) {
				/* Multi-Armed Bandits */
				case MULTI_ARMED_BANDITS:{
					// Generate the reward for the last selected action
					processed_performance = GenerateReward(type_of_reward, performance);
					break;
				}
				case RTOT_ALGORITHM:{
					// Generate the reward for the last selected action
					processed_performance = GenerateReward(type_of_reward, performance);
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
			return processed_performance;
		};

		/**
		* Based on the performance report of the WLAN, generate the reward to be passed to the ML method
		* @param "type_of_reward" [type int]: type of performance metric to be used as a reward
		* @param "performance" [type Performance]: current performance report from the AP
		* @return "reward" [type double]: reward to be passed to the ML method
		*/
		double GenerateReward(int type_of_reward, Performance performance) {
			double reward;
			// Switch to select the reward according to the metric used (rewards must be normalized)
			switch(type_of_reward){
				/* PERFORMANCE_PACKETS_SENT:
				 * - The number of packets sent are taken into account
				 * - The reward must be bounded by the maximum number of data packets
				 * 	 that can be sent in each interval (e.g., packets that were sent but lost)
				 */
				case REWARD_TYPE_PACKETS_SENT:{
					reward = performance.data_packets_sent/performance.data_packets_lost;
					break;
				}
				/* PERFORMANCE_THROUGHPUT:
				 * - The throughput experienced during the last period is taken into account
				 * - The reward must be bounded by the maximum throughput that would be experienced
				 * 	 (e.g., consider the data rate granted by the modulation and the total time)
				 */
				case REWARD_TYPE_THROUGHPUT:{
					if (performance.max_bound_throughput == 0) {
						reward = 0;
					} else {
						reward = (double) performance.throughput/performance.max_bound_throughput;
					}
					break;
				}
				/* REWARD_TYPE_PACKETS_GENERATED:
				 * -
				 */
				case REWARD_TYPE_PACKETS_GENERATED:{
					reward = (performance.num_packets_generated - performance.num_packets_dropped) /
						performance.num_packets_generated;
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
				/* Default */
				default:{
					printf("[Pre-Processor] ERROR: '%d' is not a correct type of performance indicator\n", type_of_reward);
					PrintAvailableRewardTypes();
					exit(EXIT_FAILURE);
					break;
				}
			}
			return reward;
		}

		/**
		* Process the output of the ML method and return a configuration struct
		* @param "learning_mechanism" [type int]: learning method employed
		* @param "configuration" [type Configuration]: current configuration report from the AP
		* @param "ml_output" [type int]: output of the ML method
		* @return "suggested_configuration" [type Configuration]: new configuration to be sent to the WLAN
		*/
		Configuration ProcessMLOutput(int learning_mechanism, Configuration configuration, double ml_output) {
			Configuration suggested_configuration;
			// Switch to select the reward according to the metric used (rewards must be normalized)
			switch(learning_mechanism){
				case MULTI_ARMED_BANDITS:{
					suggested_configuration = GenerateNewConfigurationBandits(configuration, ml_output);
					break;
				}
				case RTOT_ALGORITHM:{
					suggested_configuration = GenerateNewConfigurationRtotAlg(configuration, ml_output);
					break;
				}
				/* Default */
				default:{
					printf("[PP] ERROR: %d is not a correct type of learning mechanism. Returning the default configuration.\n", learning_mechanism);
					suggested_configuration = configuration;
					break;
				}
			}
			return suggested_configuration;
		};

		/***********************/
		/***********************/
		/*  BANDITS METHODS  */
		/***********************/
		/***********************/

		/**
		* Encapsulate the configuration of a node to be sent
		* @param "configuration" [type Configuration]: current configuration report from the AP
		* @param "action_ix" [type int]: index of the action that corresponds to the new configuration
		* @return "new_configuration" [type Configuration]: configuration report, corresponding to the new action
		*/
		Configuration GenerateNewConfigurationBandits(Configuration configuration, double ml_output){
			// Find which parameters correspond to the selected arm
			index2values(indexes_selected_arm, (int) ml_output, num_actions_channel,
				num_actions_sensitivity, num_actions_tx_power, num_actions_dcb_policy);
			// Update each parameter according to the configuration provided by the MAB
			int new_primary = list_of_channels[indexes_selected_arm[0]];
			double new_pd = list_of_pd_values[indexes_selected_arm[1]];
			double new_tx_power = list_of_tx_power_values[indexes_selected_arm[2]];
			int new_dcb_policy = list_of_dcb_policy[indexes_selected_arm[3]];
			// Generate the configuration object
			Configuration new_configuration;
			// Set configuration to the received one, and then change specific parameters
			new_configuration = configuration;
			//new_configuration.timestamp = sim_time;						// Timestamp
			new_configuration.selected_primary_channel = new_primary;	// Primary
			new_configuration.selected_pd = new_pd;					// pd
			new_configuration.selected_tx_power = new_tx_power;			// TX Power
			new_configuration.selected_dcb_policy = new_dcb_policy;		// DCB policy
			return new_configuration;
		}

		/**
		* Encapsulate the configuration of a node to be sent
		* @param "configuration" [type Configuration]: current configuration report from the AP
		* @param "action_ix" [type int]: index of the action that corresponds to the new configuration
		* @return "new_configuration" [type Configuration]: configuration report, corresponding to the new action
		*/
		Configuration GenerateNewConfigurationRtotAlg(Configuration configuration, double ml_output){
			// Generate the configuration object
			Configuration new_configuration;
			// Set configuration to the received one, and then change specific parameters
			new_configuration = configuration;
			new_configuration.non_srg_obss_pd = ml_output;

			return new_configuration;
		}

		/**
		* Find the action index according to the global configuration report
		* @param "configuration" [type Configuration]: configuration report including all the parameters currently selected
		* @param "indexes_selected_arm" [type int*]: array containing the index of each parameter chosen by the WLAN
		* @return "action_ix" [type Configuration]: index of the action that corresponds to the input configuration
		*/
		int FindActionIndexFromConfigurationBandits(Configuration configuration, int* &indexes_selected_arm) {
			// Find the index of each chosen parameter
			int index_channel = -1;
			int index_pd = -1;
			int index_tx_power = -1;
			int index_dcb_policy = -1;
			// Channel
			for(int i = 0; i < num_actions_channel; i++) {
				if(configuration.selected_primary_channel == list_of_channels[i]) {
					index_channel = i;
				}
			}
			// Packet Detection (PD) threshold
			for(int i = 0; i < num_actions_sensitivity; i++) {
				if(configuration.selected_pd == list_of_pd_values[i]) {
					index_pd = i;
				}
			}
			// Tx Power
			for(int i = 0; i < num_actions_tx_power; i++) {
				if(configuration.selected_tx_power == list_of_tx_power_values[i]) {
					index_tx_power = i;
				}
			}
			// DCB policy
			for(int i = 0; i < num_actions_dcb_policy; i++) {
				if(configuration.selected_dcb_policy == list_of_dcb_policy[i]) {
					index_dcb_policy = i;
				}
			}
			// Update the index of each chosen parameter
			indexes_selected_arm[0] = index_channel;
			indexes_selected_arm[1] = index_pd;
			indexes_selected_arm[2] = index_tx_power;
			indexes_selected_arm[3] = index_dcb_policy;
			// Find the action ix and return it
			int action_ix = values2index(indexes_selected_arm, num_actions_channel,
				num_actions_sensitivity, num_actions_tx_power, num_actions_dcb_policy);
			return action_ix;
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
			index2values(indexes_selected_arm, action_ix, num_actions_channel,
				num_actions_sensitivity, num_actions_tx_power, num_actions_dcb_policy);
			printf("%s Action %d ([%d %d %d %d]\n", LOG_LVL2,
				action_ix, indexes_selected_arm[0], indexes_selected_arm[1], indexes_selected_arm[2], indexes_selected_arm[3]);
			printf("%s Channel: %d\n", LOG_LVL3, list_of_channels[indexes_selected_arm[0]]);
			printf("%s pd: %.2f dBm\n", LOG_LVL3,
				ConvertPower(PW_TO_DBM, list_of_pd_values[indexes_selected_arm[1]]));
			printf("%s Tx Power: %.2f dBm\n", LOG_LVL3,
				ConvertPower(PW_TO_DBM, list_of_tx_power_values[indexes_selected_arm[2]]));
			printf("%s DCB policy: %d\n", LOG_LVL3, list_of_dcb_policy[indexes_selected_arm[3]]);
		}

		/**
		* Print the available reward types
		*/
		void PrintAvailableRewardTypes(){
			printf("%s Available types of rewards:\n%s REWARD_TYPE_PACKETS_SENT (%d)\n"
				"%s REWARD_TYPE_THROUGHPUT (%d)\n%s REWARD_TYPE_PACKETS_GENERATED (%d)\n",
				LOG_LVL2, LOG_LVL3, REWARD_TYPE_PACKETS_SENT, LOG_LVL3, REWARD_TYPE_THROUGHPUT,
				LOG_LVL3, REWARD_TYPE_PACKETS_GENERATED);
		}

		/**
		 * Print the available ML mechanisms types
		 */
		void PrintAvailableLearningMechanisms(){
			printf("%s Available types of learning mechanisms:\n", LOG_LVL2);
			printf("%s MULTI_ARMED_BANDITS (%d)\n", LOG_LVL3, MULTI_ARMED_BANDITS);
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
		void index2values(int *indexes, int action_ix, int size_channels, int size_pd, int size_tx_power, int size_dcb_policy) {
			indexes[0] = (int) action_ix/(size_pd * size_tx_power * size_dcb_policy);
			indexes[1] = (int) (action_ix - indexes[0] * (size_pd * size_tx_power * size_dcb_policy))
					/(size_tx_power * size_dcb_policy);
			indexes[2] = (int) (action_ix -  indexes[0] * (size_pd * size_tx_power * size_dcb_policy)
					- indexes[1] * (size_tx_power * size_dcb_policy))
					/(size_dcb_policy);
			indexes[3] = action_ix % size_dcb_policy;
		}

		/**
		* Given different indexes of actions, generate the index of the joint action (which represents a combination of each parameter)
		* @param "indexes" [type int*]: array we want to fill (3 positions, one for each parameter - channel, PD, Tx power)
		* @param "size_channels" [type int]: size of channels possibilities
		* @param "size_pd" [type int]: size of pd possibilities
		* NOTE: size of tx power elements is not necessary
		* @return  "index" [type int]: index of the action, which represents a combination of channel, pd and tx power
		*/
		int values2index(int *indexes, int size_channels, int size_pd, int size_tx_power, int size_dcb_policy) {
			int index = indexes[0] * (size_pd * size_tx_power * size_dcb_policy)
				+ indexes[1] * (size_tx_power * size_dcb_policy)
				+ indexes[2] * size_dcb_policy
				+ indexes[3];
			return index;
		}

		/**
		 * Initialize the variables used by the PP
		 */
		void InitializeVariables(){
			// Lists of modifiable parameters
			list_of_channels = new int[num_actions_channel];
			list_of_pd_values = new double[num_actions_sensitivity];
			list_of_tx_power_values = new double[num_actions_tx_power];
			list_of_dcb_policy = new int[num_actions_dcb_policy];
			// Variable to keep track of the indexes belonging to each parameter's list
			indexes_selected_arm = new int[NUM_FEATURES_ACTIONS]; // 4 features considered
		}

};

#endif
