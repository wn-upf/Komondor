/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * channel_access_methods.h: Channel access policy abstraction.
 *
 * Defines ChannelAccessPolicy — a strategy struct whose single function pointer
 * encapsulates the CCA check + channel-bonding selection that determines whether
 * a node may transmit when its backoff expires.
 *
 * The default policy is CSMA_CA: it calls GetChannelOccupancyByCCA() followed
 * by GetTxChannels() and is suitable for all standard IEEE 802.11ax scenarios.
 * Future access mechanisms (OFDMA trigger-based, token-ring, scheduled) can
 * register an alternative function here without touching the rest of node.h.
 *
 * Usage in node.h:
 *   1. Add member:  ChannelAccessPolicy channel_access_policy;
 *   2. Initialize:  channel_access_policy.checkAndSelectChannels =
 *                       CSMA_CA_CheckAndSelectChannels;
 *   3. In EndBackoff, replace the dual GetChannelOccupancyByCCA/GetTxChannels
 *      calls with:
 *        double effective_pd = (sr_state.spatial_reuse_enabled && sr_state.txop_sr_identified)
 *            ? sr_state.current_obss_pd_threshold : current_pd;
 *        channel_access_policy.checkAndSelectChannels(
 *            node_params.current_primary_channel, node_params.pifs_activated,
 *            channels_free, node_params.min_channel_allowed,
 *            node_params.max_channel_allowed, &channel_power, effective_pd,
 *            timestamp_channel_becomes_free, SimTime(), PIFS,
 *            node_params.current_dcb_policy, NUM_CHANNELS_KOMONDOR,
 *            channel_aggregation_cca_model, channels_for_tx);
 *
 * Dependencies: power_channel_methods.h (GetChannelOccupancyByCCA),
 *               channel_bonding_methods.h (GetTxChannels).
 *   Both must be included before this file.
 */

#ifndef CHANNEL_ACCESS_METHODS_H
#define CHANNEL_ACCESS_METHODS_H

/**
 * ChannelAccessPolicy — strategy struct for the CCA + channel-selection step.
 *
 * checkAndSelectChannels():
 *   Determines which channels are free (via CCA with threshold pd), then selects
 *   the set of channels to use for the upcoming transmission according to the
 *   Dynamic Channel Bonding policy.
 *
 *   Outputs:
 *     channels_free[]   — per-channel boolean: TRUE if idle (updated in-place)
 *     channels_for_tx[] — per-channel boolean: TRUE if selected for TX;
 *                         channels_for_tx[0] == TX_NOT_POSSIBLE means TX blocked
 *
 *   Parameters match the signatures of GetChannelOccupancyByCCA +
 *   GetTxChannels so that the default CSMA/CA implementation is a trivial
 *   two-call wrapper.
 */
struct ChannelAccessPolicy {
	void (*checkAndSelectChannels)(
		int    primary_channel,
		int    pifs_activated,
		int   *channels_free,
		int    min_channel_allowed,
		int    max_channel_allowed,
		double **channel_power,
		double pd,
		double *timestamp_channel_becomes_free,
		double sim_time,
		double pifs,
		int    dcb_policy,
		int    num_channels_komondor,
		int    channel_aggregation_cca_model,
		int   *channels_for_tx);
};

/**
 * CSMA_CA_CheckAndSelectChannels — default policy implementation.
 *
 * Performs standard 802.11 CCA: marks channels whose received power is below
 * pd as free, then uses the configured DCB policy to pick the widest available
 * contiguous block aligned to the primary channel.
 */
static void CSMA_CA_CheckAndSelectChannels(
		int    primary_channel,
		int    pifs_activated,
		int   *channels_free,
		int    min_channel_allowed,
		int    max_channel_allowed,
		double **channel_power,
		double pd,
		double *timestamp_channel_becomes_free,
		double sim_time,
		double pifs,
		int    dcb_policy,
		int    num_channels_komondor,
		int    channel_aggregation_cca_model,
		int   *channels_for_tx)
{
	GetChannelOccupancyByCCA(
		primary_channel, pifs_activated, channels_free,
		min_channel_allowed, max_channel_allowed,
		channel_power, pd,
		timestamp_channel_becomes_free, sim_time, pifs);

	GetTxChannels(
		channels_for_tx, dcb_policy, channels_free,
		min_channel_allowed, max_channel_allowed, primary_channel,
		num_channels_komondor, channel_power, channel_aggregation_cca_model);
}

#endif /* CHANNEL_ACCESS_METHODS_H */
