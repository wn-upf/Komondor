/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * mcs_methods.h: PHY-layer MCS selection helpers.
 *
 * Free functions:
 *   - SelectMCSResponse  — maps received power (pW) to MCS for 1/2/4/8 channels
 *   - ComputeEbToNoise   — computes Eb/N0 from SINR (currently unused)
 *
 * Dependencies: list_of_macros.h (MODULATION_* constants, ConvertPower,
 *               NUM_OPTIONS_CHANNEL_LENGTH, PW_TO_DBM).
 */

#ifndef MCS_METHODS_H
#define MCS_METHODS_H

/**
 * Select the proper MCS per each number of channels based on the power received from transmitter
 * @param "mcs_response" [type int*]: array to fill with MCS for 1/2/4/8 channels
 * @param "power_rx_interest" [type double]: received power of interest (pW)
 */
void SelectMCSResponse(int *mcs_response, double power_rx_interest) {

	// RSSI thresholds (dBm, baseline for 20 MHz / ch_num_ix=0).
	// For wider bandwidths add ch_num_ix*3 dB to each threshold.
	static const int N_THRESHOLDS = 13;
	static const double MCS_THRESHOLDS[N_THRESHOLDS] = {
		-79, -77, -74, -70, -66, -65, -64, -59, -57, -54, -52, -48, -46
	};
	static const int MCS_VALUES[N_THRESHOLDS + 1] = {
		MODULATION_BPSK_1_2,    // <  -79
		MODULATION_QPSK_1_2,    // >= -79, < -77
		MODULATION_QPSK_3_4,    // >= -77, < -74
		MODULATION_16QAM_1_2,   // >= -74, < -70
		MODULATION_16QAM_3_4,   // >= -70, < -66
		MODULATION_64QAM_2_3,   // >= -66, < -65
		MODULATION_64QAM_3_4,   // >= -65, < -64
		MODULATION_64QAM_5_6,   // >= -64, < -59
		MODULATION_256QAM_3_4,  // >= -59, < -57
		MODULATION_256QAM_5_6,  // >= -57, < -54
		MODULATION_1024QAM_3_4, // >= -54, < -52
		MODULATION_1024QAM_5_6, // >= -52, < -48
		MODULATION_4096QAM_3_4, // >= -48, < -46
		MODULATION_4096QAM_5_6  // >= -46
	};

	double pw_rx_dbm = ConvertPower(PW_TO_DBM, power_rx_interest);

	for (int ch_num_ix = 0; ch_num_ix < NUM_OPTIONS_CHANNEL_LENGTH; ++ch_num_ix) {	// For 20, 40, 80, 160, 320 MHz channels
		double offset = ch_num_ix * 3.0;
		int ix;
		for (ix = 0; ix < N_THRESHOLDS; ++ix) {
			if (pw_rx_dbm < MCS_THRESHOLDS[ix] + offset) break;
		}
		mcs_response[ch_num_ix] = MCS_VALUES[ix];
	}
}

/**
 * Compute Eb/N0 (energy-per-bit to noise spectral density ratio).
 * @param "sinr" [type double]: SINR received (linear)
 * @param "bit_rate" [type double]: bit rate (bps)
 * @param "bandwidth" [type int]: channel bandwidth (Hz)
 * @param "modulation_type" [type int]: number of modulation symbols
 * @return Eb/N0 in linear
 * NOTE: currently unused.
 */
double ComputeEbToNoise(double sinr, double bit_rate, int bandwidth, int modulation_type){
	double Es_to_N0 (sinr * (bit_rate / bandwidth));
	double Eb_to_N0 (Es_to_N0 * log2(modulation_type));
	return Eb_to_N0;
}

#endif /* MCS_METHODS_H */
