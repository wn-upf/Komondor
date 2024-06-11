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
 * modulations_methods.h: this file contains functions related to the main Komondor's operation
 *
 * - This file contains the methods related to "modulations" operations
 */


#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../list_of_macros.h"

/**
* Select the proper MCS per each number of channels based on the power received from transmitter
* @param "mcs_response" [type int*]: array containing the MCS to be used for each number of channels
* @param "power_rx_interest" [type double]: power received of interest
*/
void SelectMCSResponse(int *mcs_response, double power_rx_interest) {

	double pw_rx_intereset_dbm (ConvertPower(PW_TO_DBM, power_rx_interest));

	for ( int ch_num_ix = 0; ch_num_ix < 4; ++ ch_num_ix ){	// For 1, 2, 4 and 8 channels

		//if(pw_rx_intereset_dbm < -82 +(ch_num_ix*3)){ mcs_response[ch_num_ix] = MODULATION_FORBIDDEN; }
		if(pw_rx_intereset_dbm < -82 +(ch_num_ix*3)){ mcs_response[ch_num_ix] = MODULATION_BPSK_1_2; }
		else if (pw_rx_intereset_dbm >= -82 + (ch_num_ix*3) && pw_rx_intereset_dbm < -79 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_BPSK_1_2;}
		else if (pw_rx_intereset_dbm >= -79 + (ch_num_ix*3) && pw_rx_intereset_dbm < -77 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_QPSK_1_2;}
		else if (pw_rx_intereset_dbm >= -77 + (ch_num_ix*3) && pw_rx_intereset_dbm < -74 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_QPSK_3_4;}
		else if (pw_rx_intereset_dbm >= -74 + (ch_num_ix*3) && pw_rx_intereset_dbm < -70 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_16QAM_1_2;}
		else if (pw_rx_intereset_dbm >= -70 + (ch_num_ix*3) && pw_rx_intereset_dbm < -66 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_16QAM_3_4;}
		else if (pw_rx_intereset_dbm >= -66 + (ch_num_ix*3) && pw_rx_intereset_dbm < -65 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_64QAM_2_3;}
		else if (pw_rx_intereset_dbm >= -65 + (ch_num_ix*3) && pw_rx_intereset_dbm < -64 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_64QAM_3_4;}
		else if (pw_rx_intereset_dbm >= -64 + (ch_num_ix*3) && pw_rx_intereset_dbm < -59 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_64QAM_5_6;}
		else if (pw_rx_intereset_dbm >= -59 + (ch_num_ix*3) && pw_rx_intereset_dbm < -57 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_256QAM_3_4;}
		else if (pw_rx_intereset_dbm >= -57 + (ch_num_ix*3) && pw_rx_intereset_dbm < -54 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_256QAM_5_6;}
		else if (pw_rx_intereset_dbm >= -54 + (ch_num_ix*3) && pw_rx_intereset_dbm < -52 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_1024QAM_3_4;}
		else { mcs_response[ch_num_ix] = MODULATION_1024QAM_5_6;}
	}
}

/**
* Compute the "Energy per bit to noise power spectral density ratio"(normalized SNR to compare the BER performance of a digital modulation scheme)
* @param "sinr" [type double]: SINR received (pW)
* @param "bit_rate" [type double]: bit rate (bps)
* @param "bandwidth" [type double]: channel bandwidth (Hz)
* @param "modulation_type" [type double]: number of alternative modulation symbols
* @return "Eb_to_N0" [type double]: Eb_to_N0 value in linear
*/
double ComputeEbToNoise(double sinr, double bit_rate, int bandwidth, int modulation_type){

	double Es_to_N0 (sinr * (bit_rate/bandwidth));
	//printf("Es_to_N0 = %f (%f) \n", Es_to_N0, ConvertPower(LINEAR_TO_DB, Es_to_N0));
	double Eb_to_N0 (Es_to_N0 * log2(modulation_type));
	//printf("Eb_to_N0 = %f (%f)\n", Eb_to_N0, ConvertPower(LINEAR_TO_DB, Eb_to_N0));

	return Eb_to_N0;

}
