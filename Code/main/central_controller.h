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
#include "../methods/auxiliary_methods.h"
#include "../methods/agent_methods.h"

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
		void ComputeNewConfiguration();
		void SendNewConfigurationToAgent(int destination_agent_id);

		// Print methods
		void PrintCentralControllerInfo();


	// Public items (entered by agents constructor in komondor_main)
	public:

		int agents_number;
		int *list_of_agents;

		int *num_requests;
		double time_between_requests;

		int save_controller_logs;
		int print_controller_logs;

	// Private items (just for node operation)
	private:

		Configuration configuration;
		Configuration new_configuration;

		Report report;

		// File for writting node logs
		FILE *output_log_file;				// File for logs in which the agent is involved
		char own_file_path[32];				// Name of the file for agent logs
		Logger central_controller_logger;	// struct containing the attributes needed for writting logs in a file
		char *header_string;				// Header string for the logger

		int counter_responses_received; 	// Needed to determine the number of answers that the controller receives from agents

	// Connections and timers
	public:

		// INPORT connections for receiving notifications
		inport void inline InportReceivingInformationFromAgent(Configuration &configuration, Report &report, int agent_id);

		// OUTPORT connections for sending notifications
		outport void outportRequestInformationToAgent(int destination_agent_id);
		outport void outportSendConfigurationToAgent(int destination_agent_id, Configuration &new_configuration);

		// Triggers
		Timer <trigger_t> trigger_request_information_to_agents; // Timer for requesting information to the AP

		// Every time the timer expires execute this
		inport inline void RequestInformationToAgent(trigger_t& t1);

		// Connect timers to methods
		CentralController () {
			connect trigger_request_information_to_agents.to_component,RequestInformationToAgent;
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

	if(save_controller_logs) fprintf(central_controller_logger.file,"%.18f;CC;%s;%s Start()\n",
			SimTime(), LOG_B00, LOG_LVL1);

	// Generate the first request, to be triggered after "time_between_requests"
	trigger_request_information_to_agents.Set(fix_time_offset(SimTime() + time_between_requests,13,12));

};

/*
 * Stop()
 */
void CentralController :: Stop(){

	if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s Central Controller Stop()\n",
			SimTime(), LOG_C00, LOG_LVL1);

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

	for (int agents_ix = 0 ; agents_ix < agents_number ; agents_ix ++) {

		//printf("%s Central Controller: Requesting information to Agent %d\n", LOG_LVL1, list_of_agents[agents_ix]);
		if(save_controller_logs) fprintf(central_controller_logger.file, "----------------------------------------------------------------\n");

		if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s RequestInformationToAgent() %d\n",
				SimTime(), LOG_F00, LOG_LVL1, num_requests[agents_ix]);

		if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s Requesting information to Agent %d\n",
				SimTime(), LOG_C00, LOG_LVL2, list_of_agents[agents_ix]);

		outportRequestInformationToAgent(list_of_agents[agents_ix]);

		num_requests[agents_ix] ++;

	}

};

/*
 * InportReceivingInformationFromAgent(): called when some node (this one included) starts a TX
 * Input arguments:
 * - to be defined
 */
void CentralController :: InportReceivingInformationFromAgent(Configuration &received_configuration, Report &received_report, int agent_id){

//	printf("%s Agent #%d: Message received from the AP\n", LOG_LVL1, agent_id);

	if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s InportReceivingInformationFromAgent()\n",
			SimTime(), LOG_F00, LOG_LVL1);

	if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s New information has been received from Agent %d\n",
			SimTime(), LOG_C00, LOG_LVL2, agent_id);

	configuration = received_configuration;

	//if(save_controller_logs) WriteConfiguration(configuration);

	report = received_report;

	counter_responses_received ++;

	// Generate the reward for the last selected action
	//GenerateRewardSelectedArm();

	// Once all the information is available, compute a new configuration according to the updated rewards
	if (counter_responses_received == agents_number) {
		ComputeNewConfiguration();
		counter_responses_received = 0;
	}

};

/*
 * ComputeNewConfiguration():
 * Input arguments:
 * -
 */
void CentralController :: ComputeNewConfiguration(){

	//printf("%s Central Controller: Computing a new configuration\n", LOG_LVL1);

	if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s ComputeNewConfiguration()\n",
			SimTime(), LOG_F00, LOG_LVL1);

	if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s Computing a new configuration\n",
			SimTime(), LOG_C00, LOG_LVL2);

	// TODO: Implement X algorithm according to current configuration and performance
	// ...

	// Generate new configuration *** TODO: right now, we return the same configuration
	new_configuration = configuration;

	// Send the configuration to the AP
	for (int agent_ix = 0 ; agent_ix < agents_number ; agent_ix ++) {
		SendNewConfigurationToAgent(agent_ix);
	}

	// Set trigger for next request
	if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s Next request to be sent at %f\n",
		SimTime(), LOG_C00, LOG_LVL2, fix_time_offset(SimTime() + time_between_requests,13,12));

	//printf("Next request to be sent at %f, %f\n", fix_time_offset(SimTime() + time_between_requests,13,12), time_between_requests);

	trigger_request_information_to_agents.Set(fix_time_offset(SimTime() + time_between_requests,13,12));

}

/*
 * SendNewConfigurationToAp():
 * Input arguments:
 * - to be defined
 */
void CentralController :: SendNewConfigurationToAgent(int destination_agent_id){

//	printf("%s Central Controller: Sending new configuration to Agent%d\n", LOG_LVL1, destination_agent_id);

	if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s SendNewConfigurationToAp()\n",
			SimTime(), LOG_F00, LOG_LVL1);

	if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;CC;%s;%s Sending a new configuration to Agent %d\n",
			SimTime(), LOG_C00, LOG_LVL2, destination_agent_id);

	//if(save_controller_logs) WriteConfiguration(new_configuration);

	// TODO (LOW PRIORITY): generate a trigger to simulate delays in the agent-node communication
	outportSendConfigurationToAgent(destination_agent_id, new_configuration);

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

	for(int i = 0; i < agents_number; i++){
		num_requests[i] = 0;
	}

	save_controller_logs = TRUE;
	print_controller_logs = TRUE;

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
	printf("%s time_between_requests = %f\n", LOG_LVL4, time_between_requests);
	printf("\n");

}
