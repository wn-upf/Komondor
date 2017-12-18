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
 * - This file contains the methods related to "time" operations
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../list_of_macros.h"

// Exponential redefinition
double	Random( double v=1.0)	{ return v*drand48();}
int	Random( int v)	{ return (int)(v*drand48()); }
double	Exponential(double mean){ return -mean*log(Random());}

/*
 * ComputeTxTime(): computes the transmission time (just link rate) according to the number of channels used and packet lenght
 **/
double ComputeTxTime(int total_bits, double data_rate, int pdf_tx_time){

	double tx_time;

	// TODO: hardcoding rate to match CTMN analysis (MU for one channel)
	// data_rate = pow(10,6);

	switch(pdf_tx_time){

		case PDF_DETERMINISTIC:{

			tx_time = total_bits/data_rate;
			break;
		}

		case PDF_EXPONENTIAL:{

			tx_time = Exponential(total_bits/data_rate);
			break;
		}

		default:{

			printf("TX time model not found!\n");
			exit(EXIT_FAILURE);
		}
	}

	// TODO: THIS IS HARDCODED
	// tx_time = 1000 * MICRO_VALUE;
	// END HARCODED

	return tx_time;
}

/*
 * computeRtsTxTimeIeee80211ax: computes RTS transmission time
 **/
double computeRtsTxTime80211ax(double data_rate_20mhz){

	double rts_duration = IEEE_AX_LEGACY_PHYH_DURATION + ceil((IEEE_AX_SF_LENGTH + IEEE_AX_RTS_LENGTH
			+ IEEE_AX_TAIL_LENGTH) / data_rate_20mhz) * IEEE_AX_OFDM_SYMBOL_DURATION;

	return rts_duration;
}

/*
 * computeCtsTxTimeIeee80211ax: computes CTS transmission time
 **/
double computeCtsTxTime80211ax(double data_rate_20mhz){

	double cts_duration = IEEE_AX_LEGACY_PHYH_DURATION + ceil((IEEE_AX_SF_LENGTH + IEEE_AX_CTS_LENGTH
			+ IEEE_AX_TAIL_LENGTH) / data_rate_20mhz) * IEEE_AX_OFDM_SYMBOL_DURATION;

	return cts_duration;

}

/*
 * computeDataTxTimeIeee80211ax: computes data transmission time
 **/
double computeDataTxTime80211ax(int num_packets_aggregated, int data_packet_length, double data_rate){

	double data_duration = IEEE_AX_LEGACY_PHYH_DURATION + ceil( (IEEE_AX_SF_LENGTH + num_packets_aggregated * (IEEE_AX_DEL_LENGTH +
				IEEE_AX_MACH_LENGTH + data_packet_length) + IEEE_AX_TAIL_LENGTH) / data_rate) *
				IEEE_AX_OFDM_SYMBOL_DURATION + IEEE_AX_HE_PHYH_DURATION;

	return data_duration;
}

/*
 * computeAckTxTimeIeee80211ax: computes ACK transmission time
 **/
double computeAckTxTime80211ax(double data_rate_20mhz){

	double ack_duration = IEEE_AX_LEGACY_PHYH_DURATION + ceil((IEEE_AX_SF_LENGTH + IEEE_AX_BACK_LENGTH
			+ IEEE_AX_TAIL_LENGTH) / data_rate_20mhz) * IEEE_AX_OFDM_SYMBOL_DURATION;

	return ack_duration;

}

/*
 * ComputeNavTime: computes the NAV time for the RTS and CTS packets.
 **/
double ComputeNavTime(int node_state, double rts_duration, double cts_duration, double data_duration,
		double ack_duration, double sifs){

	double nav_time;

	switch(node_state){

		case STATE_TX_RTS:{

			// RTS duration taking into account due to trigger is set at the very begining of the RTS reception
			nav_time = 3 * sifs + rts_duration + cts_duration + data_duration + ack_duration;
			break;
		}

		case STATE_TX_CTS:{

			// CTS duration taking into account due to trigger is set at the very begining of the CTS reception
			nav_time = 2 * sifs + cts_duration + data_duration + ack_duration;
			break;
		}

		default:{

			printf("ERROR: Unreachable state\n");
			exit(EXIT_FAILURE);
		}
	}

	return nav_time;
}
