/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007

 * Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>
 * Everyone is permitted to copy and distribute verbatim copies
 * of this license document, but changing it is not allowed.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
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
 * simulation_stats.h: this file defines simulation metrics to be outputted
 */

// ===========================================================================
// Aggregated statistics (replaces the former module-scope global variables)
// ===========================================================================

struct SimulationStats {
	int    total_data_packets_sent;
	double total_num_packets_generated;
	double total_throughput;
	int    ix_wlan_min_throughput;
	double min_throughput;
	double max_throughput;
	double proportional_fairness;
	double jains_fairness;
	double jains_fairness_aux;
	int    total_rts_lost_slotted_bo;
	int    total_rts_cts_sent;
	double total_prob_slotted_bo_collision;
	int    total_num_tx_init_not_possible;
	double total_delay;
	double max_delay;
	double min_delay;
	double total_bandiwdth_tx;   ///< typo preserved from original
	double av_expected_backoff;
	double av_expected_waiting_time;

	SimulationStats() :
		total_data_packets_sent(0),
		total_num_packets_generated(0),
		total_throughput(0),
		ix_wlan_min_throughput(99999),
		min_throughput(999999999999999999.0),
		max_throughput(0),
		proportional_fairness(0),
		jains_fairness(0),
		jains_fairness_aux(0),
		total_rts_lost_slotted_bo(0),
		total_rts_cts_sent(0),
		total_prob_slotted_bo_collision(0),
		total_num_tx_init_not_possible(0),
		total_delay(0),
		max_delay(0),
		min_delay(9999999999.0),
		total_bandiwdth_tx(0),
		av_expected_backoff(0),
		av_expected_waiting_time(0)
	{}
};