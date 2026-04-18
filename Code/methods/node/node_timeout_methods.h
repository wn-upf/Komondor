/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * node_timeout_methods.h: ACK, CTS, DATA, and NAV timeout handler implementations.
 *
 * NOTE: This file is an implementation fragment. It must be included from node.h
 *   after the Node class definition, not included directly.
 *
 * Functions defined here:
 *   - Node::AckTimeout
 *   - Node::CtsTimeout
 *   - Node::DataTimeout
 *   - Node::NavTimeout
 */

#ifndef NODE_TIMEOUT_METHODS_H
#define NODE_TIMEOUT_METHODS_H

/**
 * Handle the ACK timeout. It is called when ACK timeout is triggered.
 */
void Node :: AckTimeout(trigger_t &){

	current_tx_duration += SIFS + TIME_OUT_EXTRA_TIME;		// Add ACK timeout to tx_duration

	for(int c = current_left_channel; c <= current_right_channel; ++c){
		node_stats.total_time_transmitting_per_channel[c] += SIFS + TIME_OUT_EXTRA_TIME;
		// Measurements in the last part of the simulation
		if (SimTime() > (node_params.simulation_time_komondor - node_stats.last_measurements_window)) {
			node_stats.last_total_time_lost_per_channel[c] += SIFS + TIME_OUT_EXTRA_TIME;
		}
	}

	HandlePacketLoss(PACKET_TYPE_DATA, node_stats.total_time_lost_in_num_channels, node_stats.total_time_lost_per_channel,
		node_stats.data_packets_lost, node_stats.rts_cts_lost, &node_stats.data_packets_lost_per_sta, &node_stats.rts_cts_lost_per_sta, current_right_channel,
		current_left_channel,current_tx_duration, node_params.node_id, current_destination_id);

	// Update performance_report measurements (performance_report is not passed to HandlePacketLoss)
	performance_report.total_time_lost_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
	for(int c = current_left_channel; c <= current_right_channel; ++c){
		performance_report.total_time_lost_per_channel[c] += current_tx_duration;
	}
	performance_report.data_packets_lost++;

	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s  ACK TIMEOUT! Data packet %d lost\n",
		SimTime(), node_params.node_id, node_state, LOG_D17, LOG_LVL4,
		packet_id);

	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Handling contention window\n",
		SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s From CW = [%d-%d], b = %d, m = %d\n",
		SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
		ca_state.current_cw_min, ca_state.current_cw_max, ca_state.cw_stage_current, node_params.cw_stage_max);
	// The CW only must be changed when ACK received or loss detected.
	HandleContentionWindow(
		node_params.cw_adaptation, INCREASE_CW, &ca_state.deterministic_bo_active, &ca_state.current_cw_min, &ca_state.current_cw_max, &ca_state.cw_stage_current,
		node_params.cw_min_default, node_params.cw_max_default, node_params.cw_stage_max, distance_to_token, node_params.backoff_type, current_traffic_type);

	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s To CW = [%d-%d], b = %d, m = %d\n",
		SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
		ca_state.current_cw_min, ca_state.current_cw_max, ca_state.cw_stage_current, node_params.cw_stage_max);

	RestartNode(TRUE);
}

/**
 * Handle the CTS timeout. It is called when CTS timeout is triggered (after sending RTS).
 */
void Node :: CtsTimeout(trigger_t &){

	// MAPC Co-TDMA: ICR timeout — coordinated AP did not reply to ICF.
	// The TXOP was grabbed fairly; continue with solo DATA instead of wasting it.
	if (node_state == STATE_WAIT_ICR
			&& wlan.mapc_enabled
			&& wlan.mapc_method_ids[mapc_active_group_idx] == CO_TDMA) {
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s ICR timeout (N%d did not reply to ICF). Continuing TXOP solo.\n",
			SimTime(), node_params.node_id, node_state, LOG_D17, LOG_LVL2, mapc_selected_peer_id);
		mapc_peer_has_data = FALSE;
		ProceedAfterIcr();
		return;
	}

	HandlePacketLoss(PACKET_TYPE_CTS, node_stats.total_time_lost_in_num_channels, node_stats.total_time_lost_per_channel,
		node_stats.data_packets_lost, node_stats.rts_cts_lost, &node_stats.data_packets_lost_per_sta, &node_stats.rts_cts_lost_per_sta, current_right_channel,
		current_left_channel,current_tx_duration, node_params.node_id, current_destination_id);
	performance_report.rts_cts_lost++;

	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s ---------------------------------------------\n",
		SimTime(), node_params.node_id, node_state, LOG_D17, LOG_LVL1);
	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s CTS TIMEOUT! RTS-CTS packet lost\n",
		SimTime(), node_params.node_id, node_state, LOG_D17, LOG_LVL2);

	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Handling contention window\n",
		SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s From CW = [%d-%d], b = %d, m = %d\n",
		SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
		ca_state.current_cw_min, ca_state.current_cw_max, ca_state.cw_stage_current, node_params.cw_stage_max);
	// The CW only must be changed when ACK received or loss detected.
	HandleContentionWindow(
		node_params.cw_adaptation, INCREASE_CW, &ca_state.deterministic_bo_active, &ca_state.current_cw_min, &ca_state.current_cw_max, &ca_state.cw_stage_current,
		node_params.cw_min_default, node_params.cw_max_default, node_params.cw_stage_max, distance_to_token, node_params.backoff_type, current_traffic_type);

	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s To CW = [%d-%d], b = %d, m = %d\n",
		SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
		ca_state.current_cw_min, ca_state.current_cw_max, ca_state.cw_stage_current, node_params.cw_stage_max);

	// Update TX time statistics (loss only; transmitting counters are handled by RestartNode)
	node_stats.total_time_lost_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
	performance_report.total_time_lost_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;

	for(int c = current_left_channel; c <= current_right_channel; ++c){
		node_stats.total_time_lost_per_channel[c] += current_tx_duration;
		performance_report.total_time_lost_per_channel[c] += current_tx_duration;
		// Measurements in the last part of the simulation
		if (SimTime() > (node_params.simulation_time_komondor - node_stats.last_measurements_window)) {
			node_stats.last_total_time_lost_per_channel[c] += current_tx_duration;
		}
	}

	RestartNode(TRUE);
}

/**
 * Handle the Data timeout. It is called when data timeout (after sending CTS) is triggered.
 */
void Node :: DataTimeout(trigger_t &){

	// In MAPC STATE_WAIT_TF the timeout means the TF frame from the coordinator did not arrive;
	// this is not a DATA/RTS-CTS loss for this node, so skip packet-loss accounting.
	if (node_state != STATE_WAIT_TF && node_state != STATE_WAIT_ACK_TF) {
		HandlePacketLoss(PACKET_TYPE_CTS, node_stats.total_time_lost_in_num_channels, node_stats.total_time_lost_per_channel,
			node_stats.data_packets_lost, node_stats.rts_cts_lost, &node_stats.data_packets_lost_per_sta, &node_stats.rts_cts_lost_per_sta, current_right_channel,
			current_left_channel,current_tx_duration, node_params.node_id, current_destination_id);
	}

	node_stats.total_time_lost_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
	performance_report.total_time_lost_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;


	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s DATA TIMEOUT! (state=%d)\n",
		SimTime(), node_params.node_id, node_state, LOG_D17, LOG_LVL4, node_state);

	// Sergio on 20/09/2017. CW only must be changed when ACK received or loss detected.

	RestartNode(TRUE);
}

/**
 * Handle the NAV timeout. It is called when NAV timeout is triggered.
 */
void Node :: NavTimeout(trigger_t &){

	LOGS(node_params.save_node_logs,node_logger.file, "\n **********************************************************************\n");

	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s NAV TIMEOUT!\n",
		SimTime(), node_params.node_id, node_state, LOG_D17, LOG_LVL1);

	node_stats.time_in_nav = node_stats.time_in_nav + (SimTime() - node_stats.last_time_not_in_nav);

	if(node_is_transmitter){

		// Apply new configuration (if it is the case)
		if (flag_apply_new_configuration) {
			ApplyNewConfiguration(new_configuration);
		}
		flag_apply_new_configuration = FALSE; // Turn flag off

		node_state = STATE_SENSING;

		int resume (HandleBackoff(RESUME_TIMER, &channel_power, node_params.current_primary_channel,
			current_pd, buffer.QueueSize()));

		// Update BO value according to TO extra time
		if (resume) {

			time_to_trigger = SimTime() + DIFS - TIME_OUT_EXTRA_TIME;

			trigger_start_backoff.Set(FixTimeOffset(time_to_trigger,13,12));

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Starting new DIFS to finsih in %.12f\n",
				SimTime(), node_params.node_id, node_state, LOG_D17, LOG_LVL3,
				trigger_start_backoff.GetTime());

		} else {
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s New DIFS cannot be started\n",
				SimTime(), node_params.node_id, node_state, LOG_D17, LOG_LVL3);
		}

	} else {

		RestartNode(TRUE);

	}

}

#endif /* NODE_TIMEOUT_METHODS_H */
