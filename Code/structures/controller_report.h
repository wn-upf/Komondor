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
	// Last configuration and performance received from each agent
	Configuration *last_configuration_array;	///> Array of configuration objects from all the agents
	Performance *last_performance_array;		///> Array of performance objects from all the agents

	// Clusters information
	int **clusters_per_wlan;
	double *cluster_performance;

	// Agents' activity
	double *performance_per_agent;				///> Last performance experienced by each agent
	double *average_performance_per_agent;		///> Average performance experienced by each agent

	int **list_of_available_actions_per_agent; 	///> Matrix containing the list of available actions in each agent
	double **performance_action_per_agent;		///> Matrix containing the performance obtained by each action in each agent
	int **times_action_played_per_agent;		///> Matrix containing the times each action has been played by each agent
	int *most_played_action_per_agent;			///> Array containing the most played action by each agent

	/**
	 * Print the configuration of the action
	 */
	void PrintReport(){
		printf("------------\n CC Report (%d):\n", id);
		printf(" * tx_power = %f dBm\n", ConvertPower(PW_TO_DBM, tx_power));
		printf("------------\n");
	}

	/**
	 * Set the size of the array timestamp_frames_aggregated
	 * @param "num_packets_aggregated" [type int]: number of packets aggregated
	 */
	void SetSizeOfArrays(int num_agents, int num_actions){

		last_configuration_array = new Configuration[num_agents];
		last_performance_array = new Performance[num_agents];

		performance_per_agent = new double[num_agents];
		average_performance_per_agent = new double[num_agents];

		list_of_available_actions_per_agent = new int[num_agents];
		performance_action_per_agent = new double[num_agents];
		times_action_played_per_agent = new int[num_agents];
		most_played_action_per_agent = new int[num_agents];

		for(int t = 0; t < num_packets_aggregated; ++t){
			timestamp_frames_aggregated[t] = 0;
		}
	}

};

#endif
