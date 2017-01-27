#include <math.h>
#include <algorithm>

#include "structures/Notification.h"
#include "structures/NACK.h"
#include "list_of_defines.h"

// Node component: "TypeII" represents components that are aware of the existence of the simulated time.
component Node : public TypeII{
	public:
		// The description of each of the following functions can be found below
		void Setup();
		void Start();
		void Stop();
		void initializeVariables();
		double computeTxTime(int num_channels_used);
		void printNodeStatistics(int write_or_print);
		int isPacketLost(Notification notification);
		void sendNack(int packet_id, int node_id_a, int node_id_b, int loss_reason);
		void processNack(NackInfo nack_info);
		void cleanNack();
		void updateChannelsPower(Notification notification, int update_type);
		Notification generateNotification(int destination_id, double tx_duration);
		void getTxChannelsByChannelBonding(int channel_bonding_model, int *channels_free);
		void getChannelOccupancyByCCA();
		double computeBackoff(int pdf, double lambda);
		void handlePacketLoss();
		void computeMaxInterference(Notification notification);
		void handleBackoff(int pause_or_resume, Notification notification);
		void pauseBackoff();
		void resumeBackoff();
		void restartNode();
		void printNodeInfo(int write_or_print);
		void printChannelPower(int write_or_print);

	public:
		int node_id; 				// Node identifier
		int destination_id;			// Destination node id
		double lambda;				// Average packet generation rate
		double mu;					// Average packet departure rate
		int primary_channel;		// Primary channel
		int min_channel_allowed;	// Min. allowed channel
		int max_channel_allowed;	// Max. allowed channel
		double tpc_default;			// Default power transmission
		double cca_default;			// Default CCA	("sensitivity" threshold)
		int x;						// X position coordinate
		int y;						// Y position coordinate
		int z;						// Z position coordinate
		double tx_gain;				// Antenna transmission gain (dB)
		double rx_gain;				// Antenna reception gain (dB)
		int channel_bonding_model;	// Channel bonding model (definition of models in function getTxChannelsByChannelBonding())
		int cochannel_model;		// Co-channel interference model (definition of models in function updateChannelsPower())
		int collisions_model;		// Collisions model

		// Same default in every node (parameters from system input and console arguments)
		double sim_time;				// Observation time (time when the simulation stops)
		int total_nodes_number;			// Total number of nodes
		double wavelength;				// Signal wavelength [m] (in WiFi 0.1249 m)
		int num_channels_komondor;		// Number of subchannels composing the whole channel
		int CW;							// Backoff contention window
		int pdf_backoff;				// Probability distribution type of the backoff (0: exponential, 1: deterministic)
		int pdf_tx_time;				// Probability distribution type of the transmission time (0: exponential, 1: deterministic)
		int packet_length;				// Packet length [bits]
		int path_loss_model;			// Path loss model (0: free-space, 1: Okumura-Hata model - Uban areas)
		int num_packets_aggregated;		// Number of packets aggregated in one transmission
		double noise_level;				// Environment noise [dBm]
		int save_node_logs;				// Flag for activating the log writting of nodes
		int basic_channel_bandwidth;	// Bandwidth of a basic channel [Mbps]
		int data_rate_array[4];			// Hardcoded data rates [bps] (corresponding to the CTMN Matlab code)

		// Statistics
		int packets_sent;
		double *total_time_transmitting_per_channel;		// Time transmitting per channel;
		double *total_time_transmitting_in_num_channels;	// Time transmitting in (ix 0: 1 channel, ix 1: 2 channels...)
		double *total_time_lost_per_channel;				// Time transmitting per channel unsuccessfully;
		double *total_time_lost_in_num_channels;			// Time transmitting in (ix 0: 1 channel, ix 1: 2 channels...) unsuccessfully
		double throughput;									// Throughput [Mbps]
		int packets_lost;									// Own packets that have been collided or lost
		int *nacks_received;								// Counter of the type of Nacks received

	private:
		// Komondor environment
		double *channel_power;				// Channel power detected in each sub-channel [pW] (pico watts for resolution issues)
		int *channels_free;					// Channels that are found free for the beginning TX
		int *channels_for_tx;				// Channels that are used in the beginning TX (depend on the channel bonding model)

		// File for writting node logs
		FILE *own_log_file;					// File for logs in which the node is involved
		char own_file_path[32];				// Name of the file for node logs

		// State and timers
		int node_state;						// Node's internal state (0: sensing the channel, 1: transmitting, 2: receiving packet)
		double remaining_backoff;			// Remaining backoff
		int progress_bar_delta;				// Percentage value between displayed values in the progress bar
		int progress_bar_counter;			// Counter for displaying the progress bar

		// Transmission parameters
		int current_left_channel;			// Left channel used in current TX
		int current_right_channel;			// Right channel used in current TX
		int current_tpc;					// Transmission power used in current TX
		int current_cca;					// Current CCA (variable "sensitivity")
		int current_destination_id;			// Current destination ID
		int num_channels_allowed;			// Maximum number of channels allowed to TX in
		double current_tx_duration;			// Duration of the TX being done
		int packet_id;						// Packet ID

		// Sensing and Reception parameters
		NackInfo nack;						// NACK to be filled in case node is the destination of tx loss
		double max_pw_interference;			// Maximum interference detected in range of interest
		int channel_max_interference;		// Channel of maximum interference detected in range of interest
		double *power_received_per_node;	// Power received from each node in the network [pW]
		double pw_received_interest;		// Power received from a TX destined to the node [pW]
		int receiving_from_node_id;			// ID of the node that is transmitting to the node (-1 if node is not receiing)
		int receiving_packet_id;			// IF of the packet that is being transmitted to me
		int *hidden_nodes_list;				// For each node, determine if is hidden node with respect to me with 1

	public:
		// INPORT connections for receiving notifications
		inport void inline inportSomeNodeStartTX(Notification &notification);
		inport void inline inportSomeNodeFinishTX(Notification &notification);
		inport void inline inportNackReceived(NackInfo &nack_info);

		// OUTPORT connections for sending notifications
		outport void outportSelfStartTX(Notification &notification);
		outport void outportSelfFinishTX(Notification &notification);
		outport void outportSendNack(NackInfo &nack_info);

		// Triggers
		Timer <trigger_t> trigger_backoff; 		// Duration of current trigger_backoff. Triggers outportSelfStartTX()
		Timer <trigger_t> trigger_toFinishTX; 	// Duration of current packet transmission. Triggers outportSelfFinishTX()
		Timer <trigger_t> trigger_sim_time;		// Timer for displaying the exectuion time status (progress bar)

		// Every time the timer expires execute this
		inport inline void endBackoff(trigger_t& t1);
		inport inline void myTXFinished(trigger_t& t1);
		inport inline void printProgressBar(trigger_t& t1);

		// Connect timers to methods
		Node () {
			connect trigger_backoff.to_component,endBackoff;
			connect trigger_toFinishTX.to_component,myTXFinished;
			connect trigger_sim_time.to_component,printProgressBar;
		}
};

void Node :: Setup(){
	// Do nothing
};

void Node :: Start(){

	// Name node log file accordingly to the node_id
	if(save_node_logs) sprintf(own_file_path,"%s%d.txt","./output/logs_output_node_", node_id);
	if(save_node_logs) remove(own_file_path);
	if(save_node_logs) own_log_file = fopen(own_file_path, "at");

	if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; Start()\n", SimTime(), node_id, STATE_UNKNOWN, LOG_B00);

	printNodeInfo(WRITE_LOG);

	initializeVariables();

	remaining_backoff = computeBackoff(pdf_backoff, lambda);
	resumeBackoff();

	// Progress bar (only print by node with id 0)
	if(node_id == 0){
		printf("PROGRESS BAR:\n");
		trigger_sim_time.Set(SimTime() + 0.000001);
	}

	// if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; Start() END\n", SimTime(), node_id, node_state, LOG_B01);
};

void Node :: Stop(){

	if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;    - Node Stop()\n", SimTime(), node_id, node_state, LOG_C00);

	printNodeStatistics(PRINT_LOG);
	printNodeStatistics(WRITE_LOG);

	if(save_node_logs) fclose(own_log_file);

	// if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;    - Node info:\n", SimTime(), node_id, node_state, LOG_C01);
};

/*************************************/
/*************************************/
/* SOME NODE HAS STARTED A PACKET TX */
/*************************************/
/*************************************/

void Node :: inportSomeNodeStartTX(Notification &notification){

	if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; inportSomeNodeStartTX(): source = %d - destination = %d\n",
			SimTime(), node_id, node_state, LOG_D00, notification.source_id, notification.tx_info.destination_id);

	if(notification.source_id == node_id){ // If node is the transmitter, do nothing

		if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - I have started a TX in channels: %d - %d to N%d\n",
				SimTime(), node_id, node_state, LOG_D02, notification.left_channel, notification.right_channel, notification.tx_info.destination_id);

	} else {	// If other node is the transmitter

		if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - N%d has started a TX in channels: %d - %d to N%d\n",
				SimTime(), node_id, node_state, LOG_D02, notification.source_id, notification.left_channel, notification.right_channel, notification.tx_info.destination_id);

		updateChannelsPower(notification, TX_INITIATED);	// Update the power sensed at each channel
		pw_received_interest = power_received_per_node[notification.source_id];
		computeMaxInterference(notification);

		// Decide action according to current state
		int loss_reason;	// Packet loss reason (if any)
		switch(node_state){

			// SENSING STATE
			case 0:{

				if(notification.tx_info.destination_id == node_id){	// Node is the destination

					if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;    + I am the TX destination (N%d)\n",
							SimTime(), node_id, node_state, LOG_D07, notification.tx_info.destination_id);

					// Check if packet has been lost due to interferences or weak signal strength
					loss_reason = isPacketLost(notification);
					if(loss_reason != PACKET_NOT_LOST) {	// If packet is lost send logical Nack

						if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;       - Reception of packet %d from %d CANNOT be started because of reason %d\n",
								SimTime(), node_id, node_state, LOG_D14, notification.tx_info.packet_id, notification.source_id, loss_reason);
						sendNack(notification.tx_info.packet_id, notification.source_id, NO_NODE_ID, loss_reason);

					} else {	// Packet can be properly received

						if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;       - Reception of packet %d from %d CAN be started\n",
								SimTime(), node_id, node_state, LOG_D15, notification.tx_info.packet_id, notification.source_id);
						node_state = STATE_RX;
						receiving_from_node_id = notification.source_id;
						receiving_packet_id = notification.tx_info.packet_id;
						pauseBackoff();

					}
				} else {	// Node is not the destination

					if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;    + I am NOT the TX destination (N%d)\n",
								SimTime(), node_id, node_state, LOG_D07, notification.tx_info.destination_id);
					handleBackoff(PAUSE_TIMER, notification);	// Check if I have to freeze the BO (if it is not already frozen)
				}
				break;
			}

			// TRANSMITTING STATE
			case 1:{

				if(save_node_logs) fprintf(own_log_file, "%f;N%d;D07; - I am in TRANSMITTING state\n",SimTime(), node_id);

				if(notification.tx_info.destination_id == node_id){ // Node is the destination

					if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;    + I am the TX destination (N%d)\n",
												SimTime(), node_id, node_state, LOG_D07, notification.tx_info.destination_id);
					// Packet cannot be received as I am transmitting...
					if(save_node_logs) fprintf(own_log_file, "%f;N%d;D14;    + I am transmitting, packet cannot be received\n",SimTime(), node_id);
					loss_reason = PACKET_LOST_DESTINATION_TX;
					sendNack(notification.tx_info.packet_id, notification.source_id, -1, loss_reason);

				} else {	// Node is NOT the destination: do nothing
					if(save_node_logs) fprintf(own_log_file, "%f;N%d;D07; - I am NOT the TX destination (N%d)\n",SimTime(), node_id, notification.tx_info.destination_id);
				}
				break;
			}

			// RECEIVING STATE
			case 2:{

				if(save_node_logs) fprintf(own_log_file, "%f;N%d;D07; - I am in RECEIVING state\n",SimTime(), node_id);

				if(notification.tx_info.destination_id == node_id){	// Node is the destination
					if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;    + I am the TX destination (N%d)\n",
												SimTime(), node_id, node_state, LOG_D07, notification.tx_info.destination_id);
					if(convertPower(PICO_TO_DBM,pw_received_interest) >= current_cca){
						// Pure collision (two nodes transmitting to me)
						if(save_node_logs) fprintf(own_log_file, "%f;N%d;D07; - I am the TX destination (N%d)\n",SimTime(),
								node_id, notification.tx_info.destination_id);
						// Send NACK
						loss_reason = 3;
						sendNack(notification.tx_info.packet_id, receiving_from_node_id, notification.source_id, loss_reason);
					} else {
						// Hidden node
						if(save_node_logs) fprintf(own_log_file, "%f;N%d;D07; - I am NOT the TX destination (N%d)\n",SimTime(),
								node_id, notification.tx_info.destination_id);
						// Send NACK
						loss_reason = 4;
						sendNack(notification.tx_info.packet_id, receiving_from_node_id, notification.source_id, loss_reason);
					}
				} else if(convertPower(PICO_TO_DBM,max_pw_interference) >= current_cca){
					loss_reason = 2;
					sendNack(notification.tx_info.packet_id, notification.source_id, -1, loss_reason);
				}
				break;
			}
			default:
				printf("ERROR: %d is not a correct state\n", node_state);
				break;
		}
	}

	// if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; inportSomeNodeStartTX() END\n", SimTime(), node_id, node_state, LOG_D00,);
};

/**************************************/
/**************************************/
/* SOME NODE HAS FINISHED A PACKET TX */
/**************************************/
/**************************************/

void Node :: inportSomeNodeFinishTX(Notification &notification){

	if(save_node_logs) fprintf(own_log_file, "%f;N%d;E00; inportSomeNodeFinishTX(): source = %d - destination = %d\n",
			SimTime(), node_id, notification.source_id, notification.tx_info.destination_id);

	if(notification.source_id == node_id){
		// Own transmission is finished: do nothing
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;E02; - I have finished a TX in channel range: %d - %d\n",
				SimTime(), node_id, notification.left_channel, notification.right_channel);
	} else {
		// Some node has finished its transmission
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;E02; - N%d has finished a TX in channel range: %d - %d\n",
				SimTime(), node_id, notification.source_id, notification.left_channel, notification.right_channel);
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;E03; - Initial power of transmitter N%d = %f pW\n",
				SimTime(), node_id, notification.source_id, power_received_per_node[notification.source_id]);
		updateChannelsPower(notification, 0);

		// DECIDE WHAT TO DO ACCORDING TO THE STATE
		switch(node_state){
			// SENSING STATE: backoff process
			case 0:{
				handleBackoff(1, notification);	// Attempt to restart Backoff
				break;
			}
			// TRANSMITTING STATE: do nothing
			case 1:{
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;E09; - I am TRANSMITTING ---> do nothing\n", SimTime(), node_id);
				break;
			}
			// RECEIVING STATE: check if my transmission has finished
			case 2:{
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;E09; - I am RECEIVING\n", SimTime(), node_id);
				// Check if my transmission has finished
				if(notification.tx_info.destination_id == node_id){
					if(save_node_logs) fprintf(own_log_file, "%f;N%d;E06; - Packet %d transmission from %d is finished successfully\n",
							SimTime(), node_id, notification.tx_info.packet_id, notification.source_id);
					node_state = 0;
					receiving_from_node_id = -1;
					receiving_packet_id = -1;
					// Check now if channel is free
					handleBackoff(1, notification);
				} else {
					// Transmission was not mine
					if(save_node_logs) fprintf(own_log_file, "%f;N%d;E09; - I am not the destination --> do nothing\n", SimTime(), node_id);
				}
				break;
			}
		}
	}
	// if(save_node_logs) fprintf(own_log_file, "%f;N%d;E00; - inportSomeNodeFinishTX() END\n", SimTime(), node_id);
};

/************************************/
/************************************/
/* SOME NODE HAS SENT A NACK NOTIF. */
/************************************/
/************************************/

// Someone has sent as Nack notification
void Node :: inportNackReceived(NackInfo &nack_info){
	if(nack_info.source_id != node_id){
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;G00; inportNackReceived() from %d\n", SimTime(), node_id, nack_info.source_id);
		processNack(nack_info);
	} else {
		// do nothing
	}
	// if(save_node_logs) fprintf(own_log_file, "%f;N%d;G01; inportNackReceived() END\n", SimTime(), node_id);
}


/**********************************/
/**********************************/
/* BACKOFF HAS EXPIRED - START TX */
/**********************************/
/**********************************/

void Node :: endBackoff(trigger_t &){
	//printf("%f: [N%d] - endBackoff()\n", SimTime(), node_id);
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;F00; endBackoff()\n", SimTime(), node_id);

	node_state = 1;	// Change state to TXing

	getChannelOccupancyByCCA();
	getTxChannelsByChannelBonding(channel_bonding_model, channels_free);

	if(save_node_logs) fprintf(own_log_file, "%f:N%d;F02 - Channels for transmitting: ",SimTime(), node_id);
	printChannelForTx(save_node_logs, 1, channels_for_tx, num_channels_komondor, own_log_file);

	// Check if TX is possible (coded in the first element of channels_for_tx array)
	if(channels_for_tx[0] != -1){
		// TX is possible
		if(save_node_logs) fprintf(own_log_file, "%f:N%d;F03 - Transmission is possible\n",SimTime(), node_id);
		current_left_channel = getBoundaryChannel(0, channels_for_tx,  num_channels_komondor);
		current_right_channel = getBoundaryChannel(1, channels_for_tx, num_channels_komondor);
		int num_channels_tx = current_right_channel - current_left_channel + 1;
		current_tx_duration = computeTxTime(num_channels_tx-1);
		if(save_node_logs) fprintf(own_log_file, "%f:N%d;F04    + Selected transmission range: %d - %d\n",
				SimTime(), node_id, current_left_channel, current_right_channel);
		// Send packet
		Notification notification = generateNotification(current_destination_id, current_tx_duration);
		outportSelfStartTX(notification);
		trigger_toFinishTX.Set(SimTime()+current_tx_duration);
		packets_sent++;
		packet_id++;
		for(int c = current_left_channel; c <= current_right_channel; c++){
			total_time_transmitting_per_channel[c] += current_tx_duration;
		}
		total_time_transmitting_in_num_channels[current_right_channel - current_left_channel] += current_tx_duration;
	} else {
		// TX not possible due to CB requirements. Compute a new backoff.
		if(save_node_logs) fprintf(own_log_file, "%f:N%d;F03 - Transmission is NOT possible\n",SimTime(), node_id);
		remaining_backoff = computeBackoff(pdf_backoff, lambda);
		resumeBackoff();
		if(save_node_logs) fprintf(own_log_file, "%f:N%d;F05    + New backoff generated of %f s\n",SimTime(), node_id, remaining_backoff);
	}
	// if(save_node_logs) fprintf(own_log_file, "%f;N%d;E01; endBackoff() END\n", SimTime(), node_id);
};

/***********************************/
/***********************************/
/* MY TX HAS FINISHED - NOTIFY ALL */
/***********************************/
/***********************************/

void Node :: myTXFinished(trigger_t &){
	// printf("%f: [N%d] - myTXFinished()\n", SimTime(), node_id);
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;G00; myTXFinished()\n", SimTime(), node_id);
	node_state = 0;	// Change state to "sensing"
	Notification notification = generateNotification(current_destination_id, 0);
	outportSelfFinishTX(notification);
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;G02; - New BO generated of %f s\n", SimTime(), node_id, remaining_backoff);
	restartNode();
	// if(save_node_logs) fprintf(own_log_file, "%f;N%d;G01; myTXFinished() end\n", SimTime(), node_id)
};

/***********************/
/***********************/
/*  POWER MANAGEMENT   */
/***********************/
/***********************/

/*
 * computePowerReceived() returns the power received in a given distance from the transmistter depending on the path loss model
 * Input arguments:
 * - path_loss_model: ...
 * - tx_power: [dBm]
 * - tx_gain and rx_gain: [dBm]
 * Output arguments:
 * - Power received [dBm]
 */
double computePowerReceived(int path_loss, int distance, double wavelength, double tx_power, int tx_gain, int rx_gain){
	double pw_received = 0;
	switch(path_loss){
		// Free space
		case 0:{
			pw_received = tx_power + tx_gain + rx_gain + 20 * log10(wavelength/(4*M_PI*distance));
			break;
		}
		// Okumura-Hata model - Uban areas
		case 1:{
			double tx_heigth = 10;		// Transmitter height [m]
			double rx_heigth = 10;		// Receiver height [m]
			double path_loss_A = 69.55 + 26.16 * log10((3*pow(10,8))/wavelength) - 13.82 * log10(tx_heigth);
			double path_loss_B = 44.9 - 6.55 * log10(tx_heigth);
			double path_loss_E = 3.2 * pow(log10(11.7554 * rx_heigth),2) - 4.97;
			double path_loss = path_loss_A + path_loss_B * log10(distance/1000) - path_loss_E;
			pw_received = tx_power + tx_gain + rx_gain - path_loss;
			break;
		}
		// Indoor model (could suite an apartments building scenario)
		case 2: {
			double path_loss_factor = 5;
			double shadowing = 9.5;
			double obstacles = 30;
			double walls_frequency = 5; //  One wall each 5 meters on average
			double shadowing_at_wlan = (((double) rand())/RAND_MAX)*shadowing;
			double obstacles_at_wlan = (((double) rand())/RAND_MAX)*obstacles;
			double alpha = 4.4; // Propagation model
			double path_loss = path_loss_factor + 10*alpha*log10(distance) + shadowing_at_wlan + (distance/walls_frequency)*obstacles_at_wlan;
			pw_received = tx_power + tx_gain - path_loss; // Power in dBm
			break;
		}
		// Indoor model without variability
		case 3: {
			double path_loss_factor = 5;
			double shadowing = 9.5;
			double obstacles = 30;
			double walls_frequency = 5; //  One wall each 5 meters on average
			double shadowing_at_wlan = 1/2*shadowing;
			double obstacles_at_wlan = 1/2*obstacles;
			double alpha = 4.4; // Propagation model
			double path_loss = path_loss_factor + 10*alpha*log10(distance) + shadowing_at_wlan + (distance/walls_frequency)*obstacles_at_wlan;
			pw_received = tx_power + tx_gain - path_loss; // Power in dBm
			break;
		}
		default:{
			printf("Path loss model not found!\n");
			break;
		}
	}
	return pw_received;
}

/*
 * convertPower()
 * Input arguments:
 * - conversion_type
 * - power
 */
double convertPower(int conversion_type, double power){
	double converted_power;
	switch(conversion_type){

		case PICO_TO_DBM:{
			// pW to dBm
			converted_power = 10 * log10(power * pow(10,-9));
			break;
		}

		case DBM_TO_PICO:{
			// dBm to pW
			converted_power = pow(10,(power + 90)/10) ;
			break;
		}
		default:{
			printf("Power conversion type not found!\n");
			break;
		}
	}
	return converted_power;
}

/*
 * computeDistance(): returns the distance between 2 points
 * Input arguments:
 * - x1,y1,z1: first point position
 * - x2,y2,z2: second point position
 */
double computeDistance(int x1, int y1, int z1, int x2, int y2, int z2){
	double distance = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
	return distance;
}

/*
 * checkCCA() determines if the node is able to resume the backoff as the channel/s are free enough in terms of power detected
 * Input arguments:
 * - channel_power
 * - primary_channel
 * - cca
 */
int exceededCCA(double* channel_power, int primary_channel, int cca){
	// Transmission not allowed (channel power exceeds CCA level)
	if(channel_power[primary_channel] > convertPower(DBM_TO_PICO, cca)){
		return 1;
	} else {
		return 0;
	}
}

/**********************/
/**********************/
/* TX TIME MANAGEMENT */
/**********************/
/**********************/

/*
 * computeTxTime()
 * Input arguments:
 * - num_channels_used
 */
double Node :: computeTxTime(int ix_num_channels_used){

	double tx_time;
	double data_rate = data_rate_array[ix_num_channels_used];
	switch(pdf_tx_time){
		// Exponential
		case 0:{
			tx_time = Exponential((packet_length * num_packets_aggregated)/data_rate);
			break;
		}
		// Deterministic
		case 1:{
			tx_time = (packet_length * num_packets_aggregated)/data_rate;
			break;
		}
		default:{
			printf("TX time model not found!\n");
			break;
		}
	}
	return tx_time;
}

/***********************/
/***********************/
/* CHANNELS MANAGEMENT */
/***********************/
/***********************/

/*
 * getBoundaryChannel()
 * Input arguments:
 * - position: 0 = first, 1 = last
 * - channels_availab
 */
int getBoundaryChannel(int position, int *channels_available, int total_channels_number){
	int left_tx_ch = 0;
	int left_tx_ch_is_set = 0;
	int right_tx_ch = 0;
	for(int c = 0; c < total_channels_number; c++){
		if(channels_available[c]){
			if(!left_tx_ch_is_set){
				left_tx_ch = c;
				left_tx_ch_is_set = 1;
			}
			if(right_tx_ch < c){
				right_tx_ch = c;
			}
		}
	}
	if(position) {
		return right_tx_ch; //LAST
	} else {
		return left_tx_ch; //FIRST
	}
}

/*
 * getChannelOccupancyByCCA(): Returns array with 0/1 (0: channel NOT free, 1: channel free)
 * Input arguments:
 * - channels_free_aux
 * - channel_power
 * - cca
 * - channel_left
 * - channel_right
 */

void Node :: getChannelOccupancyByCCA(){
	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
		if(channel_power[c] < convertPower(DBM_TO_PICO, current_cca)){
			channels_free[c] = 1;
		} else {
			channels_free[c] = 0;
		}
	}
}

/*
 * updateChannelsPower: updates the power in the channel power array depending on the notifications received (tx start, tx end) and the cochannel model
 * Arguments:
 * - notification: type of co-channel model applied (0: no co-channel interference, 1: 20 dB mask, ...)
 * - update_type: type of power update (0: end of TX - decrease sensed power, 1: start of TX - increase sensed power)
 * - pw_received_pico
 */
void Node :: updateChannelsPower(Notification notification, int update_type){

	double pw_received_pico;

	if(update_type == TX_FINISHED) {

		pw_received_pico = power_received_per_node[notification.source_id];
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - Pre update channel state [dBm]: ", SimTime(), node_id, node_state, LOG_D03);

	} else if(update_type == TX_INITIATED) {

		double distance = computeDistance(x, y, z, notification.tx_info.x, notification.tx_info.y, notification.tx_info.z);
		double pw_received = computePowerReceived(path_loss_model, distance, wavelength, notification.tx_info.tx_power, tx_gain, rx_gain);
		pw_received_pico = convertPower(DBM_TO_PICO, pw_received);	// dBm to pW
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - Pre update channel state [dBm]: ", SimTime(), node_id, node_state, LOG_E04);

	} else {
		printf("ERROR: update_type %d does not exist!!!", update_type);
	}
	printChannelPower(WRITE_LOG);

	double distance = computeDistance(x, y, z, notification.tx_info.x, notification.tx_info.y, notification.tx_info.z);
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - Distance to N%d = %f m\n",
			SimTime(), node_id, node_state, LOG_D04, notification.source_id, distance);
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - Power received from transmitter N%d = %f dBm (%f pW)\n",
			SimTime(), node_id, node_state, LOG_D05, notification.source_id, convertPower(PICO_TO_DBM, pw_received_pico), pw_received_pico);
	power_received_per_node[notification.source_id] = pw_received_pico;

	// Total power (of interest and interference) genetared only by the incomming or outgoing TX
	double total_power[num_channels_komondor];
	for(int i = 0; i < num_channels_komondor; i++) {
		total_power[i] = 0;
	}

	// Direct power (power of the channels used for transmitting)
	for(int i = notification.left_channel; i <= notification.right_channel; i++){
		total_power[i] = pw_received_pico;
	}

	// Co-channel intereference power
	switch(cochannel_model){
		// No co-channel interference
		case 0:{
			// Do nothing
			break;
		}
		// (RECOMMENDED) Boundary co-channel interference: only boundary channels (left and right) used in the TX affect the rest of channels
		case 1:{
			for(int c = 0; c < num_channels_komondor; c++) {
				if(c < notification.left_channel || c > notification.right_channel){
					if(c < notification.left_channel) {
						total_power[c] += convertPower(DBM_TO_PICO, convertPower(PICO_TO_DBM, pw_received_pico) - 20*abs(c-notification.left_channel));
					} else if(c > notification.right_channel) {
						total_power[c] += convertPower(DBM_TO_PICO, convertPower(PICO_TO_DBM, pw_received_pico) - 20*abs(c-notification.right_channel));
					}
					if(total_power[c] < 0.000001){
						total_power[c] = 0;
					}
				} else {
					// Inside TX range --> do nothing
				}
			}
			break;
		}
		// Extreme co-channel interference: ALL channels used in the TX affect the rest of channels
		case 2:{
			for(int i = 0; i < num_channels_komondor; i++) {
				for(int j = notification.left_channel; j <= notification.right_channel; j++){
					if(i != j) {
						total_power[i] += convertPower(PICO_TO_DBM, convertPower(DBM_TO_PICO, pw_received_pico) - 20*abs(i-j));
						if(total_power[i] < 0.00001){
							total_power[i] = 0;
						}
					}
				}
			}
			break;
		}
		default:{
			break;
		}
	}

	// Increase/decrease power sensed if TX started/finished
	for(int c = 0; c < num_channels_komondor; c++){
		if(update_type == 0) {
			channel_power[c] -= total_power[c];
		} else {
			channel_power[c] += total_power[c];
		}
	}

	if(save_node_logs){
		if(update_type == TX_INITIATED)	fprintf(own_log_file, "%f;N%d;S%d;%s; - Post update channel state [dBm]: ",
				SimTime(), node_id, node_state, LOG_D06);
		if(update_type == TX_FINISHED) fprintf(own_log_file, "%f;N%d;S%d;%s; - Post update channel state [dBm]: ",
				SimTime(), node_id, node_state, LOG_E05);
	}
	printChannelPower(WRITE_LOG);
}

/*
 * getTxChannelsByChannelBonding: returns the channels to TX in depending on the channel_bonding scheme and channel_power state.
 */
void Node :: getTxChannelsByChannelBonding(int channel_bonding_model, int *channels_free){
	// printf("\n\n ------------ getTxChannelsByChannelBonding() ----------------\n");

	// Reset channels_for_tx;
	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
		channels_for_tx[c] = 0;
	}

	// I. Get left and right channels available (or free)
	int left_free_ch = 0;
	int left_free_ch_is_set = 0;
	int right_free_ch = 0;
	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
		if(channels_free[c]){
			if(!left_free_ch_is_set){
				left_free_ch = c;
				left_free_ch_is_set = 1;
			}
			if(right_free_ch < c){
				right_free_ch = c;
			}
		}
	}
	// printf("left_free_ch = %d\n", left_free_ch);
	// printf("right_free_ch = %d\n", right_free_ch);
	// printf("primary_channel = %d\n", primary_channel);
	int num_free_ch = right_free_ch - left_free_ch + 1;
	int num_available_ch = max_channel_allowed - min_channel_allowed + 1;


	int m;			// Auxiliary variable representing a modulus
	int left_tx_ch;	// Left channel to TX
	int right_tx_ch;	// Right channel to TX


	// printf("PRE\n");
	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
		// printf("- channels_for_tx[%d] =  %d\n", c, channels_for_tx[c]);
	}

	// Select channels to transmit depending on the sensed power
	switch(channel_bonding_model){

		// Only Primary Channel used if FREE
		case 0:{
			if(primary_channel >= left_free_ch && primary_channel <= right_free_ch){
				channels_for_tx[primary_channel] = 1;
			}
			break;
		}

		// Aggresive SCB: if all channels are FREE, transmit. If not, generate a new backoff.
		case 1:{
			int tx_possible = 1;
			// If all channels are FREE, transmit. If not, generate a new backoff.
			for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
				if(!channels_free[c]){
					tx_possible = 0;
				}
			}
			if(tx_possible){
				left_tx_ch = left_free_ch;
				right_tx_ch = right_free_ch;
				for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
					channels_for_tx[c] = 1;
				}
			} else {
				// TX not possible (code it with negative value)
				channels_for_tx[0] = -1;
			}
			break;
		}

		// Log2 SCB:  if all channels accepted by the log2 mapping are FREE, transmit. If not, generate a new backoff.
		case 2:{
			while(1){
				// printf("num_channels_available = %d\n", num_available_ch);
				// II. If num_free_ch is power of 2
				if(fmod(log10(num_available_ch)/log10(2), 1) == 0){
					// printf("num_free_ch is power of 2!\n");
					m = primary_channel % num_available_ch;
					left_tx_ch = primary_channel - m;
					right_tx_ch = primary_channel + num_available_ch - m - 1;
					// Check if tx channels are inside the free ones
					if((left_tx_ch >= min_channel_allowed) && (right_tx_ch <= max_channel_allowed)){
						// printf("TX channels found are OKAY: %d - %d\n", left_tx_ch, right_tx_ch);
						// TX channels found!
						break;
					} else {
						// printf("TX channels found are NOT POSSIBLE: %d - %d\n", left_tx_ch, right_tx_ch);
						num_available_ch --;
					}
				} else{
					// printf("num_free_ch is NOT power of 2!\n");
					num_available_ch --;
				}
			}

			// If all channels accepted by the log2 mapping, transmit. If not, generate a new backoff.
			int tx_possible = 1;
			for(int c = left_tx_ch; c <= right_tx_ch; c++){
				if(!channels_free[c]){
					tx_possible = 0;
				}
			}
			if(tx_possible){
				for(int c = left_tx_ch; c <= right_tx_ch; c++){
					channels_for_tx[c] = 1;
				}
			} else {
				// TX not possible (code it with negative value)
				channels_for_tx[0] = -1;
			}
			break;
		}

		// Aggresive DCB: TX in all the free channels contiguous to the primary channel
		case 3:{
			for(int c = left_free_ch; c <= right_free_ch; c++){
				channels_for_tx[c] = 1;
			}
			break;
		}

		// Log2 DCB: TX in the larger channel range allowed by the log2 mapping
		case 4:{
			while(1){
				// printf("num_free_ch = %d\n", num_free_ch);
				// II. If num_free_ch is power of 2
				if(fmod(log10(num_free_ch)/log10(2), 1) == 0){
					// printf("num_free_ch is power of 2!\n");
					m = primary_channel % num_free_ch;
					left_tx_ch = primary_channel - m;
					right_tx_ch = primary_channel + num_free_ch - m - 1;
					// Check if tx channels are inside the free ones
					if((left_tx_ch >= min_channel_allowed) && (right_tx_ch <= max_channel_allowed)){
						// printf("TX channels found are OKAY: %d - %d\n", left_tx_ch, right_tx_ch);
						// TX channels found!
						for(int c = left_tx_ch; c <= right_tx_ch; c++){
							channels_for_tx[c] = 1;
						}
						break;
					} else {
						// printf("TX channels found are NOT POSSIBLE: %d - %d\n", left_tx_ch, right_tx_ch);
						num_free_ch --;
					}
				} else{
					// printf("num_free_ch is NOT power of 2!\n");
					num_free_ch --;
				}
			}
			break;
		}
		default:{
			// printf("channel_bonding_model %d is NOT VALID!\n", channel_bonding_model);
			break;
		}
	}


	// printf("Summary:\n- channel_bonding_model: %d\n- channels allowed: %d - %d\n", channel_bonding_model, min_channel_allowed, max_channel_allowed);
	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
		// printf("- channels_for_tx[%d] =  %d\n", c, channels_for_tx[c]);
	}

	// printf("------------ ----------------------------- ----------------\n\n");

}

/*
 * computeMaxInterference():
 * Arguments:
 * - notification:
 * */
void Node :: computeMaxInterference(Notification notification) {
	max_pw_interference = 0;
	for(int c = current_left_channel; c <= current_right_channel; c++){
		if(max_pw_interference <= (channel_power[c] - power_received_per_node[notification.source_id])){
			max_pw_interference = channel_power[c] - power_received_per_node[notification.source_id];
			channel_max_interference = c;
		}
	}
}

/*********************/
/*********************/
/*  HANDLE PACKETS   */
/*********************/
/*********************/

/*
 * generateNotification: Sends a Notification packet
 * Arguments:
 * - destination_id
 * - tx_duration
 */

Notification Node :: generateNotification(int destination_id, double tx_duration){
	Notification notification;
	TxInfo tx_info;
	tx_info.destination_id = destination_id;
	tx_info.tx_duration = tx_duration;
	tx_info.tx_power = current_tpc;
	tx_info.x = x;
	tx_info.y = y;
	tx_info.z = z;
	tx_info.packet_id = packet_id;
	notification.tx_info = tx_info;
	notification.source_id = node_id;
	notification.left_channel = current_left_channel;
	notification.right_channel = current_right_channel;
	notification.packet_length = packet_length;

	return notification;
}

/*
 * sendNack: Sends a NACK packet
 * Arguments:
 * - packet_id:
 * - node_id_a:
 * - node_id_b:
 * - loss_reason:
 */

void Node :: sendNack(int packet_id, int node_id_a, int node_id_b, int loss_reason){
	NackInfo nack_info;
	nack_info.source_id = node_id;
	nack_info.packet_id = packet_id;
	nack_info.loss_reason = loss_reason;
	nack_info.node_id_a = node_id_a;
	nack_info.node_id_b = node_id_b;
	outportSendNack(nack_info);
}

/*
 * cleanNack():
 */
void Node :: cleanNack(){
	nack.source_id = NO_NODE_ID;
	nack.packet_id = NO_PACKET_ID;
	nack.loss_reason = PACKET_NOT_LOST;
	nack.node_id_a = NO_NODE_ID;
	nack.node_id_b = NO_NODE_ID;
}

/*
 * processNack(): Processes a NACK packet
 * Arguments:
 * - nack_info:
 */
void Node :: processNack(NackInfo nack_info) {

	if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - processNack()\n",
							SimTime(), node_id, node_state, LOG_D02);

	if(nack_info.node_id_a == node_id || nack_info.node_id_b == node_id){

		// Node is implied in the logical NACK
		nacks_received[nack_info.loss_reason] ++;

		switch(nack_info.loss_reason){

			case PACKET_LOST_DESTINATION_TX:{
				// Destination was already transmitting when the transmission was attempted
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;G03;    + Destination %d was transmitting!s\n",
						SimTime(), node_id, nack_info.source_id);
				// Add to hidden nodes list ("I was not listening to him!")
				hidden_nodes_list[nack_info.source_id] = 1;
				handlePacketLoss();
				break;
			}

			case PACKET_LOST_LOW_SIGNAL:{
				// Signal strenght is not enough (< CCA) to be decoded
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;G03;    + Power received in destination %d is less than its CCA!\n",
						SimTime(), node_id, nack_info.source_id);
				handlePacketLoss();
				break;
			}

			case PACKET_LOST_INTERFERENCE:{
				// There are interference signals greater than cca (collision)
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;G03;    + Interference sensed in destination %d is greater than its CCA!\n",
						SimTime(), node_id, nack_info.source_id);
				handlePacketLoss();
				break;
			}

			case PACKET_LOST_PURE_COLLISION:{
				// Two nodes transmitting to same destination with signal strengths enough to be decoded
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;G03;    + Pure collision detected at destination %d! %d was transmitting and %d appeared\n",
										SimTime(), node_id, nack_info.source_id, nack_info.node_id_a, nack_info.node_id_b);
				// Add to hidden nodes list
				if(nack_info.node_id_a != node_id) {
					hidden_nodes_list[nack_info.node_id_a] = 1;
				} else if (nack_info.node_id_b != node_id) {
					hidden_nodes_list[nack_info.node_id_b] = 1;
				}
				handlePacketLoss();
				break;
			}
			case PACKET_LOST_LOW_SIGNAL_AND_RX:{
				// Destination already receiving and new signal strength was not enough to be decoded (Case 'PACKET_LOST_LOW_SIGNAL' with hidden node)
				// Only node_id_a has lost the packet, so that node_id_b is his hidden node
				if(nack_info.node_id_a == node_id) {
					if(save_node_logs) fprintf(own_log_file, "%f;N%d;G03;    + Collision detected at destination %d! %d appeared when %d was transmitting\n",
							SimTime(), node_id, nack_info.source_id, nack_info.node_id_b, nack_info.node_id_a);
					hidden_nodes_list[nack_info.node_id_b] = 1;
					handlePacketLoss();
				}
				break;
			}
			default:{
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;G03;    + Unknown reason for packet loss\n", SimTime(), node_id);
				break;
			}
		}
	} else {
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;G02; I am NOT implied in the NACK\n", SimTime(), node_id);
	}
}

/************************/
/************************/
/*PACKET LOSS MANAGEMENT*/
/************************/
/************************/

/*
 * handlePacketLoss():
 */
void Node :: handlePacketLoss(){
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;G02; Packet %d has been LOST!\n", SimTime(), node_id, packet_id);
	for(int c = current_left_channel; c <= current_right_channel; c++){
		total_time_lost_per_channel[c] += current_tx_duration;
	}
	total_time_lost_in_num_channels[current_right_channel - current_left_channel] += current_tx_duration;
	packets_lost ++;
}

/*
 * isPacketLost(): Computes packet loss according to pw_received and interferences (compare with CCA)
 * Arguments:
 * - channel_power:
 * - pw_received_interest:
 * - modulation:
 * Output:
 * - reason
 */
int Node :: isPacketLost(Notification notification){

	//computeMaxInterference(notification);
	int loss_reason = PACKET_NOT_LOST;
	if(convertPower(DBM_TO_PICO, max_pw_interference) >= current_cca){
		// There are interference signal greater than cca (collision)
		loss_reason = PACKET_LOST_INTERFERENCE;
	}
	if (convertPower(DBM_TO_PICO, pw_received_interest) < current_cca) {
		// Signal strenght is not enough (< CCA) to be decoded
		loss_reason = PACKET_LOST_LOW_SIGNAL;
	}
	return loss_reason;
}

/************************/
/************************/
/*  BACKOFF MANAGEMENT  */
/************************/
/************************/

/*
 * computeBackoff():
 * Arguments:
 * - pdf_backoff:
 * - lambda:
 * Output:
 * - backoff:
 * */
double Node :: computeBackoff(int pdf, double lambda){
	double backoff;
	switch(pdf){
		// Exponential
		case 0:{
			backoff = Exponential(1/lambda);
			break;
		}
		// Deterministic
		case 1:{
			backoff = 1/lambda;
			break;
		}
		default:{
			printf("Backoff model not found!\n");
			break;
		}
	}
	return backoff;
}

/*
 * handleBackoff():
 * Arguments:
 * - pause_or_resume
 * - notification
 * */
void Node :: handleBackoff(int pause_or_resume, Notification notification){

	switch(pause_or_resume){

		case PAUSE_TIMER:{

			if(save_node_logs) fprintf(own_log_file, "%f;N%d;D10; - primary_channel (#%d) affected\n", SimTime(), node_id, primary_channel);
			if(save_node_logs) fprintf(own_log_file, "%f;N%d;D11;    + Power sensed in primary channel:  %f dBm (%f pW)\n", SimTime(), node_id, convertPower(DBM_TO_PICO, channel_power[primary_channel]), channel_power[primary_channel]);

			if(exceededCCA(channel_power, primary_channel, current_cca) == CCA_EXCEEDED){ // CCA exceeded

				if(save_node_logs) fprintf(own_log_file, "%f;N%d;D12;    + CCA (%d dBm) exceeded\n", SimTime(), node_id, current_cca);
				pauseBackoff();

			} else {	// CCA NOT exceeded

				if(save_node_logs) fprintf(own_log_file, "%f;N%d;D12;    + CCA (%d dBm) NOT exceeded\n", SimTime(), node_id, current_cca);
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;D10; - primary_channel (#%d) NOT affected\n", SimTime(), node_id, primary_channel);

			}
			break;
		}

		case RESUME_TIMER:{

			if(save_node_logs) fprintf(own_log_file, "%f;N%d;E09; - I am SENSING\n", SimTime(), node_id);
			if(save_node_logs) fprintf(own_log_file, "%f;N%d;E06; - Primary_channel (#%d) affected\n",
					SimTime(), node_id, primary_channel);
			if(save_node_logs) fprintf(own_log_file, "%f;N%d;E07; - Power sensed in primary channel:  %f dBm (%f pW)\n",
					SimTime(), node_id, convertPower(DBM_TO_PICO, channel_power[primary_channel]), channel_power[primary_channel]);

			if(exceededCCA(channel_power, primary_channel, current_cca) == CCA_NOT_EXCEEDED){ // CCA NOT exceeded
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;E08;    + CCA (%d dBm) NOT exceeded\n",
						SimTime(), node_id, current_cca);
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;D10; - primary_channel (#%d) NOT affected\n", SimTime(), node_id, primary_channel);
				resumeBackoff();

			} else {	// CCA exceeded
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;E08;    + CCA (%d dBm) exceeded\n", SimTime(), node_id, current_cca);
			}
			break;
		}
		default:{
			printf("pause_or_resume mode unknown!");
			break;
		}
	}
}

/*
 * pauseBackoff():
 * */
void Node :: pauseBackoff(){
	// If backoff trigger is active, freeze it
	if(trigger_backoff.Active()){
		remaining_backoff = trigger_backoff.GetTime()-SimTime();
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;D13;       - Backoff is active --> freeze it at %f s\n", SimTime(), node_id, remaining_backoff);
		trigger_backoff.Cancel();
		//node_state = 3;
	} else {
		// printf("%f: [N%d]    - Backoff is already frozen at %f s\n", SimTime(), node_id, trigger_backoff.GetTime());
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;D13;       - Backoff is NOT active (frozen at %f s)\n", SimTime(), node_id, trigger_backoff.GetTime());
	}
}

/*
 * resumeBackoff():
 * */
void Node :: resumeBackoff(){
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;E09;       - resuming backoff at %f\n",
			SimTime(), node_id, remaining_backoff);
	// Resume Backoff
	//node_state = 0;
	trigger_backoff.Set(SimTime() + remaining_backoff);
}

/***********************/
/***********************/
/* AUXILIARY FUNCTIONS */
/***********************/
/***********************/

/*
 * restartNode(): Go back to initial state (sensing and decreasing BO)
 */
void Node :: restartNode(){
	total_time_transmitting_in_num_channels[current_right_channel - current_left_channel] += current_tx_duration;
	// Reinitialize parameters
	current_tx_duration = 0;
	node_state = 0;
	receiving_from_node_id = -1;
	receiving_packet_id = -1;
	remaining_backoff = computeBackoff(pdf_backoff, lambda);
	resumeBackoff();
	cleanNack();
}

void Node :: printNodeInfo(int write_or_print){
	switch(write_or_print){
		case PRINT_LOG:{
			printf("    - Node %d info:\n", node_id);
			printf("       · position = (%d, %d, %d)\n", x, y, z);
			printf("       · primary_channel = %d\n", primary_channel);
			printf("       · min_channel_allowed = %d\n", min_channel_allowed);
			printf("       · max_channel_allowed = %d\n", max_channel_allowed);
			printf("       · lambda = %f packets/s\n", lambda);
			printf("       · channel_bonding_model = %d\n", channel_bonding_model);
			printf("       · destination_id = %d\n", destination_id);
			printf("       · tpc_default = %f dBm\n", tpc_default);
			printf("       · cca_default = %f dBm\n", cca_default);
			break;
		}
		case WRITE_LOG:{
			if(save_node_logs) {
				fprintf(own_log_file, "%f;N%d;S%d;%s;    - Node info:\n", SimTime(), node_id, node_state, LOG_Z00);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · position = (%d, %d, %d)\n", SimTime(), node_id, node_state, LOG_Z00, x, y, z);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · primary_channel = %d\n", SimTime(), node_id, node_state, LOG_Z00, primary_channel);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · min_channel_allowed = %d\n", SimTime(), node_id, node_state, LOG_Z00, min_channel_allowed);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · max_channel_allowed = %d\n", SimTime(), node_id, node_state, LOG_Z00, max_channel_allowed);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · lambda = %f packets/s\n", SimTime(), node_id, node_state, LOG_Z00, lambda);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · channel_bonding_model = %d\n", SimTime(), node_id, node_state, LOG_Z00, channel_bonding_model);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · destination_id = %d\n", SimTime(), node_id, node_state, LOG_Z00, destination_id);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · tpc_default = %f dBm\n", SimTime(), node_id, node_state, LOG_Z00, tpc_default);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · cca_default = %f dBm\n", SimTime(), node_id, node_state, LOG_Z00, cca_default);
			}
			break;
		}
	}


}

/*
 * printProgressBar(): prints the progress bar of the Komondor simulation
 */
void Node :: printProgressBar(trigger_t &){
	printf("* %d %% *\n", progress_bar_counter * progress_bar_delta);
	trigger_sim_time.Set(SimTime()+sim_time/(100/progress_bar_delta));

	// *** END PROGRESS BAR (ONLY NODE 0) ***
	if(node_id == 0 && progress_bar_counter == (100/progress_bar_delta)-1){
		trigger_sim_time.Set(SimTime()+sim_time/(100/progress_bar_delta)-0.0001);
	}
	progress_bar_counter ++;
}

/*
 * initializeVariables(): initializes all the necessary variables
 */
void Node :: initializeVariables() {

	channel_power = (double *) malloc(num_channels_komondor * sizeof(*channel_power));
	num_channels_allowed = (max_channel_allowed - min_channel_allowed + 1);
	total_time_transmitting_per_channel = (double *) malloc(num_channels_komondor * sizeof(*total_time_transmitting_per_channel));
	channels_free = (int *) malloc(num_channels_komondor * sizeof(*channels_free));
	channels_for_tx = (int *) malloc(num_channels_komondor * sizeof(*channels_for_tx));
	total_time_lost_per_channel = (double *) malloc(num_channels_komondor * sizeof(*total_time_lost_per_channel));
	for(int i = 0; i < num_channels_komondor; i++){
		channel_power[i] = 0;
		total_time_transmitting_per_channel[i] = 0;
		channels_free[i] = 0;
		channels_for_tx[i] = 0;
		total_time_lost_per_channel[i] = 0;
	}

	total_time_transmitting_in_num_channels = (double *) malloc(num_channels_allowed * sizeof(*total_time_transmitting_in_num_channels));
	total_time_lost_in_num_channels = (double *) malloc(num_channels_allowed * sizeof(*total_time_lost_in_num_channels));
	for(int i = 0; i < num_channels_allowed; i++){
		total_time_transmitting_in_num_channels[i] = 0;
		total_time_lost_in_num_channels[i] = 0;
	}
	// Power received per each node (we assume it is the same during all the transmission)
	power_received_per_node = (double *) malloc(total_nodes_number * sizeof(*power_received_per_node));
	// List of hidden nodes (1 indicates hidden nodes, 0 indicates the opposite)
	hidden_nodes_list = (int *) malloc(total_nodes_number * sizeof(*hidden_nodes_list));
	for(int i = 0; i < total_nodes_number; i++){
		power_received_per_node[i] = 0;
		hidden_nodes_list[i] = 0;
	}

	// 5 types of NACKs TODO constant
	nacks_received = (int *) malloc(5 * sizeof(*nacks_received));
	for(int i = 0; i < 5; i++){
		nacks_received[i] = 0;
	}

	current_destination_id = destination_id;
	pw_received_interest = 0; 	// in pW!!
	progress_bar_delta = 5;	// TODO constant
	progress_bar_counter = 0;
	packets_sent = 0;
	node_state = 0; // Sensing the channel
	throughput = 0;
	current_left_channel =  min_channel_allowed;
	current_right_channel = max_channel_allowed;
	current_tpc = tpc_default;
	current_cca = cca_default;

	node_state = 0;

	data_rate_array[0] = 81.5727 * packet_length * num_packets_aggregated;	// 1 channel
	data_rate_array[1] = 150.8068 * packet_length * num_packets_aggregated; // 2 channels
	data_rate_array[2] = 215.7497 * packet_length * num_packets_aggregated;	// 4 channels
	data_rate_array[3] = 284.1716 * packet_length * num_packets_aggregated; // 8 channels

	packets_lost = 0;
}


/*
 * printChannelPower: prints (or writes) the channel_power array representing the power sensed by the node in each subchannel
 */
void Node :: printChannelPower(int write_or_print){

	switch(write_or_print){
		case PRINT_LOG:{
			printf("channel_power [dBm]: ");
			for(int c = 0; c < num_channels_komondor; c++){
				printf("%f  ", convertPower(PICO_TO_DBM, channel_power[c]));
			}
			printf("\n");
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < num_channels_komondor; c++){
				 if(save_node_logs) fprintf(own_log_file, "%f  ", convertPower(PICO_TO_DBM, channel_power[c]));
			}
			if(save_node_logs)  fprintf(own_log_file, "\n");
			break;
		}
	}
}

/*
 * printChannelForTx: prints (or writes) the channels_for_tx array representing the channels used for TX
 */
void printChannelForTx(int save_node_logs, int print_location, int *channels_for_tx, int num_channels_komondor, FILE *own_log_file){
	if(print_location == 1){
		for(int c = 0; c < num_channels_komondor; c++){
			 if(save_node_logs)  fprintf(own_log_file, "%d  ", channels_for_tx[c]);
			// printf("%d  ", channels_for_tx[c]);
		}
		 if(save_node_logs)  fprintf(own_log_file, "\n");
	} else {
		printf("channels_for_tx: ");
		for(int c = 0; c < num_channels_komondor; c++){
			printf("%d  ", channels_for_tx[c]);
			// printf("%d  ", channels_for_tx[c]);
		}
		printf("\n");
	}
}

/*
 * printNodeStatistics(): prints (or writes) final statistics at the given node
 */
void Node :: printNodeStatistics(int write_or_print){

	// Process statistics
	double packets_lost_percentage = 0;
	if (packets_sent > 0) {
		packets_lost_percentage = double(packets_lost * 100)/double(packets_sent);
	}
	throughput = (((double)(packets_sent-packets_lost) * packet_length * num_packets_aggregated / 1000000))/SimTime();
	int hidden_nodes_number = 0;
	for(int i = 0; i < total_nodes_number; i++){
		if(hidden_nodes_list[i] == 1) hidden_nodes_number++;
	}

	switch(write_or_print){

		case PRINT_LOG:{

			printf("------- N%d ------\n", node_id);

			// Throughput
			printf(" - Throughput = %f Mbps\n", throughput);

			// Packets sent and lost
			printf(" - Packets sent = %d - Packets lost = %d  (%f %% lost)\n",
							packets_sent, packets_lost, packets_lost_percentage);

			// Time EFFECTIVELY transmitting in a given number of channels (no losses)
			printf("    · Time EFFECTIVELY transmitting in N channels: ");
			for(int n = 0; n < num_channels_allowed; n++){
				printf("(%d) %f  ", n+1, total_time_transmitting_in_num_channels[n] - total_time_lost_in_num_channels[n]);
			}
			printf("\n");

			// Time EFFECTIVELY transmitting in each of the channels (no losses)
			printf("    · Time EFFECTIVELY transmitting in each channel: ");
			for(int c = 0; c < num_channels_komondor; c++){
				printf("(#%d) %f ", c, total_time_transmitting_per_channel[c] - total_time_lost_per_channel[c]);
			}
			printf("\n");

			// Time LOST transmitting in a given number of channels
			printf("    · Time LOST transmitting in N channels: ");
			for(int n = 0; n < num_channels_allowed; n++){
				printf("(%d) %f  ", n+1, total_time_lost_in_num_channels[n]);
			}
			printf("\n");

			// Time LOST transmitting in each of the channels
			printf("    · Time LOST transmitting in each channel: ");
			for(int c = 0; c < num_channels_komondor; c++){
				printf("(#%d) %f ", c, total_time_lost_per_channel[c]);
			}
			printf("\n");

			// Hidden nodes
			printf(" - Total hidden nodes: %d\n", hidden_nodes_number);
			printf(" - Hidden nodes list: ");
			for(int i = 0; i < total_nodes_number; i++){
				printf("%d  ", hidden_nodes_list[i]);
			}
			printf("\n\n");
			break;
		}

		case WRITE_LOG:{

			if (save_node_logs){

				// Throughput
				fprintf(own_log_file, "%f;N%d;S%d;%s;    - Throughput = %f Mbps\n",
						SimTime(), node_id, node_state, LOG_Z00, throughput);

				// Packets sent and lost
				fprintf(own_log_file, "%f;N%d;S%d;%s;    - Packets sent: %d\n", SimTime(), node_id, node_state, LOG_Z00, packets_sent);
				fprintf(own_log_file, "%f;N%d;S%d;%s;    - Packets lost: %d\n", SimTime(), node_id, node_state, LOG_Z00, packets_lost);
				fprintf(own_log_file, "%f;N%d;S%d;%s;    - Loss ratio: %f\n", SimTime(), node_id, node_state, LOG_Z00, packets_lost_percentage);

				// Time EFFECTIVELY transmitting in a given number of channels (no losses)
				fprintf(own_log_file, "%f;N%d;S%d;%s;        · Time EFFECTIVELY transmitting in N channels: ",
						SimTime(), node_id, node_state, LOG_Z00);
				for(int n = 0; n < num_channels_allowed; n++){
					fprintf(own_log_file, "(%d) %f  ", n+1, total_time_transmitting_in_num_channels[n] - total_time_lost_in_num_channels[n]);
				}
				fprintf(own_log_file, "\n");

				// Time EFFECTIVELY transmitting in each of the channels (no losses)
				fprintf(own_log_file, "%f;N%d;S%d;%s;        · Time EFFECTIVELY transmitting in each channel: ",
						SimTime(), node_id, node_state, LOG_Z00);
				for(int c = 0; c < num_channels_komondor; c++){
					fprintf(own_log_file, "(#%d) %f ", c, total_time_transmitting_per_channel[c] - total_time_lost_per_channel[c]);
				}
				fprintf(own_log_file, "\n");

				// Time LOST transmitting in a given number of channels
				fprintf(own_log_file, "%f;N%d;S%d;%s;        · Time LOST transmitting in N channels: ",
						SimTime(), node_id, node_state, LOG_Z00);
				for(int n = 0; n < num_channels_allowed; n++){
					fprintf(own_log_file, "(%d) %f  ", n+1, total_time_lost_in_num_channels[n]);
				}
				fprintf(own_log_file, "\n");

				// Time LOST transmitting in each of the channels
				fprintf(own_log_file, "%f;N%d;S%d;%s;        · Time LOST transmitting in each channel: ",
						SimTime(), node_id, node_state, LOG_Z00);
				for(int c = 0; c < num_channels_komondor; c++){
					fprintf(own_log_file, "(#%d) %f ", c, total_time_lost_per_channel[c]);
				}
				fprintf(own_log_file, "\n");

				// Hidden nodes
				int hidden_nodes_number = 0;
				for(int n = 0; n < total_nodes_number; n++){
					if(hidden_nodes_list[n] == 1) hidden_nodes_number++;
				}
				fprintf(own_log_file, "%f;N%d;S%d;%s;    - Total hidden nodes: %d\n",
						SimTime(), node_id, node_state, LOG_Z00, hidden_nodes_number);

				fprintf(own_log_file, "%f;N%d;S%d;%s;- Hidden nodes list: ",
						SimTime(), node_id, node_state, LOG_Z00);
				for(int i = 0; i < total_nodes_number; i++){
					fprintf(own_log_file, "%d  ", hidden_nodes_list[i]);
				}
			}
			break;
		}
	}
}
