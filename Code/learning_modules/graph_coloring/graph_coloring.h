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
//#include "action_selection_strategies/thompson_sampling.h"

#ifndef _AUX_GRAPH_COLORING_
#define _AUX_GRAPH_COLORING_

class GraphColoring {

	// Public items
	public:

		int save_controller_logs;
		int print_controller_logs;
		int agents_number;
		int wlans_number;
		int num_channels;
		int total_nodes_number;

	// Private items
	private:

		// Generic variables to all the learning strategies
		int initial_reward;
		int num_iterations;

		// Variables used bby the graph coloring method
		double **rssi_table;
		double **weight_edges;

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
		 * 	- central_controller_logger: logger object to write logs
		 * 	- sim_time: current simulation time
		 * OUTPUT:
		 *  - EMPTY: updates the configuration struct
		 */
		void UpdateConfiguration(Configuration *configuration_array, Performance *performance_array,
			Logger &central_controller_logger, double sim_time) {

			// Update the weights of edges between WLANs
			UpdateWeightsWlans(performance_array);

			// Optimization phase
			GraphColoringOptimization(configuration_array);

			if(save_controller_logs) {
				fprintf(central_controller_logger.file, "%.15f;%s;CC;%s GraphColoring: "
					"New configurations provided\n", sim_time, LOG_C00, LOG_LVL2);
				for (int i = 0; i < agents_number; ++ i) {
					fprintf(central_controller_logger.file, "%.15f;%s;CC;%s Agent %d - Assigned channel %d\n",
					sim_time, LOG_C00, LOG_LVL3, i,	configuration_array[i].selected_primary_channel);
				}
			}

		}

		/*
		 * UpdateWeightsWlans():
		* INPUT:
		 * 	- configuration: current configuration report from the AP
		 * 	- central_controller_logger: logger object to write logs
		 * 	- sim_time: current simulation time
		 * OUTPUT:
		 *  - EMPTY: updates the configuration struct
		 */
		void UpdateWeightsWlans(Performance *performance_array) {

			// Update the RSSI table
			for (int i = 0; i < agents_number; ++ i) {
//				printf("rssi_table (agent %d): ", i);
				for (int j = 0; j < agents_number; ++ j) {
					rssi_table[i][j] = performance_array[i].rssi_list[j];
//					printf(" %f ", rssi_table[i][j]);
				}
//				printf("\n");
			}

			// Update the weights table (BY NOW, SET THE WEIGHT AS THE MAX RSSI)
			for (int i = 0; i < agents_number; ++ i) {
//				printf("weights_table (agent %d): ", i);
				for (int j = 0; j < agents_number; ++ j) {
					// BY NOW, SET THE WEIGHT AS THE MAX RSSI
					weight_edges[i][j] = rssi_table[i][j];
//					printf(" %f ", weight_edges[i][j]);
				}
//				printf("\n");
			}

		}

		/*
		 * FindBestChannel(): finds the best channel, according to the selected policy
		* INPUT:
		 * 	- wlan_ix: index of the WLAN that must change the channel
		 * 	- current_channel: current channel being used by WLAN wlan_ix
		 * OUTPUT:
		 *  - new_channel: new channel to be selected by WLAN wlan_ix
		 */
		int FindBestChannel(int wlan_ix, int current_channel) {

			int new_channel;
			int max_distance = 0;
			for (int i = 0; i < num_channels; ++i) {
				if (abs(current_channel - i) > max_distance) {
					max_distance = abs(current_channel - i);
					new_channel = i;
				}
			}

			return new_channel;

		}

		/*
		 * GraphColoringOptimization(): optimizes the current configuration, according to the weight of edges
		* INPUT:
		 * 	- configuration: current configuration report from the AP
		 * 	- central_controller_logger: logger object to write logs
		 * 	- sim_time: current simulation time
		 * OUTPUT:
		 *  - EMPTY: updates the configuration struct
		 */
		void GraphColoringOptimization(Configuration *configuration_array) {

			// For each AP "i", find the maximum weight to AP "j" sharing the same channel
			double max_weight;
			bool flag_change_channel;
			for (int i = 0; i < agents_number; ++ i) {
				max_weight = -100000;
				flag_change_channel = false;
//				printf("max weight (agent %d): ", i);
				for (int j = 0; j < agents_number; ++ j) {
					if (i != j && configuration_array[i].selected_primary_channel ==
						configuration_array[j].selected_primary_channel) {
						flag_change_channel = true;
						if (weight_edges[i][j] > max_weight) {
							max_weight = weight_edges[i][j];
						}
					}
//					printf("%f: ", max_weight);
				}
				if (flag_change_channel) {
					// Choose a new color for AP "i", so that H(c) = min H(c) for c = 1...k
					configuration_array[i].selected_primary_channel = FindBestChannel(i, configuration_array[i].selected_primary_channel);
				}
//				printf("NEW CHANNEL (agent %d) = %d\n", i, configuration_array[i].selected_primary_channel);
			}
		}

		/*
		 * GraphColoringInitialization(): initializes the centralized graph coloring method
		* INPUT:
		 * 	- configuration: current configuration report from the AP
		 * 	- central_controller_logger: logger object to write logs
		 * 	- sim_time: current simulation time
		 * OUTPUT:
		 *  - EMPTY: updates the configuration struct
		 */
		void GraphColoringInitialization(Configuration *configuration_array) {

			// Set the same primary to all the agents/WLANs
			for (int i = 0; i < agents_number; ++ i) {
				configuration_array[i].selected_primary_channel = 0;	// Primary
			}

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

			initial_reward = 0;
			num_iterations = 1;

			rssi_table = new double*[total_nodes_number];
			for (int i = 0 ; i < total_nodes_number ; ++ i) {
				rssi_table[i] = new double[total_nodes_number];
				for (int j = 0 ; j < total_nodes_number ; ++ j) {
					rssi_table[i][j] = 0;
				}
			}

			weight_edges = new double*[total_nodes_number];
			for (int i = 0 ; i < total_nodes_number ; ++ i) {
				weight_edges[i] = new double[total_nodes_number];
				for (int j = 0 ; j < total_nodes_number ; ++ j) {
					weight_edges[i][j] = 0;
				}
			}

		}

};

#endif
