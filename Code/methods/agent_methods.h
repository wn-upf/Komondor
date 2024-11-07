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
 * agents_methods.h: this file contains functions related to the agents' operation
 *
 * - This file contains the methods employed by intelligent agents
 */

#include "../list_of_macros.h"

#ifndef _AUX_AGENT_METHODS_
#define _AUX_AGENT_METHODS_

/**
* Restarts the performance metrics being tracked by a given AP-agent pair
* @param "current_performance" [type Performance]: performance obtained by the corresponding WLAN
* @param "sim_time" [type double]: simulation time at the moment of calling the function (for logging purposes)
*/
void RestartPerformanceMetrics(Performance *current_performance, double sim_time, int num_channels_allowed) {

	current_performance->timestamp = sim_time;
	current_performance->throughput = 0;
	//current_performance->max_bound_throughput = 0;
	current_performance->sum_delays = 0;
	current_performance->num_delay_measurements = 0;
	current_performance->sum_waiting_time = 0;
	current_performance->num_waiting_time_measurements = 0;
	current_performance->average_delay = 0;
	current_performance->max_delay = 0;
	current_performance->min_delay = 10000;
	current_performance->max_waiting_time = 0;
	current_performance->min_waiting_time = 10000;
	current_performance->data_packets_sent = 0;
	current_performance->data_packets_lost = 0;
	current_performance->rts_cts_sent = 0;
	current_performance->rts_cts_lost = 0;
	current_performance->num_packets_generated = 0;
	current_performance->num_packets_dropped = 0;

	for(int n = 0; n < num_channels_allowed; ++n){
		current_performance->total_time_transmitting_in_num_channels[n] = 0;
		current_performance->total_time_lost_in_num_channels[n] = 0;
	}

}

/**
 * Check the validity of the current information held by the agent, based on the timestamp and the maximum expiration time
 * @return "data_still_valid" [type int]: boolean indicating whether data is still valid or not
 */
bool CheckValidityOfData(Configuration configuration, Performance performance,
		double sim_time, double max_time_validity_information) {

//	printf("sim_time = %f / max_time_validity_information = %f\n",sim_time,max_time_validity_information);

	bool data_still_valid(false);
	if ( (sim_time - performance.timestamp > max_time_validity_information)
			|| (sim_time - configuration.timestamp > max_time_validity_information) ) {
		data_still_valid = false;
	} else {
		data_still_valid = true;
	}
	return data_still_valid;
}

#endif
