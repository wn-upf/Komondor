/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * node_config_methods.h: Node agent management and configuration method implementations.
 *
 * NOTE: This file is an implementation fragment. It must be included from node.h
 *   after the Node class definition, not included directly.
 *
 * Functions defined here:
 *   - Node::GenerateConfiguration
 *   - Node::InportReceivingRequestFromAgent
 *   - Node::InportReceiveConfigurationFromAgent
 *   - Node::ApplyNewConfiguration
 *   - Node::BroadcastNewConfigurationToStas
 *   - Node::InportNewWlanConfigurationReceived
 */

#ifndef NODE_CONFIG_METHODS_H
#define NODE_CONFIG_METHODS_H

/**
 * Encapsulate the configuration of a node to be sent
 */
void Node :: GenerateConfiguration(){

	// Capabilities
	NodeCapabilities capabilities;
	capabilities.node_code = node_code.c_str();
	capabilities.node_id = node_id;
	capabilities.x = x;
	capabilities.y = y;
	capabilities.z = z;
	capabilities.node_type = node_type;
	capabilities.primary_channel = current_primary_channel;
	capabilities.min_channel_allowed = min_channel_allowed;
	capabilities.max_channel_allowed = max_channel_allowed;
	capabilities.num_channels_allowed = num_channels_allowed;
	capabilities.tx_power_default = tx_power_default;
	capabilities.sensitivity_default = sensitivity_default;
	capabilities.current_max_bandwidth = current_max_bandwidth;

	// Configuration
	configuration.capabilities = capabilities;

	configuration.timestamp = SimTime();
	configuration.selected_primary_channel = current_primary_channel;
	configuration.selected_pd = current_pd;
	configuration.selected_tx_power = current_tx_power;
	configuration.selected_max_bandwidth = current_max_bandwidth;
	configuration.frame_length = frame_length;
	configuration.max_num_packets_aggregated = max_num_packets_aggregated;

	// 11ax SR
	configuration.spatial_reuse_enabled = spatial_reuse_enabled;
	configuration.bss_color = bss_color;
	configuration.srg = srg;
	configuration.non_srg_obss_pd = non_srg_obss_pd;
	configuration.srg_obss_pd = srg_obss_pd;

}

/**
 * Called when some agent answers for information to the AP
 */
void Node :: InportReceivingRequestFromAgent() {

//	printf("%s Node #%d: New information request received from the Agent\n", LOG_LVL1, node_id);

	LOGS(save_node_logs, node_logger.file, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s InportReceivingRequestFromAgent()\n",
		SimTime(), node_id, node_state, LOG_F02, LOG_LVL2);

	// Generate the configuration to be sent to the agent
	GenerateConfiguration();

	// Update the performance-related metrics
	UpdatePerformanceMeasurements();

	// Answer to the agent
	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Sending information to the Agent\n",
		SimTime(), node_id, node_state, LOG_F02, LOG_LVL2);

	outportAnswerToAgent(configuration, performance_report);

	// Restart performance metrics for future requests
	RestartPerformanceMetrics(&performance_report, SimTime(), num_channels_allowed);

	LOGS(save_node_logs,node_logger.file, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

}

/**
 * Called when an agent sends a new configuration to the AP
 * @param "received_configuration" [type Configuration]: received configuration
 */
void Node :: InportReceiveConfigurationFromAgent(Configuration &received_configuration) {

	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s New configuration received from the Agent\n",
		SimTime(), node_id, node_state, LOG_F02, LOG_LVL2);

	if(!flag_apply_new_configuration) {
		new_configuration = received_configuration;
		if(save_node_logs) WriteNodeConfiguration(node_logger, header_str);
		if(save_node_logs) WriteReceivedConfiguration(node_logger, header_str, new_configuration);
		// Set flag to true in order to apply the new configuration next time the node restarts
		flag_apply_new_configuration = TRUE;
		if(node_state == STATE_SENSING) {
			// FORCE RESTART TO APPLY CHANGES
			RestartNode(FALSE);
		}
	} else {
		printf("%.15f;N%d Received a new configuration before applying the last one!\n", SimTime(), node_id);
	}

}

/**
 * Apply the new configuration received from either the Agent (in case of being an AP) or the AP (in case of being an STA)
 * @param "new_configuration" [type Configuration]: struct containing the new configuration to be applied
 */
void Node :: ApplyNewConfiguration(Configuration &new_configuration) {
	// TODO: think about recommendation levels done by agents (e.g., Critical, Recommended ...)
	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Applying the new received configuration\n",
		SimTime(), node_id, node_state, LOG_F02, LOG_LVL2);
	// Set new configuration according to received instructions
	current_primary_channel = new_configuration.selected_primary_channel;
	if (spatial_reuse_enabled){
		non_srg_obss_pd = new_configuration.selected_pd;
	} else {
		current_pd = new_configuration.selected_pd;
	}

	if(current_tx_power != new_configuration.selected_tx_power) flag_change_in_tx_power = TRUE;
	current_tx_power = new_configuration.selected_tx_power;
	current_max_bandwidth = new_configuration.selected_max_bandwidth;

	// current_max_bandwidth determines the min and max allowed channels given a primary
	GetMinAndMaxAllowedChannels(min_channel_allowed, max_channel_allowed,
			current_primary_channel, current_max_bandwidth);

	// Re-compute MCS according to the new configuration
	if (node_type == NODE_TYPE_AP) {
		for(int n = 0; n < wlan.num_stas; ++n) {
			change_modulation_flag[n] = TRUE;
		}
		// Broadcast the new configuration to the associated STAs
		BroadcastNewConfigurationToStas(new_configuration);
	}
	// Print new configuration
	if(save_node_logs) WriteNodeConfiguration(node_logger, header_str);
}

/**
 * Broadcast a new configuration to be applied by all the STAs (only executed by AP nodes)
 * @param "new_configuration" [type Configuration]: struct containing the new configuration to be broadcasted
 */
void Node :: BroadcastNewConfigurationToStas(Configuration &new_configuration) {
	// ONLY APs connected to agents
	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Broadcasting the new configuration to STAs\n",
		SimTime(), node_id, node_state, LOG_F02, LOG_LVL2);
	// Send the new configuration to the associated STAs
	outportSetNewWlanConfiguration(new_configuration);
}

/**
 * Called when a new configuration is received (only STAs)
 * @param "received_configuration" [type Configuration]: struct containing the new configuration to be applied
 */
void Node :: InportNewWlanConfigurationReceived(Configuration &received_configuration) {

	if (node_type == NODE_TYPE_STA) {

		LOGS(save_node_logs, node_logger.file, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

		LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s New configuration received from the AP\n",
			SimTime(), node_id, node_state, LOG_F02, LOG_LVL2);

		// Set new configuration
		new_configuration = received_configuration;
		if (save_node_logs) WriteReceivedConfiguration(node_logger, header_str, new_configuration);
		// Set flag to true in order to apply the new configuration next time the node restarts
		flag_apply_new_configuration = TRUE;
		LOGS(save_node_logs,node_logger.file, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

//		if(node_state == STATE_SENSING) RestartNode(FALSE);
		// Force restart
		if(node_state == STATE_SENSING || node_state == STATE_NAV) {
            RestartNode(FALSE);
		}

	} else {
		printf("ERROR: the broadcast of a new configuration cannot be received at APs\n");
	}

}

#endif /* NODE_CONFIG_METHODS_H */
