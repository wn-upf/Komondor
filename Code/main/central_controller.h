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
 * central_controller.h: this file defines the central controller component
 *
 *  - This file contains the methods and functionalities held by the Central Controller (CC),
 * including the communication with the agents attached to it.
 *
 *  - The CC includes functionalities of the "Collector" (C) and the "Distributor" (D) nodes in the ML pipeline.
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include <iostream>
#include <stdlib.h>

#include "../list_of_macros.h"

#include "../structures/node_configuration.h"
#include "../structures/performance_metrics.h"

#include "../methods/auxiliary_methods.h"
#include "../methods/agent_methods.h"

#include "../network_optimization/channel_assignment/centralized_graph_coloring.h"

#include "../learning_modules/pre_processor.h"
#include "../learning_modules/ml_model.h"

// Agent component: "TypeII" represents components that are aware of the existence of the simulated time.
component CentralController : public TypeII{

	// Methods
	public:

		// COST
		void Setup();
		void Start();
		void Stop();

		// Generic
		void InitializeCentralController();

		// Communication with AP
		void RequestInformationToAgents();

		void GenerateAndSendNewConfiguration();
		void SendConfigurationToAllAgents();
		void SendConfigurationToSingleAgent(int destination_agent_id, Configuration conf);

		// Print methods
		void PrintControllerInfo();
		void WriteControllerInfo(Logger logger);
		void PrintOrWriteControllerStatistics(int print_or_write);

		// Initialization
		void InitializePreProcessor();
		void InitializeMlModel();


	// Public items (entered by agents constructor in komondor_main)
	public:

		int agents_number;			///> Number of agents controlled by the CC
		int *list_of_agents;		///> List of the identifiers of the agents controlled by the CC
		int wlans_number;			///> Number of WLANs

		int *num_requests;				///> List of the number of requests performed to each agent
		double time_between_requests;	///> Time between requests

		int save_controller_logs;		///> Boolean that indicates whether to write CC's logs or not
		int print_controller_logs;		///> Boolean that indicates whether to print CC's logs or not

		int controller_mode; 			///> The CC can be either passive (0) or active (1)

		int type_of_reward;				///> Type of reward
		int learning_mechanism;			///> Index of the chosen learning mechanism
		int action_selection_strategy;	///> Index of the chosen action-selection strategy

		int num_channels;				///> Number of channels
		int total_nodes_number;			///> Number of nodes

		int controller_on;				///> Flag indicating whether the CC is active or not

	// Private items (just for node operation)
	private:

		Configuration configuration;		///> Configuration object
		Configuration new_configuration;	///> Auxiliary configuration object

		Performance performance;			///> Performance object

		Configuration *configuration_array;	///> Array of configuration objects from all the agents
		Performance *performance_array;		///> Array of performance objects from all the agents

		MlModel ml_model;					///> Instantiation of the ML Model
		PreProcessor pre_processor;			///> Instantiation of the Pre-Processor

		// File for writting node logs
		FILE *output_log_file;				///> File for logs in which the agent is involved
		char own_file_path[32];				///> Name of the file for agent logs
		Logger central_controller_logger;	///> struct containing the attributes needed for writing logs in a file
		char *header_string;				///> Header string for the logger

		int counter_responses_received; 	///> Needed to determine the number of answers that the controller receives from agents

		GraphColoring graph_coloring;		///> Instantiation of the Graph coloring approach

		bool initialization_flag;			///> Boolean indicating whether initialization has been performed or not

	// Connections and timers
	public:

		// INPORT connections for receiving notifications
		inport void inline InportReceivingInformationFromAgent(Configuration &configuration,
			Performance &performance, int agent_id);

		// OUTPORT connections for sending notifications
		outport void outportRequestInformationToAgent(int destination_agent_id);
		outport void outportSendConfigurationToAgent(int destination_agent_id,
			Configuration &new_configuration);

		// Triggers
		Timer <trigger_t> trigger_request_information_to_agents; // Timer for requesting information to the AP
		Timer <trigger_t> trigger_safe_responses_collection;

		// Every time the timer expires execute this
		inport inline void RequestInformationToAgents(trigger_t& t1);
		inport inline void GenerateAndSendNewConfiguration(trigger_t& t1);

		// Connect timers to methods
		CentralController () {
			connect trigger_request_information_to_agents.to_component,RequestInformationToAgents;
			connect trigger_safe_responses_collection.to_component,GenerateAndSendNewConfiguration;
		}

};

/**
 * Setup()
 */
void CentralController :: Setup(){
	// Do nothing
};

/**
 * Start()
 */
void CentralController :: Start(){

	if (controller_on) {
		// Create CC logs file (if required)
		if(save_controller_logs) {
			sprintf(own_file_path,"%s_CENTRAL_CONTROLLER.txt","../output/logs_output");
			remove(own_file_path);
			output_log_file = fopen(own_file_path, "at");
			central_controller_logger.save_logs = save_controller_logs;
			central_controller_logger.file = output_log_file;
			central_controller_logger.SetVoidHeadString();
		}
		LOGS(save_controller_logs,central_controller_logger.file,
			"%.18f;CC;%s;%s Start()\n", SimTime(), LOG_B00, LOG_LVL1);
		// Initialize the PP and the ML Method
		InitializePreProcessor();
		InitializeMlModel();
		// Hardcoded [TODO: introduce this parameter from the input]
		controller_mode = MODE_ACTIVE;
		// According to the defined mode, start the learning operation by activating triggers
		switch(controller_mode) {
			// Passive mode: the agents send information to the CC
			case MODE_PASSIVE: {
				break;
			}
			// Active mode: the CC requests information to agents
			case MODE_ACTIVE: {
				if(learning_mechanism == GRAPH_COLORING) {
					// Generate the request for initialization at the beginning (no need to collect performance data)
					trigger_request_information_to_agents.Set(FixTimeOffset(SimTime() + 0.001,13,12));
				} else {
					// Generate the first request, to be triggered after "time_between_requests"
					trigger_request_information_to_agents.Set(FixTimeOffset(SimTime() + time_between_requests, 13, 12));
				}
				break;
			}
			// Unknown controller mode
			default:{
				printf("[CC] ERROR: Undefined controller mode %d\n"
					"	- Use MODE_PASSIVE (%d) or MODE_ACTIVE (%d)\n",
					controller_mode, MODE_PASSIVE, MODE_ACTIVE);
			}
		}
	} else {
//		printf("The central controller is NOT active\n");
	}

};

/**
 * Stop()
 */
void CentralController :: Stop() {

	if (controller_on) {
		LOGS(save_controller_logs,central_controller_logger.file,
			"%.15f;CC;%s;%s Central Controller Stop()\n", SimTime(), LOG_C00, LOG_LVL1);
		// Print and write node statistics
		PrintOrWriteControllerStatistics(PRINT_LOG);
		PrintOrWriteControllerStatistics(WRITE_LOG);
		// Close node logs file
		if(save_controller_logs) fclose(central_controller_logger.file);
	}

};

/**************************/
/**************************/
/*  COMMUNICATION METHODS */
/**************************/
/**************************/

/**
 * Request information (configuration and performance) to agents upon trigger-based activation
 */
void CentralController :: RequestInformationToAgents(trigger_t &){

	LOGS(save_controller_logs,central_controller_logger.file,
		"%.15f;CC;%s;%s Requesting information to Agents\n", SimTime(), LOG_C00, LOG_LVL1);
	// Request information to every agent associated to the CC
	for (int ix = 0 ; ix < agents_number ; ++ix ) {
		LOGS(save_controller_logs,central_controller_logger.file,
			"%.15f;CC;%s;%s Requesting information to Agent %d\n", SimTime(), LOG_C00, LOG_LVL2, ix);
		outportRequestInformationToAgent(ix);
		++ num_requests[ix] ;
	}

};

/**
 * Called when the CC receives information from an agent
 * @param "received_configuration" [type Configuration]: configuration of the agent
 * @param "received_performance" [type Performance]: performance of the agent
 * @param "agent_id" [type int]: identifier of the agent
 */
void CentralController :: InportReceivingInformationFromAgent(Configuration &received_configuration,
	Performance &received_performance, int agent_id){

	LOGS(save_controller_logs,central_controller_logger.file,
		"%.15f;CC;%s;%s InportReceivingInformationFromAgent()\n", SimTime(), LOG_F00, LOG_LVL1);
	LOGS(save_controller_logs,central_controller_logger.file,
		"%.15f;CC;%s;%s New information has been received from Agent %d\n", SimTime(), LOG_C00, LOG_LVL2, agent_id);

	// Update the configuration and performance received
	configuration_array[agent_id] = received_configuration;
	performance_array[agent_id] = received_performance;

	// Print configuration and performance report
	if(save_controller_logs) {
		configuration_array[agent_id].WriteConfiguration(central_controller_logger, SimTime());
		configuration_array[agent_id].capabilities.WriteCapabilities(central_controller_logger, SimTime());
	}

	// According to the defined mode, behave in one way or another
	switch(controller_mode) {
		// Passive mode: the agents send information to the CC
		case MODE_PASSIVE: {
			break;
		}
		// Active mode: the CC requests information to agents
		case MODE_ACTIVE: {
			// Update the number of responses received
			++ counter_responses_received ;
			// Once all the information is available, compute a new configuration according to the updated rewards
			if (counter_responses_received == agents_number) {
				if(initialization_flag) {
					// Initialization phase of the graph coloring method
					graph_coloring.GraphColoringInitialization(configuration_array);
					// Send the initial configuration to all the associated agents
					SendConfigurationToAllAgents();
					initialization_flag = false;
					trigger_request_information_to_agents.Set(FixTimeOffset(SimTime() + time_between_requests,13,12));
				} else {
					trigger_safe_responses_collection.Set(FixTimeOffset(SimTime(),13,12));
				}
				counter_responses_received = 0;
			}
			break;
		}
		// Unknown controller mode
		default:{
			printf("[CC] ERROR: Undefined controller mode %d\n"
				"	- Use MODE_PASSIVE (%d) or MODE_ACTIVE (%d)\n",
				controller_mode, MODE_PASSIVE, MODE_ACTIVE);
		}
	}

};

/**
 * Generate a new configuration and send it to agents. Activated by a trigger.
 */
void CentralController :: GenerateAndSendNewConfiguration(trigger_t &){
	LOGS(save_controller_logs,central_controller_logger.file,
		"%.15f;CC;%s;%s GenerateAndSendNewConfiguration()\n", SimTime(), LOG_F00, LOG_LVL1);
	// Compute the new configuration according to the ML method used
	ml_model.ComputeGlobalConfiguration(configuration_array, performance_array,
		central_controller_logger, SimTime());
	// Send the configuration to the AP
	SendConfigurationToAllAgents();
	// Set trigger for next request
	trigger_request_information_to_agents.Set(FixTimeOffset(SimTime() + time_between_requests,13,12));
	LOGS(save_controller_logs,central_controller_logger.file,
		"%.15f;CC;%s;%s Next request to be sent at %f\n",
		SimTime(), LOG_C00, LOG_LVL2, FixTimeOffset(SimTime() + time_between_requests,13,12));
}

/**
 * Send the configuration to all the agents
 */
void CentralController :: SendConfigurationToAllAgents(){
	// Iterate for all the agents attached to the CC
	for (int ix = 0 ; ix < agents_number ; ++ ix ) {
		SendConfigurationToSingleAgent(ix, configuration_array[ix]);
	}
}

/**
 * Send a new configuration to a specific agent attached to the CC
 * @param "destination_agent_id" [type int]: identifier of the destination agent
 * @param "new_conf" [type Configuration]: new configuration to be applied by the destination agent
 */
void CentralController :: SendConfigurationToSingleAgent(int destination_agent_id, Configuration new_conf){
	LOGS(save_controller_logs,central_controller_logger.file,
		"%.15f;CC;%s;%s Sending a new configuration to Agent %d\n",
		SimTime(), LOG_C00, LOG_LVL2, destination_agent_id);
	// TODO (LOW PRIORITY): generate a trigger to simulate delays in the agent-node communication
	outportSendConfigurationToAgent(destination_agent_id, new_conf);
};

/******************************/
/******************************/
/*  VARIABLES INITIALIZATION  */
/******************************/
/******************************/

/**
 * Initialize the Pre-Processor (PP)
 */
void CentralController :: InitializePreProcessor() {
	pre_processor.type_of_reward = type_of_reward;
	pre_processor.InitializeVariables();
}

/**
 * Initialize the ML Model
 */
void CentralController :: InitializeMlModel() {
	ml_model.learning_mechanism = learning_mechanism;
	ml_model.num_channels = num_channels;
	ml_model.save_controller_logs = save_controller_logs;
	ml_model.print_controller_logs = print_controller_logs;
	ml_model.action_selection_strategy = action_selection_strategy;
	ml_model.InitializeVariables();
}


/**
 * Initialize the Central Controller
 */
void CentralController :: InitializeCentralController() {

	counter_responses_received = 0;
	num_requests = new int[agents_number];
	configuration_array = new Configuration[agents_number];
	performance_array  = new Performance[agents_number];

	for(int i = 0; i < agents_number; ++i){
		num_requests[i] = 0;
//		performance_array[i].SetSizeOfRssiList(agents_number);
	}

	save_controller_logs = TRUE;
	print_controller_logs = TRUE;

}

/************************/
/************************/
/*  PRINT INFORMATION   */
/************************/
/************************/

/**
 * Print CC's information
 */
void CentralController :: PrintControllerInfo() {
	printf("%s Central Controller info:\n", LOG_LVL3);
	printf("%s controller_on = %d\n", LOG_LVL4, controller_on);
	printf("%s agents_number = %d\n", LOG_LVL4, agents_number);
	printf("%s time_between_requests = %f\n", LOG_LVL4, time_between_requests);
	printf("%s learning_mechanism = %d\n", LOG_LVL4, learning_mechanism);
	printf("%s total_nodes_number = %d\n", LOG_LVL4, total_nodes_number);
	printf("%s list of agents: ", LOG_LVL4);
	for (int i = 0; i < agents_number; ++ i) {
		printf("%d ", list_of_agents[i]);
	}
	printf("\n");
}

/**
 * Write CC's information
 * @param "logger" [type Logger]: logger object for writing information into a file
 */
void CentralController :: WriteControllerInfo(Logger logger) {
	LOGS(save_controller_logs, logger.file,
		"%.15f;CC;%s;%s Central Controller info\n", SimTime(), LOG_C00, LOG_LVL3);
	LOGS(save_controller_logs, logger.file,
		"%.15f;CC;%s;%s controller_on = %d\n", SimTime(), LOG_C00, LOG_LVL4, controller_on);
	LOGS(save_controller_logs, logger.file,
		"%.15f;CC;%s;%s agents_number = %d\n", SimTime(), LOG_C00, LOG_LVL4, agents_number);
	LOGS(save_controller_logs, logger.file,
		"%.15f;CC;%s;%s time_between_requests = %f\n", SimTime(), LOG_C00, LOG_LVL4, time_between_requests);
	LOGS(save_controller_logs, logger.file,
		"%.15f;CC;%s;%s learning_mechanism = %d\n", SimTime(), LOG_C00, LOG_LVL4, learning_mechanism);
	LOGS(save_controller_logs, logger.file,
		"%.15f;CC;%s;%s total_nodes_number = %d\n", SimTime(), LOG_C00, LOG_LVL4, total_nodes_number);
	LOGS(save_controller_logs, logger.file,
		"%.15f;CC;%s;%s list of agents: ", SimTime(), LOG_C00, LOG_LVL4);
	for (int i = 0; i < agents_number; ++ i) {
		LOGS(save_controller_logs, logger.file, "%d ", list_of_agents[i]);
	}
	LOGS(save_controller_logs, logger.file, "\n");
}

/**
 * Print or write CC's statistics
 * @param "print_or_write" [type int]: boolean indicating whether to print by console or write logs into a file
 */
void CentralController :: PrintOrWriteControllerStatistics(int print_or_write) {
	switch(print_or_write){
		case PRINT_LOG:{
			printf("\n STATISTICS CENTRAL CONTROLLER:\n");
			ml_model.PrintOrWriteStatistics(PRINT_LOG, central_controller_logger, SimTime());
			break;
		}
		case WRITE_LOG:{
			LOGS(save_controller_logs, central_controller_logger.file,
				"\n%.15f;CC;%s;%s STATISTICS CENTRAL CONTROLLER:\n", SimTime(), LOG_C00, LOG_LVL1);
			ml_model.PrintOrWriteStatistics(WRITE_LOG, central_controller_logger, SimTime());
			break;
		}
	}
}
