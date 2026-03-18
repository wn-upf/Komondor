/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * node_backoff_methods.h: Backoff management, node restart, and channel-sensing
 *   helper method implementations.
 *
 * NOTE: This file is an implementation fragment. It must be included from node.h
 *   after the Node class definition, not included directly.
 *
 * Functions defined here:
 *   - Node::AbortInitialTransmission
 *   - Node::ScheduleBackoffAfterDIFS
 *   - Node::UpdateSINRFromNotification
 *   - Node::PauseBackoff
 *   - Node::ResumeBackoff
 *   - Node::CallRestartSta
 *   - Node::RestartNode
 *   - Node::StartSavingLogs
 *   - Node::HandleSlottedBackoffCollision
 *   - Node::RecoverFromCtsTimeout
 *   - Node::CallSensing
 */

#ifndef NODE_BACKOFF_METHODS_H
#define NODE_BACKOFF_METHODS_H

/**
 * Used when a node ends its backoff but cannot transmit
 */
void Node :: AbortInitialTransmission(){

	if(node_params.backoff_type == BACKOFF_DETERMINISTIC_QUALCOMM){
	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s ca_state.deterministic_bo_active = %d, ca_state.num_bo_interruptions = %d.\n",
		SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL4,
		ca_state.deterministic_bo_active, ca_state.num_bo_interruptions);
	}

	node_stats.num_tx_init_not_possible ++;
	// Compute a new backoff and trigger a new DIFS
	ca_state.remaining_backoff = ComputeBackoff(ca_state.current_cw_min, ca_state.current_cw_max, node_params.backoff_type,
			current_traffic_type, ca_state.deterministic_bo_active, ca_state.num_bo_interruptions, ca_state.base_backoff_deterministic,
			ca_state.previous_backoff);

	ca_state.previous_backoff = ca_state.remaining_backoff;	// Update the last used backoff

	node_stats.expected_backoff += ca_state.remaining_backoff;
	node_stats.num_new_backoff_computations++;
	node_state = STATE_SENSING;

	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Transmission is NOT possible\n",
		SimTime(), node_params.node_id, node_state, LOG_F03, LOG_LVL3);

}

/**
 * Schedule a new backoff countdown after a DIFS idle period.
 * Replaces the repeated 2-line idiom:
 *   time_to_trigger = SimTime() + DIFS;
 *   trigger_start_backoff.Set(FixTimeOffset(time_to_trigger, 13, 12));
 */
void Node :: ScheduleBackoffAfterDIFS() {
	double time_to_trigger = SimTime() + DIFS;
	trigger_start_backoff.Set(FixTimeOffset(time_to_trigger, 13, 12));
}

/**
 * Compute SINR for an incoming notification.
 * Sets member variables power_rx_interest, max_pw_interference, and current_sinr.
 * Steps 1-3 of the standard "can the packet be decoded?" sequence.
 * Step 4 (IsPacketLost) is left to the caller so intermediate LOGS remain visible.
 */
void Node :: UpdateSINRFromNotification(const Notification &notification) {
	power_rx_interest = power_received_per_node[notification.source_id];
	ComputeMaxInterference(&max_pw_interference, &channel_max_intereference,
		notification, node_state, power_received_per_node, &channel_power);
	current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);
}

/**
 * Pause the backoff
 */
void Node :: PauseBackoff(){

	if(trigger_start_backoff.Active()){
		LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Cancelling DIFS. BO still frozen at %.9f (%.2f slots)\n",
			SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL3,
			ca_state.remaining_backoff * pow(10,6), ca_state.remaining_backoff / SLOT_TIME);

		trigger_start_backoff.Cancel();
	} else {

		if(trigger_end_backoff.Active()){	// If backoff trigger is active, freeze it

			ca_state.remaining_backoff = ComputeRemainingBackoff(node_params.backoff_type, trigger_end_backoff.GetTime() - SimTime());

			++ca_state.num_bo_interruptions;

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s BO is active. Freezing it from %.9f (%.2f slots) to %.9f (%.2f slots) -> BO interruptions = %d\n",
				SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL3,
				(trigger_end_backoff.GetTime() - SimTime()) * pow(10,6),
				(trigger_end_backoff.GetTime() - SimTime())/SLOT_TIME,
				ca_state.remaining_backoff * pow(10,6), ca_state.remaining_backoff/SLOT_TIME, ca_state.num_bo_interruptions);

//			LOGS(node_params.save_node_logs,node_logger.file,
//								"%.15f;N%d;S%d;%s;%s Original remaining BO: %.9f us\n",
//								SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL3,
//								(trigger_end_backoff.GetTime() - SimTime())*pow(10,6));

//			LOGS(node_params.save_node_logs,node_logger.file,
//					"%.15f;N%d;S%d;%s;%s Backoff is active --> freeze it at %.9f us (%.2f slots)\n",
//					SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL3,
//					ca_state.remaining_backoff * pow(10,6), ca_state.remaining_backoff/SLOT_TIME);

			trigger_end_backoff.Cancel();

		} else {	// If backoff trigger is frozen

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Backoff is NOT active - it is already frozen at %.9f us (%.2f slots)\n",
				SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL3,
				ca_state.remaining_backoff * pow(10,6), ca_state.remaining_backoff / SLOT_TIME);

			trigger_end_backoff.Cancel(); // Redundant (for safety)

		}

	}
}

/**
 * Resume the backoff (triggered after DIFS is completed)
 */
void Node :: ResumeBackoff(trigger_t &){

//	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s DIFS finished\n",
//					SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL2);

	time_to_trigger = SimTime() + ca_state.remaining_backoff;

	trigger_end_backoff.Set(FixTimeOffset(time_to_trigger,13,12));

	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Resuming backoff in %.9f us (%.2f slots)\n",
		SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL3,
		(ca_state.remaining_backoff * pow(10,6)), (ca_state.remaining_backoff / (double) SLOT_TIME));

//	LOGS(node_params.save_node_logs,node_logger.file,
//				"%.15f;N%d;S%d;%s;%s DIFS: active = %d, t_DIFS = %f - backoff: active = %d - t_back = %f\n",
//				SimTime(), node_params.node_id, node_state, LOG_D02, LOG_LVL3,
//				trigger_start_backoff.Active(), trigger_start_backoff.GetTime() - SimTime(),
//				trigger_end_backoff.Active(), trigger_end_backoff.GetTime() - SimTime());

}

/**
 * Calls RestartNode() when called by the trigger. It is useful to handle transmission
 * ocurring at the same time
 * STAs should wait MAX_DIFFERENCE_SAME_TIME in order to avoid entering in NAV when it is not required.
 * E.g. STA A is sensing and is able to decode a packet from AP A. At the same time AP B transmits and
 * harms AP A - STA A transmission. STA A is restarted. Again, at the same time AP C transmits. Then,
 * in order to avoid entering in NAV when in fact a slotted BO collision did happen, STA A should not
 * listen to AP C packet. After MAX_DIFFERENCE_SAME_TIME, no same time events are ensured and STA A can
 * start sensing again.
 */
void Node :: CallRestartSta(trigger_t &){

	RestartNode(FALSE);

}

/**
 * Re-initializes the nodes. Puts it in the initial state (sensing and decreasing BO)
 * @param "called_by_time_out" [type int]: indicated whether this method was called after a timeout or not
 */
void Node :: RestartNode(int called_by_time_out){

	LOGS(node_params.save_node_logs, node_logger.file, "\n **********************************************************************\n");
	LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s Node Restarted (%d)\n",
		SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL1,
		called_by_time_out);

	// Update TX time statistics

	int ix_num_ch = (int)log2(current_right_channel - current_left_channel + 1);

	node_stats.total_time_transmitting_in_num_channels[ix_num_ch] += current_tx_duration;
	performance_report.total_time_transmitting_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;

	for(int c = current_left_channel; c <= current_right_channel; ++c){
		node_stats.total_time_transmitting_per_channel[c] += current_tx_duration;
		performance_report.total_time_transmitting_per_channel[c] += current_tx_duration;
		// Measurements in the last part of the simulation
		if (SimTime() > (node_params.simulation_time_komondor - node_stats.last_measurements_window)) {
			node_stats.last_total_time_transmitting_per_channel[c] += current_tx_duration;
		}
	}

	// Apply new configuration (if it is the case)
	if (flag_apply_new_configuration) {
		ApplyNewConfiguration(new_configuration);
	}
	flag_apply_new_configuration = FALSE; // Turn flag off
	//PrintNodeInfo(INFO_DETAIL_LEVEL_2);

	// Reinitialize parameters
	node_state = STATE_SENSING;
	current_tx_duration = 0;
	power_rx_interest = 0;
	max_pw_interference = 0;

	receiving_from_node_id = NODE_ID_NONE;
	receiving_packet_id = NO_PACKET_ID;
	sr_state.mapc_cosr_active = 0;

	// Cancel triggers for safety
	trigger_end_backoff.Cancel();
	trigger_recover_cts_timeout.Cancel();
	trigger_start_backoff.Cancel();

	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s node_is_transmitter = %d "
			"/ buffer.QueueSize() = %d\n",
		SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL3,
		node_is_transmitter, buffer.QueueSize());

	// Generate new BO in case of being a TX node
	if(node_is_transmitter && buffer.QueueSize() > 0){

		// Set the ID of the next packet
		++packet_id;

		// In case of being an AP
		ca_state.remaining_backoff = ComputeBackoff(ca_state.current_cw_min, ca_state.current_cw_max,
				node_params.backoff_type, current_traffic_type, ca_state.deterministic_bo_active, ca_state.num_bo_interruptions, ca_state.base_backoff_deterministic, ca_state.previous_backoff);
		ca_state.previous_backoff = ca_state.remaining_backoff;
		node_stats.expected_backoff = node_stats.expected_backoff + ca_state.remaining_backoff;
		++node_stats.num_new_backoff_computations;

		// Sergio on June 26 th
		// - compute average waiting time to access the channel
		node_stats.timestamp_new_trial_started = SimTime();

		if(node_params.backoff_type == BACKOFF_DETERMINISTIC_QUALCOMM){
		LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s ca_state.deterministic_bo_active = %d, ca_state.num_bo_interruptions = %d.\n",
			SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL4,
			ca_state.deterministic_bo_active, ca_state.num_bo_interruptions);
		}

		// Restart the counter for the deterministic backoff
		ca_state.num_bo_interruptions = 0;

		LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s New backoff computed: %f (%.0f slots).\n",
			SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL3,
			ca_state.remaining_backoff, ca_state.remaining_backoff/SLOT_TIME);

		// Add extra slot since node has transmitted
		ca_state.remaining_backoff = ca_state.remaining_backoff + SLOT_TIME;

		LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Extra slot added --> remaining BO %f slots\n",
			SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL4,
			ca_state.remaining_backoff / SLOT_TIME);

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Checking if BO can be resumed. Pow(primary #%d) =  %.2f dBm\n",
			SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL4,
			node_params.current_primary_channel, ConvertPower(PW_TO_DBM, channel_power[node_params.current_primary_channel]));

		// Freeze backoff immediately if primary channel is occupied
		int resume;
		if (sr_state.spatial_reuse_enabled && sr_state.txop_sr_identified) {
			resume = HandleBackoff(RESUME_TIMER, &channel_power, node_params.current_primary_channel,
				sr_state.current_obss_pd_threshold, buffer.QueueSize());
		} else {
			resume = HandleBackoff(RESUME_TIMER, &channel_power, node_params.current_primary_channel,
				current_pd, buffer.QueueSize());
		}

		// Check if node has to freeze the BO (if it is not already frozen)
		if (resume) {
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s BO can be resumed! Starting DIFS...\n",
				SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL5);
			// time_to_trigger = SimTime() + DIFS - TIME_OUT_EXTRA_TIME;
			time_to_trigger = SimTime() + DIFS; // TODO: EDCA TO BE IMPLEMENTED -> AIFSN[AC] * SLOT_TIME + SIFS
			trigger_start_backoff.Set(FixTimeOffset(time_to_trigger,13,12));
		} else {
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s BO cannot be resumed!\n",
				SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL5);
		}
	}

	// Clean the logical NACK to avoid errors
	CleanNack(&logical_nack);

	// Cancel timeout triggers for safety
	trigger_ACK_timeout.Cancel();			// Trigger when ACK hasn't arrived in time
	trigger_CTS_timeout.Cancel();			// Trigger when CTS hasn't arrived in time
	trigger_DATA_timeout.Cancel();			// Trigger when DATA TX could not start due to RTS/CTS failure
	trigger_NAV_timeout.Cancel();  			// Trigger for the NAV

}

/**
 * Start saving logs from a given initial value
 */
void Node:: StartSavingLogs(trigger_t &){
	node_params.save_node_logs = TRUE;
}

/**
 * Handle collisions by slotted backoff:
 * STAs should wait MAX_DIFFERENCE_SAME_TIME in order to avoid entering in NAV when it is not required.
 * E.g. STA A is sensing and is able to decode a packet from AP A. At the same time AP B transmits and
 * harms AP A - STA A transmission. STA A is restarted. Again, at the same time AP C transmits. Then,
 * in order to avoid entering in NAV when in fact a slotted BO collision did happen, STA A should not
 * listen to AP C packet. After MAX_DIFFERENCE_SAME_TIME, no same time events are ensured and STA A can
 * start sensing again.
 */
void Node:: HandleSlottedBackoffCollision() {
	// Slotted BO collision (case where STA is receiving)
	loss_reason = PACKET_LOST_BO_COLLISION;
	if(!node_is_transmitter) {
		node_state = STATE_SLEEP; // avoid listening to notifications until restart
		time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;
		trigger_restart_sta.Set(FixTimeOffset(time_to_trigger,13,12));
	} else {
		// In case STAs can send to AP
		RestartNode(FALSE);
	}
}

/**
 * Recover from a CTS timeout
 */
void Node:: RecoverFromCtsTimeout(trigger_t &) {
	// Sergio on 25 Oct 2017
	// - Just restart the node to start the DIFS
	LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s RecoverFromCtsTimeout\n",
		SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL3);
	// Cancel trigger for safety
	trigger_recover_cts_timeout.Cancel();
	RestartNode(TRUE);
}

/**
 * Used to return to Sensing state in case several conditions hold
 */
void Node:: CallSensing(trigger_t &){

	LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s State changed to sensing due to NAV collision\n",
		SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL3);

	node_state = STATE_SENSING;

	int resume (HandleBackoff(RESUME_TIMER, &channel_power,
		node_params.current_primary_channel, current_pd, buffer.QueueSize()));

	// Check if node has to freeze the BO (if it is not already frozen)
	if (resume) {
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s BO can be resumed! Starting DIFS...\n",
			SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL5);
		// time_to_trigger = SimTime() + DIFS - TIME_OUT_EXTRA_TIME;
		ScheduleBackoffAfterDIFS();
	} else {
		// Spatial Reuse: check for SR TXOP when BO cannot be resumed
		CheckSRTXOPAtCallSensing();
	}

}

/**
 * Handle InportSomeNodeStartTX for STATE_WAIT_ICR (coordinator waiting for ICR)
 */
void Node :: HandleStartTX_StateWaitIcr(const Notification &notification) {
	if (notification.packet_type == PACKET_TYPE_ICR
			&& notification.destination_id == node_params.node_id
			&& notification.mapc_group_id == wlan.mapc_group_ids[mapc_active_group_idx]) {
		trigger_CTS_timeout.Cancel();
		incoming_notification = notification;
		// Initialize power_rx_interest so that SINR is correct if an interferer
		// arrives during ICR reception (HandleStartTX_StateRxData uses it).
		UpdateSINRFromNotification(notification);
		node_state = STATE_RX_ICR;
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s Received ICR from N%d -> STATE_RX_ICR\n",
			SimTime(), node_params.node_id, STATE_RX_ICR, LOG_D07, LOG_LVL2,
			notification.source_id);
	}
}

/**
 * Handle InportSomeNodeStartTX for STATE_WAIT_MU_RTS (coordinated AP waiting for MU-RTS/TXS)
 */
void Node :: HandleStartTX_StateWaitMuRts(const Notification &notification) {
	if (notification.packet_type == PACKET_TYPE_MU_RTS_TXS
			&& notification.destination_id == node_params.node_id) {
		trigger_NAV_timeout.Cancel();
		incoming_notification = notification;
		// Initialize power_rx_interest so that SINR is correct if an interferer
		// arrives during MU-RTS reception (HandleStartTX_StateRxData uses it).
		UpdateSINRFromNotification(notification);
		node_state = STATE_RX_MU_RTS;
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s Received MU-RTS/TXS from N%d -> STATE_RX_MU_RTS\n",
			SimTime(), node_params.node_id, STATE_RX_MU_RTS, LOG_D07, LOG_LVL2,
			notification.source_id);
	}
}

/**
 * Handle InportSomeNodeStartTX for STATE_WAIT_TF (coordinated AP waiting for TF)
 */
void Node :: HandleStartTX_StateWaitTf(const Notification &notification) {
	if (notification.packet_type == PACKET_TYPE_TF
			&& notification.destination_id == NODE_ID_MAPC_BROADCAST
			&& notification.mapc_group_id == wlan.mapc_group_ids[mapc_active_group_idx]) {
		trigger_DATA_timeout.Cancel();
		incoming_notification = notification;
		// Initialize power_rx_interest so that SINR is correct if an interferer
		// arrives during TF reception (HandleStartTX_StateRxData uses it).
		UpdateSINRFromNotification(notification);
		node_state = STATE_RX_TF;
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s Received TF from N%d -> STATE_RX_TF\n",
			SimTime(), node_params.node_id, STATE_RX_TF, LOG_D07, LOG_LVL2,
			notification.source_id);
	}
}

#endif /* NODE_BACKOFF_METHODS_H */
