/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 *
 * -----------------------------------------------------------------
 *
 * Author  : Sergio Barrachina-Muñoz and Francesc Wilhelmi
 * Created : 2016-12-05
 * Updated : $Date: 2017/03/20 10:32:36 $
 *           $Revision: 1.0 $
 *
 * -----------------------------------------------------------------
 */

/**
 * channel_bonding_methods.h: channel bonding and channel selection logic.
 *
 * Contains:
 *  - GetTxChannelsByChannelBondingCCA11ax(): CCA with bandwidth-dependent thresholds (802.11ax)
 *  - GetTxChannelsByChannelBondingCCASame(): CCA with a uniform threshold across bandwidths
 *  - GetTxChannels(): dispatcher between the two CCA models
 *  - GetMinAndMaxAllowedChannels(): determines the channel range given primary and max bandwidth
 *
 * Extracted from power_channel_methods.h during refactoring for maintainability.
 * power_channel_methods.h includes this file at the end, so existing callers are unaffected.
 */

#include <math.h>
#include "../../list_of_macros.h"
#include "../utils/auxiliary_methods.h"

#ifndef _CHANNEL_BONDING_METHODS_
#define _CHANNEL_BONDING_METHODS_

// Forward declaration: defined in power_channel_methods.h
double ConvertPower(int conversion_type, double power_magnitude_in);

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

/**
 * Returns the secondary 20 MHz channel paired with the given primary channel.
 * Primary at an even index uses primary+1 as secondary; odd index uses primary-1.
 */
static inline int GetSecondary20MHzChannel(int primary_channel) {
	return (primary_channel % 2 == 0) ? primary_channel + 1 : primary_channel - 1;
}

/**
 * Sets a contiguous channel range in channels_for_tx[] for transmission.
 * The range is the log2-aligned block of size num_channels that contains primary_channel.
 * @param "channels_for_tx" [type int*]: boolean TX channel array (updated in place)
 * @param "primary_channel"  [type int]: primary channel index
 * @param "num_channels"     [type int]: number of channels (must be 1, 2, 4, or 8)
 */
static void SetChannelRangeForTx(int *channels_for_tx, int primary_channel, int num_channels) {
	switch (num_channels) {
		case 1: { // 20 MHz
			channels_for_tx[primary_channel] = TRUE;
			break;
		}
		case 2: { // 40 MHz
			int base = (primary_channel / 2) * 2;
			channels_for_tx[base]     = TRUE;
			channels_for_tx[base + 1] = TRUE;
			break;
		}
		case 4: { // 80 MHz
			int base = (primary_channel <= 3) ? 0 : 4;
			for (int c = base; c < base + 4; ++c) channels_for_tx[c] = TRUE;
			break;
		}
		case 8: { // 160 MHz
            int base = (primary_channel <= 7) ? 0 : 8; // Assuming system size up to 16+
            for (int c = base; c < base + 8; ++c) channels_for_tx[c] = TRUE;
            break;
        }
        case 16: { // 320 MHz (> Wi-Fi 7)
            int base = (primary_channel <= 15) ? 0 : 16;
            for (int c = base; c < base + 16; ++c) channels_for_tx[c] = TRUE;
            break;
        }
	}
}

/**
 * Identify the channels to TX using the 802.11ax CCA model (different CCA threshold per bandwidth).
 * Starting from the maximum allowed bandwidth, the function reduces bandwidth until all required
 * channels pass their respective CCA thresholds.
 *
 * NOTE: case 8 intentionally falls through into case 4 to share the 80 MHz CCA logic.
 *
 * @param "channels_for_tx"       [type int*]:    TX channel boolean array (updated in place)
 * @param "channel_bonding_model" [type int]:     channel bonding model (only CB_ONLY_PRIMARY and
 *                                                CB_ALWAYS_MAX_LOG2 are handled here)
 * @param "min_channel_allowed"   [type int]:     leftmost allowed channel
 * @param "max_channel_allowed"   [type int]:     rightmost allowed channel
 * @param "primary_channel"       [type int]:     primary channel
 * @param "channel_power"         [type double**]: power sensed per channel
 */
void GetTxChannelsByChannelBondingCCA11ax(int *channels_for_tx, int channel_bonding_model,
		int min_channel_allowed, int max_channel_allowed, int primary_channel,
		double **channel_power) {

	int num_channels_allowed = max_channel_allowed - min_channel_allowed + 1;

	// Reset channels for transmitting
	for (int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c) channels_for_tx[c] = FALSE;

	switch (channel_bonding_model) {

		case CB_ONLY_PRIMARY: {
			if ((*channel_power)[primary_channel] < ConvertPower(DBM_TO_PW, -82))
				channels_for_tx[primary_channel] = TRUE;
			break;
		}

		case CB_ALWAYS_MAX_LOG2: {

			switch (num_channels_allowed) {

				case 16: { // 320 MHz (802.11be / Wi-Fi 7)
					int num_ch_tx_possible = 16;

					// Determine the Primary 160 block vs Secondary 160 block
					// Assuming 320 MHz spans indices 0-15
					if (primary_channel <= 7) {
						// Primary is in the lower 160 MHz (0-7). Check lower as Primary, upper as Secondary.
						for (int c = 0; c <= 7; ++c)
							if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_160MHZ))   num_ch_tx_possible = 8;
						for (int c = 8; c <= 15; ++c)
							if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_160MHZ)) num_ch_tx_possible = 8;
					} else {
						// Primary is in the upper 160 MHz (8-15).
						for (int c = 0; c <= 7; ++c)
							if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_160MHZ)) num_ch_tx_possible = 8;
						for (int c = 8; c <= 15; ++c)
							if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_160MHZ))   num_ch_tx_possible = 8;
					}

					// If 320 MHz failed, num_ch_tx_possible is now 8. 
					// We now evaluate if 160 MHz is possible within the primary's 160MHz block.
					if (num_ch_tx_possible == 8) {
						// Identify the 160 MHz boundaries for the primary channel
						int base160 = (primary_channel <= 7) ? 0 : 8;
						int mid160 = base160 + 3; // split point between the two 80MHz blocks
						
						if (primary_channel <= mid160) {
							for (int c = base160; c <= mid160; ++c)
								if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_80MHZ))   num_ch_tx_possible = 4;
							for (int c = mid160 + 1; c <= base160 + 7; ++c)
								if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_80MHZ)) num_ch_tx_possible = 4;
						} else {
							for (int c = base160; c <= mid160; ++c)
								if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_80MHZ)) num_ch_tx_possible = 4;
							for (int c = mid160 + 1; c <= base160 + 7; ++c)
								if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_80MHZ))   num_ch_tx_possible = 4;
						}
					}

					if (num_ch_tx_possible == 4) {
						// Check 80 MHz: primary 40 MHz block vs CCA_PRIMARY_40MHZ, other vs CCA_SECONDARY_40MHZ
						if (primary_channel <= 3) {
							if (primary_channel <= 1) {
								for (int c = 0; c <= 1; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_40MHZ))   num_ch_tx_possible = 2;
								for (int c = 2; c <= 3; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
							} else {
								for (int c = 0; c <= 1; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
								for (int c = 2; c <= 3; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_40MHZ))   num_ch_tx_possible = 2;
							}
						} else {
							if (primary_channel <= 5) {
								for (int c = 4; c <= 5; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_40MHZ))   num_ch_tx_possible = 2;
								for (int c = 6; c <= 7; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
							} else {
								for (int c = 4; c <= 5; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
								for (int c = 6; c <= 7; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_40MHZ))   num_ch_tx_possible = 2;
							}
						}
					}

					if (num_ch_tx_possible == 2) {
						// Check 40 MHz: primary 20 MHz vs CCA_PRIMARY_20MHZ, secondary vs CCA_SECONDARY_20MHZ
						int secondary = GetSecondary20MHzChannel(primary_channel);
						if ((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_20MHZ))   num_ch_tx_possible = 1;
						if ((*channel_power)[secondary]        > ConvertPower(DBM_TO_PW, CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
					}

					SetChannelRangeForTx(channels_for_tx, primary_channel, num_ch_tx_possible);
					break;
				}

				// 160 MHz — falls through into the 80 MHz case after handling the extra 80 MHz check
				case 8: {

					int num_ch_tx_possible = 8;

					// Check if 160 MHz is feasible: primary 80 MHz block must pass CCA_PRIMARY_80MHZ,
					// the other 80 MHz block must pass CCA_SECONDARY_80MHZ.
					if (primary_channel <= 3) {
						for (int c = 0; c <= 3; ++c)
							if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_80MHZ))   num_ch_tx_possible = 4;
						for (int c = 4; c <= 7; ++c)
							if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_80MHZ)) num_ch_tx_possible = 4;
					} else {
						for (int c = 0; c <= 3; ++c)
							if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_80MHZ)) num_ch_tx_possible = 4;
						for (int c = 4; c <= 7; ++c)
							if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_80MHZ))   num_ch_tx_possible = 4;
					}

					if (num_ch_tx_possible == 4) {
						// Check 80 MHz: primary 40 MHz block vs CCA_PRIMARY_40MHZ, other vs CCA_SECONDARY_40MHZ
						if (primary_channel <= 3) {
							if (primary_channel <= 1) {
								for (int c = 0; c <= 1; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_40MHZ))   num_ch_tx_possible = 2;
								for (int c = 2; c <= 3; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
							} else {
								for (int c = 0; c <= 1; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
								for (int c = 2; c <= 3; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_40MHZ))   num_ch_tx_possible = 2;
							}
						} else {
							if (primary_channel <= 5) {
								for (int c = 4; c <= 5; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_40MHZ))   num_ch_tx_possible = 2;
								for (int c = 6; c <= 7; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
							} else {
								for (int c = 4; c <= 5; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
								for (int c = 6; c <= 7; ++c)
									if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_40MHZ))   num_ch_tx_possible = 2;
							}
						}
					}

					if (num_ch_tx_possible == 2) {
						// Check 40 MHz: primary 20 MHz vs CCA_PRIMARY_20MHZ, secondary vs CCA_SECONDARY_20MHZ
						int secondary = GetSecondary20MHzChannel(primary_channel);
						if ((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_20MHZ))   num_ch_tx_possible = 1;
						if ((*channel_power)[secondary]        > ConvertPower(DBM_TO_PW, CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
					}

					SetChannelRangeForTx(channels_for_tx, primary_channel, num_ch_tx_possible);
					break;
					// NOTE: original code had no break here, causing fallthrough into case 4 which
					// re-evaluated the 80 MHz CCA check and could override the assignments above.
					// The break above preserves the intended semantics (160 MHz block is self-contained).
				}

				// 80 MHz
				case 4: {

					int num_ch_tx_possible = 4;

					// Check 80 MHz feasibility
					if (primary_channel <= 3) {
						if (primary_channel <= 1) {
							for (int c = 0; c <= 1; ++c)
								if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_40MHZ))   num_ch_tx_possible = 2;
							for (int c = 2; c <= 3; ++c)
								if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
						} else {
							for (int c = 0; c <= 1; ++c)
								if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
							for (int c = 2; c <= 3; ++c)
								if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_40MHZ))   num_ch_tx_possible = 2;
						}
					} else {
						if (primary_channel <= 5) {
							for (int c = 4; c <= 5; ++c)
								if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_40MHZ))   num_ch_tx_possible = 2;
							for (int c = 6; c <= 7; ++c)
								if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
						} else {
							for (int c = 4; c <= 5; ++c)
								if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
							for (int c = 6; c <= 7; ++c)
								if ((*channel_power)[c] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_40MHZ))   num_ch_tx_possible = 2;
						}
					}

					if (num_ch_tx_possible == 2) {
						int secondary = GetSecondary20MHzChannel(primary_channel);
						if ((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_20MHZ))   num_ch_tx_possible = 1;
						if ((*channel_power)[secondary]        > ConvertPower(DBM_TO_PW, CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
					}

					SetChannelRangeForTx(channels_for_tx, primary_channel, num_ch_tx_possible);
					break;
				}

				// 40 MHz
				case 2: {

					int num_ch_tx_possible = 2;

					int secondary = GetSecondary20MHzChannel(primary_channel);
					if ((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW, CCA_PRIMARY_20MHZ))   num_ch_tx_possible = 1;
					if ((*channel_power)[secondary]        > ConvertPower(DBM_TO_PW, CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;

					SetChannelRangeForTx(channels_for_tx, primary_channel, num_ch_tx_possible);
					break;
				}

				// 20 MHz
				case 1: {
					channels_for_tx[primary_channel] = TRUE;
					break;
				}
			}
			break;
		}
	}
}

/**
 * Identify the channels to TX using a uniform CCA threshold across all bandwidths.
 * @param "channels_for_tx"       [type int*]:    TX channel boolean array (updated in place)
 * @param "channel_bonding_model" [type int]:     channel bonding model
 * @param "channels_free"         [type int*]:    array indicating whether a channel is free (1) or not (0)
 * @param "min_channel_allowed"   [type int]:     leftmost allowed channel
 * @param "max_channel_allowed"   [type int]:     rightmost allowed channel
 * @param "primary_channel"       [type int]:     primary channel
 * @param "num_channels_system"   [type int]:     total number of channels in the system
 */
void GetTxChannelsByChannelBondingCCASame(int *channels_for_tx, int channel_bonding_model,
		int *channels_free, int min_channel_allowed, int max_channel_allowed,
		int primary_channel, int num_channels_system) {

	// Reset channels for transmitting
	for (int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c) channels_for_tx[c] = FALSE;

	// Find leftmost and rightmost free channels within the allowed range
	int left_free_ch (0);
	int left_free_ch_is_set (0);
	int right_free_ch (0);

	for (int c = min_channel_allowed; c <= max_channel_allowed; ++c) {
		if (channels_free[c]) {
			if (!left_free_ch_is_set) {
				left_free_ch = c;
				left_free_ch_is_set = TRUE;
			}
			if (right_free_ch < c) right_free_ch = c;
		}
	}

	int num_available_ch (max_channel_allowed - min_channel_allowed + 1);
	int log2_modulus;
	int left_tx_ch;
	int right_tx_ch;

	// Identify which log2-aligned channel ranges are feasible
	int all_channels_free_in_range (TRUE);
	int possible_channel_ranges_ixs[4] = {FALSE, FALSE, FALSE, FALSE};

	// Check 20 MHz (primary only)
	if (channels_free[primary_channel]) possible_channel_ranges_ixs[0] = TRUE;

	// Check 40 MHz (primary + 1 secondary)
	if (NUM_CHANNELS_KOMONDOR > 1) {
		int secondary = GetSecondary20MHzChannel(primary_channel);
		if (channels_free[secondary]) possible_channel_ranges_ixs[1] = TRUE;
	}

	// Check 80 MHz (primary + 3 secondaries)
	if (NUM_CHANNELS_KOMONDOR > 3) {
		all_channels_free_in_range = TRUE;
		int base80 = (primary_channel > 3) ? 4 : 0;
		for (int c = 0; c < 4; ++c)
			if (!channels_free[base80 + c]) all_channels_free_in_range = FALSE;
		if (all_channels_free_in_range) possible_channel_ranges_ixs[2] = TRUE;
	}

	// Check 160 MHz (full system range)
	if (NUM_CHANNELS_KOMONDOR > 7) {
		all_channels_free_in_range = TRUE;
		for (int c = 0; c < 8; ++c)
			if (!channels_free[c]) all_channels_free_in_range = FALSE;
		if (all_channels_free_in_range) possible_channel_ranges_ixs[3] = TRUE;
	}

	if (!left_free_ch_is_set) {
		channels_for_tx[0] = TX_NOT_POSSIBLE;
		return;
	}

	switch (channel_bonding_model) {

		case CB_ONLY_PRIMARY: {
			if (primary_channel >= left_free_ch && primary_channel <= right_free_ch)
				channels_for_tx[primary_channel] = TRUE;
			break;
		}

		// SCB: transmit only if ALL allowed channels are free; otherwise backoff
		case CB_SCB: {
			int tx_possible = TRUE;
			for (int c = min_channel_allowed; c <= max_channel_allowed; ++c)
				if (!channels_free[c]) tx_possible = FALSE;
			if (tx_possible) {
				for (int c = min_channel_allowed; c <= max_channel_allowed; ++c)
					channels_for_tx[c] = TRUE;
			} else {
				channels_for_tx[0] = TX_NOT_POSSIBLE;
			}
			break;
		}

		// SCB log2: transmit if all channels in the log2 mapping are free; otherwise backoff
		case CB_SCB_LOG2: {
			while (1) {
				// Check that the number of channels is a power of 2 (equivalent to "fmod(log10(num_available_ch) / log10(2), 1) == 0")
				if ((num_available_ch > 0) && ((num_available_ch & (num_available_ch - 1)) == 0)) {
					log2_modulus = primary_channel % num_available_ch;
					left_tx_ch  = primary_channel - log2_modulus;
					right_tx_ch = primary_channel + num_available_ch - log2_modulus - 1;
					if (left_tx_ch >= min_channel_allowed && right_tx_ch <= max_channel_allowed)
						break;
					else
						--num_available_ch;
				} else {
					--num_available_ch;
				}
			}
			int tx_possible = TRUE;
			for (int c = left_tx_ch; c <= right_tx_ch; ++c)
				if (!channels_free[c]) tx_possible = FALSE;
			if (tx_possible) {
				for (int c = left_tx_ch; c <= right_tx_ch; ++c) channels_for_tx[c] = TRUE;
			} else {
				channels_for_tx[0] = TX_NOT_POSSIBLE;
			}
			break;
		}

		// Always-max (DCB): TX in all free channels contiguous to the primary
		case CB_ALWAYS_MAX: {
			for (int c = left_free_ch; c <= right_free_ch; ++c) channels_for_tx[c] = TRUE;
			break;
		}

		// Always-max log2: TX in the largest log2-aligned range that is fully free
		case CB_ALWAYS_MAX_LOG2: {
			int ch_range_ix = GetNumberOfSpecificElementInArray(1, possible_channel_ranges_ixs, 4);
			switch (ch_range_ix) {
				case 1: {
					channels_for_tx[primary_channel] = TRUE;
					break;
				}
				case 2: {
					channels_for_tx[primary_channel] = TRUE;
					channels_for_tx[GetSecondary20MHzChannel(primary_channel)] = TRUE;
					break;
				}
				case 3: {
					int base = (primary_channel > 3) ? 4 : 0;
					for (int c = base; c < base + 4; ++c) channels_for_tx[c] = TRUE;
					break;
				}
				case 4: {
					for (int c = 0; c < 8; ++c) channels_for_tx[c] = TRUE;
					break;
				}
			}
			break;
		}

		// Log2 probabilistic uniform: pick with equal probability any available channel range
		case CB_PROB_UNIFORM_LOG2: {
			int ch_range_ix = GetNumberOfSpecificElementInArray(1, possible_channel_ranges_ixs, 4);
			int random_value = 1 + rand() % ch_range_ix;

			channels_for_tx[primary_channel] = TRUE;  // primary is always included

			if (random_value >= 2) {
				channels_for_tx[GetSecondary20MHzChannel(primary_channel)] = TRUE;
			}
			if (random_value >= 3) {
				int base = (primary_channel > 3) ? 4 : 0;
				for (int c = base; c < base + 4; ++c) channels_for_tx[c] = TRUE;
			}
			if (random_value == 4) {
				for (int c = 0; c < 8; ++c) channels_for_tx[c] = TRUE;
			}
			break;
		}

		default: {
			printf("channel_bonding_model %d is NOT VALID!\n", channel_bonding_model);
			exit(EXIT_FAILURE);
			break;
		}
	}
}

/**
 * Dispatcher: selects the correct CCA model and calls the corresponding channel bonding function.
 * @param "channels_for_tx"              [type int*]:    TX channel boolean array (updated in place)
 * @param "channel_bonding_model"        [type int]:     channel bonding model
 * @param "channels_free"                [type int*]:    array indicating channel free/occupied status
 * @param "min_channel_allowed"          [type int]:     leftmost allowed channel
 * @param "max_channel_allowed"          [type int]:     rightmost allowed channel
 * @param "primary_channel"              [type int]:     primary channel
 * @param "num_channels_komondor"        [type int]:     number of channels in the system
 * @param "channel_power"                [type double**]: power sensed per channel
 * @param "channel_aggregation_cca_model"[type int]:     CCA model (SAME or 11AX)
 */
void GetTxChannels(int *channels_for_tx, int channel_bonding_model, int *channels_free,
		int min_channel_allowed, int max_channel_allowed, int primary_channel,
		int num_channels_komondor, double **channel_power, int channel_aggregation_cca_model) {

	switch (channel_aggregation_cca_model) {
		case CHANNEL_AGGREGATION_CCA_SAME: {
			GetTxChannelsByChannelBondingCCASame(channels_for_tx, channel_bonding_model, channels_free,
					min_channel_allowed, max_channel_allowed, primary_channel, num_channels_komondor);
			break;
		}
		case CHANNEL_AGGREGATION_CCA_11AX: {
			GetTxChannelsByChannelBondingCCA11ax(channels_for_tx, channel_bonding_model,
					min_channel_allowed, max_channel_allowed, primary_channel, channel_power);
			break;
		}
	}
}

/**
 * Compute the minimum and maximum allowed channels given the primary channel and maximum bandwidth.
 * @param "min_ch"        [type int&]: minimum channel (output)
 * @param "max_ch"        [type int&]: maximum channel (output)
 * @param "primary_channel" [type int]: primary channel index
 * @param "max_bandwidth" [type int]:  maximum bandwidth in number of 20 MHz channels (1, 2, 4, or 8)
 */
void GetMinAndMaxAllowedChannels(int &min_ch, int &max_ch, int primary_channel, int max_bandwidth) {

	switch (max_bandwidth) {
		case 1: {   // 20 MHz
			min_ch = primary_channel;
			max_ch = primary_channel;
			break;
		}
		case 2: {   // 40 MHz
			int base = (primary_channel / 2) * 2;
			min_ch = base;
			max_ch = base + 1;
			break;
		}
		case 4: {   // 80 MHz
			min_ch = (primary_channel <= 3) ? 0 : 4;
			max_ch = min_ch + 3;
			break;
		}
		case 8: {   // 160 MHz
            min_ch = (primary_channel <= 7) ? 0 : 8;
            max_ch = min_ch + 7;
            break;
        }
        case 16: {  // 320 MHz
            min_ch = (primary_channel <= 15) ? 0 : 16;
            max_ch = min_ch + 15;
            break;
        }
	}
}

#endif  // _CHANNEL_BONDING_METHODS_
