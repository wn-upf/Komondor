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
 * ml_model.h: this file contains functions related to the agents' operation
 *
 *  - This file contains the methods used by the ML Model in the Machine Learning (ML) pipeline.
 * 	 In particular, this module manages the ML operation
 */

#include "../list_of_macros.h"

#include "../structures/node_configuration.h"
#include "../structures/performance_metrics.h"

#include "../network_optimization/channel_assignment/centralized_graph_coloring.h"
#include "../network_optimization/spatial_reuse/rtot_algorithm.h"

#include "/multi_armed_bandits/multi_armed_bandits.h"


#ifndef _AUX_ML_MODEL_
#define _AUX_ML_MODEL_

class MlModel {

	// Public items
	public:

		int learning_mechanism;			///> Index of the learning mechanism employed

		// Information of the network
		int agents_number;				///> Number of agents
		int wlans_number;				///> Number of WLANs
		int total_nodes_number;			///> Number of nodes

		// Parameters
		int num_channels;				///> Number of channels

		// Logs
		int save_controller_logs;		///> Flag to indicate whether to save the controller logs or not
		int print_controller_logs;		///> Flag to indicate whether to print the controller logs or not
		int save_agent_logs;			///> Flag to indicate whether to save the agents logs or not
		int print_agent_logs;			///> Flag to indicate whether to print the agents logs or not

		GraphColoring graph_coloring;	///> Graph coloring object

		int agent_id;					///> ID of the agent calling the ML method

		// Multi-armed bandits
		MultiArmedBandit mab_agent;		///> Multi-Armed Bandit object
		int action_selection_strategy;	///> Index of the chosen action-selection strategy
		int num_actions;				///> Number of actions (Bandits)

		// RTOT algorithm
		RtotAlgorithm rtot_alg;
		int num_stas;
		double margin;

		// Centralized action banning

	// Private items
	private:

	// Methods
	public:

		/********************************/
		/********************************/
		/*  CENTRAL CONTROLLER METHODS  */
		/********************************/
		/********************************/

		/**
		* Method for computing the global configuration
		* @param "configuration_array" [type Configuration*]: array of configurations of each AP (to be updated by this method)
		* @param "performance_array" [type Performance*]: array of performances of each AP
		* @param "central_controller_logger" [type Logger]: logger object to write logs
		* @param "sim_time" [type double]: simulation time at the moment of calling the function (for logging purposes)
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
				case RTOT_ALGORITHM: {
					break;
				}
				case CENTRALIZED_ACTION_BANNING: {
					break;
				}
				default: {
					printf("[ML MODEL] ERROR, UNKOWN LEARNING MECHANISM ('%d')\n", learning_mechanism);
					exit(-1);
				}
			}

		}

		/**
		* Method for computing an individual configuration (decentralized case)
		* @param "arm_ix" [type int]: index of the current selected arm (bandits)
		* @param "reward" [type Performance*]: array of performances of each AP
		* @param "agent_logger" [type Logger]: logger object to write logs
		* @param "sim_time" [type double]: simulation time at the moment of calling the function (for logging purposes)
		* @return "new_action" [type int]: index of the new selected action
		*/
		double ComputeIndividualConfiguration(int arm_ix, double reward, Logger &agent_logger, double sim_time) {
			double new_action(0);
			switch(learning_mechanism) {
				/* MULTI_ARMED_BANDITS */
				case MULTI_ARMED_BANDITS: {
					// Update the reward of the last played configuration
					mab_agent.UpdateArmStatistics(arm_ix, reward);
					// Select a new action according to the updated information
					new_action = (double) mab_agent.SelectNewAction();
					break;
				}
				case RTOT_ALGORITHM: {
					new_action = rtot_alg.UpdateObssPd(reward);
					break;
				}
				case CENTRALIZED_ACTION_BANNING: {
					break;
				}
				default: {
					printf("[ML MODEL] ERROR, UNKOWN LEARNING MECHANISM ('%d')\n", learning_mechanism);
					exit(-1);
				}
			}
			return new_action;
		};


		/*******************/
		/*******************/
		/*  OHTER METHODS  */
		/*******************/
		/*******************/

		/**
		* Initialize variables in the ML model
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
				case RTOT_ALGORITHM: {
					rtot_alg.num_stas = num_stas;
					rtot_alg.margin = margin;
					rtot_alg.InitializeVariables();
					break;
				}
				case CENTRALIZED_ACTION_BANNING: {
					printf("[ML Model] Centralized action banning\n");
					break;
				}
				//  TODO: provide more learning mechanisms
				// case Q_LEARNING:
				// ...

				/* UNKNOWN */
				default: {
					printf("[ML MODEL] ERROR: '%d' is not a correct learning mechanism\n", learning_mechanism);
					PrintAvailableLearningMechanisms();
					exit(EXIT_FAILURE);
					break;
				}
			}
		}

		/**
		* Print or write statistics of the ML Model
		* @param "write_or_print" [type int]: variable to indicate whether to print on the  console or to write on the the output logs file
		* @param "logger" [type Logger]: logger object to write logs
		* @param "sim_time" [type double]: simulation time at the moment of calling the function (for logging purposes)
		*/
		void PrintOrWriteStatistics(int write_or_print, Logger &logger, double sim_time) {

			switch(learning_mechanism) {
				/* GRAPH COLORING */
				case GRAPH_COLORING: {
					graph_coloring.PrintOrWriteStatistics(write_or_print, logger);
					break;
				}
				/* Multi-Armed Bandits */
				case MULTI_ARMED_BANDITS: {
					mab_agent.PrintOrWriteStatistics(write_or_print, logger, sim_time);
					break;
				}
				case RTOT_ALGORITHM: {
//					rtot_alg.PrintOrWriteInformation(write_or_print, logger);
//					rtot_alg.PrintOrWriteStatistics(write_or_print, logger);
					break;
				}
				case CENTRALIZED_ACTION_BANNING: {
					if (write_or_print == PRINT_LOG) {
//						printf("Available actions (%d agents with %d actions):\n", agents_number, num_actions);
//						for (int i = 0; i < agents_number; ++i) {
//							for (int j = 0; j < num_actions; ++j) {
//								printf("%d ", available_actions_per_agent[i][j]);
//							}
//							printf("\n");
//						}
					}
					break;
				}
				/* UNKNOWN */
				default: {
					printf("[ML MODEL] ERROR: '%d' is not a correct learning mechanism\n", learning_mechanism);
					PrintAvailableLearningMechanisms();
					exit(EXIT_FAILURE);
					break;
				}
			}

		}

		/**
		* Print the available ML mechanisms types
		*/
		void PrintAvailableLearningMechanisms(){
			printf("%s Available types of learning mechanisms:\n", LOG_LVL2);
			printf("%s MULTI_ARMED_BANDITS (#%d)\n", LOG_LVL3, MULTI_ARMED_BANDITS);
			printf("%s GRAPH_COLORING (#%d)\n", LOG_LVL3, GRAPH_COLORING);
			printf("%s ACTION_BANNING (#%d)\n", LOG_LVL3, ACTION_BANNING);
			printf("%s RTOT_ALGORITHM (#%d)\n", LOG_LVL3, RTOT_ALGORITHM);
		}

};

#endif
