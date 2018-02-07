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
 * File description: defines the agent component
 *
 * - This file contains the instructions to be followed by an agent, as well
 * as the communication with the APs of the controlled networks
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../list_of_macros.h"
#include "../structures/node_configuration.h"

// Agent component: "TypeII" represents components that are aware of the existence of the simulated time.
component Agent : public TypeII{

	// Methods
	public:

		// COST
		void Setup();
		void Start();
		void Stop();

		// Generic
		void InitializeAgent();
		void RequestInformationToAp();
		void ComputeNewConfiguration();
		void SendNewConfigurationToAp();

		Configuration GenerateNewConfiguration(double timestamp);

	// Public items (entered by nodes constructor in Komondor simulation)
	public:

		// Specific to a node
		int agent_id; 				// Node identifier

		// FRANKY: HARDCODED data for testing agents basic structure
		int time_between_requests; // Time between two information requests to the AP (for a given measurement)
		// ...

	// Private items (just for node operation)
	private:
		Configuration configuration;
		Configuration new_configuration;

	// Connections and timers
	public:

		// INPORT connections for receiving notifications
		inport void inline InportReceivingInformationFromAp(Configuration &configuration);

		// OUTPORT connections for sending notifications
		outport void outportRequestInformationToAp();
		outport void outportSendConfigurationToAp(Configuration &new_configuration);

};

/*
 * Setup()
 */
void Agent :: Setup(){
	// Do nothing
};

/*
 * Start()
 */
void Agent :: Start(){

	// Initialize variables
	InitializeAgent();

};

/*
 * Stop()
 */
void Agent :: Stop(){


};

/*
 * InportReceivingInformationFromAp(): called when some node (this one included) starts a TX
 * Input arguments:
 * - to be defined
 */
void Agent :: InportReceivingInformationFromAp(Configuration &configuration){

	printf("%s Agent #%d: Message received from the AP\n", LOG_LVL1, agent_id);

	configuration.PrintConfiguration();

	ComputeNewConfiguration();

};

/*
 * RequestInformationToAp():
 * Input arguments:
 * - to be defined
 */
void Agent :: RequestInformationToAp(){

	printf("%s Agent #%d: Requesting information to AP\n", LOG_LVL1, agent_id);
	outportRequestInformationToAp();

};

/*
 * ComputeNewConfiguration():
 * Input arguments:
 * - to be defined
 */
void Agent :: ComputeNewConfiguration(){

	printf("%s Agent #%d: Computing a new configuration\n", LOG_LVL1, agent_id);

	// TODO: Implement X algorithm according to current configuration and performance
	// ...

	// Generate new configuration according to the algorithm's output
	// ...

	// HARDCODED: generate new configuration
	new_configuration = GenerateNewConfiguration(SimTime());

	SendNewConfigurationToAp();

}

/*
 * SendNewConfigurationToAp():
 * Input arguments:
 * - to be defined
 */
void Agent :: SendNewConfigurationToAp(){

	printf("%s Agent #%d: Sending new configuration to AP\n", LOG_LVL1, agent_id);
	outportSendConfigurationToAp(new_configuration);

};


/*
 * GenerateNewConfiguration: encapsulates the configuration of a node to be sent
 **/
Configuration Agent :: GenerateNewConfiguration(double timestamp){

	Configuration new_configuration;

	new_configuration.timestamp = timestamp;

	new_configuration.destination_id = configuration.destination_id;
	new_configuration.lambda = configuration.lambda;
	new_configuration.primary_channel = configuration.primary_channel;
	new_configuration.num_channels_allowed = configuration.num_channels_allowed;
	double new_power = 100.0;
	new_configuration.tpc_default = new_power;
	new_configuration.cca_default = configuration.cca_default;
	new_configuration.channel_bonding_model = configuration.channel_bonding_model;

	return new_configuration;

}

/*****************************/
/*****************************/
/*  VARIABLE INITIALIZATION  */
/*****************************/
/*****************************/

/*
 * InitializeVariables(): initializes all the necessary variables
 */
void Agent :: InitializeAgent() {

	printf("Agent #%d says: I'm alive!\n", agent_id);
	RequestInformationToAp();

	// TODO: instead of starting requesting information to the AP,
	// call a "generator" of requests, which is governed by input
	// parameters (to be defined in "komondor_main")

}
