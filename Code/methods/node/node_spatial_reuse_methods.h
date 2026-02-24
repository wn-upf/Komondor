/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
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

#endif /* NODE_SPATIAL_REUSE_METHODS_H */
