/* TODO: DEFINE copyright headers.*/

/* This is just an skecth of what our Komondor headers should look like.
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
 * File description: Node is a component representing any device in the
 * network (i.e., AP, STA, etc.). Node contains all the logic and algorithms
 * for simulating nodes operation.
 *
 * - Bla bla bla...
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "ListOfDefines.h"
#include "structures/Notification.h"
#include "structures/LogicalNack.h"
#include "structures/Wlan.h"
#include "structures/Logger.h"

// Node component: "TypeII" represents components that are aware of the existence of the simulated time.
component Node : public TypeII{

	// Methods
	public:

		// COST
		void Setup();
		void Start();
		void Stop();

		// Generic
		void initializeVariables();
		void restartNode();
		void printNodeInfo(int info_detail_level);
		void writeNodeInfo(Logger node_logger, int info_detail_level, char *header_string);
		void printOrWriteNodeStatistics(int write_or_print);
		void printOrWriteChannelForTx(int write_or_print);
		void printOrWriteChannelPower(int write_or_print);
		void printOrWriteChannelsFree(int write_or_print);
		void printOrWriteNodesTransmitting (int write_or_print);

		// Channel
		double computePowerReceived(double distance, double tx_power, int tx_gain, int rx_gain);
		void updateChannelsPower(Notification notification, int update_type);
		void getChannelOccupancyByCCA();	// depreacated (binary state of channels)
		void getTxChannelsByChannelBonding(int channel_bonding_model, int *channels_free);
		void computeMaxInterference(Notification notification);
		void updateSINR(double pw_received_interest);
		void applyInterferenceModel(Notification notification, int update_type);

		// Packets
		Notification generateNotification(int packet_type, int destination_id, double tx_duration);
		void selectDestination();
		double computeTxTime(int num_channels_used, int packet_size);
		void sendAck();
		void ackTimeout();
		int applyModulationProbabilityError(Notification notification);
		void selectMCSResponse();
		int attemptToDecodePacket();
		void requestMCS();

		// NACK
		int isPacketLost(double channel_power, Notification notification);
		void generateAndSendLogicalNack(int packet_id, int node_id_a, int node_id_b, int reason_id);
		void processNack(LogicalNack logical_nack_info);
		void cleanNack();
		void handlePacketLoss();

		// Backoff
		double computeBackoff();
		void handleBackoff(int pause_or_resume);
		void pauseBackoff();
		void resumeBackoff();
		void handleCW(int mode);

	// Public items (entered by nodes constructor in Komondor simulation)
	public:

		// Specific to a node
		int node_id; 				// Node identifier
		int x;						// X position coordinate
		int y;						// Y position coordinate
		int z;						// Z position coordinate
		char *node_code;			// Name of the Node (only for information displaying purposes)
		int node_type;				// Node type (e.g., AP, STA, ...)
		int destination_id;			// Destination node id (for nodes not belonging to any WLAN)
		double lambda;				// Average notification generation rate [notification/s]
		int primary_channel;		// Primary channel
		int min_channel_allowed;	// Min. allowed channel
		int max_channel_allowed;	// Max. allowed channel
		int num_channels_allowed;	// Maximum number of channels allowed to TX in
		double tpc_min;				// Min. power transmission
		double tpc_default;			// Default power transmission
		double tpc_max;				// Max. power transmission
		double cca_min;				// Min. CCA	("sensitivity" threshold)
		double cca_default;			// Default CCA	("sensitivity" threshold)
		double cca_max;				// Max. CCA ("sensitivity" threshold)
		double tx_gain;				// Antenna transmission gain (dB)
		double rx_gain;				// Antenna reception gain (dB)
		int channel_bonding_model;	// Channel bonding model (definition of models in function getTxChannelsByChannelBonding())
		int modulation_default;		// Default modulation

		// WLAN
		char *wlan_code;			// Code of the WLAN to which the node belongs
		Wlan wlan;					// Wlan to which the node belongs

		/* Same default in every node (parameters from system input and console arguments) */
		// Generic
		double simulation_time_komondor;	// Observation time (time when the simulation stops)
		int total_nodes_number;				// Total number of nodes
		int collisions_model;				// Collisions model (TODO: implement different collision models)
		double capture_effect;				// Threshold from which PER is 10% (otherwise, 100%)
		double constant_PER;				// Constant PER for correct transmissions
		int save_node_logs;					// Flag for activating the log writting of nodes
		int print_node_logs;				// Flag for activating the printing of node logs
		char *simulation_code;				// Simulation code

		// Channel
		int basic_channel_bandwidth;		// Channel unit bandwidth [MHz]
		int num_channels_komondor;			// Number of subchannels composing the whole channel
		int cochannel_model;				// Co-channel interference model (definition of models in function updateChannelsPower())

		// Transmissions
		int default_destination_id;			// Current destination node ID
		double noise_level;					// Environment noise [dBm]
		int current_modulation;				// Current_modulation used by nodes
		int channel_max_intereference;		// Channel of interest suffering maximum interference
		double SIFS;						// SIFS [s]
		double DIFS;						// DIFS [s]
		double central_frequency;			// Central frequency (GHz)
		int CW_min;							// Backoff minimum Contention Window
		int CW_max;							// Backoff maximum Contention Window
		int pdf_backoff;					// Probability distribution type of the backoff (0: exponential, 1: deterministic)
		int path_loss_model;				// Path loss model (0: free-space, 1: Okumura-Hata model - Uban areas)

		// Data rate - modulations
		int modulation_rates[4][12];		// Modulation rates in bps used in IEEE 802.11ax
		int err_prob_modulation[4][12];		// BER associated to each modulation (TO BE FILLED!!)
		int data_rate_array[8];				// Hardcoded data rates [bps] (corresponding to the CTMN Matlab code)
		double coding_rate_modulation[12];  // Coding rate used for each modulation scheme
		int bits_per_symbol_modulation[12]; // Bits per symbol used by each modulation

		// Packets
		int pdf_tx_time;					// Probability distribution type of the transmission time (0: exponential, 1: deterministic)
		int packet_length;					// Notification length [bits]
		int num_packets_aggregated;			// Number of packets aggregated in one transmission
		int ack_length;						// ACK length [bits]

	// Statistics (accessible when simulation finished through Komondor simulation class)
	public:

		int packets_sent;
		double *total_time_transmitting_per_channel;		// Time transmitting per channel;
		double *total_time_transmitting_in_num_channels;	// Time transmitting in (ix 0: 1 channel, ix 1: 2 channels...)
		double *total_time_lost_per_channel;				// Time transmitting per channel unsuccessfully;
		double *total_time_lost_in_num_channels;			// Time transmitting in (ix 0: 1 channel, ix 1: 2 channels...) unsuccessfully
		double throughput;									// Throughput [Mbps]
		double throughput_loss;								// Throughput of lost packets [Mbps]
		int packets_lost;									// Own packets that have been collided or lost
		int *nacks_received;								// Counter of the type of Nacks received
		int num_tx_init_not_possible;						// Number of TX initiations that have been not possible due to channel state and DCB model

	// Private items (just for node operation)
	private:

		// Komondor environment
		double *channel_power;				// Channel power detected in each sub-channel [pW] (pico watts for resolution issues)
		int *channels_free;					// Channels that are found free for the beginning TX
		int *channels_for_tx;				// Channels that are used in the beginning TX (depend on the channel bonding model)

		// File for writting node logs
		FILE *output_log_file;				// File for logs in which the node is involved
		char own_file_path[32];				// Name of the file for node logs
		Logger node_logger;					// struct containing the attributes needed for writting logs in a file
		char *header_string;				// Header string for the logger

		// State and timers
		int node_state;						// Node's internal state (0: sensing the channel, 1: transmitting, 2: receiving notification)
		double remaining_backoff;			// Remaining backoff
		int progress_bar_counter;			// Counter for displaying the progress bar

		// Transmission parameters
		int node_is_transmitter;			// Flag for determining if node is able to tranmsit packet (e.g., AP in downlink)
		int current_left_channel;			// Left channel used in current TX
		int current_right_channel;			// Right channel used in current TX
		double current_tpc;					// Transmission power used in current TX [dBm]
		double current_cca;					// Current CCA (variable "sensitivity")	[dBm]
		int current_destination_id;			// Current destination node ID
		double current_tx_duration;			// Duration of the TX being done [s]
		int packet_id;						// Notification ID
		Notification ack;					// ACK to be filled before sending it
		Notification ongoing_notification; 	// Current notification (DATA or ACK) being received
		double current_sinr;				// SINR perceived in current TX [no unit] (linear)
		int default_modulation;				// Default MCS identifier
		double current_data_rate;			// Data rate being used currently
		int current_CW;						// Congestion Window being used currently

		int **mcs_per_node;			// Modulation selected for each of the nodes (only transmitting nodes)
		int *change_modulation_flag;		// Flag for changig the MCS of any of the potential receivers
		int *mcs_response;					// MCS response received from receiver

		// Sensing and Reception parameters
		LogicalNack nack;					// NACK to be filled in case node is the destination of tx loss
		double max_pw_interference;			// Maximum interference detected in range of interest [pW]
		int channel_max_interference;		// Channel of maximum interference detected in range of interest
		int *nodes_transmitting;			// IDs of the nodes which are transmitting to any destination
		double *power_received_per_node;	// Power received from each node in the network [pW]
		double pw_received_interest;		// Power received from a TX destined to the node [pW]
		int receiving_from_node_id;			// ID of the node that is transmitting to the node (-1 if node is not receiing)
		int receiving_packet_id;			// ID of the notification that is being transmitted to me
		int *hidden_nodes_list;				// Store nodes that for sure are hidden (1 indicates that node "i" is hidden)
		int *potential_hidden_nodes;		// Maintain a list of the times a node participated in a collision by hidden node
		int collisions_by_hidden_node; 		// Number of noticed collisions by hidden node (maintained by the transmitter)
		double BER;							// Bit error rate (deprecated)
		double PER;							// Packet error rate (deprecated)

	// Connections and timers
	public:

		// INPORT connections for receiving notifications
		inport void inline inportSomeNodeStartTX(Notification &notification);
		inport void inline inportSomeNodeFinishTX(Notification &notification);
		inport void inline inportNackReceived(LogicalNack &logical_nack_info);

		inport void inline inportMCSRequestReceived(Notification &notification);
		inport void inline inportMCSResponseReceived(Notification &notification);

		// OUTPORT connections for sending notifications
		outport void outportSelfStartTX(Notification &notification);
		outport void outportSelfFinishTX(Notification &notification);
		outport void outportSendLogicalNack(LogicalNack &logical_nack_info);

		outport void outportAskForTxModulation(Notification &notification);
		outport void outportAnswerTxModulation(Notification &notification);

		// Triggers
		Timer <trigger_t> trigger_backoff; 		// Duration of current trigger_backoff. Triggers outportSelfStartTX()
		Timer <trigger_t> trigger_toFinishTX; 	// Duration of current notification transmission. Triggers outportSelfFinishTX()
		Timer <trigger_t> trigger_sim_time;		// Timer for displaying the exectuion time status (progress bar)
		Timer <trigger_t> trigger_DIFS;			// Timer for the DIFS
		Timer <trigger_t> trigger_SIFS;			// Timer for the SIFS
		Timer <trigger_t> trigger_ACK_timeout;	// Determines whether a whole transmission has been lost

		// Every time the timer expires execute this
		inport inline void endBackoff(trigger_t& t1);
		inport inline void myTXFinished(trigger_t& t1);
		inport inline void printProgressBar(trigger_t& t1);
		inport inline void resumeBackoff(trigger_t& t1);
		inport inline void sendAck(trigger_t& t1);
		inport inline void ackTimeout(trigger_t& t1);

		// Connect timers to methods
		Node () {
			connect trigger_backoff.to_component,endBackoff;
			connect trigger_toFinishTX.to_component,myTXFinished;
			connect trigger_sim_time.to_component,printProgressBar;
			connect trigger_DIFS.to_component,resumeBackoff;
			connect trigger_SIFS.to_component,sendAck;
			connect trigger_ACK_timeout.to_component,ackTimeout;
		}
};

/*
 * Setup()
 */
void Node :: Setup(){
	// Do nothing
};

/*
 * Start()
 */
void Node :: Start(){

	if(print_node_logs) printf("%s(N%d) Start\n", node_code, node_id);

	// Create node logs file if required
	if(save_node_logs) {
		// Name node log file accordingly to the node_id
		sprintf(own_file_path,"%s_%s_N%d_%s.txt","./output/logs_output_", simulation_code, node_id, node_code);
		remove(own_file_path);
		output_log_file = fopen(own_file_path, "at");
		node_logger.save_logs = save_node_logs;
		node_logger.file = output_log_file;
	}

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Start()\n",
			SimTime(), node_id, STATE_UNKNOWN, LOG_B00, LOG_LVL1);

	// Write node info
	header_string = (char *) malloc(LOG_HEADER_NODE_SIZE);
	sprintf(header_string, "%f;N%d;S%d;%s;%s", SimTime(), node_id, node_state, LOG_Z00, LOG_LVL3);
	if(save_node_logs) writeNodeInfo(node_logger, INFO_DETAIL_LEVEL_2, header_string);

	// Initialize variables
	initializeVariables();

	// Start backoff procedure only if node is able to tranmsit
	if(node_is_transmitter) {
		remaining_backoff = computeBackoff();
		trigger_backoff.Set(SimTime() + remaining_backoff);
	} else {
		current_destination_id = wlan.ap_id;	// TODO: for uplink traffic. Set STAs destination to the GW
	}

	// Progress bar (trick: it is only printed by node with id 0)
	if(PROGRESS_BAR_DISPLAY){
		if(node_id == 0){
			if(print_node_logs) printf("%s PROGRESS BAR:\n", LOG_LVL1);
			trigger_sim_time.Set(SimTime() + MIN_VALUE_C_LANGUAGE);
		}
	}
	// if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Start() END\n", SimTime(), node_id, node_state, LOG_B01, LOG_LVL1);
};

/*
 * Stop()
 */
void Node :: Stop(){

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Node Stop()\n",
			SimTime(), node_id, node_state, LOG_C00, LOG_LVL1);

	// Print and write node statistics if required
	printOrWriteNodeStatistics(PRINT_LOG);
	printOrWriteNodeStatistics(WRITE_LOG);

	// Close node logs file
	if(save_node_logs) fclose(node_logger.file);

	// if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Node info:\n", SimTime(), node_id, node_state, LOG_C01, LOG_LVL1);
};

/*
 * inportSomeNodeStartTX(): called when some node (this one included) starts a TX
 * Input arguments:
 * - notification: notification containing the information of the transmission start perceived
 */
void Node :: inportSomeNodeStartTX(Notification &notification){

	if(save_node_logs) fprintf(node_logger.file,
			"%f;N%d;S%d;%s;%s inportSomeNodeStartTX(): N%d to N%d\n",
			SimTime(), node_id, node_state, LOG_D00, LOG_LVL1, notification.source_id, notification.tx_info.destination_id);

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s List of nodes transmitting: ",
			SimTime(), node_id, node_state, LOG_E00, LOG_LVL2);

	// Identify node that has started the transmission as transmitting node in the array
	nodes_transmitting[notification.source_id] = TRUE;
	if(save_node_logs) printOrWriteNodesTransmitting(WRITE_LOG);


	if(notification.source_id == node_id){ // If OWN NODE IS THE TRANSMITTER, do nothing

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s I have started a TX of packet #%d (type %d) to N%d in channels %d - %d\n",
				SimTime(), node_id, node_state, LOG_D02, LOG_LVL2, notification.tx_info.packet_id,
				notification.packet_type, notification.tx_info.destination_id,
				notification.left_channel, notification.right_channel);

		if(save_node_logs) fprintf(node_logger.file,
						"%f;N%d;S%d;%s;%s DIFS: active = %d, t_DIFS = %f - backoff: active = %d - t_back = %f\n",
						SimTime(), node_id, node_state, LOG_D02, LOG_LVL3,
						trigger_DIFS.Active(), trigger_DIFS.GetTime() - SimTime(),
						trigger_backoff.Active(), trigger_backoff.GetTime() - SimTime());


	} else {	// If OTHER NODE IS THE TRANSMITTER

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s N%d has started a TX of packet #%d (type %d) to N%d in channels %d - %d\n",
				SimTime(), node_id, node_state, LOG_D02, LOG_LVL2, notification.source_id,
				notification.tx_info.packet_id,	notification.packet_type, notification.tx_info.destination_id,
				notification.left_channel, notification.right_channel);

		// Update the power sensed at each channel
		updateChannelsPower(notification, TX_INITIATED);

		// Compute max interference (the highest one perceived in the reception channel range)
		computeMaxInterference(notification);

		// Decide action according to current state and Notification initiated

		int loss_reason;	// Packet loss reason (if any)

		switch(node_state){

			/* STATE_SENSING:
			 * - If node IS destination and notification is data packet:
			 *   * check if packet can be received
			 *      - If packet lost: generateNack and handle backoff
			 *      - If packet can be received: state = RECEIVE_DATA and pause backoff
			 * - If node IS NOT destination: handle backoff
			 */
			case STATE_SENSING:{

				if(notification.tx_info.destination_id == node_id){	// Node IS THE DESTINATION

					// Save ongoing notification information
					ongoing_notification = notification;

					// Update power received of interest
					pw_received_interest = power_received_per_node[notification.source_id];

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
							SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, notification.tx_info.destination_id);

					if(notification.packet_type == PACKET_TYPE_DATA){	// Notification COTAINS a DATA PACKET

						// Check if notification has been lost due to interferences or weak signal strength
						loss_reason = isPacketLost(channel_power[primary_channel], notification);

						if(loss_reason != PACKET_NOT_LOST) {	// If data packet IS LOST, send logical Nack

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
									SimTime(), node_id, node_state, LOG_D15, LOG_LVL4, notification.tx_info.packet_id,
									notification.source_id, loss_reason);

							// Send logical NACK to transmitter sending incoming notification for indicating the loss reason
							generateAndSendLogicalNack(notification.tx_info.packet_id, notification.source_id, NODE_ID_NONE, loss_reason);

							// Check if node has to freeze the BO (if it is not already frozen)
							if (node_is_transmitter) handleBackoff(PAUSE_TIMER);

						} else {	// Data packet IS NOT LOST (it can be properly received)

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s Reception of packet %d from N%d CAN be started\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.tx_info.packet_id,
									notification.source_id);

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s current_sinr = %f dB (%f)\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL5, convertPower(LINEAR_TO_DB,current_sinr),
									current_sinr);

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s capacity = %f Mbps\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL5, 20*log10(1+current_sinr)/log10(2));

							// Change state and update receiving info
							node_state = STATE_RX_DATA;
							receiving_from_node_id = notification.source_id;
							receiving_packet_id = notification.tx_info.packet_id;

							// Pause backoff as node has began a reception
							if(node_is_transmitter) pauseBackoff();

						}

					} else {	//	Notification does NOT CONTAIN a DATA PACKET
						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s Unexpected packet type (%d) received!\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL4, notification.packet_type);
					}

				} else {	// Node IS NOT THE DESTINATION

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL3, notification.tx_info.destination_id);

					// Check if node has to freeze the BO (if it is not already frozen)
					if (node_is_transmitter) handleBackoff(PAUSE_TIMER);
				}

				break;
			}


			/* STATE_TX_DATA || STATE_TX_ACK:
			 * - If node IS destination: generateNack to incoming notification transmitter
			 * - If node IS NOT destiatnion: do nothing
			 */
			case STATE_TX_DATA:
			case STATE_TX_ACK:{

				if(notification.tx_info.destination_id == node_id){ // Node IS THE DESTINATION

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
							SimTime(), node_id, node_state, LOG_D07, LOG_LVL3,
							notification.tx_info.destination_id);

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s I am transmitting, packet cannot be received\n",
							SimTime(), node_id, node_state, LOG_D18, LOG_LVL3);

					// Send logical NACK to incoming notification transmitter due to receiver (node) was already receiving
					generateAndSendLogicalNack(notification.tx_info.packet_id, notification.source_id,
							NODE_ID_NONE, PACKET_LOST_DESTINATION_TX);

				} else {	// Node IS NOT THE DESTINATION, do nothing

					if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL3, notification.tx_info.destination_id);

				}
				break;
			}

			/* STATE_RX_DATA || STATE_RX_ACK:
			 * - If node IS destination: generateNack to incoming notification transmitter
			 * - Check if ongoing packet reception is lost:
			 *   * If packet lost: generate Nack to source of ongoing reception and restart node
			 *   * If packet NOT lost: do nothing
			 */
			case STATE_RX_DATA:
			case STATE_RX_ACK:{

				if(notification.tx_info.destination_id == node_id){	// Node IS THE DESTINATION

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
							SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, notification.tx_info.destination_id);

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s Power in channel #%d: channel_power = %f dBm (%f pW) - pw_received_interest = %f dBm (%f pW)"
							"- max_pw_interference = %f dBm (%f pW)\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL5, channel_max_intereference,
							convertPower(PICO_TO_DBM, channel_power[channel_max_intereference]), channel_power[channel_max_intereference],
							convertPower(PICO_TO_DBM, power_received_per_node[receiving_from_node_id]),
							power_received_per_node[receiving_from_node_id], convertPower(PICO_TO_DBM, max_pw_interference), max_pw_interference);


					// Check if ongoing notification has been lost due to interferences caused by new transmission
					loss_reason = isPacketLost(channel_power[primary_channel], ongoing_notification);

					if(loss_reason != PACKET_NOT_LOST) {	// If ongoing data packet IS LOST

						// Pure collision (two nodes transmitting to me with enough power)
						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s Pure collision! Already receiving from N%d\n",
								SimTime(), node_id, node_state, LOG_D19, LOG_LVL4, receiving_from_node_id);

						// Send NACK to both ongoing transmitter and incoming interferer nodes
						generateAndSendLogicalNack(ongoing_notification.tx_info.packet_id, ongoing_notification.source_id,
								notification.source_id, PACKET_LOST_PURE_COLLISION);

						restartNode();

					} else {	// If ongoing data packet IS NOT LOST (incoming transmission does not affect ongoign reception)

						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s Low strength signal received while already receiving from N%d\n",
							SimTime(), node_id, node_state, LOG_D20, LOG_LVL4, receiving_from_node_id);

						// Send logical NACK to incoming transmitter indicating that node is already receiving
						generateAndSendLogicalNack(notification.tx_info.packet_id, receiving_from_node_id,
								notification.source_id, PACKET_LOST_LOW_SIGNAL_AND_RX);

					}

				} else {	// Node is NOT THE DESTINATION

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL3, notification.tx_info.destination_id);

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s Power in channel #%d: channel_power = %f dBm (%f pW) - pw_received_interest = %f dBm (%f pW)"
							"- max_pw_interference = %f dBm (%f pW)\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL5, channel_max_intereference,
							convertPower(PICO_TO_DBM, channel_power[channel_max_intereference]), channel_power[channel_max_intereference],
							convertPower(PICO_TO_DBM, power_received_per_node[receiving_from_node_id]),
							power_received_per_node[receiving_from_node_id], convertPower(PICO_TO_DBM, max_pw_interference), max_pw_interference);

					// Check if the ongoing reception is affected
					loss_reason = isPacketLost(channel_power[primary_channel], ongoing_notification);

					if(loss_reason != PACKET_NOT_LOST) { 	// If ongoing packet reception IS LOST

						// Collision by hidden node
						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s Collision by Hidden node! Receiving from N%d and N%d appeared\n",
								SimTime(), node_id, node_state, LOG_D19, LOG_LVL4, receiving_from_node_id, notification.source_id);

						// Send logical NACK to ongoing transmitter
						generateAndSendLogicalNack(ongoing_notification.tx_info.packet_id, receiving_from_node_id,
								NODE_ID_NONE, PACKET_LOST_INTERFERENCE);

						restartNode();
					}
				}
				break;
			}


			/* STATE_WAIT_ACK:
			 * - If node IS destination and packet is ACK:
			 *   * Check if ACK packet can be received:
			 *     - If packet lost: generate Nack to source of ongoing ACK reception and do nothing until ACK timeout expires
			 *     - If packet NOT lost: cancel ACK timeout and state = STATE_RX_ACK
			 * - If node IS NOT destination: do nothing
			 */
			case STATE_WAIT_ACK:{

				if(notification.tx_info.destination_id == node_id){	// Node is the destination

					pw_received_interest = power_received_per_node[notification.source_id];
					ongoing_notification = notification;

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
							SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, notification.tx_info.destination_id);

					if(notification.packet_type == PACKET_TYPE_ACK){	// ACK packet transmission started

						// Check if notification has been lost due to interferences or weak signal strength
						loss_reason = isPacketLost(channel_power[primary_channel], notification);

						if(loss_reason != PACKET_NOT_LOST) {	// If ACK packet IS LOST, send logical Nack

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
									SimTime(), node_id, node_state, LOG_D15, LOG_LVL4, notification.tx_info.packet_id,
									notification.source_id, loss_reason);

							// Send logical NACK to ACK transmitter
							generateAndSendLogicalNack(notification.tx_info.packet_id, notification.source_id,
									NODE_ID_NONE, loss_reason);

							// Do nothing until ACK timeout is triggered

						} else {	// If ACK packet IS NOT LOST (it can be properly received)

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s Reception of ACK %d from N%d CAN be started\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.tx_info.packet_id, notification.source_id);

							// Cancel ACK timeout and go to STATE_RX_ACK while updating receiving info
							trigger_ACK_timeout.Cancel();
							node_state = STATE_RX_ACK;
							receiving_from_node_id = notification.source_id;
							receiving_packet_id = notification.tx_info.packet_id;

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.tx_info.destination_id);

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s current_sinr = %f dB (%f)\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL5, convertPower(LINEAR_TO_DB,current_sinr),
									current_sinr);

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s capacity = %f Mbps\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL5, 20*log10(1+current_sinr)/log10(2));

						}

					}  else {	//	Some packet type received that is not ACK
						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s Unexpected packet type received!\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION, do nothing

					if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL3, notification.tx_info.destination_id);

				}
				break;
			}

			default:{
				printf("ERROR: %d is not a correct state\n", node_state);
				exit(EXIT_FAILURE);
				break;
			}
		}
	}
	// if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s inportSomeNodeStartTX() END\n", SimTime(), node_id, node_state, LOG_D01, LOG_LVL1);
};

/*
 * inportSomeNodeFinishTX(): called when some node finishes a packet TX (data or ACK)
 * Input arguments:
 * - notification: notification containing the information of the transmission that has finished
 */
void Node :: inportSomeNodeFinishTX(Notification &notification){

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s inportSomeNodeFinishTX(): N%d to N%d\n",
		SimTime(), node_id, node_state, LOG_E00, LOG_LVL1, notification.source_id, notification.tx_info.destination_id);

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s List of nodes transmitting: ",
			SimTime(), node_id, node_state, LOG_E00, LOG_LVL2);

	// Identify node that has finished the transmission as non-transmitting node in the array
	nodes_transmitting[notification.source_id] = FALSE;
	if(save_node_logs) printOrWriteNodesTransmitting(WRITE_LOG);

	if(notification.source_id == node_id){	// Node is the TX source: do nothing

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s I have finished the TX of packet #%d (type %d) in channel range: %d - %d\n",
				SimTime(), node_id, node_state, LOG_E18, LOG_LVL2, notification.tx_info.packet_id,
				notification.packet_type, notification.left_channel, notification.right_channel);

	} else {	// Node is not the TX source

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s N%d has finished the TX of packet #%d (type %d) in channel range: %d - %d\n",
				SimTime(), node_id, node_state, LOG_E18, LOG_LVL2, notification.source_id,
				notification.tx_info.packet_id, notification.packet_type, notification.left_channel,
				notification.right_channel);

		// Update power sensed in each channel
		updateChannelsPower(notification, TX_FINISHED);

		switch(node_state){

			/* STATE_SENSING:
			 * - handle backoff
			 */
			case STATE_SENSING:{	// Do backoff process

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Attempting to restart backoff.\n",
						SimTime(), node_id, node_state, LOG_E11, LOG_LVL3);

				if (node_is_transmitter) handleBackoff(RESUME_TIMER);	// Attempt to resume Backoff

				break;
			}

			/* STATE_TX_DATA || STATE_TX_ACK:
			 * - Do nothing
			 */
			case STATE_TX_DATA:
			case STATE_TX_ACK:{	// Do nothing

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s I am currently transmitting ---> Do nothing.\n",
						SimTime(), node_id, node_state, LOG_E13, LOG_LVL3);
				break;
			}

			/* STATE_RX_DATA:
			 * - If node IS the destination and data packet transmission finished:
			 *   * start SIFS and generate ACK
			 *   * state = STATE_TX_ACK
			 * - If node IS NOT the destination: do nothing
			 */
			case STATE_RX_DATA:{	// Check if the current reception is finished

				if(notification.tx_info.destination_id == node_id){ 	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_DATA){	// Data packet transmission finished

						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s Packet %d reception from N%d is finished successfully.\n",
								SimTime(), node_id, node_state, LOG_E14, LOG_LVL3, notification.tx_info.packet_id,
								notification.source_id);

						// Generate and send ACK to transmitter after SIFS
						current_left_channel = notification.left_channel;
						current_right_channel = notification.right_channel;
						int num_channels_tx = current_right_channel - current_left_channel + 1;
						node_state = STATE_TX_ACK;
						current_tx_duration = computeTxTime(num_channels_tx, ack_length);
						current_destination_id = notification.source_id;
						ack = generateNotification(PACKET_TYPE_ACK, current_destination_id, current_tx_duration);
						trigger_SIFS.Set(SimTime() + SIFS); // triggers the sendAck() function after SIFS

					} else {	// Other packet type transmission finished
						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION, do nothing

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s Still noticing a packet transmission (#%d) from N%d.\n",
							SimTime(), node_id, node_state, LOG_E15, LOG_LVL3, notification.tx_info.packet_id,
							notification.source_id);

				}
				break;
			}

			/* STATE_RX_ACK:
			 * - If node IS the destination and ACK packet transmission finished:
			 *   * decrease congestion window and restart node
			 *   * state = STATE_SENSING (implicit on restart)
			 * - If node IS NOT the destination: do nothing
			 */
			case STATE_RX_ACK:{	// Check if the current reception is finished

				if(notification.tx_info.destination_id == node_id){	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_ACK){	// ACK packet transmission finished

						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s ACK %d reception from N%d is finished successfully.\n",
								SimTime(), node_id, node_state, LOG_E14, LOG_LVL3, notification.tx_info.packet_id,
								notification.source_id);

						current_tx_duration += (notification.tx_info.tx_duration + SIFS);	// Add ACK time to tx_duration

						// Transmission succeeded ---> decrease congestion window
						handleCW(DECREASE_CW);

						// Restart node (implicitely to STATE_SENSING)
						restartNode();

					} else {	// Other packet type transmission finished
						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION

					if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Still receiving packet %d reception from N%d.\n",
							SimTime(), node_id, node_state, LOG_E15, LOG_LVL3, notification.tx_info.packet_id,
							notification.source_id);
				}

				break;
			}

			/* STATE_WAIT_ACK:
			 * - Do nothing
			 */
			case STATE_WAIT_ACK:{
				// Do nothing
				break;
			}

			default:{
				printf("ERROR: %d is not a correct state\n", node_state);
				exit(EXIT_FAILURE);
				break;
			}
		}
	}
	// if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s inportSomeNodeFinishTX() END",	SimTime(), node_id, node_state, LOG_E01, LOG_LVL1);
};

/*
 * inportNackReceived(): called when some node sends a logical NACK
 * Input arguments:
 * - logical_nack_info: NACK information
 */
void Node :: inportNackReceived(LogicalNack &logical_nack_info){

	if(save_node_logs) fprintf(node_logger.file,
			"%f;N%d;S%d;%s;%s inportNackReceived(): N%d to N%d (A) and N%d (B)\n",
			SimTime(), node_id, node_state, LOG_H00, LOG_LVL1, logical_nack_info.source_id,
			logical_nack_info.node_id_a, logical_nack_info.node_id_b);

	if(logical_nack_info.source_id != node_id){	// Node is not the NACK transmitter, process NACK

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s NACK of packet #%d received from N%d sent to a) N%d and b) N%d with reason %d\n",
				SimTime(), node_id, node_state, LOG_H00, LOG_LVL2, logical_nack_info.packet_id, logical_nack_info.source_id,
				logical_nack_info.node_id_a, logical_nack_info.node_id_b, logical_nack_info.loss_reason);

		// Process logical NACK for statistics purposes
		processNack(logical_nack_info);

	} else {	// Node is the NACK transmitter, do nothing

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s NACK of packet #%d sent to a) N%d and b) N%d with reason %d\n",
				SimTime(), node_id, node_state, LOG_H00, LOG_LVL2, logical_nack_info.packet_id,
				logical_nack_info.node_id_a, logical_nack_info.node_id_b, logical_nack_info.loss_reason);

	}

	// if(save_node_logs) fprintf(node_logger.file, "%f;N%d;G01;%s inportNackReceived() END\n", SimTime(), node_id, LOG_LVL1);
}

/*
 * inportMCSRequestReceived(): called when some node asks (logically) the receiver for selectable MCS configurations
 * Input arguments:
 * - notification: notification containing the MCS request
 */
void Node :: inportMCSRequestReceived(Notification &notification){

	if(notification.tx_info.destination_id == node_id) {	// If node IS THE DESTINATION

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s MCS request received from %d\n",
				SimTime(), node_id, node_state, LOG_F00, LOG_LVL1, notification.source_id);

		// Compute distance and power received from transmitter
		double distance = computeDistance(x, y, z, notification.tx_info.x, notification.tx_info.y, notification.tx_info.z);
		double pw_received_interest_dBm = computePowerReceived(distance, notification.tx_info.tx_power, tx_gain, rx_gain);
		pw_received_interest = convertPower(DBM_TO_PICO,pw_received_interest_dBm);

		// Select the modulation according to the SINR perceived corresponding to incoming transmitter
		selectMCSResponse();

		// Fill and send MCS response
		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Answering MCS\n",
								SimTime(), node_id, node_state, LOG_F00, LOG_LVL1);

		Notification response_mcs  = generateNotification(PACKET_TYPE_MCS_RESPONSE, notification.source_id, TX_DURATION_NONE);

		outportAnswerTxModulation(response_mcs);

	} else { 	// If node IS NOT THE DESTINATION
		// Do nothing
	}
}

/*
 * inportMCSResponseReceived(): called when some node answers back to a MCS request
 * Input arguments:
 * - notification: notification containing the MCS response
 */
void Node :: inportMCSResponseReceived(Notification &notification){

	if(notification.tx_info.destination_id == node_id) {	// If node IS THE DESTINATION


		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s inportMCSResponseReceived()\n",
				SimTime(), node_id, node_state, LOG_F00, LOG_LVL1);

		int ix_aux = current_destination_id - wlan.list_sta_id[0];	// Auxiliary index for correcting the node id offset

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s MCS per number of channels: ",
				SimTime(), node_id, node_state, LOG_F00, LOG_LVL2);

		// Set receiver modulation to the received one
		for (int i = 0; i < NUM_OPTIONS_CHANNEL_LENGTH; i++){

			mcs_per_node[ix_aux][i] = notification.tx_info.modulation_schemes[i];

			if(save_node_logs) fprintf(node_logger.file, "%d ", mcs_per_node[ix_aux][i]);
		}

		if(save_node_logs) fprintf(node_logger.file, "\n");

		// TODO: ADD LOGIC TO HANDLE WRONG SITUATIONS (cannot transmit over none of the channel combinations)
		if(mcs_per_node[ix_aux][0] == -1) {

			// CANNOT TX EVEN FOR 1 CHANNEL
			if(current_tpc < MAX_POWER) {

				current_tpc ++;
				change_modulation_flag[ix_aux] = TRUE;

			} else {

				// NODE UNREACHABLE
				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Unreachable node: transmissions to N%d are cancelled\n",
						SimTime(), node_id, node_state, LOG_G00, LOG_LVL3, current_destination_id);
				// TODO: unreachable_nodes[current_destination_id] = TRUE;
			}
		}

	} else {	// If node IS NOT THE DESTINATION
		// Do nothing
	}
}

/*
 * selectMCSResponse(): select the proper MCS of transmitter per number of channels
 */
void Node :: selectMCSResponse(){

	double pw_received_interest_dBm = convertPower(PICO_TO_DBM, pw_received_interest);

	for (int ch_num_ix = 0; ch_num_ix < 4; ch_num_ix ++){	// For 1, 2, 4 and 8 channels

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Power received of interest (%d channels) = %f dBm\n",
						SimTime(), node_id, node_state, LOG_F00, LOG_LVL4, ch_num_ix, pw_received_interest_dBm);

		if(pw_received_interest_dBm < -82 +(ch_num_ix*3)){ mcs_response[ch_num_ix] = MODULATION_FORBIDDEN; }
		else if (pw_received_interest_dBm >= -82 + (ch_num_ix*3) && pw_received_interest_dBm < -79 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_BPSK_1_2;}
		else if (pw_received_interest_dBm >= -79 + (ch_num_ix*3) && pw_received_interest_dBm < -77 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_QPSK_1_2;}
		else if (pw_received_interest_dBm >= -77 + (ch_num_ix*3) && pw_received_interest_dBm < -74 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_QPSK_3_4;}
		else if (pw_received_interest_dBm >= -74 + (ch_num_ix*3) && pw_received_interest_dBm < -70 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_16QAM_1_2;}
		else if (pw_received_interest_dBm >= -70 + (ch_num_ix*3) && pw_received_interest_dBm < -66 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_16QAM_3_4;}
		else if (pw_received_interest_dBm >= -66 + (ch_num_ix*3) && pw_received_interest_dBm < -65 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_64QAM_2_3;}
		else if (pw_received_interest_dBm >= -65 + (ch_num_ix*3) && pw_received_interest_dBm < -64 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_64QAM_3_4;}
		else if (pw_received_interest_dBm >= -64 + (ch_num_ix*3) && pw_received_interest_dBm < -59 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_64QAM_5_6;}
		else if (pw_received_interest_dBm >= -59 + (ch_num_ix*3) && pw_received_interest_dBm < -57 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_256QAM_3_4;}
		else if (pw_received_interest_dBm >= -57 + (ch_num_ix*3) && pw_received_interest_dBm < -54 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_256QAM_5_6;}
		else if (pw_received_interest_dBm >= -54 + (ch_num_ix*3) && pw_received_interest_dBm < -52 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_1024QAM_3_4;}
		else { mcs_response[ch_num_ix] = MODULATION_1024QAM_5_6;}

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s mcs_response[%d] = %d\n",
				SimTime(), node_id, node_state, LOG_F00, LOG_LVL5, ch_num_ix, mcs_response[ch_num_ix]);

		pw_received_interest_dBm = pw_received_interest_dBm - 3;	// Power received is half per channel if range is doubled

	}

}

/*
 * endBackoff(): called when backoff finishes
 * Input arguments:
 * - trigger_t: backoff trigger
 */
void Node :: endBackoff(trigger_t &){

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s endBackoff()\n",
			SimTime(), node_id, node_state, LOG_F00, LOG_LVL1);

	// Check if MCS already defined for every potential receiver
	for(int n = 0; n < wlan.num_stas; n++) {
		current_destination_id = wlan.list_sta_id[n];
		// Receive the possible MCS to be used for each number of channels
		if (change_modulation_flag[n]) requestMCS();
	}

	// Pick one receiver from the pool of potential receivers
	selectDestination();

	printOrWriteChannelPower(WRITE_LOG);

	// (DEPRECATED) Just an informative representation of channels with higher power than CCA
	getChannelOccupancyByCCA();
	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Channels free: ",
			SimTime(), node_id, node_state, LOG_F02, LOG_LVL2);
	printOrWriteChannelsFree(WRITE_LOG);

	// Identify the channel range to TX in depending on the channel bonding scheme and power sensed
	getTxChannelsByChannelBonding(channel_bonding_model, channels_free);
	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Channels for transmitting: ",
			SimTime(), node_id, node_state, LOG_F02, LOG_LVL2);
	printOrWriteChannelForTx(WRITE_LOG);

	if(channels_for_tx[0] != TX_NOT_POSSIBLE){	// Transmission IS POSSIBLE

		// Change state
		node_state = STATE_TX_DATA;

		// Generate Notification
		current_left_channel = getBoundaryChannel(FIRST_ONE_IN_ARRAY, channels_for_tx,  num_channels_komondor);
		current_right_channel = getBoundaryChannel(LAST_ONE_IN_ARRAY, channels_for_tx, num_channels_komondor);
		int num_channels_tx = current_right_channel - current_left_channel + 1;

		current_tx_duration = computeTxTime(num_channels_tx, packet_length * num_packets_aggregated);

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s Transmission is possible in range: %d - %d\n",
				SimTime(), node_id, node_state, LOG_F04, LOG_LVL3, current_left_channel, current_right_channel);

		Notification notification = generateNotification(PACKET_TYPE_DATA, current_destination_id, current_tx_duration);

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s Transmission of data packet #%d started\n",
				SimTime(), node_id, node_state, LOG_F04, LOG_LVL3, packet_id);

		// Send data packet notification and trigger to finsih transmission
		outportSelfStartTX(notification);
		trigger_toFinishTX.Set(SimTime() + current_tx_duration);

		// Sum sent packet for statistics purposes
		packets_sent++;

		trigger_DIFS.Cancel();	// Safety instruction

	} else {	// Transmission IS NOT, compute a new backoff.

		// Remain on STATE_SENSING (redundant)
		node_state = STATE_SENSING;

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Transmission is NOT possible\n",
				SimTime(), node_id, node_state, LOG_F03, LOG_LVL3);

		// Compute a new backoff and trigger a new DIFS
		remaining_backoff = computeBackoff();
		trigger_DIFS.Set(SimTime() + DIFS);

	}

	// if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s endBackoff() END\n", SimTime(), node_id, node_state, LOG_F01, LOG_LVL1);
};

/*
 * myTXFinished(): called when own transmission is finished
 * Input arguments:
 * - trigger_t: transmission trigger
 */
void Node :: myTXFinished(trigger_t &){

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s myTXFinished()\n",
			SimTime(), node_id, node_state, LOG_G00, LOG_LVL1);

	switch(node_state){

		case STATE_TX_DATA:{ 	// Change state to STATE_WAIT_ACK

			Notification notification = generateNotification(PACKET_TYPE_DATA, current_destination_id, TX_DURATION_NONE);
			outportSelfFinishTX(notification);

			// Set ACK timeout and change state to STATE_WAIT_ACK
			trigger_ACK_timeout.Set(SimTime() + (1 + MIN_VALUE_C_LANGUAGE) * SIFS);
			node_state = STATE_WAIT_ACK;

			break;
		}

		case STATE_TX_ACK:{		// Restart node

			Notification notification = generateNotification(PACKET_TYPE_ACK, current_destination_id, TX_DURATION_NONE);
			outportSelfFinishTX(notification);

			restartNode();

			break;
		}

		default:
			break;
	}

	// if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;  myTXFinished()\n", SimTime(), node_id, node_state, LOG_G01, LOG_LVL1);
};

/*
 * requestMCS(): performs a negotiation of the MCS to be used according to the tx power sensed by the receiver
 */
void Node :: requestMCS(){

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s requestMCS()\n",
				SimTime(), node_id, node_state, LOG_G00, LOG_LVL1);

	// Only one channel required (logically!)
	// Receiver is able to determine the power received when transmitter uses more than one channel by its own
	current_left_channel = primary_channel;
	current_right_channel = primary_channel;

	// Send request MCS notification
	Notification request_modulation = generateNotification(PACKET_TYPE_MCS_REQUEST, current_destination_id, TX_DURATION_NONE);
	outportAskForTxModulation(request_modulation);

	int ix_aux = current_destination_id - wlan.list_sta_id[0];	// Auxiliary variable for correcting the node id offset

	// MCS of receiver is not pending anymore
	change_modulation_flag[ix_aux] = FALSE;

	// if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s requestMCS() END\n", SimTime(), node_id, node_state, LOG_G00, LOG_LVL1);

}

/*
 * selectDestination(): pick destination node
 */
void Node :: selectDestination(){

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s selectDestination()\n",
			SimTime(), node_id, node_state, LOG_G00, LOG_LVL1);

	// Pick one of the STAs in the WLAN uniformely
	if(wlan.num_stas > 0){

		int rand_ix = rand()%(wlan.num_stas);
		current_destination_id = wlan.list_sta_id[rand_ix];

	} else {

		current_destination_id = NODE_ID_NONE;
		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s WLAN has no STAs! Transmitting to no node in particular.\n",
			SimTime(), node_id, node_state, LOG_G00, LOG_LVL2);

	}

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Selected destination: N%d\n",
			SimTime(), node_id, node_state, LOG_G00, LOG_LVL2, current_destination_id);

	// if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s selectDestination() END\n", SimTime(), node_id, node_state, LOG_G00, LOG_LVL1);
}


/***********************/
/***********************/
/*  POWER MANAGEMENT   */
/***********************/
/***********************/

/*
 * computePowerReceived() returns the power received in a given distance from the transmitter depending on the path loss model
 * Input arguments:
 * - distance: distance between transceiver and received [m]
 * - tx_power: TX power [dBm]
 * - tx_gain: transmitter gain [dB]
 * - rx_gain: receiver gain [dB]
 * Output arguments:
 * - Power received [dBm]
 */
double Node::computePowerReceived(double distance, double tx_power, int tx_gain, int rx_gain) {

	double pw_received = 0;
	double wavelength = 3*pow(10,8)/(central_frequency*pow(10,9));
	double loss;

	switch(path_loss_model){
		// Free space - Calculator: https://www.pasternack.com/t-calculator-fspl.aspx
		case PATH_LOSS_LFS:{
			loss = 32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000);
			pw_received = tx_power + tx_gain + rx_gain - loss;
			break;
		}
		// Okumura-Hata model - Urban areas
		case PATH_LOSS_OKUMURA_HATA:{
			double tx_heigth = 10;		// Transmitter height [m]
			double rx_heigth = 10;		// Receiver height [m]
			double path_loss_A = 69.55 + 26.16 * log10(3*pow(10,8)/wavelength) - 13.82 * log10(tx_heigth);
			double path_loss_B = 44.9 - 6.55 * log10(tx_heigth);
			double path_loss_E = 3.2 * pow(log10(11.7554 * rx_heigth),2) - 4.97;
			double path_loss = path_loss_A + path_loss_B * log10(distance/1000) - path_loss_E;
			pw_received = tx_power + tx_gain + rx_gain - path_loss;
			break;
		}
		// Indoor model (could suite an apartments building scenario)
		case PATH_LOSS_INDOOR: {
			double path_loss_factor = 5;
			double shadowing = 9.5;
			double obstacles = 30;
			double walls_frequency = 5; //  One wall each 5 meters on average
			double shadowing_at_wlan = (((double) rand())/RAND_MAX)*shadowing;
			double obstacles_at_wlan = (((double) rand())/RAND_MAX)*obstacles;
			double alpha = 4.4; // Propagation model
			double path_loss = path_loss_factor + 10*alpha*log10(distance) + shadowing_at_wlan +
					(distance/walls_frequency)*obstacles_at_wlan;
			pw_received = tx_power + tx_gain - path_loss; // Power in dBm
			break;
		}
		// Indoor model without variability
		case PATH_LOSS_INDOOR_2: {
			double path_loss_factor = 5;
			double shadowing = 9.5;
			double obstacles = 30;
			double walls_frequency = 5; //  One wall each 5 meters on average
			double shadowing_at_wlan = 1/2*shadowing;
			double obstacles_at_wlan = 1/2*obstacles;
			double alpha = 4.4; // Propagation model
			double path_loss = path_loss_factor + 10*alpha*log10(distance) + shadowing_at_wlan +
					(distance/walls_frequency)*obstacles_at_wlan;
			pw_received = tx_power + tx_gain - path_loss; // Power in dBm
			break;
		}

		// Residential - 5 dB/wall and 18.3 dB per floor, and 4 dB shadow
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx

		// IEEE 802.11ax uses the TGn channel B path loss model for performance evaluation of simulation scenario #1
		// with extra indoor wall and floor penetration loss.
		case PATH_LOSS_SCENARIO_1_TGax: {

			int n_walls = 10; 	// Wall frequency (n_walls walls each m)
			int n_floors = 3; 	// Floor frequency (n_floors floors each m)
			int L_iw = 5; 		// Penetration for a single wall (dB)

			double LFS = 32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000);

			int d_BP = 5;		// Break-point distance (m)

			if (distance >= d_BP) {
				loss = LFS + 35*log10(distance/d_BP) + 18.3*pow(n_floors,((n_floors+2)/(n_floors+1)) - 0.46) + L_iw*n_walls;
			} else {
				loss = LFS;
			}

			pw_received = tx_power + tx_gain + rx_gain - loss;
			break;

		}

		// Enterprise - 5 dB/wall and 18.3 dB per floor, and 4 dB shadow
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx

		// IEEE 802.11ax uses the TGn channel D path loss model for performance evaluation of simulation scenario #2
		// with extra indoor wall and floor penetration loss.
		case PATH_LOSS_SCENARIO_2_TGax: {

			int n_walls = 2; 	// Wall frequency (n_walls walls each m)
			int n_floors = 3; 		// Floor frequency (n_floors floors each m)
			int L_iw = 7; 	// Penetration for a single wall (dB)

			double LFS = 32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000);

			int d_BP = 10;		// Break-point distance (m)

			if (distance >= d_BP) {
				loss = LFS + 35*log10(distance/d_BP) + 18.3*pow(n_floors,((n_floors+2)/(n_floors+1)) - 0.46) + L_iw*n_walls;
			} else {
				loss = LFS;
			}

			pw_received = tx_power + tx_gain + rx_gain - loss;
			break;

		}

		// Indoor small BSSs
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx

		// IEEE 802.11ax uses the TGn channel D path loss model for performance evaluation
		// of simulation scenario #3.
		case PATH_LOSS_SCENARIO_3_TGax: {

			double LFS = 32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000);

			int d_BP = 10;		// Break-point distance (m)

			if (distance >= d_BP) {
				loss = LFS + 35*log10(distance/d_BP);
			} else {
				loss = LFS;
			}

			pw_received = tx_power + tx_gain + rx_gain - loss;
			break;
		}

		// Outdoor large BSS scenario
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx
		case PATH_LOSS_SCENARIO_4_TGax: {

			double h_AP = 10;		// Height of the AP in m
			double h_STA = 1.5; 	// Height of the STA in m

			double d_BP = (4 * (h_AP - 1) * (h_STA - 1) * central_frequency * pow(10,9)) / SPEED_LIGHT;


			if (distance < d_BP && distance >= 10) {
				loss = 22 * log10(distance) + 28 + 20  *log10(central_frequency);
			} else if (distance >= d_BP && distance < 5000) {
				loss = 40 * log10(distance) + 7.8 + 18 * log10(h_AP - 1) -
						18 * log10(h_STA - 1)	+ 20 * log10(central_frequency);
			}

			pw_received = tx_power + tx_gain - loss; // Power in dBm
			break;
		}

		// Outdoor large BSS scenario + Residential
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx
		case PATH_LOSS_SCENARIO_4a_TGax: {

//			double h_AP = 10;		// Height of the AP in m
//			double h_STA = 1.5; 	// Height of the STA in m

			if (distance < 2000 && distance >= 10) {
				loss = 36.7 * log10(distance) + 22.7 + 26  * log10(central_frequency);
			}

			// Outdoor-to-Indoor building penetration loss
			// TODO: important to consider specifying d_outdoor and d_indoor
			double d_outdoor = 0;
			double d_indoor = 0;
			loss = loss * (d_outdoor + d_indoor) + 20 + 0.5 * d_indoor;

			pw_received = tx_power + tx_gain - loss; // Power in dBm
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
 * convertPower(): convert power units
 * Input arguments:
 * - conversion_type: unit conversion type
 * - power: power value
 * Output:
 * - converted_power: power converted to required unit
 */
double convertPower(int conversion_type, double power){
	double converted_power;
	switch(conversion_type){
		// pW to dBm
		case PICO_TO_DBM:{
			converted_power = 10 * log10(power * pow(10,-9));
			break;
		}
		// dBm to pW
		case DBM_TO_PICO:{
			converted_power = pow(10,(power + 90)/10);
			break;
		}
		// mW to dBm
		case MILLI_TO_DBM:{
			converted_power = 10 * log10(power * pow(10,-6));
			break;
		}
		// dBm to mW (dB to linear)
		case DBM_TO_MILLI:
		case DB_TO_LINEAR:
		case DB_TO_W: {
			converted_power = pow(10,power/10);
			break;
		}
		// W to dB
		case W_TO_DB:
		case LINEAR_TO_DB: {
			converted_power = 10 * log10(power);
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
 * Output:
 * - distance: distance [m]
 */
double computeDistance(int x1, int y1, int z1, int x2, int y2, int z2){
	double distance = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
	return distance;
}


/**********************/
/**********************/
/* TX TIME MANAGEMENT */
/**********************/
/**********************/

/*
 * computeTxTime(): computes the transmission time (just link rate) according to the number of channels used and packet lenght
 * Input arguments:
 * - num_channels_used: number of channels (OR INDEX) used in the transmission
 * - total_bits: number of bits sent in the transmission
 */
double Node :: computeTxTime(int ix_num_channels_used, int total_bits){

	double tx_time;

	// Decide current modulation and channels to be used during transmission

	if(node_state == STATE_TX_DATA) {

		int channels_to_use_ix = current_right_channel - current_left_channel;
		int ix_aux = current_destination_id - wlan.list_sta_id[0];
		// TODO: by now, dumb policy: maximum modulation and channels provided by CB model
		current_modulation = mcs_per_node[ix_aux][channels_to_use_ix];
		current_data_rate = modulation_rates[channels_to_use_ix][current_modulation-1]
										 * coding_rate_modulation[current_modulation-1];

	} else if (node_state == STATE_TX_ACK) {

		current_data_rate = ongoing_notification.tx_info.data_rate;

	}

	// Hardcoded for 1 channel
	//current_data_rate = data_rate_array[0];

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Data rate (%d channels and modulation %d): %f Mbps\n",
				SimTime(), node_id, node_state, LOG_Z01, LOG_LVL3, ix_num_channels_used, current_modulation, current_data_rate*pow(10,-6));

	switch(pdf_tx_time){

		case PDF_DETERMINISTIC:{
			tx_time = total_bits/current_data_rate;	// 0.01234 s
			break;
		}

		case PDF_EXPONENTIAL:{
			tx_time = Exponential(total_bits/current_data_rate);
			break;
		}

		default:{
			printf("TX time model not found!\n");
			break;
		}
	}

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Computed TX duration: %f s\n",
			SimTime(), node_id, node_state, LOG_Z01, LOG_LVL3, tx_time);

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
 * - position: flag indicating to find the first or last one in the array
 * - channels_available: range to consider
 * - total_channels_number: total number of channels
 * Output:
 * - left_tx_ch or right_tx_ch: first or last one in the array
 */
int getBoundaryChannel(int position, int *channels_available, int total_channels_number){
	int left_tx_ch = 0;
	int left_tx_ch_is_set = 0;
	int right_tx_ch = 0;
	for(int c = 0; c < total_channels_number; c++){
		if(channels_available[c]){
			if(!left_tx_ch_is_set){
				left_tx_ch = c;
				left_tx_ch_is_set = TRUE;
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
 * getChannelOccupancyByCCA(): (DEPECATED) indicates the channels occupied and free in a binary way
 */
void Node :: getChannelOccupancyByCCA(){

	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){

		if(channel_power[c] < convertPower(DBM_TO_PICO, current_cca)){
			channels_free[c] = CHANNEL_OCCUPIED;
		} else {
			channels_free[c] = CHANNEL_FREE;
		}
	}
}

/*
 * updateChannelsPower: updates the power sensed by the node in every channel array depending on the notifications received
 * (tx start, tx end) and the cochannel model.
 * Arguments:
 * - notification: type of co-channel model applied (no co-channel interference, 20 dB mask, ...)
 * - update_type: type of power update (sum or subtract)
 */
void Node :: updateChannelsPower(Notification notification, int update_type){

	double pw_received_pico;

	if(update_type == TX_FINISHED) {
		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Decreasing channel power sensed\n",
						SimTime(), node_id, node_state, LOG_E04, LOG_LVL3);
		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Pre update channel state [dBm]: ",
				SimTime(), node_id, node_state, LOG_E04, LOG_LVL4);

	} else if(update_type == TX_INITIATED) {

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Increasing channel power sensed\n",
								SimTime(), node_id, node_state, LOG_E04, LOG_LVL3);

		double distance = computeDistance(x, y, z, notification.tx_info.x, notification.tx_info.y, notification.tx_info.z);

		double pw_received = computePowerReceived(distance, notification.tx_info.tx_power, tx_gain, rx_gain);

		pw_received_pico = convertPower(DBM_TO_PICO, pw_received);

		power_received_per_node[notification.source_id] = pw_received_pico;

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Distance to transmitting node N%d: %f m\n",
				SimTime(), node_id, node_state, LOG_D04, LOG_LVL4, notification.source_id, distance);

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Power received from N%d: = %f dBm (%f pW)\n",
				SimTime(), node_id, node_state, LOG_D04, LOG_LVL4, notification.source_id,
				convertPower(PICO_TO_DBM, pw_received_pico), pw_received_pico);

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Pre update channel state [dBm]: ",
				SimTime(), node_id, node_state, LOG_D03, LOG_LVL4);

	} else {
		printf("ERROR: update_type %d does not exist!!!", update_type);
	}

	printOrWriteChannelPower(WRITE_LOG);

	applyInterferenceModel(notification, update_type);

	if(save_node_logs){

		if(update_type == TX_INITIATED)	fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s Post update channel state [dBm]: ",
				SimTime(), node_id, node_state, LOG_D06, LOG_LVL4);

		if(update_type == TX_FINISHED) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s Post update channel state [dBm]: ",
				SimTime(), node_id, node_state, LOG_E05, LOG_LVL4);

	}

	printOrWriteChannelPower(WRITE_LOG);
}

/*
 * applyInterferenceModel: applies the interference model
 * Arguments:
 * - notification: type of co-channel model applied (0: no co-channel interference, 1: 20 dB mask, ...)
 * - update_type: type of power update (sum or subtract)
 */
void Node :: applyInterferenceModel(Notification notification, int update_type){

	// Total power (of interest and interference) generated only by the incoming or outgoing TX
	double total_power[num_channels_komondor];
	for(int i = 0; i < num_channels_komondor; i++) {
		total_power[i] = 0;
	}

	// Direct power (power of the channels used for transmitting)
	for(int i = notification.left_channel; i <= notification.right_channel; i++){
		total_power[i] = power_received_per_node[notification.source_id];
	}

	// Co-channel interference power
	switch(cochannel_model){

		case COCHANNEL_NONE:{
			// Do nothing
			break;
		}

		// (RECOMMENDED) Boundary co-channel interference: only boundary channels (left and right) used in the TX affect the rest of channels
		case COCHANNEL_BOUNDARY:{

			for(int c = 0; c < num_channels_komondor; c++) {

				if(c < notification.left_channel || c > notification.right_channel){

					if(c < notification.left_channel) {

						total_power[c] += convertPower(DBM_TO_PICO,
								convertPower(PICO_TO_DBM, power_received_per_node[notification.source_id])
								- 20*abs(c-notification.left_channel));

					} else if(c > notification.right_channel) {

						total_power[c] += convertPower(DBM_TO_PICO,
								convertPower(PICO_TO_DBM, power_received_per_node[notification.source_id])
								- 20*abs(c-notification.right_channel));

					}

					if(total_power[c] < MIN_VALUE_C_LANGUAGE){

						total_power[c] = 0;

					}

				} else {
					// Inside TX range --> do nothing
				}
			}
			break;
		}
		case COCHANNEL_EXTREME:{

			for(int i = 0; i < num_channels_komondor; i++) {

				for(int j = notification.left_channel; j <= notification.right_channel; j++){

					if(i != j) {

						total_power[i] += convertPower(PICO_TO_DBM, convertPower(DBM_TO_PICO, power_received_per_node[notification.source_id]) - 20*abs(i-j));

						if(total_power[i] < MIN_VALUE_C_LANGUAGE) total_power[i] = 0;

					}
				}
			}
			break;
		}
		default:{
			break;
		}
	}

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Power to sum or subtract [dBm]: ",
			SimTime(), node_id, node_state, LOG_D06, LOG_LVL5);

	for(int i = 0; i < num_channels_komondor; i++) {

		if(save_node_logs) fprintf(node_logger.file,"%f ", convertPower(PICO_TO_DBM, total_power[i]));

	}
	if(save_node_logs) fprintf(node_logger.file,"\n");

	// Increase/decrease power sensed if TX started/finished
	for(int c = 0; c < num_channels_komondor; c++){

		if(update_type == TX_FINISHED) {

			channel_power[c] -= total_power[c];

		} else if (update_type == TX_INITIATED) {

			channel_power[c] += total_power[c];

		}
	}

}

/*
 * getTxChannelsByChannelBonding: identifies the channels to TX in depending on the channel_bonding scheme
 * and channel_power state.
 */
void Node :: getTxChannelsByChannelBonding(int channel_bonding_model, int *channels_free){

	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
		channels_for_tx[c] = FALSE;
	}

	// I. Get left and right channels available (or free)
	int left_free_ch = 0;
	int left_free_ch_is_set = 0;
	int right_free_ch = 0;
	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
		if(channels_free[c]){
			if(!left_free_ch_is_set){
				left_free_ch = c;
				left_free_ch_is_set = TRUE;
			}
			if(right_free_ch < c){
				right_free_ch = c;
			}
		}
	}

	int num_free_ch = right_free_ch - left_free_ch + 1;
	int num_available_ch = max_channel_allowed - min_channel_allowed + 1;

	int m;				// Auxiliary variable representing a modulus
	int left_tx_ch;		// Left channel to TX
	int right_tx_ch;	// Right channel to TX

	if(left_free_ch_is_set){
		// Select channels to transmit depending on the sensed power
		switch(channel_bonding_model){

			// Only Primary Channel used if FREE
			case CB_ONLY_PRIMARY:{
				if(primary_channel >= left_free_ch && primary_channel <= right_free_ch){
					channels_for_tx[primary_channel] = CHANNEL_OCCUPIED;
				}
				break;
			}

			// Aggressive SCB: if all channels are FREE, transmit. If not, generate a new backoff.
			case CB_AGGRESIVE_SCB:{
				int tx_possible = TRUE;
				// If all channels are FREE, transmit. If not, generate a new backoff.
				for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
					if(!channels_free[c]){
						tx_possible = FALSE;
					}
				}
				if(tx_possible){
					left_tx_ch = left_free_ch;
					right_tx_ch = right_free_ch;
					for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
						channels_for_tx[c] = CHANNEL_OCCUPIED;
					}
				} else {
					// TX not possible (code it with negative value)
					channels_for_tx[0] = TX_NOT_POSSIBLE;
				}
				break;
			}

			// Log2 SCB:  if all channels accepted by the log2 mapping are FREE, transmit. If not, generate a new backoff.
			case CB_LOG2_SCB:{
				while(1){
					// II. If num_free_ch is power of 2
					if(fmod(log10(num_available_ch)/log10(2), 1) == 0){
						m = primary_channel % num_available_ch;
						left_tx_ch = primary_channel - m;
						right_tx_ch = primary_channel + num_available_ch - m - 1;
						// Check if tx channels are inside the free ones
						if((left_tx_ch >= min_channel_allowed) && (right_tx_ch <= max_channel_allowed)){
							// TX channels found!
							break;
						} else {
							num_available_ch --;
						}
					} else{
						num_available_ch --;
					}
				}

				// If all channels accepted by the log2 mapping, transmit. If not, generate a new backoff.
				int tx_possible = TRUE;
				for(int c = left_tx_ch; c <= right_tx_ch; c++){
					if(!channels_free[c]){
						tx_possible = FALSE;
					}
				}
				if(tx_possible){
					for(int c = left_tx_ch; c <= right_tx_ch; c++){
						channels_for_tx[c] = CHANNEL_OCCUPIED;
					}
				} else {
					// TX not possible (code it with negative value)
					channels_for_tx[0] = TX_NOT_POSSIBLE;
				}
				break;
			}

			// Aggressive DCB: TX in all the free channels contiguous to the primary channel
			case CB_AGGRESIVE_DCB:{
				for(int c = left_free_ch; c <= right_free_ch; c++){
					channels_for_tx[c] = CHANNEL_OCCUPIED;
				}
				break;
			}

			// Log2 DCB: TX in the larger channel range allowed by the log2 mapping
			case CB_LOG2_DCB:{
				while(1){
					// II. If num_free_ch is power of 2
					if(fmod(log10(num_free_ch)/log10(2), 1) == 0){
						m = primary_channel % num_free_ch;
						left_tx_ch = primary_channel - m;
						right_tx_ch = primary_channel + num_free_ch - m - 1;
						// Check if tx channels are inside the free ones
						if((left_tx_ch >= min_channel_allowed) && (right_tx_ch <= max_channel_allowed)){
							// TX channels found!
							for(int c = left_tx_ch; c <= right_tx_ch; c++){
								channels_for_tx[c] = CHANNEL_OCCUPIED;
							}
							break;
						} else {
							num_free_ch --;
						}
					} else{
						num_free_ch --;
					}
				}
				break;
			}
			default:{
				printf("channel_bonding_model %d is NOT VALID!\n", channel_bonding_model);
				break;
			}
		}
	} else{	// No channel is free

		channels_for_tx[0] = TX_NOT_POSSIBLE;

	}

	if(channels_for_tx[0] == TX_NOT_POSSIBLE){

		num_tx_init_not_possible++;

	}


}

/*
 * updateSINR(): Updates the current_sinr parameter
 * Arguments:
 * - pw_received_interest: power received of interest
 * */
void Node :: updateSINR(double pw_received_interest){

	double sinr_db = convertPower(PICO_TO_DBM, pw_received_interest)
						- convertPower(PICO_TO_DBM, (convertPower(DBM_TO_PICO, noise_level) + max_pw_interference));

	current_sinr = convertPower(DB_TO_LINEAR, sinr_db);

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s SINR (after update) = %f dBm\n",
			SimTime(), node_id, node_state, LOG_D04, LOG_LVL4, convertPower(LINEAR_TO_DB, current_sinr));
}

/*
 * computeMaxInterference(): computes the maximum interference perceived in the channels of interest
 * Arguments:
 * - notification: notification info
 * */
void Node :: computeMaxInterference(Notification notification) {

	max_pw_interference = 0;

	for(int c = current_left_channel; c <= current_right_channel; c++){

		if(node_state == STATE_RX_DATA || node_state == STATE_RX_ACK){

			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Computing Max Interference (power_of_interest = %f dBm / channel_power = %f dBm)\n",
						SimTime(), node_id, node_state, LOG_D04, LOG_LVL4,
						convertPower(LINEAR_TO_DB, power_received_per_node[receiving_from_node_id]),
						convertPower(LINEAR_TO_DB, channel_power[c]));


			if(max_pw_interference <= (channel_power[c] - power_received_per_node[receiving_from_node_id])){ // power of interest!!!

				max_pw_interference = channel_power[c] - power_received_per_node[receiving_from_node_id];
				channel_max_intereference = c;

			}

			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Max interference (after update) = %f dBm\n",
						SimTime(), node_id, node_state, LOG_D04, LOG_LVL5, convertPower(PICO_TO_DBM,max_pw_interference));

		}
	}
}

/*********************/
/*********************/
/*  HANDLE PACKETS   */
/*********************/
/*********************/

/*
 * generateNotification: generates a Notification
 * Arguments:
 * - packet_id: packet type
 * - destination_id: node destination ID
 * - tx_duration: transmission duration
 */
Notification Node :: generateNotification(int packet_type, int destination_id, double tx_duration){

	Notification notification;
	TxInfo tx_info;

	tx_info.setSizeOfMCS(4);

	tx_info.destination_id = destination_id;
	tx_info.tx_duration = tx_duration;
	tx_info.tx_power = current_tpc;

	tx_info.data_rate = current_data_rate;
	tx_info.x = x;
	tx_info.y = y;
	tx_info.z = z;
	tx_info.packet_id = packet_id;

	notification.tx_info = tx_info;
	notification.packet_type = packet_type;
	notification.source_id = node_id;
	notification.left_channel = current_left_channel;
	notification.right_channel = current_right_channel;

	switch(packet_type){

		case PACKET_TYPE_DATA:{
			notification.packet_length = packet_length;
			break;
		}

		case PACKET_TYPE_ACK:{
			notification.packet_length = ack_length;
			break;
		}

		case PACKET_TYPE_MCS_REQUEST:{
			// Do nothing
			break;
		}

		case PACKET_TYPE_MCS_RESPONSE:{
			for(int i = 0; i < 4; i++) {
				tx_info.modulation_schemes[i] = mcs_response[i];
			}
			break;
		}

		default:
			break;
	}

	return notification;
}

/*
 * generateAndSendLogicalNack: Sends a NACK notification
 * Arguments:
 * - packet_id:
 * - node_id_a:
 * - node_id_b:
 * - reason_id:
 */
void Node :: generateAndSendLogicalNack(int packet_id, int node_id_a, int node_id_b, int loss_reason){

	LogicalNack logical_nack_info;
	logical_nack_info.source_id = node_id;
	logical_nack_info.packet_id = packet_id;
	logical_nack_info.loss_reason = loss_reason;
	logical_nack_info.node_id_a = node_id_a;
	logical_nack_info.node_id_b = node_id_b;
	logical_nack_info.BER = BER;
	logical_nack_info.SINR = current_sinr;

	outportSendLogicalNack(logical_nack_info);

	if(save_node_logs) fprintf(node_logger.file,
			"%f;N%d;S%d;%s;%s NACK sent to a) N%d and b) N%d with reason %d\n",
			SimTime(), node_id, node_state, LOG_I00, LOG_LVL2, logical_nack_info.node_id_a,
			logical_nack_info.node_id_b, logical_nack_info.loss_reason);

}

/*
 * cleanNack(): re-initializes the Nack info.
 */
void Node :: cleanNack(){

	nack.source_id = NODE_ID_NONE;
	nack.packet_id = NO_PACKET_ID;
	nack.loss_reason = PACKET_NOT_LOST;
	nack.node_id_a = NODE_ID_NONE;
	nack.node_id_b = NODE_ID_NONE;

}

/*
 * sendAck(): sends and ACK. It is called when SIFS is triggered.
 */
void Node :: sendAck(trigger_t &){

	if(save_node_logs) fprintf(node_logger.file,
			"%f;N%d;S%d;%s;%s SIFS completed, sending ACK\n",
			SimTime(), node_id, node_state, LOG_I00, LOG_LVL2);

	outportSelfStartTX(ack);
	trigger_toFinishTX.Set(SimTime() + ack.tx_info.tx_duration);
}

/*
 * ackTimeout(): handles ACK timeout. It is called when ACK timeout is triggered.
 */
void Node :: ackTimeout(trigger_t &){

	if(save_node_logs) fprintf(node_logger.file,
			"%f;N%d;G02; - ACK TIMEOUT! Transmission %d has been lost\n", SimTime(), node_id, packet_id);

	current_tx_duration += (1 + MIN_VALUE_C_LANGUAGE) * SIFS;		// Add ACK timeout to tx_duration

	for(int c = current_left_channel; c <= current_right_channel; c++){
		total_time_transmitting_per_channel[c] += (1 + MIN_VALUE_C_LANGUAGE) * SIFS;
	}

	handlePacketLoss();
	restartNode();
}

/*
 * processNack(): processes a NACK notification.
 * Arguments:
 * - logical_nack_info: Nack information
 */
void Node :: processNack(LogicalNack logical_nack_info) {

	// Nodes implied in the NACK
	int node_A = logical_nack_info.node_id_a;
	int node_B = logical_nack_info.node_id_b;

	if(node_A == node_id ||  node_B == node_id){		// If node IMPLIED in the NACK

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s I am implied in the NACK with packet id #%d\n",
				SimTime(), node_id, node_state, LOG_H02, LOG_LVL2, logical_nack_info.packet_id);

		// Sum new loss reason to corresponding type (for statistics purposes)
		nacks_received[logical_nack_info.loss_reason] ++;

		switch(logical_nack_info.loss_reason){

			case PACKET_LOST_DESTINATION_TX:{	// Destination was already transmitting when the packet transmission was attempted

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Destination N%d was transmitting!s\n",
						SimTime(), node_id, node_state, LOG_H02, LOG_LVL2, logical_nack_info.source_id);

				// Add receiver to hidden nodes list ("I was not listening to him!")
				hidden_nodes_list[logical_nack_info.source_id] = TRUE;

				break;
			}

			case PACKET_LOST_LOW_SIGNAL:{	// Signal strength is not enough to be decoded (less than capture effect)

				if(save_node_logs) fprintf(node_logger.file,
						"%f;N%d;S%d;%s;%s Power received in destination N%d is less than the required capture effect!\n",
						SimTime(), node_id, node_state, LOG_H02, LOG_LVL2, logical_nack_info.source_id);

				break;
			}

			case PACKET_LOST_INTERFERENCE:{ 	// There are interference signals making node not comply with the capture effect

				if(save_node_logs) fprintf(node_logger.file,
						"%f;N%d;S%d;%s;%s High interferences sensed in destination N%d (capture effect not accomplished)!\n",
						SimTime(), node_id, node_state, LOG_H02, LOG_LVL2, logical_nack_info.source_id);

				// Increase the number of times of POTENTIAL hidden nodes with the current transmitting nodes
				for(int i = 0; i < total_nodes_number; i++) {
					if (nodes_transmitting[i] && i != node_id && i != logical_nack_info.source_id){
						potential_hidden_nodes[i] ++;
					}
				}

				break;
			}

			case PACKET_LOST_PURE_COLLISION:{	// Two nodes transmitting to same destination with signal strengths enough to be decoded

				if(save_node_logs) fprintf(node_logger.file,
					"%f;N%d;S%d;%s;%s Pure collision detected at destination %d! %d was transmitting and %d appeared\n",
					SimTime(), node_id, node_state, LOG_H02, LOG_LVL2, logical_nack_info.source_id,
					node_A, node_B);

				// Add to hidden nodes list
				if(node_A != node_id) {
					hidden_nodes_list[node_A] = TRUE;
				} else if (node_B != node_id) {
					hidden_nodes_list[node_B] = TRUE;
				}

				break;
			}

			case PACKET_LOST_LOW_SIGNAL_AND_RX:{ // Destination already receiving and new signal strength was not enough to be decoded

				// Only node_id_a has lost the packet, so that node_id_b is his hidden node
				if(node_A == node_id) {

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s Destination N%d already receiving from N%d and N%d transmitted with not enough"
							" power to be decoded\n",
							SimTime(), node_id, node_state, LOG_H02, LOG_LVL2, logical_nack_info.source_id, node_A, node_B);

					hidden_nodes_list[node_B] = TRUE;
				}

				break;
			}

			case PACKET_LOST_SINR_PROB:{	// Packet lost due to SINR probability (deprecated)

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Packet lost due to the BER (%f) "
						"associated to the current SINR (%f dB)\n", SimTime(), node_id, node_state, LOG_H02, LOG_LVL2,
						logical_nack_info.BER, convertPower(LINEAR_TO_DB,logical_nack_info.SINR));

				break;
			}

			default:{

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Unknown reason for packet loss\n",
						SimTime(), node_id, node_state, LOG_H02, LOG_LVL2);
				exit(EXIT_FAILURE);
				break;
			}

		}

	} else {	// If node NOT IMPLIED in the NACK, do nothing
		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s I am NOT implied in the NACK\n",
				SimTime(), node_id, node_state, LOG_H02, LOG_LVL2);
	}
}

/************************/
/************************/
/*PACKET LOSS MANAGEMENT*/
/************************/
/************************/

/*
 * handleCW(): increase or decrease the CW.
 * - mode: flag for indicating increase or decrease of CW
 */
void Node :: handleCW(int mode) {
	// http://article.sapub.org/pdf/10.5923.j.jwnc.20130301.01.pdf
	if(INCREASE_CW){
		if(2*current_CW < CW_max) {
			current_CW = 2*current_CW;
		} else {
			current_CW = CW_max;
		}
	} else if(DECREASE_CW){
		current_CW = CW_min;
	}

	// Set lambda accordingly
	double EB = (double) (current_CW-1)/2;
	lambda =  1/(EB * SLOT_TIME);
}

/*
 * handlePacketLoss(): handles a packet loss.
 */
void Node :: handlePacketLoss(){

	for(int c = current_left_channel; c <= current_right_channel; c++){
		total_time_lost_per_channel[c] += current_tx_duration;
	}

	total_time_lost_in_num_channels[current_right_channel - current_left_channel] += current_tx_duration;

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Packet lost\n",
					SimTime(), node_id, node_state, LOG_D17, LOG_LVL4);

	handleCW(INCREASE_CW);
	packets_lost ++;
}

/*
 * isPacketLost(): computes notification loss according to SINR received
 * Arguments:
 * - channel_power: power sensed in the channel of interest
 * - notification: notification info received
 * Output:
 * - reason: reason ID of the packet loss
 */
int Node :: isPacketLost(double channel_power, Notification notification){

	int loss_reason = PACKET_NOT_LOST;
	int is_packet_lost;	// Determines if the current notification has been lost (1) or not (0)

	updateSINR(power_received_per_node[notification.source_id]);

	//is_packet_lost = applyModulationProbabilityError(notification);

	is_packet_lost = attemptToDecodePacket();

	if (is_packet_lost) {

		if (convertPower(PICO_TO_DBM, pw_received_interest) < current_cca) {	// Signal strength is not enough (< CCA) to be decoded

			if(save_node_logs) fprintf(node_logger.file,
					"%f;N%d;S%d;%s;%s Signal strength (%f dBm) is less than CCA (%f dBm)\n",
				SimTime(), node_id, node_state, LOG_D17, LOG_LVL4,
				convertPower(PICO_TO_DBM, pw_received_interest), current_cca);

			loss_reason = PACKET_LOST_LOW_SIGNAL;
			hidden_nodes_list[notification.source_id] = TRUE;

		} else if (convertPower(LINEAR_TO_DB, current_sinr) < capture_effect){	// Capture effect not accomplished

			if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s Capture effect not accomplished (current_sinr = %f dBm)\n",
				SimTime(), node_id, node_state, LOG_D17, LOG_LVL4, convertPower(LINEAR_TO_DB, current_sinr));
			loss_reason = PACKET_LOST_INTERFERENCE;

		} else {

			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Incoming packet is lost due to PER\n",
				SimTime(), node_id, node_state, LOG_D17, LOG_LVL4);
			loss_reason = PACKET_LOST_SINR_PROB;

		}
	}

	return loss_reason;
}

/*
 * applyModulationProbabilityError():
 * "Energy per bit to noise power spectral density ratio"
 * (normalized SNR to compare the BER performance of a digital modulation scheme)
 * Arguments:
 * - SINR: SINR received (pW)
 * - bit_rate: bit rate (bps)
 * - B: channel bandwidth (Hz)
 * - M: number of alternative modulation symbols
 * Output:
 * - Eb_to_N0: Eb_to_N0 value in dB
 */
double EbToNoise(double SINR, double bit_rate, int B, int M){

	double Es_to_N0 = SINR * (bit_rate/B);

	//printf("Es_to_N0 = %f (%f) \n", Es_to_N0, convertPower(LINEAR_TO_DB, Es_to_N0));

	double Eb_to_N0 = Es_to_N0 * log2(M);

	//printf("Eb_to_N0 = %f (%f)\n", Eb_to_N0, convertPower(LINEAR_TO_DB, Eb_to_N0));

	return convertPower(LINEAR_TO_DB, Eb_to_N0);
}

int Node :: attemptToDecodePacket(){

	int packet_lost;

	// Try to decode when power received is greater than CCA
	if(convertPower(LINEAR_TO_DB, current_sinr) < capture_effect
			&& convertPower(PICO_TO_DBM, pw_received_interest) > current_cca) {

		PER = 1;

	} else {

		PER = constant_PER;

	}

	packet_lost = ((double) rand() / (RAND_MAX)) < PER;
	return packet_lost;
}

/*
 * applyModulationProbabilityError():
 * Arguments:
 *
 */
int Node :: applyModulationProbabilityError(Notification notification){

	//TODO: CHANGE THE WAY BER IS COMPUTED!!!

	int is_packet_lost = PACKET_NOT_LOST;
	double Eb_to_N0 = 0;
	double bit_rate;
	int channels_used;
	int M;
	int bw;

	if(convertPower(LINEAR_TO_DB, current_sinr) < capture_effect) {

		BER = 1; //100% lost

	} else {

		// To avoid errors in case modulation is not set
		if(current_modulation > 0) {
			channels_used = (notification.right_channel - notification.left_channel) + 1;
			M = bits_per_symbol_modulation[current_modulation-1];
			bit_rate = notification.tx_info.data_rate;
			bw = channels_used*basic_channel_bandwidth*pow(10,6);
			Eb_to_N0 = EbToNoise(current_sinr, bit_rate, bw, M);

		} else {
			//ERROR!
		}

		//printf("Eb_to_N0 = %f W (%f dB)\n",Eb_to_N0, convertPower(W_TO_DB,Eb_to_N0));
		switch(current_modulation) {
			case MODULATION_NONE: {
				break; // Ideal: no notification loss
			}
			case MODULATION_BPSK_1_2:
			case MODULATION_QPSK_1_2:
			case MODULATION_QPSK_3_4: {
				BER = erfc(sqrt(2*Eb_to_N0));
				break;
			}
			case MODULATION_16QAM_1_2:
			case MODULATION_16QAM_3_4:
			case MODULATION_64QAM_2_3:
			case MODULATION_64QAM_3_4:
			case MODULATION_64QAM_5_6:
			case MODULATION_256QAM_3_4:
			case MODULATION_256QAM_5_6:
			case MODULATION_1024QAM_3_4:
			case MODULATION_1024QAM_5_6: {
				BER = 4/log2(M) * erfc(sqrt((3*Eb_to_N0*log2(M))/(M-1)));
				break;
			}
			default: {
				break;
			}
		}
		// Compute packet loss according to the bit error probability
		PER = 1 - pow((1 - BER), notification.packet_length);
		is_packet_lost =  ((double) rand() / (RAND_MAX)) < PER;

		if(save_node_logs && is_packet_lost) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Packet has been lost: Modulation used = %d, BER = %f, PER = %f\n",
				SimTime(), node_id, node_state, LOG_F00, LOG_LVL5, current_modulation, BER, PER);

	}

	return is_packet_lost;

}

/************************/
/************************/
/*  BACKOFF MANAGEMENT  */
/************************/
/************************/

/*
 * computeBackoff(): computes a new backoff according to its pdf and packet generation rate.
 * Output:
 * - backoff_time: generated backoff
 * */
double Node :: computeBackoff(){

	double backoff_time;

	switch(pdf_backoff){

		case PDF_DETERMINISTIC:{
			backoff_time = 1/lambda;
			break;
		}

		case PDF_EXPONENTIAL:{
			backoff_time = Exponential(1/lambda);
			break;
		}

		default:{
			printf("Backoff model not found!\n");
			break;
		}
	}
	return backoff_time;
}

/*
 * handleBackoff(): handles the backoff. It is called when backoff may be paused or resumed.
 * Arguments:
 * - pause_or_resume: flag for identifying if function must try to pause or resume the backoff
 * - notification: notification info
 * */
void Node :: handleBackoff(int pause_or_resume){

	switch(pause_or_resume){

		case PAUSE_TIMER:{

			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s primary_channel (#%d) affected\n",
				SimTime(), node_id, node_state, LOG_F00, LOG_LVL2, primary_channel);

			if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s Power sensed in primary channel:  %f dBm (%f pW)\n",
				SimTime(), node_id, node_state, LOG_F00, LOG_LVL3, convertPower(PICO_TO_DBM, channel_power[primary_channel]),
				channel_power[primary_channel]);

			if(channel_power[primary_channel] > convertPower(DBM_TO_PICO, current_cca)){	// CCA exceeded

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s CCA (%f dBm) exceeded\n",
						SimTime(), node_id, node_state, LOG_F00, LOG_LVL3, current_cca);

				pauseBackoff();

			} else {	// CCA NOT exceeded

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s CCA (%f dBm) NOT exceeded\n",
					SimTime(), node_id, node_state, LOG_F00, LOG_LVL3, current_cca);
				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s primary_channel (#%d) NOT affected\n",
					SimTime(), node_id, node_state, LOG_F00, LOG_LVL3, primary_channel);
			}
			break;
		}

		case RESUME_TIMER:{

			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Primary_channel (#%d) affected\n",
				SimTime(), node_id, node_state, LOG_F00, LOG_LVL2, primary_channel);

			if(save_node_logs) fprintf(node_logger.file,
					"%f;N%d;S%d;%s;%s Power sensed in primary channel:  %f dBm (%f pW)\n",
					SimTime(), node_id, node_state, LOG_F00, LOG_LVL3, convertPower(PICO_TO_DBM,
							channel_power[primary_channel]), channel_power[primary_channel]);

			if(channel_power[primary_channel] <= convertPower(DBM_TO_PICO, current_cca)){	// CCA NOT exceeded

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s CCA (%f dBm) NOT exceeded\n",
					SimTime(), node_id, node_state, LOG_F00, LOG_LVL3, current_cca);
				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s primary_channel (#%d) NOT affected\n",
					SimTime(), node_id, node_state, LOG_F00, LOG_LVL3, primary_channel);

				trigger_DIFS.Set(SimTime() + DIFS);

			} else {	// CCA exceeded

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s CCA (%f dBm) exceeded\n",
					SimTime(), node_id, node_state, LOG_F00, LOG_LVL3, current_cca);

			}
			break;
		}

		default:{
			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Unknown mode %d! (not resume nor pause)\n",
				SimTime(), node_id, node_state, LOG_F00, LOG_LVL3, pause_or_resume);
			break;
		}
	}
}

/*
 * pauseBackoff(): pauses the backoff
 * */
void Node :: pauseBackoff(){

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Pausing the backoff\n",
		SimTime(), node_id, node_state, LOG_F00, LOG_LVL2);

	trigger_DIFS.Cancel();

	if(trigger_backoff.Active()){	// If backoff trigger is active, freeze it

		remaining_backoff = trigger_backoff.GetTime() - SimTime();

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s Backoff is active --> freeze it at %f s\n",
				SimTime(), node_id, node_state, LOG_F00, LOG_LVL3, remaining_backoff);

		trigger_backoff.Cancel();

	} else {	// If backoff trigger is frozen

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s Backoff is NOT active (it is already frozen at %f s)\n",
				SimTime(), node_id, node_state, LOG_F00, LOG_LVL3, trigger_backoff.GetTime());

		trigger_backoff.Cancel(); // Redundant (for safety)

	}

}

/*
 * resumeBackoff(): resumes the backoff
 * */
void Node :: resumeBackoff(trigger_t &){

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s DIFS finsihed\n",
					SimTime(), node_id, node_state, LOG_F00, LOG_LVL2);

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Resuming backoff at %f s\n",
				SimTime(), node_id, node_state, LOG_F00, LOG_LVL2, remaining_backoff);

	trigger_backoff.Set(SimTime() + remaining_backoff);

	if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s DIFS: active = %d, t_DIFS = %f - backoff: active = %d - t_back = %f\n",
				SimTime(), node_id, node_state, LOG_D02, LOG_LVL3,
				trigger_DIFS.Active(), trigger_DIFS.GetTime() - SimTime(),
				trigger_backoff.Active(), trigger_backoff.GetTime() - SimTime());
}

/***********************/
/***********************/
/* AUXILIARY FUNCTIONS */
/***********************/
/***********************/

/*
 * restartNode(): re-initializes the nodes. Puts it in the initial state (sensing and decreasing BO)
 */
void Node :: restartNode(){

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Node Restarted\n",
			SimTime(), node_id, node_state, LOG_Z00, LOG_LVL2);

	// Update TX time statistics
	total_time_transmitting_in_num_channels[current_right_channel - current_left_channel] += current_tx_duration;
	for(int c = current_left_channel; c <= current_right_channel; c++){
		total_time_transmitting_per_channel[c] += current_tx_duration;
	}

	// Reinitialize parameters
	current_tx_duration = 0;
	pw_received_interest = 0;
	node_state = STATE_SENSING;
	receiving_from_node_id = NODE_ID_NONE;
	receiving_packet_id = NO_PACKET_ID;

	// Set the ID of the next packet
	packet_id++;

	// Generate new BO in case of being a TX node
	if(node_is_transmitter){

		// In case of being an AP
		remaining_backoff = computeBackoff();

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s New backoff computed: %f\n",
						SimTime(), node_id, node_state, LOG_Z00, LOG_LVL3, remaining_backoff);

		trigger_DIFS.Set(SimTime() + DIFS);
		selectDestination();

		// Freeze backoff immediately if primary channel is occupied
		handleBackoff(PAUSE_TIMER);

	}

	cleanNack();
}

/*
 * printNodeInfo(): prints Node info
 * Input arguments:
 * - info_detail_level: level of detail of the info printed
 */
void Node :: printNodeInfo(int info_detail_level){

	printf("%s Node %s info:\n", LOG_LVL3, node_code);
	printf("%s node_id = %d\n", LOG_LVL4, node_id);
	printf("%s node_type = %d\n", LOG_LVL4, node_type);
	printf("%s position = (%d, %d, %d)\n", LOG_LVL4, x, y, z);
	printf("%s primary_channel = %d\n", LOG_LVL4, primary_channel);
	printf("%s min_channel_allowed = %d\n", LOG_LVL4, min_channel_allowed);
	printf("%s max_channel_allowed = %d\n", LOG_LVL4, max_channel_allowed);
	printf("%s channel_bonding_model = %d\n", LOG_LVL4, channel_bonding_model);

	if(info_detail_level > INFO_DETAIL_LEVEL_0){
		printf("%s wlan:\n", LOG_LVL4);
		printf("%s wlan code = %s\n", LOG_LVL5, wlan.wlan_code);
		printf("%s wlan id = %d\n", LOG_LVL5, wlan.wlan_id);
		printf("%s wlan AP id = %d\n", LOG_LVL5, wlan.ap_id);
		printf("%s STAs in WLAN (%d): ", LOG_LVL5, wlan.num_stas);
		wlan.printStaIds();
	}

	if(info_detail_level > INFO_DETAIL_LEVEL_1){
		printf("%s lambda = %f packets/s\n", LOG_LVL4, lambda);
		printf("%s CW_min = %d\n", LOG_LVL4, CW_min);
		printf("%s CW_max = %d\n", LOG_LVL4, CW_max);
		printf("%s destination_id = %d\n", LOG_LVL4, destination_id);
		printf("%s tpc_min = %f dBm\n", LOG_LVL4, tpc_min);
		printf("%s tpc_default = %f dBm\n", LOG_LVL4, tpc_default);
		printf("%s tpc_max = %f dBm\n", LOG_LVL4, tpc_max);
		printf("%s cca_min = %f dBm\n", LOG_LVL4, cca_min);
		printf("%s cca_default = %f dBm\n", LOG_LVL4, cca_default);
		printf("%s cca_max = %f dBm\n", LOG_LVL4, cca_max);
		printf("%s tx_gain = %f dBm\n", LOG_LVL4, tx_gain);
		printf("%s rx_gain = %f dBm\n", LOG_LVL4, rx_gain);
		printf("%s modulation_default = %d\n", LOG_LVL4, modulation_default);
		printf("%s central_frequency = %f\n", LOG_LVL4, central_frequency);
	}
	printf("\n");
}

/*
 * writeNodeInfo(): writes Node info
 * Input arguments:
 * - info_detail_level: level of detail of the info printed
 */
void Node :: writeNodeInfo(Logger node_logger, int info_detail_level, char *header_string){

	if(header_string == NULL){
		header_string = (char *) malloc(1);
		sprintf(header_string, " ");
	}

	fprintf(node_logger.file, "%s Node %s info:\n", header_string, node_code);
	fprintf(node_logger.file, "%s - node_id = %d\n", header_string, node_id);
	fprintf(node_logger.file, "%s - node_type = %d\n", header_string, node_type);
	fprintf(node_logger.file, "%s - position = (%d, %d, %d)\n", header_string, x, y, z);
	fprintf(node_logger.file, "%s - primary_channel = %d\n", header_string, primary_channel);
	fprintf(node_logger.file, "%s - min_channel_allowed = %d\n", header_string, min_channel_allowed);
	fprintf(node_logger.file, "%s - max_channel_allowed = %d\n", header_string, max_channel_allowed);
	fprintf(node_logger.file, "%s - channel_bonding_model = %d\n", header_string, channel_bonding_model);

	if(info_detail_level > INFO_DETAIL_LEVEL_0){

		wlan.writeWlanInfo(node_logger, header_string);

	}

	if(info_detail_level > INFO_DETAIL_LEVEL_1){
		fprintf(node_logger.file, "%s - lambda = %f packets/s\n", header_string, lambda);
		fprintf(node_logger.file, "%s - CW_min = %d\n", header_string, CW_min);
		fprintf(node_logger.file, "%s - CW_max = %d\n", header_string, CW_max);
		fprintf(node_logger.file, "%s - destination_id = %d\n", header_string, destination_id);
		fprintf(node_logger.file, "%s - tpc_default = %f dBm\n", header_string, tpc_default);
		fprintf(node_logger.file, "%s - cca_default = %f dBm\n", header_string, cca_default);
	}

}


/*
 * printProgressBar(): prints the progress bar of the Komondor simulation
 * Input arguments:
 * - trigger_t: trigger of the progress bar
 */
void Node :: printProgressBar(trigger_t &){

	// if(print_node_logs) printf("* %d %% *\n", progress_bar_counter * PROGRESS_BAR_DELTA);
	printf("* %d %% *\n", progress_bar_counter * PROGRESS_BAR_DELTA);
	trigger_sim_time.Set(SimTime() + simulation_time_komondor / (100/PROGRESS_BAR_DELTA));

	// End progress bar
	if(node_id == 0 && progress_bar_counter == (100/PROGRESS_BAR_DELTA)-1){
		trigger_sim_time.Set(SimTime() + simulation_time_komondor/(100/PROGRESS_BAR_DELTA) - MIN_VALUE_C_LANGUAGE);
	}
	progress_bar_counter ++;
}

/*
 * initializeVariables(): initializes all the necessary variables
 */
void Node :: initializeVariables() {

	// Output file - logger
	node_logger.save_logs = save_node_logs;
	node_logger.file = node_logger.file;

	// Arrays and other
	channel_power = (double *) malloc(num_channels_komondor * sizeof(*channel_power));
	num_channels_allowed = (max_channel_allowed - min_channel_allowed + 1);
	total_time_transmitting_per_channel = (double *) malloc(num_channels_komondor
			* sizeof(*total_time_transmitting_per_channel));
	channels_free = (int *) malloc(num_channels_komondor * sizeof(*channels_free));
	channels_for_tx = (int *) malloc(num_channels_komondor * sizeof(*channels_for_tx));
	total_time_lost_per_channel = (double *) malloc(num_channels_komondor
			* sizeof(*total_time_lost_per_channel));
	for(int i = 0; i < num_channels_komondor; i++){
		channel_power[i] = 0;
		total_time_transmitting_per_channel[i] = 0;
		channels_free[i] = FALSE;
		channels_for_tx[i] = FALSE;
		total_time_lost_per_channel[i] = 0;
	}

	total_time_transmitting_in_num_channels = (double *) malloc(num_channels_allowed
			* sizeof(*total_time_transmitting_in_num_channels));
	total_time_lost_in_num_channels = (double *) malloc(num_channels_allowed
			* sizeof(*total_time_lost_in_num_channels));
	for(int i = 0; i < num_channels_allowed; i++){
		total_time_transmitting_in_num_channels[i] = 0;
		total_time_lost_in_num_channels[i] = 0;
	}

	power_received_per_node = (double *) malloc(total_nodes_number * sizeof(*power_received_per_node));
	nodes_transmitting = (int *) malloc(total_nodes_number * sizeof(*nodes_transmitting));
	for(int n = 0; n < total_nodes_number; n++){
		power_received_per_node[n] = 0;
		nodes_transmitting[n] = FALSE;
	}

	// List of hidden nodes (1 indicates hidden nodes, 0 indicates the opposite)
	hidden_nodes_list = (int *) malloc(total_nodes_number * sizeof(*hidden_nodes_list));
	// Counter for the times a node was implied in a collision by hidden node
	potential_hidden_nodes = (int *) malloc(total_nodes_number * sizeof(*potential_hidden_nodes));
	for(int i = 0; i < total_nodes_number; i++){
		hidden_nodes_list[i] = FALSE;
		potential_hidden_nodes[i] = 0;
	}
	potential_hidden_nodes[node_id] = -1; // To indicate that the node cannot be hidden from itself
	nacks_received = (int *) malloc(7 * sizeof(*nacks_received));
	for(int i = 0; i < 7; i++){
		nacks_received[i] = 0;
	}

	// Rest of variables
	pw_received_interest = 0;
	progress_bar_counter = 0;
	current_left_channel =  min_channel_allowed;
	current_right_channel = max_channel_allowed;
	current_tpc = tpc_default;
	current_cca = cca_default;
	node_state = STATE_SENSING;
	current_modulation = modulation_default;
	current_CW = CW_min;

	if(node_type == NODE_TYPE_AP) {
		node_is_transmitter = TRUE;
	} else {
		node_is_transmitter = FALSE;
	}

	// Statistics
	packets_sent = 0;
	throughput = 0;
	throughput_loss = 0;
	packets_lost = 0;
	num_tx_init_not_possible = 0;

	// Modulation data rates

	// Fixed data rates (CTMN - Matlab)
	data_rate_array[0] = 81.5727 * packet_length * num_packets_aggregated;	// 1 channel
	data_rate_array[1] = 150.8068 * packet_length * num_packets_aggregated; // 2 channels
	data_rate_array[2] = 0;
	data_rate_array[3] = 215.7497 * packet_length * num_packets_aggregated;	// 4 channels
	data_rate_array[4] = 0;
	data_rate_array[5] = 0;
	data_rate_array[6] = 0;
	data_rate_array[7] = 284.1716 * packet_length * num_packets_aggregated; // 8 channels

	default_modulation = MODULATION_NONE;

	int modulation_rates_aux[4][12] = {	// rows: modulation type, colums: number of channels (1, 2, 4, 8)
		{4,16,24,33,49,65,73,81,98,108,122,135},
		{8,33,49,65,98,130,146,163,195,217,244,271},
		{17,68,102,136,204,272,306,340,408,453,510,567},
		{34,136,204,272,408,544,613,681,817,907,1021,1134}
	};

	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 12; j++){
			modulation_rates[i][j] = modulation_rates_aux[i][j] * pow(10,6);
		}
	}

	coding_rate_modulation[0] = 1/double(2);
	coding_rate_modulation[1] = 1/double(2);
	coding_rate_modulation[2] = 3/double(4);
	coding_rate_modulation[3] = 1/double(2);
	coding_rate_modulation[4] = 3/double(4);
	coding_rate_modulation[5] = 2/double(3);
	coding_rate_modulation[6] = 3/double(4);
	coding_rate_modulation[7] = 5/double(6);
	coding_rate_modulation[8] = 3/double(4);
	coding_rate_modulation[9] = 5/double(6);
	coding_rate_modulation[10] = 3/double(4);
	coding_rate_modulation[11] = 5/double(6);

	bits_per_symbol_modulation[0] = 2;
	bits_per_symbol_modulation[1] = 4;
	bits_per_symbol_modulation[2] = 4;
	bits_per_symbol_modulation[3] = 16;
	bits_per_symbol_modulation[4] = 16;
	bits_per_symbol_modulation[5] = 64;
	bits_per_symbol_modulation[6] = 64;
	bits_per_symbol_modulation[7] = 64;
	bits_per_symbol_modulation[8] = 256;
	bits_per_symbol_modulation[9] = 256;
	bits_per_symbol_modulation[10] = 1024;
	bits_per_symbol_modulation[11] = 1024;

	mcs_response = (int *) malloc(4 * sizeof(int));
	for(int n = 0; n < 4; n++){
		mcs_response[n] = 0;
	}

	int *modulations_list = (int*)calloc(4, sizeof(int));
	mcs_per_node = (int**)calloc(wlan.num_stas, sizeof(int*));
	change_modulation_flag = (int *) malloc(wlan.num_stas * sizeof(int));
	for(int n = 0; n < wlan.num_stas; n++){
		mcs_per_node[n] = modulations_list;
		change_modulation_flag[n] = TRUE;
	}

}

/*
 * printOrWriteChannelPower: prints (or writes) the channel_power array representing the power sensed by
 * the node in each subchannel.
 */
void Node :: printOrWriteChannelPower(int write_or_print){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("channel_power [dBm]: ");
				for(int c = 0; c < num_channels_komondor; c++){
					printf("%f  ", convertPower(PICO_TO_DBM, channel_power[c]));
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < num_channels_komondor; c++){
				 if(save_node_logs) fprintf(node_logger.file, "%f  ", convertPower(PICO_TO_DBM, channel_power[c]));
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

/*
 * printOrWriteChannelsFree: prints (or writes) the channels_free array representing the channels that are free.
 */
void Node :: printOrWriteChannelsFree(int write_or_print){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("channels_free: ");
				for(int c = 0; c < num_channels_komondor; c++){
					printf("%d  ", channels_free[c]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < num_channels_komondor; c++){
				 if(save_node_logs) fprintf(node_logger.file, "%d ", channels_free[c]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

/*
 * printOrWriteNodesTransmitting: prints (or writes) the array representing the transmitting nodes.
 */
void Node :: printOrWriteNodesTransmitting(int write_or_print){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("Nodes transmitting: ");
				for(int n = 0; n < total_nodes_number; n++){
					printf("%d  ", nodes_transmitting[n]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int n = 0; n < total_nodes_number; n++){
				 if(save_node_logs){
					 if(nodes_transmitting[n])  fprintf(node_logger.file, "N%d ", n);
				 }
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

/*
 * printOrWriteChannelForTx: prints (or writes) the channels_for_tx array representing the channels used for TX
 */
void Node :: printOrWriteChannelForTx(int write_or_print){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("channels_for_tx: ");
				for(int c = 0; c < num_channels_komondor; c++){
					printf("%d  ", channels_for_tx[c]);
					// printf("%d  ", channels_for_tx[c]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < num_channels_komondor; c++){
				 if(save_node_logs)  fprintf(node_logger.file, "%d  ", channels_for_tx[c]);
				// printf("%d  ", channels_for_tx[c]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

/*
 * printOrWriteNodeStatistics(): prints (or writes) final statistics at the given node
 */
void Node :: printOrWriteNodeStatistics(int write_or_print){

	// Process statistics
	double packets_lost_percentage = 0;
	if (packets_sent > 0) {
		packets_lost_percentage = double(packets_lost * 100)/double(packets_sent);
	}
	throughput = (((double)(packets_sent-packets_lost) * packet_length * num_packets_aggregated / 1000000))
			/SimTime();
	int hidden_nodes_number = 0;
	for(int i = 0; i < total_nodes_number; i++){
		if(hidden_nodes_list[i] == 1) hidden_nodes_number++;
	}

	switch(write_or_print){

		case PRINT_LOG:{

			if (node_is_transmitter && print_node_logs) {
				printf("------- %s (N%d) ------\n", node_code, node_id);

				// Throughput
				printf("%s Throughput = %f Mbps\n", LOG_LVL2, throughput);

				// Packets sent and lost
				printf("%s Packets sent = %d - Packets lost = %d  (%f %% lost)\n",
						LOG_LVL2, packets_sent, packets_lost, packets_lost_percentage);

				// Time EFFECTIVELY transmitting in a given number of channels (no losses)
				printf("%s Time EFFECTIVELY transmitting in N channels: ", LOG_LVL3);
				for(int n = 0; n < num_channels_allowed; n++){
					printf("(%d) %f  ", n+1, total_time_transmitting_in_num_channels[n] -
							total_time_lost_in_num_channels[n]);
				}
				printf("\n");

				// Time EFFECTIVELY transmitting in each of the channels (no losses)
				printf("%s Time EFFECTIVELY transmitting in each channel: ", LOG_LVL3);
				for(int c = 0; c < num_channels_komondor; c++){
					printf("(N%d) %f ", c, total_time_transmitting_per_channel[c] -
							total_time_lost_per_channel[c]);
				}
				printf("\n");

				// Time LOST transmitting in a given number of channels
				printf("%s Time LOST transmitting in N channels: ", LOG_LVL3);
				for(int n = 0; n < num_channels_allowed; n++){
					printf("(%d) %f  ", n+1, total_time_lost_in_num_channels[n]);
				}
				printf("\n");

				// Time LOST transmitting in each of the channels
				printf("%s Time LOST transmitting in each channel: ", LOG_LVL3);
				for(int c = 0; c < num_channels_komondor; c++){
					printf("(N%d) %f ", c, total_time_lost_per_channel[c]);
				}
				printf("\n");

				// Number of TX initiations that have been not possible due to channel state and DCB model
				printf("%s num_tx_init_not_possible = %d\n", LOG_LVL2, num_tx_init_not_possible);

				// Hidden nodes
				printf("%s Total number of hidden nodes: %d\n", LOG_LVL2, hidden_nodes_number);
				printf("%s Hidden nodes list: ", LOG_LVL2);
				for(int i = 0; i < total_nodes_number; i++){
					printf("%d  ", hidden_nodes_list[i]);
				}
				printf("\n");

				printf("%s Times a node was implied in a collision by hidden node: ",LOG_LVL2);
				for(int i=0; i < total_nodes_number; i++) {
					printf("%d ", potential_hidden_nodes[i]);
				}

				printf("\n\n");
			}
			break;
		}

		case WRITE_LOG:{

			if (save_node_logs){

				if (node_is_transmitter) {
					// Throughput
					fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Throughput = %f Mbps\n",
							SimTime(), node_id, node_state, LOG_C02, LOG_LVL2, throughput);

					// Packets sent and lost
					fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s Packets sent: %d\n",
							SimTime(), node_id, node_state, LOG_C03, LOG_LVL2, packets_sent);
					fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s Packets lost: %d\n",
							SimTime(), node_id, node_state, LOG_C04, LOG_LVL2, packets_lost);
					fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s Loss ratio: %f\n",
							SimTime(), node_id, node_state, LOG_C05, LOG_LVL2, packets_lost_percentage);

					// Time EFFECTIVELY transmitting in a given number of channels (no losses)
					fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Time EFFECTIVELY transmitting in N channels: ",
							SimTime(), node_id, node_state, LOG_C06, LOG_LVL2);
					for(int n = 0; n < num_channels_allowed; n++){
						fprintf(node_logger.file, "(%d) %f  ",
								n+1, total_time_transmitting_in_num_channels[n] - total_time_lost_in_num_channels[n]);
					}
					fprintf(node_logger.file, "\n");

					// Time EFFECTIVELY transmitting in each of the channels (no losses)
					fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s Time EFFECTIVELY transmitting in each channel: ",
							SimTime(), node_id, node_state, LOG_C07, LOG_LVL2);
					for(int c = 0; c < num_channels_komondor; c++){
						fprintf(node_logger.file, "(#%d) %f ",
								c, total_time_transmitting_per_channel[c] - total_time_lost_per_channel[c]);
					}
					fprintf(node_logger.file, "\n");

					// Time LOST transmitting in a given number of channels
					fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Time LOST transmitting in N channels: ",
							SimTime(), node_id, node_state, LOG_C08, LOG_LVL2);
					for(int n = 0; n < num_channels_allowed; n++){
						fprintf(node_logger.file, "(%d) %f  ", n+1, total_time_lost_in_num_channels[n]);
					}
					fprintf(node_logger.file, "\n");

					// Time LOST transmitting in each of the channels
					fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Time LOST transmitting in each channel: ",
							SimTime(), node_id, node_state, LOG_C09, LOG_LVL2);
					for(int c = 0; c < num_channels_komondor; c++){
						fprintf(node_logger.file, "(#%d) %f ", c, total_time_lost_per_channel[c]);
					}
					fprintf(node_logger.file, "\n");

					// Number of TX initiations that have been not possible due to channel state and DCB model
					fprintf(node_logger.file, "%f;N%d;S%d;%s;%s num_tx_init_not_possible = %d\n",
							SimTime(), node_id, node_state, LOG_C09, LOG_LVL2, num_tx_init_not_possible);

				}

				// Hidden nodes
				int hidden_nodes_number = 0;
				for(int n = 0; n < total_nodes_number; n++){
					if(hidden_nodes_list[n]) hidden_nodes_number++;
				}
				fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Total hidden nodes: %d\n",
						SimTime(), node_id, node_state, LOG_C10, LOG_LVL2, hidden_nodes_number);

				fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Hidden nodes list: ",
						SimTime(), node_id, node_state, LOG_C11, LOG_LVL2);
				for(int i = 0; i < total_nodes_number; i++){
					fprintf(node_logger.file, "%d  ", hidden_nodes_list[i]);
				}
			}
			break;
		}
	}
}
