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
#include "../structures/performance_metrics.h"
#include "../methods/auxiliary_methods.h"
#include "../methods/agent_methods.h"
#include "../learning_modules/graph_coloring/graph_coloring.h"

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
		void RequestInformationToAgent();
		void SendNewConfigurationToAgent(int destination_agent_id);

		void InitializeLearningAlgorithm();
		void ComputeNewConfiguration();
		void SendConfigurationToAllAgents();
		void SendConfigurationToSingleAgent(int destination_agent_id, Configuration conf);

		// Print methods
		void PrintCentralControllerInfo();


	// Public items (entered by agents constructor in komondor_main)
	public:

		int agents_number;
		int *list_of_agents;

		int wlans_number;

		int *num_requests;
		double time_between_requests;

		int save_controller_logs;
		int print_controller_logs;

		int type_of_reward;
		int learning_mechanism;
		int action_selection_strategy;

		int num_channels;

		int total_nodes_number;

	// Private items (just for node operation)
	private:

		Configuration configuration;
		Configuration new_configuration;

		Performance performance;

		Configuration *configuration_array;
		Performance *performance_array;

		// File for writting node logs
		FILE *output_log_file;				// File for logs in which the agent is involved
		char own_file_path[32];				// Name of the file for agent logs
		Logger central_controller_logger;	// struct containing the attributes needed for writting logs in a file
		char *header_string;				// Header string for the logger

		int counter_responses_received; 	// Needed to determine the number of answers that the controller receives from agents

		GraphColoring graph_coloring;

		bool initialization_flag;

	// Connections and timers
	public:

		// INPORT connections for receiving notifications
		inport void inline InportReceivingInformationFromAgent(Configuration &configuration,
				Performance &performance, int agent_id);

		// OUTPORT connections for sending notifications
		outport void outportRequestInformationToAgent(int destination_agent_id);
		outport void outportSendConfigurationToAgent(int destination_agent_id, Configuration &new_configuration);

		// Triggers
		Timer <trigger_t> trigger_request_information_to_agents; // Timer for requesting information to the AP
		Timer <trigger_t> trigger_safe_responses_collection;

		// Every time the timer expires execute this
		inport inline void RequestInformationToAgent(trigger_t& t1);
		inport inline void ComputeNewConfiguration(trigger_t& t1);

		// Connect timers to methods
		CentralController () {
			connect trigger_request_information_to_agents.to_component,RequestInformationToAgent;
			connect trigger_safe_responses_collection.to_component,ComputeNewConfiguration;
		}

};

/*
 * Setup()
 */
void CentralController :: Setup(){
	// Do nothing
};

/*
 * Start()
 */
void CentralController :: Start(){

	// Create agent logs file if required
	if(save_controller_logs) {
		sprintf(own_file_path,"%s_CENTRAL_CONTROLLER.txt","../output/logs_output");
		remove(own_file_path);
		output_log_file = fopen(own_file_path, "at");
		central_controller_logger.save_logs = save_controller_logs;
		central_controller_logger.file = output_log_file;
		central_controller_logger.SetVoidHeadString();
	}

	if(save_controller_logs) fprintf(central_controller_logger.file,
		"%.18f;CC;%s;%s Start()\n", SimTime(), LOG_B00, LOG_LVL1);

	// Initialize learning algorithm in the CC
	InitializeLearningAlgorithm();

	if(learning_mechanism == GRAPH_COLORING) {
		// Generate the request for initialization at the beginning (no need to collect performance data)
		trigger_request_information_to_agents.Set(fix_time_offset(SimTime() + 0.001,13,12));
	} else {
		// Generate the first request, to be triggered after "time_between_requests"
		trigger_request_information_to_agents.Set(fix_time_offset(SimTime() + time_between_requests,13,12));
	}

};

/*
 * Stop()
 */
void CentralController :: Stop() {

	if(save_controller_logs) fprintf(central_controller_logger.file,
		"%.15f;CC;%s;%s Central Controller Stop()\n", SimTime(), LOG_C00, LOG_LVL1);

	// Print and write node statistics if required
//	PrintOrWriteAgentStatistics(PRINT_LOG);
//	PrintOrWriteAgentStatistics(WRITE_LOG);

	// Close node logs file
	if(save_controller_logs) fclose(central_controller_logger.file);

};

/**************************/
/**************************/
/*  COMMUNICATION METHODS */
/**************************/
/**************************/

/*
 * RequestInformationToAgent():
 * Input arguments:
 * - to be defined
 */
void CentralController :: RequestInformationToAgent(trigger_t &){

	if(save_controller_logs) fprintf(central_controller_logger.file,"%.15f;CC;%s;%s "
		"Requesting information to Agents\n", SimTime(), LOG_C00, LOG_LVL1);

	for (int ix = 0 ; ix < agents_number ; ++ix ) {

		if(save_controller_logs) fprintf(central_controller_logger.file,
			"%.15f;CC;%s;%s Requesting information to Agent %d\n",
			SimTime(), LOG_C00, LOG_LVL2, ix);

		outportRequestInformationToAgent(ix);

		++ num_requests[ix] ;

	}

};

/*
 * InportReceivingInformationFromAgent(): called when some node (this one included) starts a TX
 * Input arguments:
 * - to be defined
 */
void CentralController :: InportReceivingInformationFromAgent(Configuration &received_configuration,
	Performance &received_performance, int agent_id){

	if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s InportReceivingInformationFromAgent()\n",
		SimTime(), LOG_F00, LOG_LVL1);

	if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s New information has been received from Agent %d\n",
		SimTime(), LOG_C00, LOG_LVL2, agent_id);

	// Update the configuration and performance received
	configuration_array[agent_id] = received_configuration;
	performance_array[agent_id] = received_performance;

	// Print configuration and performance report
	if(save_controller_logs) {
		configuration_array[agent_id].WriteConfiguration(central_controller_logger, SimTime());
		configuration_array[agent_id].capabilities.WriteCapabilities(central_controller_logger, SimTime());
	}

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
			trigger_request_information_to_agents.Set(fix_time_offset(SimTime() + time_between_requests,13,12));
		} else {
			trigger_safe_responses_collection.Set(fix_time_offset(SimTime(),13,12));
//			ComputeNewConfiguration();
		}
		counter_responses_received = 0;
	}

};

/*
 * ComputeNewConfiguration():
 * Input arguments:
 * -
 */
void CentralController :: ComputeNewConfiguration(trigger_t &){
//	printf("%s Central Controller: Computing a new configuration\n", LOG_LVL1);
	if(save_controller_logs) fprintf(central_controller_logger.file,
		"%.15f;CC;%s;%s ComputeNewConfiguration()\n",
		SimTime(), LOG_F00, LOG_LVL1);
	if(save_controller_logs) fprintf(central_controller_logger.file,
		"%.15f;CC;%s;%s Computing a new configuration\n",
		SimTime(), LOG_C00, LOG_LVL2);
	// Apply Hminmax to decide the new channels configuration
	graph_coloring.UpdateConfiguration(configuration_array, performance_array,
		central_controller_logger, SimTime());
	// Send the configuration to the AP
	SendConfigurationToAllAgents();
	// Set trigger for next request
	if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s Next request to be sent at %f\n",
		SimTime(), LOG_C00, LOG_LVL2, fix_time_offset(SimTime() + time_between_requests,13,12));
	trigger_request_information_to_agents.Set(fix_time_offset(SimTime() + time_between_requests,13,12));
}

/*
 * SendConfigurationToAllAgents():
 * Input arguments:
 * - to be defined
 */
void CentralController :: SendConfigurationToAllAgents(){
	for (int ix = 0 ; ix < agents_number ; ++ ix ) {
		SendConfigurationToSingleAgent(ix, configuration_array[ix]);
	}
}

/*
 * SendConfigurationToSingleAgent():
 * Input arguments:
 * - to be defined
 */
void CentralController :: SendConfigurationToSingleAgent(int destination_agent_id, Configuration new_conf){
//	printf("%s Central Controller: Sending new configuration to Agent%d\n", LOG_LVL1, destination_agent_id);
	if(save_controller_logs) fprintf(central_controller_logger.file,
		"%.15f;CC;%s;%s SendNewConfigurationToAp()\n", SimTime(), LOG_F00, LOG_LVL1);
	if(save_controller_logs) fprintf(central_controller_logger.file,
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

/*
 * InitializeCentralController(): initializes all the necessary variables
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

/*
 * InitializeLearningAlgorithm(): initializes all the necessary variables of the chosen learning alg.
 */
void CentralController :: InitializeLearningAlgorithm() {

	switch(learning_mechanism) {

		/* GRAPH COLORING */
		case GRAPH_COLORING:{
			initialization_flag = true;
			// Initialize the graph coloring method
			graph_coloring.save_controller_logs = save_controller_logs;
			graph_coloring.print_controller_logs = print_controller_logs;
			graph_coloring.agents_number = agents_number;
			graph_coloring.wlans_number = wlans_number;
			graph_coloring.num_channels = num_channels;
			graph_coloring.total_nodes_number = total_nodes_number;
			// Initialize variables characteristic to the graph coloring method
			graph_coloring.InitializeVariables();
			break;
		}

		default:{
			printf("[CC] ERROR: %d is not a correct learning mechanism\n", learning_mechanism);
			exit(EXIT_FAILURE);
			break;
		}
	}
}

/************************/
/************************/
/*  PRINT INFORMATION   */
/************************/
/************************/

/*
 * PrintCentralControllerInfo(): prints Central Controller's info
 */
void CentralController :: PrintCentralControllerInfo(){

	printf("%s Central Controller info:\n", LOG_LVL3);
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
