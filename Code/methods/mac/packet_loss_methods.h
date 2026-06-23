/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
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
 * packet_loss_methods.h: this file contains functions related to the main Komondor's operation
 *
 * - This file contains the methods related to packet loss detection and handling
 */

#include <math.h>
#include <stddef.h>
#include "../../list_of_macros.h"

/**
* Handles a packet loss
* @param "type" [type int]: type of packet loss
* @param "total_time_lost_in_num_channels" [type double*]: array containing the total time lost for each number of channels used (to be updated by this method)
* @param "total_time_lost_per_channel" [type double*]: array containing the total time lost in each channels used (to be updated by this method)
* @param "packets_lost" [type int]: total number of packets lost (to be updated by this method)
* @param "rts_cts_lost" [type int]: total number of RTS/CTS packets lost (to be updated by this method)
* @param "packets_lost_per_sta" [type int**]: list of the total number of packets lost per STA (to be updated by this method)
* @param "rts_cts_lost_per_sta" [type int**]: list of the total number of RTS/CTS packets lost per STA (to be updated by this method)
* @param "current_right_channel" [type int]: current right channel
* @param "current_left_channel" [type int]: current left channel
* @param "current_tx_duration" [type double]: current transmission duration
* @param "node_id" [type int]: node id
* @param "destination_id" [type int]: destination id
*/
void HandlePacketLoss(int type, double *total_time_lost_in_num_channels, double *total_time_lost_per_channel,
		int &packets_lost, int &rts_cts_lost, int **packets_lost_per_sta, int **rts_cts_lost_per_sta,
		int current_right_channel, int current_left_channel, double current_tx_duration, int node_id, int destination_id){

//	printf("destination_id = %d\n", destination_id);
//	printf("node_id = %d\n", node_id);
//	printf("rts_cts_lost_per_sta = %d\n", (*rts_cts_lost_per_sta)[destination_id-node_id-1]);

	if(type == PACKET_TYPE_DATA) {
		for(int c = current_left_channel; c <= current_right_channel; c++){
			total_time_lost_per_channel[c] += current_tx_duration;
		}
		total_time_lost_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
		++packets_lost;
		++(*packets_lost_per_sta)[destination_id-node_id-1];
	} else if(type == PACKET_TYPE_CTS){
		++rts_cts_lost;
		++(*rts_cts_lost_per_sta)[destination_id-node_id-1];
	}

}

/**
* Attempt to decode incoming packet according to SINR and the capture effect (CE)
* @param "sinr" [type double]: SINR in pW
* @param "capture_effect" [type double]: capture effect threshold in pW
* @param "pd" [type double]: packet detect (PD) threshold in pW
* @param "power_rx_interest" [type double]: power received of interest in pW
* @param "constant_per" [type int]: constant packet error rate (PER)
* @param "node_id" [type int]: node id
* @param "packet_type" [type int]: type of packet being decoded
* @param "destination_id" [type int]: destination id
* @return "packet_lost" [type int]: boolean indicating whether the packet can be decoded or not
*/
int AttemptToDecodePacket(double sinr, double capture_effect, double pd,
		double power_rx_interest, double constant_per, int node_id, int packet_type,
		int destination_id){

	int packet_lost;
	double per (0);

	// Try to decode when power received is greater than the packet detect (pd) threshold
	if(sinr < capture_effect) { //  || power_rx_interest < pd) {
		per = 1;
	} else {
		// Just apply PER to DATA packets
		if( (destination_id == node_id) && (packet_type == PACKET_TYPE_DATA) ){
			per = constant_per;
		}
	}

	packet_lost = ((double) rand() / (RAND_MAX)) < per;

	return packet_lost;
}

/**
* Compute notification loss according to SINR received and other parameters
* @param "primary_channel" [type int]: primary channel
* @param "incoming_notification" [type Notification]: notification that was being decoded after detecting the newest one
* @param "new_notification" [type Notification]: new detected notification
* @param "sinr" [type double]: SINR in pW
* @param "capture_effect" [type double]: capture effect threshold in pW
* @param "pd" [type double]: packet detect (PD) threshold in pW
* @param "power_rx_interest" [type double]: power received of interest in pW
* @param "constant_per" [type int]: constant packet error rate (PER)
* @param "node_id" [type int]: node id
* @param "capture_effect_model" [type int]: capture effect model
* @return "loss_reason" [type int]: loss reason
*/
int IsPacketLost(int primary_channel, Notification incoming_notification, Notification new_notification,
		double sinr, double capture_effect, double pd, double power_rx_interest, double constant_per,
		int node_id, int capture_effect_model){

	int loss_reason (PACKET_NOT_LOST);
	int is_packet_lost;	// Determines if the current notification has been lost (1) or not (0)

	switch(capture_effect_model) {

		case CE_DEFAULT: {

			// Sergio on 25 Oct 2017:
			// - Change the way packets are determined are lost
			// - Use both incoming (interest) and new (sometimes noisy) notifications
			// - We were missing some cases. E.g. when RX_DATA and new packet arrived

			// Check if incoming notification (of interest) involves the primary channel
			if(primary_channel >= incoming_notification.left_channel && primary_channel <= incoming_notification.right_channel){

				// Attempt to decode (or continue decoding) the notification of interest
				is_packet_lost = AttemptToDecodePacket(sinr, capture_effect, pd, power_rx_interest, constant_per, node_id,
					new_notification.packet_type, new_notification.destination_id);

				if (is_packet_lost) {	// Incoming packet is lost
					if (power_rx_interest < pd) {	// Signal strength is not enough (< pd) to be decoded
						loss_reason = PACKET_LOST_LOW_SIGNAL;
//						hidden_nodes_list[new_notification.source_id] = TRUE;
					} else if (sinr < capture_effect){	// Capture effect not accomplished
						loss_reason = PACKET_LOST_INTERFERENCE;
					} else {	// Incoming packet lost due to PER
						loss_reason = PACKET_LOST_SINR_PROB;
					}
				}

			} else{

				loss_reason = PACKET_LOST_OUTSIDE_CH_RANGE;

			}

			break;
		}

		case CE_IEEE_802_11: {

			// Check if the RSSI is higher than the CST
			if (power_rx_interest > pd) {
				// The packet can be properly decoded
				loss_reason = -1;
			} else {
				loss_reason = PACKET_LOST_LOW_SIGNAL;
			}

			break;
		}

	}

	return loss_reason;

}
