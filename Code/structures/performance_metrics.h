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
 * File description: this is the main Komondor file
 *
 * - This file defines a NOTIFICATION and provides basic displaying methods
 */

#ifndef _AUX_PERFORMANCE_
#define _AUX_PERFORMANCE_

struct Performance
{

	double last_time_measured;

	double throughput;
	double max_bound_throughput;
	int data_packets_sent;
	int data_packets_lost;
	int rts_cts_packets_sent;
	int rts_cts_packets_lost;
	double num_packets_generated;
	double num_packets_dropped;

	// Function to print the node's report
	void PrintReport(void){

		printf("%s Report (last measurement in %f):\n", LOG_LVL4, last_time_measured);
		printf("%s throughput = %f\n", LOG_LVL5, throughput);
		printf("%s max_bound_throughput = %f\n", LOG_LVL5, max_bound_throughput);
		printf("%s data_packets_sent = %d\n", LOG_LVL5, data_packets_sent);
		printf("%s data_packets_lost = %d\n", LOG_LVL5, data_packets_lost);
		printf("%s rts_cts_packets_sent = %d\n", LOG_LVL5, rts_cts_packets_sent);
		printf("%s rts_cts_packets_lost = %d\n", LOG_LVL5, rts_cts_packets_lost);
		printf("%s num_packets_generated = %f\n", LOG_LVL5, num_packets_generated);
		printf("%s num_packets_dropped = %f\n", LOG_LVL5, num_packets_dropped);
		printf("\n");

	}

};

#endif
