/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
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
		total_time_transmitting_per_channel[c] += SIFS + TIME_OUT_EXTRA_TIME;
		// Measurements in the last part of the simulation
		if (SimTime() > (simulation_time_komondor - last_measurements_window)) {
			last_total_time_lost_per_channel[c] += SIFS + TIME_OUT_EXTRA_TIME;
		}
	}

	HandlePacketLoss(PACKET_TYPE_DATA, total_time_lost_in_num_channels, total_time_lost_per_channel,
		data_packets_lost, rts_cts_lost, &data_packets_lost_per_sta, &rts_cts_lost_per_sta, current_right_channel,
		current_left_channel,current_tx_duration, node_id, current_destination_id);

	// Sergio on 16 July 2018: [AGENTS] add data packet lost for partial throughput computations
	// Update performance measurements
	total_time_lost_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
	performance_report.total_time_lost_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
	for(int c = current_left_channel; c <= current_right_channel; ++c){
		total_time_lost_per_channel[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
		performance_report.total_time_lost_per_channel[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
	}
	performance_report.data_packets_lost++;

	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s  ACK TIMEOUT! Data packet %d lost\n",
		SimTime(), node_id, node_state, LOG_D17, LOG_LVL4,
		packet_id);

	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Handling contention window\n",
		SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s From CW = [%d-%d], b = %d, m = %d\n",
		SimTime(), node_id, node_state, LOG_D08, LOG_LVL5,
		current_cw_min, current_cw_max, cw_stage_current, cw_stage_max);
	// The CW only must be changed when ACK received or loss detected.
	HandleContentionWindow(
		cw_adaptation, INCREASE_CW, &deterministic_bo_active, &current_cw_min, &current_cw_max, &cw_stage_current,
		cw_min_default, cw_max_default, cw_stage_max, distance_to_token, backoff_type);

	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s To CW = [%d-%d], b = %d, m = %d\n",
		SimTime(), node_id, node_state, LOG_D08, LOG_LVL5,
		current_cw_min, current_cw_max, cw_stage_current, cw_stage_max);

	RestartNode(TRUE);
}

/**
 * Handle the CTS timeout. It is called when CTS timeout is triggered (after sending RTS).
 */
void Node :: CtsTimeout(trigger_t &){

	HandlePacketLoss(PACKET_TYPE_CTS, total_time_lost_in_num_channels, total_time_lost_per_channel,
		data_packets_lost, rts_cts_lost, &data_packets_lost_per_sta, &rts_cts_lost_per_sta, current_right_channel,
		current_left_channel,current_tx_duration, node_id, current_destination_id);

	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s ---------------------------------------------\n",
		SimTime(), node_id, node_state, LOG_D17, LOG_LVL1);
	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s CTS TIMEOUT! RTS-CTS packet lost\n",
		SimTime(), node_id, node_state, LOG_D17, LOG_LVL2);

	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Handling contention window\n",
		SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s From CW = [%d-%d], b = %d, m = %d\n",
		SimTime(), node_id, node_state, LOG_D08, LOG_LVL5,
		current_cw_min, current_cw_max, cw_stage_current, cw_stage_max);
	// The CW only must be changed when ACK received or loss detected.
	HandleContentionWindow(
		cw_adaptation, INCREASE_CW, &deterministic_bo_active, &current_cw_min, &current_cw_max, &cw_stage_current,
		cw_min_default, cw_max_default, cw_stage_max, distance_to_token, backoff_type);

	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s To CW = [%d-%d], b = %d, m = %d\n",
		SimTime(), node_id, node_state, LOG_D08, LOG_LVL5,
		current_cw_min, current_cw_max, cw_stage_current, cw_stage_max);

	// Update TX time statistics
	total_time_transmitting_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
	performance_report.total_time_transmitting_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
	total_time_lost_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
	performance_report.total_time_lost_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;

	for(int c = current_left_channel; c <= current_right_channel; ++c){
		total_time_transmitting_per_channel[c] += current_tx_duration;
		performance_report.total_time_transmitting_per_channel[c] += current_tx_duration;
		total_time_lost_per_channel[c] += current_tx_duration;
		performance_report.total_time_lost_per_channel[c] += current_tx_duration;
		// Measurements in the last part of the simulation
		if (SimTime() > (simulation_time_komondor - last_measurements_window)) {
			last_total_time_transmitting_per_channel[c] += current_tx_duration;
			last_total_time_lost_per_channel[c] += current_tx_duration;
		}
	}

	RestartNode(TRUE);
}

/**
 * Handle the Data timeout. It is called when data timeout (after sending CTS) is triggered.
 */
void Node :: DataTimeout(trigger_t &){

	HandlePacketLoss(PACKET_TYPE_CTS, total_time_lost_in_num_channels, total_time_lost_per_channel,
		data_packets_lost, rts_cts_lost, &data_packets_lost_per_sta, &rts_cts_lost_per_sta, current_right_channel,
		current_left_channel,current_tx_duration, node_id, current_destination_id);

	total_time_lost_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
	performance_report.total_time_lost_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;


	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s DATA TIMEOUT! RTS-CTS packet lost\n",
		SimTime(), node_id, node_state, LOG_D17, LOG_LVL4);

	// Sergio on 20/09/2017. CW only must be changed when ACK received or loss detected.

	RestartNode(TRUE);
}

/**
 * Handle the NAV timeout. It is called when NAV timeout is triggered.
 */
void Node :: NavTimeout(trigger_t &){

	LOGS(save_node_logs,node_logger.file, "\n **********************************************************************\n");

	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s NAV TIMEOUT!\n",
		SimTime(), node_id, node_state, LOG_D17, LOG_LVL1);

	time_in_nav = time_in_nav + (SimTime() - last_time_not_in_nav);

	if(node_is_transmitter){

		// Apply new configuration (if it is the case)
		if (flag_apply_new_configuration) {
			ApplyNewConfiguration(new_configuration);
		}
		flag_apply_new_configuration = FALSE; // Turn flag off

		node_state = STATE_SENSING;

		int resume (HandleBackoff(RESUME_TIMER, &channel_power, current_primary_channel,
			current_pd, buffer.QueueSize()));

		// Update BO value according to TO extra time
		if (resume) {

			time_to_trigger = SimTime() + DIFS - TIME_OUT_EXTRA_TIME;

			trigger_start_backoff.Set(FixTimeOffset(time_to_trigger,13,12));

			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Starting new DIFS to finsih in %.12f\n",
				SimTime(), node_id, node_state, LOG_D17, LOG_LVL3,
				trigger_start_backoff.GetTime());

		} else {
			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s New DIFS cannot be started\n",
				SimTime(), node_id, node_state, LOG_D17, LOG_LVL3);
		}

	} else {

		RestartNode(TRUE);

	}

}

#endif /* NODE_TIMEOUT_METHODS_H */
