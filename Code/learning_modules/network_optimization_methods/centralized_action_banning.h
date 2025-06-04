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

		int save_logs;				///> Boolean for saving logs
		int print_logs;				///> Boolean for printing logs

		double *initial_banning_threshold;
		double *current_banning_threshold;
		double MAX_THRESHOLD_BANNING;
		double MIN_THRESHOLD_BANNING;
		double MARGIN_THRESHOLD_BANNING;
        int banning_threshold_type;
        int banning_iteration;

        int banned_in_last_iteration;

        double *max_reward_seen_per_agent;

		int agents_number;
		int max_number_of_actions;

		int **list_of_available_actions_per_agent;
		int *num_arms_per_agent;
		double *average_performance_per_agent;
		double *cluster_performance;
		int **clusters_per_wlan;
		int *most_played_action_per_agent;
		int **times_action_played_per_agent;
		Configuration *configuration_array;

		// Variables for restoring banned actions (rollback)
		double *previous_performance_per_agent;        // Store the performance of each agent before a banning iteration
		int *previously_banned_action_per_agent;    // Index of the action banned by each agent in the previous banning iteration
		double DELTA;                               // Allowed error to assess that performance is worse

	// Methods
	public:

		/******************/
		/******************/
		/*  MAIN METHODS  */
		/******************/
		/******************/

		/**
		* Method for banning actions (configurations) based on different criteria. Edits the "configuration_array".
		* @param "controller_report" [type ControllerReport]: report with the statistics gathered by the controller
		* @param "central_controller_logger" [type Logger]: logger object to write logs
		* @param "sim_time" [type double]: simulation time
		*/
		void UpdateConfiguration(Configuration *configuration_array, ControllerReport &controller_report,
			Logger central_controller_logger, double sim_time) {

			// Update variables based on the controller's report
			UpdateVariables(controller_report);

			// Restore previously banned actions
			RestoreBannedActions(configuration_array, controller_report, central_controller_logger, sim_time);

			// Ban actions
			BanActions(configuration_array, controller_report, central_controller_logger, sim_time);

			// Update the action-banning threshold
			UpdateBanningThreshold(controller_report, central_controller_logger, sim_time);

		}

		/**
		* Method for banning actions (configurations) based on different criteria. Edits the "configuration_array".
		* @param "controller_report" [type ControllerReport]: report with the statistics gathered by the controller
		* @param "central_controller_logger" [type Logger]: logger object to write logs
		* @param "sim_time" [type double]: simulation time
		*/
		void BanActions(Configuration *configuration_array, ControllerReport &controller_report,
			Logger central_controller_logger, double sim_time) {

			// Check which agents have been the affected - they obtained less than a minimum amount of resources
			for(int i = 0; i < agents_number; ++i) {
                // Check the performance of each other agent in the same cluster (i.e., affectation to the environment)
				for(int j = 0; j < agents_number; ++j) {
					if(i != j && clusters_per_wlan[i][j] == 1) {
//					    if (average_performance_per_agent[j] + MARGIN_THRESHOLD_BANNING < current_banning_threshold[i]) {
                        if (average_performance_per_agent[j] < current_banning_threshold[i]) {
							// Assess whether the agent in the cluster affected negatively to the others
							if(AssessActionNegativeImpact(i)) {
                                LOGS(save_logs, central_controller_logger.file, "%.15f;CC;%s;%s Banned action %d of A%d\n",
                                     sim_time, LOG_C00, LOG_LVL1, most_played_action_per_agent[i], i);
								list_of_available_actions_per_agent[i][most_played_action_per_agent[i]] = 0;
								configuration_array[i].agent_capabilities.available_actions[most_played_action_per_agent[i]] = 0;
                                previously_banned_action_per_agent[i] = most_played_action_per_agent[i];
							}
						}
					}
					// Update the previous performance as the one experienced during this CC iteration
					previous_performance_per_agent[j] = average_performance_per_agent[j];
				}
			}

			++ banning_iteration;

		}

        /**
        * Method for restoring banned actions (configurations).
        * @param "controller_report" [type ControllerReport]: report with the statistics gathered by the controller
        * @param "central_controller_logger" [type Logger]: logger object to write logs
        * @param "sim_time" [type double]: simulation time
        */
		void RestoreBannedActions(Configuration *configuration_array, ControllerReport &controller_report,
            Logger central_controller_logger, double sim_time) {
            // Check which agents have been affected - i.e., they obtained a lower performance than before banning
            for(int i = 0; i < agents_number; ++i) {
//                if (average_performance_per_agent[i] < (previous_performance_per_agent[i] - DELTA)) {
                if (average_performance_per_agent[i] <= previous_performance_per_agent[i]) {
                    // For each other agent in the same cluster (i.e., the environment), check which actions were previously banned
                    for(int j = 0; j < agents_number; ++j) {
                        if(i != j && clusters_per_wlan[i][j] == 1 && previously_banned_action_per_agent[j] >= 0) {
                            LOGS(save_logs, central_controller_logger.file, "%.15f;CC;%s;%s Restored action %d of A%d\n",
                                 sim_time, LOG_C00, LOG_LVL1, previously_banned_action_per_agent[j], j);
                            list_of_available_actions_per_agent[j][previously_banned_action_per_agent[j]] = 1;
                            configuration_array[j].agent_capabilities.available_actions[previously_banned_action_per_agent[j]] = 1;
                            // Reset the banning threshold for the BSSs that caused action-banning in node "j"
                            ResetBanningThreshold(i);
                        }
                    }
                }
                // Reset this variable
                previously_banned_action_per_agent[i] = -1;
            }
		}

		/**
		* Method for assessing the negative impact of a given action to the environment
		* @param "agent_id" [type int]: identifier of the agent being evaluated
		*/
		int AssessActionNegativeImpact(int agent_id) {

		    // Count the total number of actions played during the last CC iteration
		    int total_num_arms_played(0);
		    for (int i = 0; i < num_arms_per_agent[agent_id]; ++i) {
                total_num_arms_played += times_action_played_per_agent[agent_id][i];
		    }

			// Define the minimum number of times an action has to be played before banning
			double POPULARITY_PERCENTAGE(0.33);
			int min_num_times_action_is_played = (POPULARITY_PERCENTAGE * total_num_arms_played);

			// Count the number of actions available in every agent (to prevent deleting all the actions)
			int sum_available_actions(0);
			// BAN the most popular action(s) if played a minimum number of times
            int  times_played_most_popular_action = times_action_played_per_agent[agent_id][most_played_action_per_agent[agent_id]];
			if (sum_available_actions > 1 && times_played_most_popular_action >= min_num_times_action_is_played) {
				banned_in_last_iteration = TRUE;
				return TRUE;

			} else {
				banned_in_last_iteration = FALSE;
				return FALSE;
			}

		}

		/**
		* Method for updating variables provided by the central controller
		* @param "controller_report" [type ControllerReport]: report with the statistics gathered by the controller
		*/
		void UpdateVariables(ControllerReport controller_report) {

			// Update information related to actions
			num_arms_per_agent = controller_report.num_arms_per_agent;
			list_of_available_actions_per_agent = controller_report.list_of_available_actions_per_agent;
			most_played_action_per_agent = controller_report.most_played_action_per_agent;
			times_action_played_per_agent = controller_report.times_action_played_per_agent;

			// Update information related to performance
			configuration_array = controller_report.last_configuration_array;
			average_performance_per_agent = controller_report.average_performance_per_agent;

            // Detect changes in clusters
            int change_in_cluster_detected(FALSE);
            for (int i = 0; i < agents_number; ++i) {
            	if (average_performance_per_agent[i] > max_reward_seen_per_agent[i]) {
            		max_reward_seen_per_agent[i] = average_performance_per_agent[i];
            	}

                for (int j = 0; j < agents_number; ++j) {
                    if (clusters_per_wlan[i][j] != controller_report.clusters_per_wlan[i][j]) {
                        change_in_cluster_detected = TRUE;
                        break;
                    }
                }
            }

            // Update clusters information
			clusters_per_wlan = controller_report.clusters_per_wlan;
			cluster_performance = controller_report.cluster_performance;

			// (Re)Compute tha banning threshold
			if (change_in_cluster_detected) {
                // Compute the initial banning threshold for each BSS, based on the number of agents in its cluster
                ComputeBanningThreshold();
			}

		}

		/**************************/
		/**************************/
		/*  BANNING THR. METHODS  */
		/**************************/
		/**************************/

        /**
        * Method for computing the banning threshold based on the number of agents in each cluster
        * @param "controller_report" [type ControllerReport]: report with the statistics gathered by the controller
        */
        void ComputeBanningThreshold() {
            for(int i = 0; i < agents_number; ++i) {
                int agents_in_cluster(0);
                for(int j = 0; j < agents_number; ++j) {
                    if(clusters_per_wlan[i][j] == 1) {
                        agents_in_cluster += 1;
                    }
                }
                initial_banning_threshold[i] = (1.0 / agents_in_cluster);
                current_banning_threshold[i] = initial_banning_threshold[i];
            }
        }

        /**
        * Method for reseting the banning threshold in case of banning wrongly
        * @param "agent_id" [type int]: identifier of the agent that needs to restart the banning threshold
        */
        void ResetBanningThreshold(int agent_id) {
        	current_banning_threshold[agent_id] = MIN_THRESHOLD_BANNING;
            banning_iteration = 1;
        }

        /**
        * Method for updating the banning threshold (dynamic approach)
        * @param "controller_report" [type ControllerReport]: report with the statistics gathered by the controller
        * @param "central_controller_logger" [type Logger]: logger object to write logs
        * @param "sim_time" [type double]: simulation time
        * @param "agent_id" [type int]: identifier of the agent that needs to restart the banning threshold
        */
        void UpdateBanningThreshold(ControllerReport controller_report,
			Logger central_controller_logger, double sim_time) {

            // Update the banning threshold (if necessary)
			if(controller_report.cc_iteration > 0) {
				for(int i = 0; i < agents_number; ++i) {
					// Check if the threshold needs to be updated or not and whether an increase or a decrease is required
					for(int j = 0; j < agents_number; ++j) {
						if(i != j && clusters_per_wlan[i][j] == 1) {
							double threshold_shift (ComputeBanningShift());
							if(average_performance_per_agent[j] > current_banning_threshold[i])
								current_banning_threshold[i] += threshold_shift;
							else if(max_reward_seen_per_agent[j] < current_banning_threshold[i])
								current_banning_threshold[i] -= threshold_shift;
							// Security check
							if (current_banning_threshold[i] > MAX_THRESHOLD_BANNING) {
								current_banning_threshold[i] = MAX_THRESHOLD_BANNING;
							}
							if (current_banning_threshold[i] < MIN_THRESHOLD_BANNING)
								current_banning_threshold[i] = MIN_THRESHOLD_BANNING;
						}
						LOGS(save_logs, central_controller_logger.file, "%.15f;CC;%s;%s Updating the banning threshold of Agent %d (mode %d)... \n",
							 sim_time, LOG_C00, LOG_LVL1, i, banning_threshold_type);
						LOGS(save_logs, central_controller_logger.file, "%.15f;CC;%s;%s New threshold = %f\n",
							 sim_time, LOG_C00, LOG_LVL2, current_banning_threshold[i]);
					}
				}
			}

        }

        /**
		* Method for computing the banning threshold shift
		*/
        double ComputeBanningShift() {
        	double threshold_shift(0);
        	switch(banning_threshold_type) {
				case BANNING_THRESHOLD_STATIC : {
					// Do nothing, the threshold does not change
					break;
				}
				case BANNING_THRESHOLD_LINEAR : {
					threshold_shift = 0.05;
					break;
				}
				case BANNING_THRESHOLD_LOGARITHMIC : {
					threshold_shift = 1/(10*sqrt(banning_iteration));
					break;
				}
			}
        	return threshold_shift;
        }

		/*************************/
		/*************************/
		/*  PRINT/WRITE METHODS  */
		/*************************/
		/*************************/

        /**
        * Print or write the statistics of Centralized Action-Banning
        * @param "write_or_print" [type int]: variable to indicate whether to print on the  console or to write on the the output logs file
        * @param "logger" [type Logger]: logger object to write on the output file
        */
        void PrintOrWriteStatistics(int write_or_print, Logger &logger) {
            // Write or print according the input parameter "write_or_print"
            switch(write_or_print){
                // Print logs in console
                case PRINT_LOG:{
                    printf("%s Centralized action-banning statistics...\n", LOG_LVL1);
                    printf("%s Available actions (%d agents):\n", LOG_LVL2, agents_number);
                    for (int i = 0; i < agents_number; ++i) {
                        for (int j = 0; j < num_arms_per_agent[i]; ++j) {
                            printf("%d ", list_of_available_actions_per_agent[i][j]);
                        }
                        printf("\n");
                    }
                    break;
                }
                // Write logs in agent's output file
                case WRITE_LOG:{
                    fprintf(logger.file, "%s Centralized action-banning statistics...\n", LOG_LVL1);
                    fprintf(logger.file, "%s Available actions (%d agents):\n", LOG_LVL2, agents_number);
                    for (int i = 0; i < agents_number; ++i) {
                        for (int j = 0; j < num_arms_per_agent[i]; ++j) {
                            fprintf(logger.file, "%d ", list_of_available_actions_per_agent[i][j]);
                        }
                        fprintf(logger.file, "\n");
                    }
                    break;
                }
            }
        }

		/***********************/
		/***********************/
		/*  AUXILIARY METHODS  */
		/***********************/
		/***********************/

		/**
		 * Initialize the variables used by the centralized action banner
		 */
		void InitializeVariables(){

		    // Banning threshold
            initial_banning_threshold = new double[agents_number];
            current_banning_threshold = new double[agents_number];
            MAX_THRESHOLD_BANNING = 0.9;                        // Maximum banning threshold
            MIN_THRESHOLD_BANNING = 0.1;						// Minimum banning threshold
            MARGIN_THRESHOLD_BANNING = 0.1;                     // Margin to avoid a too strict banning procedure
            banning_threshold_type = BANNING_THRESHOLD_LOGARITHMIC;  // Type of adaptation of the banning threshold [TODO: generate file that stores algorithm-specific variables]
            banning_iteration = 1;

            banned_in_last_iteration = FALSE;

            // Restoring banned actions
            previous_performance_per_agent = new double[agents_number];
            previously_banned_action_per_agent = new int[agents_number];
            DELTA = 0.1;

            max_reward_seen_per_agent = new double[agents_number];

            // Information provided by the CC
			list_of_available_actions_per_agent = new int *[agents_number];
			num_arms_per_agent = new int[agents_number];
			average_performance_per_agent = new double[agents_number];
			cluster_performance = new double[agents_number];
			clusters_per_wlan = new int *[agents_number];
			most_played_action_per_agent = new int[agents_number];
			times_action_played_per_agent = new int *[agents_number];
			configuration_array = new Configuration[agents_number];

			for(int i = 0; i < agents_number; ++i){
                initial_banning_threshold[i] = 0;
                current_banning_threshold[i] = 0;
				list_of_available_actions_per_agent[i] = new int[max_number_of_actions];	// Set the initial reward
				num_arms_per_agent[i] = 0;
				average_performance_per_agent[i] = 0;
                previous_performance_per_agent[i] = 0;
                previously_banned_action_per_agent[i] = -1;
				cluster_performance[i] = 0;
				clusters_per_wlan[i] = new int[agents_number];
				most_played_action_per_agent[i] = 0;
				times_action_played_per_agent[i] = new int[max_number_of_actions];
				max_reward_seen_per_agent[i] = 0;
			}

		}

};

#endif
