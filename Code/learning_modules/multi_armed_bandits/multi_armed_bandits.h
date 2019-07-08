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
 * File description: this file contains functions related to the agents' operation
 *
 * - This file contains the methods related to "time" operations
 */

#include "../../list_of_macros.h"
#include "action_selection_strategies/epsilon_greedy.h"
#include "action_selection_strategies/thompson_sampling.h"
//#include "action_selection_strategies/thompson_sampling.h"

#ifndef _AUX_MABS_
#define _AUX_MABS_

class MultiArmedBandit {

	// Public items
	public:

		int agent_id;
		int save_agent_logs;
		int print_agent_logs;
		int num_actions;
		int type_of_reward;

		int selected_strategy;

		int num_actions_channel;
		int num_actions_sensitivity;
		int num_actions_tx_power;
		int num_actions_dcb_policy;

		int *list_of_channels;
		double *list_of_pd_values;
		double *list_of_tx_power_values;
		int *list_of_dcb_policy;

	// Private items
	private:

		// Generic variables to all the learning strategies
		int initial_reward;
		int num_iterations;

		// Arms statistics
		double *reward_per_arm;
		double *cumulative_reward_per_arm;
		double *average_reward_per_arm;
		double *estimated_reward_per_arm;
		int *times_arm_has_been_selected;
		int *indexes_selected_arm;

		// e-greedy specific variables
		double initial_epsilon;
		double epsilon;

	// Methods
	public:

		/******************/
		/******************/
		/*  MAIN METHODS  */
		/******************/
		/******************/

		/*
		 * UpdateConfiguration(): main method for updating the configuration according to past experience
		* INPUT:
		 * 	- configuration: current configuration report from the AP
		 * 	- performance: current performance report from the AP
		 * 	- agent_logger: logger object to write logs
		 * 	- sim_time: current simulation time
		 * OUTPUT:
		 *  - suggested_configuration: configuration suggested to the AP
		 */
		Configuration UpdateConfiguration(Configuration configuration, Performance performance,
				Logger &agent_logger, double sim_time) {

			// Find the action ix according to the AP's configuration
			int current_action_ix = FindActionIndexFromConfiguration(configuration, indexes_selected_arm);

			// Generate the reward for the last selected action
			double reward = GenerateReward(type_of_reward, performance);
			if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s Throughput (action %d) = %f\n",
				sim_time, agent_id, LOG_C00, LOG_LVL2, current_action_ix, performance.throughput);
			if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s Reward (action %d) = %f\n",
				sim_time, agent_id, LOG_C00, LOG_LVL2, current_action_ix, reward);
			if(save_agent_logs) PrintOrWriteRewardPerArm(WRITE_LOG, agent_logger, sim_time);

			// Update the reward of the last played configuration
			UpdateArmStatistics(current_action_ix, reward);

//			printf("current_action_ix = %d\n", current_action_ix);
//			printf("reward = %f\n", reward);
//
//			for (int i = 0 ; i < num_actions ; i ++) {
//				printf("Action %d:", i);
//				printf("  - estimated_reward_per_arm[i] = %f\n", estimated_reward_per_arm[i]);
//				printf("  - times_arm_has_been_selected[i] = %d\n", times_arm_has_been_selected[i]);
//			}

			// Select a new action according to the updated information
			int new_action_ix = SelectNewAction();

//			printf("new_action_ix = %d\n", new_action_ix);

			// Generate the suggested configuration and return it
			Configuration suggested_configuration = GenerateNewConfiguration(configuration, new_action_ix);
			return suggested_configuration;

		};

		/*
		 * UpdateRewardStatistics(): updates the statistics maintained for each arm
		 * INPUT:
		 * 	- action_ix: index of the action to be updated
		 * 	- reward: last reward observed from the action of interest
		 **/
		void UpdateArmStatistics(int action_ix, double reward){

			if(action_ix >= 0) { // Avoid indexing errors
				// Update the reward for the chosen arm
				reward_per_arm[action_ix] = reward;
				// Update the times the chosen arm has been selected
				times_arm_has_been_selected[action_ix] += 1;
				// Update the cumulative reward for the chosen arm
				cumulative_reward_per_arm[action_ix] += reward;
				// Update the average reward for the chosen arm
				average_reward_per_arm[action_ix] = cumulative_reward_per_arm[action_ix] /
					times_arm_has_been_selected[action_ix];
				// Update the estimated reward per arm
				estimated_reward_per_arm[action_ix] = ((estimated_reward_per_arm[action_ix]
					* times_arm_has_been_selected[action_ix])
					+ reward) / (times_arm_has_been_selected[action_ix] + 2);
			} else {
				printf("ERROR: The action ix (%d) is not correct!\n", action_ix);
				exit(EXIT_FAILURE);
			}

		}

		/*
		 * SelectNewAction(): selects a new action according to the chosen action selection strategy
		 * OUTPUT:
		 *  - action_ix: index of the selected action
		 */
		int SelectNewAction() {

			int action_ix;
			// Select an action according to the chosen strategy: TODO improve this part (now it is hardcoded)
			//int selected_strategy = STRATEGY_EGREEDY;
			switch(selected_strategy) {

				/*
				 * epsilon-greedy strategy:
				 */
				case STRATEGY_EGREEDY:{

					// Update epsilon
					epsilon = initial_epsilon / sqrt( (double) num_iterations);
					// Pick an action according to e-greedy
					action_ix = PickArmEgreedy(num_actions, average_reward_per_arm, epsilon);
					// Increase the number of iterations
					num_iterations ++;

					break;
				}

				/*
				 * Thompson sampling strategy:
				 */
				case STRATEGY_THOMPSON_SAMPLING:{

					// Pick an action according to Thompson sampling
					action_ix = PickArmThompsonSampling(num_actions,
						estimated_reward_per_arm, times_arm_has_been_selected);
					// Increase the number of iterations
					num_iterations ++;

					break;

				}

			}

			return action_ix;

		}

		/***********************/
		/***********************/
		/*  AUXILIARY METHODS  */
		/***********************/
		/***********************/

		/*
		 * InitializeVariables(): initializes the variables used by the MAB
		 */
		void InitializeVariables(){

			// TODO: generate file that stores algorithm-specific variables
			initial_epsilon = 1;
			epsilon = initial_epsilon;

			initial_reward = 0;
			num_iterations = 1;

			// Initialize the rewards assigned to each arm
			reward_per_arm = new double[num_actions];
			cumulative_reward_per_arm = new double[num_actions];
			average_reward_per_arm = new double[num_actions];
			estimated_reward_per_arm = new double[num_actions];

			// Initialize the array containing the times each arm has been played
			times_arm_has_been_selected = new int[num_actions];

			for(int i = 0; i < num_actions; i++){
				reward_per_arm[i] = initial_reward;	// Set the initial reward
				cumulative_reward_per_arm[i] = initial_reward;
				average_reward_per_arm[i] = initial_reward;
				estimated_reward_per_arm[i] = initial_reward;
				times_arm_has_been_selected[i] = 0;
			}

			list_of_channels = new int[num_actions_channel];
			list_of_pd_values = new double[num_actions_sensitivity];
			list_of_tx_power_values = new double[num_actions_tx_power];
			list_of_dcb_policy = new int[num_actions_dcb_policy];

			// Variable to keep track of the indexes belonging to each parameter's list
			indexes_selected_arm = new int[NUM_FEATURES_ACTIONS]; // 4 features considered

		}

		/*
		 * GenerateNewConfiguration: encapsulates the configuration of a node to be sent
		 * INPUT:
		 * 	- configuration: configuration report including all the parameters currently selected
		 * 	- action_ix: index of the action that corresponds to the new configuration
		 * OUTPUT:
		 *  - new_configuration: new configuration report, corresponding to the new action
		 */
		Configuration GenerateNewConfiguration(Configuration configuration, int action_ix){

			// Find which parameters correspond to the selected arm
			index2values(indexes_selected_arm, action_ix, num_actions_channel,
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

		/*
		 * FindIndexesOfConfiguration(): given a configuration, fills the "indexes_selected_arm" object, which
		 * includes the index of each parameter
		 * INPUT:
		 * 	- configuration: configuration of the AP
		 * 	- indexes_selected_arm: array to be filled
		 */
		void FindIndexesOfConfiguration(Configuration configuration, int *indexes_selected_arm) {

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
			// pd
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

			indexes_selected_arm[0] = index_channel;
			indexes_selected_arm[1] = index_pd;
			indexes_selected_arm[2] = index_tx_power;
			indexes_selected_arm[3] = index_dcb_policy;

		}

		/*
		 * FindActionIndexFromConfiguration(): finds the action index according to the global configuration report
		 * INPUT:
		 * 	- configuration: configuration report including all the parameters currently selected
		 * 	- indexes_selected_arm: array containing the index of each parameter chosen by the WLAN
		 * OUTPUT:
		 *  - action_ix: index of the action that corresponds to the input configuration
		 */
		int FindActionIndexFromConfiguration(Configuration configuration, int* &indexes_selected_arm) {

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
			// pd
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

		/*
		 * index2values(): given different lists of parameters, outputs the index in each list
		 * according to the index of the joint action (which considers each parameter)
		 * INPUT:
		 * 	- indexes: array we want to fill (3 positions, one for each parameter - channel, pd, Tx power)
		 * 	- action_ix: index of the action, which represents a combination of channel, pd and tx power
		 * 	- size_channels: size of channels possibilities
		 * 	- size_pd: size of pd possibilities
		 * 	- size_tx_power: size of Tx power possibilities
		 * 	- size_dcb_policy: size of the DCB policy possibilities
		 * OUTPUT:
		 *  - fills "indexes", which indicates the index of the parameter in each of the lists
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

		/*
		 * values2index(): given different indexes of actions, outputs the index of the
		 * joint action (which represents a combination of each parameter)
		 * INPUT:
		 * 	- indexes: array of 3 positions, one for each parameter - channel, pd, Tx power)
		 * 	- size_channels: size of channels possibilities
		 * 	- size_pd: size of pd possibilities
		 *	- NOTE: size of tx power elements is not necessary
		 * OUTPUT:
		 *  - index: index of the action, which represents a combination of channel, pd and tx power
		 */
		int values2index(int *indexes, int size_channels, int size_pd, int size_tx_power, int size_dcb_policy) {

			int index = indexes[0] * (size_pd * size_tx_power * size_dcb_policy)
				+ indexes[1] * (size_tx_power * size_dcb_policy)
				+ indexes[2] * size_dcb_policy
				+ indexes[3];

			return index;

		}

		/*************************/
		/*************************/
		/*  PRINT/WRITE METHODS  */
		/*************************/
		/*************************/

		/*
		 * PrintAction(): prints a given action
		 * INPUT:
		 * 	- action_ix: index of the action to be printed
		 */
		void PrintAction(int action_ix){

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

		/*
		 * printOrWriteNodesTransmitting: prints (or writes) the array representing the transmitting nodes.
		 * INPUT:
		 * 	- write_or_print: variable to indicate whether to print on the  console or to write on the the output logs file
		 * 	- aggent_logger: logger object to write on the output file
		 * 	- sim_time: simulation time
		 */
		void PrintOrWriteRewardPerArm(int write_or_print, Logger &agent_logger, double sim_time) {

			switch(write_or_print){
				case PRINT_LOG:{
					if(print_agent_logs){
						printf("Reward per arm: ");
						for(int n = 0; n < num_actions; n++){
							printf("%f  ", reward_per_arm[n]);
						}
						printf("\nCumulative reward per arm: ");
						for(int n = 0; n < num_actions; n++){
							printf("%f  ", cumulative_reward_per_arm[n]);
						}
						printf("\nTimes each arm has been selected: ");
						for(int n = 0; n < num_actions; n++){
							printf("%d  ", times_arm_has_been_selected[n]);
						}
						printf("\n");
					}
					break;
				}
				case WRITE_LOG:{
					if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s Reward per arm: ",
						sim_time, agent_id, LOG_C00, LOG_LVL3);
					for(int n = 0; n < num_actions; n++){
						 if(save_agent_logs){
							 fprintf(agent_logger.file, "%f  ", reward_per_arm[n]);
						 }
					}
					if(save_agent_logs) fprintf(agent_logger.file, "\n%.15f;A%d;%s;%s Cumulative reward per arm: ",
						sim_time, agent_id, LOG_C00, LOG_LVL3);
					for(int n = 0; n < num_actions; n++){
						 if(save_agent_logs){
							 fprintf(agent_logger.file, "%f  ", cumulative_reward_per_arm[n]);
						 }
					}
					fprintf(agent_logger.file, "\n%.15f;A%d;%s;%s Times each arm has been selected: ",
									sim_time, agent_id, LOG_C00, LOG_LVL3);
					for(int n = 0; n < num_actions; n++){
						if(save_agent_logs){
							fprintf(agent_logger.file, "%d ", times_arm_has_been_selected[n]);
						}
					}
					if(save_agent_logs) fprintf(agent_logger.file, "\n");
					break;
				}
			}
		}

		/*
		 * PrintOrWriteAgentStatistics(): prints (or writes) final statistics at the given agent
		 * INPUT:
		 * 	- write_or_print: variable to indicate whether to print on the  console or to write on the the output logs file
		 * 	- aggent_logger: logger object to write on the output file
		 * 	- sim_time: simulation time
		 */
		void PrintOrWriteAgentStatistics(int write_or_print, Logger &agent_logger, double sim_time) {

			// Process statistics
			// ...

			switch(write_or_print){

				case PRINT_LOG:{

					if (print_agent_logs) {
//						printf("------- Agent A%d (WLAN %s) ------\n", agent_id, wlan_code.c_str());
						printf("* Actions performance:\n");
						// Detailed summary of arms
						for(int i = 0; i < num_actions; i++){
							if (print_agent_logs) printf("%s times_arm_has_been_selected  = %d\n", LOG_LVL3, times_arm_has_been_selected[i]);
							if (print_agent_logs) printf("%s average_reward_per_arm  = %f\n", LOG_LVL3, average_reward_per_arm[i]);
						}
						printf("\n\n");
					}
					break;
				}

				case WRITE_LOG:{

					if (save_agent_logs){

						fprintf(agent_logger.file, "******************************************\n");

						// Detailed summary of arms
						int indexes_selected_arm[NUM_FEATURES_ACTIONS];

						for(int i = 0; i < num_actions; i++){

							index2values(indexes_selected_arm, i, num_actions_channel,
									num_actions_sensitivity, num_actions_tx_power, num_actions_dcb_policy);
							// Write logs

							fprintf(agent_logger.file, "%.15f;A%d;%s;%s Action %d:\n", sim_time, agent_id, LOG_C03, LOG_LVL2, i);
							fprintf(agent_logger.file, "%.15f;A%d;%s;%s Channel: %d\n", sim_time, agent_id, LOG_C03, LOG_LVL3,list_of_channels[indexes_selected_arm[0]]);
							fprintf(agent_logger.file, "%.15f;A%d;%s;%s pd: %f (%f dBm)\n", sim_time, agent_id, LOG_C03, LOG_LVL3, list_of_pd_values[indexes_selected_arm[1]],
									ConvertPower(PW_TO_DBM, list_of_pd_values[indexes_selected_arm[1]]));
							fprintf(agent_logger.file, "%.15f;A%d;%s;%s Tx Power: %f (%f dBm)\n", sim_time, agent_id, LOG_C03, LOG_LVL3, list_of_tx_power_values[indexes_selected_arm[2]],
									ConvertPower(PW_TO_DBM, list_of_tx_power_values[indexes_selected_arm[2]]));
							fprintf(agent_logger.file, "%.15f;A%d;%s;%s DCB policy: %d\n", sim_time, agent_id, LOG_C03, LOG_LVL3,list_of_dcb_policy[indexes_selected_arm[3]]);

							fprintf(agent_logger.file, "%.15f;A%d;%s;%s times_arm_has_been_selected = %d\n",
									sim_time, agent_id, LOG_C03, LOG_LVL3, times_arm_has_been_selected[i]);
							fprintf(agent_logger.file, "%.15f;A%d;%s;%s average_reward_per_arm = %f\n",
									sim_time, agent_id, LOG_C03, LOG_LVL3, average_reward_per_arm[i]);

						}
					}
					break;
				}
			}
		}

};

#endif
