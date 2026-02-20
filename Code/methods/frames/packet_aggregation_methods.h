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
 * time_methods.h: this file contains functions related to the main Komondor's operation
 *
 * - This file contains the methods related to "time" operations
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include "../../list_of_macros.h"

/**
* Compute the minimum number of packets to be transmitted within the maximum PPDU time (IEEE_AX_MAX_PPDU_DURATION)
* @param "num_packets_aggregated" [type int]: current number of aggregated packets
* @param "data_packet_length" [type int]: length of a data packet
* @param "bits_ofdm_sym" [type double]: bits of an OFDM symbol
* @return "limited_num_packets_aggregated" [type int]: limited number of packets aggregated
*/
int FindMaximumPacketsAggregated(int num_packets_aggregated, int data_packet_length, double bits_ofdm_sym){

	double data_duration;
	int limited_num_packets_aggregated (num_packets_aggregated);

	while (limited_num_packets_aggregated > 0) {

		data_duration = IEEE_AX_PHY_HE_SU_DURATION
			+ ceil( ( (double) IEEE_AX_SF_LENGTH + (double) limited_num_packets_aggregated
			* ( (double) IEEE_AX_MD_LENGTH + (double) IEEE_AX_MH_LENGTH + (double) data_packet_length ) )
			/ bits_ofdm_sym ) * IEEE_AX_OFDM_SYMBOL_GI32_DURATION;

		if(data_duration <= IEEE_AX_MAX_PPDU_DURATION) {
			break;
		} else {
			limited_num_packets_aggregated--;
		}
	}

	return limited_num_packets_aggregated;

}