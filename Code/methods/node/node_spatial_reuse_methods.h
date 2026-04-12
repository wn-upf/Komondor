/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * node_spatial_reuse_methods.h: Node spatial-reuse event handler and inport
 *   method implementations.
 *
 * NOTE: This file is an implementation fragment. It must be included from node.h
 *   after the Node class definition, not included directly.
 *
 * Functions defined here:
 *   - Node::SpatialReuseOpportunityEnds
 *   - Node::InportRequestSpatialReuseConfiguration
 *   - Node::InportNewSpatialReuseConfiguration
 *   - Node::UpdateSRStateForIncomingFrame  (common pre-processing for every StartTX event)
 *   - Node::TryIdentifySRTXOP             (identify SR TXOP while sensing)
 *   - Node::ComputeNavCollisions          (compute nav/inter-bss-nav collision flags)
 *   - Node::DetectSRTXOPInNavState        (detect / cancel SR TXOP while in NAV)
 *   - Node::DetectSRTXOPWhileTransmitting (detect / cancel SR TXOP while transmitting)
 *   - Node::ApplySRParametersAtBackoffEnd (apply SR TX power limits when backoff expires)
 *   - Node::CheckSRTXOPAtCallSensing      (detect SR TXOP when BO cannot be resumed)
 */

#ifndef NODE_SPATIAL_REUSE_METHODS_H
#define NODE_SPATIAL_REUSE_METHODS_H

/**
 * Called when an SR-based TXOP finished (trigger-based operation)
 */
void Node :: SpatialReuseOpportunityEnds(trigger_t &){
	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s SpatialReuseOpportunityEnds()\n",
		SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL2);
	// Set the SR parameters to the default values (disable mechanism to activate SR opportunities)
	sr_state.current_obss_pd_threshold = current_pd;
	sr_state.txop_sr_identified = FALSE;
	sr_state.current_tx_power_sr = current_tx_power;
	sr_state.flag_change_in_tx_power = FALSE;
	// Indicate that the MCS must be changed
	for(int n = 0; n < wlan.num_stas; ++n) {
		change_modulation_flag[n] = true;
	}

	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s sr_state.current_obss_pd_threshold = %f\n",
		SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL3,
		ConvertPower(PW_TO_DBM,sr_state.current_obss_pd_threshold));
	LOGS(node_params.save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s sr_state.current_tx_power_sr = %f\n",
		SimTime(), node_params.node_id, node_state, LOG_F00, LOG_LVL3,
		ConvertPower(PW_TO_DBM,sr_state.current_tx_power_sr));

}

/**
 * Called when a request is received for returning the current SR configuration
 */
void Node :: InportRequestSpatialReuseConfiguration() {
	// Update the SR configuration
	spatial_reuse_configuration.capabilities = node_params;
	spatial_reuse_configuration.spatial_reuse_enabled = sr_state.spatial_reuse_enabled;
	spatial_reuse_configuration.non_srg_obss_pd = node_params.non_srg_obss_pd;
	spatial_reuse_configuration.srg_obss_pd = node_params.srg_obss_pd;
	// Send it to STAs
	outportNewSpatialReuseConfiguration(spatial_reuse_configuration);
}

/**
 * Called when a new SR configuration is received
 * @param "received_configuration" [type Configuration]: received SR configuration
 */
void Node :: InportNewSpatialReuseConfiguration(Configuration &received_configuration) {
	sr_state.spatial_reuse_enabled = received_configuration.spatial_reuse_enabled;
	node_params.bss_color = received_configuration.capabilities.bss_color;
	node_params.srg = received_configuration.capabilities.srg;
	node_params.non_srg_obss_pd = received_configuration.non_srg_obss_pd;
	node_params.srg_obss_pd = received_configuration.srg_obss_pd;
}

/**
 * Update sr_state for an incoming frame at the start of InportSomeNodeStartTX.
 * Identifies the frame's BSS origin and computes the potential OBSS/PD threshold.
 * Called once per StartTX event before the per-state dispatch switch.
 * @param "notification" [type const Notification&]: incoming TX notification
 */
void Node :: UpdateSRStateForIncomingFrame(const Notification &notification) {
	if (sr_state.spatial_reuse_enabled) {
		// Identify the source of detected packet
		sr_state.type_last_sensed_packet = CheckPacketOrigin(notification, node_params.bss_color, node_params.srg);
		// Obtain the CST to be used
		sr_state.potential_obss_pd_threshold = GetSensitivitySpatialReuse(sr_state.type_last_sensed_packet,
			node_params.srg_obss_pd, node_params.non_srg_obss_pd, current_pd, power_received_per_node[notification.source_id]);
		// In case of detecting an inter-BSS frame, print the information
		if (sr_state.type_last_sensed_packet != INTRA_BSS_FRAME) {
			LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s SPATIAL REUSE OPERATION: \n",
				SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL3);
			LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s sr_state.type_last_sensed_packet = %d\n",
				SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL4, sr_state.type_last_sensed_packet);
			LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s Previous sr_state.current_obss_pd_threshold = %f\n",
				SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL4, ConvertPower(PW_TO_DBM, sr_state.current_obss_pd_threshold));
			LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s previous sr_state.txop_sr_identified = %d\n",
				SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL4, sr_state.txop_sr_identified);
			LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s New sr_state.potential_obss_pd_threshold = %f\n",
				SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL4, ConvertPower(PW_TO_DBM, sr_state.potential_obss_pd_threshold));
		}
	}
}

/**
 * While in STATE_SENSING, check whether the decoded frame can be ignored under SR rules.
 * If an SR TXOP is identified (new or better than the current one), updates sr_state,
 * sets loss_reason to PACKET_IGNORED_SPATIAL_REUSE, and arms txop_sr_end.
 * @param "notification" [type const Notification&]: incoming TX notification
 * @param "loss_reason"  [type int&]: in/out — current packet loss reason; may be overwritten
 */
void Node :: TryIdentifySRTXOP(const Notification &notification, int &loss_reason) {
	// If the packet is not lost, check if we can ignore it by applying another pd
	if (sr_state.spatial_reuse_enabled && loss_reason == PACKET_NOT_LOST) {
		// The incoming packet can be decoded by the default pd
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s The packet could be decoded with the default pd (%f dBm)...\n",
			SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, ConvertPower(PW_TO_DBM, current_pd));
		// Check if a new SR-based opportunity can be identified to ignore the incoming tranmission
		int new_txop_sr_identified(IdentifySpatialReuseOpportunity(power_rx_interest, sr_state.potential_obss_pd_threshold));
		// Two cases:
		// (1) An SR-based opportunity was already identified and needs to be overwritten
		// (2) None SR opportunites were previously detected
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s sr_state.txop_sr_identified = %d / new_txop_sr_identified = %d\n",
			SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4, sr_state.txop_sr_identified, new_txop_sr_identified);
		if ( (sr_state.txop_sr_identified && new_txop_sr_identified &&
				sr_state.potential_obss_pd_threshold <= sr_state.current_obss_pd_threshold) ||
			(!sr_state.txop_sr_identified && new_txop_sr_identified) ) {
			// Set the current OBSS/PD threshold for inter-BSS transmissions
			sr_state.current_obss_pd_threshold = sr_state.potential_obss_pd_threshold;
			// Update the variable that indicates that an SR-based opportunity has been detected
			sr_state.txop_sr_identified = new_txop_sr_identified;
			// Indicate that the packet was "lost" in order to continue with the backoff procedure
			loss_reason = PACKET_IGNORED_SPATIAL_REUSE;
			// Define the limited transmission power
			sr_state.next_tx_power_limit = ApplyTxPowerRestriction(sr_state.current_obss_pd_threshold, current_tx_power);
			// Start (update) the trigger that indicates the end of the SR-based opportunity
			time_to_trigger = SimTime() + notification.tx_info.nav_time;
			txop_sr_end.Set(FixTimeOffset(time_to_trigger,13,12));
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s An SR TXOP was detected for OBSS_PD = %f dBm "
				"(received RTS/CTS while being in SENSING state.)\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3,
				ConvertPower(PW_TO_DBM, sr_state.current_obss_pd_threshold));
		}
	}
}

/**
 * Compute nav_collision and inter_bss_nav_collision for the incoming notification.
 * Used in STATE_NAV and STATE_TX_DATA handlers to detect simultaneous transmissions.
 * @param "notification"            [type const Notification&]: incoming TX notification
 * @param "nav_collision"           [type int&]: out — set to 1 if intra-BSS NAV collision
 * @param "inter_bss_nav_collision" [type int&]: out — set to 1 if inter-BSS NAV collision
 */
void Node :: ComputeNavCollisions(const Notification &notification,
		int &nav_collision, int &inter_bss_nav_collision) {
	if (sr_state.spatial_reuse_enabled && sr_state.type_last_sensed_packet != INTRA_BSS_FRAME) {
		inter_bss_nav_collision = fabs(nav_notification.timestamp -
			notification.timestamp) < MAX_DIFFERENCE_SAME_TIME;
	} else {
		nav_collision = fabs(nav_notification.timestamp -
			notification.timestamp) < MAX_DIFFERENCE_SAME_TIME;
	}
}

/**
 * While in STATE_NAV, attempt to identify (or cancel) an SR TXOP for the incoming frame.
 * If no SR TXOP is found, also refreshes the inter-BSS or intra-BSS NAV trigger.
 * @param "notification" [type const Notification&]: incoming TX notification
 * @param "loss_reason"  [type int]: current packet loss result (read-only here)
 */
void Node :: DetectSRTXOPInNavState(const Notification &notification, int loss_reason) {
	// Check if the packet could have been decoded with SR pd
	// This allows transmitting once the NAV is over
	int loss_reason_sr (1);
	int power_condition_sr (1);
	if (sr_state.spatial_reuse_enabled && sr_state.type_last_sensed_packet != INTRA_BSS_FRAME && node_is_transmitter) { 	// Check for TXOP
		double power_interference (power_received_per_node[notification.source_id]);
		// SR TXOP: use direct power comparison (consistent units, pW) matching TryIdentifySRTXOP.
		// loss_reason_sr signals "SR opportunity" when the frame is below the SR threshold.
		loss_reason_sr = IdentifySpatialReuseOpportunity(power_interference, sr_state.potential_obss_pd_threshold)
			? PACKET_LOST_LOW_SIGNAL : PACKET_NOT_LOST;
		// Require the frame to also be above the default pd (channel is busy from legacy perspective).
		power_condition_sr = (power_interference >= current_pd);
	}
	if (loss_reason_sr != PACKET_NOT_LOST && power_condition_sr) {
		sr_state.txop_sr_identified = TRUE;	// TXOP identified!
		sr_state.next_pd_spatial_reuse = sr_state.potential_obss_pd_threshold;	// Update the pd
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s TXOP detected while being in NAV state\n",
			SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3);
	} else {
		if (loss_reason == PACKET_NOT_LOST && sr_state.txop_sr_identified) sr_state.txop_sr_identified = FALSE; // Cancel SR TXOP!
		if (sr_state.spatial_reuse_enabled && sr_state.type_last_sensed_packet != INTRA_BSS_FRAME) { // Update inter-BSS NAV trigger
			nav_notification = notification;
			if(trigger_inter_bss_NAV_timeout.GetTime() < notification.tx_info.nav_time) {
				time_to_trigger = SimTime() +  notification.tx_info.nav_time + TIME_OUT_EXTRA_TIME;
				trigger_inter_bss_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Updating inter-BSS NAV timeout to the more restrictive one: From %.12f to %.12f\n",
					SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL4,
					trigger_inter_bss_NAV_timeout.GetTime(), time_to_trigger);
			}
		} else {	// Update NAV trigger
			nav_notification = notification;
			if(trigger_NAV_timeout.GetTime() < notification.tx_info.nav_time) {
				time_to_trigger = SimTime() +  notification.tx_info.nav_time + TIME_OUT_EXTRA_TIME;
				trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Updating NAV timeout to the more restrictive one: From %.12f to %.12f\n",
					SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL4,
					trigger_NAV_timeout.GetTime(), time_to_trigger);
			}
		}
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s New RTS/CTS arrived from (N%d). Setting NAV to new value %.18f\n",
			SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3,
			notification.source_id, trigger_NAV_timeout.GetTime());
	}
}

/**
 * While in STATE_TX_DATA, check whether the incoming inter-BSS frame allows (or cancels)
 * an SR TXOP that can be used in the next transmission attempt.
 * @param "notification" [type const Notification&]: incoming TX notification
 */
void Node :: DetectSRTXOPWhileTransmitting(const Notification &notification) {
	if (sr_state.spatial_reuse_enabled && sr_state.type_last_sensed_packet != INTRA_BSS_FRAME && node_is_transmitter) {

		double power_interference (power_received_per_node[notification.source_id]);
		double sinr_interference (UpdateSINR(power_interference, max_pw_interference));

		// Is packet lost with the default pd?
		// TODO: method for checking whether the detected transmission can be decoded or not
		int loss_reason_legacy (IsPacketLost(node_params.current_primary_channel, notification, notification,
			sinr_interference, node_params.capture_effect, node_params.sensitivity_default, power_interference, node_params.constant_per,
			node_params.node_id, node_params.capture_effect_model));
		// Is packet lost with the SR pd?
		// TODO: method for checking whether the detected transmission can be decoded or not
		int loss_reason_sr (IsPacketLost(node_params.current_primary_channel, notification, notification,
			sinr_interference, node_params.capture_effect, sr_state.potential_obss_pd_threshold, power_interference, node_params.constant_per,
			node_params.node_id, node_params.capture_effect_model));

		if(node_params.save_node_logs && node_params.node_id == 0) LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s sinr_interference = %f - node_params.capture_effect = %f - sr_state.pd_spatial_reuse = %f"
			" - power_interference = %f)\n",
			SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3,
			ConvertPower(LINEAR_TO_DB, sinr_interference), node_params.capture_effect,
			ConvertPower(PW_TO_DBM,sr_state.pd_spatial_reuse),ConvertPower(PW_TO_DBM,power_interference));

		if(node_params.save_node_logs && node_params.node_id == 0) fprintf(node_logger.file,
			"%.15f;N%d;S%d;%s;%s CHECKING TXOP in TX state (pd_sr = %f - lost = %d)\n",
			SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3,
			ConvertPower(PW_TO_DBM,sr_state.pd_spatial_reuse), loss_reason_sr);

		// If the packet has been ignored due to the OBSS_PD, then detect a TXOP
		if (loss_reason_legacy == PACKET_NOT_LOST && loss_reason_sr != PACKET_NOT_LOST) {
			sr_state.txop_sr_identified = TRUE;	// TXOP identified!
			sr_state.current_obss_pd_threshold = sr_state.potential_obss_pd_threshold;
			// Define the limited transmission power
			sr_state.next_tx_power_limit = ApplyTxPowerRestriction(sr_state.current_obss_pd_threshold, current_tx_power);
			// Start (update) the trigger that indicates the end of the SR-based opportunity
			time_to_trigger = SimTime() + notification.tx_info.nav_time;
			txop_sr_end.Set(FixTimeOffset(time_to_trigger,13,12));
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s TXOP detected while being in TX state\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3);
		} else if (loss_reason_legacy == PACKET_NOT_LOST && sr_state.txop_sr_identified) {
			// Cancel SR TXOP
			sr_state.txop_sr_identified = FALSE;
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s Cancelling SR TXOP while being in TX state\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3);
		}
	}
}

/**
 * At the start of EndBackoff, apply SR TX power and PD threshold if a TXOP was
 * previously identified. No-op if SR is disabled.
 */
void Node :: ApplySRParametersAtBackoffEnd() {
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
}

/**
 * Inside CallSensing, check whether the channel is busy due to an inter-BSS frame that
 * can be ignored under SR rules, identifying a new SR TXOP.
 * Called only when the channel is not free (BO cannot be resumed).
 */
void Node :: CheckSRTXOPAtCallSensing() {
	int loss_reason_sr = 1;	// lost by default
	// Check if the packet can be decoded with the CST indicated by the SR operation
	if (loss_reason == PACKET_NOT_LOST && sr_state.spatial_reuse_enabled) {
		// TODO: method for checking whether the detected transmission can be decoded or not
		loss_reason_sr = IsPacketLost(node_params.current_primary_channel, nav_notification, nav_notification,
			current_sinr, node_params.capture_effect, sr_state.potential_obss_pd_threshold, power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);
		if (loss_reason_sr != PACKET_NOT_LOST && node_is_transmitter) {
			sr_state.txop_sr_identified = TRUE;	// TXOP identified!
			sr_state.current_obss_pd_threshold = sr_state.potential_obss_pd_threshold;	// Update the pd
			if(node_params.save_node_logs) fprintf(node_logger.file,
				"%.15f;N%d;S%d;%s;%s TXOP detected for OBSS_PD = %f dBm (in CallSensing())\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, ConvertPower(PW_TO_DBM, sr_state.current_obss_pd_threshold));
		}
	} else {
		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s BO canot be resumed!\n",
			SimTime(), node_params.node_id, node_state, LOG_Z00, LOG_LVL5);
	}
}

#endif /* NODE_SPATIAL_REUSE_METHODS_H */
