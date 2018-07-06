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
#include <iostream>
#include <stdlib.h>

#include "../list_of_macros.h"
#include "../structures/node_configuration.h"
#include "../structures/performance_report.h"
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
		void SendNewConfigurationToAp(Configuration &configuration_to_send);

		// Communication with other Agents (distributed methods)


		// Handle rewards
		void GenerateRewardSelectedArm();

		Configuration GenerateNewConfiguration();

		// Print methods
		void PrintAgentInfo();
		void PrintAction(int action_ix, int detail);
		//void WriteActionReport(int action_ix);

		void WriteConfiguration(Configuration configuration_to_write);

		void PrintOrWriteAgentStatistics(int write_or_print);

	// Public items (entered by agents constructor in komondor_main)
	public:

		// Specific to each agent
		int agent_id; 			// Node identifier
		int centralized;		// Indicates whether the node is controlled by a central entity or not
		std::string wlan_code;
		int *list_of_channels; 	// List of channels
		double *list_of_cca_values;	// List of CCA values
		double *list_of_tx_power_values;	// List of tx power values
		int *list_of_dcb_policy;	// List of DCB policies
		Action *actions;		// List of actions
		int num_actions;		// Number of actions (depends on the configuration parameters - CCA, TPC, channels, etc.)
		int num_actions_channel;
		int num_actions_cca;
		int num_actions_tx_power;
		int num_actions_dcb_policy;
		int num_requests;		// Number of requests made by the agent to the AP

		int ix_selected_arm; 	// Index of the current selected arm

		double *reward_per_arm;				// Reward experienced after playing each arm
		double *average_reward_per_arm;
		double *cumulative_reward_per_arm;
		int *times_arm_has_been_selected; 	// Number of times an arm has been played

		int type_of_reward;
		double time_between_requests; 	// Time between two information requests to the AP (for a given measurement)
		double initial_reward;			// Initial reward assigned to each arm
		double epsilon;					// Epsilon parameter of the epsilon-greedy
		double initial_epsilon;			// Initial value of epsilon
		int num_epsilon_iterations;		// Number of epsilon-greedy iterations
		// ...

		int save_agent_logs;
		int print_agent_logs;

		std::string simulation_code;				// Simulation code
	// Private items (just for node operation)
	private:
		Configuration configuration;
		Configuration new_configuration;
		Configuration configuration_from_controller;

		Report report;

		// File for writting node logs
		FILE *output_log_file;				// File for logs in which the agent is involved
		char own_file_path[32];				// Name of the file for agent logs
		Logger agent_logger;				// struct containing the attributes needed for writting logs in a file
		char *header_string;				// Header string for the logger

	// Connections and timers
	public:

		// INPORT connections for receiving notifications
		inport void inline InportReceivingInformationFromAp(Configuration &configuration, Report &report);

		// INPORT (centralized system only)
		inport void inline InportReceivingRequestFromController(int destination_agent_id);
		inport void inline InportReceiveConfigurationFromController(int destination_agent_id, Configuration &new_configuration);

		// OUTPORT connections for sending notifications
		outport void outportRequestInformationToAp();
		outport void outportSendConfigurationToAp(Configuration &new_configuration);

		// OUTPORT (centralized system only)
		outport void outportAnswerToController(Configuration &configuration, Report &report, int agent_id);

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

	// Create agent logs file if required
	if(save_agent_logs) {
		// Name agent log file accordingly to the agent_id
		sprintf(own_file_path,"%s_A%d_%s.txt","../output/logs_output", agent_id, wlan_code.c_str());
		remove(own_file_path);
		output_log_file = fopen(own_file_path, "at");
		agent_logger.save_logs = save_agent_logs;
		agent_logger.file = output_log_file;
		agent_logger.SetVoidHeadString();
	}
	
	// TODO: agent output filename
	// Name agent log file accordingly to the agent_id
	// sprintf(own_file_path,"../output/agents_output_%s", simulation_code.c_str());
	// remove(own_file_path);
	// output_log_file = fopen(own_file_path, "at");
	// agent_logger.save_logs = save_agent_logs;
	// agent_logger.file = output_log_file;
	// agent_logger.SetVoidHeadString();
	// fprintf(agent_logger.file, "%s;",simulation_code.c_str());
	// ----

	if(save_agent_logs) fprintf(agent_logger.file,"%.18f;A%d;%s;%s Start()\n",
			SimTime(), agent_id, LOG_B00, LOG_LVL1);


	if(centralized) {
		// --- Do nothing ---
		// In case of being centralized, wait for a request from the controller
	} else {
		// Generate the first request, to be triggered after "time_between_requests"
		// *** We generate here the first request in order to obtain the AP's configuration
		double extra_wait_test = 0.005 * (double) agent_id;
		trigger_request_information_to_ap.Set(fix_time_offset(SimTime() + time_between_requests + extra_wait_test,13,12));
	}

};

/*
 * Stop()
 */
void Agent :: Stop(){

	if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s Agent Stop()\n",
			SimTime(), agent_id, LOG_C00, LOG_LVL1);

	// Print and write node statistics if required
	PrintOrWriteAgentStatistics(PRINT_LOG);
	PrintOrWriteAgentStatistics(WRITE_LOG);

	// Close node logs file
	if(save_agent_logs) fclose(agent_logger.file);

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
	if(save_agent_logs && !centralized) fprintf(agent_logger.file, "----------------------------------------------------------------\n");

	if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s RequestInformationToAp() (request #%d)\n",
			SimTime(), agent_id, LOG_F00, LOG_LVL1, num_requests);

	if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s Requesting information to AP\n",
			SimTime(), agent_id, LOG_C00, LOG_LVL2);

	outportRequestInformationToAp();

	num_requests ++;

};

/*
 * InportReceivingInformationFromAp(): called when some node (this one included) starts a TX
 * Input arguments:
 * - to be defined
 */
void Agent :: InportReceivingInformationFromAp(Configuration &received_configuration, Report &received_report){

//	printf("%s Agent #%d: Message received from the AP\n", LOG_LVL1, agent_id);

	if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s InportReceivingInformationFromAp()\n",
			SimTime(), agent_id, LOG_F00, LOG_LVL1);

	if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s New information has been received from the AP\n",
			SimTime(), agent_id, LOG_C00, LOG_LVL2);

	configuration = received_configuration;

	if(save_agent_logs) WriteConfiguration(configuration);

	report = received_report;

	if (centralized) {
		// Forward the information to the controller
		if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s Answering to the controller with current information\n",
			SimTime(), agent_id, LOG_F02, LOG_LVL2);
		outportAnswerToController(configuration, report, agent_id);

	} else {
		// Generate the reward for the last selected action
		GenerateRewardSelectedArm();
		// Compute a new configuration according to the updated rewards
		ComputeNewConfiguration();
	}

};

/*
 * ComputeNewConfiguration():
 * Input arguments:
 * - to be defined
 */
void Agent :: ComputeNewConfiguration(){

	//printf("%s Agent #%d: Computing a new configuration\n", LOG_LVL1, agent_id);

	if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s ComputeNewConfiguration()\n",
			SimTime(), agent_id, LOG_F00, LOG_LVL1);

	if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s Computing a new configuration\n",
			SimTime(), agent_id, LOG_C00, LOG_LVL2);

	// TODO: Implement X algorithm according to current configuration and performance
	// ...

	// Generate new configuration according to the algorithm's output
	epsilon = initial_epsilon / sqrt( (double) num_epsilon_iterations);

	ix_selected_arm = PickArmEgreedy(num_actions, average_reward_per_arm, epsilon);
	//printf("ix_selected_arm = %d\n",ix_selected_arm);
	if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s Epsilon-greedy iteration #%d (eps = %.3f) selected arm %d\n",
				SimTime(), agent_id, LOG_C00, LOG_LVL2,
				num_epsilon_iterations, epsilon, ix_selected_arm);

	times_arm_has_been_selected[ix_selected_arm] ++;
	num_epsilon_iterations ++;
	//PrintSelectedAction();

	// Generate new configuration
	new_configuration = GenerateNewConfiguration();

	// Send the configuration to the AP
	SendNewConfigurationToAp(new_configuration);

}

/*
 * SendNewConfigurationToAp():
 * Input arguments:
 * - configuration_to_send: Configuration struct that is sent to the corresponding AP
 */
void Agent :: SendNewConfigurationToAp(Configuration &configuration_to_send){

//	printf("%s Agent #%d: Sending new configuration to AP\n", LOG_LVL1, agent_id);

	if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s SendNewConfigurationToAp()\n",
			SimTime(), agent_id, LOG_F00, LOG_LVL1);

	if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s Sending a new configuration to the AP\n",
			SimTime(), agent_id, LOG_C00, LOG_LVL2);

	if(save_agent_logs) WriteConfiguration(configuration_to_send);

	// TODO (LOW PRIORITY): generate a trigger to simulate delays in the agent-node communication
	outportSendConfigurationToAp(configuration_to_send);

	// Set trigger for next request in case of being an independent agent (not controlled by a central entity)
	if (!centralized) {
		if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s Next request to be sent at %f\n",
				SimTime(), agent_id, LOG_C00, LOG_LVL2, fix_time_offset(SimTime() + time_between_requests,13,12));

		trigger_request_information_to_ap.Set(fix_time_offset(SimTime() + time_between_requests,13,12));
	} else {
		if(save_agent_logs) fprintf(agent_logger.file, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	}

};


/*
 * GenerateNewConfiguration: encapsulates the configuration of a node to be sent
 **/
Configuration Agent :: GenerateNewConfiguration(){

	// Step 1: According to the selected arm, find each configuration
	int indexes_selected_arm[NUM_FEATURES_ACTIONS]; // 4 features considered
	index2values(indexes_selected_arm, ix_selected_arm,
			num_actions_channel, num_actions_cca, num_actions_tx_power, num_actions_dcb_policy);

	// Step 2: build configuration accordingly
	Configuration new_configuration;
	new_configuration = configuration; 		// Set configuration to the received one, and then change specific parameters

	new_configuration.timestamp = SimTime();
	// TODO: by now, consider only 1 channel (left ch. = right ch.)
	new_configuration.selected_primary_channel = list_of_channels[indexes_selected_arm[0]];
	//new_configuration.selected_left_channel = list_of_channels[indexes_selected_arm[0]];
	//new_configuration.selected_right_channel = list_of_channels[indexes_selected_arm[0]];
	new_configuration.selected_cca = list_of_cca_values[indexes_selected_arm[1]];
	new_configuration.selected_tx_power = list_of_tx_power_values[indexes_selected_arm[2]];
	new_configuration.selected_dcb_policy = list_of_dcb_policy[indexes_selected_arm[3]];

	return new_configuration;

}

/*******************************/
/*******************************/
/*  CENTRAL CONTROLLER METHODS */
/*******************************/
/*******************************/

/*
 * InportReceivingRequestFromAgent(): called when some agent answers for information to the AP
 * Input arguments:
 * -
 */
void Agent :: InportReceivingRequestFromController(int destination_agent_id) {

//	printf("%s Agent #%d: New information request received from the Controller\n", LOG_LVL1, agent_id);

	if(agent_id == destination_agent_id) {

		if(save_agent_logs) fprintf(agent_logger.file, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

		if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s New information request received from the Controller for Agent %d\n",
			SimTime(), agent_id, LOG_F02, LOG_LVL2, destination_agent_id);

		trigger_request_information_to_ap.Set(fix_time_offset(SimTime(),13,12));

	} else {

//		if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s I am NOT the destination!\n",
//			SimTime(), agent_id, LOG_F02, LOG_LVL3);

	}

}

/*
 * InportReceiveConfigurationFromAgent(): called when some agent sends instructions to the AP
 * Input arguments:
 * -
 */
void Agent :: InportReceiveConfigurationFromController(int destination_agent_id, Configuration &received_configuration) {

	if(agent_id == destination_agent_id) {

		if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s New configuration received from the Controller to Agent %d\n",
			SimTime(), agent_id, LOG_F02, LOG_LVL2, destination_agent_id);

		//	printf("%s Agent #%d: New configuration received from the Controller\n", LOG_LVL1, agent_id);

		configuration_from_controller = received_configuration;
		SendNewConfigurationToAp(configuration_from_controller);

	} else {

//		if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s I am NOT the destination!\n",
//			SimTime(), agent_id, LOG_F02, LOG_LVL3);

	}

}

/********************/
/********************/
/*  ACTION METHODS  */
/********************/
/********************/

/*
 * GenerateRewardSelectedArm():
 * Input arguments:
 * - Only one metric will be taken into account (different actions are done, accordingly)
 */
void Agent :: GenerateRewardSelectedArm() {

	if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s GenerateRewardSelectedArm()\n",
			SimTime(), agent_id, LOG_F00, LOG_LVL1);

	double reward;

	// Switch to select the reward according to the metric used (rewards must be normalized)
	switch(type_of_reward){

		/* PERFORMANCE_PACKETS_SENT:
		 * - The number of packets sent are taken into account
		 * - The reward must be bounded by the maximum number of data packets
		 * 	 that can be sent in each interval (e.g., packets that were sent but lost)
		 */
		case REWARD_TYPE_PACKETS_SENT:{
			reward = report.data_packets_sent /
					report.data_packets_lost;
			break;
		}

		/* PERFORMANCE_THROUGHPUT:
		 * - The throughput experienced during the last period is taken into account
		 * - The reward must be bounded by the maximum throughput that would be experienced
		 * 	 (e.g., consider the data rate granted by the modulation and the total time)
		 */
		case REWARD_TYPE_THROUGHPUT:{

			if (report.max_bound_throughput == 0) {
				reward = 0;
			} else {
				reward = report.throughput /
						report.max_bound_throughput;

			}

			if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s throughput = %.2f Mbps\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL2, report.throughput * pow(10,-6));

//			if(save_agent_logs) fprintf(agent_logger.file, "[DATA-THR];%.5f;A%d;%d;%.2f;%.5f\n",
//							SimTime(), agent_id, ix_selected_arm, report.throughput * pow(10,-6), reward);

			if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s max_bound_throughput = %f Mbps\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL2, report.max_bound_throughput * pow(10,-6));

			if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s reward = %f\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL2, reward);

			break;
		}

		/* REWARD_TYPE_PACKETS_GENERATED:
		 * -
		 */
		case REWARD_TYPE_PACKETS_GENERATED:{
			reward = (report.num_packets_generated -
					report.num_packets_dropped) /
					report.num_packets_generated;
			break;
		}

		default:{
			printf("ERROR: %d is not a correct type of performance indicator\n", type_of_reward);
			exit(EXIT_FAILURE);
			break;
		}

	}

//	WriteConfiguration(configuration);

	// Step 1: find index according to the current configuration
	int indexes_selected_arm[NUM_FEATURES_ACTIONS];	// 4 features considered (primary, CCA, Pow tx, DCB policy)

	FindIndexesOfConfiguration(indexes_selected_arm, configuration,
			num_actions_channel, num_actions_cca, num_actions_tx_power, num_actions_dcb_policy,
			list_of_channels, list_of_cca_values, list_of_tx_power_values, list_of_dcb_policy);

	ix_selected_arm = values2index(indexes_selected_arm,
			num_actions_channel, num_actions_cca, num_actions_tx_power, num_actions_dcb_policy);

	if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s ix_selected_arm = %d ([%d %d %d %d])\n",
			SimTime(), agent_id, LOG_C00, LOG_LVL2,
			ix_selected_arm, indexes_selected_arm[0], indexes_selected_arm[1],
			indexes_selected_arm[2], indexes_selected_arm[3]);

	// Step 2: set the reward accordingly
	if(ix_selected_arm >= 0) {

		reward_per_arm[ix_selected_arm] = reward;
		cumulative_reward_per_arm[ix_selected_arm] += reward;
		average_reward_per_arm[ix_selected_arm] = cumulative_reward_per_arm[ix_selected_arm] /
				times_arm_has_been_selected[ix_selected_arm];

	}

	int print_agent_logs = TRUE;
	if(save_agent_logs) PrintOrWriteRewardPerArm(WRITE_LOG, save_agent_logs, print_agent_logs, agent_logger,
			num_actions, reward_per_arm, cumulative_reward_per_arm, times_arm_has_been_selected, agent_id, SimTime());

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

	//printf("Agent #%d says: I'm alive!\n", agent_id);

	num_requests = 0;

	list_of_channels = new int[num_actions_channel];
	list_of_cca_values = new double[num_actions_cca];
	list_of_tx_power_values = new double[num_actions_tx_power];
	list_of_dcb_policy = new int[num_actions_dcb_policy];

	num_actions = num_actions_channel * num_actions_cca * num_actions_tx_power * num_actions_dcb_policy;

	// Generate actions
	actions = new Action[num_actions];

	// Initialize the rewards assigned to each arm
	reward_per_arm = new double[num_actions];
	cumulative_reward_per_arm = new double[num_actions];
	average_reward_per_arm = new double[num_actions];

	// Initialize the array containing the times each arm has been played
	times_arm_has_been_selected = new int[num_actions];

	initial_reward = 0;
	initial_epsilon = 1;
	num_epsilon_iterations = 1;

	for(int i = 0; i < num_actions; i++){
		reward_per_arm[i] = initial_reward;	// Set the initial reward
		cumulative_reward_per_arm[i] = initial_reward;
		average_reward_per_arm[i] = initial_reward;
		times_arm_has_been_selected[i] = 0;
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
	printf("%s wlan_code = %s\n", LOG_LVL4, wlan_code.c_str());
	printf("%s centralized = %d\n", LOG_LVL4, centralized);
	printf("%s time_between_requests = %f\n", LOG_LVL4, time_between_requests);
	printf("%s type_of_reward = %d\n", LOG_LVL4, type_of_reward);
	printf("%s initial_reward = %f\n", LOG_LVL4, initial_reward);
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

	printf("%s list_of_dcb_policy: ", LOG_LVL4);
	for (int i = 0; i < num_actions_dcb_policy; i ++) {
		printf("%d  ", list_of_dcb_policy[i]);
	}
	printf("\n");

	printf("%s save_agent_logs: %d\n", LOG_LVL4, save_agent_logs);
	printf("%s print_agent_logs: %d\n", LOG_LVL4, print_agent_logs);

	printf("\n");

}

/*
 * PrintAction(): prints Agent's selected action
 */
void Agent :: PrintAction(int action_ix, int detail){

	int indexes_selected_arm[NUM_FEATURES_ACTIONS];	// 4 features considered

	index2values(indexes_selected_arm, action_ix, num_actions_channel,
			num_actions_cca, num_actions_tx_power, num_actions_dcb_policy);

	printf("%s Action %d ([%d %d %d %d]\n", LOG_LVL2,
			action_ix, indexes_selected_arm[0], indexes_selected_arm[1], indexes_selected_arm[2], indexes_selected_arm[3]);

	if(detail){

		printf("%s Channel: %d\n", LOG_LVL3,
					 list_of_channels[indexes_selected_arm[0]]);
		printf("%s CCA: %.2f dBm\n", LOG_LVL3,
				ConvertPower(PW_TO_DBM, list_of_cca_values[indexes_selected_arm[1]]));
		printf("%s Tx Power: %.2f dBm\n", LOG_LVL3,
				ConvertPower(PW_TO_DBM, list_of_tx_power_values[indexes_selected_arm[2]]));
		printf("%s DCB policy: %d\n", LOG_LVL3,
				list_of_dcb_policy[indexes_selected_arm[3]]);
	}

}

/*
 * WriteConfiguration(): writes Agent info
 */
void Agent :: WriteConfiguration(Configuration configuration_to_write) {

	fprintf(agent_logger.file, "%.15f;A%d;%s;%s Configuration:\n", SimTime(), agent_id, LOG_C03, LOG_LVL2);

	fprintf(agent_logger.file, "%.15f;A%d;%s;%s selected_primary_channel = %d\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,
			configuration_to_write.selected_primary_channel);
//	fprintf(agent_logger.file, "%.15f;A%d;%s;%s selected_left_channel = %d\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,
//			configuration_to_write.selected_left_channel);
//	fprintf(agent_logger.file, "%.15f;A%d;%s;%s selected_right_channel = %d\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,
//			configuration_to_write.selected_right_channel);
	fprintf(agent_logger.file, "%.15f;A%d;%s;%s selected_cca = %f dBm\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,
			ConvertPower(PW_TO_DBM,configuration_to_write.selected_cca));
	fprintf(agent_logger.file, "%.15f;A%d;%s;%s selected_tx_power = %f dBm\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,
			ConvertPower(PW_TO_DBM,configuration_to_write.selected_tx_power));
	fprintf(agent_logger.file, "%.15f;A%d;%s;%s selected_dcb_policy = %d\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,
			configuration_to_write.selected_dcb_policy);

}


/*
 * PrintOrWriteAgentStatistics(): prints (or writes) final statistics at the given agent
 */
void Agent :: PrintOrWriteAgentStatistics(int write_or_print) {

	// Process statistics
	// ...

	switch(write_or_print){

		case PRINT_LOG:{

			if (print_agent_logs) {
				printf("------- Agent A%d (WLAN %s) ------\n", agent_id, wlan_code.c_str());
				printf("* Actions report:\n");
				// Detailed summary of arms
				for(int i = 0; i < num_actions; i++){
					// Print logs
					if (print_agent_logs) PrintAction(i, FALSE);
					//if (print_agent_logs) actions[i].PrintAction();
					if (print_agent_logs) printf("%s times_arm_has_been_selected  = %d\n", LOG_LVL3, times_arm_has_been_selected[i]);
					if (print_agent_logs) printf("%s average_reward_per_arm  = %f\n", LOG_LVL3, average_reward_per_arm[i]);
				}
				printf("\n\n");
			}
			break;
		}

		case WRITE_LOG:{

			if (save_agent_logs){

				fprintf(agent_logger.file, "******************************************\n");

				// Detailed summary of arms
				int indexes_selected_arm[NUM_FEATURES_ACTIONS];
				for(int i = 0; i < num_actions; i++){

					index2values(indexes_selected_arm, i, num_actions_channel,
							num_actions_cca, num_actions_tx_power, num_actions_dcb_policy);
					// Write logs

					fprintf(agent_logger.file, "%.15f;A%d;%s;%s Action %d:\n", SimTime(), agent_id, LOG_C03, LOG_LVL2, i);
					fprintf(agent_logger.file, "%.15f;A%d;%s;%s Channel: %d\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,list_of_channels[indexes_selected_arm[0]]);
					fprintf(agent_logger.file, "%.15f;A%d;%s;%s CCA: %f (%f dBm)\n", SimTime(), agent_id, LOG_C03, LOG_LVL3, list_of_cca_values[indexes_selected_arm[1]],
							ConvertPower(PW_TO_DBM, list_of_cca_values[indexes_selected_arm[1]]));
					fprintf(agent_logger.file, "%.15f;A%d;%s;%s Tx Power: %f (%f dBm)\n", SimTime(), agent_id, LOG_C03, LOG_LVL3, list_of_tx_power_values[indexes_selected_arm[2]],
							ConvertPower(PW_TO_DBM, list_of_tx_power_values[indexes_selected_arm[2]]));
					fprintf(agent_logger.file, "%.15f;A%d;%s;%s DCB policy: %d\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,list_of_dcb_policy[indexes_selected_arm[3]]);

					fprintf(agent_logger.file, "%.15f;A%d;%s;%s times_arm_has_been_selected = %d\n",
							SimTime(), agent_id, LOG_C03, LOG_LVL3, times_arm_has_been_selected[i]);
					fprintf(agent_logger.file, "%.15f;A%d;%s;%s average_reward_per_arm = %f\n",
							SimTime(), agent_id, LOG_C03, LOG_LVL3, average_reward_per_arm[i]);
				}
			}
			break;
		}
	}
}