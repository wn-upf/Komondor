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
 * komondor_main.cc: this is the main Komondor file.
 *
 * - This file generates the wireless network according to the input files. Then, it initiates nodes to start sending packets until the simulation time is over. Finally, it processes the results.
 */

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <map>
#include <string>     // std::string, std::to_string
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>    // For strdup, strcmp, etc.
#include <memory>     // For smart pointers

#include ".././COST/cost.h"

#include "../list_of_macros.h"

#include "../structures/logical_nack.h"
#include "../structures/notification.h"
#include "../structures/wlan.h"

#include "../methods/output_generation_methods.h"

#include "node.h"
#include "traffic_generator.h"
#include "agent.h"
#include "central_controller.h"
#include "file_manager.h"
#include "path_manager.h"
#include "argument_parser.h"
#include "configuration_parser.h"

/* Sequential simulation engine from where the system to be simulated is derived. */
component Komondor : public CostSimEng {

	// Methods
	public:

		void Setup(double simulation_time_komondor, int save_node_logs, int save_agent_logs,
			int print_node_logs, int print_system_logs, int print_agent_logs, const char *nodes_filename,
			const char *script_filename, const char *simulation_code, int seed_console, int agents_enabled, const char *agents_filename);
		void Stop();
		void Start();
		void InputChecker();

		void SetupEnvironmentByReadingConfigFile();
		void GenerateNodesByReadingInputFile(const char *nodes_filename);

		void GenerateAgents(const char *agents_filename, const char *simulation_code_console);
		void GenerateCentralController(const char *agents_filename);

		int GetNumOfLines(const char *nodes_filename);
		int GetNumOfNodes(const char *nodes_filename, int node_type, std::string wlan_code);
		int CheckCentralController(const char *agents_filename);

		void PrintSystemInfo();
		void PrintAllWlansInfo();
		void PrintAllAgentsInfo();
		void PrintCentralControllerInfo();
		void PrintAllNodesInfo(int info_detail_level);
		void PrintMlOperationInfo();
		void WriteSystemInfo(Logger logger);
		void WriteAllWlansInfo(Logger logger, std::string header_str);
		void WriteAllNodesInfo(Logger logger, int info_detail_level,  std::string header_str);
		void WriteAllAgentsInfo(Logger logger, std::string header_str);

	// Public items (to shared with the nodes)
	public:

		Node[] node_container;			///> Container of nodes (i.e., APs, STAs, ...)
		Wlan *wlan_container;			///> Container of WLANs
		TrafficGenerator[]
		   traffic_generator_container; ///> Container of traffic generators (associated to nodes)

		int total_wlans_number;						///> Total number of WLANs
		int total_agents_number;					///> Total number of agents
		int total_controlled_agents_number;		///> Total number of agents attached to the central controller
		int total_nodes_number;					///> Total number of nodes (public accessor for compatibility)

		// Parameters entered per console
		int save_node_logs;					///> Flag for activating the log writting of nodes
		int print_node_logs;				///> Flag for activating the printing of node logs
		int save_agent_logs;				///> Flag for activating the log writting of agents
		int print_agent_logs;				///> Flag for activating the printing of agent logs
		double simulation_time_komondor;	///> Simulation time [s]

		// Parameters regarding system model
		int pdf_backoff;				///> Probability distribution type of the backoff (0: exponential, 1: deterministic)
		int pdf_tx_time;				///> Probability distribution type of the transmission time (0: exponential, 1: deterministic)
		int path_loss_model;			///> Path loss model (0: free-space, 1: Okumura-Hata model - Uban areas)
		int adjacent_channel_model;		///> Co-channel interference model
		int collisions_model;			///> Collisions model
		double constant_per;			///> Constant PER for successful transmissions
		int capture_effect_model;		///> Capture Effect model (default or IEEE 802.11-based)
		int simulation_index;			///> Simulation index for selecting the type of output in scripts

		int agents_enabled;				///> Determined according to the input (for generating agents or not)

		// Public items (to shared with the agents)
		public:

		// Agents info
		Agent[] agent_container;		///> Array containing all the agents
		int num_arms_channel;		///> Number of available channels
		int num_arms_sensitivity;	///> Number of available sensitivity levels
		int num_arms_tx_power;		///> Number of available transmit power levels
		int num_arms_max_bandwidth;		///> Number of available DCB policies

		double *actions_pd;				///> Array of Packet Detect (PD) actions
		double *actions_tx_power;		///> Array of transmission power actions

		// Central controller info
		CentralController[] central_controller;	///> Central Controller object

	// Private items
	private:

		int seed;							///> Simulation seed number
		int print_system_logs;				///> Flag for activating the printing of system logs
		std::string simulation_code;		///> Komondor simulation code
		const char *nodes_input_filename;	///> Filename of the nodes (AP or Deterministic Nodes) input CSV
		const char *agents_input_filename;	///> Filename of the agents input CSV
		FILE *script_output_file;			///> File for the whole input files included in the script TODO
		Logger logger_script;				///> Logger for the script file (containing 1+ simulations) Readable version

		// Auxiliar variables
		int first_line_skiped_flag;		///> Flag for skipping first informative line of input file
		int central_controller_flag; 	///> In order to allow the generation of the central controller

		// Private helper methods for Setup()
		void InitializeConfiguration(double sim_time_console, int save_node_logs_console,
			int save_agent_logs_console, int print_system_logs_console, int print_node_logs_console,
			int print_agent_logs_console, const char *nodes_input_filename_console,
			const char *agents_input_filename_console, int seed_console, int agents_enabled_console);
		void InitializeOutputFiles(const char *script_output_filename, const char *simulation_code_console);
		void ComputeNodeDistancesAndPower();
		void ComputeMaxPowerPerWlan();
		void InitializeTokenAccess();
		void SetupNodeConnections();
		void SetupAgentConnections();
		void PrintConfigurationSummary();

};

/**
 * Setup()
 * @param "sim_time_console" [type double]: simulation observation time [s]
 * @param "save_system_logs_console" [type int]: flag for activating system logs
 * @param "save_node_logs_console" [type int]: flag for activating nodes logs
 * @param "save_agent_logs_console" [type int]: flag for activating agent logs
 * @param "print_system_logs_console" [type int]: flag for activating system prints
 * @param "print_node_logs_console" [type int]: flag for activating nodes prints
 * @param "print_agent_logs_console" [type int]: flag for activating agent prints
 * @param "system_input_filename" [type char*]: filename of the system input CSV
 * @param "nodes_input_filename" [type char*]: filename of the nodes (AP or Deterministic Nodes) input CSV
 * @param "script_output_filename" [type char*]: filename of the output file generated by the script of multiple simulations
 * @param "simulation_code_console" [type char*]: simulation code assigned to current simulation (it is an string)
 * @param "seed_console" [type int]: random seed
 * @param "agents_enabled_console" [type int]: flag indicating that agents are enabled
 * @param "agents_input_filename_console" [type char*]: filename of the agents input CSV
 */
void Komondor :: Setup(double sim_time_console, int save_node_logs_console,
		int save_agent_logs_console, int print_system_logs_console, int print_node_logs_console,
		int print_agent_logs_console, const char *nodes_input_filename_console,
		const char *script_output_filename, const char *simulation_code_console, int seed_console,
		int agents_enabled_console, const char *agents_input_filename_console){

	// Initialize configuration parameters
	InitializeConfiguration(sim_time_console, save_node_logs_console, save_agent_logs_console,
		print_system_logs_console, print_node_logs_console, print_agent_logs_console,
		nodes_input_filename_console, agents_input_filename_console, seed_console, agents_enabled_console);

	// Initialize output files
	InitializeOutputFiles(script_output_filename, simulation_code_console);

	// Read system (environment) configuration file
	SetupEnvironmentByReadingConfigFile();

	// Generate nodes from input file
	GenerateNodesByReadingInputFile(nodes_input_filename);

	// Compute distances and received power between all node pairs
	ComputeNodeDistancesAndPower();

	// Compute maximum power received from each WLAN (for APs)
	ComputeMaxPowerPerWlan();

	// Initialize token-based channel access arrays
	InitializeTokenAccess();

	// Generate agents (if enabled)
	central_controller_flag = 0;
	if (agents_enabled) { 
		GenerateAgents(agents_input_filename, simulation_code_console); 
	}
	// Generate the central controller (if enabled)
	if (agents_enabled && central_controller_flag) { 
		GenerateCentralController(agents_input_filename); 
	}

	// Print configuration summary
	PrintConfigurationSummary();

	// Run input validation
	InputChecker();

	// Setup node connections (traffic generators, notifications, MCS, etc.)
	SetupNodeConnections();

	// Setup agent connections (APs to agents, agents to central controller)
	SetupAgentConnections();
}

/**
 * Initialize configuration parameters from console arguments
 */
void Komondor::InitializeConfiguration(double sim_time_console, int save_node_logs_console,
	int save_agent_logs_console, int print_system_logs_console, int print_node_logs_console,
	int print_agent_logs_console, const char *nodes_input_filename_console,
	const char *agents_input_filename_console, int seed_console, int agents_enabled_console) {

	simulation_time_komondor = sim_time_console;
	save_node_logs = save_node_logs_console;
	save_agent_logs = save_agent_logs_console;
	print_node_logs = print_node_logs_console;
	print_system_logs = print_system_logs_console;
	print_agent_logs = print_agent_logs_console;
	nodes_input_filename = nodes_input_filename_console;
	agents_input_filename = agents_input_filename_console;
	seed = seed_console;
	agents_enabled = agents_enabled_console;
	total_wlans_number = 0;
	total_nodes_number = 0;  // Initialize total nodes number
}

/**
 * Initialize output files for logging and script output
 */
void Komondor::InitializeOutputFiles(const char *script_output_filename, const char *simulation_code_console) {
	if (print_system_logs) printf("\n%s Creating output files\n", LOG_LVL1);
	
	// Remove old simulation log file if it exists
	std::string simulation_filename_remove;
	simulation_filename_remove.append("output/logs_console_").append(simulation_code_console).append(".txt");
	if(remove(simulation_filename_remove.c_str()) == 0){
		if (print_system_logs) printf("%s Simulation output file '%s' found and removed. New one created!\n",
			LOG_LVL2, simulation_filename_remove.c_str());
	} else {
		if (print_system_logs) printf("%s Simulation output file '%s' created!\n",
			LOG_LVL2, simulation_filename_remove.c_str());
	}

	// Ensure output directory exists for script output file
	std::string script_output_dir = PathManager::GetDirectory(script_output_filename);
	if (!script_output_dir.empty() && script_output_dir != "." && !FileManager::DirectoryExists(script_output_dir)) {
		if (!FileManager::CreateDirectoryIfNotExists(script_output_dir)) {
			fprintf(stderr, "%s ERROR: Cannot create output directory: %s\n", LOG_LVL1, script_output_dir.c_str());
			exit(-1);
		}
	}

	// Initialize script output file (readable)
	script_output_file = fopen(script_output_filename, "at");	// Script output is removed when script is executed
	if (script_output_file == nullptr) {
		fprintf(stderr, "%s ERROR: Cannot open script output file: %s\n", LOG_LVL1, script_output_filename);
		fprintf(stderr, "Check that the directory exists and you have write permissions.\n");
		exit(-1);
	}
	logger_script.save_logs = SAVE_LOG;
	logger_script.file = script_output_file;
	fprintf(logger_script.file, "%s KOMONDOR SIMULATION '%s' (seed %d)", LOG_LVL1, simulation_code_console, seed);
}

/**
 * Compute distances and received power between all node pairs
 */
void Komondor::ComputeNodeDistancesAndPower() {
	for(int i = 0; i < total_nodes_number; ++i) {
		node_container[i].distances_array = new double[total_nodes_number];
		node_container[i].received_power_array = new double[total_nodes_number];
		for(int j = 0; j < total_nodes_number; ++j) {
			// Compute and assign distances for each other node
			node_container[i].distances_array[j] = ComputeDistance(node_container[i].x, node_container[i].y,
				node_container[i].z, node_container[j].x, node_container[j].y, node_container[j].z);
			// Compute and assign the received power from each other node
			if(i == j) {
				node_container[i].received_power_array[j] = 0;
			} else {
				node_container[i].received_power_array[j] = ComputePowerReceived(node_container[i].distances_array[j],
					node_container[j].tx_power_default, node_container[i].central_frequency, path_loss_model);
			}
		}
	}
}

/**
 * Compute maximum power received from each WLAN (for APs)
 */
void Komondor::ComputeMaxPowerPerWlan() {
	for(int i = 0; i < total_nodes_number; ++i) {
		if (node_container[i].node_type == NODE_TYPE_AP) {
			node_container[i].max_received_power_in_ap_per_wlan = new double[total_wlans_number];
			for(int j = 0; j < total_wlans_number; ++j) {
				if (strcmp(node_container[i].wlan_code.c_str(), wlan_container[j].wlan_code.c_str()) == 0) {
					// Same WLAN - no interference from own WLAN
					node_container[i].max_received_power_in_ap_per_wlan[j] = 0;
				} else {
					// Different WLAN - find maximum received power from any node in WLAN j
					double max_power_received_per_wlan = -1000;
					for (int k = 0; k < total_nodes_number; ++k) {
						// Check only nodes in WLAN "j"
						if(strcmp(node_container[k].wlan_code.c_str(), wlan_container[j].wlan_code.c_str()) == 0) {
							if (node_container[i].received_power_array[k] > max_power_received_per_wlan) {
								max_power_received_per_wlan = node_container[i].received_power_array[k];
							}
						}
					}
					node_container[i].max_received_power_in_ap_per_wlan[j] = max_power_received_per_wlan;
				}
			}
		}
	}
}

/**
 * Initialize token-based channel access arrays
 */
void Komondor::InitializeTokenAccess() {
	for (int i = 0; i < total_nodes_number; ++i) {
		if (node_container[i].backoff_type == BACKOFF_TOKENIZED){
			node_container[i].token_order_list = new int[total_nodes_number];
			node_container[i].num_missed_tokens_list = new int[total_nodes_number];
			for(int j = 0; j < total_nodes_number; ++j) {
				node_container[i].token_order_list[j] = DEVICE_INACTIVE_FOR_TOKEN;
				node_container[i].num_missed_tokens_list[j] = 0;
			}
		}
	}
}

/**
 * Setup node connections (traffic generators, notifications, MCS, spatial reuse, etc.)
 */
void Komondor::SetupNodeConnections() {
	for(int n = 0; n < total_nodes_number; ++n){
		// Connect traffic generator to node
		connect traffic_generator_container[n].outportNewPacketGenerated, node_container[n].InportNewPacketGenerated;

		// Connect node to all other nodes for notifications
		for(int m = 0; m < total_nodes_number; ++m) {
			// Global connections (all nodes)
			connect node_container[n].outportSelfStartTX, node_container[m].InportSomeNodeStartTX;
			connect node_container[n].outportSelfFinishTX, node_container[m].InportSomeNodeFinishTX;
			connect node_container[n].outportSendLogicalNack, node_container[m].InportNackReceived;

			// Connections for nodes belonging to the same WLAN
			if(strcmp(node_container[n].wlan_code.c_str(), node_container[m].wlan_code.c_str()) == 0 && n != m) {
				// Connections regarding MCS (Modulation and Coding Scheme)
				connect node_container[n].outportAskForTxModulation, node_container[m].InportMCSRequestReceived;
				connect node_container[n].outportAnswerTxModulation, node_container[m].InportMCSResponseReceived;
				
				// Connections regarding changes in the WLAN configuration
				connect node_container[n].outportSetNewWlanConfiguration, node_container[m].InportNewWlanConfigurationReceived;
				
				// Connections for Spatial Reuse (AP to STA)
				if(node_container[n].node_type == NODE_TYPE_AP && node_container[m].node_type == NODE_TYPE_STA) {
					connect node_container[m].outportRequestSpatialReuseConfiguration, node_container[n].InportRequestSpatialReuseConfiguration;
					connect node_container[n].outportNewSpatialReuseConfiguration, node_container[m].InportNewSpatialReuseConfiguration;
				}
			}
		}
	}
}

/**
 * Setup agent connections (APs to agents, agents to central controller)
 */
void Komondor::SetupAgentConnections() {
	// Connect APs to their corresponding agents
	if (agents_enabled) {
		for(int n = 0; n < total_nodes_number; ++n) {
			if (node_container[n].node_type == NODE_TYPE_AP) {
				for(int w = 0; w < total_agents_number; ++w){
					// Connect the agent to the corresponding AP, according to "wlan_code"
					if (strcmp(node_container[n].wlan_code.c_str(), agent_container[w].wlan_code.c_str()) == 0) {
						connect agent_container[w].outportRequestInformationToAp, node_container[n].InportReceivingRequestFromAgent;
						connect node_container[n].outportAnswerToAgent, agent_container[w].InportReceivingInformationFromAp;
						connect agent_container[w].outportSendConfigurationToAp, node_container[n].InportReceiveConfigurationFromAgent;
					}
				}
			}
		}
	}

	// Connect agents to the central controller, if applicable
	if (agents_enabled && central_controller_flag) {
		for(int w = 0; w < total_agents_number; ++w){
			if(agent_container[w].agent_centralized && central_controller[0].controller_on) {
				connect agent_container[w].outportAnswerToController, central_controller[0].InportReceivingInformationFromAgent;
				connect central_controller[0].outportSendCommandToAgent, agent_container[w].InportReceiveCommandFromController;
			}
		}
	}
}

/**
 * Print configuration summary (system, nodes, WLANs, and agents)
 */
void Komondor::PrintConfigurationSummary() {
	if (print_system_logs) {
		printf("%s System configuration: \n", LOG_LVL2);
		PrintSystemInfo();
		printf("%s Wlans generated!\n", LOG_LVL2);
		PrintAllWlansInfo();
		if (print_system_logs) printf("\n");
		printf("%s Nodes generated!\n", LOG_LVL2);
		PrintAllNodesInfo(INFO_DETAIL_LEVEL_2);
		if (print_system_logs) printf("\n");
		if (print_system_logs && agents_enabled) {
			PrintMlOperationInfo();
		}
	}
}

/**
 * Start()
 */
void Komondor :: Start(){
	// Do nothing
};

/**
 * Stop(): called when the simulation is done to  collect and display statistics.
 */
void Komondor :: Stop(){

	printf("\n%s STOP KOMONDOR SIMULATION '%s' (seed %d)", LOG_LVL1, simulation_code.c_str(), seed);

	// Display (in logs and files) statistics of the simulation
	// Use std::vector instead of raw arrays to avoid memory leaks
	std::vector<Performance> performance_per_node(total_nodes_number);
	std::vector<Configuration> configuration_per_node(total_nodes_number);
	for (int i = 0; i < total_nodes_number; ++i) {
		performance_per_node[i] = node_container[i].simulation_performance;
		configuration_per_node[i] = node_container[i].configuration;
	}

	// Generate the output for scripts
	GenerateScriptOutput(simulation_index, performance_per_node.data(), configuration_per_node.data(), logger_script,
		total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor);

	// End of logs
	if (script_output_file != nullptr) {
		fclose(script_output_file);
		script_output_file = nullptr;
	}

	// Clean up arrays allocated in Setup() method that are shared with simulation_performance
	// These arrays are now safe to free because GenerateScriptOutput() has finished using them
	for (int i = 0; i < total_nodes_number; ++i) {
		// Free arrays allocated in komondor_main.cc Setup() method
		if (node_container[i].distances_array != nullptr) {
			delete[] node_container[i].distances_array;
			node_container[i].distances_array = nullptr;
		}
		if (node_container[i].received_power_array != nullptr) {
			delete[] node_container[i].received_power_array;
			node_container[i].received_power_array = nullptr;
		}
		if (node_container[i].max_received_power_in_ap_per_wlan != nullptr) {
			delete[] node_container[i].max_received_power_in_ap_per_wlan;
			node_container[i].max_received_power_in_ap_per_wlan = nullptr;
		}
		if (node_container[i].token_order_list != nullptr) {
			delete[] node_container[i].token_order_list;
			node_container[i].token_order_list = nullptr;
		}
		if (node_container[i].num_missed_tokens_list != nullptr) {
			delete[] node_container[i].num_missed_tokens_list;
			node_container[i].num_missed_tokens_list = nullptr;
		}
		// Free arrays allocated in Node::InitializeVariables() that are shared with simulation_performance
		if (node_container[i].total_time_transmitting_per_channel != nullptr) {
			delete[] node_container[i].total_time_transmitting_per_channel;
			node_container[i].total_time_transmitting_per_channel = nullptr;
		}
		if (node_container[i].last_total_time_transmitting_per_channel != nullptr) {
			delete[] node_container[i].last_total_time_transmitting_per_channel;
			node_container[i].last_total_time_transmitting_per_channel = nullptr;
		}
		if (node_container[i].total_time_lost_per_channel != nullptr) {
			delete[] node_container[i].total_time_lost_per_channel;
			node_container[i].total_time_lost_per_channel = nullptr;
		}
		if (node_container[i].last_total_time_lost_per_channel != nullptr) {
			delete[] node_container[i].last_total_time_lost_per_channel;
			node_container[i].last_total_time_lost_per_channel = nullptr;
		}
		if (node_container[i].total_time_channel_busy_per_channel != nullptr) {
			delete[] node_container[i].total_time_channel_busy_per_channel;
			node_container[i].total_time_channel_busy_per_channel = nullptr;
		}
		if (node_container[i].num_trials_tx_per_num_channels != nullptr) {
			delete[] node_container[i].num_trials_tx_per_num_channels;
			node_container[i].num_trials_tx_per_num_channels = nullptr;
		}
		if (node_container[i].total_time_transmitting_in_num_channels != nullptr) {
			delete[] node_container[i].total_time_transmitting_in_num_channels;
			node_container[i].total_time_transmitting_in_num_channels = nullptr;
		}
		if (node_container[i].total_time_lost_in_num_channels != nullptr) {
			delete[] node_container[i].total_time_lost_in_num_channels;
			node_container[i].total_time_lost_in_num_channels = nullptr;
		}
		if (node_container[i].rssi_per_sta != nullptr) {
			delete[] node_container[i].rssi_per_sta;
			node_container[i].rssi_per_sta = nullptr;
		}
	}

	printf("%s SIMULATION '%s' FINISHED\n", LOG_LVL1, simulation_code.c_str());
	printf("------------------------------------------\n");

};

/**
 * Identify errors in the introduced input to prevent unexpected situations during the simulation
 */
void Komondor::InputChecker() {
	if (print_system_logs) printf("%s Validating input files...\n", LOG_LVL2);

	// Use std::vector instead of C-style arrays for better safety and modern C++
	std::vector<int> nodes_ids(total_nodes_number);
	std::vector<double> nodes_x(total_nodes_number);
	std::vector<double> nodes_y(total_nodes_number);
	std::vector<double> nodes_z(total_nodes_number);

	// Collect node data and validate individual node parameters
	for (int i = 0; i < total_nodes_number; ++i) {
		nodes_ids[i] = node_container[i].node_id;
		nodes_x[i] = node_container[i].x;
		nodes_y[i] = node_container[i].y;
		nodes_z[i] = node_container[i].z;

		// Check the range of transmission power values (min <= default <= max)
		if (node_container[i].tx_power_default > ConvertPower(DBM_TO_PW, MAX_TX_POWER_DBM)
				|| node_container[i].tx_power_default < ConvertPower(DBM_TO_PW, MIN_TX_POWER_DBM)) {
			printf("\nERROR: tx_power values are not properly configured at node in line %d\n"
				"MIN_TX_POWER_DBM = %d\nnode_container[%d].tx_power_default = %f\nMAX_TX_POWER_DBM = %d\n\n",
				i+2, MIN_TX_POWER_DBM, i, ConvertPower(PW_TO_DBM, node_container[i].tx_power_default), MAX_TX_POWER_DBM);
			exit(-1);
		}

		// Check the range of sensitivity values (min <= default <= max)
		const double max_sensitivity_pw = ConvertPower(DBM_TO_PW, MAX_SENSITIVITY_DBM);
		const double min_sensitivity_pw = ConvertPower(DBM_TO_PW, MIN_SENSITIVITY_DBM);
		if (node_container[i].sensitivity_default > max_sensitivity_pw
				|| node_container[i].sensitivity_default < min_sensitivity_pw) {
			printf("\nERROR: sensitivity values are not properly configured at node in line %d\n"
				"sensitivity_default = %f (range: [%f, %f])\n\n", 
				i+2, node_container[i].sensitivity_default, min_sensitivity_pw, max_sensitivity_pw);
			exit(-1);
		}

		// Check the range of channel values (min <= primary <= max)
		if (node_container[i].current_primary_channel > node_container[i].max_channel_allowed
				|| node_container[i].current_primary_channel < node_container[i].min_channel_allowed
				|| node_container[i].min_channel_allowed > node_container[i].max_channel_allowed
				|| node_container[i].current_primary_channel > NUM_CHANNELS_KOMONDOR
				|| node_container[i].min_channel_allowed > (NUM_CHANNELS_KOMONDOR-1)
				|| node_container[i].max_channel_allowed > (NUM_CHANNELS_KOMONDOR-1)) {
			printf("\nERROR: Channels are not properly configured at node in line %d\n\n", i+2);
			exit(-1);
		}
	}

	// Validate node uniqueness (IDs and positions)
	for (int i = 0; i < total_nodes_number; ++i) {
		for (int j = i + 1; j < total_nodes_number; ++j) {
			// Node IDs must be different
			if (nodes_ids[i] == nodes_ids[j]) {
				printf("\nERROR: Nodes in lines %d and %d have the same ID\n\n", i+2, j+2);
				exit(-1);
			}
			// The position of nodes must be different
			if (nodes_x[i] == nodes_x[j] && nodes_y[i] == nodes_y[j] && nodes_z[i] == nodes_z[j]) {
				printf("%s ERROR: Nodes in lines %d and %d are exactly at the same position\n\n", LOG_LVL2, i+2, j+2);
				exit(-1);
			}
		}
	}

	if (print_system_logs) printf("%s Input files validated!\n", LOG_LVL3);
}

/**
 * Set up the Komondor environment by reading the system input file
 * @note Modernized with improved error handling using ArgumentParser
 */
void Komondor::SetupEnvironmentByReadingConfigFile() {
	const std::string config_filename = "../config_models";
	const std::string delimiter = "=";
	
	if (print_system_logs) {
		printf("\n%s Reading system configuration file '%s'...\n", LOG_LVL1, config_filename.c_str());
	}

	FILE* config_file = fopen(config_filename.c_str(), "r");
	if (config_file == nullptr) {
		fprintf(stderr, "%s ERROR: Cannot open configuration file: %s\n", 
			LOG_LVL1, config_filename.c_str());
		exit(-1);
	}

	char line[CHAR_BUFFER_SIZE];
	int ix_param = 0;
	const int expected_params = 6;  // Number of expected configuration parameters
	
	try {
		while (fgets(line, sizeof(line), config_file)) {
			// Ignore comments and empty lines
			if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
				continue;
			}
			
			// Find the delimiter
			char* equals_pos = std::strchr(line, '=');
			if (equals_pos == nullptr) {
				fprintf(stderr, "%s WARNING: Invalid configuration line (missing '='): %s\n",
					LOG_LVL3, line);
				continue;
			}
			
			// Extract value (skip key, get value after '=')
			char* value_str = equals_pos + 1;
			
			// Trim leading whitespace from value
			while (*value_str == ' ' || *value_str == '\t') {
				++value_str;
			}
			
			// Remove trailing whitespace and newline
			std::string value(value_str);
			// Remove trailing whitespace
			while (!value.empty() && (value.back() == ' ' || value.back() == '\t' || 
			                          value.back() == '\n' || value.back() == '\r')) {
				value.pop_back();
			}
			
			// Skip empty values
			if (value.empty()) {
				continue;
			}
			
			// Parse value using ArgumentParser for robust error handling
			int param_value = 0;
			try {
				param_value = ArgumentParser::ParseInt(value, "config_parameter");
			} catch (const ArgumentParseException& e) {
				fprintf(stderr, "%s ERROR: Failed to parse configuration parameter at index %d: %s\n",
					LOG_LVL1, ix_param, e.what());
				fclose(config_file);
				exit(-1);
			}
			
			// Store parameter based on index (maintaining backward compatibility)
			switch (ix_param) {
				case 0:
					path_loss_model = param_value;
					break;
				case 1:
					adjacent_channel_model = param_value;
					break;
				case 2:
					collisions_model = param_value;
					break;
				case 3:
					pdf_backoff = param_value;
					break;
				case 4:
					pdf_tx_time = param_value;
					break;
				case 5:
					simulation_index = param_value;
					break;
				default:
					// Ignore extra parameters
					break;
			}
			
			++ix_param;
			if (ix_param >= expected_params) {
				break;  // All parameters read
			}
		}
		
		fclose(config_file);
		
		if (ix_param < expected_params) {
			fprintf(stderr, "%s WARNING: Expected %d configuration parameters, found %d\n",
				LOG_LVL2, expected_params, ix_param);
		}
		
		if (print_system_logs) {
			printf("%s System environment properly set!\n", LOG_LVL2);
		}
		
	} catch (const std::exception& e) {
		fclose(config_file);
		fprintf(stderr, "%s ERROR: Unexpected error reading configuration: %s\n",
			LOG_LVL1, e.what());
		exit(-1);
	}
}

/* *******************
 * * NODE GENERATION *
 * *******************
 */

/**
 * Generate the nodes deterministically, according to the input nodes file
 * @param "nodes_filename" [type char*]: filename of the nodes input CSV
 */
void Komondor :: GenerateNodesByReadingInputFile(const char *nodes_filename) {

	if (print_system_logs) printf("\n%s Generating nodes DETERMINISTICALLY through NODES input file...\n", LOG_LVL1);

	if (print_system_logs) printf("%s Reading nodes input file '%s'...\n", LOG_LVL2, nodes_filename);

		total_wlans_number = GetNumOfNodes(nodes_filename, NODE_TYPE_AP, ToString(""));
		if (print_system_logs) printf("%s Num. of WLANs detected: %d\n", LOG_LVL3, total_wlans_number);
		wlan_container = new Wlan[total_wlans_number];
		FILE* stream_nodes = fopen(nodes_filename, "r");
		char line_nodes[CHAR_BUFFER_SIZE];
		first_line_skiped_flag = 0;	// Flag for skipping first informative line of input file
		int wlan_ix (0);			// Auxiliar wlan index

		// Identify WLANs
		while (fgets(line_nodes, CHAR_BUFFER_SIZE, stream_nodes)){
			if(!first_line_skiped_flag){	// Skip the first line of the .csv file
				first_line_skiped_flag = 1;
			} else{
				// Node type - use local variable for GetField (needs modifiable C string)
				char* tmp_nodes = strdup(line_nodes);
				int node_type (atoi(GetField(tmp_nodes, IX_NODE_TYPE)));
				if(node_type == NODE_TYPE_AP){	// If node is AP
					// WLAN ID
					wlan_container[wlan_ix].wlan_id = wlan_ix;
					// WLAN code
					free(tmp_nodes);  // Free previous allocation
					tmp_nodes = strdup(line_nodes);
					std::string wlan_code_aux = ToString(GetField(tmp_nodes, IX_WLAN_CODE));
					wlan_container[wlan_ix].wlan_code = wlan_code_aux;
					++wlan_ix;
					free(tmp_nodes);
					tmp_nodes = nullptr;  // Safety
				} else {
					free(tmp_nodes);
					tmp_nodes = nullptr;
				}
			}
		}

		// Get number of STAs in each WLAN
		for(int w = 0; w < total_wlans_number; ++w){
			int num_stas_in_wlan (GetNumOfNodes(nodes_filename, NODE_TYPE_STA, wlan_container[w].wlan_code));
			wlan_container[w].num_stas = num_stas_in_wlan;
			wlan_container[w].SetSizeOfSTAsArray(num_stas_in_wlan);
		}

		// Generate nodes (without wlan item), finish WLAN with ID lists, and set the wlan item of each STA.
		if (print_system_logs) printf("%s Generating nodes...\n", LOG_LVL3);
		total_nodes_number = GetNumOfNodes(nodes_filename, NODE_TYPE_UNKWNOW, ToString(""));
		node_container.SetSize(total_nodes_number);
		traffic_generator_container.SetSize(total_nodes_number);
		stream_nodes = fopen(nodes_filename, "r");
		int node_ix (0);	// Auxiliar index for nodes
		wlan_ix = 0;		// Auxiliar index for WLANs
		first_line_skiped_flag = 0;

		while (fgets(line_nodes, CHAR_BUFFER_SIZE, stream_nodes)){	// For each WLAN

			if(!first_line_skiped_flag){	// Skip the first line of the .csv file
				first_line_skiped_flag = 1;
			} else {
				// Use local variable for GetField (needs modifiable C string)
				// Declare once at the start of the scope
				char* tmp_nodes = nullptr;
				
				// Node ID (auto-assigned)
				node_container[node_ix].node_id = node_ix;
				// Node code
				tmp_nodes = strdup(line_nodes);
				std::string node_code = ToString(GetField(tmp_nodes, IX_NODE_CODE));
				node_container[node_ix].node_code = node_code;
				free(tmp_nodes);
				// Node type
				tmp_nodes = strdup(line_nodes);
				int node_type (atoi(GetField(tmp_nodes, IX_NODE_TYPE)));
				node_container[node_ix].node_type = node_type;
				free(tmp_nodes);
				// WLAN code: add AP or STA ID to corresponding WLAN - convert to string immediately
				tmp_nodes = strdup(line_nodes);
				const char *wlan_code_aux (GetField(tmp_nodes, IX_WLAN_CODE));
				std::string wlan_code = ToString(wlan_code_aux);  // Convert immediately to own the string
				node_container[node_ix].wlan_code = wlan_code;
				free(tmp_nodes);  // Free immediately after converting to string
				for(int w = 0; w < total_wlans_number; ++w){
					if(strcmp(wlan_code.c_str(), wlan_container[w].wlan_code.c_str()) == 0){	// If nodes belong to WLAN
						if(node_container[node_ix].node_type == NODE_TYPE_AP){	// If node is AP
							wlan_container[w].ap_id = node_container[node_ix].node_id;
						} else if (node_container[node_ix].node_type == NODE_TYPE_STA){	// If node is STA
							for(int s = 0; s < wlan_container[w].num_stas; ++s){
								if(wlan_container[w].list_sta_id[s] == NODE_ID_NONE){
									wlan_container[w].list_sta_id[s] = node_container[node_ix].node_id;
									break;
								}
							}
						}
					}
				}
				// Position
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].x = atof(GetField(tmp_nodes, IX_POSITION_X));
				free(tmp_nodes);
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].y = atof(GetField(tmp_nodes, IX_POSITION_Y));
				free(tmp_nodes);
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].z = atof(GetField(tmp_nodes, IX_POSITION_Z));
				free(tmp_nodes);
				// Central frequency in GHz (e.g. 2.4)
				tmp_nodes = strdup(line_nodes);
				const char* central_frequency_char (GetField(tmp_nodes, IX_CENTRAL_FREQ));
				node_container[node_ix].central_frequency = atof(central_frequency_char) * pow(10,9);
				free(tmp_nodes);
				// Channel bonding model
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].current_dcb_policy = atoi(GetField(tmp_nodes, IX_CHANNEL_BONDING_MODEL));
				free(tmp_nodes);
				// Primary channel
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].current_primary_channel = atoi(GetField(tmp_nodes, IX_PRIMARY_CHANNEL));
				free(tmp_nodes);
				// Min channel allowed
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].min_channel_allowed = atoi(GetField(tmp_nodes, IX_MIN_CH_ALLOWED));
				free(tmp_nodes);
				// Max channel allowed
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].max_channel_allowed = atoi(GetField(tmp_nodes, IX_MAX_CH_ALLOWED));
				free(tmp_nodes);
				// Default tx_power
				tmp_nodes = strdup(line_nodes);
				double tx_power_default_dbm (atof(GetField(tmp_nodes, IX_TX_POWER_DEFAULT)));
				node_container[node_ix].tx_power_default = ConvertPower(DBM_TO_PW, tx_power_default_dbm);
				free(tmp_nodes);
				// Default pd threshold
				tmp_nodes = strdup(line_nodes);
				double sensitivity_default_dbm (atoi(GetField(tmp_nodes, IX_PD_DEFAULT)));
				node_container[node_ix].sensitivity_default = ConvertPower(DBM_TO_PW, sensitivity_default_dbm);
				free(tmp_nodes);
				// Traffic model - extract and convert immediately before freeing tmp_nodes
				tmp_nodes = strdup(line_nodes);
				const char* traffic_model_char (GetField(tmp_nodes, IX_TRAFFIC_MODEL));
				int traffic_model = atoi(traffic_model_char);  // Convert immediately
				free(tmp_nodes);
				// Traffic load (packet generation rate) - extract and convert immediately
				tmp_nodes = strdup(line_nodes);
				const char* traffic_load_char (GetField(tmp_nodes, IX_TRAFFIC_LOAD));
				double traffic_load = atof(traffic_load_char);  // Convert immediately
				free(tmp_nodes);
				// Packet length - convert immediately
				tmp_nodes = strdup(line_nodes);
				const char* packet_length_char (GetField(tmp_nodes, IX_PACKET_LENGTH));
				int frame_length = atoi(packet_length_char);
				node_container[node_ix].frame_length = frame_length;
				free(tmp_nodes);
				// Maximum number of aggregated packets - convert immediately
				tmp_nodes = strdup(line_nodes);
				const char* packets_aggregated_char (GetField(tmp_nodes, IX_NUM_PACKETS_AGG));
				int max_num_packets_aggregated = atoi(packets_aggregated_char);
				node_container[node_ix].max_num_packets_aggregated = max_num_packets_aggregated;
				free(tmp_nodes);
				// Capture effect model -  0=default (recommended) or 1=IEEE 802.11-like - convert immediately
				tmp_nodes = strdup(line_nodes);
				int capture_effect_model = atoi(GetField(tmp_nodes, IX_CAPTURE_EFFECT_MODEL));
				node_container[node_ix].capture_effect_model = capture_effect_model;
				free(tmp_nodes);
				// Capture effect threshold in dB - convert immediately
				tmp_nodes = strdup(line_nodes);
				const char* capture_effect_char (GetField(tmp_nodes, IX_CAPTURE_EFFECT_THR));
				double capture_effect_db = atof(capture_effect_char);
				node_container[node_ix].capture_effect = ConvertPower(DB_TO_LINEAR, capture_effect_db);
				free(tmp_nodes);
				// Constant PER
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].constant_per = atof(GetField(tmp_nodes, IX_CONSTANT_PER));
				free(tmp_nodes);
				// PIFS activated
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].pifs_activated = atoi(GetField(tmp_nodes, IX_PIFS_ACTIVATED));
				free(tmp_nodes);
				// BACKOFF TYPE
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].backoff_type = atoi(GetField(tmp_nodes, IX_BACKOFF_TYPE));
				free(tmp_nodes);
				// CW adaptation activated
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].cw_adaptation = atoi(GetField(tmp_nodes, IX_CW_ADAPTATION_FLAG));
				free(tmp_nodes);
				// CW min default
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].cw_min_default = atoi(GetField(tmp_nodes, IX_CW_MIN_DEFAULT));
				free(tmp_nodes);
				// CW max default
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].cw_max_default = atoi(GetField(tmp_nodes, IX_CW_MAX_DEFAULT));
				free(tmp_nodes);
				// CW max stage
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].cw_stage_max = atoi(GetField(tmp_nodes, IX_CW_STAGE_MAX));
				free(tmp_nodes);
				// SPATIAL REUSE parameters - extract and convert immediately
				//  - BSS color
				tmp_nodes = strdup(line_nodes);
				const char* bss_color_char = GetField(tmp_nodes, IX_BSS_COLOR);
				int bss_color = (bss_color_char != NULL && strlen(bss_color_char) > 0) ? atoi(bss_color_char) : -1;
				free(tmp_nodes);
				//  - Spatial Reuse Group (SRG)
				tmp_nodes = strdup(line_nodes);
				const char* srg_char = GetField(tmp_nodes, IX_SRG);
				int srg = (srg_char != NULL && strlen(srg_char) > 0) ? atoi(srg_char) : -1;
				free(tmp_nodes);
				//  - non-SRG OBSS_PD
				tmp_nodes = strdup(line_nodes);
				const char* non_srg_obss_pd_char = GetField(tmp_nodes, IX_NON_SRG_OBSS_PD);
				double non_srg_obss_pd_dbm = (non_srg_obss_pd_char != NULL && strlen(non_srg_obss_pd_char) > 0) ? atof(non_srg_obss_pd_char) : -1.0;
				free(tmp_nodes);
				//  - SRG OBSS_PD
				tmp_nodes = strdup(line_nodes);
				const char* srg_obss_pd_char = GetField(tmp_nodes, IX_SRG_OBSS_PD);
				double srg_obss_pd_dbm = (srg_obss_pd_char != NULL && strlen(srg_obss_pd_char) > 0) ? atof(srg_obss_pd_char) : -1.0;
				free(tmp_nodes);
				// System and models
				node_container[node_ix].simulation_time_komondor = simulation_time_komondor;
				node_container[node_ix].total_wlans_number = total_wlans_number;
				node_container[node_ix].total_nodes_number = total_nodes_number;
				node_container[node_ix].collisions_model = collisions_model;
				node_container[node_ix].save_node_logs = save_node_logs;
				node_container[node_ix].print_node_logs = print_node_logs;
				node_container[node_ix].adjacent_channel_model = adjacent_channel_model;
				node_container[node_ix].pdf_backoff = pdf_backoff;
				node_container[node_ix].path_loss_model = path_loss_model;
				node_container[node_ix].pdf_tx_time = pdf_tx_time;
				node_container[node_ix].simulation_code = simulation_code;
				// SPATIAL REUSE - use already converted values
				if (bss_color >= 0) { // Check if the input file is compliant with SR
					node_container[node_ix].bss_color = bss_color;
					node_container[node_ix].srg = srg;
					node_container[node_ix].non_srg_obss_pd = ConvertPower(DBM_TO_PW, non_srg_obss_pd_dbm);
					node_container[node_ix].srg_obss_pd = ConvertPower(DBM_TO_PW, srg_obss_pd_dbm);
				} else {
					node_container[node_ix].bss_color = -1;
					node_container[node_ix].srg = -1;
					node_container[node_ix].non_srg_obss_pd = -1;
					node_container[node_ix].srg_obss_pd = -1;
				}
				// Traffic generator - use already converted values
				traffic_generator_container[node_ix].node_type = node_type;
				traffic_generator_container[node_ix].node_id = node_ix;
				traffic_generator_container[node_ix].traffic_model = traffic_model;
				node_container[node_ix].traffic_model = traffic_model; // Tell the node in case full buffer model is selected
				traffic_generator_container[node_ix].traffic_load = traffic_load;

				++node_ix;
				// tmp_nodes was already freed after last use (line 763), no need to free again
			}
		}
		// Set corresponding WLAN to each node
		for(int n = 0; n < total_nodes_number; ++n){
			for(int w = 0; w < total_wlans_number; ++w){
				if (strcmp(node_container[n].wlan_code.c_str(), wlan_container[w].wlan_code.c_str()) == 0) {
					node_container[n].wlan = wlan_container[w];
				}
			}
		}

		if (print_system_logs) printf("%s Nodes generated!\n", LOG_LVL3);
}

/**
 * Generate the agents deterministically, according to the input agents file
 * @param "agents_filename" [type char*]: filename of the agents input CSV
 */
void Komondor :: GenerateAgents(const char *agents_filename, const char *simulation_code_console) {

	if (print_system_logs) printf("%s Generating agents...\n", LOG_LVL1);
	if (print_system_logs) printf("%s Reading agents input file '%s'...\n", LOG_LVL2, agents_filename);

	// STEP 1: CHECK IF THERE IS A CC AND PARSE ITS INFORMATION DIFFERENTLY THAN FROM AGENTS
	central_controller_flag = CheckCentralController(agents_filename);
	// STEP 2: SET SIZE OF THE AGENTS CONTAINER
	total_agents_number = GetNumOfLines(agents_filename) - central_controller_flag;
	agent_container.SetSize(total_agents_number);
	if (print_system_logs) printf("%s Num. of agents (WLANs): %d/%d\n", LOG_LVL3, total_agents_number, total_wlans_number);
	// STEP 3: read the input file to determine the action space
	if (print_system_logs) printf("%s Setting action space...\n", LOG_LVL4);
	FILE* stream_agents = fopen(agents_filename, "r");
	char line_agents[CHAR_BUFFER_SIZE];
	first_line_skiped_flag = 0;	// Flag for skipping first informative line of input file
	int agent_ix (0);	// Auxiliary index
	while (fgets(line_agents, CHAR_BUFFER_SIZE, stream_agents)){
		if(!first_line_skiped_flag){	// Skip the first line of the .csv file
			first_line_skiped_flag = 1;
		} else{
			char* tmp_agents = strdup(line_agents);
			const char *wlan_code_aux (GetField(tmp_agents, IX_AGENT_WLAN_CODE));
			std::string wlan_code;
			wlan_code.append(ToString(wlan_code_aux));
			// Skip the line in case we find a Central Controller (CC). Otherwise, read it and initialize the agent
			if (strcmp(wlan_code.c_str(), "NULL") == 0) continue;
			// Find the length of the channel actions array
			tmp_agents = strdup(line_agents);
			const char *channel_values_aux (GetField(tmp_agents, IX_AGENT_CHANNEL_VALUES));
			std::string channel_values_text;
			channel_values_text.append(ToString(channel_values_aux));
			const char *channel_aux = nullptr;
			channel_aux = strtok((char*)channel_values_text.c_str(), ",");
			num_arms_channel = 0;
			while (channel_aux != nullptr) {
				channel_aux = strtok(nullptr, ",");
				++num_arms_channel;
			}
			// Set the length of channel actions to agent's field
			agent_container[agent_ix].num_arms_channel = num_arms_channel;
			// Find the length of the pd actions array
			tmp_agents = strdup(line_agents);
			const char *pd_values_aux (GetField(tmp_agents, IX_AGENT_PD_VALUES));
			std::string pd_values_text;
			pd_values_text.append(ToString(pd_values_aux));
			const char *pd_aux = nullptr;
			pd_aux = strtok((char*)pd_values_text.c_str(), ",");
			num_arms_sensitivity = 0;
			while (pd_aux != nullptr) {
				pd_aux = strtok(nullptr, ",");
				++num_arms_sensitivity;
			}
			// Set the length of sensitivity actions to agent's field
			agent_container[agent_ix].num_arms_sensitivity = num_arms_sensitivity;
			// Find the length of the Tx power actions array
			tmp_agents = strdup(line_agents);
			const char *tx_power_values_aux (GetField(tmp_agents, IX_AGENT_TX_POWER_VALUES));
			std::string tx_power_values_text;
			tx_power_values_text.append(ToString(tx_power_values_aux));
			const char *tx_power_aux = nullptr;
			tx_power_aux = strtok((char*)tx_power_values_text.c_str(), ",");
			num_arms_tx_power = 0;
			while (tx_power_aux != nullptr) {
				tx_power_aux = strtok(nullptr, ",");
				++num_arms_tx_power;
			}
			// Set the length of Tx power actions to agent's field
			agent_container[agent_ix].num_arms_tx_power = num_arms_tx_power;
			// Find the length of the DCB actions actions array
			tmp_agents = strdup(line_agents);
			const char *max_bandwidth_values_aux (GetField(tmp_agents, IX_AGENT_MAX_BANDWIDTH));
			std::string max_bandwidth_values_text;
			max_bandwidth_values_text.append(ToString(max_bandwidth_values_aux));
			const char *max_bandwidth_aux = nullptr;
			max_bandwidth_aux = strtok((char*)max_bandwidth_values_text.c_str(), ",");
			num_arms_max_bandwidth = 0;
			while (max_bandwidth_aux != nullptr) {
				max_bandwidth_aux = strtok(nullptr, ",");
				++num_arms_max_bandwidth;
			}
			// Set the length of max bandwidth to agent's field
			agent_container[agent_ix].num_arms_max_bandwidth = num_arms_max_bandwidth;

			// Set the length of the total actions in the agent (combinations of parameters)
			agent_container[agent_ix].num_arms = num_arms_channel * num_arms_sensitivity
				* num_arms_tx_power * num_arms_max_bandwidth;

			// Set the simulation code for generating output files
			agent_container[agent_ix].simulation_code.append(ToString(simulation_code_console));

			++agent_ix;
			free(tmp_agents);
		}
	}

	if (print_system_logs) printf("%s Action space set!\n", LOG_LVL4);

	// STEP 3: set agents parameters
	if (print_system_logs) printf("%s Setting agents parameters...\n", LOG_LVL4);
	stream_agents = fopen(agents_filename, "r");
	first_line_skiped_flag = 0;		// Flag for skipping first informative line of input file
	agent_ix = 0;	// Auxiliary index
	total_controlled_agents_number = 0;
	while (fgets(line_agents, CHAR_BUFFER_SIZE, stream_agents)){
		if(!first_line_skiped_flag){	// Skip the first line of the .csv file
			first_line_skiped_flag = 1;
		} else{
			// WLAN code
			char* tmp_agents (strdup(line_agents));
			const char *wlan_code_aux (GetField(tmp_agents, IX_AGENT_WLAN_CODE));
			std::string wlan_code;
			wlan_code.append(ToString(wlan_code_aux));
			// Skip the line in case we find a Central Controller (CC). Otherwise, read it and initialize the agent
			if (strcmp(wlan_code.c_str(), "NULL") == 0) {
				continue;
			} else {
				// Agent ID
				agent_container[agent_ix].agent_id = agent_ix;
				agent_container[agent_ix].wlan_code = wlan_code.c_str();
				// WLAN Id
				for(int w=0; w < total_wlans_number; ++w){
					if(strcmp(wlan_container[w].wlan_code.c_str(), agent_container[agent_ix].wlan_code.c_str()) == 0) {
						agent_container[agent_ix].wlan_id = w;
					}
				}
				// Initialize actions and arrays in agents
				agent_container[agent_ix].InitializeAgent();
				// Agent associated to the Central Controller (CC)
				tmp_agents = strdup(line_agents);
				if (tmp_agents == nullptr) {
					fprintf(stderr, "%s ERROR: Memory allocation failed\n", LOG_LVL1);
					exit(-1);
				}
				const char* agent_centralized_str = GetField(tmp_agents, IX_COMMUNICATION_LEVEL);
				int agent_centralized = (agent_centralized_str != nullptr) ? 
					ArgumentParser::ParseInt(std::string(agent_centralized_str), "agent_centralized") : 0;
				agent_container[agent_ix].agent_centralized = agent_centralized;
				free(tmp_agents);
				
				// Check if the central controller has to be created or not
				if(agent_centralized) ++total_controlled_agents_number;
				
				// Time between requests (in seconds)
				tmp_agents = strdup(line_agents);
				if (tmp_agents == nullptr) {
					fprintf(stderr, "%s ERROR: Memory allocation failed\n", LOG_LVL1);
					exit(-1);
				}
				const char* time_between_requests_str = GetField(tmp_agents, IX_AGENT_TIME_BW_REQUESTS);
				double time_between_requests = (time_between_requests_str != nullptr) ? 
					ArgumentParser::ParseDouble(std::string(time_between_requests_str), "time_between_requests") : 0.0;
				agent_container[agent_ix].time_between_requests = time_between_requests;
				free(tmp_agents);
				tmp_agents = nullptr;
				// Channel values
				tmp_agents = strdup(line_agents);
				std::string channel_values_text = ToString(GetField(tmp_agents, IX_AGENT_CHANNEL_VALUES));
				// Fill the channel actions array
				// Use std::string for safer string manipulation
				std::string channel_values_text_copy = channel_values_text;
				char *channel_aux_2 = nullptr;
				channel_aux_2 = strtok(&channel_values_text_copy[0], ",");
				int ix = 0;
				while (channel_aux_2 != nullptr) {
					int a = ArgumentParser::ParseInt(std::string(channel_aux_2), "channel_value");
					agent_container[agent_ix].list_of_channels[ix] = a;
					channel_aux_2 = strtok(nullptr, ",");
					++ix;
				}
				// If no channel actions are provided, use the primary and range already assigned to the BSS
				if (agent_container[agent_ix].num_arms_channel == 1) {
					int wlan_id_aux(agent_container[agent_ix].wlan_id);
					agent_container[agent_ix].list_of_channels[0] = node_container[wlan_container[wlan_id_aux].ap_id].current_primary_channel;
				}
				// sensitivity values
				tmp_agents = strdup(line_agents);
				std::string pd_values_text = ToString(GetField(tmp_agents, IX_AGENT_PD_VALUES));
				// Fill the sensitivity actions array
				std::string pd_values_text_copy = pd_values_text;
				char *pd_aux_2 = nullptr;
				pd_aux_2 = strtok(&pd_values_text_copy[0], ",");
				ix = 0;
				while (pd_aux_2 != nullptr) {
					int a = ArgumentParser::ParseInt(std::string(pd_aux_2), "pd_value");
					agent_container[agent_ix].list_of_pd_values[ix] = ConvertPower(DBM_TO_PW, a);
					pd_aux_2 = strtok(nullptr, ",");
					++ix;
				}
				// Tx Power values
				tmp_agents = strdup(line_agents);
				std::string tx_power_values_text = ToString(GetField(tmp_agents, IX_AGENT_TX_POWER_VALUES));
				// Fill the TX power actions array
				std::string tx_power_values_text_copy = tx_power_values_text;
				char *tx_power_aux_2 = nullptr;
				tx_power_aux_2 = strtok(&tx_power_values_text_copy[0], ",");
				ix = 0;
				while (tx_power_aux_2 != nullptr) {
					int a = ArgumentParser::ParseInt(std::string(tx_power_aux_2), "tx_power_value");
					agent_container[agent_ix].list_of_tx_power_values[ix] = ConvertPower(DBM_TO_PW, a);
					tx_power_aux_2 = strtok(nullptr, ",");
					++ix;
				}
				// Max bandwidth values
				tmp_agents = strdup(line_agents);
				std::string max_bandwidth_values_text = ToString(GetField(tmp_agents, IX_AGENT_MAX_BANDWIDTH));
				// Fill the max bandwidth actions array
				std::string max_bandwidth_values_text_copy = max_bandwidth_values_text;
				char *max_bandwidth_aux_2 = nullptr;
				max_bandwidth_aux_2 = strtok(&max_bandwidth_values_text_copy[0], ",");
				ix = 0;
				while (max_bandwidth_aux_2 != nullptr) {
					int a = ArgumentParser::ParseInt(std::string(max_bandwidth_aux_2), "max_bandwidth_value");
					agent_container[agent_ix].list_of_max_bandwidth[ix] = a;
					max_bandwidth_aux_2 = strtok(nullptr, ",");
					++ix;
				}
				// Type of reward
				tmp_agents = strdup(line_agents);
				if (tmp_agents == nullptr) {
					fprintf(stderr, "%s ERROR: Memory allocation failed\n", LOG_LVL1);
					free(tmp_agents);
					exit(-1);
				}
				const char* type_of_reward_str = GetField(tmp_agents, IX_AGENT_TYPE_OF_REWARD);
				int type_of_reward = (type_of_reward_str != nullptr) ? 
					ArgumentParser::ParseInt(std::string(type_of_reward_str), "type_of_reward") : 0;
				agent_container[agent_ix].type_of_reward = type_of_reward;
				free(tmp_agents);
				
				// Learning mechanism
				tmp_agents = strdup(line_agents);
				if (tmp_agents == nullptr) {
					fprintf(stderr, "%s ERROR: Memory allocation failed\n", LOG_LVL1);
					exit(-1);
				}
				const char* learning_mechanism_str = GetField(tmp_agents, IX_AGENT_LEARNING_MECHANISM);
				int learning_mechanism = (learning_mechanism_str != nullptr) ? 
					ArgumentParser::ParseInt(std::string(learning_mechanism_str), "learning_mechanism") : 0;
				agent_container[agent_ix].learning_mechanism = learning_mechanism;
				free(tmp_agents);
				
				// Selected strategy
				tmp_agents = strdup(line_agents);
				if (tmp_agents == nullptr) {
					fprintf(stderr, "%s ERROR: Memory allocation failed\n", LOG_LVL1);
					exit(-1);
				}
				const char* action_selection_strategy_str = GetField(tmp_agents, IX_AGENT_SELECTED_STRATEGY);
				int action_selection_strategy = (action_selection_strategy_str != nullptr) ? 
					ArgumentParser::ParseInt(std::string(action_selection_strategy_str), "action_selection_strategy") : 0;
				agent_container[agent_ix].action_selection_strategy = action_selection_strategy;
				free(tmp_agents);
				tmp_agents = nullptr;
				// Other information
				agent_container[agent_ix].save_agent_logs = save_agent_logs;
				agent_container[agent_ix].print_agent_logs = print_agent_logs;
				agent_container[agent_ix].num_stas = wlan_container[agent_container[agent_ix].wlan_id].num_stas;
				// TRICKY - USE THE FIRST ELEMENT INT HE LIST OF PD VALUES AS THE MARGIN
				if(agent_container[agent_ix].learning_mechanism == RTOT_ALGORITHM) {
					agent_container[agent_ix].margin_rtot = agent_container[agent_ix].list_of_pd_values[0];
				}

				//agent_container[agent_ix].PrintAgentInfo();

				++agent_ix;

			}
			free(tmp_agents);
		}
	}
	if (print_system_logs) printf("%s Agents parameters set!\n", LOG_LVL4);

}

/**
 * Generate the central controller (if active), according to the input agents file
 * @param "agents_filename" [type char*]: filename of the agents input CSV
 */
void Komondor :: GenerateCentralController(const char *agents_filename) {

	if (print_system_logs) printf("%s Generating the Central Controller...\n", LOG_LVL1);
	// So far, we consider a single controller. For scalability purposes, the CC must be declared as an array
	if (central_controller_flag) central_controller.SetSize(1);
	if (total_controlled_agents_number > 0) {	// Check that the CC has one or more agents attached
		central_controller[0].controller_on = TRUE;
		central_controller[0].agents_number = total_controlled_agents_number;
		central_controller[0].wlans_number = total_wlans_number;
		int max_number_of_actions(0);
		for (int agent_ix = 0; agent_ix < total_controlled_agents_number; ++agent_ix) {
			if(agent_container[agent_ix].num_arms > max_number_of_actions) max_number_of_actions = agent_container[agent_ix].num_arms;
		}
		central_controller[0].max_number_of_actions = max_number_of_actions;
		// Initialize the CC
		central_controller[0].InitializeCentralController();
		int *agents_list;
		agents_list = new int[total_controlled_agents_number];
		int agent_list_ix (0);					// Index considering the agents attached to the central entity

//		double max_time_between_requests (0);	// To determine the maximum time between requests for agents
		// Check which agents are attached to the central controller
		for (int agent_ix = 0; agent_ix < total_controlled_agents_number; ++agent_ix) {
			if(agent_container[agent_ix].agent_centralized) {
				// Add agent id to list of agents attached to the controller
				agents_list[agent_list_ix] = agent_container[agent_ix].agent_id;
				agent_container[agent_ix].controller_on = central_controller[0].controller_on;
				central_controller[0].num_arms_per_agent[agent_ix] = agent_container[agent_ix].num_arms;
//				double agent_time_between_requests (agent_container[agent_list_ix].time_between_requests);
//				// Store the maximum time between requests
//				if (agent_time_between_requests > max_time_between_requests) {
//					central_controller[0].time_between_requests = agent_time_between_requests;
//				}
				++agent_list_ix;
			}
		}

		// The overall "time between requests" is set to the maximum among all the agents
		central_controller[0].list_of_agents = agents_list;
		// Initialize the CC with parameters from the agents input file
		FILE* stream_cc = fopen(agents_filename, "r");
		char line_agents[CHAR_BUFFER_SIZE];
		char* tmp_cc (strdup(line_agents));
		first_line_skiped_flag = 0;		// Flag for skipping first informative line of input file
		while (fgets(line_agents, CHAR_BUFFER_SIZE, stream_cc)){
			if(!first_line_skiped_flag){
				first_line_skiped_flag = 1;
			} else{
				tmp_cc = strdup(line_agents);
				const char *wlan_code_aux (GetField(tmp_cc, IX_AGENT_WLAN_CODE));
				std::string wlan_code;
				wlan_code.append(ToString(wlan_code_aux));
				// Skip the line in case we find a Central Controller (CC). Otherwise, read it and initialize the agent
				if (strcmp(wlan_code.c_str(), "NULL") == 0) {
					// Time between requests
					tmp_cc = strdup(line_agents);
					double time_between_requests (atoi(GetField(tmp_cc, IX_AGENT_TIME_BW_REQUESTS)));
					central_controller[0].time_between_requests = time_between_requests;
					// Type of reward
					tmp_cc = strdup(line_agents);
					int type_of_reward (atoi(GetField(tmp_cc, IX_AGENT_TYPE_OF_REWARD)));
					central_controller[0].type_of_reward = type_of_reward;
					// Learning mechanism
					tmp_cc = strdup(line_agents);
					int learning_mechanism (atoi(GetField(tmp_cc, IX_AGENT_LEARNING_MECHANISM)));
					central_controller[0].learning_mechanism = learning_mechanism;
					// Selected strategy
					tmp_cc = strdup(line_agents);
					int action_selection_strategy (atoi(GetField(tmp_cc, IX_AGENT_SELECTED_STRATEGY)));
					central_controller[0].action_selection_strategy = action_selection_strategy;
					// Find the length of the channel actions array
					tmp_cc = strdup(line_agents);
					const char *channel_values_aux (GetField(tmp_cc, IX_AGENT_CHANNEL_VALUES));
					std::string channel_values_text;
					channel_values_text.append(ToString(channel_values_aux));
					std::string channel_values_text_copy = channel_values_text;
					const char *channels_aux = nullptr;
					if (!channel_values_text_copy.empty()) {
						channels_aux = strtok(&channel_values_text_copy[0], ",");
					}
					int num_arms_channels = 0;
					while (channels_aux != nullptr) {
						channels_aux = strtok(nullptr, ",");
						++num_arms_channels;
					}
					free(tmp_cc);
					break;	// Don't read all the other lines (entailed for agents)
				} else {
					continue; // Keep reading until finding "NULL", which indicated the CC line
				}
			}
		}

		// System logs
		central_controller[0].save_controller_logs = save_agent_logs;
		central_controller[0].print_controller_logs = print_agent_logs;
		central_controller[0].total_nodes_number = total_nodes_number;

	} else {
		printf("%s WARNING: THE CENTRAL CONTROLLER DOES NOT HAVE ANY ATTACHED AGENT! CHECK YOUR AGENTS' INPUT FILE\n", LOG_LVL2);
		central_controller[0].controller_on = FALSE;
	}

	central_controller[0].PrintControllerInfo();

}

/***************************/
/* LOG AND DEBUG FUNCTIONS */
/***************************/

/**
 * Print the Komondor's environment general information
 */
void Komondor :: PrintSystemInfo(){
	if (print_system_logs){
		printf("%s total_nodes_number = %d\n", LOG_LVL3, total_nodes_number);
		printf("%s pdf_backoff = %d\n", LOG_LVL3, pdf_backoff);
		printf("%s pdf_tx_time = %d\n", LOG_LVL3, pdf_tx_time);
		printf("%s path_loss_model = %d\n", LOG_LVL3, path_loss_model);
		printf("%s adjacent_channel_model = %d\n", LOG_LVL3, adjacent_channel_model);
		printf("%s collisions_model = %d\n", LOG_LVL3, collisions_model);
		printf("\n");
	}
}

/**
 * Write the Komondor's environment general information into an output file
 * @param "logger" [type Logger]: logger object that writes logs into a file
 */
void Komondor :: WriteSystemInfo(Logger logger){
	fprintf(logger.file, "%s total_nodes_number = %d\n", LOG_LVL3, total_nodes_number);
	fprintf(logger.file, "%s pdf_backoff = %d\n", LOG_LVL3, pdf_backoff);
	fprintf(logger.file, "%s pdf_tx_time = %d\n", LOG_LVL3, pdf_tx_time);
	fprintf(logger.file, "%s path_loss_model = %d\n", LOG_LVL3, path_loss_model);
	fprintf(logger.file, "%s adjacent_channel_model = %d\n", LOG_LVL3, adjacent_channel_model);
	fprintf(logger.file, "%s collisions_model = %d\n", LOG_LVL3, collisions_model);
}

/**
 * Print the information of all the nodes
 * @param "info_detail_level" [type int]: level of detail of the written logs
 */
void Komondor :: PrintAllNodesInfo(int info_detail_level){
	for(int n = 0; n < total_nodes_number; ++n ){
		node_container[n].PrintNodeInfo(info_detail_level);
	}
}

/**
 * Print the information of all the WLANs
 */
void Komondor :: PrintAllWlansInfo(){
	for(int w = 0; w < total_wlans_number; ++w){
		wlan_container[w].PrintWlanInfo();
	}
}

/**
 * Print the information of all the agents
 */
void Komondor :: PrintMlOperationInfo(){
	printf("%s Agents generated!\n\n", LOG_LVL2);
	for(int a = 0; a < total_agents_number; ++a ){
		agent_container[a].PrintAgentInfo();
	}
	if (central_controller_flag) {
		printf("%s Central Controller generated!\n\n", LOG_LVL2);
		central_controller[0].PrintControllerInfo();
	}
}

/**
 * Write the information of all the WLANs into a file
 * @param "logger" [type Logger]: logger object that writes logs into a file
 * @param "header_str" [type std::string]: header string
 */
void Komondor :: WriteAllWlansInfo(Logger logger, std::string header_str){
	for(int w = 0; w < total_wlans_number; ++w){
		wlan_container[w].WriteWlanInfo(logger, header_str.c_str());
	}
}

/**
 * Write the information of all the nodes into a file
 * @param "logger" [type Logger]: logger object that writes logs into a file
 * @param "info_detail_level" [type int]: level of detail of the written logs
 * @param "header_str" [type std::string]: header string
 */
void Komondor :: WriteAllNodesInfo(Logger logger, int info_detail_level, std::string header_str){
	for(int n = 0; n < total_nodes_number; ++n){
		node_container[n].WriteNodeInfo(logger, info_detail_level, header_str.c_str());
	}
}

/**
 * Write the information of all the agents into a file
 * @param "logger" [type Logger]: logger object that writes logs into a file
 * @param "header_str" [type std::string]: header string
 */
void Komondor :: WriteAllAgentsInfo(Logger logger, std::string header_str){
	for(int n = 0; n < total_agents_number; ++n){
		agent_container[n].WriteAgentInfo(logger, header_str.c_str());
	}
	if (central_controller_flag) {
		central_controller[0].WriteControllerInfo(logger);
	}
}


/*******************/
/* FILES FUNCTIONS */
/*******************/

/**
 * Return a field corresponding to a given index from a CSV file
 * @param line Line of the CSV (modified by strtok, so non-const)
 * @param num Field number (index, 1-based)
 * @return Pointer to the field corresponding to the introduced index, or nullptr if not found
 */
const char* GetField(char* line, int num) {
    if (line == nullptr || num <= 0) {
        return nullptr;
    }
    
    const char* tok = nullptr;
    for (tok = strtok(line, ";");
            tok != nullptr && *tok != '\0';
            tok = strtok(nullptr, ";\n")) {
        if (--num == 0) {
            return tok;
        }
    }
    return nullptr;
}

/**
 * Return the number of lines of a csv file
 * @param "filename" [type char*]: CSV filename
 * @return "num_lines" [type int]: number of lines in the csv file
 */
int Komondor :: GetNumOfLines(const char *filename){
	int num_lines (0);
	// Nodes file
	FILE* stream = fopen(filename, "r");
	if (!stream){
		printf("Nodes configuration file %s not found!\n", filename);
		exit(-1);
	}
	char line[CHAR_BUFFER_SIZE];
	while (fgets(line, CHAR_BUFFER_SIZE, stream))
	{
		++num_lines;
	}
	num_lines--;
	fclose(stream);
	return num_lines;
}

/**
 * Return the number of nodes of a given type (0: AP, 1: STA, 2: Free Node)
 * @param "nodes_filename" [type char*]: nodes configuration filename
 * @param "node_type" [type int]: type of node to consider in the counting
 * @param "wlan_code" [type std::string]: code of the wlan to consider in the counting
 * @return "num_nodes" [type int]: number of nodes of the introduced type in the indicated WLAN
 */
int Komondor :: GetNumOfNodes(const char *nodes_filename, int node_type, std::string wlan_code){

	int num_nodes(0);
	char line_nodes[CHAR_BUFFER_SIZE];
	first_line_skiped_flag = 0;
	int type_found;
	std::string wlan_code_found;

	FILE* stream_nodes = fopen(nodes_filename, "r");

	if (!stream_nodes){
		printf("[MAIN] ERROR: Nodes configuration file %s not found!\n", nodes_filename);
		exit(-1);
	}

	if(node_type == NODE_TYPE_UNKWNOW){	// Count all type of nodes

		num_nodes = GetNumOfLines(nodes_filename);

	} else {	// Count specific nodes

		while (fgets(line_nodes, CHAR_BUFFER_SIZE, stream_nodes)){

			if(!first_line_skiped_flag){

				first_line_skiped_flag = 1;

			} else{
				// Use local variable for GetField (needs modifiable C string)
				char* tmp_nodes = nullptr;

				// Node type
				tmp_nodes = strdup(line_nodes);
				type_found = atof(GetField(tmp_nodes, IX_NODE_TYPE));
				free(tmp_nodes);
				tmp_nodes = nullptr;

				// WLAN code
				tmp_nodes = strdup(line_nodes);
				wlan_code_found = ToString(GetField(tmp_nodes, IX_WLAN_CODE));
				free(tmp_nodes);
				tmp_nodes = nullptr;

				if(wlan_code.compare(ToString("")) > 0){
					if(type_found == node_type && strcmp(wlan_code_found.c_str(), wlan_code.c_str()) == 0) ++num_nodes;
				} else {
					if(type_found == node_type) ++num_nodes;
				}
			}
		}
	}

	fclose(stream_nodes);
	return num_nodes;
}

/**
 * Return TRUE if there is a Central Controller declared. FALSE, otherwise.
 * The CC is declared in any line by setting the WLAN_CODE field to "NULL"
 * @param "nodes_filename" [type char*]: nodes configuration filename
 * @return "presence_central_cotnroller" [type bool]: flag indicating whether a CC is present or not
 */
int Komondor :: CheckCentralController(const char *agents_filename){
	int presence_central_cotnroller(FALSE);
	FILE* stream_agents = fopen(agents_filename, "r");
	char line_agents[CHAR_BUFFER_SIZE];
	first_line_skiped_flag = 0;	// Flag for skipping first informative line of input file
	while (fgets(line_agents, CHAR_BUFFER_SIZE, stream_agents)){
		if(!first_line_skiped_flag){	// Skip the first line of the .csv file
			first_line_skiped_flag = 1;
		} else{
			// WLAN code
			char* tmp_agents (strdup(line_agents));
			const char *wlan_code_aux (GetField(tmp_agents, IX_AGENT_WLAN_CODE));
			std::string wlan_code;
			wlan_code.append(ToString(wlan_code_aux));
			// Skip the line in case we find a Central Controller (CC). Otherwise, read it and initialize the agent
			if (strcmp(wlan_code.c_str(), "NULL") == 0) {
				presence_central_cotnroller = TRUE;
			}
		}
	}
	return presence_central_cotnroller;
}

/**********/
/* main() */
/**********/
int main(int argc, char *argv[]){

	printf("\n");
	printf("*************************************************************************************\n");
	printf("%s KOMONDOR Wireless Network Simulator\n", LOG_LVL1);
	printf("%s Copyright (C) 2017-2022, and GNU GPL'd, by Sergio Barrachina & Francesc Wilhelmi\n", LOG_LVL1);
	printf("%s GitHub repository: https://github.com/wn-upf/Komondor\n", LOG_LVL2);
	printf("*************************************************************************************\n");
	printf("\n\n");

	// Input variables
	const char *nodes_input_filename = nullptr;
	const char *agents_input_filename = nullptr;
	std::string script_output_filename;
	std::string simulation_code;
	int save_node_logs = 0;
	int save_agent_logs = 0;
	int print_system_logs = 1;
	int print_node_logs = 1;
	int print_agent_logs = 0;
	double sim_time = 0.0;
	int seed = 0;
	int agents_enabled = FALSE;

	// Usage message for error reporting
	const std::string usage_message = 
		"Usage:\n"
		"  For FULL configuration with agents:\n"
		"    ./komondor_main <nodes_file> <agents_file> <output_file> <sim_code> "
		"<save_node_logs> <save_agent_logs> <print_system_logs> <print_node_logs> "
		"<print_agent_logs> <sim_time> <seed>\n"
		"  For FULL configuration without agents:\n"
		"    ./komondor_main <nodes_file> <output_file> <sim_code> <save_node_logs> "
		"<print_system_logs> <print_node_logs> <sim_time> <seed>\n"
		"  For PARTIAL configuration:\n"
		"    ./komondor_main <nodes_file> <sim_time> <seed>\n"
		"  For PARTIAL configuration (scripts):\n"
		"    ./komondor_main <nodes_file> <sim_code> <sim_time> <seed>\n";

	try {
		// Parse arguments based on argc
		// Note: total_nodes_number is now a member of Komondor class, initialized in Setup()
		if(argc == NUM_FULL_ARGUMENTS_CONSOLE){	// Full configuration entered per console
			nodes_input_filename = argv[1];
			agents_input_filename = argv[2];
			
			// Validate input files
			ArgumentParser::ValidateFileExists(nodes_input_filename, "nodes_input_filename");
			ArgumentParser::ValidateFileExists(agents_input_filename, "agents_input_filename");
			
			// Parse string arguments
			script_output_filename = std::string(argv[3]);
			simulation_code = std::string(argv[4]);
			
			// Parse and validate integer flags (0 or 1)
			save_node_logs = ArgumentParser::ParseIntRange(std::string(argv[5]), 0, 1, "save_node_logs");
			save_agent_logs = ArgumentParser::ParseIntRange(std::string(argv[6]), 0, 1, "save_agent_logs");
			print_system_logs = ArgumentParser::ParseIntRange(std::string(argv[7]), 0, 1, "print_system_logs");
			print_node_logs = ArgumentParser::ParseIntRange(std::string(argv[8]), 0, 1, "print_node_logs");
			print_agent_logs = ArgumentParser::ParseIntRange(std::string(argv[9]), 0, 1, "print_agent_logs");
			
			// Parse and validate simulation time (must be positive)
			sim_time = ArgumentParser::ParseDoubleRange(std::string(argv[10]), 0.0, 1e9, "sim_time");
			if (sim_time <= 0.0) {
				throw ArgumentParseException("sim_time must be positive, got: " + std::string(argv[10]));
			}
			
			// Parse and validate seed (must be non-negative)
			seed = ArgumentParser::ParseIntRange(std::string(argv[11]), 0, INT_MAX, "seed");
			
			// Enable the operation of agents
			agents_enabled = TRUE;
			if (print_system_logs) printf("%s FULL configuration entered per console (AGENTS ENABLED).\n", LOG_LVL1);
			
		} else if(argc == NUM_FULL_ARGUMENTS_CONSOLE_NO_AGENTS){	// Configuration without agents
			nodes_input_filename = argv[1];
			
			// Validate input file
			ArgumentParser::ValidateFileExists(nodes_input_filename, "nodes_input_filename");
			
			// Parse string arguments
			script_output_filename = std::string(argv[2]);
			simulation_code = std::string(argv[3]);
			
			// Parse and validate integer flags (0 or 1)
			save_node_logs = ArgumentParser::ParseIntRange(std::string(argv[4]), 0, 1, "save_node_logs");
			print_system_logs = ArgumentParser::ParseIntRange(std::string(argv[5]), 0, 1, "print_system_logs");
			print_node_logs = ArgumentParser::ParseIntRange(std::string(argv[6]), 0, 1, "print_node_logs");
			
			// Parse and validate simulation time (must be positive)
			sim_time = ArgumentParser::ParseDoubleRange(std::string(argv[7]), 0.0, 1e9, "sim_time");
			if (sim_time <= 0.0) {
				throw ArgumentParseException("sim_time must be positive, got: " + std::string(argv[7]));
			}
			
			// Parse and validate seed (must be non-negative)
			seed = ArgumentParser::ParseIntRange(std::string(argv[8]), 0, INT_MAX, "seed");
			
			// Disable the operation of agents
			agents_enabled = FALSE;
			agents_input_filename = nullptr;
			if (print_system_logs) printf("%s FULL configuration entered per console (AGENTS DISABLED).\n", LOG_LVL1);
			
		} else if(argc == NUM_PARTIAL_ARGUMENTS_CONSOLE) {	// Partial configuration entered per console
			nodes_input_filename = argv[1];
			
			// Validate input file
			ArgumentParser::ValidateFileExists(nodes_input_filename, "nodes_input_filename");
			
			// Parse and validate simulation time (must be positive)
			sim_time = ArgumentParser::ParseDoubleRange(std::string(argv[2]), 0.0, 1e9, "sim_time");
			if (sim_time <= 0.0) {
				throw ArgumentParseException("sim_time must be positive, got: " + std::string(argv[2]));
			}
			
			// Parse and validate seed (must be non-negative)
			seed = ArgumentParser::ParseIntRange(std::string(argv[3]), 0, INT_MAX, "seed");
			
			// Default values
			script_output_filename = std::string(DEFAULT_SCRIPT_FILENAME);
			simulation_code = std::string(DEFAULT_SIMULATION_CODE);
			save_node_logs = DEFAULT_WRITE_NODE_LOGS;
			print_system_logs = DEFAULT_PRINT_SYSTEM_LOGS;
			print_node_logs = DEFAULT_PRINT_NODE_LOGS;
			save_agent_logs = 0;
			print_agent_logs = 0;
			
			// Disable the operation of agents
			agents_enabled = FALSE;
			agents_input_filename = nullptr;
			if (print_system_logs) printf("%s PARTIAL configuration entered per console. "
				"Some parameters are set by DEFAULT.\n", LOG_LVL1);
				
		} else if(argc == NUM_PARTIAL_ARGUMENTS_SCRIPT) {	// Partial configuration entered per console (useful for scripts)
			nodes_input_filename = argv[1];
			
			// Validate input file
			ArgumentParser::ValidateFileExists(nodes_input_filename, "nodes_input_filename");
			
			// Parse string arguments
			simulation_code = std::string(argv[2]);	// For scripts --> useful to identify simulations
			
			// Parse and validate simulation time (must be positive)
			sim_time = ArgumentParser::ParseDoubleRange(std::string(argv[3]), 0.0, 1e9, "sim_time");
			if (sim_time <= 0.0) {
				throw ArgumentParseException("sim_time must be positive, got: " + std::string(argv[3]));
			}
			
			// Parse and validate seed (must be non-negative)
			seed = ArgumentParser::ParseIntRange(std::string(argv[4]), 0, INT_MAX, "seed");
			
			// Default values
			script_output_filename = std::string(DEFAULT_SCRIPT_FILENAME);
			save_node_logs = DEFAULT_WRITE_NODE_LOGS;
			print_system_logs = DEFAULT_PRINT_SYSTEM_LOGS;
			print_node_logs = DEFAULT_PRINT_NODE_LOGS;
			save_agent_logs = 0;
			print_agent_logs = 0;
			
			// Disable the operation of agents
			agents_enabled = FALSE;
			agents_input_filename = nullptr;
			if (print_system_logs) printf("%s PARTIAL configuration entered per script. "
				"Some parameters are set by DEFAULT.\n", LOG_LVL1);
				
		} else {
			// Invalid number of arguments
			fprintf(stderr, "%s ERROR: Invalid number of arguments. Expected %d, %d, %d, or %d arguments, got %d\n",
				LOG_LVL1, NUM_FULL_ARGUMENTS_CONSOLE, NUM_FULL_ARGUMENTS_CONSOLE_NO_AGENTS,
				NUM_PARTIAL_ARGUMENTS_CONSOLE, NUM_PARTIAL_ARGUMENTS_SCRIPT, argc);
			fprintf(stderr, "%s\n", usage_message.c_str());
			return 1;
		}

		// Print configuration
		if (print_system_logs) {
			printf("%s Komondor input configuration:\n", LOG_LVL1);
			printf("%s nodes_input_filename: %s\n", LOG_LVL2, nodes_input_filename);
			printf("%s agents_enabled: %d\n", LOG_LVL2, agents_enabled);
			if (agents_enabled && agents_input_filename != nullptr) { 
				printf("%s agents_input_filename: %s\n", LOG_LVL2, agents_input_filename); 
			}
			printf("%s script_output_filename: %s\n", LOG_LVL2, script_output_filename.c_str());
			printf("%s simulation_code: %s\n", LOG_LVL2, simulation_code.c_str());
			printf("%s save_node_logs: %d\n", LOG_LVL2, save_node_logs);
			printf("%s save_agent_logs: %d\n", LOG_LVL2, save_agent_logs);
			printf("%s print_system_logs: %d\n", LOG_LVL2, print_system_logs);
			printf("%s print_node_logs: %d\n", LOG_LVL2, print_node_logs);
			printf("%s print_agent_logs: %d\n", LOG_LVL2, print_agent_logs);
			printf("%s sim_time: %f s\n", LOG_LVL2, sim_time);
			printf("%s seed: %d\n", LOG_LVL2, seed);
		}

		// Create output directory if not exists
		std::string output_dir = PathManager::GetOutputDirectory();
		if (!FileManager::DirectoryExists(output_dir)) {
			if (print_system_logs) {
				printf("- Output folder does not exist --> creating it...\n");
			}
			if (!FileManager::CreateDirectoryIfNotExists(output_dir)) {
				fprintf(stderr, "%s ERROR: Output folder could not be created: %s\n"
					"Check permissions and try again.\n", LOG_LVL1, output_dir.c_str());
				return 1;
			}
			if (print_system_logs) {
				printf("- Output folder successfully created: %s\n", output_dir.c_str());
			}
		}

		// Generate a Komondor component to start the simulation
		Komondor komondor_simulation;
		komondor_simulation.Seed = seed;
		srand(seed); // Needed for ensuring randomness dependency on seed
		komondor_simulation.StopTime(sim_time);
		komondor_simulation.Setup(sim_time, save_node_logs, save_agent_logs, print_system_logs,
			print_node_logs, print_agent_logs, nodes_input_filename, script_output_filename.c_str(),
			simulation_code.c_str(), seed, agents_enabled, agents_input_filename);

		printf("------------------------------------------\n");
		printf("%s SIMULATION '%s' STARTED\n", LOG_LVL1, simulation_code.c_str());

		komondor_simulation.Run();

		return 0;

	} catch (const ArgumentParseException& e) {
		fprintf(stderr, "%s ERROR: Argument parsing failed: %s\n", LOG_LVL1, e.what());
		fprintf(stderr, "%s\n", usage_message.c_str());
		return 1;
	} catch (const std::exception& e) {
		fprintf(stderr, "%s ERROR: Unexpected error: %s\n", LOG_LVL1, e.what());
		return 1;
	} catch (...) {
		fprintf(stderr, "%s ERROR: Unknown error occurred during argument parsing\n", LOG_LVL1);
		return 1;
	}
};
