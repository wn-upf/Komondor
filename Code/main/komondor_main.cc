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
#include <getopt.h> // Required for parsing flags from user input

#include ".././COST/cost.h"

#include "../list_of_macros.h"

#include "../structures/logical_nack.h"
#include "../structures/notification.h"
#include "../structures/wlan.h"

#include "node.h"
#include "traffic_generator.h"
#include "agent.h"
#include "central_controller.h"

/* Sequential simulation engine from where the system to be simulated is derived. */
component Komondor : public CostSimEng {

	// Methods
	public:

		void Setup(double simulation_time_komondor, int save_node_logs, int save_agent_logs,
			int print_node_logs, int print_system_logs, int print_agent_logs, const char *nodes_filename,
			const char *script_filename, const char *simulation_code, int seed_console, int agents_enabled,
			const char *agents_filename, int mapc_enabled, const char *mapc_filename);
		void Stop();
		void Start();

		void SetupEnvironmentByReadingConfigFile(const char *config_filename);
		void GenerateNodesByReadingInputFile(const char *nodes_filename);

		void GenerateAgents(const char *agents_filename, const char *simulation_code_console);
		void GenerateCentralController(const char *agents_filename);

		void GenerateMapcConfiByReadingInputFile(const char *mapc_filename);

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

		int total_nodes_number;						///> Total number of nodes
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
		int pdf_tx_time;				///> Probability distribution type of the transmission time (0: exponential, 1: deterministic)
		int path_loss_model;			///> Path loss model (0: free-space, 1: Okumura-Hata model - Uban areas)
		int adjacent_channel_model;		///> Co-channel interference model
		int collisions_model;			///> Collisions model
		double constant_per;			///> Constant PER for successful transmissions
		int capture_effect_model;		///> Capture Effect model (default or IEEE 802.11-based)
		int simulation_index;			///> Simulation index for selecting the type of output in scripts

		int agents_enabled;				///> Determined according to the input (for generating agents or not)
		int mapc_enabled;				///> Determined according to the input (for using MAPC or not)

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
		const char *mapc_input_filename;	///> Filename of the MAPC configuration CSV
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
		int agents_enabled_console, const char *agents_input_filename_console,
		int mapc_enabled_console, const char *mapc_input_filename_console) {

	// Setup variables corresponding to the console's input
	simulation_time_komondor = sim_time_console;
	save_node_logs = save_node_logs_console;
	save_agent_logs = save_agent_logs_console;
	print_node_logs = print_node_logs_console;
	print_system_logs = print_system_logs_console;
	print_agent_logs = print_agent_logs_console;
	nodes_input_filename = nodes_input_filename_console;
	mapc_input_filename = mapc_input_filename_console;
	agents_input_filename = agents_input_filename_console;
	seed = seed_console;
	agents_enabled = agents_enabled_console;
	mapc_enabled = mapc_enabled_console;
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
	const char *filename_test = "../config_models"; // HARDCODED
	SetupEnvironmentByReadingConfigFile(filename_test);

	// Generate nodes
	GenerateNodesByReadingInputFile(nodes_input_filename);

	// Compute distance of each pair of nodes
	for(int i = 0; i < total_nodes_number; ++i) {
		node_container[i].distances_array = new double[total_nodes_number];
		node_container[i].received_power_array = new double[total_nodes_number];
		for(int j = 0; j < total_nodes_number; ++j) {
			// Compute and assign distances for each other node
			node_container[i].distances_array[j] = ComputeDistance(node_container[i].node_params.x,node_container[i].node_params.y,
				node_container[i].node_params.z,node_container[j].node_params.x,node_container[j].node_params.y,node_container[j].node_params.z);
			// Compute and assign the received power from each other node
			if(i == j) {
				node_container[i].received_power_array[j] = 0;
			} else {
				node_container[i].received_power_array[j] = ComputePowerReceived(node_container[i].distances_array[j],
					node_container[j].node_params.tx_power_default, node_container[i].node_params.central_frequency, path_loss_model);
			}
		}
	}

	// Compute the maximum power received from each WLAN
	for(int i = 0; i < total_nodes_number; ++i) {
		double max_power_received_per_wlan;
		if (node_container[i].node_params.node_type == NODE_TYPE_AP) {
			node_container[i].max_received_power_in_ap_per_wlan = new double[total_wlans_number];
			for(int j = 0; j < total_wlans_number; ++j) {
				if (strcmp(node_container[i].node_params.wlan_code.c_str(),wlan_container[j].wlan_code.c_str()) == 0) {
					// Same WLAN
					node_container[i].max_received_power_in_ap_per_wlan[j] = 0;
				} else {
					// Different WLAN
					max_power_received_per_wlan = -1000;
					for (int k = 0; k < total_nodes_number; ++k) {
						// Check only nodes in WLAN "j"
						if(strcmp(node_container[k].node_params.wlan_code.c_str(),wlan_container[j].wlan_code.c_str()) == 0) {
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

	// Populate per-node position lookup arrays (used for per-TXOP beamforming)
	for (int i = 0; i < total_nodes_number; ++i) {
		node_container[i].all_node_x = new double[total_nodes_number];
		node_container[i].all_node_y = new double[total_nodes_number];
		node_container[i].all_node_z = new double[total_nodes_number];
		for (int j = 0; j < total_nodes_number; ++j) {
			node_container[i].all_node_x[j] = node_container[j].node_params.x;
			node_container[i].all_node_y[j] = node_container[j].node_params.y;
			node_container[i].all_node_z[j] = node_container[j].node_params.z;
		}
	}

	// Initialize arrays for the token-based channel access
	for (int i = 0; i < total_nodes_number; ++i) {
		if (node_container[i].node_params.backoff_type == BACKOFF_TOKENIZED){
			node_container[i].token_order_list = new int[total_nodes_number];
			node_container[i].num_missed_tokens_list = new int[total_nodes_number];
			for(int j = 0; j < total_nodes_number; ++j) {
				node_container[i].token_order_list[j] = DEVICE_INACTIVE_FOR_TOKEN;
				node_container[i].num_missed_tokens_list[j] = 0;
			}
		}
	}

	// Generate MAPC groups
	GenerateMapcConfiByReadingInputFile(mapc_input_filename);

	// Re-link nodes with updated WLAN info (MAPC fields populated after GenerateNodesByReadingInputFile)
	for (int n = 0; n < total_nodes_number; ++n) {
		for (int w = 0; w < total_wlans_number; ++w) {
			if (strcmp(node_container[n].node_params.wlan_code.c_str(), wlan_container[w].wlan_code.c_str()) == 0) {
				node_container[n].wlan = wlan_container[w];
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
		printf("\n%s System configuration: \n", LOG_LVL2);
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
	ValidateInput(total_nodes_number, node_container, print_system_logs);

	// Set connections among nodes
	for(int n = 0; n < total_nodes_number; ++n){

		connect traffic_generator_container[n].outportNewPacketGenerated,node_container[n].InportNewPacketGenerated;

		for(int m=0; m < total_nodes_number; ++m) {

			connect node_container[n].outportSelfStartTX,node_container[m].InportSomeNodeStartTX;
			connect node_container[n].outportSelfFinishTX,node_container[m].InportSomeNodeFinishTX;
			connect node_container[n].outportSendLogicalNack,node_container[m].InportNackReceived;

			// Nodes belonging to the same WLAN
			if(strcmp(node_container[n].node_params.wlan_code.c_str(),node_container[m].node_params.wlan_code.c_str()) == 0 && n!=m) {
				// Connections regarding MCS
				connect node_container[n].outportAskForTxModulation,node_container[m].InportMCSRequestReceived;
				connect node_container[n].outportAnswerTxModulation,node_container[m].InportMCSResponseReceived;
				// Connections regarding changes in the WLAN
				connect node_container[n].outportSetNewWlanConfiguration,node_container[m].InportNewWlanConfigurationReceived;
				// Connections Spatial Reuse
				if(node_container[n].node_params.node_type == NODE_TYPE_AP && node_container[m].node_params.node_type == NODE_TYPE_STA) {
					connect node_container[m].outportRequestSpatialReuseConfiguration,node_container[n].InportRequestSpatialReuseConfiguration;
					connect node_container[n].outportNewSpatialReuseConfiguration,node_container[m].InportNewSpatialReuseConfiguration;
				}
			}
		}

		if (agents_enabled) {
			// Set connections among APs and Agents
			if ( node_container[n].node_params.node_type == NODE_TYPE_AP ) {
				for(int w = 0; w < total_agents_number; ++w){
					// Connect the agent to the corresponding AP, according to "wlan_code"
					if (strcmp(node_container[n].node_params.wlan_code.c_str(), agent_container[w].wlan_code.c_str()) == 0) {
						connect agent_container[w].outportRequestInformationToAp,node_container[n].InportReceivingRequestFromAgent;
						connect node_container[n].outportAnswerToAgent,agent_container[w].InportReceivingInformationFromAp;
						connect agent_container[w].outportSendConfigurationToAp,node_container[n].InportReceiveConfigurationFromAgent;
					}
				}
			}
		}
	}

	// Connect the agents to the central controller, if enabled
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

/*******************/
/* Include methods */
/*******************/
#include "../methods/utils/output_generation_methods.h"
#include "../methods/utils/print_and_write_methods.h"
#include "../methods/utils/input_methods/input_validator.h"
#include "../methods/utils/input_methods/input_loader.h"

/**********/
/* main() */
/**********/
int main(int argc, char *argv[]){

	printf("\n");
	printf("*************************************************************************************\n");
	printf("%s KOM8NDOR Wireless Network Simulator\n", LOG_LVL1);
	printf("%s Copyright (C) 2026-2031, and GNU GPL'd, by Francesc Wilhelmi & Sergio Barrachina\n", LOG_LVL1);
	printf("%s GitHub repository: https://github.com/wn-upf/Komondor\n", LOG_LVL2);
	printf("*************************************************************************************\n");
	printf("\n\n");

	// -------------------------------------------------------
    // 1. SET DEFAULTS 
    // -------------------------------------------------------
    std::string nodes_input_filename = "";      // Mandatory
    std::string agents_input_filename = "";     // Optional
    std::string mapc_input_filename = "";       // Optional
    std::string script_output_filename = "../output/default_output.txt";
    std::string simulation_code = "SIM_001";
    
    int save_node_logs = 0;
    int save_agent_logs = 0;
    int print_system_logs = 1; // Default to ON for visibility
    int print_node_logs = 1;
    int print_agent_logs = 1;
    
    double sim_time = 10.0;
    int seed = 1;

    // "Modes" are now just flags. Default to false.
    int agents_enabled = 0;
    int mapc_enabled = 0;

    // -------------------------------------------------------
    // 2. DEFINE FLAGS
    // -------------------------------------------------------
    static struct option long_options[] = {
        // Essential Inputs
        {"nodes",     required_argument, 0, 'n'},
        {"time",      required_argument, 0, 't'},
        {"seed",      required_argument, 0, 's'},
        {"code",      required_argument, 0, 'c'},
        {"out",       required_argument, 0, 'o'},

        // Optional Features (Modes)
        {"agents",    required_argument, 0, 'a'}, // Providing this ENABLES agents
        {"mapc",      required_argument, 0, 'm'}, // Providing this ENABLES MAPC

        // Flags (0/1)
        {"logs-sys",  	required_argument, 0, 'L'}, 
        {"logs-node", 	required_argument, 0, 'l'},
        {"save-node", 	required_argument, 0, 'S'},
		{"save-agent", 	required_argument, 0, 'A'},
        
        // Help
        {"help",      no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    // -------------------------------------------------------
    // 3. PARSE ARGUMENTS
    // -------------------------------------------------------
    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "n:t:s:c:o:a:m:L:l:S:A:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'n': nodes_input_filename = optarg; break;
            case 't': sim_time = atof(optarg); break;
            case 's': seed = atoi(optarg); break;
            case 'c': simulation_code = optarg; break;
            case 'o': script_output_filename = optarg; break;
            
            // Auto-enable modes if file is provided
            case 'a': 
                agents_input_filename = optarg; 
                agents_enabled = 1; 
                break;
            case 'm': 
                mapc_input_filename = optarg; 
                mapc_enabled = 1; 
                break;

            // Logging
            case 'L': print_system_logs = atoi(optarg); break;
            case 'l': print_node_logs = atoi(optarg); break;
            case 'S': save_node_logs = atoi(optarg); break;
			case 'A': save_agent_logs = atoi(optarg); break;

            case 'h':
            default:
                printf("Usage: ./Komondor --nodes <file> [OPTIONS]\n");
                printf("  --nodes <file>   : Input nodes file (Required)\n");
                printf("  --out <file>     : Output file (Default: ../output/default_output.txt)\n");
                printf("  --time <sec>     : Sim time (Default: 10.0)\n");
                printf("  --code <str>     : Simulation code (Default: SIM_001)\n");
                printf("  --seed <int>	   : Random seed (Default: 1)\n");

				printf("Flags:\n");
				printf("  --logs-sys <int>      : Print system logs (Default: 0)\n");
				printf("  --logs-node <int>     : Print node logs (Default: 0)\n");
				printf("  --save-node <int>     : Save node logs (Default: 0)\n");
				
				printf("Optional arguments:\n");
				printf("  --agents <file>  : Input agents file (Enables Agents)\n");
				printf("  --save-agent <int>     : Random seed (Default: 0)\n");
                printf("  --mapc <file>    : Input MAPC file (Enables MAPC)\n");
				printf("\n");
                exit(0);
        }
    }

    // -------------------------------------------------------
    // 4. VALIDATION
    // -------------------------------------------------------
    if (nodes_input_filename.empty()) {
        printf("ERROR: You must provide a nodes file using --nodes or -n\n");
        exit(-1);
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

	// -------------------------------------------------------
    // 5. RUN SIMULATION
    // -------------------------------------------------------

    Komondor komondor_simulation;
    
	komondor_simulation.StopTime(sim_time);
	
    komondor_simulation.Setup(
        sim_time, 
        save_node_logs, 
        save_agent_logs, 
        print_system_logs,
        print_node_logs, 
        print_agent_logs, 
        nodes_input_filename.c_str(), 
        script_output_filename.c_str(),
        simulation_code.c_str(), 
        seed, 
        agents_enabled, 
        agents_input_filename.c_str(), 
        mapc_enabled, 
        mapc_input_filename.c_str()
    );

    printf("------------------------------------------\n");
    printf("%s SIMULATION '%s' STARTED\n", LOG_LVL1, simulation_code.c_str());
    // if(agents_enabled) printf("%s Agents: ENABLED\n", LOG_LVL1);
    // if(mapc_enabled)   printf("%s MAPC:   ENABLED\n", LOG_LVL1);

    komondor_simulation.Run();

    return 0;

};
