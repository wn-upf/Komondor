#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../ListOfDefines.h"

/*
 * generateAndSendLogicalNack: Sends a NACK notification
 * Arguments:
 * - packet_id:
 * - node_id_a:
 * - node_id_b:
 * - reason_id:
 */
LogicalNack generateLogicalNack(int packet_type, int packet_id, int node_id,
		int node_id_a, int node_id_b, int loss_reason, double BER, double current_sinr){

	LogicalNack logical_nack;

	logical_nack.packet_type = packet_type;
	logical_nack.source_id = node_id;
	logical_nack.packet_id = packet_id;
	logical_nack.loss_reason = loss_reason;
	logical_nack.node_id_a = node_id_a;
	logical_nack.node_id_b = node_id_b;
	logical_nack.BER = BER;
	logical_nack.SINR = current_sinr;

	return logical_nack;

}

/*
 * processNack(): processes a NACK notification.
 * Arguments:
 * - logical_nack: Nack information
 */
void processNack(LogicalNack logical_nack, int node_id, Logger node_logger, int node_state,
		int save_node_logs,	double SimTime, int *nacks_received, int *hidden_nodes_list,
		int *potential_hidden_nodes, int total_nodes_number, int *nodes_transmitting) {

	// Nodes implied in the NACK
	int node_A = logical_nack.node_id_a;
	int node_B = logical_nack.node_id_b;

	if(node_A == node_id ||  node_B == node_id){		// If node IMPLIED in the NACK

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s I am implied in the NACK with packet id #%d\n",
				SimTime, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.packet_id);

		// Sum new loss reason to corresponding type (for statistics purposes)
		nacks_received[logical_nack.loss_reason] ++;

		switch(logical_nack.loss_reason){

			case PACKET_LOST_DESTINATION_TX:{	// Destination was already transmitting when the packet transmission was attempted

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Destination N%d was transmitting!s\n",
						SimTime, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id);

				// Add receiver to hidden nodes list ("I was not listening to him!")
				hidden_nodes_list[logical_nack.source_id] = TRUE;

				break;
			}

			case PACKET_LOST_LOW_SIGNAL:{	// Signal strength is not enough to be decoded (less than capture effect)

				if(save_node_logs) fprintf(node_logger.file,
						"%f;N%d;S%d;%s;%s Power received in destination N%d is less than the required capture effect!\n",
						SimTime, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id);

				break;
			}

			case PACKET_LOST_INTERFERENCE:{ 	// There are interference signals making node not comply with the capture effect

				if(save_node_logs) fprintf(node_logger.file,
						"%f;N%d;S%d;%s;%s High interferences sensed in destination N%d (capture effect not accomplished)!\n",
						SimTime, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id);

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
					"%f;N%d;S%d;%s;%s Pure collision detected at destination %d! %d was transmitting and %d appeared\n",
					SimTime, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id,
					node_A, node_B);

				// Add to hidden nodes list
				if(node_A != node_id) {
					hidden_nodes_list[node_A] = TRUE;
				} else if (node_B != node_id) {
					hidden_nodes_list[node_B] = TRUE;
				}

				break;
			}

			case PACKET_LOST_LOW_SIGNAL_AND_RX:{ // Destination already receiving and new signal strength was not enough to be decoded

				// Only node_id_a has lost the packet, so that node_id_b is his hidden node
				if(node_A == node_id) {

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s Destination N%d already receiving from N%d and N%d transmitted with not enough"
							" power to be decoded\n",
							SimTime, node_id, node_state, LOG_H02, LOG_LVL2, logical_nack.source_id, node_A, node_B);

					hidden_nodes_list[node_B] = TRUE;
				}

				break;
			}

			case PACKET_LOST_SINR_PROB:{	// Packet lost due to SINR probability (deprecated)

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Packet lost due to the BER (%f) "
						"associated to the current SINR (%f dB)\n", SimTime, node_id, node_state, LOG_H02, LOG_LVL2,
						logical_nack.BER, convertPower(LINEAR_TO_DB, logical_nack.SINR));

				break;
			}

			case PACKET_LOST_RX_IN_NAV:{			// Packet lost because node was in NAV
				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Packet lost due to NAV\n",
						SimTime, node_id, node_state, LOG_H02, LOG_LVL2);

				break;
			}

			default:{

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Unknown reason for packet loss\n",
						SimTime, node_id, node_state, LOG_H02, LOG_LVL2);
				exit(EXIT_FAILURE);
				break;
			}

		}

	} else {	// If node NOT IMPLIED in the NACK, do nothing
		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s I am NOT implied in the NACK\n",
				SimTime, node_id, node_state, LOG_H02, LOG_LVL2);
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
 * attemptToDecodePacket(): attempts to decode incoming packet according to SINR and the CE
 */
int attemptToDecodePacket(double current_sinr_dbm, double capture_effect_dbm, double current_cca_dbm,
		double pw_received_interest_dbm, double constant_PER, int node_id){

	int packet_lost;
	double PER;

	// Try to decode when power received is greater than CCA
	if(current_sinr_dbm < capture_effect_dbm
			&& pw_received_interest_dbm > current_cca_dbm) {

		PER = 1;

	} else {

		PER = constant_PER;

	}

	packet_lost = ((double) rand() / (RAND_MAX)) < PER;
	return packet_lost;
}

/*
 * isPacketLost(): computes notification loss according to SINR received
 * Arguments:
 * - channel_power: power sensed in the channel of interest
 * - notification: notification info received
 * Output:
 * - reason: reason ID of the packet loss
 */

int isPacketLost(Notification notification, double current_sinr_dbm,
		double capture_effect_dbm, double current_cca_dbm,
		double pw_received_interest_dbm, double constant_PER, int *hidden_nodes_list,
		int save_node_logs, Logger node_logger, int node_id, int node_state,
		double SimTime){

	int loss_reason = PACKET_NOT_LOST;
	int is_packet_lost;	// Determines if the current notification has been lost (1) or not (0)

	//is_packet_lost = applyModulationProbabilityError(notification);

	is_packet_lost = attemptToDecodePacket(current_sinr_dbm, capture_effect_dbm, current_cca_dbm,
			pw_received_interest_dbm, constant_PER, node_id);

	if (is_packet_lost) {

		if (pw_received_interest_dbm < current_cca_dbm) {	// Signal strength is not enough (< CCA) to be decoded

			if(save_node_logs) fprintf(node_logger.file,
					"%f;N%d;S%d;%s;%s Signal strength (%f dBm) is less than CCA (%f dBm)\n",
				SimTime, node_id, node_state, LOG_D17, LOG_LVL4,
				pw_received_interest_dbm, current_cca_dbm);

			loss_reason = PACKET_LOST_LOW_SIGNAL;
			hidden_nodes_list[notification.source_id] = TRUE;

		} else if (current_sinr_dbm < capture_effect_dbm){	// Capture effect not accomplished

			if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s Capture effect not accomplished (current_sinr = %f dBm)\n",
				SimTime, node_id, node_state, LOG_D17, LOG_LVL4, current_sinr_dbm);
			loss_reason = PACKET_LOST_INTERFERENCE;

		} else {

			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Incoming packet is lost due to PER\n",
				SimTime, node_id, node_state, LOG_D17, LOG_LVL4);
			loss_reason = PACKET_LOST_SINR_PROB;

		}
	}

	return loss_reason;

}
