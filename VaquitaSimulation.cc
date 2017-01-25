#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "./COST/cost.h"

#include "Node.h"
#include "AuxStructures.h"

#define SLOT 0.000009

component VaquitaEnvironment : public CostSimEng
{
	public:
		void Setup(double sim_time, int save_node_logs, char *system_filename, char *nodes_filename, char *script_filename);
		void Stop();
		void Start();
		void inputChecker();
		void setupEnvironmentByReadingInputFile(char *system_filename);
		void generateNodesByReadingInputFile(char *nodes_filename, double sim_time);
		void readNodesFile(char *nodes_filename, double sim_time);

	public:
		Node[] node_container;

		double mu;					// Packet departure rate [1/s]
		double wavelength;			// Signal wavelength [m] (in WiFi 0.1249 m)
		int num_channels_vaquita;	// Number of subchannels composing the whole channel
		int basic_channel_bandwidth;	// Bandwidth of a basic channel [Mbps]
		int CW;						// Backoff contention window
		int pdf_backoff;			// Probability distribution type of the backoff (0: exponential, 1: deterministic)
		int pdf_tx_time;			// Probability distribution type of the transmission time (0: exponential, 1: deterministic)
		int packet_length;				// Packet length [bits]
		int path_loss_model;		// Path loss model (0: free-space, 1: Okumura-Hata model - Uban areas)
		int num_packets_aggregated;	// Number of packets aggregated in one transmission
		double noise_level;			// Environment noise [dBm]
		int cochannel_model;		// Co-channel interference model
		int collisions_model;		// Collisions model
		int logs_flag;				// NOT IMPLEMENTED YET
		int save_node_logs;			// Flag for activating the log writting of nodes

	private:
		// Parameters
		int total_nodes_number;
		FILE *logs_output_file;		// File for the output logs (including statistics)
		FILE *script_file;			// File for the output logs (including statistics)
};

void VaquitaEnvironment :: Setup(double sim_time, int save_node_logs_console, char *system_filename, char *nodes_filename, char *script_filename){
	printf("-------------- Setup() --------------\n");

	save_node_logs = save_node_logs_console;

	remove("/output/logs_output.txt");
	logs_output_file = fopen("./output/logs_output.txt","at");

	script_file = fopen(script_filename,"at");
	fprintf(script_file, "------------------------------------\n");

	printf("Reading input configuration files...\n");

	// Read system (environment) file
	setupEnvironmentByReadingInputFile(system_filename);

	// Read nodes file
	generateNodesByReadingInputFile(nodes_filename, sim_time);

	// Check input
	inputChecker();

	printf(" ----------------------------- \n");
	printf("Configuration ready!\n");

	printf("total_nodes_number = %d\n", total_nodes_number);
	// Set connections
	for(int n = 0; n < total_nodes_number; n++){
		for(int m=0; m < total_nodes_number; m++) {
			connect node_container[n].outportSelfStartTX,node_container[m].inportSomeNodeStartTX;
			connect node_container[n].outportSelfFinishTX,node_container[m].inportSomeNodeFinishTX;
			connect node_container[n].outportSendNack,node_container[m].inportNackReceived;
		}
	}
};

void VaquitaEnvironment :: Start(){
	printf("--------------  MAIN Start() --------------\n");
	fprintf(logs_output_file,"[MAIN] - SIMULATION STARTED!\n");
};

/*********************/
/*********************/
/* OUTPUT GENERATION */
/*********************/
/*********************/

void VaquitaEnvironment :: Stop(){
	printf("-------------- MAIN Stop() --------------\n");

	fclose(logs_output_file);

	int total_packets_sent = 0;
	double total_throughput = 0;
	for(int m=0;m<total_nodes_number;m++){
		total_packets_sent += node_container[m].packets_sent;
		total_throughput += node_container[m].throughput;
	}
	// printf("[STATS] No one TXing share of time TX = %f\n", ((SimTime()-total_time_transmitting)/SimTime()));
	printf("[STATS] Total number of packets sent = %d\n", total_packets_sent);
	printf("[STATS] Total throughput = %f\n", total_throughput);
	printf("[STATS] Average number of packets sent = %d\n", (total_packets_sent/total_nodes_number));
	printf("[STATS] Average throughput = %f\n", (total_throughput/total_nodes_number));
	printf("\n\n");

	// Script info
	fprintf(script_file, "- Total number of packets sent = %d\n", total_packets_sent);

	fclose(script_file);
};

/*
 * inputChecker()
 */
void VaquitaEnvironment :: inputChecker(){
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

	for (int i = 0; i < total_nodes_number; i++) {
		nodes_ids[i] = node_container[i].node_id;
		nodes_x[i] = node_container[i].x;
		nodes_y[i] = node_container[i].y;
		nodes_z[i] = node_container[i].z;

		// Check that channels selected make sense
		if (node_container[i].primary_channel > node_container[i].max_channel_allowed
				|| node_container[i].primary_channel < node_container[i].min_channel_allowed
				|| node_container[i].min_channel_allowed > node_container[i].max_channel_allowed
				|| node_container[i].primary_channel > num_channels_vaquita
				|| node_container[i].min_channel_allowed > num_channels_vaquita
				|| node_container[i].max_channel_allowed > num_channels_vaquita) {
			printf("\nERROR: Channels are not properly configured at node in line %d\n",i+2);
			printf("   · primary_channel = %d, range: %d - %d  \n",
					node_container[i].primary_channel, node_container[i].min_channel_allowed, node_container[i].max_channel_allowed);
			exit(-1);
		}
	}

	for (int i = 0; i < total_nodes_number; i++) {
		for (int j = 0; j < total_nodes_number; j++) {
			// Check that all nodes have different ids
			if(i!=j && nodes_ids[i] == nodes_ids[j] && i < j) {
				printf("\nERROR: Nodes in lines %d and %d have the same ID\n\n",i+2,j+2);
				exit(-1);
			}
			// Check that nodes positions make sense
			if(i!=j && nodes_x[i] == nodes_x[j] && nodes_y[i] == nodes_y[j] && nodes_z[i] == nodes_z[j] && i < j) {
				printf("\nWARNING: Nodes in lines %d and %d are exactly at the same position\n\n",i+2,j+2);
			}
		}
	}
}

/*
 * setupEnvironmentByReadingInputFile()
 * Input arguments:
 * - system_filename: ...
 */
void VaquitaEnvironment :: setupEnvironmentByReadingInputFile(char *system_filename) {
	// printf("Reading system configuration...\n");
	fprintf(logs_output_file, "Reading system configuration...\n");

	FILE* stream_system = fopen(system_filename, "r");
	if (!stream_system){
		// printf("Vaquita system file not found************!\nExiting...\n");
		fprintf(logs_output_file, "Vaquita system file '%s' not found!\n", system_filename);
		exit(-1);
	}
	char line_system[1024];
	int first_line_skiped_flag = 0;
	int field_ix;
	while (fgets(line_system, 1024, stream_system))
	{
		if(!first_line_skiped_flag){
			first_line_skiped_flag = 1;
		} else{

			field_ix = 1;
			char* tmp = strdup(line_system);

			tmp = strdup(line_system);
			const char* num_channels_vaquita_char = getfield(tmp, field_ix);
			num_channels_vaquita = atoi(num_channels_vaquita_char);
			// printf("- num_channels_vaquita_char = %d\n", num_channels_vaquita);
			fprintf(logs_output_file, "- num_channels_vaquita = %d\n", num_channels_vaquita);
			field_ix++;

			tmp = strdup(line_system);
			const char* basic_channel_bandwidth_char = getfield(tmp, field_ix);
			basic_channel_bandwidth = atoi(basic_channel_bandwidth_char);
			// printf("- num_channels_vaquita_char = %d\n", num_channels_vaquita);
			fprintf(logs_output_file, "- basic_channel_bandwidth = %d\n", basic_channel_bandwidth);
			field_ix++;

			tmp = strdup(line_system);
			const char* cw_char = getfield(tmp, field_ix);
			CW = atoi(cw_char);
			// printf("- CW = %d\n", CW);
			fprintf(logs_output_file, "- CW = %d\n", CW);
			field_ix++;

			tmp = strdup(line_system);
			const char* pdf_backoff_char = getfield(tmp, field_ix);
			pdf_backoff = atoi(pdf_backoff_char);
			// printf("- pdf_backoff = %d\n", pdf_backoff);
			fprintf(logs_output_file, "- pdf_backoff = %d\n", pdf_backoff);
			field_ix++;

			tmp = strdup(line_system);
			const char* pdf_tx_time_char = getfield(tmp, field_ix);
			pdf_tx_time = atoi(pdf_tx_time_char);
			// printf("- pdf_tx_time = %d\n", pdf_tx_time);
			fprintf(logs_output_file, "- pdf_tx_time = %d\n", pdf_tx_time);
			field_ix++;

			tmp = strdup(line_system);
			const char* pkt_length_char = getfield(tmp, field_ix);
			packet_length = atoi(pkt_length_char);
			printf("- packet_length = %d\n", packet_length);
			fprintf(logs_output_file, "- packet_length = %d\n", packet_length);
			field_ix++;

			tmp = strdup(line_system);
			const char* logs_flag_char = getfield(tmp, field_ix);
			logs_flag = atoi(logs_flag_char);
			printf("- logs_flag = %d\n", logs_flag);
			fprintf(logs_output_file, "- logs_flag = %d\n", logs_flag);
			field_ix++;

			tmp = strdup(line_system);
			const char* path_loss_model_char = getfield(tmp, field_ix);
			path_loss_model = atoi(path_loss_model_char);
			printf("- path_loss_model = %d\n", path_loss_model);
			fprintf(logs_output_file, "- path_loss_model = %d\n", path_loss_model);
			field_ix++;

			tmp = strdup(line_system);
			const char* num_packets_aggregated_char = getfield(tmp, field_ix);
			num_packets_aggregated = atoi(num_packets_aggregated_char);
			printf("- num_packets_aggregated = %d\n", num_packets_aggregated);
			fprintf(logs_output_file, "- num_packets_aggregated = %d\n", num_packets_aggregated);
			field_ix++;

			tmp = strdup(line_system);
			const char* wavelength_char = getfield(tmp, field_ix);
			wavelength = atof(wavelength_char);
			printf("- wavelength = %f m\n", wavelength);
			fprintf(logs_output_file, "- wavelength = %f m\n", wavelength);
			field_ix++;

			tmp = strdup(line_system);
			const char* noise_level_char = getfield(tmp, field_ix);
			noise_level = atof(noise_level_char);
			printf("- noise_level = %f dBm\n", noise_level);
			fprintf(logs_output_file, "- noise_level = %f dBm\n", noise_level);
			field_ix++;

			tmp = strdup(line_system);
			const char* cochannel_model_char = getfield(tmp, field_ix);
			cochannel_model = atof(cochannel_model_char);
			printf("- cochannel_model = %d\n", cochannel_model);
			fprintf(logs_output_file, "- cochannel_model = %d\n", cochannel_model);
			field_ix++;

			tmp = strdup(line_system);
			const char* collisions_model_char = getfield(tmp, field_ix);
			collisions_model = atof(collisions_model_char);
			printf("- collisions_model = %d\n", collisions_model);
			fprintf(logs_output_file, "- collisions_model = %d\n", collisions_model);
			field_ix++;

			free(tmp);
		}
	}
	fclose(stream_system);
}


/*
 * generateNodesByReadingInputFile()
 * Input arguments:
 * - nodes_filename: ...
 * - sim_time: [dBm]
 */
void VaquitaEnvironment :: generateNodesByReadingInputFile(char *nodes_filename, double sim_time) {
	printf("Reading nodes configuration...\n");
	fprintf(logs_output_file, "Reading nodes configuration...\n");
	total_nodes_number = getNumOfLines(nodes_filename);
	fprintf(logs_output_file, "- Number of nodes: %d\n", total_nodes_number);
	node_container.SetSize(total_nodes_number);
	FILE* stream_nodes = fopen(nodes_filename, "r");

	char line_nodes[1024];
	int first_line_skiped_flag = 0;
	int node_ix = 0;
	int node_id_aux = 0;
	int field_ix;
	while (fgets(line_nodes, 1024, stream_nodes))
	{
		field_ix = 1;
		if(!first_line_skiped_flag){
			first_line_skiped_flag = 1;
		} else{
			char* tmp_nodes = strdup(line_nodes);
			fprintf(logs_output_file, " ----------------------------- \n");

			// Set nodes
			node_container[node_ix].total_nodes_number = total_nodes_number;
			tmp_nodes = strdup(line_nodes);
			node_id_aux = atoi(getfield(tmp_nodes, field_ix));
			node_container[node_ix].node_id = node_id_aux;
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].primary_channel = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].min_channel_allowed = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].max_channel_allowed = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].tpc_default = atof(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].cca_default = atof(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].x = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].y = atof(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].z = atof(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].tx_gain = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].rx_gain = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].channel_bonding_model = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].destination_id = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			double EB = (double) (CW-1)/2;
			node_container[node_ix].lambda =  1/(EB * SLOT);
			node_container[node_ix].wavelength = wavelength;
			node_container[node_ix].path_loss_model = path_loss_model;
			node_container[node_ix].packet_length = packet_length;
			node_container[node_ix].num_packets_aggregated = num_packets_aggregated;
			node_container[node_ix].num_channels_vaquita = num_channels_vaquita;
			node_container[node_ix].noise_level = noise_level;
			node_container[node_ix].sim_time = sim_time;
			node_container[node_ix].cochannel_model = cochannel_model;
			node_container[node_ix].save_node_logs = save_node_logs;
			node_container[node_ix].collisions_model = collisions_model;
			node_container[node_ix].basic_channel_bandwidth = basic_channel_bandwidth;

			free(tmp_nodes);
			node_container[node_ix].printNodeInfo();
			node_ix++;
		}
	}
}


/*
 * getfield(): returns a field corresponding to a given index from a csv file
 */
const char* getfield(char* line, int num){
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
 * getNumOfLines(): returns the number of lines of a csv file
 */
int getNumOfLines(char *nodes_filename){
	int num_lines = 0;
	// Nodes file
	FILE* stream = fopen(nodes_filename, "r");
	if (!stream){
		printf("Nodes configuration file %s not found!\n", nodes_filename);
		exit(-1);
	}
	char line[1024];
	while (fgets(line, 1024, stream))
	{
		num_lines++;
	}
	num_lines--;
	fclose(stream);
	return num_lines;
}

/*****************/
/*****************/
/* MAIN FUNCTION */
/*****************/
/*****************/

int main(int argc, char *argv[]){

	printf("\n\n\n------------------------------------------\n");
	printf("*******************************\n");
	printf("* VAQUITA SIMULATION EXECUTED *\n");
	printf("*******************************\n");

	printf("\n");
	printf("System configuration:\n");
	printf("- System input file := %s\n", argv[1]);
	printf("- Nodes input file := %s\n", argv[2]);
	printf("- Script output file := %s\n", argv[3]);
	printf("- Observation time := %s s\n", argv[4]);
	printf("- Seed := %s\n", argv[5]);
	printf("- Logs on := %s\n", argv[6]);
	printf("\n");

	if(argc != 7) {
		printf("error: Arguments where not set properly!\n  Execute './VaquitaSimulation -stop_time [s] -seed -logs_on -system_filename -nodes_filename'.\n\n");
		return(-1);
	}

	char *system_filename = argv[1];
	char *nodes_filename = argv[2];
	char *script_filename = argv[3];
	double Sim_time = atof(argv[4]);
	int seed = atoi(argv[5]);
	int save_node_logs = atoi(argv[6]);

	// Generate VaquitaEnvironment component
	VaquitaEnvironment test;
	test.Seed = seed;
	test.StopTime(Sim_time);
	test.Setup(Sim_time, save_node_logs,system_filename, nodes_filename, script_filename);
	test.Run();
	return(0);
};
