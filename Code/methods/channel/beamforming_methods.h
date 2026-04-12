/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 *
 * -----------------------------------------------------------------
 * beamforming_methods.h
 *
 * Simplified ULA (Uniform Linear Array) beamforming model for Komondor.
 *
 * Model summary:
 *   - Horizontal-plane ULA; phase shift φ_n = 2π·d·n·sin(az)
 *   - Projection-based null steerer (LCMV / Capon-style):
 *       w = a_des − Σ_k [ (a_k'·a_des)/(a_k'·a_k) · a_k ]
 *       normalized so that w'·a_des = 1 (unity gain on main beam)
 *   - Beam gain at receiver = |AF(az_rx)|² = |w'·a(az_rx)|²
 *   - All complex arithmetic uses parallel real/imag double arrays
 *     (C++98 compatible, no std::complex required)
 *
 * Usage:
 *   Called from UpdatePowerSensedPerNode() via ComputeRxBeamGain().
 *   Beam parameters are embedded per-TXOP in TxInfo by GenerateNotification().
 * -----------------------------------------------------------------
 */

#ifndef BEAMFORMING_METHODS_H
#define BEAMFORMING_METHODS_H

#include <math.h>
#include "../../structures/notification.h"
#include "../../list_of_macros.h"

/* Maximum ULA elements supported by stack arrays in this file.
 * Increase if needed; 32 covers all practical 802.11ax cases. */
#define BF_MAX_N_ELEMENTS  32

/* ---------------------------------------------------------------
 * ComputeDirectionAzimuth
 *   Returns the azimuth angle [rad] of the direction (dx, dy, dz).
 *   Elevation is projected out: the ULA phase only uses sin(az)
 *   where az = atan2(dy, dx) in the horizontal plane.
 * --------------------------------------------------------------- */
static double ComputeDirectionAzimuth(double dx, double dy, double dz) {
	(void)dz;  /* elevation not used in horizontal ULA model */
	return atan2(dy, dx);
}

/* ---------------------------------------------------------------
 * ComputeULASteeringVector
 *   Fills a_real[N] and a_imag[N] with the steering vector for
 *   a ULA of N elements with spacing d [wavelengths] at azimuth az [rad].
 *     a_n = exp(j · 2π · d · n · sin(az))  for n = 0..N-1
 * --------------------------------------------------------------- */
static void ComputeULASteeringVector(double az_rad, int N, double d,
		double *a_real, double *a_imag) {
	double sin_az = sin(az_rad);
	for (int n = 0; n < N; ++n) {
		double phase = 2.0 * M_PI * d * n * sin_az;
		a_real[n] = cos(phase);
		a_imag[n] = sin(phase);
	}
}

/* ---------------------------------------------------------------
 * ComplexDot
 *   Returns  result = Σ_n  conj(a_n) · b_n  (complex dot product).
 *   Outputs via res_real, res_imag pointers.
 * --------------------------------------------------------------- */
static void ComplexDot(const double *a_real, const double *a_imag,
		const double *b_real, const double *b_imag, int N,
		double *res_real, double *res_imag) {
	double rr = 0.0, ri = 0.0;
	for (int n = 0; n < N; ++n) {
		/* conj(a_n) · b_n = (a_r - j·a_i)(b_r + j·b_i)
		 *                  = a_r·b_r + a_i·b_i  +  j·(a_r·b_i - a_i·b_r) */
		rr += a_real[n] * b_real[n] + a_imag[n] * b_imag[n];
		ri += a_real[n] * b_imag[n] - a_imag[n] * b_real[n];
	}
	*res_real = rr;
	*res_imag = ri;
}

/* ---------------------------------------------------------------
 * ComputeBeamWeights
 *   Computes the weight vector w[] for a ULA beamformer that:
 *     - steers the main lobe toward az_main_rad
 *     - places nulls at null_az_rad[0..num_nulls-1]
 *
 *   Algorithm (sequential projection, one null at a time):
 *     1. w = a_des  (start from main-beam steering vector)
 *     2. For each null k:
 *          a_k = steering vector at null_az_rad[k]
 *          proj_scalar = (a_k' · w) / (a_k' · a_k)     [complex scalar]
 *          w = w − proj_scalar · a_k                    [project out null direction]
 *     3. Normalize: scale = w' · a_des;  w = w / scale  (unity gain toward main beam)
 *
 *   If N == 1, sets w = {1, 0} (omnidirectional fallback).
 *   Outputs w_real[N] and w_imag[N].
 * --------------------------------------------------------------- */
static void ComputeBeamWeights(double az_main_rad,
		const double *null_az_rad, int num_nulls,
		int N, double d,
		double *w_real, double *w_imag) {
	int n, k;

	/* Omnidirectional fallback */
	if (N <= 1) {
		w_real[0] = 1.0;
		w_imag[0] = 0.0;
		return;
	}

	/* Step 1: w = a_des */
	ComputeULASteeringVector(az_main_rad, N, d, w_real, w_imag);

	/* Step 2: project out each null direction */
	double ak_real[BF_MAX_N_ELEMENTS], ak_imag[BF_MAX_N_ELEMENTS];
	for (k = 0; k < num_nulls; ++k) {
		ComputeULASteeringVector(null_az_rad[k], N, d, ak_real, ak_imag);

		/* proj_scalar = (ak' · w) / (ak' · ak) */
		double num_r, num_i, den_r, den_i;
		ComplexDot(ak_real, ak_imag, w_real, w_imag, N, &num_r, &num_i);
		ComplexDot(ak_real, ak_imag, ak_real, ak_imag, N, &den_r, &den_i);
		/* den is real and positive (ak'·ak = N for unit-magnitude elements) */
		double den = den_r;
		if (den < 1e-15) den = 1e-15;  /* guard against zero */

		double ps_r = num_r / den;
		double ps_i = num_i / den;

		/* w -= proj_scalar * ak */
		for (n = 0; n < N; ++n) {
			/* (ps_r + j·ps_i)(ak_r + j·ak_i) = ps_r·ak_r - ps_i·ak_i + j·(ps_r·ak_i + ps_i·ak_r) */
			w_real[n] -= ps_r * ak_real[n] - ps_i * ak_imag[n];
			w_imag[n] -= ps_r * ak_imag[n] + ps_i * ak_real[n];
		}
	}

	/* Step 3: normalize so w'·a_des = 1 */
	double ad_real[BF_MAX_N_ELEMENTS], ad_imag[BF_MAX_N_ELEMENTS];
	ComputeULASteeringVector(az_main_rad, N, d, ad_real, ad_imag);
	double sc_r, sc_i;
	ComplexDot(w_real, w_imag, ad_real, ad_imag, N, &sc_r, &sc_i);
	/* divide w by the complex scalar (sc_r + j·sc_i) */
	double sc_mag2 = sc_r * sc_r + sc_i * sc_i;
	if (sc_mag2 < 1e-30) sc_mag2 = 1e-30;  /* guard */
	for (n = 0; n < N; ++n) {
		/* w_n / (sc_r + j·sc_i) = w_n · conj(sc) / |sc|² */
		double wr = w_real[n], wi = w_imag[n];
		w_real[n] = (wr * sc_r + wi * sc_i) / sc_mag2;
		w_imag[n] = (wi * sc_r - wr * sc_i) / sc_mag2;
	}
}

/* ---------------------------------------------------------------
 * EvaluateBeamGain
 *   Returns |AF(az_rad)|² = |w'·a(az_rad)|² (linear power gain).
 *   Unity gain (1.0) in the main beam direction after normalization.
 * --------------------------------------------------------------- */
static double EvaluateBeamGain(const double *w_real, const double *w_imag,
		double az_rad, int N, double d) {
	double a_real[BF_MAX_N_ELEMENTS], a_imag[BF_MAX_N_ELEMENTS];
	ComputeULASteeringVector(az_rad, N, d, a_real, a_imag);
	double af_r, af_i;
	ComplexDot(w_real, w_imag, a_real, a_imag, N, &af_r, &af_i);
	return af_r * af_r + af_i * af_i;
}

/* ---------------------------------------------------------------
 * ComputeRxBeamGain
 *   Top-level function called by UpdatePowerSensedPerNode().
 *
 *   Given the transmitter's beam parameters (embedded in tx_info)
 *   and the receiver's position (rx_x, rx_y, rx_z), returns the
 *   linear power gain factor to apply to the path-loss-based
 *   received power.
 *
 *   Returns 1.0 if beamforming is disabled or N_elements <= 1.
 * --------------------------------------------------------------- */
static double ComputeRxBeamGain(const TxInfo &tx_info,
		double rx_x, double rx_y, double rx_z) {
	if (!tx_info.beamforming_active || tx_info.beam_N_elements <= 1)
		return 1.0;

	double dx = rx_x - tx_info.x;
	double dy = rx_y - tx_info.y;
	double dz = rx_z - tx_info.z;

	/* Self-node: return 1 (distance = 0 has no physical meaning here) */
	if (dx == 0.0 && dy == 0.0 && dz == 0.0) return 1.0;

	double az_rx = ComputeDirectionAzimuth(dx, dy, dz);

	int N = tx_info.beam_N_elements;
	if (N > BF_MAX_N_ELEMENTS) N = BF_MAX_N_ELEMENTS;

	double w_real[BF_MAX_N_ELEMENTS], w_imag[BF_MAX_N_ELEMENTS];
	ComputeBeamWeights(tx_info.beam_az_main_rad,
		tx_info.beam_null_az_rad, tx_info.beam_num_nulls,
		N, tx_info.beam_d_spacing,
		w_real, w_imag);

	return EvaluateBeamGain(w_real, w_imag, az_rx, N, tx_info.beam_d_spacing);
}

#endif /* BEAMFORMING_METHODS_H */
