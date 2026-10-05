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
 * traffic_generator.h: this file contains the traffic generator component
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include <iostream>
#include <stdlib.h>

#include "../list_of_macros.h"
#include "../methods/utils/auxiliary_methods.h"

// Agent component: "TypeII" represents components that are aware of the existence of the simulated time.
// TRAFFIC_LLM_TOKENS: bounded-uniform inter-arrival range, in seconds.
// Approximates per-token generation gaps of an LLM response stream.
// Frame size is NOT modeled here -- set the scenario's packet_length CSV
// column to the desired representative token size (e.g. 100-500 bytes),
// exactly as done for every other traffic model.
#define LLM_TRAFFIC_IAT_MIN_S	0.005	///> Minimum inter-arrival time: 5 ms
#define LLM_TRAFFIC_IAT_MAX_S	0.020	///> Maximum inter-arrival time: 20 ms

// TRAFFIC_LLM_TOKENS_BURSTY_REALISTIC: burst ACCENTUATION fix, grounded in
// https://spearlab.nl/theses/offered/networking-traffic-usage-of-ai-especially-llms
// which states real LLM streaming responses last 10-60+ SECONDS (not sub-
// second) at 30-100 tokens/s. TRAFFIC_LLM_TOKENS_BURSTY/_SCALED_N/_CONC all
// divided burst DURATION by N along with everything else, compressing a
// burst to well under 1s -- the opposite of a real session. Here, burst
// DURATION is kept at its real (un-compressed) length; only the per-token
// IAT and the idle gap are divided by N (representing N concurrent
// sessions' aggregate token rate during that real-time window). Token RATE
// is also recalibrated to the cited 30-100 tok/s (vs the other bursty
// models' faster 50-200 tok/s).
#define LLM_REALISTIC_IAT_MIN_S			0.010	///> Minimum intra-burst IAT: 10 ms (100 tok/s)
#define LLM_REALISTIC_IAT_MAX_S			0.033	///> Maximum intra-burst IAT: 33 ms (~30 tok/s)
#define LLM_BURST_DURATION_MIN_S			10.0	///> Minimum burst (= one streaming response) duration: 10 s, NOT divided by N
#define LLM_BURST_DURATION_MAX_S			60.0	///> Maximum burst duration: 60 s, NOT divided by N
#define LLM_REALISTIC_IDLE_GAP_MIN_S		5.0		///> Minimum between-turn idle gap: 5 s (user reads/types next prompt)
#define LLM_REALISTIC_IDLE_GAP_MAX_S		30.0	///> Maximum between-turn idle gap: 30 s

component TrafficGenerator : public TypeII{

	// Methods
	public:
		// COST
		void Setup();
		void Start();
		void Stop();
		// Generic
		void InitializeTrafficGenerator();
		void GenerateTraffic();
//		void NewPacketGenerated();

	// Public items (entered by agents constructor in komondor_main)
	public:

		int node_type;			///> Type of node associated to the traffic generator
		int node_id; 			///> Node identifier associated to the traffic generator
		int traffic_model;		///> Traffic model
		double traffic_load;	///> Average traffic load of the AP [packets/s]
		// Burst traffic
		double burst_rate;		///> Average time between two packet generation bursts [bursts/s]
		int num_bursts;			///> Total number of bursts occurred in the simulation
		// TRAFFIC_LLM_TOKENS_BURSTY state: tokens remaining in the current
		// burst after the packet just generated (0 at init, forcing the
		// very first packet to start a fresh burst after an idle gap).
		int llm_burst_tokens_left;
		// TRAFFIC_LLM_TOKENS_BURSTY_REALISTIC state: absolute sim time when
		// the current burst ends (0 at init, forcing the first packet to
		// start a fresh burst after an idle gap).
		double llm_burst_end_time;

	// Private items (just for node operation)
	private:

	// Connections and timers
	public:

		// INPORT connections to receive packets being generated
		inport inline void NewPacketGenerated(trigger_t& t1);
		// OUTPORT connections for sending notifications
		outport void outportNewPacketGenerated();
		// Timer ruled by the packet generation ratio
		Timer <trigger_t> trigger_new_packet_generated;
		// Connect the timer with the inport method
		TrafficGenerator () {
			connect trigger_new_packet_generated.to_component,NewPacketGenerated;
		}

};

/**
 * Setup()
 */
void TrafficGenerator :: Setup(){
	// Do nothing
};

/**
 * Start()
 */
void TrafficGenerator :: Start(){

	if (node_type == NODE_TYPE_AP) { // TODO: modify this condition in order to include "transmitters", not APs
		InitializeTrafficGenerator();
	}

};

/**
 * Stop()
 */
void TrafficGenerator :: Stop(){

};

/**
 * Main method for generating traffic
 */
void TrafficGenerator :: GenerateTraffic() {

	double time_for_next_packet (0);
	double time_to_trigger (0);
//	printf("traffic_model = %d\n", traffic_model);

	switch(traffic_model) {

		// TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION is not considered (handled by the node)
		case TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION:{
			// DO NOTHING: THE NODES HANDLE IT
//			printf("TG%d TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION!\n", node_id);
			break;
		}

		// 0
		// Sergio on 16 Jan:
		// - Full buffer is still to be implemented.
		// - To simulate it, just use Poisson traffic with large traffic loads.
		case TRAFFIC_FULL_BUFFER:{
			if(node_id == 0) printf("WARNING: FULL BUFFER NOT IMPLEMENTED! SIMULATE IT: just use Poisson traffic with large traffic loads\n");
			// Change to Poisson with huge traffic load to simulate saturation
			traffic_model = TRAFFIC_POISSON;
			traffic_load = 1000;
			// --- Poisson ---
			time_for_next_packet = Exponential(1/traffic_load);
			time_to_trigger = SimTime() + time_for_next_packet;
			trigger_new_packet_generated.Set(FixTimeOffset(time_to_trigger,13,12));
			break;
		}

		// 1
		case TRAFFIC_POISSON:{
			// Sergio on 11th January 2018
			// - Traffic generation depends on the traffic load (not on the lambda parameter, which is related
			//   with BO generation exponentially distributed.
			// time_for_next_packet = Exponential(1/lambda);
			// Generates new packet when the trigger expires
			time_for_next_packet = Exponential(1/traffic_load);
			time_to_trigger = SimTime() + time_for_next_packet;
			trigger_new_packet_generated.Set(FixTimeOffset(time_to_trigger,13,12));
			break;
		}

		// 2
		case TRAFFIC_DETERMINISTIC:{
			int lambda = 10000;
			time_for_next_packet = 1/lambda;
			time_to_trigger = SimTime() + time_for_next_packet;
			trigger_new_packet_generated.Set(FixTimeOffset(time_to_trigger,13,12));
			break;
		}

		// 3
		case TRAFFIC_POISSON_BURST:{
			// Sergio on 2nd February 2018
			// - Input: traffic load and average time between bursts
			time_for_next_packet = Exponential(burst_rate/traffic_load);
			time_to_trigger = SimTime() + time_for_next_packet;
//			if(save_node_logs) fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s New generation burst will be triggered in %f ms\n",
//				SimTime(), node_id, node_state, LOG_F00, LOG_LVL3,
//				time_for_next_packet * 1000);
			trigger_new_packet_generated.Set(FixTimeOffset(time_to_trigger,13,12));
			break;
		}

		// 4
		case TRAFFIC_LLM_TOKENS:{
			// LLM token-generation traffic: each inter-arrival gap is drawn
			// independently and uniformly at random within a bounded range
			// -> traffic_load is used to determine the concurrent LLM flows
			double _n_equivalent_flows = (traffic_load >= 1.0) ? traffic_load : 1.0;
			time_for_next_packet = (LLM_TRAFFIC_IAT_MIN_S
				+ Random(LLM_TRAFFIC_IAT_MAX_S - LLM_TRAFFIC_IAT_MIN_S)) / _n_equivalent_flows;
			time_to_trigger = SimTime() + time_for_next_packet;
			trigger_new_packet_generated.Set(FixTimeOffset(time_to_trigger,13,12));
			break;
		}

		// 5
		case TRAFFIC_LLM_TOKENS_BURSTY_REALISTIC:{
			// Realistic LLM traffic where burst DURATION is kept at its real length (10-60s, NOT divided by N) 
			// -> only the per-token IAT and the idle gap are divided by N. 
			double _n_equivalent_flows = (traffic_load >= 1.0) ? traffic_load : 1.0;
			if (SimTime() < llm_burst_end_time) {
				// Still inside the burst window
				time_for_next_packet = (LLM_REALISTIC_IAT_MIN_S
					+ Random(LLM_REALISTIC_IAT_MAX_S - LLM_REALISTIC_IAT_MIN_S)) / _n_equivalent_flows;
			} else {
				// Start a new burst: draw a REAL (not N-compressed) duration
				double _burst_duration = LLM_BURST_DURATION_MIN_S
					+ Random(LLM_BURST_DURATION_MAX_S - LLM_BURST_DURATION_MIN_S);
				llm_burst_end_time = SimTime() + _burst_duration;
				time_for_next_packet = (LLM_REALISTIC_IDLE_GAP_MIN_S
					+ Random(LLM_REALISTIC_IDLE_GAP_MAX_S - LLM_REALISTIC_IDLE_GAP_MIN_S)) / _n_equivalent_flows;
			}
			time_to_trigger = SimTime() + time_for_next_packet;
			trigger_new_packet_generated.Set(FixTimeOffset(time_to_trigger,13,12));
			break;
		}

		default:{
			printf("Wrong traffic model!\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
}

/**
 * Generate a new packet upon trigger-based activation
 */
void TrafficGenerator :: NewPacketGenerated(trigger_t &){
//	printf("TG%d NewPacketGenerated!\n", node_id);
	outportNewPacketGenerated();
	GenerateTraffic();
}

/**
 * Initialize all the variables of the traffic generator and starts generating traffic
 */
void TrafficGenerator :: InitializeTrafficGenerator() {
	/*
	 * HARDCODED VARIABLES FOR TESTING PURPOSES
	 * - This variables are initialized in the code itself
	 * - While this is not the most efficient approach, it allows us testing new feature
	 */
	burst_rate = 10;
	num_bursts = 0;
	llm_burst_tokens_left = 0;
	llm_burst_end_time = 0.0;
	GenerateTraffic();
}
