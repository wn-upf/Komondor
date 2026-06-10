/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * node_fsm_methods.h: FSM state handler implementations for the Node component.
 *
 * NOTE: This file is an implementation fragment. It must be included from
 *   node_impl.h (and thus indirectly from node.h) after the Node class
 *   definition, not included directly.
 *
 * Functions defined here (InportSomeNodeStartTX / InportSomeNodeFinishTX
 * dispatch + per-state handlers):
 *   - Node::HandleStartTX_StateSensing
 *   - Node::HandleStartTX_StateNav
 *   - Node::HandleStartTX_StateTxData
 *   - Node::HandleStartTX_StateRxData
 *   - Node::HandleStartTX_StateWaitAck
 *   - Node::HandleStartTX_StateWaitCts
 *   - Node::HandleStartTX_StateWaitData
 *   - Node::InportSomeNodeStartTX
 *   - Node::HandleFinishTX_StateSensing
 *   - Node::HandleFinishTX_StateRxData
 *   - Node::HandleFinishTX_StateTxData
 *   - Node::InportSomeNodeFinishTX
 */

#ifndef NODE_FSM_METHODS_H
#define NODE_FSM_METHODS_H

/**
 * Handle InportSomeNodeStartTX for STATE_SENSING
 * Called when some node (this one included) starts a transmission
 * @param "notification" [type Notification]: notification containing the information of the transmission start perceived
 */
void Node :: HandleStartTX_StateSensing(const Notification &notification) {

	if(notification.destination_id == node_params.node_id){	// Node IS THE DESTINATION

		// Update power received of interest
		power_rx_interest = power_received_per_node[notification.source_id];

		current_left_channel = notification.left_channel;
		current_right_channel = notification.right_channel;
		current_modulation = notification.modulation_id;

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
			SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3,
			notification.destination_id);

		// Issue #146 RTS/CTS BW indication
		// - If incoming packet is RTS or CTS sent to me, focus first just on the primary channel
		// - If RTS/CTS decodable in the primary, packet not lost.
		// - After decoding the RTS/CTS at the primary, perform CCA assessment in all the range
		// - So, if RTS/CTS: max_pw_interference is now referred just to primary channel interference
		// - Keep max_pw_interference for all range if DATA or ACK.

		// Compute interference from all active transmitters except the intended source.
		// Both RTS and DATA use the same sum-of-power_received_per_node approach so
		// that beamforming-corrected powers are used consistently for SINR, while
		// channel_power (raw) is preserved for CCA/NAV.
		ComputeMaxInterference(&max_pw_interference, &channel_max_interference,
			notification, node_state, power_received_per_node, &channel_power);

		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %.2f dBm - P_if = %.2f dBm - P_noise = %.2f dBm\n",
			SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
			channel_max_interference,
			ConvertPower(PW_TO_DBM, channel_power[channel_max_interference]),
			ConvertPower(PW_TO_DBM, power_rx_interest),
			ConvertPower(PW_TO_DBM, max_pw_interference),
			ConvertPower(PW_TO_DBM, NOISE_LEVEL_DBM));

		if(notification.packet_type == PACKET_TYPE_RTS || notification.packet_type == PACKET_TYPE_ICF) {	// Notification is an RTS or ICF

			current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

			LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s SINR = %.2f dBm\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
				ConvertPower(LINEAR_TO_DB, current_sinr));

			// Check if notification has been lost due to interferences or weak signal strength
			loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification,
					current_sinr, node_params.capture_effect, current_pd,
					power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			if(loss_reason != PACKET_NOT_LOST) {	// If RTS IS LOST, send logical Nack

				// Check if lost due to BO collision
				if(loss_reason == PACKET_LOST_INTERFERENCE){
					if(fabs(outrange_nav_notification.timestamp - notification.timestamp)
						< MAX_DIFFERENCE_SAME_TIME){
						loss_reason = PACKET_LOST_BO_COLLISION;
					}
				}

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
					SimTime(), node_params.node_id, node_state, LOG_D15, LOG_LVL4, notification.packet_id,
					notification.source_id, loss_reason);

				if(node_params.nack_activated) {
					// Send logical NACK to transmitter sending incoming notification for indicating the loss reason
					logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
						node_params.node_id, notification.source_id, NODE_ID_NONE, loss_reason, BER, current_sinr);
					SendLogicalNack(logical_nack);
				}

				if(node_is_transmitter){
					int pause (HandleBackoff(PAUSE_TIMER, &channel_power,
						node_params.current_primary_channel, current_pd, buffer.QueueSize()));
					// Check if node has to freeze the BO (if it is not already frozen)
					if (pause) {
						PauseBackoff();
					}
				}

			} else {	// Data packet IS NOT LOST (it can be properly received)

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of RTS #%d from N%d CAN be started (SINR = %f dB)\n",
					SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id,
					notification.source_id, ConvertPower(LINEAR_TO_DB, current_sinr));

				/*
					* Save incoming notification. This is kept in order to compare new notifications to the current
					* one for determining if it is lost and stuff like that.
					*/
				incoming_notification = notification;

				// Change state and update receiving info
				data_duration = notification.tx_info.data_duration;
				ack_duration = notification.tx_info.ack_duration;
				rts_duration = notification.tx_info.rts_duration;
				cts_duration = notification.tx_info.cts_duration;

				current_left_channel = notification.left_channel;
				current_right_channel = notification.right_channel;

				node_state = STATE_RX_RTS;
				receiving_from_node_id = notification.source_id;
				receiving_packet_id = notification.packet_id;

				// Pause backoff as node has began a reception
				if(node_is_transmitter) PauseBackoff();

			}

		} else if (notification.packet_type == PACKET_TYPE_DATA) {	// 2-way: DATA received directly (no RTS/CTS)

			current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

			loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification,
					current_sinr, node_params.capture_effect, current_pd,
					power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			if(loss_reason != PACKET_NOT_LOST) {

				if(node_params.nack_activated) {
					logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
						node_params.node_id, notification.source_id, NODE_ID_NONE, loss_reason, BER, current_sinr);
					SendLogicalNack(logical_nack);
				}

				if(node_is_transmitter) PauseBackoff();

			} else {

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of DATA #%d from N%d CAN be started (SINR = %f dB)\n",
					SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id,
					notification.source_id, ConvertPower(LINEAR_TO_DB, current_sinr));

				incoming_notification = notification;

				data_duration = notification.tx_info.data_duration;
				ack_duration = notification.tx_info.ack_duration;

				current_left_channel = notification.left_channel;
				current_right_channel = notification.right_channel;

				node_state = STATE_RX_DATA;
				receiving_from_node_id = notification.source_id;
				receiving_packet_id = notification.packet_id;

				if(node_is_transmitter) PauseBackoff();

			}

		} else if (notification.packet_type == PACKET_TYPE_DSO_ICF) {
			if (node_is_transmitter) PauseBackoff();
			// DSO ICF to this STA: DATA will follow on secondary subband. Stay in SENSING.
		} else if (notification.packet_type == PACKET_TYPE_NPCA_ICF) {
			if (node_is_transmitter) PauseBackoff();
			// STA: update primary channel so DATA on NPCA subband can be received
			if (!node_is_transmitter && npca_enabled && npca_primary_channel >= 0) {
				npca_stored_primary_channel = node_params.current_primary_channel;
				node_params.current_primary_channel = npca_primary_channel;
				npca_sta_on_npca_channel = 1;
			}
		} else {	//	Notification does NOT CONTAIN an RTS or DATA
			LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Unexpected packet type (%d) received!\n",
					SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4, notification.packet_type);
		}

	} else {	// Node IS NOT THE DESTINATION

		// MAPC: check for group-addressed ICF targeted at this node's group
		if (notification.packet_type == PACKET_TYPE_ICF
				&& notification.destination_id == NODE_ID_MAPC_BROADCAST
				&& wlan.mapc_enabled
				&& wlan.FindMapcGroupIdx(notification.mapc_group_id) >= 0
				&& notification.source_id != node_params.node_id
				&& node_params.node_type == NODE_TYPE_AP) {

			UpdateSINRFromNotification(notification);
			loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification,
				current_sinr, node_params.capture_effect, current_pd,
				power_rx_interest, node_params.constant_per,
				node_params.node_id, node_params.capture_effect_model);

			if (loss_reason == PACKET_NOT_LOST) {
				incoming_notification = notification;
				data_duration  = notification.tx_info.data_duration;
				ack_duration   = notification.tx_info.ack_duration;
				rts_duration   = notification.tx_info.rts_duration;
				cts_duration   = notification.tx_info.cts_duration;
				// Propagate ICF's NAV so ICR carries a valid nav_time for third-party nodes.
				// Without this, current_nav_time is 0 and AP_C (non-MAPC AP) would try to
				// set trigger_NAV at SimTime()-epsilon => COST trigger crash.
				current_nav_time = notification.tx_info.nav_time;
				current_left_channel  = notification.left_channel;
				current_right_channel = notification.right_channel;
				node_state        = STATE_RX_ICF;
				coordinator_ap_id = notification.source_id;
				++node_stats.num_icf_rx;
				mapc_active_group_idx = wlan.FindMapcGroupIdx(notification.mapc_group_id);
				// Stagger index: count same-group peers with smaller node_id (excl. coordinator)
				mapc_peer_position = 0;
				for (int i = 0; i < wlan.mapc_num_peers[mapc_active_group_idx]; ++i) {
					if (wlan.mapc_peer_ap_ids[mapc_active_group_idx][i] != coordinator_ap_id
							&& wlan.mapc_peer_ap_ids[mapc_active_group_idx][i] < node_params.node_id)
						++mapc_peer_position;
				}
				if (node_is_transmitter) PauseBackoff();
				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Received ICF from N%d -> STATE_RX_ICF (pos=%d)\n",
					SimTime(), node_params.node_id, STATE_RX_ICF, LOG_D07, LOG_LVL2,
					notification.source_id, mapc_peer_position);
			}

		} else if(notification.packet_type == PACKET_TYPE_RTS
			|| notification.packet_type == PACKET_TYPE_CTS
			|| notification.packet_type == PACKET_TYPE_DATA
			|| notification.packet_type == PACKET_TYPE_ACK
			|| notification.packet_type == PACKET_TYPE_ICF
			|| notification.packet_type == PACKET_TYPE_ICR
			|| notification.packet_type == PACKET_TYPE_TF
			|| notification.packet_type == PACKET_TYPE_MU_RTS_TXS
			|| notification.packet_type == PACKET_TYPE_DSO_ICF
			|| notification.packet_type == PACKET_TYPE_NPCA_ICF) {

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s I am not the TX destination (N%d to N%d). Checking if Frame can be decoded.\n",
				SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL2,
				notification.source_id, notification.destination_id);

			/** Can the packet be decoded? **/
			UpdateSINRFromNotification(notification);
			// 4 - Check if the packet is lost or not
			loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification, current_sinr,
				node_params.capture_effect, current_pd, power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Pmax_intf[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm, sinr = %f dB\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
				channel_max_interference, ConvertPower(PW_TO_DBM, channel_power[channel_max_interference]),
				ConvertPower(PW_TO_DBM, power_rx_interest),
				ConvertPower(PW_TO_DBM, max_pw_interference),
				ConvertPower(LINEAR_TO_DB,current_sinr));

			// Spatial Reuse: try to identify SR TXOP; may update loss_reason
			TryIdentifySRTXOP(notification, loss_reason);

			if(loss_reason == PACKET_NOT_LOST) { // RTS/DATA/CTS/ACK can be decoded

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Packet type %d can be decoded\n",
					SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, notification.packet_type);

				// Save NAV notification for comparing timestamps in case of need
				nav_notification = notification;

				// NPCA trigger (SS37.18.3 cond 1): inter-BSS PPDU on BSS primary channel
				if (npca_enabled && !npca_on_npca_channel
						&& node_params.bss_color >= 0
						&& notification.tx_info.bss_color >= 0
						&& notification.tx_info.bss_color != node_params.bss_color
						&& notification.left_channel <= node_params.current_primary_channel
						&& notification.right_channel >= node_params.current_primary_channel) {
					double _rem_us = notification.tx_duration / MICRO_VALUE;
					if (_rem_us > (double)npca_min_dur_threshold_us)
						CheckAndArmNpcaSwitch();
				}

				int pause (HandleBackoff(PAUSE_TIMER, &channel_power,
					node_params.current_primary_channel, current_pd, buffer.QueueSize()));

				// MAPC: if ICF or TF is for our own MAPC group, stay in STATE_SENSING so we
				// can receive our AP's DATA in the upcoming slot. Covers both STA_A (coord.
				// AP's STA, source == own AP) and STA_B (coordinated AP's STA, source != own AP).
				if ((notification.packet_type == PACKET_TYPE_ICF
							|| notification.packet_type == PACKET_TYPE_TF)
						&& node_params.node_type == NODE_TYPE_STA
						&& wlan.mapc_enabled
						&& wlan.FindMapcGroupIdx(notification.mapc_group_id) >= 0) {
					if (node_is_transmitter) PauseBackoff();
					LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s ICF/TF for own MAPC group %d: stay in SENSING to receive DATA\n",
						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, notification.mapc_group_id);
				// MAPC Co-TDMA: if ICR is for our MAPC group, don't set NAV - DATA will follow
				// for us (STA_A via coordinator AP, STA_B via coordinated AP); stay in SENSING.
				} else if (notification.packet_type == PACKET_TYPE_ICR
						&& node_params.node_type == NODE_TYPE_STA
						&& wlan.mapc_enabled
						&& wlan.FindMapcGroupIdx(notification.mapc_group_id) >= 0) {
					if (node_is_transmitter) PauseBackoff();
					LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s ICR for own MAPC group %d: stay in SENSING to receive upcoming DATA\n",
						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, notification.mapc_group_id);
				// MAPC Co-TDMA: if MU-RTS/TXS is addressed to our own AP, don't set NAV -
				// our AP will send DATA to us in the upcoming slot, so we must stay in STATE_SENSING.
				} else if (notification.packet_type == PACKET_TYPE_MU_RTS_TXS
						&& node_params.node_type == NODE_TYPE_STA
						&& notification.destination_id == wlan.ap_id) {
					if (node_is_transmitter) PauseBackoff();
					LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s MU-RTS/TXS to own AP N%d: stay in SENSING to receive upcoming DATA\n",
						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, wlan.ap_id);
				} else if(pause) {

					// Check if node has to freeze the BO (if it is not already frozen)
					if (node_is_transmitter) {
						PauseBackoff();
					}

					// Update the NAV time according to the frame's info
					current_nav_time = notification.tx_info.nav_time;

					// SERGIO on 28/09/2017:
					// - Ensure NAV TO finishes at same time (or before) than other's WLAN ACK transmission.
					// time_to_trigger = SimTime() + current_nav_time + TIME_OUT_EXTRA_TIME;
					time_to_trigger = SimTime() + current_nav_time - TIME_OUT_EXTRA_TIME;

					// SERGIO_TRIGGER
					// Differentiate between Intra-BSS and Inter-BSS NAV triggers
					if (sr_state.spatial_reuse_enabled && sr_state.type_last_sensed_packet != INTRA_BSS_FRAME) {
						trigger_inter_bss_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
					} else {
						trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
					}

					LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Entering in NAV during %.12f and setting NAV timeout to %.12f\n",
						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3,
						current_nav_time, trigger_NAV_timeout.GetTime());

//							LOGS(node_params.save_node_logs,node_logger.file,
//								"%.15f;N%d;S%d;%s;%s current_nav_time = %.12f\n",
//								SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4,
//								current_nav_time);

					node_state = STATE_NAV;
					node_stats.last_time_not_in_nav = SimTime();
					++node_stats.times_went_to_nav;

				}

			} else { // Frame cannot be decoded.

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Frame sent by N%d could not be decoded for reason %d\n",
					SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3,
					notification.source_id, loss_reason);

				// Save NAV notification for comparing timestamps in case of need
				outrange_nav_notification = notification;

				// Check if DIFS or BO must be stopped
				if(node_is_transmitter){

					LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Checking if BO must be paused...\n",
						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);

					int pause;

					if(sr_state.spatial_reuse_enabled && sr_state.txop_sr_identified) {
						pause = HandleBackoff(PAUSE_TIMER, &channel_power,
							node_params.current_primary_channel, sr_state.current_obss_pd_threshold, buffer.QueueSize());
					} else {
						pause = HandleBackoff(PAUSE_TIMER, &channel_power, node_params.current_primary_channel,
							current_pd, buffer.QueueSize());
					}

					// Check if node has to freeze the BO (if it is not already frozen)
					if (pause) {
						PauseBackoff();
					} else {
						if(trigger_end_backoff.Active()) ca_state.remaining_backoff =
								ComputeRemainingBackoff(node_params.backoff_type, trigger_end_backoff.GetTime() - SimTime());
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s BO must not be paused (%f remaining slots).\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5, ca_state.remaining_backoff/SLOT_TIME);
					}

				}

			}

		}
	}
}


/**
 * Handle InportSomeNodeStartTX for STATE_NAV
 */
void Node :: HandleStartTX_StateNav(const Notification &notification) {

	int nav_collision(0);
	int inter_bss_nav_collision(0);
	ComputeNavCollisions(notification, nav_collision, inter_bss_nav_collision);

	if(notification.destination_id == node_params.node_id){	// Node IS THE DESTINATION

		// DATA from own AP: NAV suppresses transmission, not reception.
		// Always attempt to receive DATA addressed to this node, regardless of whether a
		// simultaneous collision was detected (e.g. Co-SR: peer AP starts 1 event-tick
		// before coordinator, causing this STA to enter NAV from the peer's frame first).
		if(notification.packet_type == PACKET_TYPE_DATA) {

			// Fix 1: BF-aware interference — when the sender has beamforming active,
			// derive interference from the per-node BF-corrected powers so that
			// nulled directions do not inflate the interference estimate.
			// (channel_power uses raw path loss with no BF gain applied.)
			if (notification.tx_info.beamforming_active) {
				power_rx_interest = power_received_per_node[notification.source_id];
				max_pw_interference = 0;
				for (std::map<int,double>::iterator _it = power_received_per_node.begin();
						_it != power_received_per_node.end(); ++_it) {
					if (_it->first != notification.source_id)
						max_pw_interference += _it->second;
				}
				current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);
			} else {
				UpdateSINRFromNotification(notification);
			}

			// Fix 2: for Co-BF, the STA's NAV was set by the coordination handshake
			// (ICF/TF), not a genuine collision.  Bypass the packet-lost gate and let
			// the reception complete; HandleFinishTX_StateRxData determines outcome.
			{
				int _cobf_g = wlan.mapc_enabled
					? wlan.FindMapcGroupIdx(notification.mapc_group_id) : -1;
				if (_cobf_g >= 0 && wlan.mapc_method_ids[_cobf_g] == CO_BF) {
					loss_reason = PACKET_NOT_LOST;
				} else {
					loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification,
						current_sinr, node_params.capture_effect, current_pd,
						power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);
				}
			}

			if(loss_reason != PACKET_NOT_LOST) {

				loss_reason = PACKET_LOST_BO_COLLISION;

				if(!node_is_transmitter) {
					trigger_NAV_timeout.Cancel();
					time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;
					trigger_restart_sta.Set(FixTimeOffset(time_to_trigger,13,12));
				}

				if(node_params.nack_activated) {
					logical_nack = GenerateLogicalNack(notification.packet_type,
						notification.packet_id, node_params.node_id, notification.source_id,
						NODE_ID_NONE, loss_reason, BER, current_sinr);
					SendLogicalNack(logical_nack);
				}

			} else {

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of DATA #%d from N%d CAN be started (SINR = %f dB)\n",
					SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id,
					notification.source_id, ConvertPower(LINEAR_TO_DB, current_sinr));

				// Cancel the previous NAV
				if (sr_state.spatial_reuse_enabled) {
					trigger_inter_bss_NAV_timeout.Cancel();
				} else {
					trigger_NAV_timeout.Cancel();
				}

				incoming_notification = notification;

				data_duration = notification.tx_info.data_duration;
				ack_duration = notification.tx_info.ack_duration;

				current_left_channel = notification.left_channel;
				current_right_channel = notification.right_channel;

				node_state = STATE_RX_DATA;
				receiving_from_node_id = notification.source_id;
				receiving_packet_id = notification.packet_id;

				if(node_is_transmitter) PauseBackoff();

			}

		} else if(nav_collision || inter_bss_nav_collision) {	// RTS or other during simultaneous collision

			if(notification.packet_type == PACKET_TYPE_RTS) {	// Notification CONTAINS an RTS PACKET

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s RTS from my AP N%d sent simultaneously\n",
					SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4,
					notification.source_id);

				UpdateSINRFromNotification(notification);

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm\n",
					SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5, channel_max_interference,
					ConvertPower(PW_TO_DBM, channel_power[channel_max_interference]),
					ConvertPower(PW_TO_DBM, power_rx_interest),
					ConvertPower(PW_TO_DBM, max_pw_interference));

				// Check if notification has been lost due to interferences or weak signal strength
				// TODO: method for checking whether the detected transmission can be decoded or not
				loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification,
					current_sinr, node_params.capture_effect, current_pd,
					power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

				if(loss_reason != PACKET_NOT_LOST) {	// If RTS IS LOST, send logical Nack

					loss_reason = PACKET_LOST_BO_COLLISION;

					if(!node_is_transmitter) {

						// Sergio 18/09/2017:
						// NAV is no longer valid. It cannot be decoded due to interference.
						// Wait MAX_DIFFERENCE_SAME_TIME to detect more transmissions sent at the "same" time
						// Trigger the restart then.

						// Sergio on 27/09/2017. Review this case
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s RTS from my AP CANNOT be decoded\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5);

						trigger_NAV_timeout.Cancel();
						time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;

						// trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
						trigger_restart_sta.Set(FixTimeOffset(time_to_trigger,13,12));

					} else {

						printf("ALARM! Should not happen in downlink traffic\n");

					}
					// EOF HandleSlottedBackoffCollision();

					if(node_params.nack_activated) {
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s RTS cannot be decoded (SINR = %f dB) -> Sending NACK corresponding to BO collision to N%d\n",
							SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL5,
							ConvertPower(LINEAR_TO_DB, current_sinr), notification.source_id);
						logical_nack = GenerateLogicalNack(notification.packet_type,
							notification.packet_id, node_params.node_id, notification.source_id,
							NODE_ID_NONE, loss_reason, BER, current_sinr);
						SendLogicalNack(logical_nack);
					}

				} else {	// Data packet IS NOT LOST (it can be properly received)

					LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Reception of RTS #%d from N%d CAN be started (SINR = %f dB)\n",
						SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id,
						notification.source_id, ConvertPower(LINEAR_TO_DB, current_sinr));

					// Cancel the previous NAV
					if ( sr_state.spatial_reuse_enabled ) {
						trigger_inter_bss_NAV_timeout.Cancel(); // Cancel inter-BSS NAV
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s INTER-BSS NAV CANCELLED!\n",
							SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4);
					} else {
						trigger_NAV_timeout.Cancel();			// Cancel intra-BSS NAV (legacy)
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s DEFAULT NAV CANCELLED!\n",
							SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4);
					}

					// Change state and update receiving info
					data_duration = notification.tx_info.data_duration;
					ack_duration = notification.tx_info.ack_duration;
					rts_duration = notification.tx_info.rts_duration;
					cts_duration = notification.tx_info.cts_duration;

					current_left_channel = notification.left_channel;
					current_right_channel = notification.right_channel;

					node_state = STATE_RX_RTS;
					receiving_from_node_id = notification.source_id;
					receiving_packet_id = notification.packet_id;

					incoming_notification = notification;

					// Pause backoff as node has began a reception
					if(node_is_transmitter) PauseBackoff();

				}

			} else {	//	Notification does NOT CONTAIN an RTS or DATA
				LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Unexpected packet type (%d) received!\n",
						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4, notification.packet_type);
			}

		} else { // Notification not detected to happen at the same time

			if (node_params.nack_activated) {
				// Send logical NACK to transmitter sending incoming notification for indicating the loss reason
				logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
					node_params.node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_RX_IN_NAV, BER, current_sinr);
				SendLogicalNack(logical_nack);
			}

		}


	} else { // Node IS NOT THE DESTINATION

		if(notification.packet_type == PACKET_TYPE_RTS
			||  notification.packet_type == PACKET_TYPE_CTS
			|| notification.packet_type == PACKET_TYPE_DATA
			|| notification.packet_type == PACKET_TYPE_ACK) {	// PACKET TYPE RTS OR CTS

			// TODO: determine if can be decoded!

			UpdateSINRFromNotification(notification);
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5, channel_max_interference,
				ConvertPower(PW_TO_DBM, channel_power[channel_max_interference]),
				ConvertPower(PW_TO_DBM, power_rx_interest),
				ConvertPower(PW_TO_DBM, max_pw_interference));
			// Check if notification can be decoded
			// TODO: method for checking whether the detected transmission can be decoded or not
			int loss_reason (IsPacketLost(node_params.current_primary_channel, notification, notification,
				current_sinr, node_params.capture_effect, current_pd, power_rx_interest, node_params.constant_per,
				node_params.node_id, node_params.capture_effect_model));

			// NAV collision detected
			if((nav_collision || inter_bss_nav_collision) && loss_reason == PACKET_NOT_LOST)  {

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Updating the NAV according to the last sensed transmission\n",
					SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL2);

				if(!node_is_transmitter) {

					// Cancel the previous NAV and set it again according to the new one
					time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;
					if (sr_state.spatial_reuse_enabled && inter_bss_nav_collision) {
						trigger_inter_bss_NAV_timeout.Cancel(); // Cancel inter-BSS NAV
						trigger_inter_bss_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
						LOGS(node_params.save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s (workaround) setting inter-BSS NAV trigger to %.12f\n",
							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, time_to_trigger);
					} else {
						trigger_NAV_timeout.Cancel();			// Cancel intra-BSS NAV (legacy)
						trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
						LOGS(node_params.save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s (workaround) setting NAV trigger to %.12f\n",
							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, time_to_trigger);
					}

				} else {

					if ( (nav_collision && nav_notification.packet_type == notification.packet_type)
						|| (inter_bss_nav_collision && nav_notification.packet_type == notification.packet_type) ) {

						// if(node_params.save_node_logs) fprintf(node_logger.file,
						//	"%.15f;N%d;S%d;%s;%s Waiting just in case of more collisions.\n",
						//	SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL4);

						// Cancel the previous NAV
						if (sr_state.spatial_reuse_enabled && inter_bss_nav_collision) {
							trigger_inter_bss_NAV_timeout.Cancel();		// Cancel inter-BSS NAV
						} else {
							trigger_NAV_timeout.Cancel();				// Cancel intra-BSS NAV (legacy)
						}

						// Sergio on 27/09/2017
						// - An AP must wait EIFS after the last packet of external RTSs collisions is finished.

						// Sergio on 2018/07/06: EIFS to match Bianchi model
						time_to_trigger =
							SimTime() + notification.tx_info.rts_duration
							+ SIFS + notification.tx_info.cts_duration
							- notification.tx_info.preoccupancy_duration;

						trigger_wait_collisions.Set(FixTimeOffset(time_to_trigger,13,12));

						LOGS(node_params.save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s Recovering from EIFS at %.12f (preoc. = %.12f)\n",
							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL4,
							trigger_wait_collisions.GetTime(),
							notification.tx_info.preoccupancy_duration);

					}
				}

				// Do not send NACK because node is not the destination

			} else { // No collision

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s I am not the TX destination (N%d to N%d). Checking if new RTS/CTS can be decoded.\n",
					SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL2,
					notification.source_id, notification.destination_id);

				// Can the notification be decoded?
				UpdateSINRFromNotification(notification);

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Pmax_intf[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm, sinr = %f dB\n",
					SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
					channel_max_interference, ConvertPower(PW_TO_DBM, channel_power[channel_max_interference]),
					ConvertPower(PW_TO_DBM, power_rx_interest),
					ConvertPower(PW_TO_DBM, max_pw_interference),
					ConvertPower(LINEAR_TO_DB,current_sinr));

				// TODO: method for checking whether the detected transmission can be decoded or not
				loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification,
					current_sinr, node_params.capture_effect, current_pd, power_rx_interest, node_params.constant_per,
					node_params.node_id, node_params.capture_effect_model);

				int power_condition (channel_power[node_params.current_primary_channel] > node_params.sensitivity_default);

				if (loss_reason == PACKET_NOT_LOST && power_condition) {	// Packet IS NOT LOST

					// Spatial Reuse: detect / cancel SR TXOP while in NAV
					DetectSRTXOPInNavState(notification, loss_reason);

				} else {			// Packet IS LOST
					if(notification.packet_type == PACKET_TYPE_RTS
							||  notification.packet_type == PACKET_TYPE_CTS){
						LOGS(node_params.save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s RTS/CTS sent from N%d could not be decoded for reason %d\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3,
							notification.source_id, loss_reason);
					}
				}
			}
		}
	}

}

/**
 * Handle InportSomeNodeStartTX for STATE_TX_DATA
 */
void Node :: HandleStartTX_StateTxData(const Notification &notification) {

	if(notification.destination_id == node_params.node_id){ // Node IS THE DESTINATION

		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
			SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3,
			notification.destination_id);

		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s I am transmitting, packet cannot be received\n",
			SimTime(), node_params.node_id, node_state, LOG_D18, LOG_LVL3);

		if(node_params.nack_activated) {
			// Send logical NACK to incoming notification transmitter due to receiver (node) was already receiving
			logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
				node_params.node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_DESTINATION_TX, BER, current_sinr);

			SendLogicalNack(logical_nack);
		}

	} else {	// Node IS NOT THE DESTINATION, do nothing

//					LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
//						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, notification.destination_id);

	}


	int nav_collision(0);
	int inter_bss_nav_collision(0);
	ComputeNavCollisions(notification, nav_collision, inter_bss_nav_collision);

	// If two or more packets sent at the same time
	if(nav_collision || inter_bss_nav_collision) {

	}
	// Spatial Reuse: detect / cancel SR TXOP while in TX state
	DetectSRTXOPWhileTransmitting(notification);

}

/**
 * Handle InportSomeNodeStartTX for STATE_RX_DATA
 */
void Node :: HandleStartTX_StateRxData(const Notification &notification) {

	if(notification.destination_id == node_params.node_id){	// Node IS THE DESTINATION

//					LOGS(node_params.save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
//							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, notification.destination_id);

		// Update the SINR
		current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

		// Check if ongoing notification has been lost due to interferences caused by new transmission
		loss_reason = IsPacketLost(node_params.current_primary_channel, incoming_notification, notification,
			current_sinr, node_params.capture_effect, current_pd,
			power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

		// TODO: method for checking whether the detected transmission can be decoded or not
		// ...

		switch(node_params.capture_effect_model){

			case CE_DEFAULT:{
				if(loss_reason != PACKET_NOT_LOST
					&& loss_reason != PACKET_LOST_OUTSIDE_CH_RANGE)  {	// If ongoing data packet IS LOST
						// Pure collision (two nodes transmitting to me with enough power)
						LOGS(node_params.save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s Pure collision! Already receiving from N%d\n",
							SimTime(), node_params.node_id, node_state, LOG_D19, LOG_LVL4, receiving_from_node_id);
						loss_reason = PACKET_LOST_PURE_COLLISION;
						// If two or more packets sent at the same time
						if(fabs(notification.timestamp - incoming_notification.timestamp) < MAX_DIFFERENCE_SAME_TIME){
							// SERGIO HandleSlottedBackoffCollision();
							loss_reason = PACKET_LOST_BO_COLLISION;
							if(!node_is_transmitter) {
								time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;
								trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
							} else {
								printf("ALARM! Should not happen in downlink traffic\n");
							}
						}
						if(node_params.nack_activated) {
							// Send NACK to both ongoing transmitter and incoming interferer nodes
							logical_nack = GenerateLogicalNack(notification.packet_type, nav_notification.packet_id,
									node_params.node_id, nav_notification.source_id, notification.source_id, loss_reason, BER, current_sinr);
							SendLogicalNack(logical_nack);
						}

				} else {	// If ongoing data packet IS NOT LOST (incoming transmission does not affect ongoing reception)

					if (node_params.nack_activated) {
						LOGS(node_params.save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s Low strength signal received while already receiving from N%d\n",
							SimTime(), node_params.node_id, node_state, LOG_D20, LOG_LVL4, receiving_from_node_id);

						// Send logical NACK to incoming transmitter indicating that node is already receiving
						logical_nack = GenerateLogicalNack(notification.packet_type, receiving_from_node_id,
								node_params.node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_LOW_SIGNAL_AND_RX, BER, current_sinr);

						SendLogicalNack(logical_nack);
					}

				}
				break;
			}

			case CE_IEEE_802_11:{
				int capture_effect_condition (power_received_per_node[notification.source_id] >
					power_received_per_node[receiving_from_node_id] + node_params.capture_effect);

				if (loss_reason == PACKET_NOT_LOST && capture_effect_condition) {
					if (notification.packet_type == PACKET_TYPE_RTS) {
						// Start decoding the new packet
						incoming_notification = notification;
						// Change state and update receiving info
						data_duration = notification.tx_info.data_duration;
						ack_duration = notification.tx_info.ack_duration;
						rts_duration = notification.tx_info.rts_duration;
						cts_duration = notification.tx_info.cts_duration;
						current_left_channel = notification.left_channel;
						current_right_channel = notification.right_channel;
						node_state = STATE_RX_RTS;
						receiving_from_node_id = notification.source_id;
						receiving_packet_id = notification.packet_id;
						// Pause backoff as node has began a reception
						if(node_is_transmitter) PauseBackoff();
						if (node_params.nack_activated) {
							// Send NACK to both ongoing transmitter and incoming interferer nodes
							logical_nack = GenerateLogicalNack(notification.packet_type, nav_notification.packet_id,
									node_params.node_id, NODE_ID_NONE, notification.source_id, PACKET_LOST_CAPTURE_EFFECT, BER, current_sinr);
							SendLogicalNack(logical_nack);
						}
					}  else {
						// Pure collision (two nodes transmitting to me with enough power)
						LOGS(node_params.save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s Pure collision! Already receiving from N%d\n",
							SimTime(), node_params.node_id, node_state, LOG_D19, LOG_LVL4, receiving_from_node_id);
						loss_reason = PACKET_LOST_PURE_COLLISION;
						// If two or more packets sent at the same time
						if(fabs(notification.timestamp - incoming_notification.timestamp) < MAX_DIFFERENCE_SAME_TIME) {
							loss_reason = PACKET_LOST_BO_COLLISION;
							if(!node_is_transmitter) {
								time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;
								trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
							} else {
								printf("ALARM! Should not happen in downlink traffic\n");
							}
						}
						if(node_params.nack_activated){
							// Send NACK to both ongoing transmitter and incoming interferer nodes
							logical_nack = GenerateLogicalNack(notification.packet_type, nav_notification.packet_id,
									node_params.node_id, nav_notification.source_id, notification.source_id, loss_reason, BER, current_sinr);
							SendLogicalNack(logical_nack);
						}
					}
				} else { // If ongoing data packet IS NOT LOST (incoming transmission does not affect ongoing reception)
					if(node_params.nack_activated){
						LOGS(node_params.save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s Low strength signal received while already receiving from N%d\n",
							SimTime(), node_params.node_id, node_state, LOG_D20, LOG_LVL4, receiving_from_node_id);
						// Send logical NACK to incoming transmitter indicating that node is already receiving
						logical_nack = GenerateLogicalNack(notification.packet_type, receiving_from_node_id,
								node_params.node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_LOW_SIGNAL_AND_RX, BER, current_sinr);
						SendLogicalNack(logical_nack);
					}
				}
				break;
			}
		}

	} else {	// Node is NOT THE DESTINATION

//					LOGS(node_params.save_node_logs, node_logger.file,
//						"%.15f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
//						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, notification.destination_id);

		// Fix 1: BF-aware interference — when the arriving notification has beamforming
		// active, derive interference from per-node BF-corrected powers so that
		// nulled directions do not inflate the interference estimate.
		if (notification.tx_info.beamforming_active) {
			max_pw_interference = 0;
			for (std::map<int,double>::iterator _it = power_received_per_node.begin();
					_it != power_received_per_node.end(); ++_it) {
				if (_it->first != incoming_notification.source_id)
					max_pw_interference += _it->second;
			}
		} else {
			// Compute max interference (the highest one perceived in the reception channel range)
			ComputeMaxInterference(&max_pw_interference, &channel_max_interference,
				incoming_notification, node_state, power_received_per_node, &channel_power);
		}

		// Check if the ongoing reception is affected
		current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm - current_sinr = %.2f dBm\n",
			SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5, channel_max_interference,
			ConvertPower(PW_TO_DBM, channel_power[channel_max_interference]),
			ConvertPower(PW_TO_DBM, power_rx_interest),
			ConvertPower(PW_TO_DBM, max_pw_interference),
			ConvertPower(LINEAR_TO_DB, current_sinr));

		// Fix 2: for Co-BF, the peer AP's simultaneous DATA is expected to be nulled
		// toward this node; bypass the collision check so the reception continues.
		{
			int _cobf_g = wlan.mapc_enabled
				? wlan.FindMapcGroupIdx(notification.mapc_group_id) : -1;
			int _ongoing_g = wlan.mapc_enabled
				? wlan.FindMapcGroupIdx(incoming_notification.mapc_group_id) : -1;
			if (_cobf_g >= 0 && _cobf_g == _ongoing_g
					&& wlan.mapc_method_ids[_cobf_g] == CO_BF) {
				loss_reason = PACKET_NOT_LOST;
			// Check if the notification that was already being received is lost due to new notification
			} else if (sr_state.spatial_reuse_enabled && sr_state.txop_sr_identified) {
				loss_reason = IsPacketLost(node_params.current_primary_channel, incoming_notification, notification,
					current_sinr, node_params.capture_effect, sr_state.current_obss_pd_threshold,
					power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);
			} else {
				loss_reason = IsPacketLost(node_params.current_primary_channel, incoming_notification, notification,
					current_sinr, node_params.capture_effect, current_pd,
					power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);
			}
		}

		// TODO: method for checking whether the detected transmission can be decoded or not
		// ...

		LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s loss_reason = %d\n",
			SimTime(), node_params.node_id, node_state, LOG_D19, LOG_LVL4, loss_reason);

		if(loss_reason != PACKET_NOT_LOST) { 	// If ongoing packet reception IS LOST

			switch(node_params.capture_effect_model) {

				case CE_DEFAULT:{
					// Collision by hidden node
					LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s Collision by interferences!\n",
						SimTime(), node_params.node_id, node_state, LOG_D19, LOG_LVL4);

						// If two or more packets sent at the same time
					if(node_state == STATE_RX_RTS && notification.packet_type == PACKET_TYPE_RTS){
						if(fabs(notification.timestamp - incoming_notification.timestamp) < MAX_DIFFERENCE_SAME_TIME){
							loss_reason = PACKET_LOST_BO_COLLISION;
						}
					}
					// Send logical NACK to ongoing transmitter
					if (node_params.nack_activated) {
						logical_nack = GenerateLogicalNack(incoming_notification.packet_type,
							incoming_notification.packet_id, node_params.node_id, incoming_notification.source_id,
							NODE_ID_NONE, loss_reason, BER, current_sinr);
						SendLogicalNack(logical_nack);
					}
					RestartNode(FALSE);
					break;
				}

				case CE_IEEE_802_11:{
					int capture_effect_condition = power_received_per_node[notification.source_id] >
						power_received_per_node[receiving_from_node_id] + node_params.capture_effect;
					if (capture_effect_condition) {
						loss_reason = PACKET_LOST_CAPTURE_EFFECT;
						printf("Node %d was in state RX (from %d), and a new notification arrived from %d:\n", node_params.node_id, receiving_from_node_id, notification.source_id);
						printf("	* New RSSI: %f\n", power_received_per_node[notification.source_id]);
						printf("	* Old RSSI: %f:\n", power_received_per_node[receiving_from_node_id]);
						printf("	* CE: %f:\n", node_params.capture_effect);
						printf("	* loss_reason: %d:\n", loss_reason);
						if(node_params.nack_activated){
						// Send NACK to both ongoing transmitter and incoming interferer nodes
						logical_nack = GenerateLogicalNack(notification.packet_type, nav_notification.packet_id,
							node_params.node_id, nav_notification.source_id, notification.source_id, loss_reason, BER, current_sinr);
						SendLogicalNack(logical_nack);
						}
						RestartNode(FALSE);
					}
					break;
				}
			}
		}
	}

}

/**
 * Handle InportSomeNodeStartTX for STATE_WAIT_ACK
 */
void Node :: HandleStartTX_StateWaitAck(const Notification &notification) {

	if(notification.destination_id == node_params.node_id){	// Node is the destination

		power_rx_interest = power_received_per_node[notification.source_id];

		incoming_notification = notification;

//					LOGS(node_params.save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
//							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, notification.destination_id);

		if(notification.packet_type == PACKET_TYPE_ACK){	// ACK packet transmission started

			// Compute max interference (the highest one perceived in the reception channel range)
			ComputeMaxInterference(&max_pw_interference, &channel_max_interference,
				incoming_notification, node_state, power_received_per_node, &channel_power);

			// Check if notification has been lost due to interferences or weak signal strength
			current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

			// TODO: method for checking whether the detected transmission can be decoded or not
			loss_reason = IsPacketLost(node_params.current_primary_channel, incoming_notification, notification,
					current_sinr, node_params.capture_effect, current_pd,
					power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			if(loss_reason != PACKET_NOT_LOST
					&& loss_reason != PACKET_LOST_OUTSIDE_CH_RANGE) {	// If ACK packet IS LOST, send logical Nack

				LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
						SimTime(), node_params.node_id, node_state, LOG_D15, LOG_LVL4, notification.packet_id,
						notification.source_id, loss_reason);

				if(node_params.nack_activated){
					// Send logical NACK to ACK transmitter
					logical_nack = GenerateLogicalNack(incoming_notification.packet_type, incoming_notification.packet_id,
						node_params.node_id, receiving_from_node_id, NODE_ID_NONE, loss_reason, BER, current_sinr);
					SendLogicalNack(logical_nack);
				}

				// Do nothing until ACK timeout is triggered

			} else {	// If ACK packet IS NOT LOST (it can be properly received)

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of ACK %d from N%d CAN be started\n",
					SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id, notification.source_id);

				// Cancel ACK timeout and go to STATE_RX_ACK while updating receiving info
				trigger_ACK_timeout.Cancel();
				node_state = STATE_RX_ACK;
				receiving_from_node_id = notification.source_id;
				receiving_packet_id = notification.packet_id;

//							LOGS(node_params.save_node_logs, node_logger.file,
//									"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
//									SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.destination_id);

//							LOGS(node_params.save_node_logs, node_logger.file,
//									"%.15f;N%d;S%d;%s;%s current_sinr = %f dB\n",
//									SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL5,
//									ConvertPower(LINEAR_TO_DB,current_sinr));

			}

		}  else {	//	Some packet type received that is not ACK
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s Unexpected packet type received!\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
		}

	} else {	// Node IS NOT THE DESTINATION, do nothing
//
//					LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
//								SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, notification.destination_id);
//
	}

}

/**
 * Handle InportSomeNodeStartTX for STATE_WAIT_CTS
 */
void Node :: HandleStartTX_StateWaitCts(const Notification &notification) {

	if(notification.destination_id == node_params.node_id){	// Node is the destination

		incoming_notification = notification;

//					LOGS(node_params.save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
//							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, notification.destination_id);

		if(notification.packet_type == PACKET_TYPE_CTS){	// CTS packet transmission started

			power_rx_interest = power_received_per_node[notification.source_id];

			// Compute max interference (the highest one perceived in the reception channel range)
			ComputeMaxInterference(&max_pw_interference, &channel_max_interference,
				incoming_notification, node_state, power_received_per_node, &channel_power);

			// Check if notification has been lost due to interferences or weak signal strength
			current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

//						LOGS(node_params.save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s P_sn = %f dBm (%f pW) - P_st= %f dBm (%f pW)"
//							"- P_if = %f dBm (%f pW)\n",
//							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
//							ConvertPower(PW_TO_DBM, channel_power[channel_max_interference]), channel_power[channel_max_interference],
//							ConvertPower(PW_TO_DBM, power_rx_interest), power_rx_interest, ConvertPower(PW_TO_DBM, max_pw_interference),
//							max_pw_interference);

			// TODO: method for checking whether the detected transmission can be decoded or not
			loss_reason = IsPacketLost(node_params.current_primary_channel, incoming_notification, notification,
				current_sinr, node_params.capture_effect, current_pd,
				power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			if(loss_reason != PACKET_NOT_LOST
					&& loss_reason != PACKET_LOST_OUTSIDE_CH_RANGE)  {	// If CTS packet IS LOST, send logical Nack

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
					SimTime(), node_params.node_id, node_state, LOG_D15, LOG_LVL4, notification.packet_id,
					notification.source_id, loss_reason);

				if(node_params.nack_activated){
					// Send logical NACK to ACK transmitter
					logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
							node_params.node_id, notification.source_id,
							NODE_ID_NONE, loss_reason, BER, current_sinr);
					SendLogicalNack(logical_nack);
				}

				// Do nothing until ACK timeout is triggered

			} else {	// If CTS packet IS NOT LOST (it can be properly received)

				LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s Reception of CTS #%d from N%d CAN be started\n",
						SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4,
						notification.packet_id, notification.source_id);

				current_left_channel = notification.left_channel;
				current_right_channel = notification.right_channel;

				// Cancel ACK timeout and go to STATE_RX_ACK while updating receiving info
				trigger_CTS_timeout.Cancel();
				node_state = STATE_RX_CTS;
				receiving_from_node_id = notification.source_id;
				receiving_packet_id = notification.packet_id;

				// Change state and update receiving info
				data_duration = notification.tx_info.data_duration;
				ack_duration = notification.tx_info.ack_duration;
				cts_duration = notification.tx_info.cts_duration;

//							LOGS(node_params.save_node_logs, node_logger.file,
//									"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
//									SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.destination_id);

//							LOGS(node_params.save_node_logs, node_logger.file,
//									"%.15f;N%d;S%d;%s;%s current_sinr = %f dB\n",
//									SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL5, ConvertPower(LINEAR_TO_DB,current_sinr));

			}

		}  else {	//	Some packet type received that is not CTS
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s Unexpected packet type received!\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
		}

	} else {	// Node IS NOT THE DESTINATION, do nothing

	}

}

/**
 * Handle InportSomeNodeStartTX for STATE_WAIT_DATA
 */
void Node :: HandleStartTX_StateWaitData(const Notification &notification) {

	if(notification.destination_id == node_params.node_id){	// Node is the destination

		power_rx_interest = power_received_per_node[notification.source_id];
		incoming_notification = notification;

//					LOGS(node_params.save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
//							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, notification.destination_id);

		if(notification.packet_type == PACKET_TYPE_DATA){	// DATA packet transmission started

			// Compute max interference (the highest one perceived in the reception channel range)
			ComputeMaxInterference(&max_pw_interference, &channel_max_interference,
				incoming_notification, node_state, power_received_per_node, &channel_power);

			// Check if notification has been lost due to interferences or weak signal strength
			current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm - current_sinr = %.2f dBm\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5, channel_max_interference,
				ConvertPower(PW_TO_DBM, channel_power[channel_max_interference]),
				ConvertPower(PW_TO_DBM, power_rx_interest),
				ConvertPower(PW_TO_DBM, max_pw_interference),
				ConvertPower(LINEAR_TO_DB, current_sinr));

			// TODO: method for checking whether the detected transmission can be decoded or not
			loss_reason = IsPacketLost(node_params.current_primary_channel, incoming_notification, notification,
				current_sinr, node_params.capture_effect, current_pd,
				power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			if(loss_reason != PACKET_NOT_LOST
				&& loss_reason != PACKET_LOST_OUTSIDE_CH_RANGE)  {	// If DATA packet IS LOST, send logical Nack

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
					SimTime(), node_params.node_id, node_state, LOG_D15, LOG_LVL4, notification.packet_id,
					notification.source_id, loss_reason);

				if(node_params.nack_activated){
					// Send logical NACK to DATA transmitter
					logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
							node_params.node_id, notification.source_id,
							NODE_ID_NONE, loss_reason, BER, current_sinr);
					SendLogicalNack(logical_nack);
				}

			} else {	// If DATA packet IS NOT LOST (it can be properly received)

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of DATA %d from N%d CAN be started\n",
					SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id, notification.source_id);

				// Cancel DATA timeout and go to STATE_RX_DATA while updating receiving info
				trigger_DATA_timeout.Cancel();
				node_state = STATE_RX_DATA;
				receiving_from_node_id = notification.source_id;
				receiving_packet_id = notification.packet_id;

				// Change state and update receiving info
				data_duration = notification.tx_info.data_duration;
				ack_duration = notification.tx_info.ack_duration;

			}

		}  else {	//	Some packet type received that is not ACK
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s Unexpected packet type received!\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
		}

	} else {	// Node IS NOT THE DESTINATION, do nothing

//					LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
//								SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, notification.destination_id);

	}

}


void Node :: InportSomeNodeStartTX(Notification &notification){

	LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s InportSomeNodeStartTX(): N%d to N%d sends packet type %d in range %d-%d using a transmit power of %.2f dBm\n",
			SimTime(), node_params.node_id, node_state, LOG_D00, LOG_LVL1,
			notification.source_id, notification.destination_id, notification.packet_type,
			notification.left_channel, notification.right_channel,
			ConvertPower(PW_TO_DBM, notification.tx_info.tx_power));

	LOGS(node_params.save_node_logs,node_logger.file,
	        "%.15f;N%d;S%d;%s;%s Nodes transmitting: ",
			SimTime(), node_params.node_id, node_state, LOG_D00, LOG_LVL3);

	// Identify node that has started the transmission as transmitting node in the array
	nodes_transmitting[notification.source_id] = TRUE;
	PrintOrWriteNodesTransmitting(WRITE_LOG, node_params.save_node_logs,
		node_params.print_node_logs, node_logger, node_params.total_nodes_number, nodes_transmitting);

	// TOKENIZED BO ONLY
	if(node_is_transmitter && node_params.backoff_type == BACKOFF_TOKENIZED) {
		// 1 - Check that the incoming transmission is not originated or directed to the transmitter
		if (notification.source_id != node_params.node_id && notification.destination_id != node_params.node_id) {
			// 2 - Check that the incoming transmission is an RTS or DATA
			if (notification.packet_type == PACKET_TYPE_RTS
				|| notification.packet_type == PACKET_TYPE_DATA) {
				// 3 - Check that the incoming transmission comes from a nearby device
				if (received_power_array[notification.source_id] > current_pd) {
					// Update the list of neighboring devices (if not done)
					if (token_order_list[notification.source_id] == DEVICE_INACTIVE_FOR_TOKEN) {
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token-based channel access operation (update neighbor list):\n",
							SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3);
						LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Token's order list before the update: ",
								SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL5);
							PrintOrWriteTokenList(WRITE_LOG, node_params.save_node_logs,
								node_params.print_node_logs, node_logger, token_order_list, node_params.total_nodes_number);
						UpdateTokenList(ADD_DEVICE_TO_LIST, &token_order_list, notification.source_id);
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token's order list updated: ",
							SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL5);
						PrintOrWriteTokenList(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
							token_order_list, node_params.total_nodes_number);
					}
					// Update the status of the token
					if (node_state != STATE_TX_RTS) {
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token-based channel access operation (token ACQUISITION):\n",
							SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL4);
						UpdateTokenStatus(node_params.node_id, TAKE_TOKEN, &token_status, notification.source_id,
							token_order_list, node_params.total_nodes_number, &distance_to_token);
					} else if (node_state == STATE_TX_RTS && node_params.node_id < notification.source_id) {
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token-based channel access operation (token ACQUISITION):\n",
							SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL4);
						UpdateTokenStatus(node_params.node_id, TAKE_TOKEN, &token_status, node_params.node_id,
							token_order_list, node_params.total_nodes_number, &distance_to_token);
					} else {
						// In case of a collision (two simultaneous RTS transmissions occur),
						// solve the conflict by releasing the token (the node with lowest ID gets the token)
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token-based channel access operation (token RELEASE):\n",
							SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL4);
						UpdateTokenStatus(node_params.node_id, RELEASE_TOKEN, &token_status, node_params.node_id,
							token_order_list, node_params.total_nodes_number, &distance_to_token);
					}
					LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Token status updated, the new token holder is %d\n",
						SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL5, token_status);
					// Update the CW parameters
					HandleContentionWindow(
						node_params.cw_adaptation, -1, &ca_state.deterministic_bo_active, &ca_state.current_cw_min, &ca_state.current_cw_max, &ca_state.cw_stage_current,
						node_params.cw_min_default, node_params.cw_max_default, node_params.cw_stage_max, distance_to_token, node_params.backoff_type, current_traffic_type);
					LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Updated CW parameters (token-based BO) = [%d-%d]\n",
						SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL5, ca_state.current_cw_min, ca_state.current_cw_max);
				}
			}
		}
	}

	if(notification.source_id == node_params.node_id){ // If OWN NODE IS THE TRANSMITTER, do nothing

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s I have started a TX of packet #%d (type %d) to N%d in channels %d - %d of duration %.9f us\n",
			SimTime(), node_params.node_id, node_state, LOG_D02, LOG_LVL2, notification.packet_id,
			notification.packet_type, notification.destination_id,
			notification.left_channel, notification.right_channel, notification.tx_duration * pow(10,6));


	} else {	// If OTHER NODE IS THE TRANSMITTER

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s N%d has started a TX of packet #%d (type %d) to N%d in channels %d - %d\n",
			SimTime(), node_params.node_id, node_state, LOG_D02, LOG_LVL2, notification.source_id,
			notification.packet_id,	notification.packet_type, notification.destination_id,
			notification.left_channel, notification.right_channel);

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Power sensed per channel BEFORE updating [dBm]: ",
			SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3);

		PrintOrWriteChannelPower(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
					&channel_power);

		// Update 'power received' array in case a new tx power is used
		if (notification.tx_info.flag_change_in_tx_power) {
			received_power_array[notification.source_id] =
				ComputePowerReceived(distances_array[notification.source_id],
				notification.tx_info.tx_power, node_params.central_frequency, node_params.path_loss_model);
		}

		// Update the power sensed at each channel (raw path-loss power, used for CCA/NAV).
		// Beamforming correction is NOT applied here: channel_power must reflect the
		// omnidirectional energy on the medium so that carrier-sense works correctly.
		UpdateChannelsPower(&channel_power, notification, TX_INITIATED,
			node_params.central_frequency, node_params.path_loss_model, node_params.adjacent_channel_model, received_power_array[notification.source_id], node_params.node_id);

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Power sensed per channel [dBm]: ",
			SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3);

		PrintOrWriteChannelPower(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
			&channel_power);

		// Call UpdatePowerSensedPerNode() ONLY for adding power (some node started)
		UpdatePowerSensedPerNode(node_params.current_primary_channel, power_received_per_node, notification,
			node_params.central_frequency, node_params.path_loss_model, received_power_array[notification.source_id], TX_INITIATED,
			node_params.x, node_params.y, node_params.z);

		UpdateTimestampChannelFreeAgain(timestamp_channel_becomes_free, &channel_power,
			current_pd, SimTime());

		// Spatial Reuse: identify frame origin and potential OBSS/PD threshold
		UpdateSRStateForIncomingFrame(notification);

		// Decide action according to current state and Notification initiated
		switch(node_state){

			case STATE_SENSING:{
				HandleStartTX_StateSensing(notification);
				break;
			}
			/* ---------- */
			case STATE_TX_DATA:
			case STATE_TX_ACK:
			case STATE_TX_DATA_DSO:
			case STATE_TX_DATA_NPCA:{
				HandleStartTX_StateTxData(notification);
				break;
			}
			/* ---------- */
			case STATE_RX_DATA:
			case STATE_RX_ACK:{
				HandleStartTX_StateRxData(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_ACK:
			case STATE_WAIT_ACK_DSO:
			case STATE_WAIT_ACK_NPCA:{
				HandleStartTX_StateWaitAck(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_CTS:{
				HandleStartTX_StateWaitCts(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_DATA:{
				HandleStartTX_StateWaitData(notification);
				break;
			}
			/* ---------- */
			case STATE_TX_RTS:
			case STATE_TX_CTS:{
				HandleStartTX_StateTxData(notification);
				break;
			}
			/* ---------- */
			case STATE_RX_RTS:
			case STATE_RX_CTS:{
				HandleStartTX_StateRxData(notification);
				break;
			}
			/* ---------- */
			/* ---------- */
			case STATE_NAV:{
				HandleStartTX_StateNav(notification);
				break;
			}
			case STATE_SLEEP:{
				// do nothing
				break;
			}
			/* ---------- */
			/* MAPC states: TX states ignore incoming (already transmitting) */
			case STATE_TX_ICF:
			case STATE_TX_ICR:
			case STATE_TX_TF:
			case STATE_TX_ACK_TF:
			case STATE_TX_DSO_ICF:
			case STATE_TX_NPCA_ICF:{
				HandleStartTX_StateTxData(notification);
				break;
			}
			/* ---------- */
			/* MAPC reception states: treat as RX interference */
			case STATE_RX_ICF:
			case STATE_RX_ICR:
			case STATE_RX_DSO_ICR:
			case STATE_RX_NPCA_ICR:
			case STATE_RX_MU_RTS:
			case STATE_RX_TF:{
				HandleStartTX_StateRxData(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_ICR:{
				HandleStartTX_StateWaitIcr(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_MU_RTS:{
				HandleStartTX_StateWaitMuRts(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_TF:{
				HandleStartTX_StateWaitTf(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_ACK_TF:{
				// ACK TF is in flight from coordinator; do nothing (handle completion in FinishTX)
				HandleStartTX_StateTxData(notification);
				break;
			}
			/* ---------- */
			default:{
				printf("ERROR: %d is not a correct state\n", node_state);
				exit(EXIT_FAILURE);
			}


		}


	}

	// STATISTICS: compute the time the channel is idle (Node 0 is responsible to monitor this)
	if (node_params.node_id == 0 && node_stats.channel_idle) {
		node_stats.sum_time_channel_idle += (SimTime() - node_stats.last_time_channel_is_idle);
		node_stats.channel_idle = false;
	}

	// LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s InportSomeNodeStartTX() END\n", SimTime(), node_params.node_id, node_state, LOG_D01, LOG_LVL1);
};

/**
 * Helper for InportSomeNodeFinishTX: handles STATE_SENSING case
 */
void Node :: HandleFinishTX_StateSensing(const Notification &notification){

	// NPCA STA: DATA phase ended while still in SENSING (DATA was lost); restore primary channel
	if (npca_sta_on_npca_channel && notification.packet_type == PACKET_TYPE_DATA
			&& notification.source_id == wlan.ap_id) {
		node_params.current_primary_channel = npca_stored_primary_channel;
		npca_sta_on_npca_channel = 0;
	}

	if(node_is_transmitter) {
		if(!trigger_start_backoff.Active()
			&& !trigger_end_backoff.Active()){	// BO was paused and DIFS not initiated

			int resume (HandleBackoff(RESUME_TIMER, &channel_power, node_params.current_primary_channel, current_pd,
					buffer.QueueSize()));

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm (%f)\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
				node_params.current_primary_channel, ConvertPower(PW_TO_DBM, channel_power[node_params.current_primary_channel]), channel_power[node_params.current_primary_channel]);

			if (resume) {	// BO can be resumed
				// Sergio on 26/09/2017. EIFS vs NAV.
				// - To identify if previous packet lost to trigger the EIFS
				// - If not, just resume the backoff
				time_to_trigger = SimTime() + DIFS;
				// time_to_trigger = SimTime() + SIFS + notification.tx_info.cts_duration + DIFS;
				trigger_start_backoff.Set(FixTimeOffset(time_to_trigger,13,12));
				LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s BO will be resumed after DIFS at %.12f.\n",
					SimTime(), node_params.node_id, node_state, LOG_E11, LOG_LVL4,
					trigger_start_backoff.GetTime());
//							LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s EIFS started.\n",
//														SimTime(), node_params.node_id, node_state, LOG_E11, LOG_LVL4);
			} else {	// BO cannot be resumed
				LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s EIFS/DIFS cannot be started because the channel is busy.\n",
					SimTime(), node_params.node_id, node_state, LOG_E11, LOG_LVL4);

}
		} else {	// BO was already active
			LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s BO was already active.\n",
					SimTime(), node_params.node_id, node_state, LOG_E11, LOG_LVL4);
		}
	}
}

/**
 * Helper for InportSomeNodeFinishTX: handles STATE_RX_DATA case
 */
void Node :: HandleFinishTX_StateRxData(const Notification &notification){

	if(notification.destination_id == node_params.node_id){ 	// Node IS THE DESTINATION

		if(notification.packet_type == PACKET_TYPE_DATA){	// Data packet transmission finished

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Packet #%d reception from N%d is finished successfully.\n",
				SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3, notification.packet_id,
				notification.source_id);

			// ACK suppression: transmitter flagged ack_required=0 — skip ACK, return to sensing
			if (notification.tx_info.ack_required == 0) {
				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s ACK suppressed by TX for DATA #%d from N%d. Returning to sensing.\n",
					SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
					notification.packet_id, notification.source_id);
				current_tx_duration = 0;	// receiver sent nothing; don't charge TX time in RestartNode
				RestartNode(FALSE);
				return;
			}

			// Build ACK notification (sent immediately for normal case, or after ACK TF for Co-BF/Co-SR)
			// NAV must be computed *before* GenerateNotification so that sensing nodes receiving
			// the ACK start notification get the correct nav_time (avoids COST e->time>=m_clock crash).
			current_tx_duration = ack_duration;
			current_destination_id = notification.source_id;
			current_nav_time = ComputeNavTime(STATE_TX_ACK, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
			current_nav_time = FixTimeOffset(current_nav_time, 13, 12);
			// Co-BF/Co-SR: check MAPC method before building ACK notification
			int _cobf_cosr_g = wlan.FindMapcGroupIdx(notification.mapc_group_id);
			int _cobf_cosr = (_cobf_cosr_g >= 0)
				&& (wlan.mapc_method_ids[_cobf_cosr_g] == CO_BF
					|| wlan.mapc_method_ids[_cobf_cosr_g] == CO_SR);
			if (_cobf_cosr) {
				// Co-BF/Co-SR: both STAs ACK simultaneously.
				// Assign each AP a distinct sub-channel within the bonded range (OFDMA-like)
				// so ACKs don't collide even when both APs share the same primary channel.
				// Rank = count of peer APs with smaller node_id -> unique offset per AP.
				int _rank = 0;
				for (int _p = 0; _p < wlan.mapc_num_peers[_cobf_cosr_g]; ++_p) {
					if (wlan.mapc_peer_ap_ids[_cobf_cosr_g][_p] < wlan.ap_id)
						++_rank;
				}
				int _num_ch = current_right_channel - current_left_channel + 1;
				int _ack_ch = current_left_channel + (_rank % _num_ch);
				current_left_channel  = _ack_ch;
				current_right_channel = _ack_ch;
			}
			ack_notification = GenerateNotification(PACKET_TYPE_ACK, current_destination_id,
					notification.packet_id, notification.tx_info.num_packets_aggregated,
					notification.timestamp_generated, current_tx_duration);

						// Reset the flag that indicates whether the tx power changed or not
						sr_state.flag_change_in_tx_power = FALSE;

						// Set the preoccupancy duration for the ACK
				ack_notification.tx_info.preoccupancy_duration = time_rand_value;

			// ------------------------------------------------------------------------
			// Sergio on 07 Dec 2017: add ACK transmission time to spectrum utilization
			for(int c = current_left_channel; c <= current_right_channel; ++c){
				node_stats.total_time_channel_busy_per_channel[c] = node_stats.total_time_channel_busy_per_channel[c] + current_tx_duration;
			}
			// ------------------------------------------------------------------------

			if (_cobf_cosr) {
				// Co-BF/Co-SR: coordinator will send ACK TF; hold ACK until then.
				// Timeout = SIFS + rts_duration (ACK TF size) + extra; use rts_duration,
				// NOT cts_duration — ACK TF is the same size as RTS, not CTS.
				node_state = STATE_WAIT_ACK_TF;
				time_to_trigger = SimTime() + SIFS + notification.tx_info.rts_duration + TIME_OUT_EXTRA_TIME;
				trigger_DATA_timeout.Set(FixTimeOffset(time_to_trigger, 13, 12));
				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s DATA from N%d done; waiting for ACK TF from coordinator.\n",
					SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3, notification.source_id);
			} else {
				// Normal case: send ACK directly after SIFS (nav already computed above)
				node_state = STATE_TX_ACK;
				time_to_trigger = SimTime() + SIFS;
				trigger_SIFS.Set(FixTimeOffset(time_to_trigger,13,12));
				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s SIFS will be triggered in %.12f\n",
					SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
					trigger_SIFS.GetTime());
			}

		} else {	// Other packet type transmission finished
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
		}

	} else {	// Node IS NOT THE DESTINATION, do nothing

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Still locked into the reception of packet #%d from N%d.\n",
			SimTime(), node_params.node_id, node_state, LOG_E15, LOG_LVL3, notification.packet_id,
			notification.source_id);

	}
}

/**
 * Helper for InportSomeNodeFinishTX: handles STATE_TX_DATA/STATE_TX_ACK/STATE_WAIT_ACK/etc. cases
 */
void Node :: HandleFinishTX_StateTxData(const Notification &notification){
	// Do nothing
}

/**
 * Called when some node (this one included) finishes a packet TX (RTS, CTS, Data, or ACK)
 * @param "notification" [type Notification]: notification containing the information of the transmission that has finished
 */
void Node :: InportSomeNodeFinishTX(Notification &notification){

	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s InportSomeNodeFinishTX(): N%d to N%d (type %d)"
			" at range %d-%d "
			"- nodes transmitting: ",
		SimTime(), node_params.node_id, node_state, LOG_E00, LOG_LVL1,
		notification.source_id, notification.destination_id, notification.packet_type,
		notification.left_channel, notification.right_channel);

	// Identify node that has finished the transmission as non-transmitting node in the array
	nodes_transmitting[notification.source_id] = FALSE;
	PrintOrWriteNodesTransmitting(WRITE_LOG, node_params.save_node_logs,
			node_params.print_node_logs, node_logger, node_params.total_nodes_number, nodes_transmitting);

	// Update the list of neighboring devices
	//UpdateTokenList(ADD_DEVICE_TO_LIST, &token_order_list, notification.source_id);

	// TOKENIZED BO ONLY
	if(node_is_transmitter && node_params.backoff_type == BACKOFF_TOKENIZED &&
			token_order_list[notification.destination_id] == DEVICE_ACTIVE_FOR_TOKEN) {
		// - Check that the incoming transmission is an ACK
		if (notification.packet_type == PACKET_TYPE_ACK) {
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Token-based channel access operation (token RELEASE):\n",
				SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL4);
			// Update the status of the token
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s UPDATING the status of the token (until now, with N%d)\n",
				SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3, token_status);
			UpdateTokenStatus(node_params.node_id, RELEASE_TOKEN, &token_status, notification.destination_id,
					token_order_list, node_params.total_nodes_number, &distance_to_token);
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Token status updated, the new token holder is %d\n",
				SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL5, token_status);
			// Update the CW parameters
			HandleContentionWindow(
				node_params.cw_adaptation, -1, &ca_state.deterministic_bo_active, &ca_state.current_cw_min, &ca_state.current_cw_max, &ca_state.cw_stage_current,
				node_params.cw_min_default, node_params.cw_max_default, node_params.cw_stage_max, distance_to_token, node_params.backoff_type, current_traffic_type);
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Updated CW parameters (token-based BO) = [%d-%d]\n",
				SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL5, ca_state.current_cw_min, ca_state.current_cw_max);
		}
	}

	if(notification.source_id == node_params.node_id){	// Node is the TX source: do nothing

//		LOGS(node_params.save_node_logs,node_logger.file,
//				"%.15f;N%d;S%d;%s;%s I have finished the TX of packet #%d (type %d) in channel range: %d - %d\n",
//				SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL2, notification.packet_id,
//				notification.packet_type, notification.left_channel, notification.right_channel);

	} else {	// Node is not the TX source

//		LOGS(node_params.save_node_logs,node_logger.file,
//				"%.15f;N%d;S%d;%s;%s N%d has finished the TX of packet #%d (type %d) in channel range: %d - %d\n",
//				SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL2, notification.source_id,
//				notification.packet_id, notification.packet_type, notification.left_channel,
//				notification.right_channel);

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Power sensed per channel BEFORE updating [dBm]: ",
			SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3);

		PrintOrWriteChannelPower(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
					&channel_power);

		// Update the power sensed at each channel (raw power, matching TX_INITIATED).
		UpdateChannelsPower(&channel_power, notification, TX_FINISHED,
			node_params.central_frequency, node_params.path_loss_model, node_params.adjacent_channel_model, received_power_array[notification.source_id], node_params.node_id);

		// -------------------------
		// Safety condtion. Empty the channel when no node is transmitting
		int num_nodes_transmitting = 0;
		for(int i = 0; i < node_params.total_nodes_number; ++i){
			if(nodes_transmitting[i] == TRUE){
				++num_nodes_transmitting;
			}
		}
		if(num_nodes_transmitting == 0){
			for(int i = 0; i < NUM_CHANNELS_KOMONDOR; ++i){
				channel_power[i] = 0;
			}
		}
		// End of safety condition
		// -------------------------

		// Spatial Reuse: update type of ongoing transmissions
		if(sr_state.spatial_reuse_enabled) {
			UpdateTypeOngoingTransmissions(sr_state.type_ongoing_transmissions_sr,
				notification, node_params.bss_color, node_params.srg, 0);
		}

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Power sensed per channel [dBm]: ",
			SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3);

		PrintOrWriteChannelPower(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
				&channel_power);

		// Call UpdatePowerSensedPerNode() ONLY for adding power (some node started)
		UpdatePowerSensedPerNode(node_params.current_primary_channel, power_received_per_node, notification,
			node_params.central_frequency, node_params.path_loss_model, received_power_array[notification.source_id], TX_FINISHED,
			node_params.x, node_params.y, node_params.z);

		UpdateTimestampChannelFreeAgain(timestamp_channel_becomes_free, &channel_power,
			current_pd, SimTime());


		switch(node_state){

			/* STATE_SENSING:
			 * - handle backoff
			 */
			case STATE_SENSING:{
				HandleFinishTX_StateSensing(notification);
				break;
			}

			/* STATE_RX_DATA:
			 * - If node IS the destination and data packet transmission finished:
			 *   * start SIFS and generate ACK
			 *   * state = STATE_TX_ACK
			 * - If node IS NOT the destination: do nothing
			 */
			case STATE_RX_DATA:{
				HandleFinishTX_StateRxData(notification);
				break;
			}

			/* STATE_RX_ACK:
			 * - If node IS the destination and ACK packet transmission finished:
			 *   * decrease contention window and restart node
			 *   * state = STATE_SENSING (implicit on restart)
			 * - If node IS NOT the destination: do nothing
			 */
			case STATE_RX_ACK:{	// Check if the current reception is finished

				if(notification.destination_id == node_params.node_id){	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_ACK){	// ACK packet transmission finished

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s ACK #%d reception from N%d is finished successfully.\n",
							SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3, notification.packet_id,
							notification.source_id);

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s -------------------------------------\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL2);
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s TRANSMISSION #%d SUCCESSFULLY FINISHED!\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL2,
							node_stats.data_packets_acked);
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s -------------------------------------\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL2);

						// Mark the previous transmission as successful
						last_transmission_successful = 1;

						// Update EWMA: ACK received — reliability is confirmed for this destination
						if (current_destination_id >= 0
								&& current_destination_id < node_params.total_nodes_number) {
							ack_success_ewma[current_destination_id] =
								(1.0 - ACK_SUPPRESS_EWMA_ALPHA) * ack_success_ewma[current_destination_id]
								+ ACK_SUPPRESS_EWMA_ALPHA * 1.0;
							++ack_exchange_count[current_destination_id];
						}

						// Whole data packet ACKed
						++node_stats.data_packets_acked;
						++node_stats.data_packets_acked_per_sta[current_destination_id-node_params.node_id-1];
						++performance_report.data_packets_acked;

						current_tx_duration = current_tx_duration + (notification.tx_duration + SIFS);	// Add ACK time to tx_duration

						// Update packet statistics
						for(int i = 0; i < limited_num_packets_aggregated; ++i){

							++node_stats.data_frames_acked;
							++node_stats.data_frames_acked_per_sta[current_destination_id-node_params.node_id-1];
							++performance_report.data_frames_acked;
							++node_stats.num_delay_measurements;
							// Use GetPacketAt(i) so each aggregated frame gets its own timestamp
							double pkt_ts = buffer.GetPacketAt(i).timestamp_generated;
							node_stats.sum_delays = node_stats.sum_delays + (SimTime() - pkt_ts);

							if (pkt_ts > (node_params.simulation_time_komondor - node_stats.last_measurements_window)) {
								++node_stats.last_data_frames_acked;
								++node_stats.last_num_delay_measurements;
								node_stats.last_sum_delays = node_stats.last_sum_delays + (SimTime() - pkt_ts);
							}

							// Update the performance report with delay measurements
							performance_report.sum_delays = performance_report.sum_delays + (SimTime() - pkt_ts);
							++performance_report.num_delay_measurements;
							if ((SimTime() - pkt_ts) > performance_report.max_delay) {
								performance_report.max_delay = (SimTime() - pkt_ts);
							}
							if ((SimTime() - pkt_ts) < performance_report.min_delay) {
								performance_report.min_delay = (SimTime() - pkt_ts);
							}
							LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Packet delay: %f us (generated at %f).\n",
								SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL4,
								(SimTime() - pkt_ts) * pow(10,6),
								pkt_ts);

							// Deletion handled in PrepareNewTransmission (node_packet_methods.h)
							//buffer.DelFirstPacket();

						}

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Handling contention window\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
						LOGS(node_params.save_node_logs,node_logger.file,
									"%.15f;N%d;S%d;%s;%s From CW = [%d-%d], b = %d, m = %d\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
							ca_state.current_cw_min, ca_state.current_cw_max, ca_state.cw_stage_current, node_params.cw_stage_max);
						// - Transmission succeeded ---> reset CW if binary exponential backoff is implemented
						HandleContentionWindow(
							node_params.cw_adaptation, RESET_CW, &ca_state.deterministic_bo_active, &ca_state.current_cw_min, &ca_state.current_cw_max,
							&ca_state.cw_stage_current, node_params.cw_min_default, node_params.cw_max_default, node_params.cw_stage_max, distance_to_token, node_params.backoff_type, current_traffic_type);

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s To CW = [%d-%d], b = %d, m = %d\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
							ca_state.current_cw_min, ca_state.current_cw_max, ca_state.cw_stage_current, node_params.cw_stage_max);

						// Only the TXOP coordinator sends MU-RTS after its own DATA/ACK.
						// coordinator_ap_id == NODE_ID_NONE identifies the coordinator
						// (reset in EndBackoff; set to peer id when an ICF is received).
						if (wlan.mapc_enabled && wlan.mapc_method_ids[mapc_active_group_idx] == CO_TDMA
								&& coordinator_ap_id == NODE_ID_NONE) {
							if (mapc_peer_has_data) {
								// Peer has data (reported in ICR): grant its TDMA slot via MU-RTS/TXS
								node_state = STATE_TX_RTS;
								current_tx_duration = rts_duration;
								current_destination_id = mapc_selected_peer_id;
								mu_rts_notification = GenerateNotification(PACKET_TYPE_MU_RTS_TXS,
									mapc_selected_peer_id, data_notification.packet_id,
									limited_num_packets_aggregated,
									data_notification.timestamp_generated, current_tx_duration);
								mu_rts_notification.mapc_group_id = wlan.mapc_group_ids[mapc_active_group_idx];
								// MU-RTS NAV covers AP_B DATA + ACK slot (uses coordinator's capped data_duration)
								current_nav_time = SIFS + data_duration + SIFS + ack_duration;
								mu_rts_notification.tx_info.nav_time = current_nav_time;
								// Pass per-AP duration allocation to coordinated AP
								mu_rts_notification.tx_info.mapc_allocated_data_duration = mapc_txop_per_ap_data_duration;
								time_rand_value = 0;
								time_to_trigger = SimTime() + SIFS;
								trigger_SIFS.Set(FixTimeOffset(time_to_trigger, 13, 12));
								LOGS(node_params.save_node_logs,node_logger.file,
									"%.15f;N%d;S%d;%s;%s Sending MU-RTS/TXS to N%d (selected peer) after SIFS\n",
									SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
									current_destination_id);
							} else {
								// Peer has no data (reported in ICR): skip MU-RTS slot
								LOGS(node_params.save_node_logs,node_logger.file,
									"%.15f;N%d;S%d;%s;%s N%d has no data (ICR flag); skipping MU-RTS slot\n",
									SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
									mapc_selected_peer_id);
								++mapc_current_peer_idx;
								RestartNode(FALSE);
							}
						} else {
							// Standard: restart node
							RestartNode(FALSE);
						}

					} else {	// Other packet type transmission finished
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION

					LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Still receiving packet #%d reception from N%d.\n",
						SimTime(), node_params.node_id, node_state, LOG_E15, LOG_LVL3, incoming_notification.packet_id,
						incoming_notification.source_id);
				}

				break;
			}

			/* STATE_TX_DATA || STATE_TX_ACK:
			 * - Do nothing
			 */
			case STATE_TX_DATA:
			case STATE_TX_ACK:
			case STATE_WAIT_ACK:
			case STATE_TX_DATA_DSO:
			case STATE_WAIT_ACK_DSO:
			case STATE_TX_DATA_NPCA:
			case STATE_WAIT_ACK_NPCA:
			case STATE_TX_RTS:
			case STATE_TX_CTS:
			case STATE_WAIT_CTS:
			case STATE_WAIT_DATA:
			case STATE_NAV:{
				HandleFinishTX_StateTxData(notification);
				break;
			}

			/* STATE_RX_RTS:
			 * -
			 */
			case STATE_RX_RTS:{

				if(notification.destination_id == node_params.node_id){	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_RTS){	// RTS packet transmission finished

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s RTS #%d reception from N%d is finished successfully.\n",
							SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3, notification.packet_id,
							notification.source_id);

						// Check channel availability in order to send the CTS
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Checking if CTS can be sent: P_sen = %f dBm, pd = %f dBm.\n",
							SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
							ConvertPower(PW_TO_DBM, channel_power[node_params.current_primary_channel]),
							ConvertPower(PW_TO_DBM, current_pd));


						// Issue #146 RTS/CTS BW indication
						// - If incoming packet is RTS decodable:
						// 1. Perform CCA assessment in full range
						// 2. Derive new operation BW (e.g., if original range was 0 to 7 and 5 is busy, report 0 to 3)
						// 3. Transmit (later) CTS

						int CTS_transmission_possible = FALSE;

						GetChannelOccupancyByCCA(node_params.current_primary_channel, node_params.pifs_activated, channels_free, current_left_channel,
								current_right_channel, &channel_power, current_pd, timestamp_channel_becomes_free, SimTime(), PIFS);

						LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Channels founds free after RTS: ",
								SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL3);

						PrintOrWriteChannelsFree(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
							channels_free);

						GetTxChannels(channels_for_tx, node_params.current_dcb_policy, channels_free,
								current_left_channel, current_right_channel, node_params.current_primary_channel,
								NUM_CHANNELS_KOMONDOR, &channel_power, channel_aggregation_cca_model, NULL);
						
						LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Channels for transmitting after RTS: ",
								SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL2);

						PrintOrWriteChannelForTx(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
							channels_for_tx);

						if(channels_for_tx[0] != TX_NOT_POSSIBLE){

							// Get the transmission channels
							current_left_channel = GetFirstOrLastTrueElemOfArray(FIRST_TRUE_IN_ARRAY,
								channels_for_tx, NUM_CHANNELS_KOMONDOR);
							current_right_channel = GetFirstOrLastTrueElemOfArray(LAST_TRUE_IN_ARRAY,
								channels_for_tx, NUM_CHANNELS_KOMONDOR);

							CTS_transmission_possible = TRUE;

						} else{
							CTS_transmission_possible = FALSE;
						}


						//if(ConvertPower(PW_TO_DBM, channel_power[node_params.current_primary_channel]) < current_pd) {

						if(CTS_transmission_possible){

							LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Channel(s) is (are) clear! Sending CTS to N%d (STATE = %d) ...\n",
								SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3, current_destination_id, node_state);

							node_state = STATE_TX_CTS;
							// Generate and send CTS to transmitter after SIFS
							current_destination_id = notification.source_id;
							current_tx_duration = cts_duration;

							// Compute the NAV time
							bits_ofdm_sym =  GetNumberSubcarriers(current_right_channel - current_left_channel +1) *
								Mcs_array::modulation_bits[notification.modulation_id-1] *
								Mcs_array::coding_rates[notification.modulation_id-1] *
								IEEE_AX_SU_SPATIAL_STREAMS;

							ComputeFramesDuration(&rts_duration, &cts_duration, &data_duration, &ack_duration,
								num_channels_tx, notification.modulation_id, notification.tx_info.num_packets_aggregated,
								node_params.frame_length, bits_ofdm_sym);

							current_nav_time = ComputeNavTime(node_state, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
							current_nav_time = FixTimeOffset(current_nav_time,13,12); // Update the NAV time according to the time offsets

							// ------------------------------------------------------------------------
							// Sergio on 07 Dec 2017: add CTS transmission time to spectrum utilization
							for(int c = current_left_channel; c <= current_right_channel; ++c){
								node_stats.total_time_channel_busy_per_channel[c] = node_stats.total_time_channel_busy_per_channel[c] + current_tx_duration;
							}
							// ------------------------------------------------------------------------

							time_to_trigger = SimTime() + SIFS;
							trigger_SIFS.Set(FixTimeOffset(time_to_trigger,13,12)); // triggers the SendResponsePacket() function after SIFS

							LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s SIFS will be triggered in %.12f\n",
								SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
								trigger_SIFS.GetTime());

							cts_notification = GenerateNotification(PACKET_TYPE_CTS, current_destination_id,
								notification.packet_id, notification.tx_info.num_packets_aggregated,
								notification.timestamp_generated, notification.tx_info.total_tx_power);

							cts_notification.tx_duration = current_tx_duration;

                            // Reset the flag that indicates whether the tx power changed or not
                            sr_state.flag_change_in_tx_power = FALSE;


							// Workaround to solve the e->clock timer issue
							// (occurs when being in NAV and noticing a collision of two or more CTS frames)
							cts_notification.tx_info.preoccupancy_duration = time_rand_value;

						} else {
							// CANNOT START PACKET TX

							LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s CTS TX NOT POSSIBLE\n",
								SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);

							/*
							 * STAs should wait MAX_DIFFERENCE_SAME_TIME in order to avoid entering in NAV when it is not required.
							 * E.g. STA A is sensing and is able to decode a packet from AP A. At the same time AP B transmits and
							 * harms AP A - STA A transmission. STA A is restarted. Again, at the same time AP C transmits. Then,
							 * in order to avoid entering in NAV when in fact a slotted BO collision did happen, STA A should not
							 * listen to AP C packet. After MAX_DIFFERENCE_SAME_TIME, no same time events are ensured and STA A can
							 * start sensing again.
							 */
							if(!node_is_transmitter) {
								time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;
								trigger_restart_sta.Set(FixTimeOffset(time_to_trigger,13,12));
							} else {
								RestartNode(FALSE);
							}
						}

					} else {	// Other packet type transmission finished
						LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
								SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION

					LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Still receiving packet #%d reception from N%d.\n",
							SimTime(), node_params.node_id, node_state, LOG_E15, LOG_LVL3, incoming_notification.packet_id,
							incoming_notification.source_id);
				}

				break;
			}


			/* STATE_RX_CTS:
			 * -
			 */
			case STATE_RX_CTS:{

				if(notification.destination_id == node_params.node_id){	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_CTS){	// CTS packet transmission finished

						LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s CTS #%d reception from N%d is finished successfully.\n",
								SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
								notification.packet_id, notification.source_id);

						node_state = STATE_TX_DATA;

						// Compute the NAV time
						bits_ofdm_sym =  GetNumberSubcarriers(current_right_channel - current_left_channel +1) *
							Mcs_array::modulation_bits[notification.modulation_id-1] *
							Mcs_array::coding_rates[notification.modulation_id-1] *
							IEEE_AX_SU_SPATIAL_STREAMS;

						ComputeFramesDuration(&rts_duration, &cts_duration, &data_duration, &ack_duration,
							num_channels_tx, notification.modulation_id, notification.tx_info.num_packets_aggregated,
							node_params.frame_length, bits_ofdm_sym);

						limited_num_packets_aggregated = notification.tx_info.num_packets_aggregated;

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Transmitting DATA (N_agg = %d) in %d channels using modulation %d (%.0f bits per OFDM symbol ---> %.2f Mbps) \n",
							SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL4, limited_num_packets_aggregated,
							(current_right_channel - current_left_channel + 1), current_modulation, bits_ofdm_sym,
							bits_ofdm_sym/IEEE_AX_OFDM_SYMBOL_GI32_DURATION * pow(10,-6));

						// Compute the NAV time
						current_nav_time = ComputeNavTime(node_state, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
						current_nav_time = FixTimeOffset(current_nav_time,13,12); // Update the NAV time according to the time offsets

						// Generate and send DATA to transmitter after SIFS
						current_destination_id = notification.source_id;

						current_tx_duration = data_duration;	// This duration already computed in EndBackoff
						time_to_trigger = SimTime() + SIFS;

						// ------------------------------------------------------------------------
						// Sergio on 07 Dec 2017: add DATA transmission time to spectrum utilization
						for(int c = current_left_channel; c <= current_right_channel; ++c){
							node_stats.total_time_channel_busy_per_channel[c] = node_stats.total_time_channel_busy_per_channel[c] + current_tx_duration;
						}
						// ------------------------------------------------------------------------

						trigger_SIFS.Set(FixTimeOffset(time_to_trigger,13,12));

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s SIFS will be triggered in %.12f\n",
							SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
							trigger_SIFS.GetTime());

						data_notification = GenerateNotification(PACKET_TYPE_DATA, current_destination_id,
								notification.packet_id, notification.tx_info.num_packets_aggregated,
								notification.timestamp_generated, current_tx_duration);

						// Reset the flag that indicates whether the tx power changed or not
                        sr_state.flag_change_in_tx_power = FALSE;

						data_notification.tx_info.preoccupancy_duration = time_rand_value;



					} else {	// Other packet type transmission finished
						LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
								SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION

					LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Still receiving packet #%d reception from N%d.\n",
						SimTime(), node_params.node_id, node_state, LOG_E15, LOG_LVL3, incoming_notification.packet_id,
						incoming_notification.source_id);
				}

				break;
			}

			case STATE_SLEEP:{
				// do nothing
				break;
			}

			/* MAPC TX / wait states + DSO TX/wait states: do nothing when another node finishes TX */
			case STATE_TX_ICF:
			case STATE_TX_ICR:
			case STATE_TX_TF:
			case STATE_TX_ACK_TF:
			case STATE_WAIT_ICR:
			case STATE_WAIT_MU_RTS:
			case STATE_WAIT_TF:
			case STATE_TX_DSO_ICF:
			case STATE_RX_DSO_ICR:
			case STATE_TX_NPCA_ICF:
			case STATE_RX_NPCA_ICR:{
				HandleFinishTX_StateTxData(notification);
				break;
			}

			/* MAPC reception states: handle frame reception completion */
			case STATE_RX_ICF:{
				HandleFinishTX_StateRxIcf(notification);
				break;
			}
			case STATE_RX_ICR:{
				HandleFinishTX_StateRxIcr(notification);
				break;
			}
			case STATE_RX_MU_RTS:{
				HandleFinishTX_StateRxMuRts(notification);
				break;
			}
			case STATE_RX_TF:{
				HandleFinishTX_StateRxTf(notification);
				break;
			}
			case STATE_WAIT_ACK_TF:{
				HandleFinishTX_StateWaitAckTf(notification);
				break;
			}

			default:{
				printf("ERROR: %d is not a correct state\n", node_state);
				exit(EXIT_FAILURE);
				break;
			}
		}


	}

	// STATISTICS: compute the time the channel is idle (Node 0 is responsible to monitors this)
	if (node_params.node_id == 0) {
		int num_nodes_transmitting = 0;
		for(int i = 0; i < node_params.total_nodes_number; ++i){
			if(nodes_transmitting[i] == TRUE){
				++ num_nodes_transmitting;
			}
		}
		// Check if nobody is transmitting
		if (num_nodes_transmitting == 0) {
			// If no one is transmitting, set the current SimTime() as the last time the channel has been seen idle
			node_stats.last_time_channel_is_idle = SimTime();
			node_stats.channel_idle = true;
		}
	}

	// LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s InportSomeNodeFinishTX() END",	SimTime(), node_params.node_id, node_state, LOG_E01, LOG_LVL1);
};

#endif /* NODE_FSM_METHODS_H */
