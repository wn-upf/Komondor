/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * node_packet_methods.h: Packet generation, notification dispatch, MCS selection,
 *   and frame exchange helper method implementations.
 *
 * NOTE: This file is an implementation fragment. It must be included from node.h
 *   after the Node class definition, not included directly.
 *
 * Functions defined here:
 *   - Node::StartTransmission
 *   - Node::RequestMCS
 *   - Node::SelectDestination
 *   - Node::GenerateNotification
 *   - Node::SendLogicalNack
 *   - Node::SendResponsePacket
 */

#ifndef NODE_PACKET_METHODS_H
#define NODE_PACKET_METHODS_H

/**
 * Pre-occupancy calls this (triggered-based operation)
 */
void Node :: StartTransmission(trigger_t &){
	rts_notification.timestamp = SimTime();
	outportSelfStartTX(rts_notification);
}

/**
 * Request an MCS response from the destination node
 */
void Node :: RequestMCS(){

//	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s RequestMCS() to N%d\n",
//				SimTime(), node_id, node_state, LOG_G00, LOG_LVL1, current_destination_id);

	// Only one channel required (logically!)
	// Receiver is able to determine the power received when transmitter uses more than one channel by its own
	current_left_channel = current_primary_channel;
	current_right_channel = current_primary_channel;

	// Send request MCS notification
	Notification request_modulation = GenerateNotification(PACKET_TYPE_MCS_REQUEST, current_destination_id,
		-1, -1, -1, TX_DURATION_NONE);

	request_modulation.tx_info.flag_change_in_tx_power = TRUE;

	outportAskForTxModulation(request_modulation);

	int ix_aux (current_destination_id - wlan.list_sta_id[0]);	// Auxiliary variable for correcting the node id offset
	// MCS of receiver is not pending anymore
	change_modulation_flag[ix_aux] = FALSE;

	//if(first_time_requesting_mcs) {
	//	first_time_requesting_mcs = FALSE;
	//}
	// LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s RequestMCS() END\n", SimTime(), node_id, node_state, LOG_G00, LOG_LVL1);
}

/**
 * Select the destination node before transmitting
 */
void Node :: SelectDestination(){

//	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s SelectDestination()\n",
//			SimTime(), node_id, node_state, LOG_G00, LOG_LVL1);

	current_destination_id = PickRandomElementFromArray(wlan.list_sta_id, wlan.num_stas);
	// LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s SelectDestination() END\n", SimTime(), node_id, node_state, LOG_G00, LOG_LVL1);
}

/**
 * Generate a Notification
 * @param "packet_type" [type int]: type of packet to be generated
 * @param "destination_id" [type int]: identifier of the destination node
 * @param "packet_id" [type int]: identifier of the generated packet
 * @param "num_packets_aggregated" [type int]: number of data frames to be aggregated
 * @param "timestamp_generated" [type double]: timestamp at which the notification is generated
 * @param "tx_duration" [type double]: duration of the transmission
 * @return "Notification" [type Notification]: generated notification
 */
Notification Node :: GenerateNotification(int packet_type, int destination_id, int packet_id,
	int num_packets_aggregated, double timestamp_generated, double tx_duration){

	Notification notification;

	notification.packet_id = packet_id;				// ID of the first packet
	notification.packet_type = packet_type;
	notification.source_id = node_id;
	notification.destination_id = destination_id;
	notification.tx_duration = tx_duration;
	notification.tx_info.total_tx_power = current_tx_power;

	if(packet_type == PACKET_TYPE_MCS_REQUEST && first_time_requesting_mcs) {
		notification.left_channel = current_primary_channel;
		notification.right_channel = current_primary_channel;
		//first_time_requesting_mcs = FALSE;
	} else {
		notification.left_channel = current_left_channel;
		notification.right_channel = current_right_channel;
	}

	notification.frame_length = -1;
	notification.modulation_id = current_modulation;
	notification.timestamp = SimTime();
	notification.timestamp_generated = timestamp_generated;

	// P_tx issue #113
	num_channels_tx = current_right_channel - current_left_channel + 1;

	if (spatial_reuse_enabled && txop_sr_identified) {
		notification.tx_info = GenerateTxInfo(num_packets_aggregated, data_duration,
			ack_duration, rts_duration, cts_duration, current_tx_power_sr, num_channels_tx,
			bits_ofdm_sym, x, y, z, flag_change_in_tx_power);
	} else {
		notification.tx_info = GenerateTxInfo(num_packets_aggregated, data_duration,
			ack_duration, rts_duration, cts_duration, current_tx_power, num_channels_tx,
			bits_ofdm_sym, x, y, z, flag_change_in_tx_power);
	}

	// Spatial Reuse parameters
	notification.tx_info.bss_color = bss_color;
	notification.tx_info.srg = srg;

//	// Notify potential changes in the tx power
//	notification.tx_info.flag_change_in_tx_power = flag_change_in_tx_power;

	switch(packet_type){

		case PACKET_TYPE_DATA:{
			notification.frame_length = frame_length;
			notification.tx_info.nav_time = current_nav_time;
			break;
		}

		case PACKET_TYPE_ACK:{
			notification.frame_length = IEEE_AX_ACK_LENGTH;
			notification.tx_info.nav_time = current_nav_time;
			break;
		}

		case PACKET_TYPE_MCS_REQUEST:{
			// Do nothing
			break;
		}

		case PACKET_TYPE_MCS_RESPONSE:{
			for(int i = 0; i < NUM_OPTIONS_CHANNEL_LENGTH; ++i) {
				notification.tx_info.modulation_schemes[i] = mcs_response[i];
			}
			break;
		}

		case PACKET_TYPE_RTS:{
			notification.frame_length = IEEE_AX_RTS_LENGTH;
			notification.tx_info.nav_time = current_nav_time;
			break;
		}

		case PACKET_TYPE_CTS:{
			notification.frame_length = IEEE_AX_CTS_LENGTH;
			notification.tx_info.nav_time = current_nav_time;
			break;
		}

		default:{
			printf("ERROR: Packet type unknown\n");
			exit(EXIT_FAILURE);
			break;
		}
	}

	return notification;

}

/**
 * Send a NACK notification
 * @param "logical_nack" [type LogicalNack]: logical NACK object to be sent
 */
void Node :: SendLogicalNack(LogicalNack logical_nack){

	outportSendLogicalNack(logical_nack);

	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s NACK of packet type %d sent to a:N%d (and b:N%d) with reason %d\n",
		SimTime(), node_id, node_state, LOG_I00, LOG_LVL4, logical_nack.packet_type,
		logical_nack.node_id_a, logical_nack.node_id_b, logical_nack.loss_reason);

}

/**
 * Sends the response according to the current state (trigger-based operation)
 */
void Node :: SendResponsePacket(trigger_t &){

	switch(node_state){

		case STATE_TX_ACK:{

			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s SIFS completed after receiving DATA, sending ACK...\n",
				SimTime(), node_id, node_state, LOG_I00, LOG_LVL3);

			outportSelfStartTX(ack_notification);

			// trigger_toFinishTX.Set(SimTime() + current_tx_duration);
			// time_to_trigger = TruncateDouble(SimTime() + FEMTO_VALUE,12) + current_tx_duration;
			time_to_trigger = SimTime() + current_tx_duration;
			trigger_toFinishTX.Set(FixTimeOffset(time_to_trigger,13,12));

			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s TruncateDouble = %.12f - current_tx_duration = %.12f - trigger_toFinishTX = %.12f\n",
				SimTime(), node_id, node_state, LOG_I00, LOG_LVL3,
				TruncateDouble(SimTime() + FEMTO_VALUE,12), current_tx_duration, trigger_toFinishTX.GetTime());

			break;
		}

		case STATE_TX_CTS:{
			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s SIFS completed after receiving RTS, sending CTS (duration = %f)\n",
				SimTime(), node_id, node_state, LOG_I00, LOG_LVL3, current_tx_duration);
			outportSelfStartTX(cts_notification);

			time_to_trigger = SimTime() + current_tx_duration;
			trigger_toFinishTX.Set(FixTimeOffset(time_to_trigger,13,12));
			break;
		}

		case STATE_TX_DATA:{
			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s SIFS completed after receiving CTS, sending DATA...\n",
				SimTime(), node_id, node_state, LOG_I00, LOG_LVL3);
			outportSelfStartTX(data_notification);
			time_to_trigger = SimTime() + current_tx_duration;
			trigger_toFinishTX.Set(FixTimeOffset(time_to_trigger,13,12));
			++data_packets_sent;
			++data_packets_sent_per_sta[current_destination_id-node_id-1];

			// Update performance measurements
			++performance_report.data_packets_sent;
			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Data TX will be finished at %.15f\n",
				SimTime(), node_id, node_state, LOG_I00, LOG_LVL3,
				trigger_toFinishTX.GetTime());
			break;
		}
	}
}

#endif /* NODE_PACKET_METHODS_H */
