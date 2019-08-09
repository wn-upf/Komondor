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

#include "../list_of_macros.h"

#ifndef _AUX_AGENT_METHODS_
#define _AUX_AGENT_METHODS_

/*
 * RestartPerformanceMetrics(): restarts the performance metrics being tracked by a given AP-agent pair
 **/
void RestartPerformanceMetrics(Performance *current_performance, double sim_time) {

	current_performance->last_time_measured = sim_time;
	current_performance->throughput = 0;
	current_performance->max_bound_throughput = 0;
	current_performance->data_packets_sent = 0;
	current_performance->data_packets_lost = 0;
	current_performance->rts_cts_sent = 0;
	current_performance->rts_cts_lost = 0;
	current_performance->num_packets_generated = 0;
	current_performance->num_packets_dropped = 0;

}

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
				reward = performance.throughput/performance.max_bound_throughput;
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

		default:{
			printf("ERROR: %d is not a correct type of performance indicator\n", type_of_reward);
			exit(EXIT_FAILURE);
			break;
		}

	}

	return reward;

}

#endif
