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
#include "../structures/performance.h"
#include "../structures/action.h"
#include "../structures/controller_report.h"

#include "../methods/auxiliary_methods.h"
#include "../methods/agent_methods.h"

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
        void StartCcActivity();

		// Communication with Agents
		void RequestInformationToAgents();
		void SendCommandToAllAgents(int command_id, Configuration *conf_array);
		void SendConfigurationToSingleAgent(int destination_agent_id, Configuration conf);
		void SendCommandToSingleAgent(int destination_agent_id, int command_id, Configuration conf);

		// Methods related to ML activity
		void ApplyMlMethod();

		// Clustering methods (to group agents/BSSs)
		void GenerateClusters(int wlan_id, Performance performance, Configuration configuration);
		void PrintOrWriteClusters(int print_or_write);
		void UpdatePerformancePerCluster(int shared_performance_metric);

		// Methods for updating statistics
		void UpdateControllerReport(int agent_id, Action *actions, int current_action_id);

		// Print/write methods
		void PrintControllerInfo();
		void WriteControllerInfo(Logger logger);
		void PrintOrWriteControllerStatistics(int print_or_write);
        void WriteAgentPerformance(Action *actions, int agent_id);

		// Initialization
		void InitializeMlPipeline();
        void InitializePreProcessor();
		void InitializeMlModel();

	// Public items (entered by agents constructor in komondor_main)
	public:

		// General configuration of the CC
		int controller_on;				///> Flag indicating whether the CC is active or not
		int controller_mode; 			///> The CC can be either passive (0) or active (1)
		double time_between_requests;	///> Time between requests
        double *agent_iteration_time;   ///> Time between iterations at agents (parameter that can be modified by the CC)

        // Keep track of attached agents
		int agents_number;				///> Number of agents controlled by the CC
		int *list_of_agents;			///> List of the identifiers of the agents controlled by the CC
		int wlans_number;				///> Number of WLANs
		int total_nodes_number;			///> Number of nodes
		int *num_arms_per_agent;		///> Array containing the number of actions available to each agent
		int max_number_of_actions;		///> Maximum number of actions available for all the agents (for generating data structures)

		// Reward and ML method types
		int type_of_reward;				///> Type of reward
		int learning_mechanism;			///> Index of the chosen learning mechanism
		int action_selection_strategy;	///> Index of the chosen action-selection strategy

		// Logs
		int save_controller_logs;		///> Boolean that indicates whether to write CC's logs or not
		int print_controller_logs;		///> Boolean that indicates whether to print CC's logs or not

	// Private items (just for node operation)
	private:

		// File for writing CC logs
		FILE *output_log_file;				///> File for logs in which the agent is involved
		char own_file_path[32];				///> Name of the file for agent logs
		Logger central_controller_logger;	///> struct containing the attributes needed for writing logs in a file
		char *header_string;				///> Header string for the logger

		// CC's report
		ControllerReport controller_report;

		// Arrays with configurations and performances
		Configuration *configuration_array;	///> Array of configuration objects from all the agents
		Performance *performance_array;		///> Array of performance objects from all the agents

		// ML Pipeline elements
		MlModel ml_model;					///> Instantiation of the ML Model
		PreProcessor pre_processor;			///> Instantiation of the Pre-Processor

		// Auxiliary variables for CC operation
		int cc_iteration; 					///> Counter that keeps track of the number of iterations at the CC
		int counter_responses_received; 	///> Needed to determine the number of answers that the controller receives from agents

	// Connections and timers
	public:

		// INPORT connections for receiving notifications
		inport void inline InportReceivingInformationFromAgent(int agent_id, Configuration &configuration,
			Performance &performance, Action *actions, int processed_configuration);

		// OUTPORT connections for sending notifications
		outport void outportSendCommandToAgent(int destination_agent_id, int command_id,
			Configuration &new_configuration, double shared_performance, int type_of_reward);

		// Timers
		Timer <trigger_t> trigger_apply_ml_method;					// Timer for applying the ML method
		Timer <trigger_t> trigger_request_information_to_agents;	// Timer for requesting information to the AP

		// Every time the timer expires execute this
		inport inline void ApplyMlMethod(trigger_t& t1);
		inport inline void RequestInformationToAgents(trigger_t& t1);

		// Connect timers to methods
		CentralController () {
			connect trigger_apply_ml_method.to_component,ApplyMlMethod;
			connect trigger_request_information_to_agents.to_component,RequestInformationToAgents;
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
		//if(save_controller_logs) {
		//	sprintf(own_file_path, "%s_CENTRAL_CONTROLLER.txt","../output/logs_output");
		//	remove(own_file_path);
		//	output_log_file = fopen(own_file_path, "at");
		//	central_controller_logger.save_logs = save_controller_logs;
		//	central_controller_logger.file = output_log_file;
		//	central_controller_logger.SetVoidHeadString();
		//}
		LOGS(save_controller_logs,central_controller_logger.file,
			"%.18f;CC;%s;%s Start()\n", SimTime(), LOG_B00, LOG_LVL1);

		// Initialize the ML Pipeline
		InitializeMlPipeline();

		// Hardcoded [TODO: introduce this parameter to the input]
		controller_mode = CC_MODE_PASSIVE; // CC_MODE_ACTIVE, CC_MODE_PASSIVE

		// Start CC's activity
		StartCcActivity();

	} else {
		printf("The central controller is NOT active\n");
	}

};

/**
 * Stop()
 */
void CentralController :: Stop() {

	if (controller_on) {
		LOGS(save_controller_logs,central_controller_logger.file,
			"%.15f;CC;%s;%s Central Controller Stop()\n", SimTime(), LOG_C00, LOG_LVL1);
		if (controller_mode == CC_MODE_ACTIVE) {
			// Print and write node statistics
			PrintOrWriteControllerStatistics(PRINT_LOG);
			PrintOrWriteControllerStatistics(WRITE_LOG);
		}
		// Close node logs file
		if(save_controller_logs) fclose(central_controller_logger.file);
	}

};

/**************************/
/**************************/
/*  CONTROLLER'S ACTIVITY */
/**************************/
/**************************/

/**
 * Start requesting information to agents, according to the current CC's mode
 */
void CentralController :: StartCcActivity() {
    // According to the defined mode, start making requests by activating triggers
    if (controller_mode == CC_MODE_ACTIVE) {
        // Indicate to all the agents to send information upon receiving a trigger only
        SendCommandToAllAgents(COMMUNICATION_UPON_TRIGGER, configuration_array);    // TODO: based on the intent for the CC, decide to use triggers or not (now it is hardcoded)
        // Generate the time trigger for the first request
        if (time_between_requests > 0) {
            trigger_request_information_to_agents.Set(FixTimeOffset(SimTime() + time_between_requests, 13, 12));
        } else {
            // TODO: [Idea] when "time_between_requests" is negative, apply the ML method after the simulation ends (batch learning)
        }
	} else if (controller_mode == CC_MODE_PASSIVE) {
		// Wait until the agents send data to the CC
	}
}

/**
 * Updates the average performance obtained by every associated agent
 */
//void CentralController :: UpdateAgentAveragePerformance(int agent_id, Action *actions) {
void CentralController :: UpdateControllerReport(int agent_id, Action *actions, int current_action_id) {

	// ACTIVE MODE: Update the performance metrics for the corresponding agent
	if (controller_mode == CC_MODE_ACTIVE) {

		// Update the average performance achieved by the agent
		double cumulative_performance_per_action(0);
		double visited_actions(0);
		int times_action_played(0);

		controller_report.num_arms_per_agent = num_arms_per_agent;
		controller_report.cc_iteration = cc_iteration;

		for (int i = 0; i < max_number_of_actions; ++i) {
			controller_report.times_action_played_per_agent[agent_id][i] = actions[i].times_played_since_last_cc_request;
			if (num_arms_per_agent[agent_id] >= i) {
				controller_report.performance_action_per_agent[agent_id][i] = actions[i].average_reward_since_last_cc_request;
				cumulative_performance_per_action +=
					actions[i].average_reward_since_last_cc_request *
					actions[i].times_played_since_last_cc_request;
				visited_actions += actions[i].times_played_since_last_cc_request;
				// Update the most played action per agent
				if (actions[i].times_played_since_last_cc_request > times_action_played) {
					times_action_played = actions[i].times_played_since_last_cc_request;
					controller_report.most_played_action_per_agent[agent_id] = i;
				}
			}
		}
		if (visited_actions > 0) {
			controller_report.average_performance_per_agent[agent_id] = cumulative_performance_per_action / visited_actions;
		} else {
			controller_report.average_performance_per_agent[agent_id] = 0;
		}

	// PASSIVE MODE: Just get the last performance from the agent
	} else if (controller_mode == CC_MODE_PASSIVE) {

		controller_report.performance_per_agent[agent_id] = actions[current_action_id].instantaneous_reward;

	}

}

/**************************/
/**************************/
/*  COMMUNICATION METHODS */
/**************************/
/**************************/

/**
 * Request information (configuration and performance) to agents upon trigger-based activation
 */
void CentralController :: RequestInformationToAgents(trigger_t &){
	// Request information to every agent associated to the CC
	SendCommandToAllAgents(REQUEST_CONFIGURATION_PERFORMANCE, configuration_array);
};

/**
 * Send a command to all the agents - Unlike "RequestInformationToAgents", this method is not activated with triggers
 * @param "command_id" [type int]: identifier of the command to be sent
 * @param "conf_array" [type *Configuration]: array of configuration objects to provide additional information to the destination agents
 */
void CentralController :: SendCommandToAllAgents(int command_id, Configuration *conf_array){
	LOGS(save_controller_logs,central_controller_logger.file,
		"%.15f;CC;%s;%s SendCommandToAllAgents()\n", SimTime(), LOG_C00, LOG_LVL1);
	// Iterate for all the agents attached to the CC
	for (int ix = 0 ; ix < agents_number ; ++ ix ) {
		SendCommandToSingleAgent(ix, command_id, configuration_array[ix]);
	}
}

/**
 * Send an asynchronous command to a specific agent attached to the CC
 * @param "destination_agent_id" [type int]: identifier of the destination agent
 * @param "command_id" [type int]: identifier of the command to be sent
 * @param "conf" [type Configuration]: configuration object to provide additional information to the destination agent
 */
void CentralController :: SendCommandToSingleAgent(int destination_agent_id, int command_id, Configuration conf){

	// TODO (LOW PRIORITY): generate a trigger to simulate delays in the agent-node communication

	// According to the defined mode, behave in one way or another
	switch(command_id) {

		case REQUEST_CONFIGURATION_PERFORMANCE:{
			LOGS(save_controller_logs,central_controller_logger.file,
				"%.15f;CC;%s;%s Requesting Agent %d to send the current conf. and performance\n",
				SimTime(), LOG_C00, LOG_LVL2, destination_agent_id);
			break;
		}
		case UPDATE_REWARD:{
			LOGS(save_controller_logs,central_controller_logger.file,
				"%.15f;CC;%s;%s Requesting Agent %d to update its current reward to %f\n",
				SimTime(), LOG_C00, LOG_LVL2, destination_agent_id, controller_report.cluster_performance[destination_agent_id]);
			break;
		}
		case UPDATE_CONFIGURATION:{
			LOGS(save_controller_logs,central_controller_logger.file,
				"%.15f;CC;%s;%s Sending a new configuration to Agent %d\n",
				SimTime(), LOG_C00, LOG_LVL2, destination_agent_id);
			break;
		}
		case STOP_ACTING:
		case RESUME_ACTIVITY: {
			LOGS(save_controller_logs,central_controller_logger.file,
				"%.15f;CC;%s;%s Requesting Agent %d to STOP/RESUME its learning activity (%d)\n",
				SimTime(), LOG_C00, LOG_LVL2, destination_agent_id, command_id);
			break;
		}
		case MODIFY_ITERATION_TIME: {
			// Update the time between iterations in the configuration object
			LOGS(save_controller_logs,central_controller_logger.file,
				"%.15f;CC;%s;%s Requesting Agent %d to modify the time of an iteration to %f\n",
				SimTime(), LOG_C00, LOG_LVL2, destination_agent_id, agent_iteration_time[destination_agent_id]);
			break;
		}
		case BAN_CONFIGURATION: {
			// TODO: Add variable to indicate the time this configuration should be unavailable
			// ...
			LOGS(save_controller_logs,central_controller_logger.file,
				"%.15f;CC;%s;%s Requesting Agent %d to BAN a configuration\n",
				SimTime(), LOG_C00, LOG_LVL2, destination_agent_id);
			break;
		}
		case UNBAN_CONFIGURATION: {
			// Specify which configuration needs to be re-activated
			// ...
			LOGS(save_controller_logs,central_controller_logger.file,
				"%.15f;CC;%s;%s Requesting Agent %d to UNBAN a configuration\n",
				SimTime(), LOG_C00, LOG_LVL2, destination_agent_id);
			break;
		}
		case COMMUNICATION_UPON_TRIGGER: {
			LOGS(save_controller_logs,central_controller_logger.file,
				"%.15f;CC;%s;%s Requesting Agent %d to send information only upon receiving triggers\n",
				SimTime(), LOG_C00, LOG_LVL2, destination_agent_id);
			break;
		}
		case COMMUNICATION_AUTOMATIC: {
			LOGS(save_controller_logs,central_controller_logger.file,
				"%.15f;CC;%s;%s Requesting Agent %d to send information automatically\n",
				SimTime(), LOG_C00, LOG_LVL2, destination_agent_id);
			break;
		}
		default: { // Unknown command id
			printf("[CC] ERROR: Undefined command id %d\n", command_id);
			exit(-1);
		}
	}

	outportSendCommandToAgent(destination_agent_id, command_id, conf, controller_report.cluster_performance[destination_agent_id], type_of_reward);

};

/**
 * Called when the CC receives information from an agent
 * @param "received_configuration" [type Configuration]: configuration of the agent
 * @param "received_performance" [type Performance]: performance of the agent
 * @param "agent_id" [type int]: identifier of the agent
 */
void CentralController :: InportReceivingInformationFromAgent(int agent_id,
        Configuration &received_configuration, Performance &received_performance, Action *actions, int current_action_id) {

	LOGS(save_controller_logs,central_controller_logger.file,
		"%.15f;CC;%s;%s InportReceivingInformationFromAgent()\n", SimTime(), LOG_F00, LOG_LVL1);
	LOGS(save_controller_logs,central_controller_logger.file,
		"%.15f;CC;%s;%s New information has been received from Agent %d\n", SimTime(), LOG_C00, LOG_LVL2, agent_id);

	++ counter_responses_received;

	// Update the configuration and performance received
	// - NOT USED AT THIS MOMENT: we currently use the information processed by agents (encapsulated in "actions")
	configuration_array[agent_id] = received_configuration;
	performance_array[agent_id] = received_performance;

	// Update the average performance statistics for the agent that sent information
	UpdateControllerReport(agent_id, actions, current_action_id);

	// Print and/or write the configuration and the performance report
	if(save_controller_logs) {
	    WriteAgentPerformance(actions, agent_id);
	}

	if (counter_responses_received == agents_number) {
		LOGS(save_controller_logs,central_controller_logger.file,
			"%.15f;CC;%s;%s The information from all the agents has been collected!\n", SimTime(), LOG_C01, LOG_LVL2);
		// Create-update clusters, which can be used by the ML method
		for (int agent_ix = 0; agent_ix < agents_number; ++agent_ix) {
			GenerateClusters(agent_ix, performance_array[agent_id], configuration_array[agent_id]);
		}
		PrintOrWriteClusters(WRITE_LOG);
	    // According to the defined mode, behave in one way or another
		switch(controller_mode) {
			// Passive mode: the agents send information to the CC, which only observes (monitoring mode)
			case CC_MODE_PASSIVE: {
				LOGS(save_controller_logs,central_controller_logger.file,
					"%.15f;CC;%s;%s Updating the performance of the agents\n", SimTime(), LOG_C01, LOG_LVL3);
				// Update the shared performance per cluster
				UpdatePerformancePerCluster(MAX_MIN_PERFORMANCE); // AVERAGE_PERFORMANCE, PROP_FAIRNESS_PERFORMANCE, MAX_MIN_PERFORMANCE
				// Send the shared performance to the agents
				SendCommandToAllAgents(UPDATE_REWARD, configuration_array);
				break;
			}
			// Active mode: the CC requests information to agents
			case CC_MODE_ACTIVE: {
				LOGS(save_controller_logs,central_controller_logger.file,
					"%.15f;CC;%s;%s Updating the configuration of the agents\n", SimTime(), LOG_C01, LOG_LVL3);
				// Once all the information is available, compute a new configuration according to the updated rewards
				trigger_apply_ml_method.Set(FixTimeOffset(SimTime(), 13, 12));
				break;
			}
			// Unknown controller mode
			default:{
				printf("[CC] ERROR: Undefined controller mode %d\n"
					"	- Use CC_MODE_PASSIVE (%d) or CC_MODE_ACTIVE (%d)\n",
					controller_mode, CC_MODE_PASSIVE, CC_MODE_ACTIVE);
			}
		}
		counter_responses_received = 0;
	}

};

/**************************************/
/**************************************/
/*  CONFIGURATION GENERATION METHODS  */
/**************************************/
/**************************************/

/**
 * Request information (configuration and performance) to agents upon trigger-based activation
 */
void CentralController :: ApplyMlMethod(trigger_t &){

	LOGS(save_controller_logs,central_controller_logger.file,
		"%.15f;CC;%s;%s Applying the ML method (iteration %d)\n", SimTime(), LOG_C00, LOG_LVL1, cc_iteration);

	// STEP 1 [OPTIONAL]: Update the performance observed per cluster (shared metric)
	//    -  MAX_MIN_PERFORMANCE, PROP_FAIRNESS_PERFORMANCE, AVERAGE_PERFORMANCE
	UpdatePerformancePerCluster(AVERAGE_PERFORMANCE); // TODO: specify the shared performance from input files (now hardcoded)

	// STEP 2: APPLY THE ML METHOD
	ml_model.ComputeGlobalConfiguration(configuration_array, controller_report, central_controller_logger, SimTime());

	// STEP 3: PROVIDE A RESPONSE
	switch(ml_model.learning_mechanism) {
		// Action banning: special case where sconfigurations are banned, rather that provided
		case CENTRALIZED_ACTION_BANNING: {
			// Forward the output to agents
			SendCommandToAllAgents(BAN_CONFIGURATION, configuration_array);
			break;
		}
		// Unknown controller mode
		default:{
			printf("[CC] ERROR: Undefined centralized ML method %d\n"
				"	- Use CENTRALIZED_ACTION_BANNING (%d)\n", controller_mode, CENTRALIZED_ACTION_BANNING);
		}
	}

	// STEP 3: Set the trigger for initiating the next CC iteration
	trigger_request_information_to_agents.Set(FixTimeOffset(SimTime() + time_between_requests,13,12));
	++ cc_iteration;

};

/************************/
/************************/
/*  CLUSTERING METHODS  */
/************************/
/************************/

/**
 * For each agent, provides the list of other agents that belong to the same cluster (updates variable "clusters_per_wlan")
 * @param "wlan_id" [type int]: identifier of the WLAN of interest
 * @param "performance" [type perf]: performance object belonging to the WLAN of interest
 * @param "conf" [type Configuration]: configuration object belonging to the WLAN of interest
 */
void CentralController :: GenerateClusters(int wlan_id, Performance performance, Configuration configuration){

	int clustering_approach(CLUSTER_ALL); // TODO: read this parameter from the input file (now it is hardcoded)

	switch(clustering_approach) {
		// Physical distance
		case CLUSTER_ALL :{
			for (int j = 0; j < wlans_number; ++j) {
				if (wlan_id != j) {
					controller_report.clusters_per_wlan[wlan_id][j] = 1;
				}
			}
			break;
		}
		// CCA + Margin
		case CLUSTER_BY_CCA :{
			double margin_db(5);	// TODO: read this margin from the input file (now it is hardcoded)
			for (int j = 0; j < wlans_number; ++j) {
				if (wlan_id != j && ConvertPower(PW_TO_DBM, performance.max_received_power_in_ap_per_wlan[j])
					  > ConvertPower(PW_TO_DBM, configuration.capabilities.sensitivity_default) - margin_db ) {
					controller_report.clusters_per_wlan[wlan_id][j] = 1;
				}
			}
			break;
		}
		// Physical distance
		case CLUSTER_BY_DISTANCE :{
			// To be done...
			break;
		}
		// Default
		default :{
			printf("[CC] ERROR: clustering approach '%d' does not exist\n", clustering_approach);
			break;
		}
	}

}

/**
 * Compute the overall performance achieved in each cluster
 * @param "print_or_write" [type int]: flag indicating whether to print or write logs
 */
void CentralController :: UpdatePerformancePerCluster(int shared_performance_metric) {

	switch(shared_performance_metric) {

		// Max-min performance
		case MAX_MIN_PERFORMANCE:{
			//int applicable(0);
			for(int i = 0; i < agents_number; ++i) {
				double min_performance(1);
				for(int j = 0; j < agents_number; ++j) {
					//if (controller_report.clusters_per_wlan[i][j] && controller_report.performance_per_agent[j] > 0) {
					if (controller_report.clusters_per_wlan[i][j]) {
						//applicable = 1;
						if (controller_report.performance_per_agent[j] < min_performance)
							min_performance = controller_report.performance_per_agent[j];
							//min_performance = controller_report.average_performance_per_agent[j];
					}
				}
				controller_report.cluster_performance[i] = min_performance;
				//if(applicable) controller_report.cluster_performance[i] = min_performance;
				//else controller_report.cluster_performance[i] = -1;
			}
			break;
		}

		// Geometric sum
		case PROP_FAIRNESS_PERFORMANCE:{
			// TODO: rework this part in order to consider absolute performance values (throughput, delay, etc.)
			for(int i = 0; i < agents_number; ++i) {
				double cumulative_log_performance(0);
				for(int j = 0; j < agents_number; ++j) {
					if (controller_report.clusters_per_wlan[i][j]) {
						cumulative_log_performance += log10(1 + controller_report.performance_per_agent[j]);
					}
				}
				controller_report.cluster_performance[i] = cumulative_log_performance;
			}
			break;
		}

		// Average
		case AVERAGE_PERFORMANCE:{
			double cumulative_performance(0);
			int num_agents_involved(0);
			for(int i = 0; i < agents_number; ++i) {
				for(int j = 0; j < agents_number; ++j) {
					if (controller_report.clusters_per_wlan[i][j]) {
						cumulative_performance += controller_report.performance_per_agent[j];
						++ num_agents_involved;
					}
				}
				controller_report.cluster_performance[i] = cumulative_performance / num_agents_involved;
			}
			break;
		}

		// Average
		case HYBRID_SELFISH_COOPERATIVE:{
			double cumulative_performance(0);
			int num_agents_involved(0);
			for(int i = 0; i < agents_number; ++i) {
				for(int j = 0; j < agents_number; ++j) {
					if (controller_report.clusters_per_wlan[i][j]) {
						cumulative_performance += controller_report.performance_per_agent[j];
						++ num_agents_involved;
					}
				}
				controller_report.cluster_performance[i] = cumulative_performance / num_agents_involved;
			}
			break;
		}

		// Unknown
		default :{
			printf("Unknown performance metric\n");
			break;
		}
	}

}

/**
 * Prints or writes the list of clusters identified for each agent
 * @param "print_or_write" [type int]: flag indicating whether to print or write logs
 */
void CentralController :: PrintOrWriteClusters(int print_or_write){
	switch(print_or_write){
		case PRINT_LOG:{
			printf("Already identified clusters\n");
			for (int i = 0; i < wlans_number; ++i) {
				printf("	Agent %d:", i);
				for (int j = 0; j < wlans_number; ++j) {
					printf(" %d ", controller_report.clusters_per_wlan[i][j]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			LOGS(save_controller_logs, central_controller_logger.file,
				"%.15f;CC;%s;%s Already identified clusters:\n", SimTime(), LOG_C00, LOG_LVL3);
			for (int i = 0; i < wlans_number; ++i) {
				LOGS(save_controller_logs, central_controller_logger.file,
					"%.15f;CC;%s;%s Agent %d:" , SimTime(), LOG_C00, LOG_LVL4, i);
				for (int j = 0; j < wlans_number; ++j) {
					LOGS(save_controller_logs, central_controller_logger.file,
						" %d ", controller_report.clusters_per_wlan[i][j]);
				}
                LOGS(save_controller_logs, central_controller_logger.file, "\n");
			}
			break;
		}
	}
}

/******************************/
/******************************/
/*  VARIABLES INITIALIZATION  */
/******************************/
/******************************/

/**
 * Initialize the ML Pipeline
 */
void CentralController :: InitializeMlPipeline() {
    InitializePreProcessor();
    InitializeMlModel();
}


/**
 * Initialize the Pre-Processor (PP)
 */
void CentralController :: InitializePreProcessor() {
//	pre_processor.type_of_reward = type_of_reward;
	pre_processor.InitializeVariables();
}

/**
 * Initialize the ML Model
 */
void CentralController :: InitializeMlModel() {

	ml_model.learning_mechanism = learning_mechanism;
	ml_model.save_logs = save_controller_logs;
	ml_model.print_logs = print_controller_logs;
	ml_model.action_selection_strategy = action_selection_strategy;
	ml_model.agents_number = agents_number;
	ml_model.max_number_of_actions = max_number_of_actions;
	ml_model.InitializeVariables();

	// Initialize arrays
	for (int i = 0; i < agents_number; ++i) {
		// Fill the matrix containing the set of available actions in each agent and the matrix indicating their performance
		for (int j = 0; j < max_number_of_actions; ++j) {
			if (num_arms_per_agent[i] >= j) {
				controller_report.list_of_available_actions_per_agent[i][j] = 1;		// The action exists and is available (set to 1 by default)
				controller_report.performance_action_per_agent[i][j] = 0;		// Set default performance to 0
				controller_report.times_action_played_per_agent[i][j] = 0;
			} else {
				controller_report.list_of_available_actions_per_agent[i][j] = -1;		// The action does not exist (the index exceeds the total number of actions of that agent)
				controller_report.performance_action_per_agent[i][j] = -1;	// The actions does not exist
				controller_report.times_action_played_per_agent[i][j] = -1;
			}
		}
		// Initialize the matrix that indicates the potential interfering OBSSs of each BSS (only for agents connected to the CC)
		for (int j = 0; j < agents_number; ++j) {
			if (i == j) controller_report.clusters_per_wlan[i][j] = 1;
			else controller_report.clusters_per_wlan[i][j] = 0;
		}
		controller_report.cluster_performance[i] = 0;
		controller_report.most_played_action_per_agent[i] = -1;
	}

}

/**
 * Initialize the Central Controller
 */
void CentralController :: InitializeCentralController() {

	time_between_requests = 0;

	save_controller_logs = TRUE;
	print_controller_logs = TRUE;

	cc_iteration = 0;
	counter_responses_received = 0;

	configuration_array = new Configuration[agents_number];
	performance_array  = new Performance[agents_number];

	num_arms_per_agent = new int[agents_number];

	// Initialize the controller's report
	controller_report.agents_number = agents_number;
	controller_report.max_number_of_actions = max_number_of_actions;
	controller_report.SetSizeOfArrays();

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
 * Write the performance obtained by an agent during the last CC iteration
 * @param "actions" [type *Action]: array of action objects provided by the agent
 * @param "agent_id" [type int]: agent identifier
 */
void CentralController :: WriteAgentPerformance(Action *actions, int agent_id) {
    LOGS(save_controller_logs, central_controller_logger.file,
         "%.15f;CC;%s;%s Average performance (A%d): %.2f\n",
         SimTime(), LOG_C00, LOG_LVL3, agent_id, controller_report.average_performance_per_agent[agent_id]);
    LOGS(save_controller_logs, central_controller_logger.file,
         "%.15f;CC;%s;%s Average performance of actions (A%d): ", SimTime(), LOG_C00, LOG_LVL3, agent_id);
    for (int i = 0; i < num_arms_per_agent[agent_id]; ++i) {
        LOGS(save_controller_logs, central_controller_logger.file, "%.2f ",
             actions[i].average_reward_since_last_cc_request);
    }
    LOGS(save_controller_logs, central_controller_logger.file, "\n");
    LOGS(save_controller_logs, central_controller_logger.file,
         "%.15f;CC;%s;%s Times each action has been played (A%d): ", SimTime(), LOG_C00, LOG_LVL3, agent_id);
    for (int i = 0; i < num_arms_per_agent[agent_id]; ++i) {
        LOGS(save_controller_logs, central_controller_logger.file, "%d ", actions[i].times_played_since_last_cc_request);
    }
    LOGS(save_controller_logs, central_controller_logger.file, "\n");
}

/**
 * Print or write CC's statistics
 * @param "print_or_write" [type int]: boolean indicating whether to print by console or write logs into a file
 */
void CentralController :: PrintOrWriteControllerStatistics(int print_or_write) {
	switch(print_or_write){
		case PRINT_LOG:{
			if (print_controller_logs) printf("\n------- Central Controller ------\n");
			ml_model.PrintOrWriteStatistics(PRINT_LOG, central_controller_logger, SimTime());
			break;
		}
		case WRITE_LOG:{
			LOGS(save_controller_logs, central_controller_logger.file,
				"\n%.15f;CC;%s;%s ------- Central Controller ------\n", SimTime(), LOG_C00, LOG_LVL1);
			ml_model.PrintOrWriteStatistics(WRITE_LOG, central_controller_logger, SimTime());
			break;
		}
	}
}
