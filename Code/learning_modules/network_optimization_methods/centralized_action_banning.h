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
 * centralized_action_banning.h: this file contains functions related to the agents' operation
 *
 *  - This file contains the methods used by the centralized action banner
 */

#include "../../list_of_macros.h"

#ifndef _AUX_BANNING_
#define _AUX_BANNING_

class CentralizedActionBanning {

	// Public items
	public:

		int agent_id;						///> Identified of the agent using MABs
		int save_logs;				///> Boolean for saving logs
		int print_logs;				///> Boolean for printing logs

		double THRESHOLD_BANNING;

		int agents_number;
		int max_number_of_actions;

		int **list_of_available_actions_per_agent;
		int *num_actions_per_agent;
//		int *num_available_actions_per_agent;
		double *average_performance_per_agent;
		double *cluster_performance;
		int **clusters_per_wlan;
		int *most_played_action_per_agent;
		int **times_action_played_per_agent;
		Configuration *configuration_array;

	// Methods
	public:

		/******************/
		/******************/
		/*  MAIN METHODS  */
		/******************/
		/******************/

		/**
		* Method for banning actions (configurations) based on different criteria. Fills "available_actions_per_agent".
		* @param "controller_report" [type ControllerReport]: report with the statistics gathered by the controller
		* @param "central_controller_logger" [type Logger]: logger object to write logs
		* @param "sim_time" [type double]: simulation time
		*/
		void BanActions(Configuration *configuration_array, ControllerReport &controller_report,
			Logger central_controller_logger, double sim_time) {

			// Update variables based on the controller's report
			UpdateVariables(controller_report);

			// Check which agents have been the affected - they obtained less than a minimum amount of resources
			for(int i = 0; i < agents_number; ++i) {
				if (average_performance_per_agent[i] < THRESHOLD_BANNING) {
					// For each other agent in the same cluster (i.e., the environment), check which actions were played
					for(int j = 0; j < agents_number; ++j) {
						if(i != j && clusters_per_wlan[i][j] == 1) {
							// Assess whether the agent in the cluster affected negatively to the others
							if(AssessActionNegativeImpact(j)) {
								printf("Banned action %d of A%d\n", most_played_action_per_agent[j], j);
								list_of_available_actions_per_agent[j][most_played_action_per_agent[j]] = 0;
								configuration_array[j].agent_capabilities.available_actions[most_played_action_per_agent[j]] = 0;
							}
						}
					}
				}
//				printf("Average performance (%d) = %f\n", i, average_performance_per_agent[i]);
//				for(int j = 0; j < agents_number; ++j) {
//					if(i != j && clusters_per_wlan[i][j] == 1) {
//						printf("Num times each action is played (A%d) (total = %d): ", j, num_actions_per_agent[j]);
//						for (int k = 0; k < num_actions_per_agent[j]; ++k) {
//							printf(" %d ", times_action_played_per_agent[j][k]);
//						}
//						printf("\n");
//					}
//				}
			}

		}

		/**
		* Method for assessing the negative impact of a given agent to the environment
		* @param "agent_id" [type int]: identifier of the agent being evaluated
		*/
		int AssessActionNegativeImpact(int agent_id) {

			// Check if the most popular action of the agent has been played a minimum number of times
			int min_num_times_action_is_played = num_actions_per_agent[agent_id]/3;
			int  times_played_most_popular_action = times_action_played_per_agent[agent_id][most_played_action_per_agent[agent_id]];
//			printf("min_num_times_action_is_played = %d (%d)\n", min_num_times_action_is_played, times_played_most_popular_action);
			// Count the number of actions available in every agent (to prevent deleting all the actions)
			int sum_available_actions(0);
			for (int i = 0; i < num_actions_per_agent[agent_id]; ++i) {
				if (list_of_available_actions_per_agent[agent_id] >= 0)
					sum_available_actions += list_of_available_actions_per_agent[agent_id][i];
			}
			// BAN the action
			if (sum_available_actions > 1 && times_played_most_popular_action >= min_num_times_action_is_played) {
				return TRUE;
			} else {
				return FALSE;
			}

		}

		/**
		* Method for updating variables provided by the central controller
		* @param "controller_report" [type ControllerReport]: report with the statistics gathered by the controller
		*/
		void UpdateVariables(ControllerReport controller_report) {

			num_actions_per_agent = controller_report.num_actions_per_agent;

			list_of_available_actions_per_agent = controller_report.list_of_available_actions_per_agent;
			average_performance_per_agent = controller_report.average_performance_per_agent;

			clusters_per_wlan = controller_report.clusters_per_wlan;
			cluster_performance = controller_report.cluster_performance;

			most_played_action_per_agent = controller_report.most_played_action_per_agent;
			times_action_played_per_agent = controller_report.times_action_played_per_agent;
			configuration_array = controller_report.last_configuration_array;

		}

		/*************************/
		/*************************/
		/*  PRINT/WRITE METHODS  */
		/*************************/
		/*************************/

		// TODO
		// ...

		/***********************/
		/***********************/
		/*  AUXILIARY METHODS  */
		/***********************/
		/***********************/

		/**
		 * Initialize the variables used by the centralized action banner
		 */
		void InitializeVariables(){

			THRESHOLD_BANNING = 0.9;	// Initial banning threshold [TODO: generate file that stores algorithm-specific variables]

			list_of_available_actions_per_agent = new int *[agents_number];
			num_actions_per_agent = new int[agents_number];
			average_performance_per_agent = new double[agents_number];
			cluster_performance = new double[agents_number];
			clusters_per_wlan = new int *[agents_number];
			most_played_action_per_agent = new int[agents_number];
			times_action_played_per_agent = new int *[agents_number];
			configuration_array = new Configuration[agents_number];

			for(int i = 0; i < agents_number; ++i){
				list_of_available_actions_per_agent[i] = new int[max_number_of_actions];	// Set the initial reward
				num_actions_per_agent[i] = 0;
				average_performance_per_agent[i] = 0;
				cluster_performance[i] = 0;
				clusters_per_wlan[i] = new int[agents_number];
				most_played_action_per_agent[i] = 0;
				times_action_played_per_agent[i] = new int[max_number_of_actions];
			}

		}

};

#endif
