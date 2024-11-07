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
 * agent.h: this file defines the agent component
 *
 * - This file contains the methods and functionalities used by the agent, including the communication with the APs of the controlled networks.
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
#include "../structures/performance.h"
#include "../structures/action.h"
#include "../methods/auxiliary_methods.h"
#include "../methods/agent_methods.h"

#include "../learning_modules/pre_processor.h"
#include "../learning_modules/ml_model.h"

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
        void InitializeMlPipeline();
		void InitializePreProcessor();
		void InitializeMlModel();

		// Communication with AP
		void RequestInformationToAp();
		void ComputeNewConfiguration();
		void SendNewConfigurationToAp(Configuration &configuration_to_send);

		// Communication with CC
		void ForwardInformationToController();
		void UpdateConfigurationStatisticsController(int selected_conf_ix);
		void ResetControllerStatistics();

		// Communication with other Agents (distributed methods)
		// ... [Future feature]

		// Actions management
		void UpdateActionInformation(int action_ix);

		// Print methods
		void PrintAgentInfo();
		void WriteAgentInfo(Logger logger, std::string header_str);
		void PrintOrWriteAgentStatistics();

	// Public items (entered by agents constructor in komondor_main)
	public:

		// Specific to each agent
		int agent_id; 				///> Node identifier
		int agent_centralized;		///> Indicates the mode of the agent (DECENTRALIZED; COOPERATIVE; CENTRALIZED)
		std::string wlan_code;		///> WLAN code to which the agent belongs
		int wlan_id;                ///> WLAN identifier to which the agent belongs
		int num_stas;				///> Number of STAs associated to the WLAN

		// Learning mechanism
		int learning_mechanism;			///> Index of the chosen learning mechanism
		int action_selection_strategy;	///> Index of the chosen action-selection strategy

		// Central Controller parameters
		int controller_on;				///> Flag to determine whether the CC is on or not

		// Actions management (tunable parameters)
		int *list_of_channels; 				///> List of channels
		double *list_of_pd_values;			///> List of PD values
		double *list_of_tx_power_values;	///> List of TX power values
		int *list_of_max_bandwidth;			///> List of DCB policies

		Action *actions;					///> List of actions
		int *list_of_available_actions;		///> List of the actions that are available
		int num_arms;					///> Number of actions (depends on the configuration parameters - pd, tx_power, channels, etc.)
		int num_arms_channel;			///> Number of channels available
		int num_arms_sensitivity;		///> Number of PD levels available
		int num_arms_tx_power;			///> Number of TX power levels available
		int num_arms_max_bandwidth;			///> Number of DCB policies available

		// Other input parameters
		int type_of_reward;					///> Type of reward
		//int type_of_reward_cc;              ///> Type of reward chosen by the CC
		double time_between_requests; 		///> Time between two information requests to the AP (for a given measurement)

		// Print/write variables
		int save_agent_logs;			///> Boolean that indicates whether to write agent's logs or not
		int print_agent_logs;			///> Boolean that indicates whether to print agent's logs or not
		std::string simulation_code;	///> Simulation code

		// RTOT
		double margin_rtot;      ///> Margin for the RTOT mechanism (see https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=8319274)

	// Private items (just for internal agent operation)
	private:

        // File for writing node logs
        FILE *output_log_file;				///> File for logs in which the agent is involved
        char own_file_path[32];				///> Name of the file for agent logs
        Logger agent_logger;				///> struct containing the attributes needed for writing logs in a file
        char *header_string;				///> Header string for the logger

        // Variables to store performance and configuration reports
        Performance performance;						///> Performance object
        int *indexes_configuration;                     ///>
        Configuration configuration;					///> Configuration object
        Configuration new_configuration;				///> Object containing the new configuration to be set
        Configuration configuration_from_controller;	///> Configuration object obtained from the CC
        int ml_output;	                            ///> Output of the ML model (previous step to the new configuration)

        // ML-based architecture (see https://arxiv.org/abs/1910.03510)
        PreProcessor pre_processor;             ///> Pre-processor object
        MlModel ml_model;                       ///> ML model object
        int learning_allowed;                   ///> Flag to indicate whether learning is allowed or not
        int flag_compute_new_configuration; 	///> Flag to be activated in case of needing to compute a new configuration

		// Configuration and performance after being processed by the Pre-processor
		int processed_configuration;	///> Processed configuration
		double processed_reward;		///> Processed performance
		//double processed_reward_cc;     ///> Processed performance according to the type of reward fixed at the CC

		// Items related to the interaction with the Central Controller (CC)
        int automatic_forward_enabled;          ///> Flag to indicate that data received from the AP is automatically forwarded to the CC
		int flag_request_from_controller;		///> Flag to be activated in case the CC made a request
		int flag_information_available;			///> Flag to indicate that information is available at the agent

		// Auxiliary variables
        int num_requests;			///> Number of requests made by the agent to the AP
        double initial_reward;		///> Initial reward assigned to each arm

	// Connections and timers
	public:

		// INPORT connections for receiving notifications
		inport void inline InportReceivingInformationFromAp(Configuration &configuration, Performance &performance);
		// OUTPORT connections for sending requests/notifications
		outport void outportRequestInformationToAp();
		outport void outportSendConfigurationToAp(Configuration &new_configuration);
		// INPORT & OUTPORT (centralized system only)
        inport void inline InportReceiveCommandFromController(int destination_agent_id,
            int command_id, Configuration &new_configuration, double shared_performance, int type_of_reward);
		outport void outportAnswerToController(int agent_id, Configuration &configuration,
			Performance &performance, Action *actions, int processed_configuration);
		// Triggers
		Timer <trigger_t> trigger_request_information_to_ap;        // Timer for requesting information to the AP
		inport inline void RequestInformationToAp(trigger_t& t1);   // Every time the timer expires execute this
		Agent () { // Connect timers to methods
			connect trigger_request_information_to_ap.to_component,RequestInformationToAp;
		}

};

/**
 * Setup()
 */
void Agent :: Setup(){
	// Do nothing
};

/**
 * Start()
 */
void Agent :: Start(){

	// Create agent logs file if required
	if(save_agent_logs) {
		// Name agent log file accordingly to the agent_id
		sprintf(own_file_path,"%s_%s_A%d_%s.txt","../output/logs_output", simulation_code.c_str(), agent_id, wlan_code.c_str());
		remove(own_file_path);
		output_log_file = fopen(own_file_path, "at");
		agent_logger.save_logs = save_agent_logs;
		agent_logger.file = output_log_file;
		agent_logger.SetVoidHeadString();
	}
	LOGS(save_agent_logs, agent_logger.file, "%.18f;A%d;%s;%s Start()\n", SimTime(), agent_id, LOG_B00, LOG_LVL1);

	// Initialize the ML Pipeline
	InitializeMlPipeline();

	// Compute the new configuration by using the current ML model
	ComputeNewConfiguration();

};

/**
 * Stop()
 */
void Agent :: Stop(){
	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s Agent Stop()\n", SimTime(), agent_id, LOG_C00, LOG_LVL1);
	// Print statistics
	PrintOrWriteAgentStatistics();
	// Close node logs file
	if(save_agent_logs) fclose(agent_logger.file);
};

/***************************/
/***************************/
/*  AP-AGENT COMMUNICATION */
/***************************/
/***************************/

/**
 * Request information to the AP. This method is triggered by a process that decides the time between requests.
 */
void Agent :: RequestInformationToAp(trigger_t &){
	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s RequestInformationToAp() (request #%d)\n",
		SimTime(), agent_id, LOG_F00, LOG_LVL1, num_requests);
	// Send a request to the AP
	outportRequestInformationToAp();
	// Increase the number of requests done
	++ num_requests;
};

/**
 * Called when the agent receives information from the AP
 * @param "received_configuration" [type Configuration]: configuration object received from the corresponding AP
 * @param "received_performance" [type Performance]: performance report received from the corresponding AP
 */
void Agent :: InportReceivingInformationFromAp(Configuration &received_configuration, Performance &received_performance){

	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s InportReceivingInformationFromAp()\n",
		SimTime(), agent_id, LOG_F00, LOG_LVL1);

	// Save the Configuration and Performance reports received from the AP
	configuration = received_configuration;
	performance = received_performance;

	//printf("channel = %d / pd = %f / tx_power = %f / max_bandwidth = %d\n",
	//		configuration.selected_primary_channel, ConvertPower(PW_TO_DBM, configuration.selected_pd),
	//		ConvertPower(PW_TO_DBM, configuration.selected_tx_power), configuration.selected_max_bandwidth);

	// Process the information received from the AP
	// 1 - Find the index of the current action
	processed_configuration = pre_processor.ProcessWlanConfiguration(MULTI_ARMED_BANDITS, configuration, TRUE);
	// 2 - Compute the reward based on the observed performance
	processed_reward = pre_processor.ProcessWlanPerformance(performance, type_of_reward);
	//// 3 - Process the performance to obtain the corresponding reward according to the central controller
	//if(controller_on) processed_reward_cc = pre_processor.ProcessWlanPerformance(performance, type_of_reward_cc);

	// Update the information of the current selected action
	UpdateActionInformation(processed_configuration);

	// Write the action that is currently selected and its associated performance
	if(save_agent_logs) {
		char device_code[10];
		sprintf(device_code, "A%d", agent_id);
		actions[processed_configuration].WriteAction(agent_logger, save_agent_logs, SimTime(), device_code);
		pre_processor.WritePerformance(agent_logger, SimTime(), device_code, performance, type_of_reward, processed_reward);
	}

	// Set flag "information available" to true
	flag_information_available = true;

	// Forward the received information to the controller (if CC is enabled)
	if (controller_on && automatic_forward_enabled) {
        // Forward the information to the controller and wait for a response
		ForwardInformationToController();
	} else {
		if (learning_mechanism == MONITORING_ONLY) {
			// Keep monitoring the performance of the AP
			trigger_request_information_to_ap.Set(FixTimeOffset(SimTime() + time_between_requests,13,12));
		} else {
			// Compute a new configuration (if necessary)
			ComputeNewConfiguration();
		}
	}

	// Update the agent's capabilities (IN CASE THE SET OF ACTIONS HAS CHANGED DUE TO ACTION BANNING)
	configuration.agent_capabilities.num_arms = num_arms;
	configuration.agent_capabilities.available_actions = list_of_available_actions;

};

/**
 * Send a new configuration to the AP
 * @param "configuration_to_send" [type Configuration]: configuration object to send to the corresponding AP
 */
void Agent :: SendNewConfigurationToAp(Configuration &configuration_to_send){

	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s SendNewConfigurationToAp()\n",
		SimTime(), agent_id, LOG_F00, LOG_LVL1);

	// Print the configuration (action) to be sent
    char device_code[10];
    sprintf(device_code, "A%d", agent_id);
    processed_configuration = pre_processor.ProcessWlanConfiguration(MULTI_ARMED_BANDITS, configuration_to_send, FALSE);
	if(save_agent_logs) actions[processed_configuration].WriteAction(agent_logger, save_agent_logs, SimTime(), device_code);

	// TODO (LOW PRIORITY): generate a trigger to simulate delays in the agent-node communication
	outportSendConfigurationToAp(configuration_to_send);

	// Set trigger for next request in case of being an independent agent (not controlled by a central entity)
	LOGS(save_agent_logs, agent_logger.file, "%.15f;A%d;%s;%s Next request to be sent at %f\n",
		SimTime(), agent_id, LOG_C00, LOG_LVL2, FixTimeOffset(SimTime() + time_between_requests,13,12));
	trigger_request_information_to_ap.Set(FixTimeOffset(SimTime() + time_between_requests,13,12));
	flag_compute_new_configuration = true;

};

/***************************/
/***************************/
/*  AGENT-CC COMMUNICATION */
/***************************/
/***************************/

/**
 * Called when the agent receives a configuration from the CC
 * @param "destination_agent_id" [type int]: identifier of the agent to which the information is delivered
 * @param "received_configuration" [type Configuration]: reference of the configuration received from the controller
 * @param "controller_mode" [type int]:
 */
void Agent :: InportReceiveCommandFromController(int destination_agent_id, int command_id,
		Configuration &received_configuration, double shared_performance, int type_of_reward) {

	if(controller_on && agent_id == destination_agent_id ) {

		LOGS(save_agent_logs, agent_logger.file,
			"%.15f;A%d;%s;%s InportReceiveCommandFromController()\n",
			SimTime(), agent_id, LOG_F02, LOG_LVL1);

		// Update the type of reward desired by the CC
        //type_of_reward_cc = type_of_reward;

        switch(command_id) {
			// Send the current configuration to the CC
			case REQUEST_CONFIGURATION_PERFORMANCE:{
				LOGS(save_agent_logs,agent_logger.file,
					"%.15f;A%d;%s;%s SENDING the current configuration and performance to the CC...\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL2);
				if ( CheckValidityOfData(configuration, performance, SimTime(),
						MAX_TIME_INFORMATION_VALID) && flag_information_available) {
					ForwardInformationToController();
				} else {
					// Request information to the AP (trigger = 0)
					trigger_request_information_to_ap.Set(FixTimeOffset(SimTime(),13,12));
					flag_request_from_controller = true;
				}
				break;
			}
			// Update the reward information according to controller's information
			case UPDATE_REWARD:{
				LOGS(save_agent_logs,agent_logger.file,
					"%.15f;A%d;%s;%s UPDATING the reward (%f) for the current configuration...\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL2, shared_performance);
				// Update the performance and reward
				processed_reward = shared_performance;
				LOGS(save_agent_logs,agent_logger.file,
					"%.15f;A%d;%s;%s Updated reward = %f\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL3, processed_reward);
				// Compute a new configuration
				ComputeNewConfiguration();
				break;
			}
			// Update the configuration to the one sent by the CC
			case UPDATE_CONFIGURATION:{
				LOGS(save_agent_logs,agent_logger.file,
					"%.15f;A%d;%s;%s UPDATING configuration...\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL2);
				// Update the received configuration
				configuration_from_controller = received_configuration;
				// Forward the configuration to the AP
				SendNewConfigurationToAp(configuration_from_controller);
				break;
			}
			// Stop acting for a determined time elapse
			case STOP_ACTING: {
				LOGS(save_agent_logs,agent_logger.file,
					"%.15f;A%d;%s;%s STOPPING learning activity...\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL2);
				learning_allowed = FALSE;
			}
			// Resume the learning activity
			case RESUME_ACTIVITY: {
				LOGS(save_agent_logs,agent_logger.file,
					"%.15f;A%d;%s;%s RESUMING learning activity...\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL2);
				learning_allowed = TRUE;
				break;
			}
			// Modify the time between learning iterations
			case MODIFY_ITERATION_TIME: {
				LOGS(save_agent_logs,agent_logger.file,
					"%.15f;A%d;%s;%s Modifying the iteration time to %f...\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL2, received_configuration.agent_capabilities.time_between_requests);
				time_between_requests = received_configuration.agent_capabilities.time_between_requests;
				break;
			}
			// Ban a certain configuration/action
			case BAN_CONFIGURATION:
			case UNBAN_CONFIGURATION: {
				LOGS(save_agent_logs,agent_logger.file,
					"%.15f;A%d;%s;%s BANNING/RESTORING configuration...\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL2);
				// Update list of available actions based on the information sent by the controller
				list_of_available_actions = received_configuration.agent_capabilities.available_actions;
				char device_code[10];
				sprintf(device_code, "A%d", agent_id);
				pre_processor.PrintOrWriteAvailableActions(WRITE_LOG, device_code,
					save_agent_logs, agent_logger, SimTime(), list_of_available_actions);
				break;
			}
			// Do not send information unless receiving a request from the CC
			case COMMUNICATION_UPON_TRIGGER: {
				LOGS(save_agent_logs,agent_logger.file,
					"%.15f;A%d;%s;%s Stop sending automatic reports. Waiting until receiving requests from the CC...\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL2);
				automatic_forward_enabled = FALSE;
				break;
			}
			// Automatic information forwarding to the CC
			case COMMUNICATION_AUTOMATIC: {
				LOGS(save_agent_logs,agent_logger.file,
					"%.15f;A%d;%s;%s Start sending automatic reports...\n",
					SimTime(), agent_id, LOG_C00, LOG_LVL2);
				automatic_forward_enabled = TRUE;
				break;
			}
			// Unknown command id
			default: {
				printf("[A%d] ERROR: Undefined command id %d\n", agent_id, command_id);
				exit(-1);
			}
		}
	}

}

/**
 * Forward information (Conf. and perf.) to the central controller
 * @param "configuration" [type Configuration]: current configuration of the WLAN
 * @param "performance" [type Performance]: current performance of the WLAN
 * @param "agent_id" [type int]: identifier of the agent sending the information
 */
void Agent :: ForwardInformationToController(){

	LOGS(save_agent_logs, agent_logger.file,
		"%.15f;A%d;%s;%s Forwarding information to the controller...\n",
		SimTime(), agent_id, LOG_F02, LOG_LVL2);

	// Compute the average performance achieved by each arm
	for (int i = 0; i < num_arms; ++i) {
		UpdateConfigurationStatisticsController(i);
	}

	// Send the current configuration (and performance) to the CC
	outportAnswerToController(agent_id, configuration, performance, actions, processed_configuration);

	// Reset the CC statistics
	ResetControllerStatistics();

}

/**
 * Reset the statistics set during the last CC iteration
 */
void Agent :: ResetControllerStatistics() {
	for (int i = 0; i < num_arms; ++i) {
		actions[i].average_reward_since_last_cc_request = 0;
		actions[i].cumulative_reward_since_last_cc_request = 0;
		actions[i].times_played_since_last_cc_request = 0;
	}
}

/**
 * Update the statistics to be used at the CC
 * @param "action_ix" [type int]: index of the selected configuration
 */
void Agent :: UpdateConfigurationStatisticsController(int action_ix) {
	if (actions[action_ix].times_played_since_last_cc_request > 0) {
		actions[action_ix].average_reward_since_last_cc_request =
			actions[action_ix].cumulative_reward_since_last_cc_request /
			actions[action_ix].times_played_since_last_cc_request;
	} else {
		actions[action_ix].average_reward_since_last_cc_request = 0;
	}
}

/***************************/
/***************************/
/*  GENERATE A NEW CONFIG. */
/***************************/
/***************************/

/**
 * Compute a new configuration to be sent to the AP. Different mechanisms can be used, which are expected to return the new configuration.
 */
void Agent :: ComputeNewConfiguration(){

	// Act only if the learning procedure is allowed (to be determined by the CC, if necessary)
	if (learning_allowed) {

		LOGS(save_agent_logs, agent_logger.file, "%.15f;A%d;%s;%s ComputeNewConfiguration()\n",
			SimTime(), agent_id, LOG_F00, LOG_LVL1);

		// Compute a new configuration if information is up to date. Otherwise, request it to the AP and use it.
		if ( CheckValidityOfData(configuration, performance, SimTime(), MAX_TIME_INFORMATION_VALID)
				&& flag_information_available) {
			// Process the obtained information before sending it to the controller
			// Update the configuration according to the selected learning method
			switch(learning_mechanism) {
				/* Multi-Armed Bandits */
				case MULTI_ARMED_BANDITS:{
					// Update the configuration according to the MABs operation
					ml_output = ml_model.ComputeIndividualConfiguration
						(processed_configuration, processed_reward, agent_logger,
						SimTime(), list_of_available_actions);
					//PrintOrWriteAgentStatistics();
					break;
				}
				case RTOT_ALGORITHM:{
                    ml_output = ml_model.ComputeIndividualConfiguration
						(processed_configuration, processed_reward, agent_logger,
						SimTime(), list_of_available_actions);
					break;
				}
				default:{
					printf("[AGENT] ERROR: %d is not a correct learning mechanism\n", learning_mechanism);
					ml_model.PrintAvailableLearningMechanisms();
					exit(EXIT_FAILURE);
					break;
				}
			}
			// Process the configuration from the output of the ML method
			new_configuration = pre_processor.ProcessMLOutput(learning_mechanism, configuration, ml_output);
			// Send the configuration to the AP
			SendNewConfigurationToAp(new_configuration);

		} else {
			// Generate the first request to be triggered after "time_between_requests"
			// *** We generate here the first request in order to obtain the AP's configuration
			// TODO: add an activation time, to be introduced by the user in the agent's configuration file
			double extra_wait_test = 0.005 * (double) agent_id;
			trigger_request_information_to_ap.Set(FixTimeOffset(SimTime() + time_between_requests + extra_wait_test,13,12));
		}

	} else {
		printf("The learning operation is not allowed at this moment.\n");
	}

}

/**
 * Update the Action object for the corresponding played action
 * @param "action_ix" [type int]: index of the selected action/configuration
 */
void Agent :: UpdateActionInformation(int action_ix){
	// Current information
	actions[action_ix].performance_since_last_cc_request = performance;
	actions[action_ix].instantaneous_reward = processed_reward;
	// Full run information
	actions[action_ix].cumulative_reward += processed_reward;
	++ actions[action_ix].times_played;
	// Information since last CC request
	actions[action_ix].cumulative_reward_since_last_cc_request += processed_reward;
	++ actions[action_ix].times_played_since_last_cc_request;
}

/*****************************/
/*****************************/
/*  VARIABLE INITIALIZATION  */
/*****************************/
/*****************************/

/**
 * Initialize the agent
 */
void Agent :: InitializeAgent() {

//	printf("Agent #%d says: I'm alive!\n", agent_id);

	learning_allowed = 1;
	num_requests = 0;
	controller_on = FALSE;
	automatic_forward_enabled = TRUE;

	initial_reward = 0;

	flag_request_from_controller = false;
	flag_information_available = false;

	indexes_configuration = new int[NUM_FEATURES_ACTIONS];

	list_of_channels = new int[num_arms_channel];
	list_of_pd_values = new double[num_arms_sensitivity];
	list_of_tx_power_values = new double[num_arms_tx_power];
	list_of_max_bandwidth = new int[num_arms_max_bandwidth];

	actions = new Action[num_arms];
	list_of_available_actions = new int[num_arms];
	for (int i = 0; i < num_arms; ++i) {
		list_of_available_actions[i] = 1;
	}

}

/**
 * Initialize the pre-processor
 */
void Agent :: InitializeMlPipeline() {
    // Initialize the PP
    InitializePreProcessor();
    // Initialize the ML Method
    InitializeMlModel();
}

/**
 * Initialize the pre-processor
 */
void Agent :: InitializePreProcessor() {

	pre_processor.num_arms = num_arms;
	pre_processor.num_arms_channel = num_arms_channel;
	pre_processor.num_arms_sensitivity = num_arms_sensitivity;
	pre_processor.num_arms_tx_power = num_arms_tx_power;
	pre_processor.num_arms_max_bandwidth = num_arms_max_bandwidth;

	pre_processor.InitializeVariables();

	pre_processor.list_of_channels = list_of_channels;
	pre_processor.list_of_pd_values = list_of_pd_values;
	pre_processor.list_of_tx_power_values = list_of_tx_power_values;
	pre_processor.list_of_max_bandwidth = list_of_max_bandwidth;

    // Initialize the actions array
    actions = pre_processor.InitializeActions();

}

/**
 * Initialize the ML Model
 */
void Agent :: InitializeMlModel() {
	// Agent information
	ml_model.agent_id = agent_id;
	ml_model.num_stas = num_stas;
	// ML model information
	ml_model.learning_mechanism = learning_mechanism;
	ml_model.action_selection_strategy = action_selection_strategy;
	// MABs information
	ml_model.num_channels = num_arms_channel;
	ml_model.num_arms = num_arms;
	// Logs
	ml_model.save_logs = save_agent_logs;
	ml_model.print_logs = print_agent_logs;
	if (learning_mechanism == RTOT_ALGORITHM) {
		ml_model.margin_rtot = margin_rtot;
	}
	// Initialize variables
	ml_model.InitializeVariables();
}

/******************************/
/******************************/
/*  PRINT/WRITE INFORMATION   */
/******************************/
/******************************/

/**
 * Print Agent's information
 */
void Agent :: PrintAgentInfo(){
	printf("%s Agent %d info:\n", LOG_LVL3, agent_id);
	printf("%s wlan_code = %s\n", LOG_LVL4, wlan_code.c_str());
	printf("%s num_stas = %d\n", LOG_LVL4, num_stas);
	printf("%s agent_centralized = %d\n", LOG_LVL4, agent_centralized);
	printf("%s time_between_requests = %f\n", LOG_LVL4, time_between_requests);
	printf("%s type_of_reward = %d\n", LOG_LVL4, type_of_reward);
	printf("%s initial_reward = %f\n", LOG_LVL4, initial_reward);
	printf("%s list_of_channels: ", LOG_LVL4);
	for (int i = 0; i < num_arms_channel; ++i) {
		printf("%d  ", list_of_channels[i]);
	}
	printf("\n");
	printf("%s list_of_pd_values: ", LOG_LVL4);
	for (int i = 0; i < num_arms_sensitivity; ++i) {
		printf("%f pW (%f dBm)  ", list_of_pd_values[i], ConvertPower(PW_TO_DBM, list_of_pd_values[i]));
	}
	printf("\n");
	printf("%s list_of_tx_power_values: ", LOG_LVL4);
	for (int i = 0; i < num_arms_tx_power; ++i) {
		printf("%f pW (%f dBm)  ", list_of_tx_power_values[i], ConvertPower(PW_TO_DBM, list_of_tx_power_values[i]));
	}
	printf("\n");
	printf("%s list_of_max_bandwidth: ", LOG_LVL4);
	for (int i = 0; i < num_arms_max_bandwidth; ++i) {
		printf("%d  ", list_of_max_bandwidth[i]);
	}
	printf("\n");
	printf("%s learning_mechanism: %d\n", LOG_LVL4, learning_mechanism);
	printf("%s action_selection_strategy: %d\n", LOG_LVL4, action_selection_strategy);
	printf("%s save_agent_logs: %d\n", LOG_LVL4, save_agent_logs);
	printf("%s print_agent_logs: %d\n", LOG_LVL4, print_agent_logs);
	printf("\n");
}

/**
 * Write information of the Agent into a given logs file - NOTE: We don't use "LOGS" because this can be called from other entities (e.g., the controller)
 * @param "logger" [type Logger]: logger object that prints the information into a file
 * @param "header_str" [type std::string]: heading string
 */
void Agent :: WriteAgentInfo(Logger logger, std::string header_str){
	fprintf(logger.file, "%s Agent %d info:\n", header_str.c_str(), agent_id);
	fprintf(logger.file, "%s - wlan_code = %s\n", header_str.c_str(), wlan_code.c_str());
	fprintf(logger.file, "%s - num_stas = %d\n", header_str.c_str(), num_stas);
	fprintf(logger.file, "%s - agent_centralized = %d\n", header_str.c_str(), agent_centralized);
	fprintf(logger.file, "%s - time_between_requests = %f\n", header_str.c_str(), time_between_requests);
	fprintf(logger.file, "%s - type_of_reward = %d\n", header_str.c_str(), type_of_reward);
	fprintf(logger.file, "%s - initial_reward = %f\n", header_str.c_str(), initial_reward);
	fprintf(logger.file, "%s - list_of_channels: ", header_str.c_str());
	for (int i = 0; i < num_arms_channel; ++i) {
		fprintf(logger.file, "%d  ", list_of_channels[i]);
	}
	fprintf(logger.file, "\n");
	fprintf(logger.file, "%s - list_of_pd_values: ", header_str.c_str());
	for (int i = 0; i < num_arms_sensitivity; ++i) {
		fprintf(logger.file, "%f pW (%f dBm)  ", list_of_pd_values[i], ConvertPower(PW_TO_DBM, list_of_pd_values[i]));
	}
	fprintf(logger.file, "\n");
	fprintf(logger.file, "%s - list_of_tx_power_values: ", header_str.c_str());
	for (int i = 0; i < num_arms_channel; ++i) {
		fprintf(logger.file, "%f pW (%f dBm)  ", list_of_tx_power_values[i], ConvertPower(PW_TO_DBM, list_of_tx_power_values[i]));
	}
	fprintf(logger.file, "\n");
	fprintf(logger.file, "%s - list_of_max_bandwidth: ", header_str.c_str());
	for (int i = 0; i < num_arms_channel; ++i) {
		fprintf(logger.file, "%d  ", list_of_max_bandwidth[i]);
	}
	fprintf(logger.file, "\n");
	fprintf(logger.file, "%s - learning_mechanism = %d\n", header_str.c_str(), learning_mechanism);
	fprintf(logger.file, "%s - action_selection_strategy = %d\n", header_str.c_str(), action_selection_strategy);
	fprintf(logger.file, "%s - save_agent_logs = %d\n", header_str.c_str(), save_agent_logs);
	fprintf(logger.file, "%s - print_agent_logs = %d\n", header_str.c_str(), print_agent_logs);
}

/**
 * Print Agent's statistics
 */
void Agent :: PrintOrWriteAgentStatistics() {
	if (print_agent_logs) printf("\n------- Agent A%d ------\n", agent_id);
	ml_model.PrintOrWriteStatistics(PRINT_LOG, agent_logger, SimTime());
//	ml_model.PrintOrWriteStatistics(WRITE_LOG, agent_logger, SimTime());
}


