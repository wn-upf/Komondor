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
 * - This file contains the methods and functionalities held by the agent,
 * including the communication with the APs of the controlled networks.
 *
 * - The agent includes functionalities held by the "Collector" (C) and the "Distributor" (D) nodes in the ML pipeline
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include <iostream>
#include <stdlib.h>

#include "../list_of_macros.h"
#include "../structures/node_configuration.h"
#include "../structures/performance_metrics.h"
#include "../structures/action.h"
#include "../methods/auxiliary_methods.h"
#include "../methods/agent_methods.h"

#include "../learning_modules/pre_processor.h"
#include "../learning_modules/ml_method.h"

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
		void InitializePreProcessor();
		void InitializeMlMethod();

		// Communication with AP
		void RequestInformationToAp();
		void ComputeNewConfiguration();
		void SendNewConfigurationToAp(Configuration &configuration_to_send);

		// Communication with other Agents (distributed methods)
		// ... To be completed

		// Print methods
		void PrintAgentInfo();
		void WriteConfiguration(Configuration configuration_to_write);
		void WritePerformance(Performance performance_to_write);
		void PrintOrWriteAgentStatistics();

	// Public items (entered by agents constructor in komondor_main)
	public:

		// Specific to each agent
		int agent_id; 			// Node identifier
		int communication_level;		// Indicates whether the node is controlled by a central entity or not
		std::string wlan_code;	// WLAN code to which the agent belongs

		// Learning mechanism
		int learning_mechanism;
		int action_selection_strategy;

		// Actions management (tunable parameters)
		int *list_of_channels; 				// List of channels
		double *list_of_pd_values;			// List of PD values
		double *list_of_tx_power_values;	// List of TX power values
		int *list_of_dcb_policy;			// List of DCB policies
		Action *actions;					// List of actions
		int num_actions_channel;			// Number of channels available
		int num_actions_sensitivity;		// Number of PD levels available
		int num_actions_tx_power;			// Number of TX power levels available
		int num_actions_dcb_policy;			// Number of DCB policies available

		// Other input parameters
		int type_of_reward;				// Type of reward
		double time_between_requests; 	// Time between two information requests to the AP (for a given measurement)

		// Print/write variables
		int save_agent_logs;
		int print_agent_logs;
		std::string simulation_code;		// Simulation code

	// Private items (just for node operation)
	private:

		//
		int num_actions;					// Number of actions (depends on the configuration parameters - pd, tx_power, channels, etc.)
		int num_requests;					// Number of requests made by the agent to the AP
		int ix_selected_arm; 				// Index of the current selected arm
		double initial_reward;				// Initial reward assigned to each arm
		double *reward_per_arm;				// Reward experienced after playing each arm
		double *average_reward_per_arm;		// Average reward experienced for each arm
		double *cumulative_reward_per_arm;	// Cumulative reward experienced for each arm
		int *times_arm_has_been_selected; 	// Number of times an arm has been played

		// Variables to store performance and configuration reports
		Performance performance;
		Configuration configuration;
		Configuration new_configuration;
		Configuration configuration_from_controller;

		// File for writting node logs
		FILE *output_log_file;				// File for logs in which the agent is involved
		char own_file_path[32];				// Name of the file for agent logs
		Logger agent_logger;				// struct containing the attributes needed for writting logs in a file
		char *header_string;				// Header string for the logger

		PreProcessor pre_processor;
		MlMethod ml_method;
//		bool not_initialized;				// Boolean to determine whether the learning alg. has been initialized or not

		int processed_configuration;
		double processed_performance;

		int ML_output;

	// Connections and timers
	public:

		// INPORT connections for receiving notifications
		inport void inline InportReceivingInformationFromAp(Configuration &configuration, Performance &performance);
		// INPORT (centralized system only)
		inport void inline InportReceivingRequestFromController(int destination_agent_id);
		inport void inline InportReceiveConfigurationFromController(int destination_agent_id, Configuration &new_configuration);
		// OUTPORT connections for sending notifications
		outport void outportRequestInformationToAp();
		outport void outportSendConfigurationToAp(Configuration &new_configuration);
		// OUTPORT (centralized system only)
		outport void outportAnswerToController(Configuration &configuration, Performance &performance, int agent_id);
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
	
	LOGS(save_agent_logs, agent_logger.file,
		"%.18f;A%d;%s;%s Start()\n", SimTime(), agent_id, LOG_B00, LOG_LVL1);

	if(communication_level == PURE_CENTRALIZED) {
		// --- Do nothing ---
		// In case of being centralized, wait for a request from the controller
		printf("%s Agent %d: Learning operation managed by the CC\n", LOG_LVL3, agent_id);
	} else {
		// Initialize the PP and the ML Method
		InitializePreProcessor();
		InitializeMlMethod();
		// Generate the first request to be triggered after "time_between_requests"
		// *** We generate here the first request in order to obtain the AP's configuration
		double extra_wait_test = 0.005 * (double) agent_id;
		trigger_request_information_to_ap.Set(fix_time_offset(SimTime() + time_between_requests + extra_wait_test,13,12));
	}

};

/*
 * Stop()
 */
void Agent :: Stop(){

	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s Agent Stop()\n", SimTime(), agent_id, LOG_C00, LOG_LVL1);

	PrintOrWriteAgentStatistics();

	// Close node logs file
	if(save_agent_logs) fclose(agent_logger.file);

};

/***************************/
/***************************/
/*  AP-AGENT COMMUNICATION */
/***************************/
/***************************/

/*
 * RequestInformationToAp(): method where an information retrieval request is performed to the AP.
 * This method is triggered by a process that decides the time between requests.
 */
void Agent :: RequestInformationToAp(trigger_t &){

//	printf("%s Agent #%d: Requesting information to AP\n", LOG_LVL1, agent_id);

	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s RequestInformationToAp() (request #%d)\n",
		SimTime(), agent_id, LOG_F00, LOG_LVL1, num_requests);

	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s Requesting information to AP\n", SimTime(), agent_id, LOG_C00, LOG_LVL2);

	outportRequestInformationToAp();

	++ num_requests;

};

/*
 * InportReceivingInformationFromAp(): called when some node (this one included) starts a TX
 * INPUT:
 * - received_configuration: configuration report that is received from to the corresponding AP
 * - received_performance: performance report that is received from to the corresponding AP
 */
void Agent :: InportReceivingInformationFromAp(Configuration &received_configuration,
		Performance &received_performance){

//	printf("%s Agent #%d: Message received from the AP\n", LOG_LVL1, agent_id);

	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s InportReceivingInformationFromAp()\n",
		SimTime(), agent_id, LOG_F00, LOG_LVL1);

	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s New information has been received from the AP\n",
		SimTime(), agent_id, LOG_C00, LOG_LVL2);

	configuration = received_configuration;
	performance = received_performance;

	if(save_agent_logs) {
		WriteConfiguration(configuration);
		WritePerformance(performance);
	}

	if (communication_level == PURE_CENTRALIZED ||  communication_level == HYBRID_CENTRALIZED_DECENTRALIZED) {
		// Forward the information to the controller
		LOGS(save_agent_logs, agent_logger.file,
			"%.15f;A%d;%s;%s Answering to the controller with current information\n",
			SimTime(), agent_id, LOG_F02, LOG_LVL2);
		outportAnswerToController(configuration, performance, agent_id);
	}

	if (communication_level != 1) {
		// Compute a new configuration according to the updated rewards
		ComputeNewConfiguration();
	}

};

/*
 * SendNewConfigurationToAp():
 * INPUT:
 * - configuration_to_send: configuration report that is sent to the corresponding AP
 */
void Agent :: SendNewConfigurationToAp(Configuration &configuration_to_send){

//	printf("%s Agent #%d: Sending new configuration to AP\n", LOG_LVL1, agent_id);

	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s SendNewConfigurationToAp()\n",
		SimTime(), agent_id, LOG_F00, LOG_LVL1);

	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s Sending a new configuration to the AP\n",
		SimTime(), agent_id, LOG_C00, LOG_LVL2);

	if(save_agent_logs) WriteConfiguration(configuration_to_send);

	// TODO (LOW PRIORITY): generate a trigger to simulate delays in the agent-node communication
	outportSendConfigurationToAp(configuration_to_send);

	// Set trigger for next request in case of being an independent agent (not controlled by a central entity)
	if (!communication_level) {
		LOGS(save_agent_logs, agent_logger.file,
			"%.15f;A%d;%s;%s Next request to be sent at %f\n",
			SimTime(), agent_id, LOG_C00, LOG_LVL2, fix_time_offset(SimTime() + time_between_requests,13,12));
		trigger_request_information_to_ap.Set(fix_time_offset(SimTime() + time_between_requests,13,12));
	} else {
		LOGS(save_agent_logs, agent_logger.file,
			"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	}

};

/***************************/
/***************************/
/*  AGENT-CC COMMUNICATION */
/***************************/
/***************************/

/*
 * InportReceivingRequestFromAgent(): called when some agent answers for information to the AP
 * INPUT:
 * - destination_agent_id: id of the agent that is requested by the central controller (CC)
 */
void Agent :: InportReceivingRequestFromController(int destination_agent_id) {

	if(agent_id == destination_agent_id) {

//		printf("%s Agent #%d: New information request received from the Controller\n", LOG_LVL1, agent_id);

		LOGS(save_agent_logs, agent_logger.file,
			"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

		LOGS(save_agent_logs, agent_logger.file,
			"%.15f;A%d;%s;%s New information request received from the Controller for Agent %d\n",
			SimTime(), agent_id, LOG_F02, LOG_LVL2, destination_agent_id);

		/* Once the CC requests to retrieve information from the AP, a trigger is set, which determines
		 * the delay experienced during the CC-Agent and the Agent-AP communication
		 */
		trigger_request_information_to_ap.Set(fix_time_offset(SimTime(),13,12));

	}

}

/*
 * InportReceiveConfigurationFromAgent(): called when some agent sends instructions to the AP
 * INPUT:
 * - destination_agent_id: id of the agent to which the information is delivered
 * - received_configuration: reference of the configuration received from the controller
 */
void Agent :: InportReceiveConfigurationFromController(int destination_agent_id,
		Configuration &received_configuration) {

	if(agent_id == destination_agent_id) {

		LOGS(save_agent_logs, agent_logger.file,
			"%.15f;A%d;%s;%s New configuration received from the Controller to Agent %d\n",
			SimTime(), agent_id, LOG_F02, LOG_LVL2, destination_agent_id);

		// Update the received configuration
		configuration_from_controller = received_configuration;
		// Forward the configuration to the AP
		SendNewConfigurationToAp(configuration_from_controller);

	}

}

/***************************/
/***************************/
/*  GENERATE A NEW CONFIG. */
/***************************/
/***************************/

/*
 * ComputeNewConfiguration(): computes a new configuration to be sent to the AP.
 * Different mechanisms can be used, which are expected to return the new configuration.
 */
void Agent :: ComputeNewConfiguration(){

	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s ComputeNewConfiguration()\n", SimTime(), agent_id, LOG_F00, LOG_LVL1);

	// Process the configuration and performance reports obtained from the WLAN
	processed_configuration = pre_processor.ProcessWlanConfiguration(MULTI_ARMED_BANDITS, configuration);
	processed_performance = pre_processor.ProcessWlanPerformance(MULTI_ARMED_BANDITS, performance, type_of_reward);

	// Update the configuration according to the selected learning method
	switch(learning_mechanism) {

		/* Multi-Armed Bandits */
		case MULTI_ARMED_BANDITS:{
			// Update the configuration according to the MABs operation
			ML_output = ml_method.ComputeIndividualConfiguration
				(processed_configuration, processed_performance, agent_logger, SimTime());
			break;
		}
		default:{
			printf("[AGENT] ERROR: %d is not a correct learning mechanism\n", learning_mechanism);
			ml_method.PrintAvailableLearningMechanisms();
			exit(EXIT_FAILURE);
			break;
		}
	}

	// Process the configuration from the output of the ML method
	new_configuration = pre_processor.ProcessMLOutput(learning_mechanism, configuration, ML_output);

	// Send the configuration to the AP
	SendNewConfigurationToAp(new_configuration);

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
	list_of_pd_values = new double[num_actions_sensitivity];
	list_of_tx_power_values = new double[num_actions_tx_power];
	list_of_dcb_policy = new int[num_actions_dcb_policy];

	num_actions = num_actions_channel * num_actions_sensitivity * num_actions_tx_power * num_actions_dcb_policy;

	// Generate actions
	actions = new Action[num_actions];

//	// Specify that the learning mechanism has not been initialized (to be done for the first AP-agent interaction)
//	not_initialized = true;

}

/*
 * InitializeVariables(): initializes all the necessary variables
 */
void Agent :: InitializePreProcessor() {

	pre_processor.type_of_reward = type_of_reward;

	pre_processor.num_actions = num_actions;
	pre_processor.num_actions_channel = num_actions_channel;
	pre_processor.num_actions_sensitivity = num_actions_sensitivity;
	pre_processor.num_actions_tx_power = num_actions_tx_power;
	pre_processor.num_actions_dcb_policy = num_actions_dcb_policy;

	pre_processor.InitializeVariables();

	pre_processor.list_of_channels = list_of_channels;
	pre_processor.list_of_pd_values = list_of_pd_values;
	pre_processor.list_of_tx_power_values = list_of_tx_power_values;
	pre_processor.list_of_dcb_policy = list_of_dcb_policy;

}

/*
 * InitializeVariables(): initializes all the necessary variables
 */
void Agent :: InitializeMlMethod() {

	ml_method.agent_id = agent_id;
	ml_method.learning_mechanism = learning_mechanism;
	ml_method.num_channels = num_actions_channel;

	ml_method.save_agent_logs = save_agent_logs;
	ml_method.print_agent_logs = print_agent_logs;

	ml_method.action_selection_strategy = action_selection_strategy;
	ml_method.num_actions = num_actions;

	ml_method.InitializeVariables();

}

/******************************/
/******************************/
/*  PRINT/WRITE INFORMATION   */
/******************************/
/******************************/

/*
 * PrintAgentInfo(): prints Agent info
 */
void Agent :: PrintAgentInfo(){

	printf("%s Agent %d info:\n", LOG_LVL3, agent_id);
	printf("%s wlan_code = %s\n", LOG_LVL4, wlan_code.c_str());
	printf("%s communication_level = %d\n", LOG_LVL4, communication_level);
	printf("%s time_between_requests = %f\n", LOG_LVL4, time_between_requests);
	printf("%s type_of_reward = %d\n", LOG_LVL4, type_of_reward);
	printf("%s initial_reward = %f\n", LOG_LVL4, initial_reward);
	printf("%s list_of_channels: ", LOG_LVL4);
	for (int i = 0; i < num_actions_channel; ++i) {
		printf("%d  ", list_of_channels[i]);
	}
	printf("\n");

	printf("%s list_of_pd_values: ", LOG_LVL4);
	for (int i = 0; i < num_actions_sensitivity; ++i) {
		printf("%f pW (%f dBm)  ", list_of_pd_values[i], ConvertPower(PW_TO_DBM, list_of_pd_values[i]));
	}
	printf("\n");

	printf("%s list_of_tx_power_values: ", LOG_LVL4);
	for (int i = 0; i < num_actions_tx_power; ++i) {
		printf("%f pW (%f dBm)  ", list_of_tx_power_values[i], ConvertPower(PW_TO_DBM, list_of_tx_power_values[i]));
	}
	printf("\n");

	printf("%s list_of_dcb_policy: ", LOG_LVL4);
	for (int i = 0; i < num_actions_dcb_policy; ++i) {
		printf("%d  ", list_of_dcb_policy[i]);
	}
	printf("\n");

	printf("%s learning_mechanism: %d\n", LOG_LVL4, learning_mechanism);
	printf("%s action_selection_strategy: %d\n", LOG_LVL4, action_selection_strategy);

	printf("%s save_agent_logs: %d\n", LOG_LVL4, save_agent_logs);
	printf("%s print_agent_logs: %d\n", LOG_LVL4, print_agent_logs);

	printf("\n");

}

/*
 * WriteConfiguration(): writes Agent info
 */
void Agent :: WriteConfiguration(Configuration configuration_to_write) {
	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s Configuration:\n", SimTime(), agent_id, LOG_C03, LOG_LVL2);
	// Selected primary channel
	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s selected_primary_channel = %d\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,
		configuration_to_write.selected_primary_channel);
	// Select Packet Detect (PD) threshold
	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s selected_pd = %f dBm\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,
		ConvertPower(PW_TO_DBM,configuration_to_write.selected_pd));
	// Selected Transmit Power
	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s selected_tx_power = %f dBm\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,
		ConvertPower(PW_TO_DBM,configuration_to_write.selected_tx_power));
	// Selected DCB policy
	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s selected_dcb_policy = %d\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,
		configuration_to_write.selected_dcb_policy);
}

/*
 * WritePerformance(): writes performance
 */
void Agent :: WritePerformance(Performance performance_to_write) {
	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s Performance:\n", SimTime(), agent_id, LOG_C03, LOG_LVL2);
	// Throughput (Mbps)
	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s throughput = %.2f\n", SimTime(), agent_id, LOG_C03, LOG_LVL3,
		performance_to_write.throughput * pow(10,-6));
}


/*
 * PrintAgentInfo(): prints Agent info
 */
void Agent :: PrintOrWriteAgentStatistics(){

	printf("\n------- Agent A%d ------\n", agent_id);
	ml_method.PrintOrWriteStatistics(PRINT_LOG, agent_logger, SimTime());
	printf("\n");
//	ml_method.PrintOrWriteStatistics(WRITE_LOG, agent_logger, SimTime());

}


