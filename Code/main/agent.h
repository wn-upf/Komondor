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
#include "../structures/action.h"
#include "../methods/auxiliary_methods.h"
#include "../methods/agent_methods.h"

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

		// Communication with AP
		void RequestInformationToAp();
		void ComputeNewConfiguration();
		void SendNewConfigurationToAp();

		// Handle rewards
		void UpdateRewardSelectedArm(double reward);

		Configuration GenerateNewConfiguration(int ix_selected_arm);

		// Print methods
		void PrintAgentInfo();

	// Public items (entered by nodes constructor in Komondor simulation)
	public:

		// Specific to each agent
		int agent_id; 			// Node identifier
		char *wlan_code;
		int *list_of_channels; 	// List of channels
		double *list_of_cca_values;	// List of CCA values
		double *list_of_tx_power_values;	// List of tx power values
		Action *actions;		// List of actions
		int num_actions;		// Number of actions (depends on the configuration parameters - CCA, TPC, channels, etc.)
		int num_actions_channel;
		int num_actions_cca;
		int num_actions_tx_power;

		double *reward_per_arm;	// Number of txs trials per number of channels

		// FRANKY: HARDCODED data for testing agents basic structure
		int time_between_requests; 	// Time between two information requests to the AP (for a given measurement)
		double initial_reward;			// Initial reward assigned to each arm
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

		// Triggers
		Timer <trigger_t> trigger_request_information_to_ap; // Timer for requesting information to the AP

		// Every time the timer expires execute this
		inport inline void RequestInformationToAp(trigger_t& t1);

		// Connect timers to methods
		Agent () {
			connect trigger_request_information_to_ap.to_component,RequestInformationToAp;
		}

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

	// Generate the first request, to be triggered after "time_between_requests"
	// *** We generate here the first request in order to obtain the AP's configuration
	trigger_request_information_to_ap.Set(fix_time_offset(SimTime() + time_between_requests,13,12));

};

/*
 * Stop()
 */
void Agent :: Stop(){


};

/**************************/
/**************************/
/*  COMMUNICATION METHODS */
/**************************/
/**************************/

/*
 * RequestInformationToAp():
 * Input arguments:
 * - to be defined
 */
void Agent :: RequestInformationToAp(trigger_t &){

//	printf("%s Agent #%d: Requesting information to AP\n", LOG_LVL1, agent_id);
	outportRequestInformationToAp();

};

/*
 * InportReceivingInformationFromAp(): called when some node (this one included) starts a TX
 * Input arguments:
 * - to be defined
 */
void Agent :: InportReceivingInformationFromAp(Configuration &received_configuration){

//	printf("%s Agent #%d: Message received from the AP\n", LOG_LVL1, agent_id);

	configuration = received_configuration;

//	configuration.PrintConfiguration();
//	configuration.report.PrintReport();
	UpdateRewardSelectedArm(0.5);

	ComputeNewConfiguration();

};

/*
 * ComputeNewConfiguration():
 * Input arguments:
 * - to be defined
 */
void Agent :: ComputeNewConfiguration(){

//	printf("%s Agent #%d: Computing a new configuration\n", LOG_LVL1, agent_id);

	// TODO: Implement X algorithm according to current configuration and performance
	// ...
//	printf("%s Agent #%d: data packets sent = %d\n", LOG_LVL1, agent_id,
//			configuration.report.data_packets_sent);
	// Generate new configuration according to the algorithm's output
	// ...
	//int ix_selected_arm = rand() % num_actions;
	double epsilon = 1;
	int ix_selected_arm = PickArmEgreedy(num_actions, reward_per_arm, epsilon);
	printf("Selected arm = %d\n", ix_selected_arm);

	// HARDCODED: generate new configuration
	new_configuration = GenerateNewConfiguration(ix_selected_arm);

	SendNewConfigurationToAp();

}

/*
 * SendNewConfigurationToAp():
 * Input arguments:
 * - to be defined
 */
void Agent :: SendNewConfigurationToAp(){

//	printf("%s Agent #%d: Sending new configuration to AP\n", LOG_LVL1, agent_id);
	outportSendConfigurationToAp(new_configuration);

	// Set trigger for next request
	trigger_request_information_to_ap.Set(fix_time_offset(SimTime() + time_between_requests,13,12));

};


/*
 * GenerateNewConfiguration: encapsulates the configuration of a node to be sent
 **/
Configuration Agent :: GenerateNewConfiguration(int ix_selected_arm){

	// Step 1: According to the selected arm, find each configuration
	int indexes_array[3];
	index2values(indexes_array, ix_selected_arm, num_actions_channel, num_actions_cca, num_actions_tx_power);

	// Step 2: build configuration accordingly
	Configuration new_configuration;
	new_configuration = configuration; 		// Set configuration to the received one, and then change specific parameters

	new_configuration.timestamp = SimTime();

	new_configuration.primary_channel = list_of_channels[indexes_array[0]];
	new_configuration.cca_default = ConvertPower(PW_TO_DBM, list_of_cca_values[indexes_array[1]]);
	new_configuration.tpc_default = ConvertPower(PW_TO_DBM, list_of_tx_power_values[indexes_array[2]]);

	return new_configuration;

}

/********************/
/********************/
/*  ACTION METHODS  */
/********************/
/********************/

void Agent :: UpdateRewardSelectedArm(double reward) {

	// TODO: switch to select the reward according to the metric used

	// Step 1: find index according to the current configuration
	int indexes_selected_arm[3];

	FindIndexesOfConfiguration(indexes_selected_arm, configuration, num_actions_channel, num_actions_cca,
			num_actions_tx_power, list_of_channels, list_of_cca_values, list_of_tx_power_values);

//	printf("indexes_selected_arm[0] = %d\n", indexes_selected_arm[0]);
//	printf("indexes_selected_arm[1] = %d\n", indexes_selected_arm[1]);
//	printf("indexes_selected_arm[2] = %d\n", indexes_selected_arm[2]);

	int ix_selected_arm = values2index(indexes_selected_arm, num_actions_channel, num_actions_cca);
	//printf("ix_selected_arm = %d\n",ix_selected_arm);

	// Step 2: set the reward accordingly
	reward_per_arm[ix_selected_arm] = reward;

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
	printf("Agent #%d says: Initializing actions\n", agent_id);

	list_of_channels = (int *) malloc(num_actions_channel * sizeof(*list_of_channels));
	list_of_cca_values = (double *) malloc(num_actions_cca * sizeof(*list_of_cca_values));
	list_of_tx_power_values = (double *) malloc(num_actions_tx_power * sizeof(*list_of_tx_power_values));

//	printf("num_actions_channel: %d\n", num_actions_channel);
//	printf("num_actions_cca: %d\n", num_actions_cca);
//	printf("num_actions_tx_power: %d\n", num_actions_tx_power);
	num_actions = num_actions_channel * num_actions_cca * num_actions_tx_power;

	// Generate actions
	actions = (Action *) malloc(num_actions	* sizeof(*actions));

	// Initialize the rewards assigned to each arm
	reward_per_arm = (double *) malloc(num_actions * sizeof(*reward_per_arm));

	for(int i = 0; i < num_actions; i++){
		reward_per_arm[i] = initial_reward;	// Set the initial reward
	}


}
/***********************/
/***********************/
/*  AUXILIARY METHODS  */
/***********************/
/***********************/

/************************/
/************************/
/*  PRINT INFORMATION   */
/************************/
/************************/

/*
 * PrintAgentInfo(): prints Agent info
 */
void Agent :: PrintAgentInfo(){

	printf("%s Agent %d info:\n", LOG_LVL3, agent_id);
	printf("%s wlan_code = %s\n", LOG_LVL4, wlan_code);
	printf("%s time_between_requests = %d\n", LOG_LVL4, time_between_requests);

	printf("%s list_of_channels: ", LOG_LVL4);
	for (int i = 0; i < num_actions_channel; i ++) {
		printf("%d  ", list_of_channels[i]);
	}
	printf("\n");

	printf("%s list_of_cca_values: ", LOG_LVL4);
	for (int i = 0; i < num_actions_cca; i ++) {
		printf("%f pW (%f dBm)  ", list_of_cca_values[i], ConvertPower(PW_TO_DBM, list_of_cca_values[i]));
	}
	printf("\n");

	printf("%s list_of_tx_power_values: ", LOG_LVL4);
	for (int i = 0; i < num_actions_tx_power; i ++) {
		printf("%f pW (%f dBm)  ", list_of_tx_power_values[i], ConvertPower(PW_TO_DBM, list_of_tx_power_values[i]));
	}
	printf("\n");

}

///*
// * WriteAgentInfo(): writes Agent info
// */
// TODO: pending to decide whether to generate logs for agents or not
//void Node :: WriteAgentInfo(Logger agent_logger, int info_detail_level, char *header_string){
//
//}
