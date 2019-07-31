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
 * - This file contains the methods related to "notifications" operations
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include "../list_of_macros.h"

/*
 * GenerateLogicalNack: generates a logical NACK
 **/
LogicalNack GenerateLogicalNack(int packet_type, int packet_id, int node_id,
		int node_id_a, int node_id_b, int loss_reason, double ber, double sinr){

	LogicalNack logical_nack;

	logical_nack.packet_type = packet_type;
	logical_nack.source_id = node_id;
	logical_nack.packet_id = packet_id;
	logical_nack.loss_reason = loss_reason;
	logical_nack.node_id_a = node_id_a;
	logical_nack.node_id_b = node_id_b;
	logical_nack.ber = ber;
	logical_nack.sinr = sinr;

	return logical_nack;
}

/*
 * ProcessNack(): processes a NACK notification.
 **/
int ProcessNack(LogicalNack logical_nack, int node_id, Logger node_logger, int node_state,
		int save_node_logs,	double sim_time, int *nacks_received,
		int total_nodes_number, int *nodes_transmitting) {

	int reason (PACKET_NOT_LOST);

	// Nodes implied in the NACK
	int node_a (logical_nack.node_id_a);
	int node_b (logical_nack.node_id_b);

	if(node_a == node_id ||  node_b == node_id){		// If node IMPLIED in the NACK

//		if(save_node_logs) fprintf(node_logger.file,
//				"%.12f;N%d;S%d;%s;%s I am implied in the NACK with packet id #%d\n",
//				sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.packet_id);

		// Sum new loss reason to corresponding type (for statistics purposes)
		++ nacks_received[logical_nack.loss_reason] ;

		switch(logical_nack.loss_reason){

			case PACKET_LOST_DESTINATION_TX:{	// Destination was already transmitting when the packet transmission was attempted

				if(save_node_logs) fprintf(node_logger.file, "%.12f;N%d;S%d;%s;%s Destination N%d was transmitting!s\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id);

//				// Add receiver to hidden nodes list ("I was not listening to him!")
//				hidden_nodes_list[logical_nack.source_id] = TRUE;

				reason = PACKET_LOST_DESTINATION_TX;

				break;
			}

			case PACKET_LOST_LOW_SIGNAL:{	// Signal strength is not enough to be decoded (less than capture effect)

				if(save_node_logs) fprintf(node_logger.file,
						"%.12f;N%d;S%d;%s;%s Power received in destination N%d is less than the required capture effect!\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id);

				reason = PACKET_LOST_LOW_SIGNAL;

				break;
			}

			case PACKET_LOST_INTERFERENCE:{ 	// There are interference signals making node not comply with the capture effect

				if(save_node_logs) fprintf(node_logger.file,
					"%.12f;N%d;S%d;%s;%s High interferences sensed in destination N%d (capture effect not accomplished)!\n",
					sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id);

//				printf(
//					"%.12f;N%d;S%d;%s;%s High interferences sensed in destination N%d (capture effect not accomplished)!\n",
//					sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id);

				// Increase the number of times of POTENTIAL hidden nodes with the current transmitting nodes
//				for(int i = 0; i < total_nodes_number; i++) {
//					if (nodes_transmitting[i] && i != node_id && i != logical_nack.source_id){
//						potential_hidden_nodes[i] ++;
//					}
//				}

				reason = PACKET_LOST_INTERFERENCE;

				break;
			}

			case PACKET_LOST_PURE_COLLISION:{	// Two nodes transmitting to same destination with signal strengths enough to be decoded

				if(save_node_logs) fprintf(node_logger.file,
					"%.12f;N%d;S%d;%s;%s Pure collision detected at destination %d! %d was transmitting and %d appeared\n",
					sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id,
					node_a, node_b);

//				// Add to hidden nodes list
//				if(node_a != node_id) {
//					hidden_nodes_list[node_a] = TRUE;
//				} else if (node_b != node_id) {
//					hidden_nodes_list[node_b] = TRUE;
//				}

				reason = PACKET_LOST_PURE_COLLISION;

				break;
			}

			case PACKET_LOST_LOW_SIGNAL_AND_RX:{ // Destination already receiving and new signal strength was not enough to be decoded

				// Only node_id_a has lost the packet, so that node_id_b is his hidden node
				if(node_a == node_id) {

					if(save_node_logs) fprintf(node_logger.file,
						"%.12f;N%d;S%d;%s;%s Destination N%d already receiving from N%d and N%d transmitted with not enough"
						" power to be decoded\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id, node_a, node_b);

//					hidden_nodes_list[node_b] = TRUE;

					reason = PACKET_LOST_LOW_SIGNAL_AND_RX;

				}

				break;
			}

			case PACKET_LOST_SINR_PROB:{	// Packet lost due to SINR probability (deprecated)

				if(save_node_logs) fprintf(node_logger.file, "%.12f;N%d;S%d;%s;%s Packet lost due constant PER or due to the BER (%f) "
					"associated to the current SINR (%f dB)\n", sim_time, node_id, node_state, LOG_H02, LOG_LVL2,
					logical_nack.ber, ConvertPower(LINEAR_TO_DB, logical_nack.sinr));

				reason = PACKET_LOST_SINR_PROB;

				break;
			}

			case PACKET_LOST_RX_IN_NAV:{			// Packet lost because node was in NAV
				if(save_node_logs) fprintf(node_logger.file, "%.12f;N%d;S%d;%s;%s Packet lost due to STA was in NAV\n",
					sim_time, node_id, node_state, LOG_H02, LOG_LVL2);

				reason = PACKET_LOST_RX_IN_NAV;

				break;
			}

			case PACKET_LOST_BO_COLLISION:{
				if(save_node_logs) fprintf(node_logger.file, "%.12f;N%d;S%d;%s;%s Packet lost due to Slotted Backoff\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2);

				reason = PACKET_LOST_BO_COLLISION;

				break;
			}

			case PACKET_LOST_OUTSIDE_CH_RANGE:{	// Destination was already transmitting when the packet transmission was attempted

//				printf("%.12f;N%d;S%d;%s;%s AP is sending packets outside STAs range!\n",
//						sim_time, node_id, node_state, LOG_H02, LOG_LVL2);

				if(save_node_logs) fprintf(node_logger.file, "%.12f;N%d;S%d;%s;%s AP is sending packets outside STAs range!\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2);

				reason = PACKET_LOST_LOW_SIGNAL_AND_RX;
				//exit(-1);

				break;
			}

			case PACKET_LOST_CAPTURE_EFFECT: {

				if(save_node_logs) fprintf(node_logger.file, "%.12f;N%d;S%d;%s;%s Packet lost by Capture Effect!\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2);

				reason = PACKET_LOST_CAPTURE_EFFECT;
				//exit(-1);

				break;

			}

			default:{

				if(save_node_logs) fprintf(node_logger.file, "%.12f;N%d;S%d;%s;%s Unknown reason for packet loss\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2);
				exit(EXIT_FAILURE);
				break;
			}

		}

	} else {	// If node NOT IMPLIED in the NACK, do nothing
//		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s I am NOT implied in the NACK\n",
//				sim_time, node_id, node_state, LOG_H02, LOG_LVL2);
	}

	return reason;
}

/*
 * CleanNack(LogicalNack nack): re-initializes the Nack info.
 */
void CleanNack(LogicalNack *nack){

	nack->source_id = NODE_ID_NONE;
	nack->packet_id = NO_PACKET_ID;
	nack->loss_reason = PACKET_NOT_LOST;
	nack->node_id_a = NODE_ID_NONE;
	nack->node_id_b = NODE_ID_NONE;

}

/*
 * handlePacketLoss(): handles a packet loss.
 */
void handlePacketLoss(int type, double *total_time_lost_in_num_channels, double *total_time_lost_per_channel,
		int &packets_lost, int &rts_cts_lost, int **packets_lost_per_sta, int **rts_cts_lost_per_sta,
		int current_right_channel, int current_left_channel,double current_tx_duration, int node_id, int destination_id){

	if(type == PACKET_TYPE_DATA) {
		for(int c = current_left_channel; c <= current_right_channel; c++){
			total_time_lost_per_channel[c] += current_tx_duration;
		}
		total_time_lost_in_num_channels[current_right_channel - current_left_channel] += current_tx_duration;
		++packets_lost;
		++(*packets_lost_per_sta)[destination_id-node_id-1];
	} else if(type == PACKET_TYPE_CTS){
		++rts_cts_lost;
		++(*rts_cts_lost_per_sta)[destination_id-node_id-1];
	}

}

/*
 * AttemptToDecodePacket(): attempts to decode incoming packet according to SINR and the capture effect (CE)
 **/
int AttemptToDecodePacket(double sinr, double capture_effect, double pd,
		double power_rx_interest, double constant_per, int node_id, int packet_type,
		int destination_id){

	int packet_lost;
	double per (0);

	// Try to decode when power received is greater than pd
	if(sinr < capture_effect || power_rx_interest < pd) {

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

/*
 * IsPacketLost(): computes notification loss according to SINR received
 **/
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

/*
 * GenerateTxInfo: generates a TxInfo
 **/
TxInfo GenerateTxInfo(int num_packets_aggregated, double data_duration,	double ack_duration,
		double rts_duration, double cts_duration, double current_tx_power, int num_channels_tx,
		double tx_gain,	int bits_ofdm_sym, double x, double y, double z) {

	TxInfo tx_info;
	tx_info.SetSizeOfMCS(4);	// TODO: make size dynamic

	tx_info.num_packets_aggregated = num_packets_aggregated;

	tx_info.data_duration = data_duration;
	tx_info.ack_duration = ack_duration;
	tx_info.rts_duration = rts_duration;
	tx_info.cts_duration = cts_duration;
	tx_info.tx_power = ComputeTxPowerPerChannel(current_tx_power, num_channels_tx);
	tx_info.tx_gain = tx_gain;
	tx_info.bits_ofdm_sym = bits_ofdm_sym;
	tx_info.x = x;
	tx_info.y = y;
	tx_info.z = z;
	tx_info.nav_time = 0;

	return tx_info;

}
