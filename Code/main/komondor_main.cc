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

int total_nodes_number;			// Total number of nodes
char* tmp_nodes;

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

	// Setup variables corresponding to the console's input
	simulation_time_komondor = sim_time_console;
	save_node_logs = save_node_logs_console;
	save_agent_logs = save_agent_logs_console;
	print_node_logs = print_node_logs_console;
	print_system_logs = print_system_logs_console;
	print_agent_logs = print_agent_logs_console;
	nodes_input_filename = nodes_input_filename_console;
	agents_input_filename = agents_input_filename_console;
//	std::string simulation_code;
//    simulation_code.append(ToString(simulation_code_console));
	seed = seed_console;
	agents_enabled = agents_enabled_console;
	total_wlans_number = 0;

    // Generate output files
	if (print_system_logs) printf("\n%s Creating output files\n", LOG_LVL1);
	std::string simulation_filename_remove;
	simulation_filename_remove.append("output/logs_console_").append(simulation_code_console).append(".txt");
	std::string simulation_filename_fopen;
	simulation_filename_fopen.append("../").append(simulation_filename_remove);
	if(remove(simulation_filename_remove.c_str()) == 0){
		if (print_system_logs) printf("%s Simulation output file '%s' found and removed. New one created!\n",
			LOG_LVL2, simulation_filename_remove.c_str());
	} else {
		if (print_system_logs) printf("%s Simulation output file '%s' created!\n",
			LOG_LVL2, simulation_filename_remove.c_str());
	}

	// Script output (Readable)
	script_output_file = fopen(script_output_filename, "at");	// Script output is removed when script is executed
	logger_script.save_logs = SAVE_LOG;
	logger_script.file = script_output_file;
	fprintf(logger_script.file, "%s KOMONDOR SIMULATION '%s' (seed %d)", LOG_LVL1, simulation_code_console, seed);

	// Read system (environment) file
	SetupEnvironmentByReadingConfigFile();

	// Generate nodes
	GenerateNodesByReadingInputFile(nodes_input_filename);

	// Compute distance of each pair of nodes
	for(int i = 0; i < total_nodes_number; ++i) {
		node_container[i].distances_array = new double[total_nodes_number];
		node_container[i].received_power_array = new double[total_nodes_number];
		for(int j = 0; j < total_nodes_number; ++j) {
			// Compute and assign distances for each other node
			node_container[i].distances_array[j] = ComputeDistance(node_container[i].x,node_container[i].y,
				node_container[i].z,node_container[j].x,node_container[j].y,node_container[j].z);
			// Compute and assign the received power from each other node
			if(i == j) {
				node_container[i].received_power_array[j] = 0;
			} else {
				node_container[i].received_power_array[j] = ComputePowerReceived(node_container[i].distances_array[j],
					node_container[j].tx_power_default, node_container[i].central_frequency, path_loss_model);
			}
		}
	}

	// Compute the maximum power received from each WLAN
	for(int i = 0; i < total_nodes_number; ++i) {
		double max_power_received_per_wlan;
		if (node_container[i].node_type == NODE_TYPE_AP) {
			node_container[i].max_received_power_in_ap_per_wlan = new double[total_wlans_number];
			for(int j = 0; j < total_wlans_number; ++j) {
				if (strcmp(node_container[i].wlan_code.c_str(),wlan_container[j].wlan_code.c_str()) == 0) {
					// Same WLAN
					node_container[i].max_received_power_in_ap_per_wlan[j] = 0;
				} else {
					// Different WLAN
					max_power_received_per_wlan = -1000;
					for (int k = 0; k < total_nodes_number; ++k) {
						// Check only nodes in WLAN "j"
						if(strcmp(node_container[k].wlan_code.c_str(),wlan_container[j].wlan_code.c_str()) == 0) {
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

	// Initialize arrays for the token-based channel access
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

	// Generate agents (if enabled)
	central_controller_flag = 0;
	if (agents_enabled) { GenerateAgents(agents_input_filename, simulation_code_console); }
	// Generate the central controller (if enabled)
	if (agents_enabled && central_controller_flag) { GenerateCentralController(agents_input_filename); }

	// Print detected configuration (system, nodes and agents)
	if (print_system_logs) {
		printf("%s System configuration: \n", LOG_LVL2);
		PrintSystemInfo();
		printf("%s Wlans generated!\n", LOG_LVL2);
		PrintAllWlansInfo();
		if (print_system_logs) printf("\n");
		printf("%s Nodes generated!\n", LOG_LVL2);
		PrintAllNodesInfo(INFO_DETAIL_LEVEL_2);
		if (print_system_logs) printf("\n");
		if (print_system_logs && agents_enabled) PrintMlOperationInfo();
	}

	// Run the input checker in order to avoid unexpected situations
	InputChecker();

	// Set connections among nodes
	for(int n = 0; n < total_nodes_number; ++n){

		connect traffic_generator_container[n].outportNewPacketGenerated,node_container[n].InportNewPacketGenerated;

		for(int m=0; m < total_nodes_number; ++m) {

			connect node_container[n].outportSelfStartTX,node_container[m].InportSomeNodeStartTX;
			connect node_container[n].outportSelfFinishTX,node_container[m].InportSomeNodeFinishTX;
			connect node_container[n].outportSendLogicalNack,node_container[m].InportNackReceived;

			// Nodes belonging to the same WLAN
			if(strcmp(node_container[n].wlan_code.c_str(),node_container[m].wlan_code.c_str()) == 0 && n!=m) {
				// Connections regarding MCS
				connect node_container[n].outportAskForTxModulation,node_container[m].InportMCSRequestReceived;
				connect node_container[n].outportAnswerTxModulation,node_container[m].InportMCSResponseReceived;
				// Connections regarding changes in the WLAN
				connect node_container[n].outportSetNewWlanConfiguration,node_container[m].InportNewWlanConfigurationReceived;
				// Connections Spatial Reuse
				if(node_container[n].node_type == NODE_TYPE_AP && node_container[m].node_type == NODE_TYPE_STA) {
					connect node_container[m].outportRequestSpatialReuseConfiguration,node_container[n].InportRequestSpatialReuseConfiguration;
					connect node_container[n].outportNewSpatialReuseConfiguration,node_container[m].InportNewSpatialReuseConfiguration;
				}
			}
		}

		if (agents_enabled) {
			// Set connections among APs and Agents
			if ( node_container[n].node_type == NODE_TYPE_AP ) {
				for(int w = 0; w < total_agents_number; ++w){
					// Connect the agent to the corresponding AP, according to "wlan_code"
					if (strcmp(node_container[n].wlan_code.c_str(), agent_container[w].wlan_code.c_str()) == 0) {
//						printf("Connecting agent %d with node %d\n", agent_container[w].agent_id, node_container[n].node_id);
						connect agent_container[w].outportRequestInformationToAp,node_container[n].InportReceivingRequestFromAgent;
						connect node_container[n].outportAnswerToAgent,agent_container[w].InportReceivingInformationFromAp;
						connect agent_container[w].outportSendConfigurationToAp,node_container[n].InportReceiveConfigurationFromAgent;
					}
				}
			}
		}
	}

	// Connect the agents to the central controller, if applicable
	if (agents_enabled && central_controller_flag) {
		for(int w = 0; w < total_agents_number; ++w){
			if(agent_container[w].agent_centralized && central_controller[0].controller_on) {
//				connect central_controller[0].outportRequestInformationToAgent,agent_container[w].InportReceivingRequestFromController;
				connect agent_container[w].outportAnswerToController,central_controller[0].InportReceivingInformationFromAgent;
				connect central_controller[0].outportSendCommandToAgent,agent_container[w].InportReceiveCommandFromController;
			}
		}
	}
};

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
	Performance *performance_per_node = new Performance[total_nodes_number];
	Configuration *configuration_per_node = new Configuration[total_nodes_number];
	for (int i = 0; i < total_nodes_number; ++i) {
		performance_per_node[i] = node_container[i].simulation_performance;
		configuration_per_node[i] = node_container[i].configuration;
	}

	// Generate the output for scripts
	GenerateScriptOutput(simulation_index, performance_per_node, configuration_per_node, logger_script,
		total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor);

	// End of logs
	fclose(script_output_file);

	printf("%s SIMULATION '%s' FINISHED\n", LOG_LVL1, simulation_code.c_str());
	printf("------------------------------------------\n");

};

/**
 * Identify errors in the introduced input to prevent unexpected situations during the simulation
 */
void Komondor :: InputChecker(){

	// Auxiliary arrays
	int nodes_ids[total_nodes_number];
	double nodes_x[total_nodes_number];
	double nodes_y[total_nodes_number];
	double nodes_z[total_nodes_number];
	for(int i = 0; i<total_nodes_number;++i){
		nodes_ids[i] = 0;
		nodes_x[i] = 0;
		nodes_y[i] = 0;
		nodes_z[i] = 0;
	}

	if (print_system_logs) printf("%s Validating input files...\n", LOG_LVL2);

	for (int i = 0; i < total_nodes_number; ++i) {

		nodes_ids[i] = node_container[i].node_id;
		nodes_x[i] = node_container[i].x;
		nodes_y[i] = node_container[i].y;
		nodes_z[i] = node_container[i].z;

		// Check the range of transmission power values (min <= defalut <= max)
		if (node_container[i].tx_power_default > ConvertPower(DBM_TO_PW, MAX_TX_POWER_DBM)
				|| node_container[i].tx_power_default < ConvertPower(DBM_TO_PW, MIN_TX_POWER_DBM)) {
			printf("\nERROR: tx_power values are not properly configured at node in line %d\n"
				"MIN_TX_POWER_DBM = %d\nnode_container[i].tx_power_default = %f\nMAX_TX_POWER_DBM = %d\n\n",
				i+2, MIN_TX_POWER_DBM, ConvertPower(PW_TO_DBM, node_container[i].tx_power_default), MAX_TX_POWER_DBM);
			exit(-1);
		}

		// Check the range of sensitivity values (min <= defalut <= max)
		if (node_container[i].sensitivity_default > ConvertPower(DBM_TO_PW,MAX_SENSITIVITY_DBM)
				|| node_container[i].sensitivity_default > ConvertPower(DBM_TO_PW,MAX_SENSITIVITY_DBM)) {
			printf("\nERROR: sensitivity values are not properly configured at node in line %d\n\n",i+2);
			exit(-1);
		}

		// Check the range of channel values (min <= primary <= max)
		if (node_container[i].current_primary_channel > node_container[i].max_channel_allowed
				|| node_container[i].current_primary_channel < node_container[i].min_channel_allowed
				|| node_container[i].min_channel_allowed > node_container[i].max_channel_allowed
				|| node_container[i].current_primary_channel > NUM_CHANNELS_KOMONDOR
				|| node_container[i].min_channel_allowed > (NUM_CHANNELS_KOMONDOR-1)
				|| node_container[i].max_channel_allowed > (NUM_CHANNELS_KOMONDOR-1)) {
			printf("\nERROR: Channels are not properly configured at node in line %d\n\n",i+2);
			exit(-1);
		}
	}

	for (int i = 0; i < total_nodes_number; ++i) {
		for (int j = 0; j < total_nodes_number; ++j) {
			// Node IDs must be different
			if(i!=j && nodes_ids[i] == nodes_ids[j] && i < j) {
				printf("\nERROR: Nodes in lines %d and %d have the same ID\n\n",i+2,j+2);
				exit(-1);
			}
			// The position of nodes must be different
			if(i!=j && nodes_x[i] == nodes_x[j] && nodes_y[i] == nodes_y[j] && nodes_z[i] == nodes_z[j] && i < j) {
				printf("%s nERROR: Nodes in lines %d and %d are exactly at the same position\n\n", LOG_LVL2, i+2,j+2);
				exit(-1);
			}
		}
	}

	if (print_system_logs) printf("%s Input files validated!\n", LOG_LVL3);

}

/**
 * Set up the Komondor environment by reading the system input file (MS-DOS type)
 * @param "system_input_filename" [type char*]: filename of the system input CSV
 */
void Komondor :: SetupEnvironmentByReadingConfigFile() {

	const char *filename_test = "../config_models";
	char delim[] = "=";
	char *ptr;
	int ix_param = 0;
	if (print_system_logs) printf("\n%s Reading system configuration file '%s'...\n", LOG_LVL1, filename_test);
	FILE* test_input_config = fopen(filename_test, "r");
	if (!test_input_config){
		printf("%s Test file '%s' not found!\n", LOG_LVL3, filename_test);
		exit(-1);
	}
	char line_system[CHAR_BUFFER_SIZE];
	while (fgets(line_system, CHAR_BUFFER_SIZE, test_input_config)){
		// Ignore lines with comments
		if(line_system[0] == '#') {
			continue;
		}
		// Separate the value of the parameter from the entire line
		ptr = strtok(line_system, delim);
		ptr = strtok(NULL, delim);
		// Store the parameter as a global variable
		if (ix_param == 0){
			// Path-loss model
			path_loss_model = atoi(ptr);
		} else if (ix_param == 1) {
			// Adjacent channel interference model
			adjacent_channel_model = atoi(ptr);
		} else if (ix_param == 2) {
			// Collisions model
			collisions_model = atoi(ptr);
		} else if (ix_param == 3) {
			// PDF backoff model
			pdf_backoff = atoi(ptr);
		} else if (ix_param == 4) {
			// PDF tx time model
			pdf_tx_time = atoi(ptr);
		} else if (ix_param == 5) {
			// Simulation index (script's output)
			simulation_index = atoi(ptr);
		}
		ix_param++;
	}
	fclose(test_input_config);

	if (print_system_logs) printf("%s System environment properly set!\n", LOG_LVL2);

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
				// Node type
				tmp_nodes = strdup(line_nodes);
				int node_type (atoi(GetField(tmp_nodes, IX_NODE_TYPE)));
				if(node_type == NODE_TYPE_AP){	// If node is AP
					// WLAN ID
					wlan_container[wlan_ix].wlan_id = wlan_ix;
					// WLAN code
					tmp_nodes = strdup(line_nodes);
					std::string wlan_code_aux = ToString(GetField(tmp_nodes, IX_WLAN_CODE));
					wlan_container[wlan_ix].wlan_code = wlan_code_aux;
					++wlan_ix;
					free(tmp_nodes);
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
				// Node ID (auto-assigned)
				node_container[node_ix].node_id = node_ix;
				// Node code
				tmp_nodes = strdup(line_nodes);
				std::string node_code = ToString(GetField(tmp_nodes, IX_NODE_CODE));
				node_container[node_ix].node_code = node_code;
				// Node type
				tmp_nodes = strdup(line_nodes);
				int node_type (atoi(GetField(tmp_nodes, IX_NODE_TYPE)));
				node_container[node_ix].node_type = node_type;
				// WLAN code: add AP or STA ID to corresponding WLAN
				tmp_nodes = strdup(line_nodes);
				const char *wlan_code_aux (GetField(tmp_nodes, IX_WLAN_CODE));
				std::string wlan_code;
				wlan_code.append(ToString(wlan_code_aux));
				node_container[node_ix].wlan_code = wlan_code;
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
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].y = atof(GetField(tmp_nodes, IX_POSITION_Y));
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].z = atof(GetField(tmp_nodes, IX_POSITION_Z));
				// Central frequency in GHz (e.g. 2.4)
				tmp_nodes = strdup(line_nodes);
				const char* central_frequency_char (GetField(tmp_nodes, IX_CENTRAL_FREQ));
				node_container[node_ix].central_frequency = atof(central_frequency_char) * pow(10,9);
				// Channel bonding model
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].current_dcb_policy = atoi(GetField(tmp_nodes, IX_CHANNEL_BONDING_MODEL));
				// Primary channel
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].current_primary_channel = atoi(GetField(tmp_nodes, IX_PRIMARY_CHANNEL));
				// Min channel allowed
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].min_channel_allowed = atoi(GetField(tmp_nodes, IX_MIN_CH_ALLOWED));
				// Max channel allowed
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].max_channel_allowed = atoi(GetField(tmp_nodes, IX_MAX_CH_ALLOWED));
				// Default tx_power
				tmp_nodes = strdup(line_nodes);
				double tx_power_default_dbm (atof(GetField(tmp_nodes, IX_TX_POWER_DEFAULT)));
				node_container[node_ix].tx_power_default = ConvertPower(DBM_TO_PW, tx_power_default_dbm);
				// Default pd threshold
				tmp_nodes = strdup(line_nodes);
				double sensitivity_default_dbm (atoi(GetField(tmp_nodes, IX_PD_DEFAULT)));
				node_container[node_ix].sensitivity_default = ConvertPower(DBM_TO_PW, sensitivity_default_dbm);
				// Traffic model
				tmp_nodes = strdup(line_nodes);
				const char* traffic_model_char (GetField(tmp_nodes, IX_TRAFFIC_MODEL));
				// Traffic load (packet generation rate)
				tmp_nodes = strdup(line_nodes);
				const char* traffic_load_char (GetField(tmp_nodes, IX_TRAFFIC_LOAD));
				// Packet length
				tmp_nodes = strdup(line_nodes);
				const char* packet_length_char (GetField(tmp_nodes, IX_PACKET_LENGTH));
				node_container[node_ix].frame_length = atoi(packet_length_char);
				// Maximum number of aggregated packets
				tmp_nodes = strdup(line_nodes);
				const char* packets_aggregated_char (GetField(tmp_nodes, IX_NUM_PACKETS_AGG));
				node_container[node_ix].max_num_packets_aggregated = atoi(packets_aggregated_char);
				// Capture effect model -  0=default (recommended) or 1=IEEE 802.11-like
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].capture_effect_model = atoi(GetField(tmp_nodes, IX_CAPTURE_EFFECT_MODEL));
				// Capture effect threshold in dB
				tmp_nodes = strdup(line_nodes);
				const char* capture_effect_char (GetField(tmp_nodes, IX_CAPTURE_EFFECT_THR));
				node_container[node_ix].capture_effect = ConvertPower(DB_TO_LINEAR, atof(capture_effect_char));
				// Constant PER
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].constant_per = atof(GetField(tmp_nodes, IX_CONSTANT_PER));
				// PIFS activated
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].pifs_activated = atoi(GetField(tmp_nodes, IX_PIFS_ACTIVATED));
				// BACKOFF TYPE
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].backoff_type = atoi(GetField(tmp_nodes, IX_BACKOFF_TYPE));
				// CW adaptation activated
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].cw_adaptation = atoi(GetField(tmp_nodes, IX_CW_ADAPTATION_FLAG));
				// CW min default
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].cw_min_default = atoi(GetField(tmp_nodes, IX_CW_MIN_DEFAULT));
				// CW max default
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].cw_max_default = atoi(GetField(tmp_nodes, IX_CW_MAX_DEFAULT));
				// CW max stage
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].cw_stage_max = atoi(GetField(tmp_nodes, IX_CW_STAGE_MAX));
				// SPATIAL REUSE parameters
				//  - BSS color
				tmp_nodes = strdup(line_nodes);
				const char* bss_color_char = GetField(tmp_nodes, IX_BSS_COLOR);
				//  - Spatial Reuse Group (SRG)
				tmp_nodes = strdup(line_nodes);
				const char* srg_char = GetField(tmp_nodes, IX_SRG);
				//  - non-SRG OBSS_PD
				tmp_nodes = strdup(line_nodes);
				const char* non_srg_obss_pd_char = GetField(tmp_nodes, IX_NON_SRG_OBSS_PD);
				//  - SRG OBSS_PD
				tmp_nodes = strdup(line_nodes);
				const char* srg_obss_pd_char = GetField(tmp_nodes, IX_SRG_OBSS_PD);
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
				// SPATIAL REUSE
				if (bss_color_char != NULL) { // Check if the input file is compliant with SR
					node_container[node_ix].bss_color = atoi(bss_color_char);
					node_container[node_ix].srg = atoi(srg_char);
					double non_srg_obss_pd_dbm = atof(non_srg_obss_pd_char);
					node_container[node_ix].non_srg_obss_pd = ConvertPower(DBM_TO_PW, non_srg_obss_pd_dbm);
					double srg_obss_pd_dbm = atof(srg_obss_pd_char);
					node_container[node_ix].srg_obss_pd = ConvertPower(DBM_TO_PW, srg_obss_pd_dbm);
				} else {
					node_container[node_ix].bss_color = -1;
					node_container[node_ix].srg = -1;
					node_container[node_ix].non_srg_obss_pd = -1;
					node_container[node_ix].srg_obss_pd = -1;
				}
				// Traffic generator
				traffic_generator_container[node_ix].node_type = node_type;
				traffic_generator_container[node_ix].node_id = node_ix;
				traffic_generator_container[node_ix].traffic_model = atoi(traffic_model_char);
				node_container[node_ix].traffic_model = atoi(traffic_model_char); // Tell the node in case full buffer model is selected
				traffic_generator_container[node_ix].traffic_load = atof(traffic_load_char);

				++node_ix;
				free(tmp_nodes);
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
			const char *channel_aux;
			channel_aux = strtok ((char*)channel_values_text.c_str(),",");
			num_arms_channel = 0;
			while (channel_aux != NULL) {
				channel_aux = strtok (NULL, ",");
				++ num_arms_channel;
			}
			// Set the length of channel actions to agent's field
			agent_container[agent_ix].num_arms_channel = num_arms_channel;
			// Find the length of the pd actions array
			tmp_agents = strdup(line_agents);
			const char *pd_values_aux (GetField(tmp_agents, IX_AGENT_PD_VALUES));
			std::string pd_values_text;
			pd_values_text.append(ToString(pd_values_aux));
			const char *pd_aux;
			pd_aux = strtok ((char*)pd_values_text.c_str(),",");
			num_arms_sensitivity = 0;
			while (pd_aux != NULL) {
				pd_aux = strtok (NULL, ",");
				++ num_arms_sensitivity;
			}
			// Set the length of sensitivity actions to agent's field
			agent_container[agent_ix].num_arms_sensitivity = num_arms_sensitivity;
			// Find the length of the Tx power actions array
			tmp_agents = strdup(line_agents);
			const char *tx_power_values_aux (GetField(tmp_agents, IX_AGENT_TX_POWER_VALUES));
			std::string tx_power_values_text;
			tx_power_values_text.append(ToString(tx_power_values_aux));
			const char *tx_power_aux;
			tx_power_aux = strtok ((char*)tx_power_values_text.c_str(),",");
			num_arms_tx_power = 0;
			while (tx_power_aux != NULL) {
				tx_power_aux = strtok (NULL, ",");
				++ num_arms_tx_power;
			}
			// Set the length of Tx power actions to agent's field
			agent_container[agent_ix].num_arms_tx_power = num_arms_tx_power;
			// Find the length of the DCB actions actions array
			tmp_agents = strdup(line_agents);
			const char *max_bandwidth_values_aux (GetField(tmp_agents, IX_AGENT_MAX_BANDWIDTH));
			std::string max_bandwidth_values_text;
			max_bandwidth_values_text.append(ToString(max_bandwidth_values_aux));
			const char *max_bandwidth_aux;
			max_bandwidth_aux = strtok ((char*)max_bandwidth_values_text.c_str(),",");
			num_arms_max_bandwidth = 0;
			while (max_bandwidth_aux != NULL) {
				max_bandwidth_aux = strtok (NULL, ",");
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
				//  Agent associated to the Central Controller (CC)
				tmp_agents = strdup(line_agents);
				int agent_centralized (atoi(GetField(tmp_agents, IX_COMMUNICATION_LEVEL)));
				agent_container[agent_ix].agent_centralized = agent_centralized;
				// Check if the central controller has to be created or not
				if(agent_centralized) ++total_controlled_agents_number;
				// Time between requests (in seconds)
				tmp_agents = strdup(line_agents);
				double time_between_requests (atof(GetField(tmp_agents, IX_AGENT_TIME_BW_REQUESTS)));
				agent_container[agent_ix].time_between_requests = time_between_requests;
				// Channel values
				tmp_agents = strdup(line_agents);
				std::string channel_values_text = ToString(GetField(tmp_agents, IX_AGENT_CHANNEL_VALUES));
				// Fill the channel actions array
				char *channel_aux_2;
				char *channel_values_text_char = new char[channel_values_text.length() + 1];
				strcpy(channel_values_text_char, channel_values_text.c_str());
				channel_aux_2 = strtok (channel_values_text_char,",");
				int ix (0);
				while (channel_aux_2 != NULL) {
					int a (atoi(channel_aux_2));
					agent_container[agent_ix].list_of_channels[ix] = a;
					channel_aux_2 = strtok (NULL, ",");
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
				char *pd_aux_2;
				char *pd_values_text_char = new char[pd_values_text.length() + 1];
				strcpy(pd_values_text_char, pd_values_text.c_str());
				pd_aux_2 = strtok (pd_values_text_char,",");
				ix = 0;
				while (pd_aux_2 != NULL) {
					int a = atoi(pd_aux_2);
					agent_container[agent_ix].list_of_pd_values[ix] = ConvertPower(DBM_TO_PW, a);
					pd_aux_2 = strtok (NULL, ",");
					++ix;
				}
				// Tx Power values
				tmp_agents = strdup(line_agents);
				std::string tx_power_values_text = ToString(GetField(tmp_agents, IX_AGENT_TX_POWER_VALUES));
				// Fill the TX power actions array
				char *tx_power_aux_2;
				char *tx_power_values_text_char = new char[tx_power_values_text.length() + 1];
				strcpy(tx_power_values_text_char, tx_power_values_text.c_str());
				tx_power_aux_2 = strtok (tx_power_values_text_char,",");
				ix = 0;
				while (tx_power_aux_2 != NULL) {
					int a (atoi(tx_power_aux_2));
					agent_container[agent_ix].list_of_tx_power_values[ix] = ConvertPower(DBM_TO_PW, a);
					tx_power_aux_2 = strtok (NULL, ",");
					++ix;
				}
				// Max bandwidth values
				tmp_agents = strdup(line_agents);
				std::string max_bandwidth_values_text = ToString(GetField(tmp_agents, IX_AGENT_MAX_BANDWIDTH));
				// Fill the max bandwidth actions array
				char *max_bandwidth_aux_2;
				char *max_bandwidth_values_text_char = new char[max_bandwidth_values_text.length() + 1];
				strcpy(max_bandwidth_values_text_char, max_bandwidth_values_text.c_str());
				max_bandwidth_aux_2 = strtok (max_bandwidth_values_text_char,",");
				ix = 0;
				while (max_bandwidth_aux_2 != NULL) {
					int a (atoi(max_bandwidth_aux_2));
					agent_container[agent_ix].list_of_max_bandwidth[ix] = a;
					max_bandwidth_aux_2 = strtok (NULL, ",");
					++ix;
				}
				// Type of reward
				tmp_agents = strdup(line_agents);
				int type_of_reward (atoi(GetField(tmp_agents, IX_AGENT_TYPE_OF_REWARD)));
				agent_container[agent_ix].type_of_reward = type_of_reward;
				// Learning mechanism
				tmp_agents = strdup(line_agents);
				int learning_mechanism (atoi(GetField(tmp_agents, IX_AGENT_LEARNING_MECHANISM)));
				agent_container[agent_ix].learning_mechanism = learning_mechanism;
				// Selected strategy
				tmp_agents = strdup(line_agents);
				int action_selection_strategy (atoi(GetField(tmp_agents, IX_AGENT_SELECTED_STRATEGY)));
				agent_container[agent_ix].action_selection_strategy = action_selection_strategy;
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
					const char *channels_aux;
					channels_aux = strtok ((char*)channel_values_text.c_str(),",");
					int num_arms_channels = 0;
					while (channels_aux != NULL) {
						channels_aux = strtok (NULL, ",");
						++ num_arms_channels;
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
 * @param "line" [type char*]: line of the CSV
 * @param "num" [type int]: field number (index)
 * @return "field" [type char*]: field corresponding to the introduced index
 */
const char* GetField(char* line, int num){
    const char* tok;
    for (tok = strtok(line, ";");
            tok && *tok;
            tok = strtok(NULL, ";\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
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

				// Node type
				tmp_nodes = strdup(line_nodes);
				type_found = atof(GetField(tmp_nodes, IX_NODE_TYPE));
				free(tmp_nodes);

				// WLAN code
				tmp_nodes = strdup(line_nodes);
				wlan_code_found = ToString(GetField(tmp_nodes, IX_WLAN_CODE));
				free(tmp_nodes);

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
	char *nodes_input_filename;
	char *agents_input_filename;
	std::string script_output_filename;
	std::string simulation_code;
	int save_node_logs;
	int save_agent_logs;
	int print_system_logs;
	int print_node_logs;
	int print_agent_logs;
	double sim_time;
	int seed;
	int agents_enabled;

	total_nodes_number = 0;

	// Get input variables per console
	if(argc == NUM_FULL_ARGUMENTS_CONSOLE){	// Full configuration entered per console
		nodes_input_filename = argv[1];
		agents_input_filename = argv[2];
		script_output_filename = ToString(argv[3]);
		simulation_code = ToString(argv[4]);
		save_node_logs = atoi(argv[5]);
		save_agent_logs = atoi(argv[6]);
		print_system_logs = atoi(argv[7]);
		print_node_logs = atoi(argv[8]);
		print_agent_logs = atoi(argv[9]);
		sim_time = atof(argv[10]);
		seed = atoi(argv[11]);
		// Enable the operation of agents
		agents_enabled = TRUE;
		if (print_system_logs) printf("%s FULL configuration entered per console (AGENTS ENABLED).\n", LOG_LVL1);
	} else if(argc == NUM_FULL_ARGUMENTS_CONSOLE_NO_AGENTS){	// Configuration without agents
		nodes_input_filename = argv[1];
		script_output_filename = ToString(argv[2]);
		simulation_code = ToString(argv[3]);
		save_node_logs = atoi(argv[4]);
		print_system_logs = atoi(argv[5]);
		print_node_logs = atoi(argv[6]);
		sim_time = atof(argv[7]);
		seed = atoi(argv[8]);
		// Disable the operation of agents
		agents_enabled = FALSE;
		if (print_system_logs) printf("%s FULL configuration entered per console (AGENTS DISABLED).\n", LOG_LVL1);
	} else if(argc == NUM_PARTIAL_ARGUMENTS_CONSOLE) {	// Partial configuration entered per console
		nodes_input_filename = argv[1];
		sim_time = atof(argv[2]);
		seed = atoi(argv[3]);
		// Default values
		script_output_filename.append(ToString(DEFAULT_SCRIPT_FILENAME));
		simulation_code.append(ToString(DEFAULT_SIMULATION_CODE));
		save_node_logs = DEFAULT_WRITE_NODE_LOGS;
		print_system_logs = DEFAULT_PRINT_SYSTEM_LOGS;
		print_node_logs = DEFAULT_PRINT_NODE_LOGS;
		// Disable the operation of agents
		agents_enabled = FALSE;
		if (print_system_logs) printf("%s PARTIAL configuration entered per console. "
			"Some parameters are set by DEFAULT.\n", LOG_LVL1);
	} else if(argc == NUM_PARTIAL_ARGUMENTS_SCRIPT) {	// Partial configuration entered per console (useful for scripts)
		nodes_input_filename = argv[1];
		simulation_code = ToString(argv[2]);	// For scripts --> useful to identify simulations
		sim_time = atof(argv[3]);
		seed = atoi(argv[4]);
		// Default values
		script_output_filename.append(ToString(DEFAULT_SCRIPT_FILENAME));
		save_node_logs = DEFAULT_WRITE_NODE_LOGS;
		print_system_logs = DEFAULT_PRINT_SYSTEM_LOGS;
		print_node_logs = DEFAULT_PRINT_NODE_LOGS;
		// Disable the operation of agents
		agents_enabled = FALSE;
		if (print_system_logs) printf("%s PARTIAL configuration entered per script. "
			"Some parameters are set by DEFAULT.\n", LOG_LVL1);
	} else {
		printf("%sERROR: Console arguments were not set properly!\n "
			" + For FULL configuration setting without agents, execute\n"
			"    ./Komondor -system_input_filename -nodes_input_filename -script_output_filename "
			"-simulation_code -save_node_logs -print_node_logs -print_system_logs "
			"-sim_time -seed\n"
			" + For FULL configuration setting with agents, execute\n"
			"    ./Komondor -system_input_filename -nodes_input_filename -agents_input_filename -script_output_filename "
			"-simulation_code -save_node_logs -save_agent_logs -print_node_logs -print_system_logs "
			"-print_agent_logs -sim_time -seed\n"
			" + For PARTIAL configuration setting, execute\n"
			"    ./KomondorSimulation -system_input_filename -nodes_input_filename -sim_time -seed\n"
			" + For PARTIAL configuration setting (SCRIPTS), execute\n"
			"    ./KomondorSimulation -system_input_filename -nodes_input_filename -simulation_code -sim_time -seed\n", LOG_LVL1);
		return(-1);
	}

	if (print_system_logs) {
		printf("%s Komondor input configuration:\n", LOG_LVL1);
		printf("%s nodes_input_filename: %s\n", LOG_LVL2, nodes_input_filename);
		printf("%s agents_enabled: %d\n", LOG_LVL2, agents_enabled);
		if (agents_enabled) { printf("%s agents_input_filename: %s\n", LOG_LVL2, agents_input_filename); }
		printf("%s script_output_filename: %s\n", LOG_LVL2, script_output_filename.c_str());
		printf("%s simulation_code: %s\n", LOG_LVL2, simulation_code.c_str());
		printf("%s save_node_logs: %d\n", LOG_LVL2, save_node_logs);
		printf("%s print_system_logs: %d\n", LOG_LVL2, print_system_logs);
		printf("%s print_node_logs: %d\n", LOG_LVL2, print_node_logs);
		printf("%s sim_time: %f s\n", LOG_LVL2, sim_time);
		printf("%s seed: %d\n", LOG_LVL2, seed);
	}


	// Create output directory if not exists

	struct stat st = {0};

	if (stat("../output/", &st) == -1) {
		printf("- Output folder does not exist --> creating it...\n");
	    mkdir("../output/", 0777);
	    if (stat("../output/", &st) == -1) {
			printf("- Output folder could not be created! Check permissions\n");
			exit(-1);
		} else {
			printf("- Output folder successfully created!\n");
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

	return(0);
};
