/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * node_statistics_methods.h: Node statistics, performance reporting, and info-display
 *   method implementations.
 *
 * NOTE: This file is an implementation fragment. It must be included from node.h
 *   after the Node class definition, not included directly.
 *
 * Functions defined here:
 *   - Node::UpdatePerformanceMeasurements
 *   - Node::MeasureRho
 *   - Node::PrintNodeInfo
 *   - Node::WriteNodeInfo
 *   - Node::WriteNodeConfiguration
 *   - Node::WriteReceivedConfiguration
 *   - Node::PrintNodeConfiguration
 *   - Node::PrintProgressBar
 *   - Node::PrintOrWriteNodeStatistics
 *   - Node::SaveSimulationPerformance
 */

#ifndef NODE_STATISTICS_METHODS_H
#define NODE_STATISTICS_METHODS_H

/**
 * Update the performance-related information to be sent to agents
 */
void Node :: UpdatePerformanceMeasurements(){

	// Update performance measurements

	// - Throughput
	performance_report.throughput =
		(((double)(performance_report.data_packets_sent -
		performance_report.data_packets_lost) * node_params.frame_length
		* limited_num_packets_aggregated)) / (SimTime()-performance_report.timestamp);

	// - Delay
	//performance_report.average_delay = performance_report.sum_delays / performance_report.num_delay_measurements;

	// - Access delay (contention time)
	performance_report.average_delay = performance_report.sum_waiting_time / (double) performance_report.num_waiting_time_measurements;

	// - Max RSSI received per WLAN
	for (int i = 0 ; i < node_params.total_wlans_number; ++ i) {
		performance_report.rssi_list[i] = max_received_power_in_ap_per_wlan[i];
		performance_report.max_received_power_in_ap_per_wlan[i] = max_received_power_in_ap_per_wlan[i];
	}

	// RSSI received from each STA
	if(node_params.node_type == NODE_TYPE_AP) UpdateRssiPerSta(wlan, rssi_per_sta, received_power_array, node_params.total_nodes_number);
	performance_report.rssi_list_per_sta = rssi_per_sta;

	// -  Channel occupancy
	double successful_occupancy(0.0);
	double total_occupancy(0.0);
	for(int n = 0; n < node_params.num_channels_allowed; ++n){
		successful_occupancy += ((performance_report.total_time_transmitting_in_num_channels[n] -
			performance_report.total_time_lost_in_num_channels[n])) / (SimTime() - performance_report.timestamp);
		total_occupancy += performance_report.total_time_transmitting_in_num_channels[n] / (SimTime() - performance_report.timestamp);
	}
	performance_report.successful_channel_occupancy = successful_occupancy;
	performance_report.total_channel_occupancy = total_occupancy;

//	printf("performance_report.channel_occupancy = %f (total = %f)\n",
//		performance_report.successful_channel_occupancy, performance_report.total_channel_occupancy);

	performance_report.num_stas = wlan.num_stas;

}

/**
 * Measure the utilization of the buffer
 */
void Node:: MeasureRho(trigger_t &){
	// if ( (buffer.QueueSize() > 0) && (channel_power[node_params.current_primary_channel] < current_pd)){
	if (node_state == STATE_SENSING && channel_power[node_params.current_primary_channel] < current_pd){
		LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s RHO: Sensing + free\n",
			SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL3);
		++node_stats.num_measures_rho;
		// DIFS condition: !trigger_start_backoff.Active()
		if (buffer.QueueSize() > 0){
			LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s RHO: Packet in buffer\n",
				SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL4);
			node_stats.num_measures_rho_accomplished ++;
		} else {
			LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s RHO: Not packet in buffer\n",
				SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL4);
		}
	} else {

//		LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s No RHO!\n",
//						SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL3);
	}
	// Utilization
	++node_stats.num_measures_utilization;
	if (buffer.QueueSize() > 0) ++node_stats.num_measures_buffer_with_packets;
	trigger_rho_measurement.Set(SimTime() + node_stats.delta_measure_rho);
}

/************************/
/************************/
/*  PRINT INFORMATION   */
/************************/
/************************/

/**
 * Print Node's information
 * @param "info_detail_level" [type int]: level of detail of the info printed
 */
void Node :: PrintNodeInfo(int info_detail_level){
	printf("\n%s Node %s info:\n", LOG_LVL3, node_params.node_code.c_str());
	printf("%s node_params.node_id = %d\n", LOG_LVL4, node_params.node_id);
	printf("%s node_params.node_type = %d\n", LOG_LVL4, node_params.node_type);
	printf("%s position = (%.2f, %.2f, %.2f)\n", LOG_LVL4, node_params.x, node_params.y, node_params.z);
	printf("%s node_params.current_primary_channel = %d (range = [%d - %d])\n",
		LOG_LVL4, node_params.current_primary_channel, node_params.min_channel_allowed, node_params.max_channel_allowed);
	printf("%s node_params.current_dcb_policy = %d\n", LOG_LVL4, node_params.current_dcb_policy);
	printf("%s node_params.tx_power_default = %f pW (%f dBm)\n", LOG_LVL4, node_params.tx_power_default, ConvertPower(PW_TO_DBM, node_params.tx_power_default));
	printf("%s node_params.sensitivity_default = %f pW (%f dBm)\n", LOG_LVL4, node_params.sensitivity_default, ConvertPower(PW_TO_DBM, node_params.sensitivity_default));
	printf("%s spatial_reuse_enabled = %d\n", LOG_LVL4, (node_params.bss_color>=0));
	if(node_params.bss_color>=0) {
		printf("%s node_params.bss_color = %d\n", LOG_LVL5, node_params.bss_color);
		printf("%s node_params.srg = %d\n", LOG_LVL5, node_params.srg);
		printf("%s node_params.non_srg_obss_pd = %f dBm\n", LOG_LVL5, ConvertPower(PW_TO_DBM,node_params.non_srg_obss_pd));
		printf("%s node_params.srg_obss_pd = %f dBm\n", LOG_LVL5, ConvertPower(PW_TO_DBM,node_params.srg_obss_pd));
	}
	if(info_detail_level > INFO_DETAIL_LEVEL_0 && node_params.node_type == NODE_TYPE_AP){
		printf("%s wlan:\n", LOG_LVL4);
		printf("%s wlan code = %s\n", LOG_LVL5, wlan.wlan_code.c_str());
		printf("%s wlan id = %d\n", LOG_LVL5, wlan.wlan_id);
		printf("%s wlan AP id = %d\n", LOG_LVL5, wlan.ap_id);
		printf("%s Identifiers of STAs in WLAN (total number of STAs = %d): ", LOG_LVL5, wlan.num_stas);
		wlan.PrintStaIds();
	}
	if(info_detail_level > INFO_DETAIL_LEVEL_1){
		printf("%s node_params.backoff_type = %d\n", LOG_LVL4, node_params.backoff_type);
		printf("%s node_params.cw_adaptation = %d\n", LOG_LVL4, node_params.cw_adaptation);
		printf("%s [node_params.cw_min_default - node_params.cw_max_default] = [%d-%d]\n", LOG_LVL4, node_params.cw_min_default, node_params.cw_max_default);
		printf("%s node_params.cw_stage_max = %d\n", LOG_LVL4, node_params.cw_stage_max);
		printf("%s node_params.central_frequency = %f Hz (%f GHz)\n", LOG_LVL4, node_params.central_frequency, node_params.central_frequency * pow(10,-9));
		printf("%s node_params.capture_effect = %f [linear] (%f dB)\n", LOG_LVL4, node_params.capture_effect, ConvertPower(LINEAR_TO_DB, node_params.capture_effect));
		printf("%s Constant PER = %f\n", LOG_LVL4, node_params.constant_per);
	}
}

/**
 * Write Node's information
 * @param "node_logger" [type Logger]: logger object that writes information into a file
 * @param "info_detail_level" [type int]: level of detail of the info printed
 * @param "header_str" [type std::string]: header string
 */
void Node :: WriteNodeInfo(Logger node_logger, int info_detail_level, std::string header_str){

	fprintf(node_logger.file, "%s Node %s info:\n", header_str.c_str(), node_params.node_code.c_str());
	fprintf(node_logger.file, "%s - node_params.node_id = %d\n", header_str.c_str(), node_params.node_id);
	fprintf(node_logger.file, "%s - node_params.node_type = %d\n", header_str.c_str(), node_params.node_type);
	fprintf(node_logger.file, "%s - position = (%.2f, %.2f, %.2f)\n", header_str.c_str(), node_params.x, node_params.y, node_params.z);
	fprintf(node_logger.file, "%s - node_params.current_primary_channel = %d\n", header_str.c_str(), node_params.current_primary_channel);
	fprintf(node_logger.file, "%s - node_params.min_channel_allowed = %d\n", header_str.c_str(), node_params.min_channel_allowed);
	fprintf(node_logger.file, "%s - node_params.max_channel_allowed = %d\n", header_str.c_str(), node_params.max_channel_allowed);
	fprintf(node_logger.file, "%s - node_params.current_dcb_policy = %d\n", header_str.c_str(), node_params.current_dcb_policy);
	fprintf(node_logger.file, "%s - spatial_reuse_enabled = %d\n", header_str.c_str(), (node_params.bss_color>=0));
	if(node_params.bss_color>=0) {
		fprintf(node_logger.file, "%s node_params.bss_color = %d\n", header_str.c_str(), node_params.bss_color);
		fprintf(node_logger.file, "%s node_params.srg = %d\n", header_str.c_str(), node_params.srg);
		fprintf(node_logger.file, "%s node_params.non_srg_obss_pd = %f dBm\n", header_str.c_str(), ConvertPower(PW_TO_DBM,node_params.non_srg_obss_pd));
		fprintf(node_logger.file, "%s node_params.srg_obss_pd = %f dBm\n", header_str.c_str(), ConvertPower(PW_TO_DBM,node_params.srg_obss_pd));
	}

	if(info_detail_level > INFO_DETAIL_LEVEL_0){
		wlan.WriteWlanInfo(node_logger, header_str);
	}

	if(info_detail_level > INFO_DETAIL_LEVEL_1){
		fprintf(node_logger.file, "%s - [node_params.cw_min_default - node_params.cw_max_default] = [%d-%d]\n", header_str.c_str(), node_params.cw_min_default, node_params.cw_max_default);
		fprintf(node_logger.file, "%s - node_params.cw_stage_max = %d\n", header_str.c_str(), node_params.cw_stage_max);
		fprintf(node_logger.file, "%s - node_params.tx_power_default = %f pW\n", header_str.c_str(), node_params.tx_power_default);
		fprintf(node_logger.file, "%s - node_params.sensitivity_default = %f pW\n", header_str.c_str(), node_params.sensitivity_default);
	}

}

/**
 * Write Node's configuration
 * @param "node_logger" [type Logger]: logger object that writes information into a file
 * @param "header_str" [type std::string]: header string
 */
void Node :: WriteNodeConfiguration(Logger node_logger, std::string header_str){
	fprintf(node_logger.file, "%s Configuration %s info:\n", header_str.c_str(), node_params.node_code.c_str());
	fprintf(node_logger.file, "%s - current_primary = %d\n", header_str.c_str(), node_params.current_primary_channel);
	if(sr_state.spatial_reuse_enabled) {
		fprintf(node_logger.file, "%s - current_pd (OBSS/PD) = %f (%f dBm)\n", header_str.c_str(), node_params.non_srg_obss_pd, ConvertPower(PW_TO_DBM,node_params.non_srg_obss_pd));
	} else {
		fprintf(node_logger.file, "%s - current_pd = %f (%f dBm)\n", header_str.c_str(), current_pd, ConvertPower(PW_TO_DBM,current_pd));
	}
	fprintf(node_logger.file, "%s - current_tx_power = %f (%f dBm)\n", header_str.c_str(), current_tx_power, ConvertPower(PW_TO_DBM,current_tx_power));
	fprintf(node_logger.file, "%s - node_params.current_max_bandwidth = %d\n", header_str.c_str(), node_params.current_max_bandwidth);
}

/**
 * Write a given configuration
 * @param "node_logger" [type Logger]: logger object that writes information into a file
 * @param "header_str" [type std::string]: header string
 * @param "new_configuration" [type Configuration]: configuration to be written
 */
void Node :: WriteReceivedConfiguration(Logger node_logger, std::string header_str, Configuration new_configuration) {
	fprintf(node_logger.file, "%s Received Configuration:\n", header_str.c_str());
	fprintf(node_logger.file, "%s - selected_primary_channel = %d\n", header_str.c_str(), new_configuration.selected_primary_channel);
	fprintf(node_logger.file, "%s - selected_pd = %f (%f dBm)\n", header_str.c_str(), new_configuration.selected_pd, ConvertPower(PW_TO_DBM,new_configuration.selected_pd));
	fprintf(node_logger.file, "%s - current_tx_power = %f (%f dBm)\n", header_str.c_str(), new_configuration.selected_tx_power, ConvertPower(PW_TO_DBM,new_configuration.selected_tx_power));
	fprintf(node_logger.file, "%s - selected_max_bandwidth = %d\n", header_str.c_str(), new_configuration.selected_max_bandwidth);
}

/**
 * Print Node's configuration
 */
void Node :: PrintNodeConfiguration(){
	printf("Node%d - Configuration info:\n", node_params.node_id);
	printf(" - current_pd = %f (%f dBm)\n", current_pd, ConvertPower(PW_TO_DBM,current_pd));
	printf(" - current_tx_power = %f (%f dBm)\n", current_tx_power, ConvertPower(PW_TO_DBM,current_tx_power));
}

/**
 * Print the progress bar of the Komondor simulation (trigger-based operation)
 */
void Node :: PrintProgressBar(trigger_t &){
	// if(node_params.print_node_logs) printf("* %d %% *\n", progress_bar_counter * PROGRESS_BAR_DELTA);
	printf("* %d %% *\n", progress_bar_counter * PROGRESS_BAR_DELTA);
	trigger_sim_time.Set(RoundToDigits(SimTime() + node_params.simulation_time_komondor / (100/PROGRESS_BAR_DELTA),15));
	// End progress bar
	if(node_params.node_id == 0 && progress_bar_counter == (100/PROGRESS_BAR_DELTA)-1){
		trigger_sim_time.Set(RoundToDigits(SimTime() + node_params.simulation_time_komondor/(100/PROGRESS_BAR_DELTA) - MIN_VALUE_C_LANGUAGE,15));
	}
	++progress_bar_counter;
}

/**
 * Print or write final statistics of the given node
 * @param "write_or_print" [type int]: variable indicating whether to print or write logs
 */
void Node :: PrintOrWriteNodeStatistics(int write_or_print){
	// Process statistics
	double data_packets_lost_percentage (0);
	double rts_cts_lost_percentage (0);
	double tx_init_failure_percentage (0);
	double rts_lost_bo_percentage (0);

	if (node_stats.num_delay_measurements > 0) node_stats.average_delay = node_stats.sum_delays / (double) node_stats.num_delay_measurements;
	if (node_stats.flag_measure_rho && node_stats.num_measures_rho > 0) node_stats.average_rho = (double) node_stats.num_measures_rho_accomplished/(double) node_stats.num_measures_rho;
	if (node_stats.num_measures_utilization > 0) node_stats.average_utilization = (double) node_stats.num_measures_buffer_with_packets / (double) node_stats.num_measures_utilization;
	tx_init_failure_percentage = double(node_stats.num_tx_init_not_possible * 100)/double(node_stats.num_tx_init_tried);
	if (node_stats.data_packets_sent > 0) data_packets_lost_percentage = double(node_stats.data_packets_lost * 100)/double(node_stats.data_packets_sent);
	if (node_stats.rts_cts_sent > 0){
		rts_cts_lost_percentage = double(node_stats.rts_cts_lost * 100)/double(node_stats.rts_cts_sent);
		rts_lost_bo_percentage = double(node_stats.rts_lost_slotted_bo *100)/double(node_stats.rts_cts_sent);
		node_stats.prob_slotted_bo_collision = double(rts_lost_bo_percentage / double(100));
	}
	if (node_stats.num_packets_generated > 1){
		node_stats.generation_drop_ratio = node_stats.num_packets_dropped * 100/ node_stats.num_packets_generated;
	}
	node_stats.throughput = ((double) node_stats.data_frames_acked * node_params.frame_length) / SimTime();
	for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
		node_stats.bandwidth_used_txing += (node_stats.total_time_channel_busy_per_channel[c]/SimTime()) * 20;
	}
	node_stats.last_throughput = ((double) node_stats.last_data_frames_acked * node_params.frame_length) / node_stats.last_measurements_window;
	if (node_stats.last_num_delay_measurements > 0) node_stats.last_average_delay = node_stats.last_sum_delays / (double) node_stats.last_num_delay_measurements;

	node_stats.average_waiting_time = node_stats.sum_waiting_time / (double) node_stats.num_average_waiting_time_measurements;
	node_stats.expected_backoff = node_stats.expected_backoff / (double) node_stats.num_new_backoff_computations;

	switch(write_or_print){

		case PRINT_LOG:{

			if (node_is_transmitter && node_params.print_node_logs) {
				printf("------- %s (N%d) ------\n", node_params.node_code.c_str(), node_params.node_id);
				// Throughput
				printf("%s Throughput = %f Mbps (%.2f pkt/s)\n", LOG_LVL2,
					node_stats.throughput * pow(10,-6),
					node_stats.throughput / (node_params.frame_length * limited_num_packets_aggregated));
				// Delay
				printf("%s Average delay from %d measurements = %f s (%.2f ms)\n", LOG_LVL2,
					node_stats.num_delay_measurements, node_stats.average_delay, node_stats.average_delay * 1000);
				// Rho
				printf("%s Average rho = %f (%.2f %%)\n", LOG_LVL2,
					node_stats.average_rho, node_stats.average_rho * 100);
				printf("%s %d/%d\n", LOG_LVL3,
					node_stats.num_measures_rho_accomplished, node_stats.num_measures_rho);
				// Utilization
				printf("%s Average utilization = %f (%.2f %%)\n", LOG_LVL2,
					node_stats.average_utilization, node_stats.average_utilization * 100);
				printf("%s %d/%d\n", LOG_LVL3,
					node_stats.num_measures_buffer_with_packets, node_stats.num_measures_utilization);
				// RTS/CTS sent and lost
				printf("%s RTS/CTS sent/lost = %d/%d  (%.2f %% lost)\n",
					LOG_LVL2, node_stats.rts_cts_sent, node_stats.rts_cts_lost, rts_cts_lost_percentage);
				// RTS/CTS sent and lost
				printf("%s RTS lost due to slotted BO = %d (%f %%)\n",
					LOG_LVL3, node_stats.rts_lost_slotted_bo, rts_lost_bo_percentage);
				// Data packets sent and lost
				printf("%s Data packets sent = %d - ACKed = %d -  Lost = %d  (%f %% lost)\n",
					LOG_LVL2, node_stats.data_packets_sent, node_stats.data_packets_acked, node_stats.data_packets_lost, data_packets_lost_percentage);
				printf("%s Frames ACKed = %d, Av. frames sent per packet = %.2f\n",
					LOG_LVL2, node_stats.data_frames_acked, (double) node_stats.data_frames_acked/node_stats.data_packets_acked);
				// Data packets sent and lost
				printf("%s Buffer: packets generated = %.0f (%.2f pkt/s) - Packets dropped = %.0f  (%f %% drop ratio)\n",
					LOG_LVL2, node_stats.num_packets_generated, node_stats.num_packets_generated / SimTime(), node_stats.num_packets_dropped, node_stats.generation_drop_ratio);
				if(TRAFFIC_POISSON_BURST){
					printf("%s Buffer: num bursts = %d\n",
						LOG_LVL2,
						num_bursts);
				}
				// Number of trials to transmit
				printf("%s node_stats.num_tx_init_tried = %d - node_stats.num_tx_init_not_possible = %d (%f %% failed)\n",
					LOG_LVL2, node_stats.num_tx_init_tried, node_stats.num_tx_init_not_possible, tx_init_failure_percentage);
				// Total airtime vs successful airtime
				double total_airtime(0);
				double successful_airtime(0);
				for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
					total_airtime += node_stats.total_time_transmitting_per_channel[c];
					successful_airtime += node_stats.total_time_transmitting_per_channel[c] - node_stats.total_time_lost_per_channel[c];
				}
				printf("%s Airtime = %.2f s (%.2f %%) - Successful airtime = %.2f s (%.2f %%)\n",
					LOG_LVL2, total_airtime, (total_airtime * 100 /SimTime()), successful_airtime, (successful_airtime * 100 /SimTime()));
				// Time EFFECTIVELY transmitting in a given number of channels (no losses)
				printf("%s Successful airtime in channels of width N:", LOG_LVL3);
				for(int n = 0; n < node_params.num_channels_allowed; ++n){
					printf("\n%s - %d: %f s (%.2f %%)",
							LOG_LVL3, (int) pow(2,n),
							node_stats.total_time_transmitting_in_num_channels[n] - node_stats.total_time_lost_in_num_channels[n],
							((node_stats.total_time_transmitting_in_num_channels[n] -
									node_stats.total_time_lost_in_num_channels[n])) * 100 /SimTime());
					if((int) pow(2,n) == NUM_CHANNELS_KOMONDOR) break;
				}
				printf("\n");
				// Time EFFECTIVELY transmitting in each of the channels (no losses)
				printf("%s Total Airtime / Successful airtime per channel:", LOG_LVL3);
				double time_effectively_txing;
				for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
					time_effectively_txing = node_stats.total_time_transmitting_per_channel[c] -
						node_stats.total_time_lost_per_channel[c];
					printf("\n%s - %d: %.2f s (%.2f %%) / %.2f s (%.2f %%)",
						LOG_LVL3, c, node_stats.total_time_channel_busy_per_channel[c],
						(node_stats.total_time_channel_busy_per_channel[c] * 100 /SimTime()),
						time_effectively_txing, time_effectively_txing * 100 /SimTime());
				}
				printf("\n%s Average bandwidth used for transmitting = %.2f MHz / %d MHz (%.2f %%)\n",
					LOG_LVL3,
					node_stats.bandwidth_used_txing,
					node_params.num_channels_allowed * 20,
					node_stats.bandwidth_used_txing * 100 / (node_params.num_channels_allowed * 20));
				// Time tx trials in each number of channels
				printf("%s Number of tx trials per number of channels:", LOG_LVL3);
				for(int n = 0; n < NUM_CHANNELS_KOMONDOR; ++n){
					printf("\n%s - %d: %d (%.2f %%)",
						LOG_LVL3, (int) pow(2,n),
						node_stats.num_trials_tx_per_num_channels[n],
						(((double) node_stats.num_trials_tx_per_num_channels[n] * 100) / (double) (node_stats.rts_cts_sent)));

					if((int) pow(2,n) == NUM_CHANNELS_KOMONDOR) break;
				}
				printf("\n");
				// Number of TX initiations that have been not possible due to channel state and DCB model
				printf("%s node_stats.num_tx_init_not_possible = %d\n", LOG_LVL2, node_stats.num_tx_init_not_possible);
//				// Hidden nodes
//				printf("%s Total number of hidden nodes: %d\n", LOG_LVL2, hidden_nodes_number);
//				printf("%s Hidden nodes list: ", LOG_LVL2);
//				for(int i = 0; i < node_params.total_nodes_number; ++i){
//					printf("%d  ", hidden_nodes_list[i]);
//				}
//				printf("\n");
//
//				printf("%s Times a node was implied in a collision by hidden node: ",LOG_LVL2);
//				for(int i=0; i < node_params.total_nodes_number; ++i) {
//					printf("%d ", potential_hidden_nodes[i]);
//				}

				// TODO: Print mean MCS used per STA

				printf("%s node_stats.times_went_to_nav = %d\n", LOG_LVL2, node_stats.times_went_to_nav);
				printf("%s node_stats.time_in_nav = %f (%.2f %% of the total time)\n", LOG_LVL2,
					node_stats.time_in_nav, (node_stats.time_in_nav/node_params.simulation_time_komondor*100));

				printf("%s node_stats.average_waiting_time = %f (%f slots)\n", LOG_LVL2, node_stats.average_waiting_time, node_stats.average_waiting_time / SLOT_TIME);
				printf("%s Expected BO = %f (%f slots)\n", LOG_LVL2, node_stats.expected_backoff, node_stats.expected_backoff / SLOT_TIME);

				// REPORT PER EACH STA
				printf("%s Per-STA report:\n", LOG_LVL2);
				// Throughput
				printf("%s Throughput: {", LOG_LVL3);
				for(int n = 0; n < wlan.num_stas; ++n){
					node_stats.throughput_per_sta[n] = ((double)node_stats.data_frames_acked_per_sta[n] * (double)node_params.frame_length) / SimTime();
					printf("%.2f Mbps",  node_stats.throughput_per_sta[n] * pow(10,-6));
					if(n<wlan.num_stas-1) printf(", ");
				}
				printf("}\n%s RTS/CTS sent/lost: {", LOG_LVL3);
				for(int n = 0; n < wlan.num_stas; ++n){
					printf("%d/%d (%.2f %%)", node_stats.rts_cts_sent_per_sta[n], node_stats.rts_cts_lost_per_sta[n],
						double(node_stats.rts_cts_lost_per_sta[n] * 100)/double(node_stats.rts_cts_sent_per_sta[n]));
					if(n<wlan.num_stas-1) printf(", ");
				}
				printf("}\n%s Data packets sent/lost: {", LOG_LVL3);
				for(int n = 0; n < wlan.num_stas; ++n){
					printf("%d/%d (%.2f %%)", node_stats.data_packets_sent_per_sta[n], node_stats.data_packets_lost_per_sta[n],
							double(node_stats.data_packets_lost_per_sta[n] * 100)/double(node_stats.data_packets_sent_per_sta[n]));
					if(n<wlan.num_stas-1) printf(", ");
				}
				printf("}");
				printf("\n\n");

			}
			break;
		}

		case WRITE_LOG:{

			if (node_params.save_node_logs){

				if (node_is_transmitter) {
					// Throughput
					fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Throughput = %f Mbps\n",
						SimTime(), node_params.node_id, node_state, LOG_C02, LOG_LVL2, node_stats.throughput * pow(10,-6));

					// Data packets sent and lost
					fprintf(node_logger.file,
						"%.15f;N%d;S%d;%s;%s Data packets sent: %d\n",
						SimTime(), node_params.node_id, node_state, LOG_C03, LOG_LVL2, node_stats.data_packets_sent);
					fprintf(node_logger.file,
						"%.15f;N%d;S%d;%s;%s Data packets lost: %d\n",
						SimTime(), node_params.node_id, node_state, LOG_C04, LOG_LVL2, node_stats.data_packets_lost);
					fprintf(node_logger.file,
						"%.15f;N%d;S%d;%s;%s Loss ratio: %f\n",
						SimTime(), node_params.node_id, node_state, LOG_C05, LOG_LVL2, data_packets_lost_percentage);

					// Time EFFECTIVELY transmitting in a given number of channels (no losses)
					fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Time EFFECTIVELY transmitting in N channels: ",
						SimTime(), node_params.node_id, node_state, LOG_C06, LOG_LVL2);
					for(int n = 0; n < node_params.num_channels_allowed; ++n){
						fprintf(node_logger.file, "(%d) %f  ",
							n+1, node_stats.total_time_transmitting_in_num_channels[n] - node_stats.total_time_lost_in_num_channels[n]);
					}
					fprintf(node_logger.file, "\n");

					// Time EFFECTIVELY transmitting in each of the channels (no losses)
					fprintf(node_logger.file,
						"%.15f;N%d;S%d;%s;%s Time EFFECTIVELY transmitting in each channel: ",
						SimTime(), node_params.node_id, node_state, LOG_C07, LOG_LVL2);
					for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
						fprintf(node_logger.file, "(#%d) %f ",
							c, node_stats.total_time_transmitting_per_channel[c] - node_stats.total_time_lost_per_channel[c]);
					}
					fprintf(node_logger.file, "\n");

					// Time LOST transmitting in a given number of channels
					fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Time LOST transmitting in N channels: ",
						SimTime(), node_params.node_id, node_state, LOG_C08, LOG_LVL2);
					for(int n = 0; n < node_params.num_channels_allowed; ++n){
						fprintf(node_logger.file, "(%d) %f  ", n+1, node_stats.total_time_lost_in_num_channels[n]);
					}
					fprintf(node_logger.file, "\n");

					// Time LOST transmitting in each of the channels
					fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Time LOST transmitting in each channel: ",
						SimTime(), node_params.node_id, node_state, LOG_C09, LOG_LVL2);
					for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
						fprintf(node_logger.file, "(#%d) %f ", c, node_stats.total_time_lost_per_channel[c]);
					}
					fprintf(node_logger.file, "\n");

					// Number of TX initiations that have been not possible due to channel state and DCB model
					fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s node_stats.num_tx_init_not_possible = %d\n",
						SimTime(), node_params.node_id, node_state, LOG_C09, LOG_LVL2, node_stats.num_tx_init_not_possible);

					// Spectrum utilization
					fprintf(node_logger.file,"%s Time occupying the spectrum in each channel:", LOG_LVL3);
					for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
						fprintf(node_logger.file,"\n%s - %d = %.2f s (%.2f %%)",
							LOG_LVL3, c, node_stats.total_time_channel_busy_per_channel[c],
							(node_stats.total_time_channel_busy_per_channel[c] * 100 /SimTime()));
					}

					fprintf(node_logger.file,"\n%s - Average bandwidth used for transmitting = %.2f MHz / %d MHz (%.2f %%)\n",
						LOG_LVL4, node_stats.bandwidth_used_txing, node_params.num_channels_allowed * 20, node_stats.bandwidth_used_txing * 100 / (node_params.num_channels_allowed * 20));

					fprintf(node_logger.file,"\n");

				}

//				// Hidden nodes
//				int hidden_nodes_number = 0;
//				for(int n = 0; n < node_params.total_nodes_number; ++n){
//					if(hidden_nodes_list[n]) hidden_nodes_number++;
//				}
//				fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Total hidden nodes: %d\n",
//						SimTime(), node_params.node_id, node_state, LOG_C10, LOG_LVL2, hidden_nodes_number);
//
//				fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Hidden nodes list: ",
//						SimTime(), node_params.node_id, node_state, LOG_C11, LOG_LVL2);
//				for(int i = 0; i < node_params.total_nodes_number; ++i){
//					fprintf(node_logger.file, "%d  ", hidden_nodes_list[i]);
//				}
			}
			break;
		}
	}
}

/**
 * Save the performance observed during the simulation to the "simulation_performance" object
 */
void Node :: SaveSimulationPerformance() {

	if(node_params.node_id == 0) simulation_performance.sum_time_channel_idle = node_stats.sum_time_channel_idle;

	simulation_performance.num_stas = wlan.num_stas;
	simulation_performance.last_measurements_window = node_stats.last_measurements_window;

	// Throughput
	simulation_performance.throughput = node_stats.throughput;
	simulation_performance.throughput_loss = node_stats.throughput_loss;

	// Frames
	simulation_performance.data_packets_acked = node_stats.data_packets_acked;
	simulation_performance.data_frames_acked = node_stats.data_frames_acked;
	simulation_performance.data_packets_sent = node_stats.data_packets_sent;
	simulation_performance.data_packets_lost = node_stats.data_packets_lost;
	simulation_performance.rts_cts_sent = node_stats.rts_cts_sent;
	simulation_performance.rts_cts_lost = node_stats.rts_cts_lost;
	simulation_performance.rts_lost_slotted_bo = node_stats.rts_lost_slotted_bo;

	// Buffer
	simulation_performance.num_packets_generated = node_stats.num_packets_generated;
	simulation_performance.num_packets_dropped = node_stats.num_packets_dropped;
	simulation_performance.num_delay_measurements = node_stats.num_delay_measurements;
	simulation_performance.sum_delays = node_stats.sum_delays;
	simulation_performance.average_delay = node_stats.average_delay;
	simulation_performance.average_rho = node_stats.average_rho;
	simulation_performance.average_utilization = node_stats.average_utilization;
	simulation_performance.generation_drop_ratio = node_stats.generation_drop_ratio;

	// Channel occupancy
	simulation_performance.expected_backoff = node_stats.expected_backoff;
	simulation_performance.num_new_backoff_computations = node_stats.num_new_backoff_computations;
	simulation_performance.num_trials_tx_per_num_channels = node_stats.num_trials_tx_per_num_channels;
	simulation_performance.bandwidth_used_txing = node_stats.bandwidth_used_txing;
	simulation_performance.total_time_transmitting_per_channel = node_stats.total_time_transmitting_per_channel;
	simulation_performance.total_time_transmitting_in_num_channels = node_stats.total_time_transmitting_in_num_channels;
	simulation_performance.total_time_lost_per_channel = node_stats.total_time_lost_per_channel;
	simulation_performance.total_time_lost_in_num_channels = node_stats.total_time_lost_in_num_channels;
	simulation_performance.total_time_channel_busy_per_channel = node_stats.total_time_channel_busy_per_channel;
	simulation_performance.time_in_nav = node_stats.time_in_nav;

	// Per-STA statistics
	if(node_params.node_type == NODE_TYPE_AP) {
		simulation_performance.throughput_per_sta = node_stats.throughput_per_sta;
		simulation_performance.data_packets_sent_per_sta = node_stats.data_packets_sent_per_sta;
		simulation_performance.rts_cts_sent_per_sta = node_stats.rts_cts_sent_per_sta;
		simulation_performance.data_packets_lost_per_sta = node_stats.data_packets_lost_per_sta;
		simulation_performance.rts_cts_lost_per_sta = node_stats.rts_cts_lost_per_sta;
		simulation_performance.data_packets_acked_per_sta = node_stats.data_packets_acked_per_sta;
		simulation_performance.data_frames_acked_per_sta = node_stats.data_frames_acked_per_sta;
		simulation_performance.rssi_list_per_sta = rssi_per_sta;
		UpdatePerformanceMeasurements();
		simulation_performance.max_received_power_in_ap_per_wlan = max_received_power_in_ap_per_wlan;
//		for (int i = 0; i < node_params.total_wlans_number; i++) {
//			printf("simulation_performance.rssi_list[%d] = %f dBm\n",i,ConvertPower(PW_TO_DBM,simulation_performance.rssi_list[i]));
//		}
	}
	simulation_performance.received_power_array = received_power_array;

	// Channel access
	simulation_performance.average_waiting_time = node_stats.average_waiting_time;
	simulation_performance.num_tx_init_tried = node_stats.num_tx_init_tried;
	simulation_performance.num_tx_init_not_possible = node_stats.num_tx_init_not_possible;
	simulation_performance.prob_slotted_bo_collision = node_stats.prob_slotted_bo_collision;

	// Other
	configuration.non_srg_obss_pd = node_params.non_srg_obss_pd;

	// Last seen performance (end of the simulation)
	simulation_performance.last_throughput = node_stats.last_throughput;
	simulation_performance.last_average_delay = node_stats.last_average_delay;
	simulation_performance.last_total_time_transmitting_per_channel = node_stats.last_total_time_transmitting_per_channel;
	simulation_performance.last_total_time_lost_per_channel = node_stats.last_total_time_lost_per_channel;
	simulation_performance.last_average_access_delay = node_stats.last_sum_waiting_time/(double)node_stats.last_num_average_waiting_time_measurements;

	node_stats.sum_waiting_time = node_stats.sum_waiting_time + SimTime() - node_stats.timestamp_new_trial_started;
		++node_stats.num_average_waiting_time_measurements;

}

#endif /* NODE_STATISTICS_METHODS_H */
