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
	if (exchange_sequence.frame_types[0] == PACKET_TYPE_RTS) {
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
	int num_packets_aggregated, double timestamp_generated, double tx_duration){

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

	if (sr_state.spatial_reuse_enabled && sr_state.txop_sr_identified) {
		notification.tx_info = GenerateTxInfo(num_packets_aggregated, data_duration,
			ack_duration, rts_duration, cts_duration, sr_state.current_tx_power_sr, num_channels_tx,
			bits_ofdm_sym, node_params.x, node_params.y, node_params.z, sr_state.flag_change_in_tx_power);
	} else {
		notification.tx_info = GenerateTxInfo(num_packets_aggregated, data_duration,
			ack_duration, rts_duration, cts_duration, current_tx_power, num_channels_tx,
			bits_ofdm_sym, node_params.x, node_params.y, node_params.z, sr_state.flag_change_in_tx_power);
	}

	// Spatial Reuse parameters
	notification.tx_info.bss_color = node_params.bss_color;
	notification.tx_info.srg = node_params.srg;

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
 * @param "first_packet_type" [type int]: PACKET_TYPE_RTS for 4-way handshake,
 *   PACKET_TYPE_DATA for 2-way handshake (default: PACKET_TYPE_RTS)
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
	} else if (first_packet_type == PACKET_TYPE_ICF) {
		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Transmission of ICF #%d started\n",
			SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL3, icf_notification.packet_id);
	} else if (first_packet_type == PACKET_TYPE_TF) {
		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Transmission of TF #%d started\n",
			SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL3, tf_notification.packet_id);
	} else {
		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Transmission of DATA #%d started\n",
			SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL3, data_notification.packet_id);
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
void Node :: PrepareNewTransmission(){

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

	//  - Delete all the aggregated frames contained in the ACKed packet
	if (limited_num_packets_aggregated > 0) {
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

	// TODO: ADD TRANSMISSION DELAY

	// ********************************************************
	InitiateBurstPackets();
	// ********************************************************

	// Compute all packets durations (RTS, CTS, DATA and ACK) and NAV time
	ComputeFramesDuration(&rts_duration, &cts_duration, &data_duration, &ack_duration,
		num_channels_tx, current_modulation, limited_num_packets_aggregated, node_params.frame_length, bits_ofdm_sym);


	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Transmitting (N_agg = %d) in %d channels using modulation %d (%.0f bits per OFDM symbol ---> %.2f Mbps) \n",
		SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL4, limited_num_packets_aggregated,
		(int) pow(2, ix_num_channels_used), current_modulation, bits_ofdm_sym,
		bits_ofdm_sym/IEEE_AX_OFDM_SYMBOL_GI32_DURATION * pow(10,-6));

	if(sr_state.spatial_reuse_enabled && sr_state.txop_sr_identified) {
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
		Notification first_packet_buffer_icf = buffer.GetFirstPacket();

		icf_notification = GenerateNotification(PACKET_TYPE_ICF, current_destination_id,
			first_packet_buffer_icf.packet_id, limited_num_packets_aggregated,
			first_packet_buffer_icf.timestamp_generated, current_tx_duration);

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
		int rand_number_data (2 + rand() % (MAX_NUM_RAND_TIME-2));
		time_rand_value = (double) rand_number_data * MAX_DIFFERENCE_SAME_TIME/MAX_NUM_RAND_TIME;
		time_rand_value = FixTimeOffset(time_rand_value,13,12);
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

	/* ****************************************
	/* SPATIAL REUSE OPERATION
	 *
	 *  Determine the parameters to be used according
	 *  to the SR operation (in case of having detected a TXOP).
	 *
	 * *****************************************/
	if (sr_state.spatial_reuse_enabled) {
		LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s sr_state.txop_sr_identified = %d\n",
			SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL1, sr_state.txop_sr_identified);
		sr_state.flag_change_in_tx_power = TRUE;
		if(sr_state.txop_sr_identified) {
		// Apply the transmission power limitation
		sr_state.current_tx_power_sr = sr_state.next_tx_power_limit;
		// In order to request a new MCS (the tx power may have changed)
		for(int n = 0; n < wlan.num_stas; n++) {
			change_modulation_flag[n] = TRUE;
		}
	} else {
		// Use default values
	}
	if(node_params.save_node_logs) fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Intended values for the next TX: "
		"pd = %f dBm, Tx Power = %f dBm\n", SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL3,
		ConvertPower(PW_TO_DBM, sr_state.current_obss_pd_threshold), ConvertPower(PW_TO_DBM, sr_state.current_tx_power_sr));
	}
	/* **************************************** */

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

	// Cancel NAV TO trigger
	trigger_NAV_timeout.Cancel();

	// Cancel trigger for safety
	trigger_recover_cts_timeout.Cancel();

	// Check if MCS already defined for every potential receiver
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
			node_params.max_channel_allowed, &channel_power, sr_state.current_obss_pd_threshold, timestampt_channel_becomes_free, SimTime(), PIFS);
	} else {
		GetChannelOccupancyByCCA(node_params.current_primary_channel, node_params.pifs_activated, channels_free, node_params.min_channel_allowed,
			node_params.max_channel_allowed, &channel_power, current_pd, timestampt_channel_becomes_free, SimTime(), PIFS);
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
		AbortRtsTransmission();
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

		case STATE_TX_RTS:{		// Wait for CTS

			// Set CTS timeout and change state to STATE_WAIT_CTS
			Notification notification = GenerateNotification(PACKET_TYPE_RTS, current_destination_id,
				rts_notification.packet_id, limited_num_packets_aggregated,
				rts_notification.timestamp_generated, TX_DURATION_NONE);

			outportSelfFinishTX(notification);

			// Sergio on 2018/06/22
			// - Time out should be equal to the collision time, i,e., T_c = T_RTS + SIFS + T_CTS minus T_RTS (already txed)

			// time_to_trigger = SimTime() + SIFS + notification.tx_info.cts_duration + DIFS;
			time_to_trigger = SimTime() + SIFS + notification.tx_info.cts_duration;

			trigger_CTS_timeout.Set(FixTimeOffset(time_to_trigger,13,12));

			node_state = STATE_WAIT_CTS;

			LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s RTS #%d tx finished. Waiting for CTS until %.12f\n",
				SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2,
				notification.packet_id, trigger_CTS_timeout.GetTime());

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

		case STATE_TX_DATA:{ 	// Change state to STATE_WAIT_ACK

			Notification notification = GenerateNotification(PACKET_TYPE_DATA, current_destination_id,
				data_notification.packet_id, data_notification.tx_info.num_packets_aggregated,
				data_notification.timestamp_generated, TX_DURATION_NONE);

			outportSelfFinishTX(notification);

			// Set ACK timeout and change state to STATE_WAIT_ACK
			time_to_trigger = SimTime() + SIFS + TIME_OUT_EXTRA_TIME;
			trigger_ACK_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
			node_state = STATE_WAIT_ACK;

			LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s DATA %d tx finished. Waiting for ACK...\n",
				SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2, notification.packet_id);

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

			Notification notification = GenerateNotification(PACKET_TYPE_ICF, current_destination_id,
				icf_notification.packet_id, limited_num_packets_aggregated,
				icf_notification.timestamp_generated, TX_DURATION_NONE);

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

		case STATE_TX_TF:{		// TF sent — wait for DATA from triggered node

			Notification notification = GenerateNotification(PACKET_TYPE_TF, current_destination_id,
				tf_notification.packet_id, tf_notification.tx_info.num_packets_aggregated,
				tf_notification.timestamp_generated, TX_DURATION_NONE);

			outportSelfFinishTX(notification);

			// After TF, the triggered node is expected to transmit DATA
			time_to_trigger = SimTime() + SIFS + TIME_OUT_EXTRA_TIME;
			trigger_DATA_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
			node_state = STATE_WAIT_DATA;

			LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s TF #%d tx finished. Waiting for DATA...\n",
				SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL2, notification.packet_id);

			break;
		}

		default:
			break;
	}

    // Reset the flag that indicates whether the tx power changed or not
    sr_state.flag_change_in_tx_power = FALSE;

	// LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s  MyTxFinished()\n", SimTime(), node_params.node_id, node_state, LOG_G01, LOG_LVL1);
};

#endif /* NODE_PACKET_METHODS_H */
