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
 * controller_report.h: this file defines the report maintained by the CC
 */

#include "../list_of_macros.h"
#include "logger.h"

#ifndef _AUX_CC_REPORT_
#define _AUX_CC_REPORT_

// Action info
struct ControllerReport
{

	// Generic information
	int cc_iteration;
	int agents_number;
	int max_number_of_actions;
	// Clusters information
	int **clusters_per_wlan;
	double *cluster_performance;
	// Last configuration and performance received from each agent
	Configuration *last_configuration_array;	///> Array of configuration objects from all the agents
	Performance *last_performance_array;		///> Array of performance objects from all the agents
	// Agents' activity
	double *performance_per_agent;				///> Last performance experienced by each agent
	double *average_performance_per_agent;		///> Average performance experienced by each agent
	int *num_arms_per_agent;
	int **list_of_available_actions_per_agent; 	///> Matrix containing the list of available actions in each agent
	double **performance_action_per_agent;		///> Matrix containing the performance obtained by each action in each agent
	int **times_action_played_per_agent;		///> Matrix containing the times each action has been played by each agent
	int *most_played_action_per_agent;			///> Array containing the most played action by each agent

	/**
	* Print or write the list of banned actions
	* @param "write_or_print" [type int]: variable to indicate whether to print on the  console or to write on the the output logs file
	* @param "logger" [type Logger]: logger object to write on the output file
	* @param "save_logs" [type int]: boolean indicating whether to save logs or not
	* @param "sim_time" [type double]: simulation time
	*/
	void PrintOrWriteAvailableActions(int write_or_print, Logger &logger, int save_logs, double sim_time) {
		switch(write_or_print){
			// Print logs in console
			case PRINT_LOG:{
				printf("+ List of available actions (max = %d):\n", max_number_of_actions);
				for(int i = 0; i < agents_number; ++i) {
					printf("	 * Agent %d: ", i);
					for(int j = 0; j < max_number_of_actions; ++j) {
						printf(" %d ", list_of_available_actions_per_agent[i][j]);
					}
					printf("\n");
				}
				break;
			}
			// Write logs in agent's output file
			case WRITE_LOG:{
				LOGS(save_logs, logger.file, "%.15f;CC;%s;%s List of available actions:\n", sim_time, LOG_C00, LOG_LVL2);
				for(int i = 0; i < agents_number; ++i) {
					LOGS(save_logs, logger.file, "%.15f;CC;%s;%s Agent %d: ", sim_time, LOG_C00, LOG_LVL3, i);
					for(int j = 0; j < max_number_of_actions; ++j) {
						LOGS(save_logs, logger.file, " %d ", list_of_available_actions_per_agent[i][j]);
						printf(" %d ", list_of_available_actions_per_agent[i][j]);
					}
					LOGS(save_logs, logger.file, "\n");
				}
				break;
			}
		}
	}

	/**
	 * Set the size of the array timestamp_frames_aggregated
	 */
	void SetSizeOfArrays(){
		// Configuration & performance objects
		last_configuration_array = new Configuration[agents_number];
		last_performance_array = new Performance[agents_number];
		// Keep track of actions and performance statistics per agent
		performance_per_agent = new double[agents_number];
		average_performance_per_agent = new double[agents_number];
		performance_action_per_agent = new double *[agents_number];
		list_of_available_actions_per_agent = new int *[agents_number];
		num_arms_per_agent = new int[agents_number];
		times_action_played_per_agent = new int *[agents_number];
		most_played_action_per_agent = new int[agents_number];
		// Clusters information
		clusters_per_wlan = new int *[agents_number];
		cluster_performance = new double[agents_number];
		// Set dimensions of 2D arrays
		for (int i = 0; i < agents_number; ++i) {
			list_of_available_actions_per_agent[i] = new int[max_number_of_actions];
			performance_action_per_agent[i] = new double[max_number_of_actions];
			times_action_played_per_agent[i] = new int[max_number_of_actions];
			clusters_per_wlan[i] = new int[agents_number];
		}
	}

};

#endif
