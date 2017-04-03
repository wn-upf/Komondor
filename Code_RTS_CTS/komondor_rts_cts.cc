/* TODO: DEFINE copyright headers.*/

/* This is just an sketch of what our Komondor headers should look like.
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * All rights reserved.
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
 * - Bla bla bla...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include ".././COST/cost.h"

#include "../list_of_macros.h"
#include "../structures/logical_nack.h"
#include "../structures/notification.h"
#include "../structures/wlan.h"

#include "node_rts_cts.h"

/* Sequential simulation engine from where the system to be simulated is derived. */
component Komondor : public CostSimEng {

	// Methods
	public:

		void Setup(double simulation_time_komondor, int save_system_logs,  int save_node_logs,
				int print_node_logs, int print_system_logs, char *system_filename,
				char *nodes_filename, char *script_filename, char *simulation_code);
		void Stop();
		void Start();
		void InputChecker();

		void SetupEnvironmentByReadingInputFile(char *system_filename);
		void GenerateNodes(char *nodes_filename);
		void GenerateNodesByReadingNodesInputFile(char *nodes_filename);
		void GenerateNodesByReadingAPsInputFile(char *nodes_filename);

		int GetNumOfLines(char *nodes_filename);
		int GetNumOfNodes(char *nodes_filename, int node_type, char *wlan_code);

		void printSystemInfo();
		void PrintAllWlansInfo();
		void PrintAllNodesInfo(int info_detail_level);
		void WriteSystemInfo(Logger logger);
		void WriteAllWlansInfo(Logger logger, char *header_string);
		void WriteAllNodesInfo(Logger logger, int info_detail_level, char *header_string);

	// Public items (to shared with the nodes)
	public:

		Node[] node_container;	// Container of nodes (i.e., APs, STAs, ...)
		Wlan *wlan_container;			// Container of WLANs
		int total_nodes_number;			// Total number of nodes
		int total_wlans_number;			// Total number of WLANs

		// Parameters entered per console
		int save_node_logs;					// Flag for activating the log writting of nodes
		int print_node_logs;				// Flag for activating the printing of node logs
		double simulation_time_komondor;	// Simulation time [s]

		// Parameters entered via system file
		int num_channels_komondor;		// Number of subchannels composing the whole channel
		double basic_channel_bandwidth;	// Basic channel bandwidth [Mbps]
		int pdf_backoff;				// Probability distribution type of the backoff (0: exponential, 1: deterministic)
		int pdf_tx_time;				// Probability distribution type of the transmission time (0: exponential, 1: deterministic)
		int packet_length;				// Packet length [bits]
		int ack_length;					// ACK length [bits]
		int rts_length;					// RTS length [bits]
		int cts_length;					// CTS length [bits]
		int num_packets_aggregated;		// Number of packets aggregated in one transmission
		int path_loss_model;			// Path loss model (0: free-space, 1: Okumura-Hata model - Uban areas)
		double capture_effect;			// Capture effect threshold [linear ratio]
		double noise_level;				// Environment noise [pW]
		int cochannel_model;			// Co-channel interference model
		int collisions_model;			// Collisions model
		double SIFS;					// Short Interframe Space (SIFS) [s]
		double DIFS;					// DCF Interframe Space (DIFS) [s]
		double constant_per;			// Constant PER for successful transmissions
		int traffic_model;				// Traffic model (0: full buffer, 1: poisson, 2: deterministic)
		int backoff_type;				// Type of Backoff (0: Slotted 1: Continuous)

	// Private items
	private:

		int save_system_logs;			// Flag for activating the log writting of the Komondor system
		int print_system_logs;			// Flag for activating the printing of system logs
		char *simulation_code;			// Komondor simulation code
		FILE *simulation_output_file;	// File for the output logs (including statistics)
		FILE *script_output_file;		// File for the whole input files included in the script
		Logger logger_simulation;		// Logger for the simulation output file
		Logger logger_script;			// Logger for the script file (containing 1+ simulations)

		// Auxiliar variables
		int first_line_skiped_flag;		// Flag for skipping first informative line of input file
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
		int print_system_logs_console, int print_node_logs_console, char *system_input_filename,
		char *nodes_input_filename, char *script_output_filename, char *simulation_code_console){

	simulation_time_komondor = sim_time_console;
	save_node_logs = save_node_logs_console;
	save_system_logs = save_system_logs_console;
	print_node_logs = print_node_logs_console;
	print_system_logs = print_system_logs_console;
	simulation_code = (char *) malloc((strlen(simulation_code_console) + 1) * sizeof(*simulation_code));
	sprintf(simulation_code, "%s", simulation_code_console);
	total_nodes_number = 0;

	// Generate output files

	if (print_system_logs) printf("%s Creating output files\n", LOG_LVL1);
	const char *simulation_filename_root = "output/simulation_output";
	char *simulation_filename_remove =
			(char *) malloc( (strlen(simulation_filename_root) + strlen(simulation_code) + 6)
			* sizeof(*simulation_filename_remove));

	sprintf(simulation_filename_remove, "%s_%s.txt", simulation_filename_root, simulation_code);

	char *simulation_filename_fopen = (char *) malloc(strlen(simulation_filename_remove) + 4);
	sprintf(simulation_filename_fopen, "./%s", simulation_filename_remove);

	if(remove(simulation_filename_remove) == 0){
		if (print_system_logs) printf("%s Simulation output file '%s' found and removed. New one created!\n",
				LOG_LVL2, simulation_filename_remove);
	} else {
		if (print_system_logs) printf("%s Simulation output file '%s' created!\n",
							LOG_LVL2, simulation_filename_remove);
	}

	// Get loggers to write in output files
	simulation_output_file = fopen(simulation_filename_fopen,"at");
	logger_simulation.save_logs = SAVE_LOG;
	logger_simulation.file = simulation_output_file;

	script_output_file = fopen(script_output_filename,"at");	// Script output is removed when script is executed
	logger_script.save_logs = SAVE_LOG;
	logger_script.file = script_output_file;

	fprintf(logger_script.file, "------------------------------------\n");
	fprintf(logger_script.file, "%s KOMONDOR SIMULATION '%s'\n", LOG_LVL1, simulation_code);
	// Read system (environment) file
	SetupEnvironmentByReadingInputFile(system_input_filename);

	// Generate nodes
	GenerateNodes(nodes_input_filename);

	if (print_system_logs) {
		printf("%s System configuration: \n", LOG_LVL2);
		printSystemInfo();
		printf("%s Wlans generated!\n", LOG_LVL2);
		PrintAllWlansInfo();
		if (print_system_logs) printf("\n");
		printf("%s Nodes generated!\n", LOG_LVL2);
		PrintAllNodesInfo(INFO_DETAIL_LEVEL_0);
		if (print_system_logs) printf("\n\n");
	}

	fprintf(logger_simulation.file, "------------------------------------\n");

	if (save_system_logs){

		fprintf(logger_simulation.file, "%s System configuration: \n", LOG_LVL2);
		WriteSystemInfo(logger_simulation);
		fprintf(logger_script.file, "%s System configuration: \n", LOG_LVL2);
		WriteSystemInfo(logger_script);

		char *header_string = (char *) malloc(INTEGER_SIZE);
		sprintf(header_string, "%s", LOG_LVL3);

		fprintf(logger_simulation.file, "%s Wlans generated!\n", LOG_LVL2);
		WriteAllWlansInfo(logger_simulation, header_string);
		fprintf(logger_script.file, "%s Wlans generated!\n", LOG_LVL2);
		WriteAllWlansInfo(logger_script, header_string);

		fprintf(logger_simulation.file, "%s Nodes generated!\n", LOG_LVL2);
		WriteAllNodesInfo(logger_simulation, INFO_DETAIL_LEVEL_0, header_string);
		fprintf(logger_script.file, "%s Nodes generated!\n", LOG_LVL2);
		WriteAllNodesInfo(logger_script, INFO_DETAIL_LEVEL_0, header_string);
	}

	// Set connections among nodes
	for(int n = 0; n < total_nodes_number; n++){

		for(int m=0; m < total_nodes_number; m++) {

			connect node_container[n].outportSelfStartTX,node_container[m].InportSomeNodeStartTX;
			connect node_container[n].outportSelfFinishTX,node_container[m].InportSomeNodeFinishTX;
			connect node_container[n].outportSendLogicalNack,node_container[m].InportNackReceived;

			if(strcmp(node_container[n].wlan_code,node_container[m].wlan_code) == 0) {
				connect node_container[n].outportAskForTxModulation,node_container[m].InportMCSRequestReceived;
				connect node_container[n].outportAnswerTxModulation,node_container[m].InportMCSResponseReceived;
			}
		}
	}
};

/*
 * Start()
 */
void Komondor :: Start(){
	// do nothing
};

/*
 * Stop(): called when the simulation is done to  collect and display statistics.
 */
void Komondor :: Stop(){

	int total_packets_sent = 0;
	double total_throughput = 0;

	for(int m=0; m < total_nodes_number; m++){

		if( node_container[m].node_type == NODE_TYPE_AP){
			total_packets_sent += node_container[m].packets_sent;
			total_throughput += node_container[m].throughput;
		}
	}

	if (print_system_logs) {
		printf("\n%s General Statistics:\n", LOG_LVL1);
		printf("%s Total number of packets sent = %d\n", LOG_LVL2, total_packets_sent);
		printf("%s Total throughput = %.2f Mbps\n", LOG_LVL2, total_throughput * pow(10,-6));
		printf("%s Average number of packets sent per WLAN = %d\n", LOG_LVL2, (total_packets_sent/total_wlans_number));
		printf("%s Average throughput per WLAN = %.2f Mbps\n", LOG_LVL2, (total_throughput * pow(10,-6)/total_wlans_number));
		printf("\n\n");
	}

	if (save_system_logs) {

		// Simulation log file
		fprintf(logger_simulation.file, "%s STATISTICS:\n", LOG_LVL1);
		fprintf(logger_simulation.file, "%s Total number of packets sent = %d\n", LOG_LVL2, total_packets_sent);
		fprintf(logger_simulation.file, "%s Total number of packets sent = %d\n", LOG_LVL2, total_packets_sent);
		fprintf(logger_simulation.file, "%s Total throughput = %f\n", LOG_LVL2, total_throughput);
		fprintf(logger_simulation.file, "%s Average number of packets sent = %d\n", LOG_LVL2, (total_packets_sent/total_nodes_number));
		fprintf(logger_simulation.file, "%s Average throughput = %f\n", LOG_LVL2, (total_throughput/total_nodes_number));
		fprintf(logger_simulation.file, "\n");

		fclose(simulation_output_file);

		// Script file (for several simulations in one)
		fprintf(logger_script.file, "%s STATISTICS:\n", LOG_LVL1);
		fprintf(logger_script.file, "%s Total number of packets sent = %d\n", LOG_LVL2, total_packets_sent);
		double avg_throughput = 0;
		double min_trhoughput = 10000;
		double max_trhoughput = -1000;
		int transmitting_nodes = 0;
		for(int m=0; m < total_nodes_number; m++){
			fprintf(logger_script.file, "%s Node #%d (%s) Throughput = %f\n", LOG_LVL2, m,
					node_container[m].node_code, node_container[m].throughput);
			avg_throughput += node_container[m].throughput;
			if(node_container[m].node_type == NODE_TYPE_AP){
				transmitting_nodes ++;
			}
			if(node_container[m].throughput > max_trhoughput) max_trhoughput = node_container[m].throughput;
			if(node_container[m].throughput < min_trhoughput) min_trhoughput = node_container[m].throughput;
		}
		avg_throughput = avg_throughput/transmitting_nodes;
		fprintf(logger_script.file, "%s AVERAGE TPT = %f\n", LOG_LVL2, avg_throughput);
		fprintf(logger_script.file, "%s MIN VAL = %f\n", LOG_LVL2, min_trhoughput);
		fprintf(logger_script.file, "%s MAX VAL = %f\n", LOG_LVL2, max_trhoughput);
	}

	fclose(script_output_file);

	printf("%s SIMULATION '%s' FINISHED\n", LOG_LVL1, simulation_code);
	printf("------------------------------------------\n");
};

/*
 * InputChecker(): checks that input is set in proper format and values are acceptable
 */
void Komondor :: InputChecker(){

	// Auxiliary arrays
	int nodes_ids[total_nodes_number];
	double nodes_x[total_nodes_number];
	double nodes_y[total_nodes_number];
	double nodes_z[total_nodes_number];
	for(int i = 0; i<total_nodes_number;i++){
		nodes_ids[i] = 0;
		nodes_x[i] = 0;
		nodes_y[i] = 0;
		nodes_z[i] = 0;
	}

	if (print_system_logs) printf("%s Validating input files...\n", LOG_LVL2);

	for (int i = 0; i < total_nodes_number; i++) {

		nodes_ids[i] = node_container[i].node_id;
		nodes_x[i] = node_container[i].x;
		nodes_y[i] = node_container[i].y;
		nodes_z[i] = node_container[i].z;

		// TPC values (min <= defalut <= max)
		if (node_container[i].tpc_min > node_container[i].tpc_max
				|| node_container[i].tpc_default > node_container[i].tpc_max
				|| node_container[i].tpc_default < node_container[i].tpc_min) {
			printf("\nERROR: TPC values are not properly configured at node in line %d\n"
					"node_container[i].tpc_min = %f\n"
					"node_container[i].tpc_default = %f\n"
					"node_container[i].tpc_max = %f\n\n",
					i+2, node_container[i].tpc_min, node_container[i].tpc_default, node_container[i].tpc_max);
			exit(-1);
		}

		// CCA values (min <= defalut <= max)
		if (node_container[i].cca_min > node_container[i].cca_max
				|| node_container[i].cca_default > node_container[i].cca_max
				|| node_container[i].cca_default < node_container[i].cca_min) {
			printf("\nERROR: CCA values are not properly configured at node in line %d\n\n",i+2);
			exit(-1);
		}

		// Channel values (min <= primary <= max)
		if (node_container[i].primary_channel > node_container[i].max_channel_allowed
				|| node_container[i].primary_channel < node_container[i].min_channel_allowed
				|| node_container[i].min_channel_allowed > node_container[i].max_channel_allowed
				|| node_container[i].primary_channel > num_channels_komondor
				|| node_container[i].min_channel_allowed > num_channels_komondor
				|| node_container[i].max_channel_allowed > num_channels_komondor) {
			printf("\nERROR: Channels are not properly configured at node in line %d\n\n",i+2);
			exit(-1);
		}
	}

	for (int i = 0; i < total_nodes_number; i++) {
		for (int j = 0; j < total_nodes_number; j++) {

			// Node IDs must be different
			if(i!=j && nodes_ids[i] == nodes_ids[j] && i < j) {
				printf("\nERROR: Nodes in lines %d and %d have the same ID\n\n",i+2,j+2);
				exit(-1);
			}

			// Nodes position may be required to be different
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
void Komondor :: SetupEnvironmentByReadingInputFile(char *system_filename) {

	if (print_system_logs) printf("%s Reading system configuration file '%s'...\n", LOG_LVL1, system_filename);
	fprintf(simulation_output_file, "%s Reading system configuration file '%s'...\n", LOG_LVL2, system_filename);

	FILE* stream_system = fopen(system_filename, "r");
	if (!stream_system){
		printf("%s Komondor system file '%s' not found!\n", LOG_LVL3, system_filename);
		fprintf(simulation_output_file, "%s Komondor system file '%s' not found!\n", LOG_LVL3, system_filename);
		exit(-1);
	}

	char line_system[CHAR_BUFFER_SIZE];
	int first_line_skiped_flag = 0;	// Flag for skipping first informative line of input file

	while (fgets(line_system, CHAR_BUFFER_SIZE, stream_system)){

		if(!first_line_skiped_flag){

			first_line_skiped_flag = 1;

		} else {

			char* tmp = strdup(line_system);

			// Number of channels
			tmp = strdup(line_system);
			const char* num_channels_char = GetField(tmp, IX_NUM_CHANNELS);
			num_channels_komondor = atoi(num_channels_char);

			// Basic channel bandwidth
			tmp = strdup(line_system);
			const char* basic_channel_bandwidth_char = GetField(tmp, IX_BASIC_CH_BW);
			basic_channel_bandwidth = atoi(basic_channel_bandwidth_char);

			// Prob. distribution of backoff duration
			tmp = strdup(line_system);
			const char* pdf_backoff_char = GetField(tmp, IX_PDF_BACKOFF);
			pdf_backoff = atoi(pdf_backoff_char);

			// Prob. distribution of transmission duration
			tmp = strdup(line_system);
			const char* pdf_tx_time_char = GetField(tmp, IX_PDF_TX_TIME);
			pdf_tx_time = atoi(pdf_tx_time_char);

			// Data packet length
			tmp = strdup(line_system);
			const char* packet_length_char = GetField(tmp, IX_PACKET_LENGTH);
			packet_length = atoi(packet_length_char);

			// ACK packet length
			tmp = strdup(line_system);
			const char* ack_length_char = GetField(tmp, IX_ACK_LENGTH);
			ack_length = atoi(ack_length_char);

			// Number of packets aggregated in one transmission
			tmp = strdup(line_system);
			const char* num_packets_aggregated_char = GetField(tmp, IX_NUM_PACKETS_AGGREGATED);
			num_packets_aggregated = atoi(num_packets_aggregated_char);

			// Path loss model
			tmp = strdup(line_system);
			const char* path_loss_model_char = GetField(tmp, IX_PATH_LOSS);
			path_loss_model = atoi(path_loss_model_char);

			// capture_effect
			tmp = strdup(line_system);
			const char* capture_effect_char = GetField(tmp, IX_CAPTURE_EFFECT);
			double capture_effect_dbm = atof(capture_effect_char);
			capture_effect = ConvertPower(LINEAR_TO_DB, capture_effect_dbm);

			// Noise level
			tmp = strdup(line_system);
			const char* noise_level_char = GetField(tmp, IX_NOISE_LEVEL);
			double noise_level_dbm = atof(noise_level_char);
			noise_level = ConvertPower(DBM_TO_PW, noise_level_dbm);

			// Co-channel model
			tmp = strdup(line_system);
			const char* cochannel_model_char = GetField(tmp, IX_COCHANNEL_MODEL);
			cochannel_model = atof(cochannel_model_char);

			// Collisions model
			tmp = strdup(line_system);
			const char* collisions_model_char = GetField(tmp, IX_COLLISIONS_MODEL);
			collisions_model = atof(collisions_model_char);

			// SIFS
			tmp = strdup(line_system);
			const char* sifs_char = GetField(tmp, IX_SIFS);
			SIFS = atof(sifs_char) * pow(10,-6);
			DIFS = SIFS + (2 * SLOT_TIME);

			// Constant PER for successful transmissions
			tmp = strdup(line_system);
			const char* constant_PER_char = GetField(tmp, IX_CONSTANT_PER);
			constant_per = atof(constant_PER_char);

			// RTS packet length
			tmp = strdup(line_system);
			const char* rts_length_char = GetField(tmp, IX_RTS_LENGTH);
			rts_length = atoi(rts_length_char);

			// CTS packet length
			tmp = strdup(line_system);
			const char* cts_length_char = GetField(tmp, IX_CTS_LENGTH);
			cts_length = atoi(cts_length_char);

			// Traffic model
			tmp = strdup(line_system);
			const char* traffic_model_char = GetField(tmp, IX_TRAFFIC_MODEL);
			traffic_model = atoi(traffic_model_char);

			// Backoff type
			tmp = strdup(line_system);
			const char* backoff_type_char = GetField(tmp, IX_BO_TYPE);
			backoff_type = atoi(backoff_type_char);

			free(tmp);
		}
	}

	fclose(stream_system);
}

/* *******************
 * * NODE GENERATION *
 * *******************
 * Nodes can be generated in 2 ways:
 * - Deterministically: defining every single parameter of all the nodes in the input file.
 * - Pseudo-randomely: defining completely just the APs.
 * The filename of the nodes input will determine the way to process them.
 */

/*
 * GenerateNodes(): generates the nodes randomely if AP file is used, or deterministically if NODE file is used.
 * Input arguments:
 * - nodes_filename: AP or nodes filename
 */
void Komondor :: GenerateNodes(char *nodes_filename) {

	if (print_system_logs) printf("%s Generating nodes...\n", LOG_LVL1);
	fprintf(simulation_output_file, "%s Generating nodes...\n", LOG_LVL1);

	if(strstr(nodes_filename, FILE_NAME_CODE_APS) != NULL) {	// Generate random nodes according to APs input settings

		if (print_system_logs) printf("%s Generating nodes RANDOMLY through AP input file...\n", LOG_LVL2);
		if (save_system_logs) fprintf(simulation_output_file, "%s Generating nodes RANDOMLY through AP input file...\n", LOG_LVL2);
		GenerateNodesByReadingAPsInputFile(nodes_filename);

	} else if(strstr(nodes_filename, FILE_NAME_CODE_NODES) != NULL) {	// Generate nodes according to node input file

		if (print_system_logs) printf("%s Generating nodes DETERMINISTICALLY through NODES input file...\n", LOG_LVL2);
		if (save_system_logs) fprintf(simulation_output_file, "%s Generating nodes DETERMINISTICALLY...\n", LOG_LVL2);
		GenerateNodesByReadingNodesInputFile(nodes_filename);

	}
}

/*
 * GenerateNodesByReadingAPsInputFile(): generates the nodes RANDOMLY according to APs input file
 * Input arguments:
 * - nodes_filename: APs input file
 */
void Komondor :: GenerateNodesByReadingAPsInputFile(char *nodes_filename){

	total_wlans_number = GetNumOfLines(nodes_filename);
	wlan_container = (Wlan *) malloc(total_wlans_number * sizeof(*wlan_container));

	// Compute the number of STAs of each WLAN
	if (print_system_logs) printf("%s Computing the number of STAs in each WLAN...\n", LOG_LVL2);

	FILE* stream_nodes = fopen(nodes_filename, "r");
	char line_nodes[CHAR_BUFFER_SIZE];
	first_line_skiped_flag = 0;	// Flag for skipping first informative line of input file
	int wlan_ix = 0;	// Auxiliar wlan index

	while (fgets(line_nodes, CHAR_BUFFER_SIZE, stream_nodes)){

		if(!first_line_skiped_flag){

			first_line_skiped_flag = 1;

		} else{

			// WLAN ID
			wlan_container[wlan_ix].wlan_id = wlan_ix;

			// WLAN code
			char* tmp_nodes = strdup(line_nodes);
			const char *wlan_code_aux = GetField(tmp_nodes, IX_AP_WLAN_CODE);
			char *wlan_code = (char *) malloc(strlen(wlan_code_aux) + 1);
			sprintf(wlan_code, "%s", wlan_code_aux);
			wlan_container[wlan_ix].wlan_code = wlan_code;

			// Number of STAs in the WLAN
			tmp_nodes = strdup(line_nodes);
			int min_sta_number = atoi(GetField(tmp_nodes, IX_AP_MIN_NUM_OF_STAS));
			tmp_nodes = strdup(line_nodes);
			int max_sta_number = atoi(GetField(tmp_nodes, IX_AP_MAX_NUM_OF_STAS));
			wlan_container[wlan_ix].num_stas = rand()%(max_sta_number-min_sta_number + 1) + min_sta_number;

			total_nodes_number += (wlan_container[wlan_ix].num_stas + 1);
			wlan_ix++;
		}
	}

	// Generate Nodes
	node_container.SetSize(total_nodes_number);

	stream_nodes = fopen(nodes_filename, "r");
	int node_ix = 0;				// Auxiliar index for nodes
	wlan_ix = 0;					// Auxiliar index for WLANs
	first_line_skiped_flag = 0;		// Flag for skipping first informative line of input file
	int ap_generated;				// Flag indicating if AP is already generated
	int node_id_counter_in_wlan;	// Auxiliar counter for STAs in WLAN
	int node_ix_aux;				// Auxiliar index for nodes II

	while (fgets(line_nodes, CHAR_BUFFER_SIZE, stream_nodes)){	// For each WLAN

		if(!first_line_skiped_flag){

			first_line_skiped_flag = 1;

		} else {

			ap_generated = 0;
			wlan_container[wlan_ix].setSizeOfSTAsArray(wlan_container[wlan_ix].num_stas);

			// Max distance AP - STA
			char* tmp_nodes = strdup(line_nodes);
			int max_distance_sta = atoi(GetField(tmp_nodes, IX_AP_MAX_DISTANCE_AP_STA));

			// AP position
			tmp_nodes = strdup(line_nodes);
			double x = atof(GetField(tmp_nodes, IX_AP_POSITION_X));
			tmp_nodes = strdup(line_nodes);
			double y = atof(GetField(tmp_nodes, IX_AP_POSITION_Y));
			tmp_nodes = strdup(line_nodes);
			double z = atof(GetField(tmp_nodes, IX_AP_POSITION_Z));

			// Min CW
			tmp_nodes = strdup(line_nodes);
			int min_cw = atoi(GetField(tmp_nodes, IX_AP_CW_MIN));

			// Max CW
			tmp_nodes = strdup(line_nodes);
			int max_cw = atoi(GetField(tmp_nodes, IX_AP_CW_MAX));

			// Primary channel
			tmp_nodes = strdup(line_nodes);
			int primary_channel = atoi(GetField(tmp_nodes, IX_AP_PRIMARY_CHANNEL));

			// Min channel allowed
			tmp_nodes = strdup(line_nodes);
			int min_channel_allowed = atoi(GetField(tmp_nodes, IX_AP_MIN_CH_ALLOWED));

			// Max channel allowed
			tmp_nodes = strdup(line_nodes);
			int max_channel_allowed = atoi(GetField(tmp_nodes, IX_AP_MAX_CH_ALLOWED));

			// Min TPC
			tmp_nodes = strdup(line_nodes);
			double tpc_min_dbm = atof(GetField(tmp_nodes, IX_AP_TPC_MIN));
			double tpc_min = ConvertPower(DBM_TO_PW, tpc_min_dbm);

			// Default TPC
			tmp_nodes = strdup(line_nodes);
			double tpc_default_dbm = atof(GetField(tmp_nodes, IX_AP_TPC_DEFAULT));
			double tpc_default = ConvertPower(DBM_TO_PW, tpc_default_dbm);

			// Max TPC
			tmp_nodes = strdup(line_nodes);
			double tpc_max_dbm = atof(GetField(tmp_nodes, IX_AP_TPC_MAX));
			double tpc_max = ConvertPower(DBM_TO_PW, tpc_max_dbm);

			// Min CCA
			tmp_nodes = strdup(line_nodes);
			double cca_min_dbm = atoi(GetField(tmp_nodes, IX_AP_CCA_MIN));
			double cca_min = ConvertPower(DBM_TO_PW, cca_min_dbm);

			// Default CCA
			tmp_nodes = strdup(line_nodes);
			double cca_default_dbm = atoi(GetField(tmp_nodes, IX_AP_CCA_DEFAULT));
			double cca_default = ConvertPower(DBM_TO_PW, cca_default_dbm);

			// Max CCA
			tmp_nodes = strdup(line_nodes);
			double cca_max_dbm = atoi(GetField(tmp_nodes, IX_AP_CCA_MAX));
			double cca_max = ConvertPower(DBM_TO_PW, cca_max_dbm);

			// TX gain
			tmp_nodes = strdup(line_nodes);
			double tx_gain_db = atoi(GetField(tmp_nodes, IX_AP_TX_GAIN));
			double tx_gain = ConvertPower(DB_TO_LINEAR, tx_gain_db);

			// RX gain
			tmp_nodes = strdup(line_nodes);
			double rx_gain_db = atoi(GetField(tmp_nodes, IX_AP_RX_GAIN));
			double rx_gain = ConvertPower(DB_TO_LINEAR, rx_gain_db);

			// Channel bonding model
			tmp_nodes = strdup(line_nodes);
			int channel_bonding_model = atoi(GetField(tmp_nodes, IX_AP_CHANNEL_BONDING_MODEL));

			// Default modulation
			tmp_nodes = strdup(line_nodes);
			double modulation_default = atoi(GetField(tmp_nodes, IX_AP_MODULATION_DEFAULT));

			// Central frequency in GHz (e.g. 2.4)
			tmp_nodes = strdup(line_nodes);
			const char* central_frequency_char = GetField(tmp_nodes, IX_AP_CENTRAL_FREQ);
			double central_frequency = atof(central_frequency_char) * pow(10,9);

			// Lambda (packet generation rate)
			tmp_nodes = strdup(line_nodes);
			const char* lambda_char = GetField(tmp_nodes, IX_AP_LAMBDA);
			double lambda = atof(lambda_char);

			node_id_counter_in_wlan = 0;

			node_ix_aux = node_ix;

			while(node_ix < node_ix_aux + (wlan_container[wlan_ix].num_stas + 1)){

				// Node ID
				node_container[node_ix].node_id = node_ix;

				// Node type, node code, and position
				if(!ap_generated){
					node_container[node_ix].node_type = NODE_TYPE_AP;
					wlan_container[wlan_ix].ap_id = node_ix;
					char *node_code = (char *) malloc(strlen(wlan_container[wlan_ix].wlan_code) + INTEGER_SIZE + 1);
					sprintf(node_code, "%s_%s_%d", wlan_container[wlan_ix].wlan_code, "AP", node_ix);
					node_container[node_ix].node_code = node_code;
					node_container[node_ix].x = x;
					node_container[node_ix].y = y;
					node_container[node_ix].z = z;
					ap_generated = 1;

				} else {

					wlan_container[wlan_ix].list_sta_id[node_id_counter_in_wlan] = node_ix;	// Add node ID to WLAN
					node_id_counter_in_wlan ++;
					node_container[node_ix].node_type = NODE_TYPE_STA;
					char *node_code = (char *) malloc(strlen(wlan_container[wlan_ix].wlan_code + 1) + INTEGER_SIZE + 1);
					sprintf(node_code, "%s_%s_%d", wlan_container[wlan_ix].wlan_code, "STA", node_ix);
					node_container[node_ix].node_code = node_code;

					node_container[node_ix].x = RandomDouble(x-max_distance_sta, x + max_distance_sta);
					node_container[node_ix].y = RandomDouble(y-max_distance_sta, y + max_distance_sta);
					node_container[node_ix].z = RandomDouble(z-max_distance_sta, z + max_distance_sta);

				}

				node_container[node_ix].wlan_code = wlan_container[wlan_ix].wlan_code;
				node_container[node_ix].destination_id = NODE_ID_NONE;
				node_container[node_ix].min_cw = min_cw;
				node_container[node_ix].max_cw = max_cw;
				node_container[node_ix].primary_channel = primary_channel;
				node_container[node_ix].min_channel_allowed = min_channel_allowed;
				node_container[node_ix].max_channel_allowed = max_channel_allowed;
				node_container[node_ix].tpc_min = tpc_min;
				node_container[node_ix].tpc_default = tpc_default;
				node_container[node_ix].tpc_max = tpc_max;
				node_container[node_ix].cca_min = cca_min;
				node_container[node_ix].cca_default = cca_default;
				node_container[node_ix].cca_max = cca_max;
				node_container[node_ix].tx_gain = tx_gain;
				node_container[node_ix].rx_gain = rx_gain;
				node_container[node_ix].channel_bonding_model = channel_bonding_model;
				node_container[node_ix].modulation_default = modulation_default;

				// System
				node_container[node_ix].simulation_time_komondor = simulation_time_komondor;
				node_container[node_ix].total_nodes_number = total_nodes_number;
				node_container[node_ix].collisions_model = collisions_model;
				node_container[node_ix].capture_effect = capture_effect;
				node_container[node_ix].save_node_logs = save_node_logs;
				node_container[node_ix].print_node_logs = print_node_logs;
				node_container[node_ix].basic_channel_bandwidth = basic_channel_bandwidth;
				node_container[node_ix].num_channels_komondor = num_channels_komondor;
				node_container[node_ix].cochannel_model = cochannel_model;
				node_container[node_ix].default_destination_id = NODE_ID_NONE;
				node_container[node_ix].noise_level = noise_level;
				node_container[node_ix].SIFS = SIFS;
				node_container[node_ix].DIFS = DIFS;
				node_container[node_ix].constant_per = constant_per;
				node_container[node_ix].central_frequency = central_frequency;
				node_container[node_ix].pdf_backoff = pdf_backoff;
				node_container[node_ix].path_loss_model = path_loss_model;
				node_container[node_ix].pdf_tx_time = pdf_tx_time;
				node_container[node_ix].packet_length = packet_length;
				node_container[node_ix].num_packets_aggregated = num_packets_aggregated;
				node_container[node_ix].ack_length = ack_length;
				node_container[node_ix].rts_length = rts_length;
				node_container[node_ix].cts_length = cts_length;
				node_container[node_ix].traffic_model = traffic_model;
				node_container[node_ix].backoff_type = backoff_type;
				node_container[node_ix].lambda = lambda;
				node_container[node_ix].simulation_code = simulation_code;

				node_ix++;
			}

			node_id_counter_in_wlan = wlan_container[wlan_ix].ap_id;

			while(node_id_counter_in_wlan <= wlan_container[wlan_ix].ap_id + wlan_container[wlan_ix].num_stas){
				node_container[node_id_counter_in_wlan].wlan = wlan_container[wlan_ix];
				node_id_counter_in_wlan++;
			}

			wlan_ix ++;
			free(tmp_nodes);
		}
	}
}


/*
 * GenerateNodesByReadingNodesInputFile(): generates the nodes according to a nodes input file (deterministic)
 * Input arguments:
 * - nodes_filename: nodes input filename
 */
void Komondor :: GenerateNodesByReadingNodesInputFile(char *nodes_filename){

	if (print_system_logs) printf("%s Computing the number of STAs in each WLAN...\n", LOG_LVL2);
	total_wlans_number = GetNumOfNodes(nodes_filename, NODE_TYPE_AP, NULL);
	wlan_container = (Wlan *) malloc(total_wlans_number * sizeof(*wlan_container));

	FILE* stream_nodes = fopen(nodes_filename, "r");
	char line_nodes[CHAR_BUFFER_SIZE];
	first_line_skiped_flag = 0;	// Flag for skipping first informative line of input file
	int wlan_ix = 0;			// Auxiliar wlan index

	// Identify WLANs
	while (fgets(line_nodes, CHAR_BUFFER_SIZE, stream_nodes)){

		if(!first_line_skiped_flag){

			first_line_skiped_flag = 1;

		} else{

			// Node type
			char* tmp_nodes = strdup(line_nodes);
			int node_type = atoi(GetField(tmp_nodes, IX_NODE_TYPE));

			if(node_type == NODE_TYPE_AP){	// If node is AP

				// WLAN ID
				wlan_container[wlan_ix].wlan_id = wlan_ix;

				// WLAN code
				tmp_nodes = strdup(line_nodes);
				const char *wlan_code_aux = GetField(tmp_nodes, IX_WLAN_CODE);
				char *wlan_code = (char *) malloc(strlen(wlan_code_aux) + 1);
				sprintf(wlan_code, "%s", wlan_code_aux);
				wlan_container[wlan_ix].wlan_code = wlan_code;

				wlan_ix++;
				free(tmp_nodes);
			}
		}
	}

	// Get number of STAs in each WLAN
	for(int w = 0; w < total_wlans_number; w++){

		int num_stas_in_wlan = GetNumOfNodes(nodes_filename, NODE_TYPE_STA, wlan_container[w].wlan_code);
		wlan_container[w].num_stas = num_stas_in_wlan;
		wlan_container[w].setSizeOfSTAsArray(num_stas_in_wlan);
	}


	// Generate nodes (without wlan item), finsih WLAN with ID lists, and set the wlan item of each STA.
	total_nodes_number = GetNumOfNodes(nodes_filename, NODE_TYPE_UNKWNOW, NULL);
	node_container.SetSize(total_nodes_number);

	stream_nodes = fopen(nodes_filename, "r");
	int node_ix = 0;	// Auxiliar index for nodes
	wlan_ix = 0;		// Auxiliar index for WLANs
	first_line_skiped_flag = 0;

	while (fgets(line_nodes, CHAR_BUFFER_SIZE, stream_nodes)){	// For each WLAN

		if(!first_line_skiped_flag){

			first_line_skiped_flag = 1;

		} else {

			// Node ID (auto-assigned)
			node_container[node_ix].node_id = node_ix;

			// Node code
			char* tmp_nodes = strdup(line_nodes);
			const char *node_code_aux = GetField(tmp_nodes, IX_NODE_CODE);
			char *node_code = (char *) malloc(strlen(node_code_aux) + 1);
			sprintf(node_code, "%s", node_code_aux);
			node_container[node_ix].node_code = node_code;

			// Node type
			tmp_nodes = strdup(line_nodes);
			int node_type = atoi(GetField(tmp_nodes, IX_NODE_TYPE));
			node_container[node_ix].node_type = node_type;

			// WLAN code: add AP or STA ID to corresponding WLAN
			tmp_nodes = strdup(line_nodes);
			const char *wlan_code_aux = GetField(tmp_nodes, IX_WLAN_CODE);
			char *wlan_code = (char *) malloc(strlen(wlan_code_aux) + 1);
			sprintf(wlan_code, "%s", wlan_code_aux);
			node_container[node_ix].wlan_code = wlan_code;
			for(int w = 0; w < total_wlans_number; w++){
				if(strcmp(wlan_code, wlan_container[w].wlan_code) == 0){	// If nodes belong to WLAN
					if(node_container[node_ix].node_type == NODE_TYPE_AP){	// If node is AP
						wlan_container[w].ap_id = node_container[node_ix].node_id;
					} else if (node_container[node_ix].node_type == NODE_TYPE_STA){	// If node is STA
						for(int s = 0; s < wlan_container[w].num_stas; s++){
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
			node_container[node_ix].min_cw = atoi(GetField(tmp_nodes, IX_CW_MIN));

			// CW max
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].max_cw = atoi(GetField(tmp_nodes, IX_CW_MAX));

			// Primary channel
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].primary_channel = atoi(GetField(tmp_nodes, IX_PRIMARY_CHANNEL));

			// Min channel allowed
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].min_channel_allowed = atoi(GetField(tmp_nodes, IX_MIN_CH_ALLOWED));

			// Max channel allowed
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].max_channel_allowed = atoi(GetField(tmp_nodes, IX_MAX_CH_ALLOWED));

			// Min TPC
			tmp_nodes = strdup(line_nodes);
			double tpc_min_dbm = atof(GetField(tmp_nodes, IX_TPC_MIN));
			node_container[node_ix].tpc_min = ConvertPower(DBM_TO_PW, tpc_min_dbm);

			// Default TPC
			tmp_nodes = strdup(line_nodes);
			double tpc_default_dbm = atof(GetField(tmp_nodes, IX_TPC_DEFAULT));
			node_container[node_ix].tpc_default = ConvertPower(DBM_TO_PW, tpc_default_dbm);

			// Max TPC
			tmp_nodes = strdup(line_nodes);
			double tpc_max_dbm = atof(GetField(tmp_nodes, IX_TPC_MAX));
			node_container[node_ix].tpc_max = ConvertPower(DBM_TO_PW, tpc_max_dbm);

			// Min CCA
			tmp_nodes = strdup(line_nodes);
			double cca_min_dbm = atoi(GetField(tmp_nodes, IX_CCA_MIN));
			node_container[node_ix].cca_min = ConvertPower(DBM_TO_PW, cca_min_dbm);

			// Default CCA
			tmp_nodes = strdup(line_nodes);
			double cca_default_dbm = atoi(GetField(tmp_nodes, IX_CCA_DEFAULT));
			node_container[node_ix].cca_default = ConvertPower(DBM_TO_PW, cca_default_dbm);

			// Max CCA
			tmp_nodes = strdup(line_nodes);
			double cca_max_dbm = atoi(GetField(tmp_nodes, IX_CCA_MAX));
			node_container[node_ix].cca_max = ConvertPower(DBM_TO_PW, cca_max_dbm);

			// TX gain
			tmp_nodes = strdup(line_nodes);
			double tx_gain_db = atoi(GetField(tmp_nodes, IX_TX_GAIN));
			node_container[node_ix].tx_gain = ConvertPower(DB_TO_LINEAR, tx_gain_db);

			// RX gain
			tmp_nodes = strdup(line_nodes);
			double rx_gain_db = atoi(GetField(tmp_nodes, IX_RX_GAIN));
			node_container[node_ix].rx_gain = ConvertPower(DB_TO_LINEAR, rx_gain_db);

			// Channel bonding model
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].channel_bonding_model = atoi(GetField(tmp_nodes, IX_CHANNEL_BONDING_MODEL));

			// Default modulation
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].modulation_default = atoi(GetField(tmp_nodes, IX_MODULATION_DEFAULT));

			// Central frequency in GHz (e.g. 2.4)
			tmp_nodes = strdup(line_nodes);
			const char* central_frequency_char = GetField(tmp_nodes, IX_CENTRAL_FREQ);
			node_container[node_ix].central_frequency = atof(central_frequency_char) * pow(10,9);

			// Lambda (packet generation rate)
			tmp_nodes = strdup(line_nodes);
			const char* lambda_char = GetField(tmp_nodes, IX_LAMBDA);
			node_container[node_ix].lambda = atof(lambda_char);

			// System
			node_container[node_ix].simulation_time_komondor = simulation_time_komondor;
			node_container[node_ix].total_nodes_number = total_nodes_number;
			node_container[node_ix].collisions_model = collisions_model;
			node_container[node_ix].capture_effect = capture_effect;
			node_container[node_ix].save_node_logs = save_node_logs;
			node_container[node_ix].print_node_logs = print_node_logs;
			node_container[node_ix].basic_channel_bandwidth = basic_channel_bandwidth;
			node_container[node_ix].num_channels_komondor = num_channels_komondor;
			node_container[node_ix].cochannel_model = cochannel_model;
			node_container[node_ix].default_destination_id = NODE_ID_NONE;
			node_container[node_ix].noise_level = noise_level;
			node_container[node_ix].SIFS = SIFS;
			node_container[node_ix].DIFS = DIFS;
			node_container[node_ix].constant_per = constant_per;
			node_container[node_ix].pdf_backoff = pdf_backoff;
			node_container[node_ix].path_loss_model = path_loss_model;
			node_container[node_ix].pdf_tx_time = pdf_tx_time;
			node_container[node_ix].packet_length = packet_length;
			node_container[node_ix].num_packets_aggregated = num_packets_aggregated;
			node_container[node_ix].ack_length = ack_length;
			node_container[node_ix].rts_length = rts_length;
			node_container[node_ix].cts_length = cts_length;
			node_container[node_ix].traffic_model = traffic_model;
			node_container[node_ix].backoff_type = backoff_type;
			node_container[node_ix].simulation_code = simulation_code;

			node_ix ++;
			free(tmp_nodes);
		}
	}

	// Set corresponding WLAN to each node
	for(int n = 0; n < total_nodes_number; n++){
		for(int w = 0; w < total_wlans_number; w++){
			if (strcmp(node_container[n].wlan_code, wlan_container[w].wlan_code) == 0) {
				node_container[n].wlan = wlan_container[w];
			}
		}
	}
}

/***************************/
/* LOG AND DEBUG FUNCTIONS */
/***************************/

/*
 * printSystemInfo(): prints the Komondor environment general info
 */
void Komondor :: printSystemInfo(){

	if (print_system_logs){
		printf("%s total_nodes_number = %d\n", LOG_LVL2, total_nodes_number);
		printf("%s num_channels_komondor = %d\n", LOG_LVL3, num_channels_komondor);
		printf("%s basic_channel_bandwidth = %f MHz\n", LOG_LVL3, basic_channel_bandwidth);
		printf("%s pdf_backoff = %d\n", LOG_LVL3, pdf_backoff);
		printf("%s pdf_tx_time = %d\n", LOG_LVL3, pdf_tx_time);
		printf("%s packet_length = %d bits\n", LOG_LVL3, packet_length);
		printf("%s ack_length = %d bits\n", LOG_LVL3, ack_length);
		printf("%s cts_length = %d bits\n", LOG_LVL3, cts_length);
		printf("%s rts_length = %d bits\n", LOG_LVL3, rts_length);
		printf("%s traffic_model = %d\n", LOG_LVL3, traffic_model);
		printf("%s backoff_type = %d\n", LOG_LVL3, backoff_type);
		printf("%s num_packets_aggregated = %d\n", LOG_LVL3, num_packets_aggregated);
		printf("%s path_loss_model = %d\n", LOG_LVL3, path_loss_model);
		printf("%s capture_effect = %f [linear] (%f dB)\n", LOG_LVL3, capture_effect, ConvertPower(LINEAR_TO_DB, capture_effect));
		printf("%s noise_level = %f pW (%f dBm)\n",
				LOG_LVL3, noise_level, ConvertPower(PW_TO_DBM, noise_level));
		printf("%s cochannel_model = %d\n", LOG_LVL3, cochannel_model);
		printf("%s collisions_model = %d\n", LOG_LVL3, collisions_model);
		printf("%s SIFS = %f s\n", LOG_LVL3, SIFS);
		printf("%s DIFS = %f s\n", LOG_LVL3, DIFS);
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
	fprintf(logger.file, "%s total_nodes_number = %d\n", LOG_LVL3, total_nodes_number);
	fprintf(logger.file, "%s num_channels_komondor = %d\n", LOG_LVL3, num_channels_komondor);
	fprintf(logger.file, "%s basic_channel_bandwidth = %f\n", LOG_LVL3, basic_channel_bandwidth);
	fprintf(logger.file, "%s pdf_backoff = %d\n", LOG_LVL3, pdf_backoff);
	fprintf(logger.file, "%s pdf_tx_time = %d\n", LOG_LVL3, pdf_tx_time);
	fprintf(logger.file, "%s packet_length = %d bits\n", LOG_LVL3, packet_length);
	fprintf(logger.file, "%s ack_length = %d bits\n", LOG_LVL3, ack_length);
	fprintf(logger.file, "%s num_packets_aggregated = %d\n", LOG_LVL3, num_packets_aggregated);
	fprintf(logger.file, "%s path_loss_model = %d\n", LOG_LVL3, path_loss_model);
	fprintf(logger.file, "%s capture_effect = %f\n", LOG_LVL3, capture_effect);
	fprintf(logger.file, "%s noise_level = %f dBm\n", LOG_LVL3, noise_level);
	fprintf(logger.file, "%s cochannel_model = %d\n", LOG_LVL3, cochannel_model);
	fprintf(logger.file, "%s collisions_model = %d\n", LOG_LVL3, collisions_model);
	fprintf(logger.file, "%s SIFS = %f s\n", LOG_LVL3, SIFS);
	fprintf(logger.file, "%s DIFS = %f s\n", LOG_LVL3, DIFS);
}

/*
 * PrintAllNodesInfo(): prints the nodes info
 * Input arguments:
 * - info_detail_level: level of detail of the written logs
 */
void Komondor :: PrintAllNodesInfo(int info_detail_level){

	for(int n = 0; n < total_nodes_number; n++){
		node_container[n].PrintNodeInfo(info_detail_level);
	}
}

/*
 * PrintAllWlansInfo(): prints the WLANS info
 */
void Komondor :: PrintAllWlansInfo(){

	for(int w = 0; w < total_wlans_number; w++){
		wlan_container[w].printWlanInfo();
	}
}

/*
 * WriteAllNodesInfo(): writes the WLANs info in a file
 * Input arguments:
 * - logger: logger containing the file to write on
 */
void Komondor :: WriteAllWlansInfo(Logger logger, char *header_string){

	for(int w = 0; w < total_wlans_number; w++){
		wlan_container[w].writeWlanInfo(logger, header_string);
	}
}

/*
 * WriteAllNodesInfo(): writes the nodes info in a file
 * Input arguments:
 * - logger: logger containing the file to write on
 * - info_detail_level: level of detail of the written logs
 */
void Komondor :: WriteAllNodesInfo(Logger logger, int info_detail_level, char *header_string){

	for(int n = 0; n < total_nodes_number; n++){
		node_container[n].WriteNodeInfo(logger, info_detail_level, header_string);
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
    for (tok = strtok(line, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n"))
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
int Komondor :: GetNumOfLines(char *filename){
	int num_lines = 0;
	// Nodes file
	FILE* stream = fopen(filename, "r");
	if (!stream){
		printf("Nodes configuration file %s not found!\n", filename);
		exit(-1);
	}
	char line[CHAR_BUFFER_SIZE];
	while (fgets(line, CHAR_BUFFER_SIZE, stream))
	{
		num_lines++;
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
int Komondor :: GetNumOfNodes(char *nodes_filename, int node_type, char *wlan_code){

	int num_nodes = 0;
	char line_nodes[CHAR_BUFFER_SIZE];
	first_line_skiped_flag = 0;
	int type_found;
	char *wlan_code_found;

	FILE* stream_nodes = fopen(nodes_filename, "r");

	if (!stream_nodes){
		printf("Nodes configuration file %s not found!\n", nodes_filename);
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
				char* tmp_nodes = strdup(line_nodes);
				type_found = atof(GetField(tmp_nodes, IX_NODE_TYPE));

				// WLAN code
				tmp_nodes = strdup(line_nodes);
				const char *wlan_code_aux = GetField(tmp_nodes, IX_WLAN_CODE);
				wlan_code_found = (char *) malloc(strlen(wlan_code_aux) + 1);
				sprintf(wlan_code_found, "%s", wlan_code_aux);

				if(wlan_code != NULL){
					if(type_found == node_type && strcmp(wlan_code_found, wlan_code) == 0) num_nodes++;
				} else {
					if(type_found == node_type) num_nodes++;
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
	printf("%s KOMONDOR wireless network simulator\n", LOG_LVL1);
	printf("%s Copyright (C) 2017-2022, and GNU GPL'd, by Sergio Barrachina & Francisco Wilhelmi.\n", LOG_LVL1);
	printf("%s GitHub repository: https://github.com/wn-upf/Komondor\n", LOG_LVL2);
	printf("*************************************************************************************\n");
	printf("\n\n");

	// Input variables
	char *system_input_filename;
	char *nodes_input_filename;
	char *script_output_filename;
	char *simulation_code;
	int save_system_logs;
	int save_node_logs;
	int print_system_logs;
	int print_node_logs;
	double sim_time;
	int seed;

	// Get input variables per console
	if(argc == NUM_FULL_ARGUMENTS_CONSOLE){	// Full configuration entered per console

		system_input_filename = argv[1];
		nodes_input_filename = argv[2];
		script_output_filename = argv[3];
		simulation_code = argv[4];
		save_system_logs = atoi(argv[5]);
		save_node_logs = atoi(argv[6]);
		print_system_logs = atoi(argv[7]);
		print_node_logs = atoi(argv[8]);
		sim_time = atof(argv[9]);
		seed = atoi(argv[10]);

		if (print_system_logs) printf("%s FULL configuration entered per console.\n", LOG_LVL1);

	} else if(argc == NUM_PARTIAL_ARGUMENTS_CONSOLE) {	// Partial configuration entered per console

		system_input_filename = argv[1];
		nodes_input_filename = argv[2];
		sim_time = atof(argv[3]);
		seed = atoi(argv[4]);

		// Default values
		script_output_filename = (char *) malloc(strlen(DEFAULT_SCRIPT_FILENAME) + 1);
		sprintf(script_output_filename, "%s", DEFAULT_SCRIPT_FILENAME);

		simulation_code = (char *) malloc(strlen(DEFAULT_SIMULATION_CODE) + 1);
		sprintf(simulation_code, "%s", DEFAULT_SIMULATION_CODE);

		save_system_logs = DEFAULT_WRITE_SYSTEM_LOGS;
		save_node_logs = DEFAULT_WRITE_NODE_LOGS;
		print_system_logs = DEFAULT_PRINT_SYSTEM_LOGS;
		print_node_logs = DEFAULT_PRINT_NODE_LOGS;

		if (print_system_logs) printf("%s PARTIAL configuration entered per console. "
				"Some parameters are set by DEFAULT.\n", LOG_LVL1);

	} else if(argc == NUM_PARTIAL_ARGUMENTS_SCRIPT) {	// Partial configuration entered per console (useful for scripts)

		system_input_filename = argv[1];
		nodes_input_filename = argv[2];
		simulation_code = argv[3];	// For scripts --> usefult to identify simulations
		sim_time = atof(argv[4]);
		seed = atoi(argv[5]);

		// Default values
		script_output_filename = (char *) malloc(strlen(DEFAULT_SCRIPT_FILENAME) + 1);
		sprintf(script_output_filename, "%s", DEFAULT_SCRIPT_FILENAME);
		save_system_logs = DEFAULT_WRITE_SYSTEM_LOGS;
		save_node_logs = DEFAULT_WRITE_NODE_LOGS;
		print_system_logs = DEFAULT_PRINT_SYSTEM_LOGS;
		print_node_logs = DEFAULT_PRINT_NODE_LOGS;

		if (print_system_logs) printf("%s PARTIAL configuration entered per script. "
				"Some parameters are set by DEFAULT.\n", LOG_LVL1);

	} else {

		printf("%sERROR: Console arguments where not set properly!\n "
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
		printf("%s script_output_filename: %s\n", LOG_LVL2, script_output_filename);
		printf("%s simulation_code: %s\n", LOG_LVL2, simulation_code);
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
	test.StopTime(sim_time);
	test.Setup(sim_time, save_system_logs, save_node_logs, print_system_logs, print_node_logs,
			system_input_filename, nodes_input_filename, script_output_filename, simulation_code);

	printf("------------------------------------------\n");
	printf("%s SIMULATION '%s' STARTED\n", LOG_LVL1, simulation_code);

	test.Run();

	return(0);
};
