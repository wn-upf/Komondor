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
 * File description: this is the main Komondor file
 *
 * - This file generates the wireless network according to the input files.
 * Then, it initiates nodes to start sending packets until the simulation
 * time is over. Finally, it processes the results.
 */

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <map>
#include <string>     // std::string, std::to_string

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

		void Setup(double simulation_time_komondor, int save_system_logs, int save_node_logs, int save_agent_logs,
			int print_node_logs, int print_system_logs, int print_agent_logs, const char *system_filename,
			const char *nodes_filename, const char *script_filename, const char *simulation_code, int seed_console,
			int agents_enabled, const char *agents_filename);
		void Stop();
		void Start();
		void InputChecker();

		void SetupEnvironmentByReadingInputFile(const char *system_filename);
		void GenerateNodesByReadingInputFile(const char *nodes_filename);

		void GenerateAgents(const char *agents_filename);
		void GenerateCentralController(const char *agents_filename);

		int GetNumOfLines(const char *nodes_filename);
		int GetNumOfNodes(const char *nodes_filename, int node_type, std::string wlan_code);

		void PrintSystemInfo();
		void PrintAllWlansInfo();
		void PrintAllAgentsInfo();
		void PrintCentralControllerInfo();
		void PrintAllNodesInfo(int info_detail_level);
		void WriteSystemInfo(Logger logger);
		void WriteAllWlansInfo(Logger logger, std::string header_str);
		void WriteAllNodesInfo(Logger logger, int info_detail_level,  std::string header_str);

	// Public items (to shared with the nodes)
	public:

		Node[] node_container;			// Container of nodes (i.e., APs, STAs, ...)
		Wlan *wlan_container;			// Container of WLANs
		TrafficGenerator[] traffic_generator_container; // Container of traffic generators (associated to nodes)

		int total_wlans_number;				// Total number of WLANs
		int total_agents_number;			// Total number of agents
		int total_controlled_agents_number = 0;	// Total number of agents attached to the central controller

		// Parameters entered per console
		int save_node_logs;					// Flag for activating the log writting of nodes
		int print_node_logs;				// Flag for activating the printing of node logs
		int save_agent_logs;				// Flag for activating the log writting of agents
		int print_agent_logs;				// Flag for activating the printing of agent logs
		double simulation_time_komondor;	// Simulation time [s]

		// Parameters entered via system file
		int num_channels_komondor;		// Number of subchannels composing the whole channel
		double basic_channel_bandwidth;	// Basic channel bandwidth [Mbps]
		int pdf_backoff;				// Probability distribution type of the backoff (0: exponential, 1: deterministic)
		int pdf_tx_time;				// Probability distribution type of the transmission time (0: exponential, 1: deterministic)
		int frame_length;				// Packet length [bits]
		int ack_length;					// ACK length [bits]
		int rts_length;					// RTS length [bits]
		int cts_length;					// CTS length [bits]
		int max_num_packets_aggregated;	// Number of packets aggregated in one transmission
		int path_loss_model;			// Path loss model (0: free-space, 1: Okumura-Hata model - Uban areas)
		double capture_effect;			// Capture effect threshold [linear ratio]
		double noise_level;				// Environment noise [pW]
		int adjacent_channel_model;		// Co-channel interference model
		int collisions_model;			// Collisions model
		double constant_per;			// Constant PER for successful transmissions
		int traffic_model;				// Traffic model (0: full buffer, 1: poisson, 2: deterministic)
		int backoff_type;				// Type of Backoff (0: Slotted 1: Continuous)
		int cw_adaptation;				// CW adaptation (0: constant, 1: bineary exponential backoff)
		int pifs_activated;				// PIFS mechanism activation
		int capture_effect_model;		// Capture Effect model (default or IEEE 802.11-based)

		int agents_enabled;				// Determined according to the input (for generating agents or not)

		// Public items (to shared with the agents)
		public:

		// Agents info
		Agent[] agent_container;
		int num_actions_channel;
		int num_actions_sensitivity;
		int num_actions_tx_power;
		int num_actions_dcb_policy;

		double *actions_pd;
		double *actions_tx_power;

		// Central controller info
		CentralController[] central_controller;

	// Private items
	private:

		int seed;							// Simulation seed number
		int save_system_logs;				// Flag for activating the log writting of the Komondor system
		int print_system_logs;				// Flag for activating the printing of system logs
		std::string simulation_code;		// Komondor simulation code
		const char *nodes_input_filename;	// Filename of the nodes (AP or Deterministic Nodes) input CSV
		const char *agents_input_filename;	// Filename of the agents input CSV
		FILE *simulation_output_file;		// File for the output logs (including statistics)
		FILE *script_output_file;			// File for the whole input files included in the script TODO
		Logger logger_simulation;			// Logger for the simulation output file
		Logger logger_script;				// Logger for the script file (containing 1+ simulations) Readable version

		// Auxiliar variables
		int first_line_skiped_flag;		// Flag for skipping first informative line of input file
		int central_controller_flag; 	// In order to allow the generation of the central controller

};

/*
 * Setup()
 * Input arguments:
 * - sim_time_console: simulation observation time [s]
 * - save_system_logs_console: flag for activating system logs
 * - save_node_logs_console: flag for activating nodes logs
 * - print_system_logs_console: flag for activating system prints
 * - print_node_logs_console: flag for activating nodes prints
 * - system_input_filename: filename of the system input CSV
 * - nodes_input_filename: filename of the nodes (AP or Deterministic Nodes) input CSV
 * - script_output_filename: filename of the output file generated by the script of multiple simulations
 * - simulation_code_console: simulation code assigned to current simulation (it is an string)
 */
void Komondor :: Setup(double sim_time_console, int save_system_logs_console, int save_node_logs_console,
		int save_agent_logs_console, int print_system_logs_console, int print_node_logs_console,
		int print_agent_logs_console, const char *system_input_filename, const char *nodes_input_filename_console,
		const char *script_output_filename, const char *simulation_code_console, int seed_console,
		int agents_enabled_console, const char *agents_input_filename_console){

	// Setup variables corresponding to the console's input
	simulation_time_komondor = sim_time_console;
	save_node_logs = save_node_logs_console;
	save_system_logs = save_system_logs_console;
	save_agent_logs = save_agent_logs_console;
	print_node_logs = print_node_logs_console;
	print_system_logs = print_system_logs_console;
	print_agent_logs = print_agent_logs_console;
	nodes_input_filename = nodes_input_filename_console;
	agents_input_filename = agents_input_filename_console;
	std::string simulation_code;
	simulation_code.append(ToString(simulation_code_console));
	seed = seed_console;
	agents_enabled = agents_enabled_console;
	total_wlans_number = 0;

	// Generate output files
	if (print_system_logs) printf("%s Creating output files\n", LOG_LVL1);
	std::string simulation_filename_remove;
	simulation_filename_remove.append("output/logs_console_").append(simulation_code).append(".txt");
	std::string simulation_filename_fopen;
	simulation_filename_fopen.append("../").append(simulation_filename_remove);
	if(remove(simulation_filename_remove.c_str()) == 0){
		if (print_system_logs) printf("%s Simulation output file '%s' found and removed. New one created!\n",
			LOG_LVL2, simulation_filename_remove.c_str());
	} else {
		if (print_system_logs) printf("%s Simulation output file '%s' created!\n",
			LOG_LVL2, simulation_filename_remove.c_str());
	}

	// Output console logs
	simulation_output_file = fopen(simulation_filename_fopen.c_str(),"at");
	logger_simulation.save_logs = SAVE_LOG;
	logger_simulation.file = simulation_output_file;

	// Script output (Readable)
	script_output_file = fopen(script_output_filename, "at");	// Script output is removed when script is executed
	logger_script.save_logs = SAVE_LOG;
	logger_script.file = script_output_file;
	fprintf(logger_script.file, "%s KOMONDOR SIMULATION '%s' (seed %d)", LOG_LVL1, simulation_code.c_str(), seed);

	// Read system (environment) file
	SetupEnvironmentByReadingInputFile(system_input_filename);

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
					node_container[j].tx_power_default, node_container[j].tx_gain, node_container[i].rx_gain,
					node_container[i].central_frequency, path_loss_model);
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

	// Generate agents
	central_controller_flag = 0;

	if (agents_enabled) { GenerateAgents(agents_input_filename); }

	if (agents_enabled && central_controller_flag) { GenerateCentralController(agents_input_filename); }

	if (print_system_logs) {
		printf("%s System configuration: \n", LOG_LVL2);
		PrintSystemInfo();
		printf("%s Wlans generated!\n", LOG_LVL2);
		PrintAllWlansInfo();
		if (print_system_logs) printf("\n");
		printf("%s Nodes generated!\n", LOG_LVL2);
		PrintAllNodesInfo(INFO_DETAIL_LEVEL_2);
		if (print_system_logs) printf("\n");
		if (agents_enabled) {
			printf("%s Agents generated!\n\n", LOG_LVL2);
			PrintAllAgentsInfo();
			if (central_controller_flag) {
				printf("%s Central Controller generated!\n\n", LOG_LVL2);
				central_controller[0].PrintCentralControllerInfo();
			}
			printf("\n");
		}
	}

	InputChecker();

	fprintf(logger_simulation.file, "------------------------------------\n");

	if (save_system_logs){

		fprintf(logger_simulation.file, "%s System configuration: \n", LOG_LVL2);
		WriteSystemInfo(logger_simulation);

		std::string header_str;
		header_str.append(ToString(LOG_LVL3));

		fprintf(logger_simulation.file, "%s Wlans generated!\n", LOG_LVL2);
		WriteAllWlansInfo(logger_simulation, header_str);

		fprintf(logger_simulation.file, "%s Nodes generated!\n", LOG_LVL2);
		WriteAllNodesInfo(logger_simulation, INFO_DETAIL_LEVEL_0, header_str);

	}

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
						connect agent_container[w].outportRequestInformationToAp,node_container[n].InportReceivingRequestFromAgent;
						connect node_container[n].outportAnswerToAgent,agent_container[w].InportReceivingInformationFromAp;
						connect agent_container[w].outportSendConfigurationToAp,node_container[n].InportReceiveConfigurationFromAgent;
					}
				}
			}
		}
	}

	// Connect the agents to the central controller, if applicable
	if (agents_enabled) {
		for(int w = 0; w < total_agents_number; ++w){
			if (agent_container[w].centralized_flag) {
				connect central_controller[0].outportRequestInformationToAgent,agent_container[w].InportReceivingRequestFromController;
				connect agent_container[w].outportAnswerToController,central_controller[0].InportReceivingInformationFromAgent;
				connect central_controller[0].outportSendConfigurationToAgent,agent_container[w].InportReceiveConfigurationFromController;
			}
		}
	}

};

/*
 * Start()
 */
void Komondor :: Start(){
	// intialize variables
	// total_nodes_number = 0;
};

/*
 * Stop(): called when the simulation is done to  collect and display statistics.
 */
void Komondor :: Stop(){

	printf("%s STOP KOMONDOR SIMULATION '%s' (seed %d)", LOG_LVL1, simulation_code.c_str(), seed);

	// Display (in logs and files) statistics of the simulation
	Performance *performance_per_node = new Performance[total_nodes_number];
	Configuration *configuration_per_node = new Configuration[total_nodes_number];
	for (int i = 0; i < total_nodes_number; ++i) {
		performance_per_node[i] = node_container[i].simulation_performance;
		configuration_per_node[i] = node_container[i].configuration;
	}

	// Print and write global statistics
	PrintAndWriteSimulationStatistics(print_system_logs, save_system_logs, logger_simulation,
		performance_per_node, configuration_per_node, total_nodes_number, total_wlans_number,
		frame_length, max_num_packets_aggregated, simulation_time_komondor);

	// Generate the output for scripts
	int simulation_index (13);	// Choose the simulation index (TODO: change the way the simulation index is inputted)
	GenerateScriptOutput(simulation_index, performance_per_node, configuration_per_node, logger_script,
		total_wlans_number, total_nodes_number, frame_length, max_num_packets_aggregated,
		wlan_container, simulation_time_komondor);

	// End of logs
	fclose(simulation_output_file);
	fclose(script_output_file);

	printf("%s SIMULATION '%s' FINISHED\n", LOG_LVL1, simulation_code.c_str());
	printf("------------------------------------------\n");

};

/*
 * InputChecker(): identifies critical issues regarding the introduced input
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
		if (node_container[i].tx_power_min > node_container[i].tx_power_max
				|| node_container[i].tx_power_default > node_container[i].tx_power_max
				|| node_container[i].tx_power_default < node_container[i].tx_power_min) {
			printf("\nERROR: tx_power values are not properly configured at node in line %d\n"
					"node_container[i].tx_power_min = %f\n"
					"node_container[i].tx_power_default = %f\n"
					"node_container[i].tx_power_max = %f\n\n",
					i+2, node_container[i].tx_power_min, node_container[i].tx_power_default, node_container[i].tx_power_max);
			exit(-1);
		}

		// Check the range of sensitivity values (min <= defalut <= max)
		if (node_container[i].sensitivity_min > node_container[i].sensitivity_max
				|| node_container[i].sensitivity_default > node_container[i].sensitivity_max
				|| node_container[i].sensitivity_default < node_container[i].sensitivity_min) {
			printf("\nERROR: pd values are not properly configured at node in line %d\n\n",i+2);
			exit(-1);
		}

		// Check the range of channel values (min <= primary <= max)
		if (node_container[i].current_primary_channel > node_container[i].max_channel_allowed
				|| node_container[i].current_primary_channel < node_container[i].min_channel_allowed
				|| node_container[i].min_channel_allowed > node_container[i].max_channel_allowed
				|| node_container[i].current_primary_channel > num_channels_komondor
				|| node_container[i].min_channel_allowed > (num_channels_komondor-1)
				|| node_container[i].max_channel_allowed > (num_channels_komondor-1)) {
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

/*
 * SetupEnvironmentByReadingInputFile(): sets up the Komondor environment
 * Input arguments:
 * - system_filename: system input filename
 */
void Komondor :: SetupEnvironmentByReadingInputFile(const char *system_filename) {

	if (print_system_logs) printf("%s Reading system configuration file '%s'...\n", LOG_LVL1, system_filename);
	fprintf(simulation_output_file, "%s KOMONDOR SIMULATION '%s' (seed %d)", LOG_LVL1, simulation_code.c_str(), seed);

	FILE* stream_system = fopen(system_filename, "r");
	if (!stream_system){
		printf("%s Komondor system file '%s' not found!\n", LOG_LVL3, system_filename);
		fprintf(simulation_output_file, "%s Komondor system file '%s' not found!\n", LOG_LVL3, system_filename);
		exit(-1);
	}

	char line_system[CHAR_BUFFER_SIZE];
	int first_line_skiped_flag (0);	// Flag for skipping first informative line of input file

	while (fgets(line_system, CHAR_BUFFER_SIZE, stream_system)){

		if(!first_line_skiped_flag){

			first_line_skiped_flag = 1;

		} else {

			char* tmp = strdup(line_system);

			// Number of channels
			tmp = strdup(line_system);
			const char* num_channels_char (GetField(tmp, IX_NUM_CHANNELS));
			num_channels_komondor = atoi(num_channels_char);

			// Basic channel bandwidth
			tmp = strdup(line_system);
			const char* basic_channel_bandwidth_char (GetField(tmp, IX_BASIC_CH_BW));
			basic_channel_bandwidth = atoi(basic_channel_bandwidth_char);

			// Prob. distribution of backoff duration
			tmp = strdup(line_system);
			const char* pdf_backoff_char (GetField(tmp, IX_PDF_BACKOFF));
			pdf_backoff = atoi(pdf_backoff_char);

			// Prob. distribution of transmission duration
			tmp = strdup(line_system);
			const char* pdf_tx_time_char (GetField(tmp, IX_PDF_TX_TIME));
			pdf_tx_time = atoi(pdf_tx_time_char);

			// Data packet length
			tmp = strdup(line_system);
			const char* packet_length_char (GetField(tmp, IX_PACKET_LENGTH));
			frame_length = atoi(packet_length_char);

			// Number of packets aggregated in one transmission
			tmp = strdup(line_system);
			const char* num_packets_aggregated_char (GetField(tmp, IX_NUM_PACKETS_AGGREGATED));
			max_num_packets_aggregated = atoi(num_packets_aggregated_char);

			// Path loss model
			tmp = strdup(line_system);
			const char* path_loss_model_char (GetField(tmp, IX_PATH_LOSS));
			path_loss_model = atoi(path_loss_model_char);

			// capture_effect
			tmp = strdup(line_system);
			const char* capture_effect_char (GetField(tmp, IX_CAPTURE_EFFECT));
			double capture_effect_db (atof(capture_effect_char));
			capture_effect = ConvertPower(DB_TO_LINEAR, capture_effect_db);

			// Noise level
			tmp = strdup(line_system);
			const char* noise_level_char (GetField(tmp, IX_NOISE_LEVEL));
			double noise_level_dbm (atof(noise_level_char));
			noise_level = ConvertPower(DBM_TO_PW, noise_level_dbm);

			// Co-channel model
			tmp = strdup(line_system);
			const char* adjacent_channel_model_char (GetField(tmp, IX_COCHANNEL_MODEL));
			adjacent_channel_model = atof(adjacent_channel_model_char);

			// Collisions model
			tmp = strdup(line_system);
			const char* collisions_model_char (GetField(tmp, IX_COLLISIONS_MODEL));
			collisions_model = atof(collisions_model_char);

			// Constant PER for successful transmissions
			tmp = strdup(line_system);
			const char* constant_PER_char (GetField(tmp, IX_CONSTANT_PER));
			constant_per = atof(constant_PER_char);

			// Traffic model
			tmp = strdup(line_system);
			const char* traffic_model_char (GetField(tmp, IX_TRAFFIC_MODEL));
			traffic_model = atoi(traffic_model_char);

			// Backoff type
			tmp = strdup(line_system);
			const char* backoff_type_char (GetField(tmp, IX_BO_TYPE));
			backoff_type = atoi(backoff_type_char);

			// Contention window adaptation
			tmp = strdup(line_system);
			const char* cw_adaptation_char (GetField(tmp, IX_CW_ADAPTATION));
			cw_adaptation = atoi(cw_adaptation_char);

			// PIFS mechanism activation
			tmp = strdup(line_system);
			const char* pifs_activated_char (GetField(tmp, IX_PIFS_ACTIVATION));
			pifs_activated = atoi(pifs_activated_char);

			// PIFS mechanism activation
			tmp = strdup(line_system);
			const char* capture_effect_model_char (GetField(tmp, IX_CAPTURE_EFFECT_MODEL));
			capture_effect_model = atoi(capture_effect_model_char);

			free(tmp);
		}
	}

	fclose(stream_system);
}

/* *******************
 * * NODE GENERATION *
 * *******************
 */

/*
 * GenerateNodesByReadingInputFile(): generates the nodes deterministically, according to the input nodes file.
 * Input arguments:
 * - nodes_filename: input nodes filename
 */
void Komondor :: GenerateNodesByReadingInputFile(const char *nodes_filename) {

	if (print_system_logs) printf("%s Generating nodes DETERMINISTICALLY through NODES input file...\n", LOG_LVL1);
	if (save_system_logs) fprintf(simulation_output_file, "%s Generating nodes DETERMINISTICALLY...\n", LOG_LVL1);


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

			if(!first_line_skiped_flag){

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

		// Generate nodes (without wlan item), finsih WLAN with ID lists, and set the wlan item of each STA.
		if (print_system_logs) printf("%s Generating nodes...\n", LOG_LVL3);
		total_nodes_number = GetNumOfNodes(nodes_filename, NODE_TYPE_UNKWNOW, ToString(""));
		node_container.SetSize(total_nodes_number);
		traffic_generator_container.SetSize(total_nodes_number);

		stream_nodes = fopen(nodes_filename, "r");
		int node_ix (0);	// Auxiliar index for nodes
		wlan_ix = 0;		// Auxiliar index for WLANs
		first_line_skiped_flag = 0;

		while (fgets(line_nodes, CHAR_BUFFER_SIZE, stream_nodes)){	// For each WLAN

			if(!first_line_skiped_flag){

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

				// Destination ID
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].destination_id = atoi(GetField(tmp_nodes, IX_DESTINATION_ID));

				// Position
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].x = atof(GetField(tmp_nodes, IX_POSITION_X));
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].y = atof(GetField(tmp_nodes, IX_POSITION_Y));
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].z = atof(GetField(tmp_nodes, IX_POSITION_Z));

				// CW min
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].cw_min = atoi(GetField(tmp_nodes, IX_CW_MIN));

				// CW max
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].cw_stage_max = atoi(GetField(tmp_nodes, IX_CW_STAGE_MAX));

				// Primary channel
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].current_primary_channel = atoi(GetField(tmp_nodes, IX_PRIMARY_CHANNEL));

				// Min channel allowed
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].min_channel_allowed = atoi(GetField(tmp_nodes, IX_MIN_CH_ALLOWED));

				// Max channel allowed
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].max_channel_allowed = atoi(GetField(tmp_nodes, IX_MAX_CH_ALLOWED));

				// Min tx_power
				tmp_nodes = strdup(line_nodes);
				double tx_power_min_dbm (atof(GetField(tmp_nodes, IX_TX_POWER_MIN)));
				node_container[node_ix].tx_power_min = ConvertPower(DBM_TO_PW, tx_power_min_dbm);

				// Default tx_power
				tmp_nodes = strdup(line_nodes);
				double tx_power_default_dbm (atof(GetField(tmp_nodes, IX_TX_POWER_DEFAULT)));
				node_container[node_ix].tx_power_default = ConvertPower(DBM_TO_PW, tx_power_default_dbm);

				// Max tx_power
				tmp_nodes = strdup(line_nodes);
				double tx_power_max_dbm = atof(GetField(tmp_nodes, IX_TX_POWER_MAX));
				node_container[node_ix].tx_power_max = ConvertPower(DBM_TO_PW, tx_power_max_dbm);

				// Min pd
				tmp_nodes = strdup(line_nodes);
				double sensitivity_min_dbm (atoi(GetField(tmp_nodes, IX_PD_MIN)));
				node_container[node_ix].sensitivity_min = ConvertPower(DBM_TO_PW, sensitivity_min_dbm);

				// Default pd
				tmp_nodes = strdup(line_nodes);
				double sensitivity_default_dbm (atoi(GetField(tmp_nodes, IX_PD_DEFAULT)));
				node_container[node_ix].sensitivity_default = ConvertPower(DBM_TO_PW, sensitivity_default_dbm);

				// Max pd
				tmp_nodes = strdup(line_nodes);
				double sensitivity_max_dbm (atoi(GetField(tmp_nodes, IX_PD_MAX)));
				node_container[node_ix].sensitivity_max = ConvertPower(DBM_TO_PW, sensitivity_max_dbm);

				// TX gain
				tmp_nodes = strdup(line_nodes);
				double tx_gain_db (atoi(GetField(tmp_nodes, IX_TX_GAIN)));
				node_container[node_ix].tx_gain = ConvertPower(DB_TO_LINEAR, tx_gain_db);

				// RX gain
				tmp_nodes = strdup(line_nodes);
				double rx_gain_db (atoi(GetField(tmp_nodes, IX_RX_GAIN)));
				node_container[node_ix].rx_gain = ConvertPower(DB_TO_LINEAR, rx_gain_db);

				// Channel bonding model
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].current_dcb_policy = atoi(GetField(tmp_nodes, IX_CHANNEL_BONDING_MODEL));

				// Default modulation
				tmp_nodes = strdup(line_nodes);
				node_container[node_ix].modulation_default = atoi(GetField(tmp_nodes, IX_MODULATION_DEFAULT));

				// Central frequency in GHz (e.g. 2.4)
				tmp_nodes = strdup(line_nodes);
				const char* central_frequency_char (GetField(tmp_nodes, IX_CENTRAL_FREQ));
				node_container[node_ix].central_frequency = atof(central_frequency_char) * pow(10,9);

				// Lambda (BO generation rate)
				tmp_nodes = strdup(line_nodes);
				const char* lambda_char (GetField(tmp_nodes, IX_LAMBDA));

				// IEEE protocol type
				tmp_nodes = strdup(line_nodes);
				const char* ieee_protocol_char (GetField(tmp_nodes, IX_IEEE_PROTOCOL_TYPE));
				node_container[node_ix].ieee_protocol = atof(ieee_protocol_char);

				// Traffic load (packet generation rate)
				tmp_nodes = strdup(line_nodes);
				const char* traffic_load_char (GetField(tmp_nodes, IX_TRAFFIC_LOAD));

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

				// System
				node_container[node_ix].simulation_time_komondor = simulation_time_komondor;
				node_container[node_ix].total_wlans_number = total_wlans_number;
				node_container[node_ix].total_nodes_number = total_nodes_number;
				node_container[node_ix].collisions_model = collisions_model;
				node_container[node_ix].capture_effect = capture_effect;
				node_container[node_ix].save_node_logs = save_node_logs;
				node_container[node_ix].print_node_logs = print_node_logs;
				node_container[node_ix].basic_channel_bandwidth = basic_channel_bandwidth;
				node_container[node_ix].num_channels_komondor = num_channels_komondor;
				node_container[node_ix].adjacent_channel_model = adjacent_channel_model;
				node_container[node_ix].default_destination_id = NODE_ID_NONE;
				node_container[node_ix].noise_level = noise_level;
				node_container[node_ix].constant_per = constant_per;
				node_container[node_ix].pdf_backoff = pdf_backoff;
				node_container[node_ix].path_loss_model = path_loss_model;
				node_container[node_ix].pdf_tx_time = pdf_tx_time;
				node_container[node_ix].frame_length = frame_length;
				node_container[node_ix].max_num_packets_aggregated = max_num_packets_aggregated;
				node_container[node_ix].ack_length = ack_length;
				node_container[node_ix].rts_length = rts_length;
				node_container[node_ix].cts_length = cts_length;
				node_container[node_ix].traffic_model = traffic_model;
				node_container[node_ix].backoff_type = backoff_type;
				node_container[node_ix].cw_adaptation = cw_adaptation;
				node_container[node_ix].pifs_activated = pifs_activated;
				node_container[node_ix].capture_effect_model = capture_effect_model;
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
				traffic_generator_container[node_ix].traffic_model = traffic_model;
				traffic_generator_container[node_ix].traffic_load = atof(traffic_load_char);
				traffic_generator_container[node_ix].lambda = atof(lambda_char);
				traffic_generator_container[node_ix].burst_rate = atof(lambda_char);

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

/*
 * GenerateAgents(): generates the agents according to the information in the input file.
 * Input arguments:
 * - agents_filename: agents filename
 */
void Komondor :: GenerateAgents(const char *agents_filename) {

	if (print_system_logs) printf("%s Generating agents...\n", LOG_LVL1);

	if (print_system_logs) printf("%s Reading agents input file '%s'...\n", LOG_LVL2, agents_filename);

	// STEP 1: set size of the agents container
	total_agents_number = GetNumOfLines(agents_filename);
	agent_container.SetSize(total_agents_number);

	if (print_system_logs) printf("%s Num. of agents (WLANs): %d/%d\n", LOG_LVL3, total_agents_number, total_wlans_number);

	// STEP 2: read the input file to determine the action space
	if (print_system_logs) printf("%s Setting action space...\n", LOG_LVL4);
	FILE* stream_agents = fopen(agents_filename, "r");
	char line_agents[CHAR_BUFFER_SIZE];
	first_line_skiped_flag = 0;	// Flag for skipping first informative line of input file

	int agent_ix (0);	// Auxiliary index

	while (fgets(line_agents, CHAR_BUFFER_SIZE, stream_agents)){

		if(!first_line_skiped_flag){

			first_line_skiped_flag = 1;

		} else{

			char* tmp_agents = strdup(line_agents);

			// Find the length of the channel actions array
			tmp_agents = strdup(line_agents);
			const char *channel_values_aux (GetField(tmp_agents, IX_AGENT_CHANNEL_VALUES));
			std::string channel_values_text;
			channel_values_text.append(ToString(channel_values_aux));
			const char *channel_aux;
			channel_aux = strtok ((char*)channel_values_text.c_str(),",");
			num_actions_channel = 0;
			while (channel_aux != NULL) {
				channel_aux = strtok (NULL, ",");
				++ num_actions_channel;
			}
			// Set the length of channel actions to agent's field
			agent_container[agent_ix].num_actions_channel = num_actions_channel;

			// Find the length of the pd actions array
			tmp_agents = strdup(line_agents);
			const char *pd_values_aux (GetField(tmp_agents, IX_AGENT_PD_VALUES));
			std::string pd_values_text;
			pd_values_text.append(ToString(pd_values_aux));
			const char *pd_aux;
			pd_aux = strtok ((char*)pd_values_text.c_str(),",");
			num_actions_sensitivity = 0;
			while (pd_aux != NULL) {
				pd_aux = strtok (NULL, ",");
				++ num_actions_sensitivity;
			}

			// Set the length of sensitivity actions to agent's field
			agent_container[agent_ix].num_actions_sensitivity = num_actions_sensitivity;

			// Find the length of the Tx power actions array
			tmp_agents = strdup(line_agents);
			const char *tx_power_values_aux (GetField(tmp_agents, IX_AGENT_TX_POWER_VALUES));
			std::string tx_power_values_text;
			tx_power_values_text.append(ToString(tx_power_values_aux));
			const char *tx_power_aux;
			tx_power_aux = strtok ((char*)tx_power_values_text.c_str(),",");
			num_actions_tx_power = 0;
			while (tx_power_aux != NULL) {
				tx_power_aux = strtok (NULL, ",");
				++ num_actions_tx_power;
			}

			// Set the length of Tx power actions to agent's field
			agent_container[agent_ix].num_actions_tx_power = num_actions_tx_power;

			// Find the length of the DCB actions actions array
			tmp_agents = strdup(line_agents);
			const char *policy_values_aux (GetField(tmp_agents, IX_AGENT_DCB_POLICY));
			std::string policy_values_text;
			policy_values_text.append(ToString(policy_values_aux));
			const char *policy_aux;
			policy_aux = strtok ((char*)policy_values_text.c_str(),",");
			num_actions_dcb_policy = 0;
			while (policy_aux != NULL) {
				policy_aux = strtok (NULL, ",");
				++num_actions_dcb_policy;
			}

			// Set the length of DCB actions to agent's field
			agent_container[agent_ix].num_actions_dcb_policy = num_actions_dcb_policy;

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

	while (fgets(line_agents, CHAR_BUFFER_SIZE, stream_agents)){

		if(!first_line_skiped_flag){

			first_line_skiped_flag = 1;

		} else{

			// Initialize actions and arrays in agents
			agent_container[agent_ix].InitializeAgent();

			// Agent ID
			agent_container[agent_ix].agent_id = agent_ix;

			// WLAN code
			char* tmp_agents (strdup(line_agents));
			const char *wlan_code_aux (GetField(tmp_agents, IX_AGENT_WLAN_CODE));
			std::string wlan_code;
			wlan_code.append(ToString(wlan_code_aux));
			agent_container[agent_ix].wlan_code = wlan_code.c_str();

			//  Centralized flag
			tmp_agents = strdup(line_agents);
			int centralized_flag (atoi(GetField(tmp_agents, IX_CENTRALIZED_FLAG)));
			agent_container[agent_ix].centralized_flag = centralized_flag;

			if(centralized_flag) {
				++total_controlled_agents_number;
				central_controller_flag = 1;
			}

			// Time between requests
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

			// DCB policy values
			tmp_agents = strdup(line_agents);
			std::string dcb_policy_values_text = ToString(GetField(tmp_agents, IX_AGENT_DCB_POLICY));

			// Fill the DCB policy actions array
			char *policy_aux_2;
			char *dcb_policy_values_text_char = new char[dcb_policy_values_text.length() + 1];
			strcpy(dcb_policy_values_text_char, dcb_policy_values_text.c_str());
			policy_aux_2 = strtok (dcb_policy_values_text_char,",");

			ix = 0;
			while (policy_aux_2 != NULL) {
				int a (atoi(policy_aux_2));
				agent_container[agent_ix].list_of_dcb_policy[ix] = a;
				policy_aux_2 = strtok (NULL, ",");
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
			int selected_strategy (atoi(GetField(tmp_agents, IX_AGENT_SELECTED_STRATEGY)));
			agent_container[agent_ix].selected_strategy = selected_strategy;

			// System
			agent_container[agent_ix].save_agent_logs = save_agent_logs;
			agent_container[agent_ix].print_agent_logs = print_agent_logs;

			// Initialize learning algorithm in agent
			agent_container[agent_ix].InitializeLearningAlgorithm();

			++agent_ix;
			free(tmp_agents);

		}
	}

	if (print_system_logs) printf("%s Agents parameters set!\n", LOG_LVL4);

}

/*
 * GenerateCentralController(): generates the central controller (if applicable)
 * Input arguments:
 * -
 */
void Komondor :: GenerateCentralController(const char *agents_filename) {

	if (print_system_logs) printf("%s Generating the Central Controller...\n", LOG_LVL1);

	// Despite we only have a single controller, it must be declared as an array,
	// in order to properly perform inport & outport connections
	central_controller.SetSize(1);

	if (total_controlled_agents_number > 0) {

		central_controller[0].agents_number = total_controlled_agents_number;
		central_controller[0].wlans_number = total_wlans_number;
		central_controller[0].InitializeCentralController();

		int *agents_list;
		agents_list = new int[total_controlled_agents_number];
		int agent_list_ix (0);					// Index considering the agents attached to the central entity
		double max_time_between_requests (0);	// To determine the maximum time between requests for agents

		for (int agent_ix = 0; agent_ix < total_controlled_agents_number; ++agent_ix) {
			if(agent_container[agent_ix].centralized_flag) {
				agents_list[agent_list_ix] = agent_container[agent_ix].agent_id;
				double agent_time_between_requests (agent_container[agent_list_ix].time_between_requests);
				if (agent_time_between_requests > max_time_between_requests) {
					central_controller[0].time_between_requests = agent_time_between_requests;
				}
				++agent_list_ix;
			}
		}

		// The overall "time between requests" is set to the maximum among all the agents
		central_controller[0].list_of_agents = agents_list;

		// Initialize the CC with parameters from the agents input file
		FILE* stream_cc = fopen(agents_filename, "r");
		char line_agents[CHAR_BUFFER_SIZE];
		char* tmp_agents (strdup(line_agents));
		first_line_skiped_flag = 0;		// Flag for skipping first informative line of input file

		while (fgets(line_agents, CHAR_BUFFER_SIZE, stream_cc)){
			if(!first_line_skiped_flag){
				first_line_skiped_flag = 1;
			} else{

				// Type OF reward
				tmp_agents = strdup(line_agents);
				int type_of_reward (atoi(GetField(tmp_agents, IX_AGENT_TYPE_OF_REWARD)));
				central_controller[0].type_of_reward = type_of_reward;
				// Learning mechanism
				tmp_agents = strdup(line_agents);
				int learning_mechanism (atoi(GetField(tmp_agents, IX_AGENT_LEARNING_MECHANISM)));
				central_controller[0].learning_mechanism = learning_mechanism;
				// Selected strategy
				tmp_agents = strdup(line_agents);
				int selected_strategy (atoi(GetField(tmp_agents, IX_AGENT_SELECTED_STRATEGY)));
				central_controller[0].selected_strategy = selected_strategy;

				// Find the length of the channel actions array
				tmp_agents = strdup(line_agents);
				const char *channel_values_aux (GetField(tmp_agents, IX_AGENT_CHANNEL_VALUES));
				std::string channel_values_text;
				channel_values_text.append(ToString(channel_values_aux));
				const char *channels_aux;
				channels_aux = strtok ((char*)channel_values_text.c_str(),",");
				int num_actions_channels = 0;
				while (channels_aux != NULL) {
					channels_aux = strtok (NULL, ",");
					++ num_actions_channels;
				}
				central_controller[0].num_channels = num_actions_channels;

				free(tmp_agents);

			}
		}

		// System logs
		central_controller[0].save_controller_logs = save_agent_logs;
		central_controller[0].print_controller_logs = print_agent_logs;

		central_controller[0].total_nodes_number = total_nodes_number;

//		// Initialize learning algorithm in the CC
//		central_controller[0].InitializeLearningAlgorithm();

	} else {
		printf("%s WARNING: THE CENTRAL CONTROLLER DOES NOT HAVE ANY ATTACHED AGENT! CHECK YOUR AGENTS' INPUT FILE\n", LOG_LVL2);
	}

}

/***************************/
/* LOG AND DEBUG FUNCTIONS */
/***************************/

/*
 * PrintSystemInfo(): prints the Komondor environment general info
 */
void Komondor :: PrintSystemInfo(){

	if (print_system_logs){
		printf("%s total_nodes_number = %d\n", LOG_LVL3, total_nodes_number);
		printf("%s num_channels_komondor = %d\n", LOG_LVL3, num_channels_komondor);
		printf("%s basic_channel_bandwidth = %f MHz\n", LOG_LVL3, basic_channel_bandwidth);
		printf("%s pdf_backoff = %d\n", LOG_LVL3, pdf_backoff);
		printf("%s pdf_tx_time = %d\n", LOG_LVL3, pdf_tx_time);
		printf("%s frame_length = %d bits\n", LOG_LVL3, frame_length);
		printf("%s traffic_model = %d\n", LOG_LVL3, traffic_model);
		printf("%s backoff_type = %d\n", LOG_LVL3, backoff_type);
		printf("%s cw_adaptation = %d\n", LOG_LVL3, cw_adaptation);
		printf("%s pifs_activated = %d\n", LOG_LVL3, pifs_activated);
		printf("%s capture_effect_model = %d\n", LOG_LVL3, capture_effect_model);
		printf("%s max_num_packets_aggregated = %d\n", LOG_LVL3, max_num_packets_aggregated);
		printf("%s path_loss_model = %d\n", LOG_LVL3, path_loss_model);
		printf("%s capture_effect = %f [linear] (%f dB)\n", LOG_LVL3, capture_effect, ConvertPower(LINEAR_TO_DB, capture_effect));
		printf("%s noise_level = %f pW (%f dBm)\n",
				LOG_LVL3, noise_level, ConvertPower(PW_TO_DBM, noise_level));
		printf("%s adjacent_channel_model = %d\n", LOG_LVL3, adjacent_channel_model);
		printf("%s collisions_model = %d\n", LOG_LVL3, collisions_model);
		printf("%s Constant PER = %f\n", LOG_LVL3, constant_per);
		printf("\n");
	}
}

/*
 * WriteSystemInfo(): writes the Komondor environment general info
 * Input arguments:
 * - logger: AP or nodes filename
 */
void Komondor :: WriteSystemInfo(Logger logger){

	fprintf(logger.file, "%s total_nodes_number = %d\n", LOG_LVL3, total_nodes_number);
	fprintf(logger.file, "%s num_channels_komondor = %d\n", LOG_LVL3, num_channels_komondor);
	fprintf(logger.file, "%s basic_channel_bandwidth = %f\n", LOG_LVL3, basic_channel_bandwidth);
	fprintf(logger.file, "%s pdf_backoff = %d\n", LOG_LVL3, pdf_backoff);
	fprintf(logger.file, "%s pdf_tx_time = %d\n", LOG_LVL3, pdf_tx_time);
	fprintf(logger.file, "%s frame_length = %d bits\n", LOG_LVL3, frame_length);
	fprintf(logger.file, "%s ack_length = %d bits\n", LOG_LVL3, ack_length);
	fprintf(logger.file, "%s max_num_packets_aggregated = %d\n", LOG_LVL3, max_num_packets_aggregated);
	fprintf(logger.file, "%s path_loss_model = %d\n", LOG_LVL3, path_loss_model);
	fprintf(logger.file, "%s capture_effect = %f\n", LOG_LVL3, capture_effect);
	fprintf(logger.file, "%s noise_level = %f dBm\n", LOG_LVL3, noise_level);
	fprintf(logger.file, "%s adjacent_channel_model = %d\n", LOG_LVL3, adjacent_channel_model);
	fprintf(logger.file, "%s collisions_model = %d\n", LOG_LVL3, collisions_model);
}

/*
 * PrintAllNodesInfo(): prints the nodes info
 * Input arguments:
 * - info_detail_level: level of detail of the written logs
 */
void Komondor :: PrintAllNodesInfo(int info_detail_level){
	for(int n = 0; n < total_nodes_number; ++n ){
		node_container[n].PrintNodeInfo(info_detail_level);
	}
}

/*
 * PrintAllWlansInfo(): prints the WLANs info
 */
void Komondor :: PrintAllWlansInfo(){
	for(int w = 0; w < total_wlans_number; ++w){
		wlan_container[w].PrintWlanInfo();
	}
}

/*
 * PrintAgentsInfo(): prints the Agents info
 */
void Komondor :: PrintAllAgentsInfo(){
	for(int a = 0; a < total_agents_number; ++a ){
		agent_container[a].PrintAgentInfo();
	}
}

/*
 * WriteAllNodesInfo(): writes the WLANs info in a file
 * Input arguments:
 * - logger: logger containing the file to write on
 */
void Komondor :: WriteAllWlansInfo(Logger logger, std::string header_str){
	for(int w = 0; w < total_wlans_number; ++w){
		wlan_container[w].WriteWlanInfo(logger, header_str.c_str());
	}
}

/*
 * WriteAllNodesInfo(): writes the nodes info in a file
 * Input arguments:
 * - logger: logger containing the file to write on
 * - info_detail_level: level of detail of the written logs
 */
void Komondor :: WriteAllNodesInfo(Logger logger, int info_detail_level, std::string header_str){
	for(int n = 0; n < total_nodes_number; ++n){
		node_container[n].WriteNodeInfo(logger, info_detail_level, header_str.c_str());
	}
}

/*******************/
/* FILES FUNCTIONS */
/*******************/

/*
 * GetField(): returns a field corresponding to a given index from a CSV file
 * Input arguments:
 * - line: line of the CSV
 * - num: field number (index)
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

/*
 * GetNumOfLines(): returns the number of lines of a csv file
 * Input arguments:
 * - filename: CSV filename
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

/*
 * GetNumOfNodes(): returns the number of nodes of a given type (0: AP, 1: STA, 2: Free Node)
 * Input arguments:
 * - nodes_filename: nodes configuration filename
 * - node_type: type of node to consider in the counting
 * - wlan_id: wlan to consider in the counting
 */
int Komondor :: GetNumOfNodes(const char *nodes_filename, int node_type, std::string wlan_code){

	int num_nodes(0);
	char line_nodes[CHAR_BUFFER_SIZE];
	first_line_skiped_flag = 0;
	int type_found;
	std::string wlan_code_found;

	FILE* stream_nodes = fopen(nodes_filename, "r");

	if (!stream_nodes){
		printf("ERROR: Nodes configuration file %s not found!\n", nodes_filename);
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
	char *system_input_filename;
	char *nodes_input_filename;
	char *agents_input_filename;
	std::string script_output_filename;
	std::string simulation_code;
	int save_system_logs;
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

		system_input_filename = argv[1];
		nodes_input_filename = argv[2];
		agents_input_filename = argv[3];
		script_output_filename = ToString(argv[4]);
		simulation_code = ToString(argv[5]);
		save_system_logs = atoi(argv[6]);
		save_node_logs = atoi(argv[7]);
		save_agent_logs = atoi(argv[8]);
		print_system_logs = atoi(argv[9]);
		print_node_logs = atoi(argv[10]);
		print_agent_logs = atoi(argv[11]);
		sim_time = atof(argv[12]);
		seed = atoi(argv[13]);

		agents_enabled = TRUE;

		if (print_system_logs) printf("%s FULL configuration entered per console.\n", LOG_LVL1);

	} else if(argc == NUM_FULL_ARGUMENTS_CONSOLE_NO_AGENTS){	// Configuration without agents

		system_input_filename = argv[1];
		nodes_input_filename = argv[2];
		script_output_filename = ToString(argv[3]);
		simulation_code = ToString(argv[4]);
		save_system_logs = atoi(argv[5]);
		save_node_logs = atoi(argv[6]);
		print_system_logs = atoi(argv[7]);
		print_node_logs = atoi(argv[8]);
		sim_time = atof(argv[9]);
		seed = atoi(argv[10]);

		agents_enabled = FALSE;

		if (print_system_logs) printf("%s FULL configuration entered per console (NO AGENTS).\n", LOG_LVL1);

	} else if(argc == NUM_PARTIAL_ARGUMENTS_CONSOLE) {	// Partial configuration entered per console

		system_input_filename = argv[1];
		nodes_input_filename = argv[2];
		sim_time = atof(argv[3]);
		seed = atoi(argv[4]);

		// Default values
		script_output_filename.append(ToString(DEFAULT_SCRIPT_FILENAME));
		simulation_code.append(ToString(DEFAULT_SIMULATION_CODE));
		save_system_logs = DEFAULT_WRITE_SYSTEM_LOGS;
		save_node_logs = DEFAULT_WRITE_NODE_LOGS;
		print_system_logs = DEFAULT_PRINT_SYSTEM_LOGS;
		print_node_logs = DEFAULT_PRINT_NODE_LOGS;

		agents_enabled = FALSE;

		if (print_system_logs) printf("%s PARTIAL configuration entered per console. "
				"Some parameters are set by DEFAULT.\n", LOG_LVL1);

	} else if(argc == NUM_PARTIAL_ARGUMENTS_SCRIPT) {	// Partial configuration entered per console (useful for scripts)

		system_input_filename = argv[1];
		nodes_input_filename = argv[2];
		simulation_code = ToString(argv[3]);	// For scripts --> usefult to identify simulations
		sim_time = atof(argv[4]);
		seed = atoi(argv[5]);

		// Default values
		script_output_filename.append(ToString(DEFAULT_SCRIPT_FILENAME));

		save_system_logs = DEFAULT_WRITE_SYSTEM_LOGS;
		save_node_logs = DEFAULT_WRITE_NODE_LOGS;
		print_system_logs = DEFAULT_PRINT_SYSTEM_LOGS;
		print_node_logs = DEFAULT_PRINT_NODE_LOGS;

		agents_enabled = FALSE;

		if (print_system_logs) printf("%s PARTIAL configuration entered per script. "
				"Some parameters are set by DEFAULT.\n", LOG_LVL1);

	} else {

		printf("%sERROR: Console arguments were not set properly!\n "
				" + For FULL configuration setting execute\n"
				"    ./Komondor -system_input_filename -nodes_input_filename -script_output_filename "
				"-simulation_code -save_system_logs -save_node_logs -print_node_logs -print_system_logs "
				"- sim_time -seed\n"
				" + For PARTIAL configuration setting execute\n"
				"    ./KomondorSimulation -system_input_filename -nodes_input_filename - sim_time - seed\n", LOG_LVL1);
		return(-1);
	}

	if (print_system_logs) {
		printf("%s Komondor input configuration:\n", LOG_LVL1);
		printf("%s system_input_filename: %s\n", LOG_LVL2, system_input_filename);
		printf("%s nodes_input_filename: %s\n", LOG_LVL2, nodes_input_filename);
		printf("%s agents_enabled: %d\n", LOG_LVL2, agents_enabled);
		if (agents_enabled) { printf("%s agents_input_filename: %s\n", LOG_LVL2, agents_input_filename); }
		printf("%s script_output_filename: %s\n", LOG_LVL2, script_output_filename.c_str());
		printf("%s simulation_code: %s\n", LOG_LVL2, simulation_code.c_str());
		printf("%s save_system_logs: %d\n", LOG_LVL2, save_system_logs);
		printf("%s save_node_logs: %d\n", LOG_LVL2, save_node_logs);
		printf("%s print_system_logs: %d\n", LOG_LVL2, print_system_logs);
		printf("%s print_node_logs: %d\n", LOG_LVL2, print_node_logs);
		printf("%s sim_time: %f s\n", LOG_LVL2, sim_time);
		printf("%s seed: %d\n", LOG_LVL2, seed);
	}

	// Generate Komondor component
	Komondor test;
	test.Seed = seed;
	srand(seed); // Needed for ensuring randomness dependency on seed
	test.StopTime(sim_time);
	test.Setup(sim_time, save_system_logs, save_node_logs, save_agent_logs, print_system_logs, print_node_logs, print_agent_logs,
		system_input_filename, nodes_input_filename, script_output_filename.c_str(), simulation_code.c_str(), seed,
		agents_enabled, agents_input_filename);

	printf("------------------------------------------\n");
	printf("%s SIMULATION '%s' STARTED\n", LOG_LVL1, simulation_code.c_str());

	test.Run();

	return(0);
};
