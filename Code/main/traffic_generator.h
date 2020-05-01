/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
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
#include "../methods/auxiliary_methods.h"

// Agent component: "TypeII" represents components that are aware of the existence of the simulated time.
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

	// Private items (just for node operation)
	private:

		// ONOFF traffic model
		double time_traffic_state_initiated;	///> Timestamp where traffic state was initiated
		double duration_current_traffic_state;	///> Duration to be completed in current traffic state
		int traffic_onoff_state;				///> Traffic state indicator (0: OFF, 1: ON)
		double prob_traffic_state_on;			///> Probability next traffic state is ON
		double traffic_load_onoff;				///> Traffic load during the ON state in the ON/OFF traffic model

	// Connections and timers
	public:

		// INPORT connections to receive packets being generated
		inport inline void NewPacketGenerated(trigger_t& t1);
		inport inline void GenerateTrafficONOFF(trigger_t& t1);
		// OUTPORT connections for sending notifications
		outport void outportNewPacketGenerated();

		// Timer ruled by the packet generation ratio
		Timer <trigger_t> trigger_new_packet_generated;
		// Timer for triggering ON/OFF traffic
		Timer <trigger_t> trigger_onoff_traffic;

		// Connect the timer with the inport method
		TrafficGenerator () {
			connect trigger_new_packet_generated.to_component,NewPacketGenerated;
			connect trigger_onoff_traffic.to_component,GenerateTrafficONOFF;
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


void TrafficGenerator :: GenerateTrafficONOFF(trigger_t &) {

	GenerateTraffic();

}

/**
 * Main method for generating traffic
 */
void TrafficGenerator :: GenerateTraffic() {

	double time_for_next_packet (0);
	double time_to_trigger (0);


	/*
	 * HARDCODED FOR PAPER 10: VARIABLE MEAN TRAFFIC LOAD FOR POISSON
	 * - Hardcode a pattern of changes in the mean traffic load by checking the SimTime()
	 */

//	if(node_id == 0){
//
//		// 2 traffic load periods
//		if(SimTime()/StopTime() < 1.0/2.0){
//			//printf("First half of simulation: %f / %f (%f)\n", SimTime(), StopTime(), SimTime()/StopTime());
//			//traffic_load = 50*83.33;
//			traffic_load = 200*83.33;
//		} else {
//			//printf("Second half of simulation: %f / %f (%f)\n", SimTime(), StopTime(), SimTime()/StopTime());
//			//traffic_load = 200*83.33;
//			traffic_load = 50*83.33;
//		}
//
////		// 3 traffic load periods
////		if(SimTime()/StopTime() < 1.0/3.0){
////			printf("First third of simulation: %f / %f (%f)\n", SimTime(), StopTime(), SimTime()/StopTime());
////			// change traffic_load
////		} else if (SimTime()/StopTime() < 2.0/3.0){
////			printf("Second third of simulation: %f / %f (%f)\n", SimTime(), StopTime(), SimTime()/StopTime());
////			// change traffic_load
////		} else {
////			printf("Third third of simulation: %f / %f (%f)\n", SimTime(), StopTime(), SimTime()/StopTime());
////			// change traffic_load
////		}
////
//	}

	/*
	 * END OF HARDCODING
	 */


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

		//4
		case TRAFFIC_POISSON_ONOFF:{

			// Identify or set new traffic generation state (ON or OFF)
			double time_in_current_traffic_state = SimTime() - time_traffic_state_initiated;

			if(time_in_current_traffic_state >= (duration_current_traffic_state - 0.0001)){	// New state

				if(traffic_onoff_state == 0){
					traffic_onoff_state = 1;
					duration_current_traffic_state = Exponential(TRAFFIC_ONOFF_DURATION_ON_SECONDS);
				} else {
					traffic_onoff_state = 0;
					time_traffic_state_initiated = SimTime();
					duration_current_traffic_state = Exponential(TRAFFIC_ONOFF_DURATION_OFF_SECONDS);
					trigger_onoff_traffic.Set(FixTimeOffset(SimTime() + duration_current_traffic_state,13,12));
				}

				time_traffic_state_initiated = SimTime();

			} // else: keep the state as it was

			if(traffic_onoff_state == 1){	// If traffic state ON, generate packets

				time_for_next_packet = Exponential(1/traffic_load_onoff);
				time_to_trigger = SimTime() + time_for_next_packet;
				trigger_new_packet_generated.Set(FixTimeOffset(time_to_trigger,13,12));

			} // else: just wait until next state draw

			break;

		}

		default:{
			printf("Wrong traffic model!\n");
			printf("- traffic_model = %d\n", traffic_model);
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

	traffic_onoff_state = 0;
	time_traffic_state_initiated = 0;
	duration_current_traffic_state = 0;
	prob_traffic_state_on = TRAFFIC_ONOFF_DURATION_OFF_SECONDS
			/ (TRAFFIC_ONOFF_DURATION_OFF_SECONDS + TRAFFIC_ONOFF_DURATION_ON_SECONDS);
	traffic_load_onoff = traffic_load
			* (TRAFFIC_ONOFF_DURATION_OFF_SECONDS + TRAFFIC_ONOFF_DURATION_ON_SECONDS)
			/ TRAFFIC_ONOFF_DURATION_ON_SECONDS;

	GenerateTraffic();
}
