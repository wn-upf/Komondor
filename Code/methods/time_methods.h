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
#include "../list_of_macros.h"

// Exponential redefinition
double	Random( double v=1.0)	{ return v*drand48();}
int	Random( int v)	{ return (int)(v*drand48()); }
double	Exponential(double mean){ return -mean*log(Random());}

/**
* Compute the minimum number of packets to be transmitted within the maximum PPDU time (IEEE_AX_MAX_PPDU_DURATION)
* @param "num_packets_aggregated" [type int]: current number of aggregated packets
* @param "data_packet_length" [type int]: length of a data packet
* @param "bits_ofdm_sym" [type double]: bits of an OFDM symbol
* @return "limited_num_packets_aggregated" [type int]: limited number of packets aggregated
*/
int findMaximumPacketsAggregated(int num_packets_aggregated, int data_packet_length, double bits_ofdm_sym){

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

/**
* Compute the transmission time (just link rate) according to the number of channels used and packet length
* @param "total_bits" [type int]: total number bits to be transmitted
* @param "data_rate" [type int]: data rate employed
* @param "pdf_tx_time" [type int]: pdf type for computing the transmission type (PDF_DETERMINISTIC or PDF_EXPONENTIAL)
* @return "tx_time" [type double]: transmission time
*/
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

/**
* Compute the RTS transmission time (IEEE 802.11ax)
* @param "bits_ofdm_sym_legacy" [type double]: bits of a legacy OFDM symbol
* @return "rts_duration" [type double]: transmission time of an RTS packet
*/
double computeRtsTxTime80211ax(double bits_ofdm_sym_legacy){

//	double rts_duration (IEEE_AX_PHY_LEGACY_DURATION + ceil((double) (IEEE_AX_SF_LENGTH +
//		(double) IEEE_AX_RTS_LENGTH) / bits_ofdm_sym_legacy) * IEEE_AX_OFDM_SYMBOL_LEGACY);

	return (IEEE_AX_PHY_LEGACY_DURATION + ceil((double) (IEEE_AX_SF_LENGTH +
		(double) IEEE_AX_RTS_LENGTH) / bits_ofdm_sym_legacy) * IEEE_AX_OFDM_SYMBOL_LEGACY);

}

/**
* Compute the CTS transmission time (IEEE 802.11ax)
* @param "bits_ofdm_sym_legacy" [type double]: bits of a legacy OFDM symbol
* @return "cts_duration" [type double]: transmission time of an CTS packet
*/
double computeCtsTxTime80211ax(double bits_ofdm_sym_legacy){

//	double cts_duration = IEEE_AX_PHY_LEGACY_DURATION + ceil((double) (IEEE_AX_SF_LENGTH +
//			(double) IEEE_AX_CTS_LENGTH) / bits_ofdm_sym_legacy) * IEEE_AX_OFDM_SYMBOL_LEGACY;

//	printf("CTS = %f\n", cts_duration * pow(10,6));

	return (IEEE_AX_PHY_LEGACY_DURATION + ceil((double) (IEEE_AX_SF_LENGTH +
		(double) IEEE_AX_CTS_LENGTH) / bits_ofdm_sym_legacy) * IEEE_AX_OFDM_SYMBOL_LEGACY);

}

/**
* Compute the data transmission time (IEEE 802.11ax)
* @param "num_packets_aggregated" [type int]: number of packets aggregated
* @param "data_packet_length" [type int]: length of the data packet
* @param "bits_ofdm_sym" [type double]: bits of an OFDM symbol
* @return "data_duration" [type double]: transmission time of a data packet
*/
double computeDataTxTime80211ax(int num_packets_aggregated, int data_packet_length, double bits_ofdm_sym){

	double data_duration;

	if(num_packets_aggregated == 1){

		data_duration = IEEE_AX_PHY_HE_SU_DURATION
			+ ceil( ( (double) IEEE_AX_SF_LENGTH + (double) IEEE_AX_MH_LENGTH +
			(double) data_packet_length) / bits_ofdm_sym ) * IEEE_AX_OFDM_SYMBOL_GI32_DURATION;

	} else {

		data_duration = IEEE_AX_PHY_HE_SU_DURATION
			+ ceil( ( (double) IEEE_AX_SF_LENGTH + (double) num_packets_aggregated
			* ( (double) IEEE_AX_MD_LENGTH + (double) IEEE_AX_MH_LENGTH + (double) data_packet_length ) )
			/ bits_ofdm_sym ) * IEEE_AX_OFDM_SYMBOL_GI32_DURATION;

	}

//	printf("num_packets_aggregated = %d\n", num_packets_aggregated);
//	printf("data_packet_length = %d\n", data_packet_length);
//	printf("IEEE_AX_PHY_HE_SU_DURATION = %f\n", IEEE_AX_PHY_HE_SU_DURATION);
//	printf("bits_ofdsm_sym = %f\n", bits_ofdm_sym);
//	printf("IEEE_AX_OFDM_SYMBOL_GI32_DURATION = %f\n", IEEE_AX_OFDM_SYMBOL_GI32_DURATION);
//	printf("SIZE DATA = %d\n", num_packets_aggregated*
//			( IEEE_AX_MD_LENGTH + IEEE_AX_MH_LENGTH + data_packet_length));
//	printf("DATA DURATION = %f\n", data_duration * pow(10,6));

	return data_duration;

}

/**
* Compute the ACK (block ACK) transmission time (IEEE 802.11ax)
* @param "num_packets_aggregated" [type int]: number of packets aggregated
* @param "bits_ofdm_sym_legacy" [type double]: bits of a legacy OFDM symbol
* @return "ack_duration" [type double]: transmission time of an ACK packet
*/
double computeAckTxTime80211ax(int num_packets_aggregated, double bits_ofdm_sym_legacy){

	double ack_duration;

	if(num_packets_aggregated == 1){
		ack_duration = IEEE_AX_PHY_LEGACY_DURATION + ceil((double) (IEEE_AX_SF_LENGTH +
			(double) IEEE_AX_ACK_LENGTH) / bits_ofdm_sym_legacy) * IEEE_AX_OFDM_SYMBOL_LEGACY;

		ack_duration = 28 / pow(10,6);

	} else {
		ack_duration = IEEE_AX_PHY_LEGACY_DURATION + ceil((double) (IEEE_AX_SF_LENGTH +
			(double) IEEE_AX_BACK_LENGTH) / bits_ofdm_sym_legacy) * IEEE_AX_OFDM_SYMBOL_LEGACY;

		ack_duration = 32 / pow(10,6);

	}

	//printf("ACK = %f\n", ack_duration * pow(10,6));
	ack_duration = 32 / pow(10,6);
	return ack_duration;

}

/**
* Compute the NAV time for the RTS and CTS packets
* @param "node_state" [type int]: node state
* @param "rts_duration" [type double]: duration of the RTS packet
* @param "cts_duration" [type double]: duration of the CTS packet
* @param "data_duration" [type double]: duration of the DATA packet
* @param "ack_duration" [type double]: duration of the ACK packet
* @param "sifs" [type double]: duration of the SIFS
* @return "nav_time" [type double]: NAV time
*/
double ComputeNavTime(int node_state, double rts_duration, double cts_duration,
	double data_duration, double ack_duration, double sifs){

	double nav_time;

	switch(node_state){

		case STATE_TX_RTS:{
			// RTS duration taking into account due to trigger is set at the very beginning of the RTS reception
			nav_time = 3 * sifs + rts_duration + cts_duration + data_duration + ack_duration;
			break;
		}

		case STATE_TX_CTS:{
			// CTS duration taking into account due to trigger is set at the very beginning of the CTS reception
			nav_time = 2 * sifs + cts_duration + data_duration + ack_duration;
			break;
		}

		case STATE_TX_DATA:{
			// DATA duration taking into account due to trigger is set at the very beginning of the DATA reception
			nav_time = sifs + data_duration + ack_duration;
			break;
		}

		case STATE_TX_ACK:{
			// ACK duration taking into account due to trigger is set at the very beginning of the ACK reception
			nav_time = ack_duration;
			break;
		}

		default:{

			printf("ERROR: Unreachable state\n");
			exit(EXIT_FAILURE);
		}
	}

	return nav_time;

}

/**
* Compute the duration of a frame
* @param "rts_duration" [type double]: duration of the RTS packet (to be updated by this method)
* @param "cts_duration" [type double]: duration of the CTS packet (to be updated by this method)
* @param "data_duration" [type double]: duration of the DATA packet (to be updated by this method)
* @param "ack_duration" [type double]: duration of the ACK packet (to be updated by this method)
* @param "ieee_protocol" [type int]: IEEE protocol used
* @param "num_channels_tx" [type int]: number of channels used for transmission
* @param "current_modulation" [type int]: current MCS used
* @param "num_packets_aggregated" [type int]: number of packets aggregated
* @param "data_packet_length" [type int]: length of data packets
* @param "bits_ofdm_sym" [type int]: number of bits of an OFDM symbol
*/
void ComputeFramesDuration(double *rts_duration, double *cts_duration,
	double *data_duration, double *ack_duration, int num_channels_tx, int current_modulation,
	int num_packets_aggregated, int data_packet_length, int bits_ofdm_sym){

	// Compute the duration of each frame
	*rts_duration = computeRtsTxTime80211ax(IEEE_BITS_OFDM_SYM_LEGACY);
	*cts_duration = computeCtsTxTime80211ax(IEEE_BITS_OFDM_SYM_LEGACY);
	*data_duration = computeDataTxTime80211ax(num_packets_aggregated,
			data_packet_length, bits_ofdm_sym);
	*ack_duration = computeAckTxTime80211ax(num_packets_aggregated, IEEE_BITS_OFDM_SYM_LEGACY);

}
