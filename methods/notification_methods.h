/* This is just an sketch of what our Komondor headers should look like.
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * All rights reserved.
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
 * File description:
 *
 * - Bla bla bla...
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
void ProcessNack(LogicalNack logical_nack, int node_id, Logger node_logger, int node_state,
		int save_node_logs,	double sim_time, int *nacks_received, int *hidden_nodes_list,
		int *potential_hidden_nodes, int total_nodes_number, int *nodes_transmitting) {

	// Nodes implied in the NACK
	int node_a = logical_nack.node_id_a;
	int node_b = logical_nack.node_id_b;

	if(node_a == node_id ||  node_b == node_id){		// If node IMPLIED in the NACK

		if(save_node_logs) fprintf(node_logger.file,
				"%.15f;N%d;S%d;%s;%s I am implied in the NACK with packet id #%d\n",
				sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.packet_id);

		// Sum new loss reason to corresponding type (for statistics purposes)
		nacks_received[logical_nack.loss_reason] ++;

		switch(logical_nack.loss_reason){

			case PACKET_LOST_DESTINATION_TX:{	// Destination was already transmitting when the packet transmission was attempted

				if(save_node_logs) fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Destination N%d was transmitting!s\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id);

				// Add receiver to hidden nodes list ("I was not listening to him!")
				hidden_nodes_list[logical_nack.source_id] = TRUE;

				break;
			}

			case PACKET_LOST_LOW_SIGNAL:{	// Signal strength is not enough to be decoded (less than capture effect)

				if(save_node_logs) fprintf(node_logger.file,
						"%.15f;N%d;S%d;%s;%s Power received in destination N%d is less than the required capture effect!\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id);

				break;
			}

			case PACKET_LOST_INTERFERENCE:{ 	// There are interference signals making node not comply with the capture effect

				if(save_node_logs) fprintf(node_logger.file,
						"%.15f;N%d;S%d;%s;%s High interferences sensed in destination N%d (capture effect not accomplished)!\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id);

				// Increase the number of times of POTENTIAL hidden nodes with the current transmitting nodes
				for(int i = 0; i < total_nodes_number; i++) {
					if (nodes_transmitting[i] && i != node_id && i != logical_nack.source_id){
						potential_hidden_nodes[i] ++;
					}
				}

				break;
			}

			case PACKET_LOST_PURE_COLLISION:{	// Two nodes transmitting to same destination with signal strengths enough to be decoded

				if(save_node_logs) fprintf(node_logger.file,
					"%.15f;N%d;S%d;%s;%s Pure collision detected at destination %d! %d was transmitting and %d appeared\n",
					sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id,
					node_a, node_b);

				// Add to hidden nodes list
				if(node_a != node_id) {
					hidden_nodes_list[node_a] = TRUE;
				} else if (node_b != node_id) {
					hidden_nodes_list[node_b] = TRUE;
				}

				break;
			}

			case PACKET_LOST_LOW_SIGNAL_AND_RX:{ // Destination already receiving and new signal strength was not enough to be decoded

				// Only node_id_a has lost the packet, so that node_id_b is his hidden node
				if(node_a == node_id) {

					if(save_node_logs) fprintf(node_logger.file,
							"%.15f;N%d;S%d;%s;%s Destination N%d already receiving from N%d and N%d transmitted with not enough"
							" power to be decoded\n",
							sim_time, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id, node_a, node_b);

					hidden_nodes_list[node_b] = TRUE;
				}

				break;
			}

			case PACKET_LOST_SINR_PROB:{	// Packet lost due to SINR probability (deprecated)

				if(save_node_logs) fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Packet lost due to the BER (%f) "
						"associated to the current SINR (%f dB)\n", sim_time, node_id, node_state, LOG_H02, LOG_LVL2,
						logical_nack.ber, ConvertPower(LINEAR_TO_DB, logical_nack.sinr));

				break;
			}

			case PACKET_LOST_RX_IN_NAV:{			// Packet lost because node was in NAV
				if(save_node_logs) fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Packet lost due to NAV\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2);

				break;
			}

			case PACKET_LOST_BO_COLLISION:{
				if(save_node_logs) fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Packet lost due to Slotted Backoff\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2);

				break;
			}

			default:{

				if(save_node_logs) fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Unknown reason for packet loss\n",
						sim_time, node_id, node_state, LOG_H02, LOG_LVL2);
				exit(EXIT_FAILURE);
				break;
			}

		}

	} else {	// If node NOT IMPLIED in the NACK, do nothing
//		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s I am NOT implied in the NACK\n",
//				sim_time, node_id, node_state, LOG_H02, LOG_LVL2);
	}
}

/*
 * handlePacketLoss(): handles a packet loss.
 */
void handlePacketLoss(int type, double *total_time_lost_in_num_channels, double *total_time_lost_per_channel,
		int &packets_lost, int &rts_cts_lost, int current_right_channel, int current_left_channel,
		double current_tx_duration){

	if(type == PACKET_TYPE_DATA) {
		for(int c = current_left_channel; c <= current_right_channel; c++){
			total_time_lost_per_channel[c] += current_tx_duration;
		}

		total_time_lost_in_num_channels[current_right_channel - current_left_channel] += current_tx_duration;

		packets_lost ++;
	} else if(type == PACKET_TYPE_CTS){
		// TODO
		rts_cts_lost ++;
	}

}

/*
 * AttemptToDecodePacket(): attempts to decode incoming packet according to SINR and the capture effect (CE)
 **/
int AttemptToDecodePacket(double sinr, double capture_effect, double cca,
		double power_rx_interest, double constant_per, int node_id){

	int packet_lost;
	double per;

	// Try to decode when power received is greater than CCA
	if(sinr < capture_effect && power_rx_interest > cca) {

		per = 1;

	} else {

		per = constant_per;

	}

	packet_lost = ((double) rand() / (RAND_MAX)) < per;
	return packet_lost;
}

/*
 * IsPacketLost(): computes notification loss according to SINR received
 **/
int IsPacketLost(Notification notification, double sinr, double capture_effect, double cca,
		double power_rx_interest, double constant_per, int *hidden_nodes_list, int node_id){

	int loss_reason = PACKET_NOT_LOST;
	int is_packet_lost;	// Determines if the current notification has been lost (1) or not (0)

	//is_packet_lost = applyModulationProbabilityError(notification);

	is_packet_lost = AttemptToDecodePacket(sinr, capture_effect, cca, power_rx_interest, constant_per,
			node_id);

	if (is_packet_lost) {

		if (power_rx_interest < cca) {	// Signal strength is not enough (< CCA) to be decoded

			loss_reason = PACKET_LOST_LOW_SIGNAL;
			hidden_nodes_list[notification.source_id] = TRUE;

		} else if (sinr < capture_effect){	// Capture effect not accomplished

			loss_reason = PACKET_LOST_INTERFERENCE;

		} else {	// Incoming packet lost due to PER

			loss_reason = PACKET_LOST_SINR_PROB;

		}
	}

	return loss_reason;

}
