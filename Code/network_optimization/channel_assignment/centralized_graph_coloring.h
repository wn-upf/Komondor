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
 * graph_coloring.h: this file contains functions related to the agents' operation
 *
 * - This file contains the methods used to provide centralized graph coloring (https://dl.acm.org/citation.cfm?id=997130)
 */

#include "../../list_of_macros.h"

#ifndef _AUX_GRAPH_COLORING_
#define _AUX_GRAPH_COLORING_

class GraphColoring {

	// Public items
	public:

		int save_controller_logs;	///> Boolean for saving logs
		int print_controller_logs;	///> Boolean for printing logs
		int agents_number;			///> Number of agents
		int wlans_number;			///> Number of WLANs
		int num_channels;			///> Number of channels
		int total_nodes_number;		///> Total number of nodes

	// Private items
	private:

		// Generic variables to all the learning strategies
		int initial_reward;	 ///> Initial reward value
		int num_iterations;	 ///> Number of iterations allowed

		// Variables used by the graph coloring method
		double **rssi_table;	///> Matrix of the RSSI received by each node from the others
		double **weight_edges;  ///> Matrix to store weight edges

	// Methods
	public:

		/******************/
		/******************/
		/*  MAIN METHODS  */
		/******************/
		/******************/

		/**
		* Main method for updating the configuration according to past experience
		* @param "configuration_array" [type Configuration*]: array of configurations of each AP (to be updated by this method)
		* @param "performance_array" [type Performance*]: array of performances of each AP
		* @param "central_controller_logger" [type Logger]: logger object to write logs
		* @param "sim_time" [type double]: simulation time at the moment of calling the function (for logging purposes)
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

		/**
		* Update both the RSSI Table and the Weight edges
		* @param "performance_array" [type Performance*]: array of performances of each AP
		*/
		void UpdateWeightsWlans(Performance *performance_array) {

			// Update the RSSI table
			for (int i = 0; i < agents_number; ++ i) {
				for (int j = 0; j < agents_number; ++ j) {
					rssi_table[i][j] = performance_array[i].rssi_list[j];
				}
			}

			// Update the weights table (BY NOW, SET THE WEIGHT AS THE MAX RSSI)
			for (int i = 0; i < agents_number; ++ i) {
				for (int j = 0; j < agents_number; ++ j) {
					// BY NOW, SET THE WEIGHT AS THE MAX RSSI
					weight_edges[i][j] = rssi_table[i][j];
				}
			}

		}

		/**
		* Find the best channel, according to the selected policy
		* @param "wlan_ix" [type int]: WLAN index
		* @param "current_channel" [type int]: current primary channel
		* @param "new_channel" [type int]: new primary channel
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

		/**
		* Updates the configuration to be used by each WLAN
		* @param "configuration_array" [type Configuration*]: array of configurations of each AP (to be updated by this method)
		*/
		void GraphColoringOptimization(Configuration *configuration_array) {

			// For each AP "i", find the maximum weight to AP "j" sharing the same channel
			double max_weight;
			bool flag_change_channel;
			for (int i = 0; i < agents_number; ++ i) {
				max_weight = -100000;
				flag_change_channel = false;
				for (int j = 0; j < agents_number; ++ j) {
					if (i != j && configuration_array[i].selected_primary_channel ==
						configuration_array[j].selected_primary_channel) {
						flag_change_channel = true;
						if (weight_edges[i][j] > max_weight) {
							max_weight = weight_edges[i][j];
						}
					}
				}
				if (flag_change_channel) {
					// Choose a new color for AP "i", so that H(c) = min H(c) for c = 1...k
					configuration_array[i].selected_primary_channel = FindBestChannel(i, configuration_array[i].selected_primary_channel);
				}
//				printf("NEW CHANNEL (agent %d) = %d\n", i, configuration_array[i].selected_primary_channel);
			}
		}

		/**
		* Provides the initial configuration to be used by each WLAN, according to the centralized graph coloring method
		* @param "configuration_array" [type Configuration*]: array of configurations of each AP (to be updated by this method)
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

		/**
		* Initialize the variables used by the Graph Coloring instance
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

		/**
		* Print or write the statistics of Graph Coloring
		* @param "write_or_print" [type int]: variable to indicate whether to print on the  console or to write on the the output logs file
		* @param "logger" [type Logger]: logger object to write on the output file
		*/
		void PrintOrWriteStatistics(int write_or_print, Logger &logger) {
			// Write or print according the input parameter "write_or_print"
			switch(write_or_print){
				// Print logs in console
				case PRINT_LOG:{
					printf("Graph coloring statistics...\n");
				}
				// Write logs in agent's output file
				case WRITE_LOG:{
					fprintf(logger.file, "Graph coloring statistics...\n");
				}
			}
		}


};

#endif
