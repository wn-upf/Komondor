/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * node_mcs_methods.h: NACK/MCS-negotiation and traffic inport method implementations.
 *
 * NOTE: This file is an implementation fragment. It must be included from node.h
 *   after the Node class definition, not included directly.
 *
 * MCS selection free functions (SelectMCSResponse, ComputeEbToNoise) live in
 *   methods/phy/mcs_methods.h, included below.
 *
 * Node methods defined here:
 *   - Node::InportNackReceived
 *   - Node::InportMCSRequestReceived
 *   - Node::InportMCSResponseReceived
 *   - Node::InportNewPacketGenerated
 */

#ifndef NODE_MCS_METHODS_H
#define NODE_MCS_METHODS_H

#include "../phy/mcs_methods.h"

// ===========================================================================
// Node MCS / NACK inport methods
// ===========================================================================

void Node :: InportNackReceived(LogicalNack &logical_nack){

	int nack_reason;

//	LOGS(node_params.save_node_logs,node_logger.file,
//			"%.15f;N%d;S%d;%s;%s InportNackReceived(): N%d to N%d (A) and N%d (B)\n",
//			SimTime(), node_params.node_id, node_state, LOG_H00, LOG_LVL1, logical_nack.source_id,
//			logical_nack.node_id_a, logical_nack.node_id_b);

	// If node is involved in the NACK
	if(logical_nack.source_id != node_params.node_id &&
			(node_params.node_id == logical_nack.node_id_a || node_params.node_id == logical_nack.node_id_b)){

		LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s NACK of packet #%d received from N%d sent to a:N%d (and b:N%d) with reason %d\n",
				SimTime(), node_params.node_id, node_state, LOG_H00, LOG_LVL2, logical_nack.packet_id, logical_nack.source_id,
				logical_nack.node_id_a, logical_nack.node_id_b, logical_nack.loss_reason);

		// Process logical NACK for statistics purposes
		nack_reason = ProcessNack(logical_nack, node_params.node_id, node_logger, node_state, node_params.save_node_logs,
			SimTime(), node_stats.nacks_received, node_params.total_nodes_number, nodes_transmitting);

		if(nack_reason == PACKET_LOST_BO_COLLISION){
			++ node_stats.rts_lost_slotted_bo;

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s ++++++++++++++++++++++++++++++++\n",
				SimTime(), node_params.node_id, node_state, LOG_H00, LOG_LVL2);

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s node_stats.rts_lost_slotted_bo ++\n",
				SimTime(), node_params.node_id, node_state, LOG_H00, LOG_LVL2);

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s ++++++++++++++++++++++++++++++++\n",
				SimTime(), node_params.node_id, node_state, LOG_H00, LOG_LVL2);
		}

	} else {	// Node is the NACK transmitter, do nothing

//		LOGS(node_params.save_node_logs,node_logger.file,
//				"%.15f;N%d;S%d;%s;%s NACK of packet #%d sent to a) N%d and b) N%d with reason %d\n",
//				SimTime(), node_params.node_id, node_state, LOG_H00, LOG_LVL2, logical_nack.packet_id,
//				logical_nack.node_id_a, logical_nack.node_id_b, logical_nack.loss_reason);

	}

	// LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;G01;%s InportNackReceived() END\n", SimTime(), node_params.node_id, LOG_LVL1);
}

/**
 * Called when some node asks (logically) the receiver for the possible MCS configurations to be used based on the power sensed at the receiver
 * @param "notification" [type Notification]: notification containing the MCS request
 */
void Node :: InportMCSRequestReceived(Notification &notification){

	if(notification.destination_id == node_params.node_id) {	// If node IS THE DESTINATION

		LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s MCS request received from N%d\n",
			SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL1, notification.source_id);

//		// Compute distance and power received from transmitter
//		double distance = ComputeDistance(node_params.x, node_params.y, node_params.z, notification.tx_info.x,
//			notification.tx_info.y, notification.tx_info.z);

//		double power_rx_interest (ComputePowerReceived(distances_array[notification.source_id],
//			notification.tx_info.tx_power, rx_gain,
//			node_params.central_frequency, node_params.path_loss_model));

		// Update 'power received' array in case a new tx power is used
		if (notification.tx_info.flag_change_in_tx_power) {
			received_power_array[notification.source_id] =
				ComputePowerReceived(distances_array[notification.source_id],
				notification.tx_info.tx_power, node_params.central_frequency, node_params.path_loss_model);
		}

		LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s I am at distance: %.2f m (sensing P_rx = %.2f dBm)\n",
			SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL2,
			distances_array[notification.source_id], ConvertPower(PW_TO_DBM,
			received_power_array[notification.source_id]));

		// Select the modulation according to the SINR perceived corresponding to incoming transmitter
		SelectMCSResponse(mcs_response, received_power_array[notification.source_id]);

		LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s mcs_response for 1, 2, 4 and 8 channels: ",
			SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL3);

		PrintOrWriteArrayInt(WRITE_LOG, node_params.save_node_logs,
			node_params.print_node_logs, node_logger, mcs_response, 4);

		// Fill and send MCS response
		Notification response_mcs  = GenerateNotification(PACKET_TYPE_MCS_RESPONSE, notification.source_id,
			-1, -1, -1, TX_DURATION_NONE);

		outportAnswerTxModulation(response_mcs);

	} else { 	// If node IS NOT THE DESTINATION
		// Do nothing
	}
}

/**
 * Called when some node answers back to a MCS request
 * @param "notification" [type Notification]: notification containing the MCS response
 */
void Node :: InportMCSResponseReceived(Notification &notification){

	if(notification.destination_id == node_params.node_id) {	// If node IS THE DESTINATION

		LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s InportMCSResponseReceived()\n",
				SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL1);

		int ix_aux (current_destination_id - wlan.list_sta_id[0]);	// Auxiliary index for correcting the node id offset

		LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s MCS per number of channels: ",
			SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL2);

		// Set receiver modulation to the received one
		for (int i = 0; i < NUM_OPTIONS_CHANNEL_LENGTH; ++i){
			if (sr_state.spatial_reuse_enabled && sr_state.txop_sr_identified &&
					notification.tx_info.modulation_schemes[i] == MODULATION_FORBIDDEN) {
				// Force to use the minimum MCS in case of applying the SR operation and receiving the forbidden MCS
				mcs_per_node[ix_aux][i] = MODULATION_BPSK_1_2;
			} else {
				mcs_per_node[ix_aux][i] = notification.tx_info.modulation_schemes[i];
			}
			LOGS(node_params.save_node_logs,node_logger.file, "%d ", mcs_per_node[ix_aux][i]);
		}

		// Update performance measurements
		if (first_time_requesting_mcs) {
			double max_achievable_bits_ofdm_sym (GetNumberSubcarriers(node_params.max_channel_allowed - node_params.min_channel_allowed + 1) *
				Mcs_array::modulation_bits[mcs_per_node[ix_aux][(int) log2(node_params.max_channel_allowed-node_params.min_channel_allowed + 1)]-1] *
				Mcs_array::coding_rates[mcs_per_node[ix_aux][(int) log2(node_params.max_channel_allowed-node_params.min_channel_allowed + 1)]-1] *
				IEEE_AX_SU_SPATIAL_STREAMS);
			//	double max_achievable_bits_ofdm_sym (GetNumberSubcarriers(NUM_CHANNELS_KOMONDOR) *
			//	Mcs_array::modulation_bits[mcs_per_node[ix_aux][(int) log2(NUM_CHANNELS_KOMONDOR)]-1] *
			//	Mcs_array::coding_rates[mcs_per_node[ix_aux][(int) log2(NUM_CHANNELS_KOMONDOR)]-1] *
			//	IEEE_AX_SU_SPATIAL_STREAMS);
			double max_achievable_throughput (max_achievable_bits_ofdm_sym / IEEE_AX_OFDM_SYMBOL_GI32_DURATION);
			performance_report.max_bound_throughput = max_achievable_throughput;
			first_time_requesting_mcs = FALSE;
			LOGS(node_params.save_node_logs,node_logger.file, "\n");
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s max_achievable_throughput (%d - %d) = %.1f Mbps "
				"(%d channel/s: Y_sc = %d, MCS %d: Y_m = %d, Y_c = %.2f)\n",
				SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL3,
				node_params.min_channel_allowed, node_params.max_channel_allowed, max_achievable_throughput * pow(10,-6),
				node_params.max_channel_allowed - node_params.min_channel_allowed + 1,
				GetNumberSubcarriers(current_right_channel - current_left_channel + 1),
				mcs_per_node[ix_aux][(int) log2(node_params.max_channel_allowed-node_params.min_channel_allowed + 1)]-1,
				Mcs_array::modulation_bits[mcs_per_node[ix_aux][(int) log2(node_params.max_channel_allowed-node_params.min_channel_allowed + 1)]-1],
				Mcs_array::coding_rates[mcs_per_node[ix_aux][(int) log2(node_params.max_channel_allowed-node_params.min_channel_allowed + 1)]-1]);
		}

		// printf("\n");

		// TODO: ADD LOGIC TO HANDLE WRONG SITUATIONS (cannot transmit over none of the channel combinations)
		if(mcs_per_node[ix_aux][0] == -1) {
			// CANNOT TX EVEN FOR 1 CHANNEL
			if(current_tx_power < ConvertPower(DBM_TO_PW,MAX_TX_POWER_DBM)) {
//				current_tx_power ++;
//				change_modulation_flag[ix_aux] = TRUE;
			} else {
				// NODE UNREACHABLE
				LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Unreachable node: transmissions to N%d are cancelled\n",
					SimTime(), node_params.node_id, node_state, LOG_G00, LOG_LVL3, current_destination_id);
				// TODO: unreachable_nodes[current_destination_id] = TRUE;
			}
		}

	} else {	// If node IS NOT THE DESTINATION
		// Do nothing
	}
}

/**
 * Called when a new packet is generated by the traffic generator (refer to "traffic_generator.h")
 */
void Node :: InportNewPacketGenerated(){

//	printf("N%d New packet received from the traffic generator!\n", node_params.node_id);

	if(node_is_transmitter){

		if(buffer.QueueSize() == 0){
			// - compute average waiting time to access the channel
			node_stats.timestamp_new_trial_started = SimTime();
		}

		if(node_params.traffic_model != TRAFFIC_POISSON_BURST) { // NON-BURST TRAFFIC (i.e., packet by packet)

			++ node_stats.num_packets_generated;
			// Update performance measurements
			++ performance_report.num_packets_generated;

			if (buffer.QueueSize() < PACKET_BUFFER_SIZE) {

				// Include new packet
				new_packet = null_notification;
				new_packet.timestamp_generated = SimTime();
				new_packet.packet_id = last_packet_generated_id;
				buffer.PutPacket(new_packet);

				LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s A new packet (id: %d) has been generated (queue: %d/%d)\n",
						SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL4,
						new_packet.packet_id, buffer.QueueSize(), PACKET_BUFFER_SIZE);

				// Attempt to restart BO only if node didn't have any packet before a new packet was generated
				if(node_state == STATE_SENSING && buffer.QueueSize() == 1) {

					if(trigger_end_backoff.Active()) {
						// Backoff already counting down: just refresh the cached remaining value.
						// Do NOT call ScheduleBackoffAfterDIFS() -- that would arm trigger_start_backoff
						// while trigger_end_backoff is still live. When PauseBackoff() later cancels
						// only trigger_start_backoff, trigger_end_backoff would fire in a non-SENSING
						// state, causing a simultaneous-transmission deadlock.
						ca_state.remaining_backoff =
							ComputeRemainingBackoff(node_params.backoff_type, trigger_end_backoff.GetTime() - SimTime());
					} else if (!trigger_start_backoff.Active()) {
						// No countdown in progress: start one now.
						int resume (HandleBackoff(RESUME_TIMER, &channel_power,
							node_params.current_primary_channel, current_pd, buffer.QueueSize()));
						if (resume) {
							ScheduleBackoffAfterDIFS();
						}
					}
					// else: DIFS already running; trigger_end_backoff will follow automatically.

				}

			} else {
				// Buffer overflow - new packet is lost
				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s A new packet (id: %d) has been dropped! (queue: %d/%d)\n",
					SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL4,
					last_packet_generated_id, buffer.QueueSize(), PACKET_BUFFER_SIZE);
				++ node_stats.num_packets_dropped;
				// Update performance measurements
				++ performance_report.num_packets_dropped;
			}

			++ last_packet_generated_id;

			// End of NON-BURST TRAFFIC

		} else {	// BURST TRAFFIC (i.e., burst of consecutive packets)

			++ num_bursts;

			int num_packets_generated_in_burst (burst_rate);

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s New traffic burst (#%d) generated %d packets\n",
				SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL4,
				num_bursts,
				num_packets_generated_in_burst);

			node_stats.num_packets_generated = node_stats.num_packets_generated + num_packets_generated_in_burst;

			for(int i = 0; i < num_packets_generated_in_burst; ++i){

				if (buffer.QueueSize() < PACKET_BUFFER_SIZE) {

					// Include new packet
					Notification new_packet;
					new_packet.timestamp_generated = SimTime();
					new_packet.packet_id = last_packet_generated_id;
					buffer.PutPacket(new_packet);

					LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s A new packet (id: %d) has been generated from burst %d (buffer queue: %d/%d)\n",
							SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL4,
							new_packet.packet_id,
							num_bursts,
							buffer.QueueSize(),
							PACKET_BUFFER_SIZE);

					// Attempt to restart BO only if node didn't have any packet before a new packet was generated
					if(node_state == STATE_SENSING && buffer.QueueSize() == 1) {

						if(trigger_end_backoff.Active()) ca_state.remaining_backoff =
								ComputeRemainingBackoff(node_params.backoff_type, trigger_end_backoff.GetTime() - SimTime());

						int resume (HandleBackoff(RESUME_TIMER, &channel_power, node_params.current_primary_channel,
							current_pd, buffer.QueueSize()));

						if (resume) {
							ScheduleBackoffAfterDIFS();
						}

					}

				} else {  // Buffer overflow - new packet is lost
					++node_stats.num_packets_dropped;
				}

				++last_packet_generated_id;

			}

		} // End of BURST TRAFFIC

	}
}

#endif /* NODE_MCS_METHODS_H */
