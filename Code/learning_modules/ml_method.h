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
 * - This file contains the methods used by the ML Method in the Machine Learning (ML) pipeline.
 * 	 In particular, this module manages the ML operation
 */

#include "../list_of_macros.h"
#include "../structures/node_configuration.h"
#include "../structures/performance_metrics.h"
#include "/graph_coloring/graph_coloring.h"
#include "/multi_armed_bandits/multi_armed_bandits.h"

#ifndef _AUX_ML_METHOD_
#define _AUX_ML_METHOD_

class MlMethod {

	// Public items
	public:

		int learning_mechanism;

		// Information of the network
		int agents_number;
		int wlans_number;
		int total_nodes_number;

		// Parameters
		int num_channels;

		// Logs
		int save_controller_logs;
		int print_controller_logs;
		int save_agent_logs;
		int print_agent_logs;

		GraphColoring graph_coloring;

		int agent_id;

		// Multi-armed bandits
		MultiArmedBandit mab_agent;
		int action_selection_strategy;
		int num_actions;


	// Private items
	private:


	// Methods
	public:

		/********************************/
		/********************************/
		/*  CENTRAL CONTROLLER METHODS  */
		/********************************/
		/********************************/

		/*
		* ComputeNewConfiguration():
		* INPUT:
		*   - learning_mechanism: learning method employed
		* 	- configuration: current configuration report from the AP
		* OUTPUT:
		*   - processed_configuration: configuration to be introduced directly to the ML method
		*/
		void ComputeGlobalConfiguration(Configuration *configuration_array, Performance *performance_array,
				Logger &central_controller_logger, double sim_time) {

			switch(learning_mechanism) {
				/* GRAPH COLORING */
				case GRAPH_COLORING: {
					// Apply Hminmax to decide the new channels configuration
					graph_coloring.UpdateConfiguration(configuration_array,
						performance_array, central_controller_logger, sim_time);
					break;
				}
			}

		}

		int ComputeIndividualConfiguration(int arm_ix, double reward, Logger &agent_logger, double sim_time) {
			int new_action(0);
			switch(learning_mechanism) {
				/* MULTI_ARMED_BANDITS */
				case MULTI_ARMED_BANDITS: {
					// Update the reward of the last played configuration
					mab_agent.UpdateArmStatistics(arm_ix, reward);
					// Select a new action according to the updated information
					new_action = mab_agent.SelectNewAction();
					break;
				}
			}
			return new_action;
		};


		/*******************/
		/*******************/
		/*  OHTER METHODS  */
		/*******************/
		/*******************/


		/*
		 * InitializeVariables(): initializes the variables used by the MAB
		 */
		void InitializeVariables() {

			switch(learning_mechanism) {
				/* GRAPH COLORING */
				case GRAPH_COLORING: {
					// Initialize the graph coloring method
					graph_coloring.save_controller_logs = save_controller_logs;
					graph_coloring.print_controller_logs = print_controller_logs;
					graph_coloring.agents_number = agents_number;
					graph_coloring.wlans_number = wlans_number;
					graph_coloring.num_channels = num_channels;
					graph_coloring.total_nodes_number = total_nodes_number;
					// Initialize variables characteristic to the graph coloring method
					graph_coloring.InitializeVariables();
					break;
				}
				/* ACTION BANNING */
				case ACTION_BANNING: {
					break;
				}
				/* Multi-Armed Bandits */
				case MULTI_ARMED_BANDITS: {
					mab_agent.agent_id = agent_id;
					mab_agent.save_agent_logs = save_agent_logs;
					mab_agent.print_agent_logs = print_agent_logs;
					mab_agent.action_selection_strategy = action_selection_strategy;
					mab_agent.num_actions = num_actions;
					mab_agent.InitializeVariables();
					break;
				}

				//  TODO: provide more learning mechanisms
				// case Q_LEARNING:
				// ...

				/* UNKNOWN */
				default: {
					printf("[ML METHOD] ERROR: '%d' is not a correct learning mechanism\n", learning_mechanism);
					PrintAvailableLearningMechanisms();
					exit(EXIT_FAILURE);
					break;
				}
			}
		}

		/*
		 * PrintOrWriteStatistics(): initializes the variables used by the MAB
		 */
		void PrintOrWriteStatistics(int write_or_print, Logger &agent_logger, double sim_time) {

			switch(learning_mechanism) {
				/* GRAPH COLORING */
				case GRAPH_COLORING: {
					graph_coloring.PrintOrWriteStatistics(write_or_print);
					break;
				}
				/* Multi-Armed Bandits */
				case MULTI_ARMED_BANDITS: {
					mab_agent.PrintOrWriteStatistics(write_or_print, agent_logger, sim_time);
					break;
				}
				/* UNKNOWN */
				default: {
					printf("[ML METHOD] ERROR: '%d' is not a correct learning mechanism\n", learning_mechanism);
					PrintAvailableLearningMechanisms();
					exit(EXIT_FAILURE);
					break;
				}
			}
		}

		/*
		 * PrintAvailableLearningMechanisms(): prints the available ML mechanisms types
		 */
		void PrintAvailableLearningMechanisms(){
			printf("%s Available types of learning mechanisms:\n", LOG_LVL2);
			printf("%s MULTI_ARMED_BANDITS (%d)\n", LOG_LVL3, MULTI_ARMED_BANDITS);
			printf("%s GRAPH_COLORING (%d)\n", LOG_LVL3, GRAPH_COLORING);
			printf("%s ACTION_BANNING (%d)\n", LOG_LVL3, ACTION_BANNING);
		}

};

#endif
