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
 *     channels_free[]      — per-channel boolean: TRUE if idle (updated in-place)
 *     channels_for_tx[]    — per-channel boolean: TRUE if selected for TX;
 *                            channels_for_tx[0] == TX_NOT_POSSIBLE means TX blocked
 *     punctured_bitmap_out — bitmask of punctured sub-channels (PP); 0 for non-PP policies
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
		int   *channels_for_tx,
		int   *punctured_bitmap_out);
};

/**
 * CSMA_CA_CheckAndSelectChannels — default policy implementation.
 *
 * Performs standard 802.11 CCA: marks channels whose received power is below
 * pd as free, then uses the configured DCB policy to pick the widest available
 * contiguous block aligned to the primary channel.
 * Sets *punctured_bitmap_out = 0 (no puncturing in standard CSMA/CA).
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
		int   *channels_for_tx,
		int   *punctured_bitmap_out)
{
	if (punctured_bitmap_out) *punctured_bitmap_out = 0;

	GetChannelOccupancyByCCA(
		primary_channel, pifs_activated, channels_free,
		min_channel_allowed, max_channel_allowed,
		channel_power, pd,
		timestamp_channel_becomes_free, sim_time, pifs);

	GetTxChannels(
		channels_for_tx, dcb_policy, channels_free,
		min_channel_allowed, max_channel_allowed, primary_channel,
		num_channels_komondor, channel_power, channel_aggregation_cca_model,
		NULL);
}

/**
 * PP_CheckAndSelectChannels — preamble-puncturing policy implementation.
 *
 * Calls GetChannelOccupancyByCCA (for channels_free[] logging) then
 * GetTxChannelsByPP(), which selects the widest log2-aligned block and
 * punctures busy secondary sub-channels rather than reducing bandwidth.
 * The puncture bitmap is written to *punctured_bitmap_out.
 */
static void PP_CheckAndSelectChannels(
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
		int   *channels_for_tx,
		int   *punctured_bitmap_out)
{
	// Populate channels_free[] for logging (primary-CCA based).
	GetChannelOccupancyByCCA(
		primary_channel, pifs_activated, channels_free,
		min_channel_allowed, max_channel_allowed,
		channel_power, pd,
		timestamp_channel_becomes_free, sim_time, pifs);

	// Preamble puncturing channel selection: active channels set TRUE, punctured channels FALSE.
	int bitmap = 0;
	GetTxChannelsByPP(
		channels_for_tx, &bitmap,
		min_channel_allowed, max_channel_allowed, primary_channel,
		channel_power);

	if (punctured_bitmap_out) *punctured_bitmap_out = bitmap;
}


/**
 * DSO_CheckAndSelectChannels � DSO policy implementation.
 *
 * Performs standard primary-channel CCA (same as CSMA/CA) so backoff proceeds
 * normally.  The actual DSO secondary-subband selection is done by EndBackoff()
 * after the backoff timer expires (via GetTxChannelsByDSO).  This function
 * exists to give DSO nodes a distinct check_channel_function pointer and for
 * CCA logging consistency.
 */
static void DSO_CheckAndSelectChannels(
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
		int   *channels_for_tx,
		int   *punctured_bitmap_out)
{
	if (punctured_bitmap_out) *punctured_bitmap_out = 0;

	GetChannelOccupancyByCCA(
		primary_channel, pifs_activated, channels_free,
		min_channel_allowed, max_channel_allowed,
		channel_power, pd,
		timestamp_channel_becomes_free, sim_time, pifs);

	// Use ALWAYS_MAX_LOG2 for primary channel selection; DSO secondary chosen in EndBackoff.
	GetTxChannels(
		channels_for_tx, CB_ALWAYS_MAX_LOG2, channels_free,
		min_channel_allowed, max_channel_allowed, primary_channel,
		num_channels_komondor, channel_power, channel_aggregation_cca_model,
		NULL);
}

#endif /* CHANNEL_ACCESS_METHODS_H */
