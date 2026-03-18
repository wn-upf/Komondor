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
 *   - Node::InitiateBurstPackets
 *   - Node::ScheduleTransmission
 *   - Node::PrepareNewTransmission
 *   - Node::EndBackoff
 *   - Node::MyTxFinished
 */

#ifndef NODE_PACKET_METHODS_H
#define NODE_PACKET_METHODS_H

/**
 * Pre-occupancy calls this (triggered-based operation)
 */
void Node :: StartTransmission(trigger_t &){
	// Check STATE_TX_TF first: TF is sent mid-sequence (exchange_sequence[0] is still ICF)
	if (node_state == STATE_TX_TF) {
		tf_notification.timestamp = SimTime();
		outportSelfStartTX(tf_notification);
	} else if (exchange_sequence.frame_types[0] == PACKET_TYPE_RTS) {
		rts_notification.timestamp = SimTime();
		outportSelfStartTX(rts_notification);
	} else if (exchange_sequence.frame_types[0] == PACKET_TYPE_ICF) {
		icf_notification.timestamp = SimTime();
		outportSelfStartTX(icf_notification);
	} else {
		data_notification.timestamp = SimTime();
		outportSelfStartTX(data_notification);
	}
}

/**
 * Request an MCS response from the destination node
 */
void Node :: RequestMCS(){

//	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s RequestMCS() to N%d\n",
//				SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL1, current_destination_id);
	// Only one channel required (logically!)
	// Receiver is able to determine the power received when transmitter uses more than one channel by its own
	current_left_channel = node_params.current_primary_channel;
	current_right_channel = node_params.current_primary_channel;
	// Send request MCS notification
	Notification request_modulation = GenerateNotification(PACKET_TYPE_MCS_REQUEST, current_destination_id,
		-1, -1, -1, TX_DURATION_NONE);
	request_modulation.tx_info.flag_change_in_tx_power = TRUE;
	outportAskForTxModulation(request_modulation);
	int ix_aux (current_destination_id - wlan.list_sta_id[0]);	// Auxiliary variable for correcting the node id offset	
	change_modulation_flag[ix_aux] = FALSE; 	// MCS of receiver is not pending anymore
	// LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s RequestMCS() END\n", SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL1);
}

/**
 * Select the destination node before transmitting
 */
void Node :: SelectDestination(){
	current_destination_id = PickRandomElementFromArray(wlan.list_sta_id, wlan.num_stas);
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
	int num_packets_aggregated, double timestamp_generated, double tx_duration) {

	Notification notification;

	notification.packet_id = packet_id;				// ID of the first packet
	notification.packet_type = packet_type;
	notification.source_id = node_params.node_id;
	notification.destination_id = destination_id;
	notification.tx_duration = tx_duration;
	notification.tx_info.total_tx_power = current_tx_power;

	if(packet_type == PACKET_TYPE_MCS_REQUEST && first_time_requesting_mcs) {
		notification.left_channel = node_params.current_primary_channel;
		notification.right_channel = node_params.current_primary_channel;
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

	if (sr_state.mapc_cosr_active || (sr_state.spatial_reuse_enabled && sr_state.txop_sr_identified)) {
		notification.tx_info = GenerateTxInfo(num_packets_aggregated, data_duration,
			ack_duration, rts_duration, cts_duration, sr_state.current_tx_power_sr, num_channels_tx,
			bits_ofdm_sym, node_params.x, node_params.y, node_params.z,
		sr_state.flag_change_in_tx_power || (sr_state.mapc_cosr_active ? TRUE : FALSE));
	} else {
		notification.tx_info = GenerateTxInfo(num_packets_aggregated, data_duration,
			ack_duration, rts_duration, cts_duration, current_tx_power, num_channels_tx,
			bits_ofdm_sym, node_params.x, node_params.y, node_params.z, sr_state.flag_change_in_tx_power);
	}

	// Spatial Reuse parameters
	notification.tx_info.bss_color = node_params.bss_color;
	notification.tx_info.srg = node_params.srg;

	// Beamforming parameters (per-TXOP; updated in PrepareNewTransmission)
	if (node_params.beamforming_enabled) {
		notification.tx_info.beamforming_active = BEAMFORMING_ENABLED;
		notification.tx_info.beam_N_elements    = node_params.beam_N_elements;
		notification.tx_info.beam_d_spacing     = node_params.beam_d_spacing;
		notification.tx_info.beam_az_main_rad   = current_beam_az_main_rad;
		notification.tx_info.beam_num_nulls     = current_beam_num_nulls;
		for (int _bk = 0; _bk < current_beam_num_nulls; ++_bk)
			notification.tx_info.beam_null_az_rad[_bk] = current_beam_null_az_rad[_bk];
	}

//	// Notify potential changes in the tx power
//	notification.tx_info.flag_change_in_tx_power = sr_state.flag_change_in_tx_power;

	switch(packet_type){

		case PACKET_TYPE_DATA:{
			notification.frame_length = node_params.frame_length;
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

		case PACKET_TYPE_ICF:{
			// MAPC Initial Control Frame — similar size to RTS
			notification.frame_length = IEEE_AX_RTS_LENGTH;
			notification.tx_info.nav_time = current_nav_time;
			break;
		}

		case PACKET_TYPE_ICR:{
			// MAPC Initial Control Response — similar size to CTS
			notification.frame_length = IEEE_AX_CTS_LENGTH;
			notification.tx_info.nav_time = current_nav_time;
			break;
		}

		case PACKET_TYPE_TF:{
			// MAPC Trigger Frame — similar size to RTS
			notification.frame_length = IEEE_AX_RTS_LENGTH;
			notification.tx_info.nav_time = current_nav_time;
			break;
		}

		case PACKET_TYPE_MU_RTS_TXS:{
			// MAPC MU-RTS/TXS — grants coordinated AP TX slot; similar size to RTS
			notification.frame_length = IEEE_AX_RTS_LENGTH;
			notification.tx_info.nav_time = current_nav_time;
			break;
		}

		case PACKET_TYPE_ACK_TF:{
			// MAPC ACK Trigger Frame — similar size to RTS/CTS; no NAV needed
			notification.frame_length = IEEE_AX_RTS_LENGTH;
			notification.tx_info.nav_time = 0;
			break;
		}

		default:{
			printf("ERROR: Packet type unknown\n");
			exit(EXIT_FAILURE);
			break;
		}
	}

	// Propagate MAPC group ID (0 for standard non-MAPC frames)
	notification.mapc_group_id = wlan.mapc_group_ids[mapc_active_group_idx];
	notification.mapc_target_ap_id = NODE_ID_NONE; // default: no specific target
	notification.mapc_has_data     = TRUE;          // default: assume peer has data

	return notification;

}

/**
 * Send a NACK notification
 * @param "logical_nack" [type LogicalNack]: logical NACK object to be sent
 */
void Node :: SendLogicalNack(LogicalNack logical_nack){

	outportSendLogicalNack(logical_nack);

	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s NACK of packet type %d sent to a:N%d (and b:N%d) with reason %d\n",
		SimTime(), node_params.node_id, node_state, LOG_I00, LOG_LVL4, logical_nack.packet_type,
		logical_nack.node_id_a, logical_nack.node_id_b, logical_nack.loss_reason);

}

/**
 * Sends the response according to the current state (trigger-based operation)
 */
void Node :: SendResponsePacket(trigger_t &){

	switch(node_state){

		case STATE_TX_ACK:{

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s SIFS completed after receiving DATA, sending ACK...\n",
				SimTime(), node_params.node_id, node_state, LOG_I00, LOG_LVL3);

			outportSelfStartTX(ack_notification);

			// trigger_toFinishTX.Set(SimTime() + current_tx_duration);
			// time_to_trigger = TruncateDouble(SimTime() + FEMTO_VALUE,12) + current_tx_duration;
			time_to_trigger = SimTime() + current_tx_duration;
			trigger_toFinishTX.Set(FixTimeOffset(time_to_trigger,13,12));

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s TruncateDouble = %.12f - current_tx_duration = %.12f - trigger_toFinishTX = %.12f\n",
				SimTime(), node_params.node_id, node_state, LOG_I00, LOG_LVL3,
				TruncateDouble(SimTime() + FEMTO_VALUE,12), current_tx_duration, trigger_toFinishTX.GetTime());

			break;
		}

		case STATE_TX_CTS:{
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s SIFS completed after receiving RTS, sending CTS (duration = %f)\n",
				SimTime(), node_params.node_id, node_state, LOG_I00, LOG_LVL3, current_tx_duration);
			outportSelfStartTX(cts_notification);

			time_to_trigger = SimTime() + current_tx_duration;
			trigger_toFinishTX.Set(FixTimeOffset(time_to_trigger,13,12));
			break;
		}

		case STATE_TX_DATA:{
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s SIFS completed after receiving CTS, sending DATA...\n",
				SimTime(), node_params.node_id, node_state, LOG_I00, LOG_LVL3);
			outportSelfStartTX(data_notification);
			time_to_trigger = SimTime() + current_tx_duration;
			trigger_toFinishTX.Set(FixTimeOffset(time_to_trigger,13,12));
			++node_stats.data_packets_sent;
			++node_stats.data_packets_sent_per_sta[current_destination_id-node_params.node_id-1];

			// Update performance measurements
			++performance_report.data_packets_sent;
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Data TX will be finished at %.15f\n",
				SimTime(), node_params.node_id, node_state, LOG_I00, LOG_LVL3,
				trigger_toFinishTX.GetTime());
			break;
		}

		case STATE_TX_ICR:{
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s SIFS (+stagger) completed after receiving ICF, sending ICR...\n",
				SimTime(), node_params.node_id, node_state, LOG_I00, LOG_LVL3);
			outportSelfStartTX(icr_notification);
			time_to_trigger = SimTime() + current_tx_duration;
			trigger_toFinishTX.Set(FixTimeOffset(time_to_trigger, 13, 12));
			break;
		}

		case STATE_TX_ACK_TF:{
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s SIFS completed after DATA, sending ACK TF (broadcast)...\n",
				SimTime(), node_params.node_id, node_state, LOG_I00, LOG_LVL3);
			outportSelfStartTX(ack_tf_notification);
			time_to_trigger = SimTime() + current_tx_duration;
			trigger_toFinishTX.Set(FixTimeOffset(time_to_trigger, 13, 12));
			break;
		}

		case STATE_TX_RTS:{
			// MAPC Co-TDMA: sending MU-RTS/TXS after coordinator's own DATA/ACK
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s Sending MU-RTS/TXS to N%d after SIFS\n",
				SimTime(), node_params.node_id, node_state, LOG_I00, LOG_LVL3,
				current_destination_id);
			outportSelfStartTX(mu_rts_notification);
			time_to_trigger = SimTime() + current_tx_duration;
			trigger_toFinishTX.Set(FixTimeOffset(time_to_trigger, 13, 12));
			break;
		}
	}
}

/**
 * Helper for EndBackoff: injects burst packets into the buffer
 */
void Node :: InitiateBurstPackets(){

	// - Add another bunch of packets to the buffer if TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION
	//if(node_params.traffic_model == TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION && last_transmission_successful) {
	if(node_params.traffic_model == TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION) {

		// - Generate the aggregated frames to be sent in the next transmission
		for(int i = 0; i < limited_num_packets_aggregated; ++i){
			new_packet = null_notification;
			new_packet.timestamp_generated = SimTime();
			new_packet.packet_id = last_packet_generated_id;
			buffer.PutPacket(new_packet);
			++last_packet_generated_id;
		}

		// Set "last_transmission_successful" to 0 for the upcoming transmission
		last_transmission_successful = 0;

	}
}

/**
 * Helper for EndBackoff: sets the preoccupancy and toFinishTX triggers.
 * @param "first_packet_type" [type int]: PACKET_TYPE_RTS (RTS/CTS enabled),
 *   PACKET_TYPE_DATA (RTS/CTS disabled)
 */
void Node :: ScheduleTransmission(int first_packet_type){

	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s -------------------------------------\n",
		SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL2);
	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s TRANSMISSION #%d STARTED\n",
		SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL2,
		node_stats.rts_cts_sent);
	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s -------------------------------------\n",
		SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL2);

	if (first_packet_type == PACKET_TYPE_RTS) {
		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Transmission of RTS #%d started\n",
			SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL3, rts_notification.packet_id);
		++node_stats.rts_cts_sent;
		++node_stats.rts_cts_sent_per_sta[current_destination_id-node_params.node_id-1];
		++performance_report.rts_cts_sent;
	} else if (first_packet_type == PACKET_TYPE_ICF) {
		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Transmission of ICF #%d started\n",
			SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL3, icf_notification.packet_id);
		++node_stats.rts_cts_sent;
		++performance_report.rts_cts_sent;
		// ICF is broadcast — no per-STA breakdown (rts_cts_sent_per_sta not updated)
	} else if (first_packet_type == PACKET_TYPE_TF) {
		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Transmission of TF #%d started\n",
			SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL3, tf_notification.packet_id);
	} else if (first_packet_type == PACKET_TYPE_MU_RTS_TXS) {
		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Transmission of MU-RTS/TXS #%d started\n",
			SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL3, mu_rts_notification.packet_id);
	} else {
		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Transmission of DATA #%d started\n",
			SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL3, data_notification.packet_id);
		// 2-way path (no RTS/CTS): count DATA TX here since SendResponsePacket is never called
		++node_stats.data_packets_sent;
		++node_stats.data_packets_sent_per_sta[current_destination_id - node_params.node_id - 1];
		++performance_report.data_packets_sent;
	}

	// ------------------------------------------------------------------------
	// Sergio on 07 Dec 2017: add transmission time to spectrum utilization
	for(int c = current_left_channel; c <= current_right_channel; ++c){
		node_stats.total_time_channel_busy_per_channel[c] += current_tx_duration;
	}
	// ------------------------------------------------------------------------

	// Send preoccupancy trigger and trigger to finish transmission
	time_to_trigger = SimTime() + time_rand_value;
	trigger_preoccupancy.Set(FixTimeOffset(time_to_trigger,13,12));
	if (first_packet_type == PACKET_TYPE_RTS) {
		rts_notification.tx_info.preoccupancy_duration = time_rand_value;
	} else if (first_packet_type == PACKET_TYPE_ICF) {
		icf_notification.tx_info.preoccupancy_duration = time_rand_value;
	} else if (first_packet_type == PACKET_TYPE_TF) {
		tf_notification.tx_info.preoccupancy_duration = time_rand_value;
	} else if (first_packet_type == PACKET_TYPE_MU_RTS_TXS) {
		mu_rts_notification.tx_info.preoccupancy_duration = time_rand_value;
	} else {
		data_notification.tx_info.preoccupancy_duration = time_rand_value;
	}

	time_to_trigger = SimTime() + current_tx_duration;

	trigger_toFinishTX.Set(FixTimeOffset(time_to_trigger,13,12));
	trigger_start_backoff.Cancel();	// Safety instruction
}

/**
 * Helper for EndBackoff: selects destination, MCS, builds the first-frame notification
 */
void Node :: PrepareNewTransmission() {

	// Identify the channel range to TX in depending on the channel bonding scheme and free channels
	int ix_mcs_per_node (current_destination_id - wlan.list_sta_id[0]);

	// Get the transmission channels
	current_left_channel = GetFirstOrLastTrueElemOfArray(FIRST_TRUE_IN_ARRAY,
		channels_for_tx, NUM_CHANNELS_KOMONDOR);
	current_right_channel = GetFirstOrLastTrueElemOfArray(LAST_TRUE_IN_ARRAY,
		channels_for_tx, NUM_CHANNELS_KOMONDOR);

	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Transmission is possible in range: %d - %d\n",
		SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL3, current_left_channel, current_right_channel);

	int previous_num_channels = num_channels_tx;
	num_channels_tx = current_right_channel - current_left_channel + 1;
	++node_stats.num_trials_tx_per_num_channels[(int)log2(num_channels_tx)];
	int ix_num_channels_used (log2(num_channels_tx));

	if(previous_num_channels != num_channels_tx) sr_state.flag_change_in_tx_power = TRUE;

	// Get the current modulation according to the channels selected for transmission
	//int ix_mcs_per_node (current_destination_id - wlan.list_sta_id[0]);
	current_modulation = mcs_per_node[ix_mcs_per_node][ix_num_channels_used];

	// ********************************************************
	// Flexible packet aggregation

	// Coordinator ICF: clear stale per-AP budget from previous coordinated role
	if (wlan.mapc_enabled && coordinator_ap_id == NODE_ID_NONE
			&& exchange_sequence.frame_types[0] == PACKET_TYPE_ICF) {
		mapc_txop_per_ap_data_duration = 0.0;
		mapc_icr_requested_duration    = 0.0;
	}

	//  - Delete / flush packets from the previous transmission
	if (wlan.mapc_enabled && coordinator_ap_id == NODE_ID_NONE
			&& node_params.traffic_model == TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION
			&& exchange_sequence.frame_types[0] == PACKET_TYPE_ICF) {
		// MAPC coordinator + full-buffer: flush all stale packets before pre-filling
		while (buffer.QueueSize() > 0) {
			buffer.DelFirstPacket();
		}
	} else if (limited_num_packets_aggregated > 0
			&& (exchange_sequence.frame_types[0] != PACKET_TYPE_ICF
				|| (wlan.mapc_enabled && coordinator_ap_id == NODE_ID_NONE))) {
		// Standard path OR MAPC coordinator at ICF time (non-full-buffer traffic):
		// delete the ACKed frames from the previous TXOP.
		// Note: for the coordinator the deletion happens here (ICF time) because
		// limited_num_packets_aggregated is reset to 0 before the DATA call.
		for(int i = 0; i < limited_num_packets_aggregated; ++i){
			buffer.DelFirstPacket();
		}
	}
	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Data packet/s removed from buffer (queue: %d/%d).\n",
		SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
		buffer.QueueSize(), PACKET_BUFFER_SIZE);

	// - Number of packets to be aggregated: min(current buffer size, max num packets aggregated)
	if(buffer.QueueSize() > node_params.max_num_packets_aggregated || node_params.traffic_model == TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION){
		current_num_packets_aggregated = node_params.max_num_packets_aggregated;
	} else {
		current_num_packets_aggregated = buffer.QueueSize();
	}

	// data rate depending on CB and streams: Nsc * ym * yc * SUSS
	bits_ofdm_sym =  GetNumberSubcarriers(num_channels_tx) *
		Mcs_array::modulation_bits[current_modulation-1] *
		Mcs_array::coding_rates[current_modulation-1] *
		IEEE_AX_SU_SPATIAL_STREAMS;

	// Update the number of packets to aggregate (just in case that the max PPDU is exceeded with the current MCS)
	limited_num_packets_aggregated = FindMaximumPacketsAggregated
		(current_num_packets_aggregated, node_params.frame_length, bits_ofdm_sym);

	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Num. of packets to aggregate: %d/%d (last_transmission_successful=%d)\n",
		SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL4,
		limited_num_packets_aggregated, node_params.max_num_packets_aggregated, last_transmission_successful);

	// Guard: abort if the buffer is empty for non-full-buffer traffic (e.g., Poisson).
	// Transmitting 0 packets wastes the TXOP and produces invalid frame durations.
	if (limited_num_packets_aggregated == 0
			&& node_params.traffic_model != TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION) {
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s Buffer empty; aborting TX and restarting (no CW increase).\n",
			SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3);
		RestartNode(FALSE);
		return;
	}

	// TODO: ADD TRANSMISSION DELAY

	// ********************************************************
	// Buffer fill timing:
	//   MAPC coordinator + full-buffer model (99): fill at ICF time (before TXOP announcement)
	//   All other cases: fill at DATA preparation time (non-ICF path)
	if (wlan.mapc_enabled && coordinator_ap_id == NODE_ID_NONE
			&& node_params.traffic_model == TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION) {
		if (exchange_sequence.frame_types[0] == PACKET_TYPE_ICF) {
			InitiateBurstPackets(); // Pre-fill before ICF so lnpa is known at TXOP start
		}
		// For MAPC coordinator DATA path: buffer was pre-filled at ICF time; skip
	} else {
		// Standard, coordinated AP, or non-full-buffer MAPC: fill before DATA
		if (exchange_sequence.frame_types[0] != PACKET_TYPE_ICF) {
			InitiateBurstPackets();
		}
	}
	// ********************************************************

	// Compute all packets durations (RTS, CTS, DATA and ACK) and NAV time
	ComputeFramesDuration(&rts_duration, &cts_duration, &data_duration, &ack_duration,
		num_channels_tx, current_modulation, limited_num_packets_aggregated, node_params.frame_length, bits_ofdm_sym);

	// TXOP split: save budget at ICF time; cap DATA duration to per-AP allocation
	if (wlan.mapc_enabled && coordinator_ap_id == NODE_ID_NONE
			&& exchange_sequence.frame_types[0] == PACKET_TYPE_ICF) {
		// Coordinator ICF path: record full data_duration as TXOP budget
		mapc_txop_data_budget = data_duration;
	} else if (wlan.mapc_enabled && wlan.mapc_txop_splits[mapc_active_group_idx] == TXOP_SPLIT_EQUAL
			&& mapc_txop_per_ap_data_duration > 0.0
			&& exchange_sequence.frame_types[0] != PACKET_TYPE_ICF) {
		// DATA path (coordinator or coordinated AP): reduce lnpa to fit within budget
		while (limited_num_packets_aggregated > 1 && data_duration > mapc_txop_per_ap_data_duration) {
			--limited_num_packets_aggregated;
			ComputeFramesDuration(&rts_duration, &cts_duration, &data_duration, &ack_duration,
				num_channels_tx, current_modulation, limited_num_packets_aggregated, node_params.frame_length, bits_ofdm_sym);
		}
		// Co-BF/Co-SR: if AP has fewer packets than the TXOP slot, pad data_duration
		// to the allocated value so both APs' DATA frames end at the same time.
		// Co-TDMA is excluded (sequential slots; each AP uses its natural duration).
		if (wlan.mapc_method_ids[mapc_active_group_idx] != CO_TDMA
				&& data_duration < mapc_txop_per_ap_data_duration) {
			data_duration = mapc_txop_per_ap_data_duration;
		}
	}

	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Transmitting (N_agg = %d) in %d channels using modulation %d (%.0f bits per OFDM symbol ---> %.2f Mbps) \n",
		SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL4, limited_num_packets_aggregated,
		(int) pow(2, ix_num_channels_used), current_modulation, bits_ofdm_sym,
		bits_ofdm_sym/IEEE_AX_OFDM_SYMBOL_GI32_DURATION * pow(10,-6));

	if(sr_state.mapc_cosr_active || (sr_state.spatial_reuse_enabled && sr_state.txop_sr_identified)) {
		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Using tx power = %f dBm \n",
			SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL4,
			ConvertPower(PW_TO_DBM, sr_state.current_tx_power_sr));
	} else {
		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Using tx power = %f dBm \n",
			SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL4,
			ConvertPower(PW_TO_DBM, current_tx_power));
	}


	// --- Per-TXOP beamforming: update beam direction and null directions ---
	if (node_params.beamforming_enabled) {
		double _bf_dx, _bf_dy;
		// Main beam: steer toward current destination
		_bf_dx = all_node_x[current_destination_id] - node_params.x;
		_bf_dy = all_node_y[current_destination_id] - node_params.y;
		current_beam_az_main_rad = atan2(_bf_dy, _bf_dx);
		current_beam_num_nulls = 0;

		if (wlan.mapc_enabled && coordinator_ap_id == NODE_ID_NONE) {
			// MAPC coordinator: null toward each peer AP in the active group
			int _g = mapc_active_group_idx;
			for (int _p = 0; _p < wlan.mapc_num_peers[_g]
					&& current_beam_num_nulls < MAX_BEAM_NULLS; ++_p) {
				int _peer = wlan.mapc_peer_ap_ids[_g][_p];
				_bf_dx = all_node_x[_peer] - node_params.x;
				_bf_dy = all_node_y[_peer] - node_params.y;
				current_beam_null_az_rad[current_beam_num_nulls++] = atan2(_bf_dy, _bf_dx);
			}
		} else if (wlan.mapc_enabled && coordinator_ap_id != NODE_ID_NONE) {
			// MAPC coordinated AP: single null toward coordinator
			_bf_dx = all_node_x[coordinator_ap_id] - node_params.x;
			_bf_dy = all_node_y[coordinator_ap_id] - node_params.y;
			current_beam_null_az_rad[0] = atan2(_bf_dy, _bf_dx);
			current_beam_num_nulls = 1;
		}
	}

	if (exchange_sequence.frame_types[0] == PACKET_TYPE_RTS) {

		// 4-way handshake: start with RTS
		node_state = STATE_TX_RTS;
		current_tx_duration = rts_duration;

		// Compute the NAV time
		current_nav_time = ComputeNavTime(node_state, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
		current_nav_time = FixTimeOffset(current_nav_time,13,12); // Update the NAV time according to the time offsets

		/*
		 * IMPORTANT: to avoid synchronization problems in Slotted BO, we put a
		 * random time epsilon trigger before sending the channel occupancy notification.
		 * E.g. If two APs A and B finish their BO at the very same time, both A and B will
		 * find the channel free and will pick the corresponding channels accordingly. This
		 * way we are able to capture slotted BO collisions.
		 */
		time_rand_value = 0;
		int rand_number_rts (2 + rand() % (MAX_NUM_RAND_TIME-2));	// in [2, MAX_NUM_RAND_TIME]
		time_rand_value = (double) rand_number_rts * MAX_DIFFERENCE_SAME_TIME/MAX_NUM_RAND_TIME; // in [FEMTO_SECOND, MAX_DIFFERENCE_SAME_TIME]
		// Sergio on 28/09/2017
		// time_rand_value = RoundToDigits(time_rand_value, 15);
		time_rand_value = FixTimeOffset(time_rand_value,13,12);
		current_nav_time = current_nav_time - time_rand_value;

		// Generate the RTS notification
		Notification first_packet_buffer = buffer.GetFirstPacket();

		rts_notification = GenerateNotification(PACKET_TYPE_RTS, current_destination_id,
			first_packet_buffer.packet_id, limited_num_packets_aggregated,
			first_packet_buffer.timestamp_generated, current_tx_duration);

		// Reset the flag that indicates whether the tx power changed or not
		sr_state.flag_change_in_tx_power = FALSE;

		ScheduleTransmission(PACKET_TYPE_RTS);

	} else if (exchange_sequence.frame_types[0] == PACKET_TYPE_ICF) {

		// MAPC exchange (Co-TDMA or Co-BF/Co-SR): start with ICF
		// ICF duration is approximated by rts_duration (both are short control frames)
		node_state = STATE_TX_ICF;
		++node_stats.num_icf_tx;
		current_tx_duration = rts_duration;

		// NAV covers the full MAPC exchange; computed by ComputeNavTime STATE_TX_ICF case
		current_nav_time = ComputeNavTime(STATE_TX_ICF, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
		current_nav_time = FixTimeOffset(current_nav_time,13,12);

		time_rand_value = 0;
		int rand_number_icf (2 + rand() % (MAX_NUM_RAND_TIME-2));
		time_rand_value = (double) rand_number_icf * MAX_DIFFERENCE_SAME_TIME/MAX_NUM_RAND_TIME;
		time_rand_value = FixTimeOffset(time_rand_value,13,12);
		current_nav_time = current_nav_time - time_rand_value;

		// Generate the ICF notification
		// ICF is a control frame; data buffer is managed at DATA preparation phase
		icf_notification = GenerateNotification(PACKET_TYPE_ICF, current_destination_id,
			last_packet_generated_id, limited_num_packets_aggregated,
			SimTime(), current_tx_duration);

		// Broadcast ICF to the MAPC group (all nodes update NAV)
		icf_notification.destination_id = NODE_ID_MAPC_BROADCAST;
		icf_notification.mapc_group_id  = wlan.mapc_group_ids[mapc_active_group_idx];
		// Tag which coordinated AP should respond with ICR (round-robin)
		icf_notification.mapc_target_ap_id = mapc_selected_peer_id;

		// Reset the flag that indicates whether the tx power changed or not
		sr_state.flag_change_in_tx_power = FALSE;

		ScheduleTransmission(PACKET_TYPE_ICF);

	} else {

		// 2-way handshake: start with DATA directly
		node_state = STATE_TX_DATA;
		current_tx_duration = data_duration;

		// Compute the NAV time
		current_nav_time = ComputeNavTime(node_state, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
		current_nav_time = FixTimeOffset(current_nav_time,13,12);

		time_rand_value = 0;
		// Co-SR simultaneous DATA: suppress random jitter so both APs fire at the exact
		// same simulation time. The random offset exists only to break backoff ties, which
		// is not relevant here since neither AP is contending — both start immediately.
		if (!(wlan.mapc_enabled && sr_state.mapc_cosr_active)) {
			int rand_number_data (2 + rand() % (MAX_NUM_RAND_TIME-2));
			time_rand_value = (double) rand_number_data * MAX_DIFFERENCE_SAME_TIME/MAX_NUM_RAND_TIME;
			time_rand_value = FixTimeOffset(time_rand_value,13,12);
		}
		current_nav_time = current_nav_time - time_rand_value;

		// Generate the DATA notification
		Notification first_packet_buffer_data = buffer.GetFirstPacket();

		data_notification = GenerateNotification(PACKET_TYPE_DATA, current_destination_id,
			first_packet_buffer_data.packet_id, limited_num_packets_aggregated,
			first_packet_buffer_data.timestamp_generated, current_tx_duration);

		// Reset the flag that indicates whether the tx power changed or not
		sr_state.flag_change_in_tx_power = FALSE;

		ScheduleTransmission(PACKET_TYPE_DATA);
	}
}

/**
 * Called when backoff finishes (triggered-based operation)
 */
void Node :: EndBackoff(trigger_t &){

	LOGS(node_params.save_node_logs,node_logger.file, "\n----------------------------------------------------------\n");
	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s EndBackoff()\n",
			SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL1);

	// Spatial Reuse: apply SR TX power/PD parameters if an SR TXOP was identified
	ApplySRParametersAtBackoffEnd();

	// Select MAPC or standard frame exchange sequence
	if (wlan.mapc_enabled) {
		// Round-robin over groups: select which group to coordinate this TXOP
		mapc_active_group_idx = mapc_coordinator_group_rr % wlan.num_mapc_groups;
		++mapc_coordinator_group_rr;
		if (wlan.mapc_method_ids[mapc_active_group_idx] == CO_TDMA) {
			exchange_sequence = IEEE_802_11_COTDMA;
		} else if (wlan.mapc_method_ids[mapc_active_group_idx] == CO_BF
				|| wlan.mapc_method_ids[mapc_active_group_idx] == CO_SR) {
			exchange_sequence = IEEE_802_11_COBF_COSR;
		} else {
			printf("ERROR: MAPC method not found.\n");
		}
		// Declare this node as coordinator for the upcoming TXOP
		coordinator_ap_id = NODE_ID_NONE;
		num_coordinated_aps = wlan.mapc_num_peers[mapc_active_group_idx];
		for (int i = 0; i < num_coordinated_aps; ++i)
			coordinated_ap_ids[i] = wlan.mapc_peer_ap_ids[mapc_active_group_idx][i];
		// Round-robin: select which coordinated AP to invite this TXOP
		mapc_selected_peer_id = coordinated_ap_ids[mapc_current_peer_idx % num_coordinated_aps];
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s MAPC: selected peer N%d (round-robin idx %d)\n",
			SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL2,
			mapc_selected_peer_id, mapc_current_peer_idx % num_coordinated_aps);
	} else {
		exchange_sequence = node_params.rts_cts_enabled ? IEEE_802_11_RTS_CTS : IEEE_802_11_NO_RTS_CTS;
	}
	mapc_seq_pos = 0;
	mapc_icr_received_count = 0;

	// Sergio on 26th June 2018:
	// - Compute average BO waiting time
	node_stats.sum_waiting_time = node_stats.sum_waiting_time + SimTime() - node_stats.timestamp_new_trial_started;
	++node_stats.num_average_waiting_time_measurements;

	// Update the performance_report
	performance_report.sum_waiting_time += SimTime() - node_stats.timestamp_new_trial_started;
	++performance_report.num_waiting_time_measurements;
	if (SimTime() - node_stats.timestamp_new_trial_started > performance_report.max_waiting_time) {
		performance_report.max_waiting_time = SimTime() - node_stats.timestamp_new_trial_started;
	}
	if (SimTime() - node_stats.timestamp_new_trial_started < performance_report.min_waiting_time) {
		performance_report.min_waiting_time = SimTime() - node_stats.timestamp_new_trial_started;
	}

	// Measurements in the last part of the simulation
	if (SimTime() > (node_params.simulation_time_komondor - node_stats.last_measurements_window)) {
		node_stats.last_sum_waiting_time = node_stats.last_sum_waiting_time + SimTime() - node_stats.timestamp_new_trial_started;
		++node_stats.last_num_average_waiting_time_measurements;
	}

	// Cancel NAV TIMEOUT trigger
	trigger_NAV_timeout.Cancel();

	// Cancel RECOVER CTS trigger for safety
	trigger_recover_cts_timeout.Cancel();

	// Check if MCS has been already defined for every potential receiver
	for(int n = 0; n < wlan.num_stas; ++n) {
		current_destination_id = wlan.list_sta_id[n];
		// Receive the possible MCS to be used for each number of channels
		if (change_modulation_flag[n]) {
			LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Requesting MCS to N%d\n",
				SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL2, current_destination_id);
			RequestMCS();
		}
	}

	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Allowed LEFT/RIGHT: %d - %d\n",
		SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL2, node_params.min_channel_allowed, node_params.max_channel_allowed);

	// Pick one receiver from the pool of potential receivers
	SelectDestination();

	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Trying to start TX to STA N%d\n",
		SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL2, current_destination_id);

	// Identify free channels
	++node_stats.num_tx_init_tried;

	if (sr_state.spatial_reuse_enabled && sr_state.txop_sr_identified) {
		GetChannelOccupancyByCCA(node_params.current_primary_channel, node_params.pifs_activated, channels_free, node_params.min_channel_allowed,
			node_params.max_channel_allowed, &channel_power, sr_state.current_obss_pd_threshold, timestamp_channel_becomes_free, SimTime(), PIFS);
	} else {
		GetChannelOccupancyByCCA(node_params.current_primary_channel, node_params.pifs_activated, channels_free, node_params.min_channel_allowed,
			node_params.max_channel_allowed, &channel_power, current_pd, timestamp_channel_becomes_free, SimTime(), PIFS);
	}

	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Power sensed per channel [dBm]: ",
		SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3);

	PrintOrWriteChannelPower(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
		&channel_power);

	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Channels founds free (mind PIFS if activated): ",
		SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL3);

	PrintOrWriteChannelsFree(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
		channels_free);

	GetTxChannels(channels_for_tx, node_params.current_dcb_policy, channels_free,
			node_params.min_channel_allowed, node_params.max_channel_allowed, node_params.current_primary_channel,
			NUM_CHANNELS_KOMONDOR, &channel_power, channel_aggregation_cca_model);

	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Channels for transmitting: ",
		SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL2);

	PrintOrWriteChannelForTx(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
		channels_for_tx);

	// Act according to possible (not possible) transmission
	if(channels_for_tx[0] != TX_NOT_POSSIBLE) {
		//&& current_modulation != MODULATION_FORBIDDEN){	// Transmission IS POSSIBLE
		PrepareNewTransmission();
	} else {	// Transmission IS NOT POSSIBLE, compute a new backoff.
		AbortInitialTransmission();
	}
	// LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s EndBackoff() END\n", SimTime(), node_params.node_id, node_state, LOG_F01, LOG_LVL1);
};

/**
 * Called when own transmission is finished (triggered-based operation)
 */
void Node :: MyTxFinished(trigger_t &){

//	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s MyTxFinished()\n",
//			SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL1);

	switch(node_state){

		case STATE_TX_RTS:{

			if (wlan.mapc_enabled && wlan.mapc_method_ids[mapc_active_group_idx] == CO_TDMA) {
				// MAPC Co-TDMA: MU-RTS/TXS finished — advance round-robin and restart coordinator
				Notification notification = GenerateNotification(PACKET_TYPE_MU_RTS_TXS, current_destination_id,
					mu_rts_notification.packet_id, mu_rts_notification.tx_info.num_packets_aggregated,
					mu_rts_notification.timestamp_generated, TX_DURATION_NONE);
				// Propagate per-AP allocation so HandleFinishTX_StateRxMuRts can cap AP_B's DATA
				notification.tx_info.mapc_allocated_data_duration =
					mu_rts_notification.tx_info.mapc_allocated_data_duration;
				outportSelfFinishTX(notification);
				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s MU-RTS/TXS #%d tx finished. Advancing round-robin and restarting.\n",
					SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2,
					notification.packet_id);
				++mapc_current_peer_idx;
			++node_stats.num_mu_rts_tx;
				RestartNode(FALSE);
			} else {
				// Normal RTS: wait for CTS
				// Set CTS timeout and change state to STATE_WAIT_CTS
				Notification notification = GenerateNotification(PACKET_TYPE_RTS, current_destination_id,
					rts_notification.packet_id, limited_num_packets_aggregated,
					rts_notification.timestamp_generated, TX_DURATION_NONE);

				outportSelfFinishTX(notification);

				// Time out = T_c = T_RTS + SIFS + T_CTS minus T_RTS (already txed)
				time_to_trigger = SimTime() + SIFS + notification.tx_info.cts_duration;
				trigger_CTS_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
				node_state = STATE_WAIT_CTS;

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s RTS #%d tx finished. Waiting for CTS until %.12f\n",
					SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2,
					notification.packet_id, trigger_CTS_timeout.GetTime());
			}

			break;
		}

		case STATE_TX_CTS:{		// Wait for Data

			Notification notification = GenerateNotification(PACKET_TYPE_CTS, current_destination_id,
				cts_notification.packet_id, cts_notification.tx_info.num_packets_aggregated,
				cts_notification.timestamp_generated, TX_DURATION_NONE);

			outportSelfFinishTX(notification);

			// Set CTS timeout and change state to STATE_WAIT_DATA
			time_to_trigger = SimTime() + SIFS + TIME_OUT_EXTRA_TIME;
			trigger_DATA_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
			node_state = STATE_WAIT_DATA;

			LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s CTS %d tx finished. Waiting for DATA...\n",
				SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2, notification.packet_id);

			break;
		}

		case STATE_TX_DATA:{ 	// Change state to STATE_WAIT_ACK (or ACK TF phase for Co-BF/Co-SR)

			Notification notification = GenerateNotification(PACKET_TYPE_DATA, current_destination_id,
				data_notification.packet_id, data_notification.tx_info.num_packets_aggregated,
				data_notification.timestamp_generated, TX_DURATION_NONE);

			// Co-BF/Co-SR: tag DATA finish notification with MAPC group so STAs know
			// to wait for the ACK TF instead of auto-ACKing after SIFS.
			int cobf_cosr = wlan.mapc_enabled
				&& (wlan.mapc_method_ids[mapc_active_group_idx] == CO_BF
					|| wlan.mapc_method_ids[mapc_active_group_idx] == CO_SR);
			if (cobf_cosr)
				notification.mapc_group_id = wlan.mapc_group_ids[mapc_active_group_idx];

			outportSelfFinishTX(notification);

			if (cobf_cosr && coordinator_ap_id == NODE_ID_NONE) {
				// COORDINATOR: send ACK TF after SIFS to trigger simultaneous ACKs from both STAs
				current_tx_duration = rts_duration;  // ACK TF is a short control frame
				ack_tf_notification = GenerateNotification(PACKET_TYPE_ACK_TF, NODE_ID_MAPC_BROADCAST,
					data_notification.packet_id, 0, SimTime(), current_tx_duration);
				ack_tf_notification.mapc_group_id = wlan.mapc_group_ids[mapc_active_group_idx];
				node_state = STATE_TX_ACK_TF;
				time_to_trigger = SimTime() + SIFS;
				trigger_SIFS.Set(FixTimeOffset(time_to_trigger, 13, 12));
				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s DATA %d tx done (Co-BF/SR coordinator). Sending ACK TF after SIFS.\n",
					SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2, notification.packet_id);
			} else if (cobf_cosr && coordinator_ap_id != NODE_ID_NONE) {
				// COORDINATED AP: wait for coordinator's ACK TF
				node_state = STATE_WAIT_ACK_TF;
				time_to_trigger = SimTime() + SIFS + rts_duration + SIFS + TIME_OUT_EXTRA_TIME;
				trigger_DATA_timeout.Set(FixTimeOffset(time_to_trigger, 13, 12));
				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s DATA %d tx done (Co-BF/SR coordinated). Waiting for ACK TF.\n",
					SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2, notification.packet_id);
			} else {
				// Normal / Co-TDMA: wait for ACK directly
				time_to_trigger = SimTime() + SIFS + TIME_OUT_EXTRA_TIME;
				trigger_ACK_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
				node_state = STATE_WAIT_ACK;
				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s DATA %d tx finished. Waiting for ACK...\n",
					SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2, notification.packet_id);
			}

			break;
		}

		case STATE_TX_ACK_TF:{		// ACK TF sent � wait for STA's ACK

			Notification notification = GenerateNotification(PACKET_TYPE_ACK_TF, NODE_ID_MAPC_BROADCAST,
				ack_tf_notification.packet_id, 0,
				ack_tf_notification.timestamp_generated, TX_DURATION_NONE);
			notification.mapc_group_id = wlan.mapc_group_ids[mapc_active_group_idx];
			outportSelfFinishTX(notification);

			// Now wait for ACK from own STA
			time_to_trigger = SimTime() + SIFS + TIME_OUT_EXTRA_TIME;
			trigger_ACK_timeout.Set(FixTimeOffset(time_to_trigger, 13, 12));
			node_state = STATE_WAIT_ACK;

			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s ACK TF #%d tx finished. Waiting for ACK from N%d.\n",
				SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2,
				notification.packet_id, current_destination_id);

			break;
		}

		case STATE_TX_ACK:{		// Restart node

			Notification notification = GenerateNotification(PACKET_TYPE_ACK, current_destination_id,
				ack_notification.packet_id, ack_notification.tx_info.num_packets_aggregated,
				ack_notification.timestamp_generated, TX_DURATION_NONE);

			outportSelfFinishTX(notification);

			LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s ACK %d tx finished. Restarting node...\n",
				SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2, notification.packet_id);

			RestartNode(FALSE);

			break;
		}

		case STATE_TX_ICF:{		// Wait for ICR (MAPC response to ICF, analogous to CTS after RTS)

			// Use NODE_ID_MAPC_BROADCAST so HandleFinishTX_StateRxIcf guard passes at coordinated APs
			Notification notification = GenerateNotification(PACKET_TYPE_ICF, NODE_ID_MAPC_BROADCAST,
				icf_notification.packet_id, limited_num_packets_aggregated,
				icf_notification.timestamp_generated, TX_DURATION_NONE);
			notification.mapc_group_id = wlan.mapc_group_ids[mapc_active_group_idx];
			notification.mapc_target_ap_id = icf_notification.mapc_target_ap_id;

			outportSelfFinishTX(notification);

			// ICR timeout reuses trigger_CTS_timeout (same semantic role: waiting for control response)
			time_to_trigger = SimTime() + SIFS + notification.tx_info.cts_duration;
			trigger_CTS_timeout.Set(FixTimeOffset(time_to_trigger,13,12));

			node_state = STATE_WAIT_ICR;

			LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s ICF #%d tx finished. Waiting for ICR until %.12f\n",
				SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2,
				notification.packet_id, trigger_CTS_timeout.GetTime());

			break;
		}

		case STATE_TX_ICR:{		// ICR sent — wait for next MAPC frame (MU-RTS/TXS or TF)
			outportSelfFinishTX(icr_notification);
			int icr_rejected = FALSE;
			if (wlan.mapc_method_ids[mapc_active_group_idx] == CO_TDMA) {
				if (icr_notification.mapc_has_data) {
					// Has data: wait for MU-RTS/TXS after coordinator's DATA/ACK exchange
					node_state = STATE_WAIT_MU_RTS;
					time_to_trigger = SimTime() + SIFS + data_duration + SIFS + ack_duration
									+ SIFS + TIME_OUT_EXTRA_TIME;
					trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger, 13, 12));
				} else {
					// ICR rejection (no data): coordinator will continue TXOP solo.
					// Channel will be busy during coordinator's DATA slot, so our backoff freezes naturally.
					icr_rejected = TRUE;
					LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s ICR sent as rejection (no data). Coordinator continues solo; returning to SENSING.\n",
						SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2);
					coordinator_ap_id = NODE_ID_NONE;
					RestartNode(TRUE);
				}
			} else {
				// Co-BF / Co-SR
				if (icr_notification.mapc_has_data) {
					// Has data: wait for TF from coordinator
					node_state = STATE_WAIT_TF;
					time_to_trigger = SimTime() + SIFS + TIME_OUT_EXTRA_TIME;
					trigger_DATA_timeout.Set(FixTimeOffset(time_to_trigger, 13, 12));
				} else {
					// ICR rejection (no data): coordinator will transmit solo; return to SENSING.
					icr_rejected = TRUE;
					LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s ICR sent as rejection (no data). Coordinator continues solo; returning to SENSING.\n",
						SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2);
					coordinator_ap_id = NODE_ID_NONE;
					RestartNode(TRUE);
				}
			}
			if (!icr_rejected) {
				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s ICR tx finished; waiting for %s\n",
					SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2,
					(wlan.mapc_method_ids[mapc_active_group_idx] == CO_TDMA) ? "MU-RTS/TXS" : "TF");
			}
			break;
		}

		case STATE_TX_TF:{		// TF sent — coordinator starts DATA simultaneously (Co-BF/Co-SR)

			// Use NODE_ID_MAPC_BROADCAST to match the TF start notification (broadcast frame)
			Notification notification = GenerateNotification(PACKET_TYPE_TF, NODE_ID_MAPC_BROADCAST,
				tf_notification.packet_id, tf_notification.tx_info.num_packets_aggregated,
				tf_notification.timestamp_generated, TX_DURATION_NONE);
			notification.mapc_group_id                          = wlan.mapc_group_ids[mapc_active_group_idx];
			notification.tx_info.mapc_sr_peer_tx_power          = tf_notification.tx_info.mapc_sr_peer_tx_power;
			notification.tx_info.mapc_allocated_data_duration   = tf_notification.tx_info.mapc_allocated_data_duration;

			outportSelfFinishTX(notification);

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s TF #%d tx finished. Coordinator starts DATA simultaneously.\n",
				SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2, notification.packet_id);

			// Coordinator also transmits DATA immediately after SIFS (simultaneous with coordinated AP)
			exchange_sequence = IEEE_802_11_NO_RTS_CTS;
			SelectDestination();
			// No packets were ACKed during the ICF/ICR/TF exchange; reset so
			// PrepareNewTransmission does not attempt to delete from the empty buffer
			limited_num_packets_aggregated = 0;
			PrepareNewTransmission();

			break;
		}

		default:
			break;
	}

    // Reset the flag that indicates whether the tx power changed or not
    sr_state.flag_change_in_tx_power = FALSE;

	// LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s  MyTxFinished()\n", SimTime(), node_params.node_id, node_state, LOG_G01, LOG_LVL1);
};

/**
 * HandleFinishTX_StateRxIcf: coordinated AP responds with ICR after receiving ICF
 */
void Node :: HandleFinishTX_StateRxIcf(const Notification &notification) {
	if (notification.packet_type != PACKET_TYPE_ICF
			|| notification.destination_id != NODE_ID_MAPC_BROADCAST
			|| notification.mapc_group_id != wlan.mapc_group_ids[mapc_active_group_idx]) return;
	// Only coordinated APs (not STAs) respond; only the targeted AP responds with ICR
	if (node_params.node_type != NODE_TYPE_AP) return;
	if (notification.mapc_target_ap_id != node_params.node_id) return;

	// CCA check: same pattern as RTS -> CTS
	GetChannelOccupancyByCCA(node_params.current_primary_channel, node_params.pifs_activated,
		channels_free, current_left_channel, current_right_channel,
		&channel_power, current_pd, timestamp_channel_becomes_free, SimTime(), PIFS);

	GetTxChannels(channels_for_tx, node_params.current_dcb_policy, channels_free,
		current_left_channel, current_right_channel, node_params.current_primary_channel,
		NUM_CHANNELS_KOMONDOR, &channel_power, channel_aggregation_cca_model);

	if (channels_for_tx[0] == TX_NOT_POSSIBLE) {
		RestartNode(TRUE);
		return;
	}

	current_left_channel  = GetFirstOrLastTrueElemOfArray(FIRST_TRUE_IN_ARRAY,
		channels_for_tx, NUM_CHANNELS_KOMONDOR);
	current_right_channel = GetFirstOrLastTrueElemOfArray(LAST_TRUE_IN_ARRAY,
		channels_for_tx, NUM_CHANNELS_KOMONDOR);

	current_destination_id = coordinator_ap_id;
	node_state             = STATE_TX_ICR;
	++node_stats.num_icr_tx;
	current_tx_duration    = cts_duration;

	icr_notification = GenerateNotification(PACKET_TYPE_ICR, coordinator_ap_id,
		notification.packet_id, notification.tx_info.num_packets_aggregated,
		notification.timestamp_generated, current_tx_duration);
	icr_notification.mapc_group_id = wlan.mapc_group_ids[mapc_active_group_idx];
	icr_notification.tx_duration = current_tx_duration;
	// Report data availability to coordinator so it can skip the MU-RTS slot if needed
	icr_notification.mapc_has_data = (node_params.traffic_model == TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION
		|| buffer.QueueSize() > 0) ? TRUE : FALSE;

	// Co-SR/BF: embed this AP's desired DATA duration so coordinator can negotiate aligned TXOP.
	// Use mcs_per_node[0] (first STA); report 0 if MCS is not yet established.
	if (wlan.mapc_method_ids[mapc_active_group_idx] != CO_TDMA) {
		int _ix_ch = (int)log2((double)(current_right_channel - current_left_channel + 1));
		int _mod   = mcs_per_node[0][_ix_ch];
		if (_mod > 0) {
			double _bs = (double)GetNumberSubcarriers(current_right_channel - current_left_channel + 1)
				* Mcs_array::modulation_bits[_mod - 1]
				* Mcs_array::coding_rates[_mod - 1]
				* (double)IEEE_AX_SU_SPATIAL_STREAMS;
			int _lnpa = (node_params.traffic_model == TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION)
				? node_params.max_num_packets_aggregated
				: ((buffer.QueueSize() < node_params.max_num_packets_aggregated)
				   ? buffer.QueueSize() : node_params.max_num_packets_aggregated);
			_lnpa = FindMaximumPacketsAggregated(_lnpa, node_params.frame_length, _bs);
			if (_lnpa > 0) {
				double _req_data = 0.0, _dmy1 = 0.0, _dmy2 = 0.0, _dmy3 = 0.0;
				ComputeFramesDuration(&_dmy1, &_dmy2, &_req_data, &_dmy3,
					current_right_channel - current_left_channel + 1,
					_mod, _lnpa, node_params.frame_length, (int)_bs);
				icr_notification.tx_info.mapc_allocated_data_duration = _req_data;
			} else {
				icr_notification.tx_info.mapc_allocated_data_duration = 0.0;
			}
		} else {
			icr_notification.tx_info.mapc_allocated_data_duration = 0.0;
		}
	}

	sr_state.flag_change_in_tx_power = FALSE;

	// Stagger: position 0 → SIFS; position k → SIFS + k*(cts_duration + SIFS)
	double sifs_delay = SIFS + mapc_peer_position * (cts_duration + SIFS);
	time_to_trigger = SimTime() + sifs_delay;
	trigger_SIFS.Set(FixTimeOffset(time_to_trigger, 13, 12));

	LOGS(node_params.save_node_logs, node_logger.file,
		"%.15f;N%d;S%d;%s;%s ICF done, sending ICR after %.9f s (stagger pos %d)\n",
		SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
		sifs_delay, mapc_peer_position);
}

/**
 * HandleFinishTX_StateRxIcr: coordinator collects ICRs and proceeds when all received
 */
void Node :: HandleFinishTX_StateRxIcr(const Notification &notification) {
	if (notification.packet_type != PACKET_TYPE_ICR
			|| notification.destination_id != node_params.node_id) return;

	++mapc_icr_received_count;
	++node_stats.num_icr_rx;
	LOGS(node_params.save_node_logs, node_logger.file,
		"%.15f;N%d;S%d;%s;%s ICR received from N%d (%d/%d) has_data=%d\n",
		SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
		notification.source_id, mapc_icr_received_count, num_coordinated_aps,
		notification.mapc_has_data);

	// Co-TDMA targets exactly one peer per TXOP — proceed immediately after the first ICR
	trigger_CTS_timeout.Cancel();
	mapc_icr_received_count = 0;
	mapc_peer_has_data = notification.mapc_has_data;
	// Save coordinated AP's requested duration for TXOP negotiation in ProceedAfterIcr
	mapc_icr_requested_duration = notification.tx_info.mapc_allocated_data_duration;
	ProceedAfterIcr();
}

/**
 * ComputeCoSRTxPowers: compute TX power limits for the Co-SR simultaneous DATA phase.
 * Controlled by COSR_POWER_LIMIT_PEER_ONLY (list_of_macros.h):
 *   0 = CSV limit applied symmetrically to both coordinator and coordinated AP
 *   1 = CSV limit applied to coordinated AP (peer) only; coordinator keeps default TX power
 * OPTION B (future): replace body with ICR measurement-based computation using
 *   mapc_icr_rssi_measurements[] populated from ICR.tx_info.mapc_sr_measured_rssi.
 */
void Node :: ComputeCoSRTxPowers(double &coordinator_pW, double &peer_pW) {
	double limit_pW = ConvertPower(DBM_TO_PW, wlan.mapc_sr_tx_power_dbm[mapc_active_group_idx]);
	peer_pW = limit_pW;
#if COSR_POWER_LIMIT_PEER_ONLY == 0
	coordinator_pW = limit_pW;
#endif
}

/**
 * ProceedAfterIcr: coordinator decides next action after all ICRs collected
 */
void Node :: ProceedAfterIcr() {
	LOGS(node_params.save_node_logs, node_logger.file,
		"%.15f;N%d;S%d;%s;%s All ICRs collected. Proceeding with %s\n",
		SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL2,
		(wlan.mapc_method_ids[mapc_active_group_idx] == CO_TDMA) ? "Co-TDMA DATA" : "Co-BF/SR TF");

	if (wlan.mapc_method_ids[mapc_active_group_idx] == CO_TDMA) {
		// Coordinator sends its own DATA first (2-way: DATA/ACK)
		exchange_sequence = IEEE_802_11_NO_RTS_CTS;
		SelectDestination();
		// No packets were ACKed during the ICF/ICR exchange; reset so
		// PrepareNewTransmission does not attempt to delete from the empty buffer
		limited_num_packets_aggregated = 0;
		// Compute per-AP data duration based on TXOP split method
		if (wlan.mapc_txop_splits[mapc_active_group_idx] == TXOP_SPLIT_EQUAL) {
			int n_active = 1 + (mapc_peer_has_data ? 1 : 0);
			mapc_txop_per_ap_data_duration = mapc_txop_data_budget / n_active;
			// Cap to respect total Co-TDMA TXOP <= IEEE_AX_MAX_PPDU_DURATION.
			// Overhead for n_active>1 (ICF+SIFS+ICR+ACK_A+SIFS+SIFS+MU-RTS+ACK_B+SIFS+SIFS):
			//   = 2*rts_duration + cts_duration + 2*ack_duration + 4*SIFS
			// For n_active==1 (no MU-RTS slot): rts + cts + ack + 2*SIFS
			if (n_active > 1) {
				double exchange_overhead = 2.0 * rts_duration + cts_duration
					+ 2.0 * ack_duration + 4.0 * SIFS;
				double per_ap_txop_limit =
					(IEEE_AX_MAX_PPDU_DURATION - exchange_overhead) / (double) n_active;
				if (per_ap_txop_limit < 0.0) per_ap_txop_limit = 0.0;
				if (mapc_txop_per_ap_data_duration > per_ap_txop_limit)
					mapc_txop_per_ap_data_duration = per_ap_txop_limit;
			}
			// Enforce IEEE 802.11ax per-PPDU duration limit
			if (mapc_txop_per_ap_data_duration > IEEE_AX_MAX_PPDU_DURATION)
				mapc_txop_per_ap_data_duration = IEEE_AX_MAX_PPDU_DURATION;
		}
		// Track TXOP sharing durations (Co-TDMA)
		node_stats.total_mapc_data_duration_own += mapc_txop_per_ap_data_duration;
		if (mapc_peer_has_data)
			node_stats.total_mapc_data_duration_shared += mapc_txop_per_ap_data_duration;
		PrepareNewTransmission();
	} else {
		// Co-BF / Co-SR
		if (!mapc_peer_has_data) {
			// Peer has no data: skip TF, coordinator transmits DATA alone
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s ICR: peer N%d has no data; skipping TF, transmitting solo\n",
				SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL3,
				mapc_selected_peer_id);
			exchange_sequence = IEEE_802_11_NO_RTS_CTS;
			SelectDestination();
			limited_num_packets_aggregated = 0;
			mapc_txop_per_ap_data_duration = 0.0;
			node_stats.total_mapc_data_duration_own += data_duration;
			PrepareNewTransmission();
		} else {
			// Peer has data: send TF to trigger simultaneous DATA

			// Co-SR only: compute power limits for simultaneous phase
			double cosr_coord_pW = current_tx_power;  // default: no restriction
			double cosr_peer_pW  = current_tx_power;
			if (wlan.mapc_method_ids[mapc_active_group_idx] == CO_SR) {
				ComputeCoSRTxPowers(cosr_coord_pW, cosr_peer_pW);
				sr_state.current_tx_power_sr = cosr_coord_pW;
				sr_state.mapc_cosr_active    = TRUE;
				for (int n = 0; n < wlan.num_stas; ++n) change_modulation_flag[n] = TRUE;
			}

			// Negotiate aligned TXOP duration: max(coordinator's need, peer's request), capped by MAX_PPDU
			double mapc_agreed_data_duration = (mapc_icr_requested_duration > data_duration)
				? mapc_icr_requested_duration : data_duration;
			if (mapc_agreed_data_duration > IEEE_AX_MAX_PPDU_DURATION)
				mapc_agreed_data_duration = IEEE_AX_MAX_PPDU_DURATION;
			// Both coordinator and coordinated AP will cap/pad to this agreed duration
			mapc_txop_per_ap_data_duration = mapc_agreed_data_duration;

			node_state = STATE_TX_TF;
			++node_stats.num_tf_tx;
			node_stats.total_mapc_data_duration_own    += data_duration;
			node_stats.total_mapc_data_duration_shared += data_duration;
			current_tx_duration = rts_duration;

			// TF is a control frame; data buffer is managed at DATA preparation phase
			tf_notification = GenerateNotification(PACKET_TYPE_TF, NODE_ID_MAPC_BROADCAST,
				last_packet_generated_id, limited_num_packets_aggregated,
				SimTime(), current_tx_duration);
			tf_notification.mapc_group_id                       = wlan.mapc_group_ids[mapc_active_group_idx];
			tf_notification.tx_info.mapc_sr_peer_tx_power       = cosr_peer_pW;
			// Carry agreed TXOP duration so both APs align start and end
			tf_notification.tx_info.mapc_allocated_data_duration = mapc_agreed_data_duration;

			sr_state.flag_change_in_tx_power = FALSE;
			ScheduleTransmission(PACKET_TYPE_TF);
		}
	}
}

/**
 * HandleFinishTX_StateRxMuRts: coordinated AP (Co-TDMA) received MU-RTS/TXS, starts DATA
 */
void Node :: HandleFinishTX_StateRxMuRts(const Notification &notification) {
	if (notification.packet_type != PACKET_TYPE_MU_RTS_TXS
			|| notification.destination_id != node_params.node_id) return;

	LOGS(node_params.save_node_logs, node_logger.file,
		"%.15f;N%d;S%d;%s;%s MU-RTS/TXS done, starting DATA exchange\n",
		SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3);

	// Coordinated AP uses 2-way DATA/ACK for its TDMA slot
	exchange_sequence = IEEE_802_11_NO_RTS_CTS;
	SelectDestination();
	// AP_B never wins backoff independently, so EndBackoff's MCS-request loop
	// and GetTxChannels call are never executed for it.  Initialise both here.
	for (int ch = 0; ch < NUM_CHANNELS_KOMONDOR; ++ch)
		channels_for_tx[ch] = (ch == node_params.current_primary_channel) ? TRUE : FALSE;
	{
		int ix_dest (current_destination_id - wlan.list_sta_id[0]);
		if (change_modulation_flag[ix_dest]) {
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s MU-RTS: first TDMA slot, requesting MCS for N%d\n",
				SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
				current_destination_id);
			RequestMCS();
		}
	}
	// Apply per-AP duration allocation from coordinator's MU-RTS
	mapc_txop_per_ap_data_duration = notification.tx_info.mapc_allocated_data_duration;
	if (mapc_txop_per_ap_data_duration > IEEE_AX_MAX_PPDU_DURATION)
		mapc_txop_per_ap_data_duration = IEEE_AX_MAX_PPDU_DURATION;
	++node_stats.num_mu_rts_rx;
	node_stats.total_mapc_data_duration_allocated += mapc_txop_per_ap_data_duration;
	PrepareNewTransmission();
}

/**
 * HandleFinishTX_StateRxTf: coordinated AP (Co-BF/Co-SR) received TF, starts simultaneous DATA
 */
void Node :: HandleFinishTX_StateRxTf(const Notification &notification) {
	if (notification.packet_type != PACKET_TYPE_TF
			|| notification.mapc_group_id != wlan.mapc_group_ids[mapc_active_group_idx]) return;

	LOGS(node_params.save_node_logs, node_logger.file,
		"%.15f;N%d;S%d;%s;%s TF done, starting simultaneous DATA transmission\n",
		SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3);

	// Coordinated AP transmits DATA simultaneously with coordinator
	exchange_sequence = IEEE_802_11_NO_RTS_CTS;
	SelectDestination();
	// Same issue as Co-TDMA: coordinated AP never wins backoff independently.
	for (int ch = 0; ch < NUM_CHANNELS_KOMONDOR; ++ch)
		channels_for_tx[ch] = (ch == node_params.current_primary_channel) ? TRUE : FALSE;
	// Co-SR: apply the TX power limit assigned by the coordinator in the TF
	limited_num_packets_aggregated = 0;   // prevent stale-packet flush on first call
	if (wlan.mapc_method_ids[mapc_active_group_idx] == CO_SR) {
		sr_state.current_tx_power_sr = notification.tx_info.mapc_sr_peer_tx_power;
		sr_state.mapc_cosr_active    = TRUE;
		for (int n = 0; n < wlan.num_stas; ++n) change_modulation_flag[n] = TRUE;
	}
	{
		int ix_dest (current_destination_id - wlan.list_sta_id[0]);
		if (change_modulation_flag[ix_dest]) {
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s TF: first slot, requesting MCS for N%d\n",
				SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
				current_destination_id);
			RequestMCS();
		}
	}
	// Apply the coordinator's data_duration as the TXOP slot for this AP.
	// PrepareNewTransmission will cap packets (if over-budget) or pad data_duration
	// (if under-budget) so that both APs' DATA frames end at the same time.
	mapc_txop_per_ap_data_duration = notification.tx_info.mapc_allocated_data_duration;
	if (mapc_txop_per_ap_data_duration > IEEE_AX_MAX_PPDU_DURATION)
		mapc_txop_per_ap_data_duration = IEEE_AX_MAX_PPDU_DURATION;
	++node_stats.num_tf_rx;
	node_stats.total_mapc_data_duration_allocated += mapc_txop_per_ap_data_duration;
	PrepareNewTransmission();
}

/**
 * HandleFinishTX_StateWaitAckTf: handles ACK TF reception for coordinated AP and STAs.
 * Called when AP_A's ACK TF finishes transmitting (all nodes in STATE_WAIT_ACK_TF process this).
 * - Coordinated AP: transitions to STATE_WAIT_ACK to receive its own STA's ACK.
 * - STA: transitions to STATE_TX_ACK and sends ACK to its AP after SIFS.
 */
void Node :: HandleFinishTX_StateWaitAckTf(const Notification &notification) {
	if (notification.packet_type != PACKET_TYPE_ACK_TF
			|| wlan.FindMapcGroupIdx(notification.mapc_group_id) < 0) return;

	trigger_DATA_timeout.Cancel();

	if (node_params.node_type == NODE_TYPE_AP) {
		// Coordinated AP: ACK TF received � wait for own STA's ACK
		time_to_trigger = SimTime() + SIFS + TIME_OUT_EXTRA_TIME;
		trigger_ACK_timeout.Set(FixTimeOffset(time_to_trigger, 13, 12));
		node_state = STATE_WAIT_ACK;
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s ACK TF received. Waiting for ACK from own STA (N%d).\n",
			SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3, current_destination_id);
	} else {
		// STA: ACK TF received � send ACK to own AP after SIFS
		node_state = STATE_TX_ACK;
		time_to_trigger = SimTime() + SIFS;
		trigger_SIFS.Set(FixTimeOffset(time_to_trigger, 13, 12));
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s ACK TF received. Sending ACK to N%d after SIFS.\n",
			SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3, current_destination_id);
	}
}

#endif /* NODE_PACKET_METHODS_H */
