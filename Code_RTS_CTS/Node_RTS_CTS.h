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
 * File description: Node is a component representing any device in the
 * network (i.e., AP, STA, etc.). Node contains all the logic and algorithms
 * for simulating nodes operation.
 *
 * - Bla bla bla...
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../methods/BackoffMethods.h"
#include "../methods/PowerManagementMethods.h"
#include "../methods/TXTimeMethods.h"
#include "../methods/AuxiliaryMethods.h"
#include "../methods/ModulationsMethods.h"
#include "../methods/Modulations.h"

#include "../ListOfDefines.h"
#include "../methods/NotificationMethods.h"

#include "../structures/Notification.h"
#include "../structures/LogicalNack.h"
#include "../structures/Wlan.h"
#include "../structures/Logger.h"

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

		// Packets
		Notification generateNotification(int packet_type, int destination_id, double tx_duration);
		void selectDestination();
		void sendResponse();
		void ackTimeout();
		void ctsTimeout();
		void dataTimeout();
		void navTimeout();
		void requestMCS();
		void newPacketGenerated();
		void trafficGenerator();

		// NACK
		void sendLogicalNack(LogicalNack logical_nack);
		void cleanNack();

		// Backoff
		void pauseBackoff();
		void resumeBackoff();

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
		double tpc_min;				// Min. power transmission [pW]
		double tpc_default;			// Default power transmission [pW]
		double tpc_max;				// Max. power transmission [pW]
		double cca_min;				// Min. CCA	("sensitivity" threshold) [pW]
		double cca_default;			// Default CCA	("sensitivity" threshold) [pW]
		double cca_max;				// Max. CCA ("sensitivity" threshold)
		double tx_gain;				// Antenna transmission gain [linear]
		double rx_gain;				// Antenna reception gain [linear]
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
		double capture_effect;				// Capture effect threshold [linear ratio]
		double constant_PER;				// Constant PER for correct transmissions
		int save_node_logs;					// Flag for activating the log writting of nodes
		int print_node_logs;				// Flag for activating the printing of node logs
		char *simulation_code;				// Simulation code

		// Channel
		int basic_channel_bandwidth;		// Channel unit bandwidth [Hz]
		int num_channels_komondor;			// Number of subchannels composing the whole channel
		int cochannel_model;				// Co-channel interference model (definition of models in function updateChannelsPower())

		// Transmissions
		int default_destination_id;			// Current destination node ID
		double noise_level;					// Environment noise [pW]
		int current_modulation;				// Current_modulation used by nodes
		int channel_max_intereference;		// Channel of interest suffering maximum interference
		double SIFS;						// SIFS [s]
		double DIFS;						// DIFS [s]
		double central_frequency;			// Central frequency (Hz)
		int CW_min;							// Backoff minimum Contention Window
		int CW_max;							// Backoff maximum Contention Window
		int pdf_backoff;					// Probability distribution type of the backoff (0: exponential, 1: deterministic)
		int path_loss_model;				// Path loss model (0: free-space, 1: Okumura-Hata model - Uban areas)

		// Data rate - modulations
		int modulation_rates[4][12];		// Modulation rates in bps used in IEEE 802.11ax
		int err_prob_modulation[4][12];		// BER associated to each modulation (TO BE FILLED!!)

		// Packets
		int pdf_tx_time;					// Probability distribution type of the transmission time (0: exponential, 1: deterministic)
		int packet_length;					// Notification length [bits]
		int num_packets_aggregated;			// Number of packets aggregated in one transmission
		int ack_length;						// ACK length [bits]
		int rts_length;						// RTS length [bits]
		int cts_length;						// CTS length [bits]
		int traffic_model;					// Traffic model (0: full buffer, 1: poisson, 2: deterministic)
		int backoff_type;				// Type of Backoff (0: Slotted 1: Continuous)

	// Statistics (accessible when simulation finished through Komondor simulation class)
	public:

		int packets_sent;
		int rts_cts_sent;
		double *total_time_transmitting_per_channel;		// Time transmitting per channel;
		double *total_time_transmitting_in_num_channels;	// Time transmitting in (ix 0: 1 channel, ix 1: 2 channels...)
		double *total_time_lost_per_channel;				// Time transmitting per channel unsuccessfully;
		double *total_time_lost_in_num_channels;			// Time transmitting in (ix 0: 1 channel, ix 1: 2 channels...) unsuccessfully
		double throughput;									// Throughput [Mbps]
		double throughput_loss;								// Throughput of lost packets [Mbps]
		int packets_lost;									// Own packets that have been collided or lost
		int rts_cts_lost;
		int *nacks_received;								// Counter of the type of Nacks received
		int num_tx_init_not_possible;						// Number of TX initiations that have been not possible due to channel state and DCB model

	// Private items (just for node operation)
	private:

		// Komondor environment
		double *channel_power;				// Channel power detected in each sub-channel [pW] (pico watts for resolution issues)
		int *channels_free;					// Channels that are found free for the beginning TX (i.e. power sensed < CCA)
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
		double current_nav_time;					// Current NAV duration
		int packet_id;						// Notification ID
		int rts_cts_id;						// Id for RTS/CTS transactions
		double current_sinr;				// SINR perceived in current TX [lienar ratio]
		Notification data_notification;		// DATA notification to be filled before sending it
		Notification ack_notification;		// ACK to be filled before sending it
		Notification ongoing_notification; 	// Current notification (DATA or ACK) being received
		Notification rts_notification;		// RTS to be filled before sending it
		Notification cts_notification;		// CTS to be filled before sending it

		int default_modulation;				// Default MCS identifier
		double current_data_rate;			// Data rate being used currently
		int current_CW;						// Congestion Window being used currently

		double data_duration;
		double ack_duration;
		double rts_duration;
		double cts_duration;

		int **mcs_per_node;			// Modulation selected for each of the nodes (only transmitting nodes)
		int *change_modulation_flag;		// Flag for changig the MCS of any of the potential receivers
		int *mcs_response;					// MCS response received from receiver

		// Sensing and Reception parameters
		LogicalNack nack;					// NACK to be filled in case node is the destination of tx loss
		double max_pw_interference;			// Maximum interference detected in range of interest [pW]
		int channel_max_interference;		// Channel of maximum interference detected in range of interest [pW]
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
		int num_packets_in_buffer;			// Number of packets in the buffer pending to be transmitted
		double *timestampt_channel_becomes_free;	// Timestamp when channel becomes free (when P(channel) < CCA)

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
		Timer <trigger_t> trigger_backoff; 				// Duration of current trigger_backoff. Triggers outportSelfStartTX()
		Timer <trigger_t> trigger_toFinishTX; 			// Duration of current notification transmission. Triggers outportSelfFinishTX()
		Timer <trigger_t> trigger_sim_time;				// Timer for displaying the exectuion time status (progress bar)
		Timer <trigger_t> trigger_DIFS;					// Timer for the DIFS
		Timer <trigger_t> trigger_SIFS;					// Timer for the SIFS
		Timer <trigger_t> trigger_ACK_timeout;			// Trigger when ACK hasn't arrived in time
		Timer <trigger_t> trigger_CTS_timeout;			// Trigger when CTS hasn't arrived in time
		Timer <trigger_t> trigger_DATA_timeout; 		// Trigger when DATA TX could not start due to RTS/CTS failure
		Timer <trigger_t> trigger_NAV_timeout;  		// Trigger for the NAV
		Timer <trigger_t> trigger_new_packet_generated; // Trigger for new packets generation

		// Every time the timer expires execute this
		inport inline void endBackoff(trigger_t& t1);
		inport inline void myTXFinished(trigger_t& t1);
		inport inline void printProgressBar(trigger_t& t1);
		inport inline void resumeBackoff(trigger_t& t1);
		inport inline void sendResponse(trigger_t& t1);
		inport inline void ackTimeout(trigger_t& t1);
		inport inline void ctsTimeout(trigger_t& t1);
		inport inline void dataTimeout(trigger_t& t1);
		inport inline void navTimeout(trigger_t& t1);
		inport inline void newPacketGenerated(trigger_t& t1);

		// Connect timers to methods
		Node () {
			connect trigger_backoff.to_component,endBackoff;
			connect trigger_toFinishTX.to_component,myTXFinished;
			connect trigger_sim_time.to_component,printProgressBar;
			connect trigger_DIFS.to_component,resumeBackoff;
			connect trigger_SIFS.to_component,sendResponse;
			connect trigger_ACK_timeout.to_component,ackTimeout;
			connect trigger_CTS_timeout.to_component,ctsTimeout;
			connect trigger_DATA_timeout.to_component,dataTimeout;
			connect trigger_NAV_timeout.to_component,navTimeout;
			connect trigger_new_packet_generated.to_component,newPacketGenerated;
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
		node_logger.setVoidHeadString();
	}

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Start()\n",
			SimTime(), node_id, STATE_UNKNOWN, LOG_B00, LOG_LVL1);

	// Write node info
	header_string = (char *) malloc(LOG_HEADER_NODE_SIZE);
	sprintf(header_string, "%f;N%d;S%d;%s;%s", SimTime(), node_id, node_state, LOG_Z00, LOG_LVL3);
	if(save_node_logs) writeNodeInfo(node_logger, INFO_DETAIL_LEVEL_2, header_string);

	// Initialize variables
	initializeVariables();

	// Start backoff procedure only if node is able to transmit
	if(node_is_transmitter) {
		trafficGenerator();
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

	if(save_node_logs) printOrWriteNodesTransmitting(WRITE_LOG, save_node_logs,
			print_node_logs, node_logger, total_nodes_number, nodes_transmitting);

	if(notification.source_id == node_id){ // If OWN NODE IS THE TRANSMITTER, do nothing

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s I have started a TX of packet #%d (type %d) to N%d in channels %d - %d\n",
				SimTime(), node_id, node_state, LOG_D02, LOG_LVL2, notification.tx_info.packet_id,
				notification.packet_type, notification.tx_info.destination_id,
				notification.left_channel, notification.right_channel);

	} else {	// If OTHER NODE IS THE TRANSMITTER

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s N%d has started a TX of packet #%d (type %d) to N%d in channels %d - %d\n",
				SimTime(), node_id, node_state, LOG_D02, LOG_LVL2, notification.source_id,
				notification.tx_info.packet_id,	notification.packet_type, notification.tx_info.destination_id,
				notification.left_channel, notification.right_channel);

		// Call updatePowerReceivedPerNode() ONLY for adding power (some node started)
		updatePowerReceivedPerNode(power_received_per_node, notification, x, y, z,
				rx_gain, central_frequency, path_loss_model, SimTime(), node_logger,
				save_node_logs, node_id, node_state);

		// Update the power sensed at each channel
		updateChannelsPower(channel_power, power_received_per_node, notification, TX_INITIATED,
				SimTime(), node_logger, save_node_logs, node_id, node_state, central_frequency,
				num_channels_komondor, path_loss_model, cochannel_model);

		updateTimestamptChannelFree(timestampt_channel_becomes_free, channel_power,
				current_cca, num_channels_komondor, SimTime());

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

					// Update power received of interest
					pw_received_interest = power_received_per_node[notification.source_id];
					// Save ongoing notification information
					ongoing_notification = notification;

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
							SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, notification.tx_info.destination_id);

					if(notification.packet_type == PACKET_TYPE_RTS) {	// Notification COTAINS an RTS PACKET

						current_sinr = updateSINR(pw_received_interest, noise_level, max_pw_interference,
								save_node_logs, &node_logger, SimTime(), node_id, node_state);

						// Check if notification has been lost due to interferences or weak signal strength
						loss_reason = isPacketLost(notification, current_sinr,
								capture_effect, current_cca, pw_received_interest,
								constant_PER, hidden_nodes_list, save_node_logs, node_logger, node_id,
								node_state, SimTime());

						if(loss_reason != PACKET_NOT_LOST) {	// If RTS IS LOST, send logical Nack

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
									SimTime(), node_id, node_state, LOG_D15, LOG_LVL4, notification.tx_info.packet_id,
									notification.source_id, loss_reason);

							// Send logical NACK to transmitter sending incoming notification for indicating the loss reason
							LogicalNack logical_nack = generateLogicalNack(notification.packet_type, notification.tx_info.packet_id,
									node_id, notification.source_id, NODE_ID_NONE, loss_reason, BER, current_sinr);

							sendLogicalNack(logical_nack);

							if(node_is_transmitter){

								int pause = handleBackoff(PAUSE_TIMER, SimTime(), save_node_logs,
										node_logger, node_id, node_state, channel_power, primary_channel,
										current_cca, num_packets_in_buffer);

								// Check if node has to freeze the BO (if it is not already frozen)
								if (pause) pauseBackoff();
							}

						} else {	// Data packet IS NOT LOST (it can be properly received)

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s Reception of RTS %d from N%d CAN be started (SINR = %f dB)\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.tx_info.packet_id,
									notification.source_id, convertPower(LINEAR_TO_DB, current_sinr));

							// Change state and update receiving info
							data_duration = notification.tx_info.data_duration;
							ack_duration = notification.tx_info.ack_duration;
							rts_duration = notification.tx_info.rts_duration;
							cts_duration = notification.tx_info.cts_duration;

							current_left_channel = notification.left_channel;
							current_right_channel = notification.right_channel;

							node_state = STATE_RX_RTS;
							receiving_from_node_id = notification.source_id;
							receiving_packet_id = notification.tx_info.packet_id;

							// Pause backoff as node has began a reception
							if(node_is_transmitter) pauseBackoff();

						}

					} else {	//	Notification does NOT CONTAIN an RTS
						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s Unexpected packet type (%d) received!\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL4, notification.packet_type);
					}

				} else {	// Node IS NOT THE DESTINATION

					pw_received_interest = power_received_per_node[notification.source_id];

					if(notification.packet_type == PACKET_TYPE_RTS
							|| notification.packet_type == PACKET_TYPE_CTS){

						current_sinr = updateSINR(pw_received_interest, noise_level, max_pw_interference,
								save_node_logs, &node_logger, SimTime(), node_id, node_state);

						loss_reason = isPacketLost(notification, current_sinr,
								capture_effect, current_cca, pw_received_interest,
								constant_PER, hidden_nodes_list, save_node_logs, node_logger, node_id,
								node_state, SimTime());


						if(loss_reason == PACKET_NOT_LOST && channel_power[primary_channel] > current_cca) { // RTS/CTS affecting my BO

							if(node_is_transmitter){

								int pause = handleBackoff(PAUSE_TIMER, SimTime(), save_node_logs,
										node_logger, node_id, node_state, channel_power, primary_channel,
										current_cca, num_packets_in_buffer);

								// Check if node has to freeze the BO (if it is not already frozen)
								if (pause) pauseBackoff();
							}

							current_nav_time = notification.tx_info.nav_time;
							node_state = STATE_NAV;
							trigger_NAV_timeout.Set(SimTime() + (1 + MIN_VALUE_C_LANGUAGE)
									* current_nav_time);

							if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s RTS/CTS received from N%d. Setting NAV timeout to %f.\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL3,
								notification.tx_info.destination_id, notification.tx_info.nav_time);
						}

					} else if (notification.packet_type == PACKET_TYPE_DATA ||
							   notification.packet_type == PACKET_TYPE_ACK){

						if(node_is_transmitter){

							int pause = handleBackoff(PAUSE_TIMER, SimTime(), save_node_logs,
									node_logger, node_id, node_state, channel_power, primary_channel,
									current_cca, num_packets_in_buffer);

							// Check if node has to freeze the BO (if it is not already frozen)
							if (pause) pauseBackoff();
						}

					}

				}

				break;
			}

			/* STATE_TX_NAV
			 * - If node IS destination: generateNack to incoming notification transmitter
			 * - If node IS NOT destination: check if NAV trigger must be updated
			 */
			case STATE_NAV:{

				if(notification.tx_info.destination_id == node_id){	// Node IS THE DESTINATION

					// Send logical NACK to transmitter sending incoming notification for indicating the loss reason
					LogicalNack logical_nack = generateLogicalNack(notification.packet_type, notification.tx_info.packet_id,
							node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_RX_IN_NAV, BER, current_sinr);

					sendLogicalNack(logical_nack);

				} else {
					if(notification.packet_type == PACKET_TYPE_RTS ||
							notification.packet_type == PACKET_TYPE_CTS) {

						// Check if affects me
						current_sinr = updateSINR(pw_received_interest, noise_level, max_pw_interference,
								save_node_logs, &node_logger, SimTime(), node_id, node_state);
						loss_reason = isPacketLost(notification, convertPower(LINEAR_TO_DB, current_sinr),
								capture_effect, current_cca, convertPower(PICO_TO_DBM, pw_received_interest),
								constant_PER, hidden_nodes_list, save_node_logs, node_logger, node_id,
								node_state, SimTime());

						if(loss_reason == PACKET_NOT_LOST &&
							convertPower(PICO_TO_DBM, channel_power[primary_channel]) > current_cca) {
							// Update NAV trigger
							if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s New RTS/CTS arrived from (N%d). Setting NAV to new value %f\n",
								SimTime(), node_id, node_state, LOG_D07, LOG_LVL3,
								notification.source_id, notification.tx_info.nav_time);

							current_nav_time = notification.tx_info.nav_time;
							trigger_NAV_timeout.Set(SimTime() + (1 + MIN_VALUE_C_LANGUAGE)
									* current_nav_time);
						}
					}
				}


				break;
			}

			/* STATE_TX_DATA || STATE_TX_ACK:
			 * - If node IS destination: generateNack to incoming notification transmitter
			 * - If node IS NOT destination: do nothing
			 */
			case STATE_TX_DATA:
			case STATE_TX_ACK:
			case STATE_TX_RTS:
			case STATE_TX_CTS:{

				if(notification.tx_info.destination_id == node_id){ // Node IS THE DESTINATION

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
							SimTime(), node_id, node_state, LOG_D07, LOG_LVL3,
							notification.tx_info.destination_id);

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s I am transmitting, packet cannot be received\n",
							SimTime(), node_id, node_state, LOG_D18, LOG_LVL3);

					// Send logical NACK to incoming notification transmitter due to receiver (node) was already receiving
					LogicalNack logical_nack = generateLogicalNack(notification.packet_type, notification.tx_info.packet_id,
							node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_DESTINATION_TX, BER, current_sinr);

					sendLogicalNack(logical_nack);

				} else {	// Node IS NOT THE DESTINATION, do nothing

					if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL3, notification.tx_info.destination_id);

				}
				break;
			}

			/* STATE_RX_DATA || STATE_RX_ACK:
			 * - If node IS destination: generateNack to incoming notification transmitter
			 * - Check if ongoing packet reception is lost:
			 *   * If packet lost: generate NACK to source of ongoing reception and restart node
			 *   * If packet NOT lost: do nothing
			 */
			case STATE_RX_DATA:
			case STATE_RX_ACK:
			case STATE_RX_RTS:
			case STATE_RX_CTS:{

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
					current_sinr = updateSINR(pw_received_interest, noise_level, max_pw_interference,
							save_node_logs, &node_logger, SimTime(), node_id, node_state);
					loss_reason = isPacketLost(notification, convertPower(LINEAR_TO_DB, current_sinr),
							capture_effect, current_cca, convertPower(PICO_TO_DBM, pw_received_interest),
							constant_PER, hidden_nodes_list, save_node_logs, node_logger, node_id,
							node_state, SimTime());

					if(loss_reason != PACKET_NOT_LOST) {	// If ongoing data packet IS LOST

							// Pure collision (two nodes transmitting to me with enough power)
							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s Pure collision! Already receiving from N%d\n",
									SimTime(), node_id, node_state, LOG_D19, LOG_LVL4, receiving_from_node_id);


							if(ongoing_notification.timestampt == notification.timestampt){
								loss_reason = PACKET_LOST_BO_COLLISION;
							}

							// Send NACK to both ongoing transmitter and incoming interferer nodes
							LogicalNack logical_nack = generateLogicalNack(notification.packet_type, ongoing_notification.tx_info.packet_id,
									node_id, ongoing_notification.source_id, notification.source_id, PACKET_LOST_PURE_COLLISION, BER, current_sinr);

							sendLogicalNack(logical_nack);

							restartNode();

					} else {	// If ongoing data packet IS NOT LOST (incoming transmission does not affect ongoing reception)

						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s Low strength signal received while already receiving from N%d\n",
							SimTime(), node_id, node_state, LOG_D20, LOG_LVL4, receiving_from_node_id);

						// Send logical NACK to incoming transmitter indicating that node is already receiving
						LogicalNack logical_nack = generateLogicalNack(notification.packet_type, receiving_from_node_id,
								node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_LOW_SIGNAL_AND_RX, BER, current_sinr);

						sendLogicalNack(logical_nack);

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

					// Compute max interference (the highest one perceived in the reception channel range)
					max_pw_interference = computeMaxInterference(notification,
							current_left_channel, current_right_channel,
							node_state, node_id, save_node_logs, node_logger,
							SimTime(), power_received_per_node,
							receiving_from_node_id, channel_power);

					// Check if the ongoing reception is affected
					current_sinr = updateSINR(pw_received_interest, noise_level, max_pw_interference,
							save_node_logs, &node_logger, SimTime(), node_id, node_state);
					loss_reason = isPacketLost(notification, convertPower(LINEAR_TO_DB, current_sinr),
							capture_effect, current_cca, convertPower(PICO_TO_DBM, pw_received_interest),
							constant_PER, hidden_nodes_list, save_node_logs, node_logger, node_id,
							node_state, SimTime());

					if(loss_reason != PACKET_NOT_LOST) { 	// If ongoing packet reception IS LOST

						// Collision by hidden node
						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s Collision by interferences!\n",
								SimTime(), node_id, node_state, LOG_D19, LOG_LVL4);

						// Send logical NACK to ongoing transmitter
						LogicalNack logical_nack = generateLogicalNack(ongoing_notification.packet_type, ongoing_notification.tx_info.packet_id,
								node_id, receiving_from_node_id, NODE_ID_NONE, PACKET_LOST_INTERFERENCE, BER, current_sinr);

						sendLogicalNack(logical_nack);

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
						current_sinr = updateSINR(pw_received_interest, noise_level, max_pw_interference,
								save_node_logs, &node_logger, SimTime(), node_id, node_state);
						loss_reason = isPacketLost(notification, convertPower(LINEAR_TO_DB, current_sinr),
								capture_effect, current_cca, convertPower(PICO_TO_DBM, pw_received_interest),
								constant_PER, hidden_nodes_list, save_node_logs, node_logger, node_id,
								node_state, SimTime());
						if(loss_reason != PACKET_NOT_LOST) {	// If ACK packet IS LOST, send logical Nack

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
									SimTime(), node_id, node_state, LOG_D15, LOG_LVL4, notification.tx_info.packet_id,
									notification.source_id, loss_reason);

							// Send logical NACK to ACK transmitter
							LogicalNack logical_nack = generateLogicalNack(ongoing_notification.packet_type, ongoing_notification.tx_info.packet_id,
									node_id, receiving_from_node_id, NODE_ID_NONE, loss_reason, BER, current_sinr);

							sendLogicalNack(logical_nack);

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
									"%f;N%d;S%d;%s;%s current_sinr = %f dB\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL5,
									convertPower(LINEAR_TO_DB,current_sinr));

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

			/* STATE_WAIT_CTS:
			 * - If node IS destination and packet is CTS:
			 *   * Check if CTS packet can be received:

			 * - If node IS NOT destination: do nothing
			 */
			case STATE_WAIT_CTS:{

				if(notification.tx_info.destination_id == node_id){	// Node is the destination

					pw_received_interest = power_received_per_node[notification.source_id];
					ongoing_notification = notification;

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
							SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, notification.tx_info.destination_id);

					if(notification.packet_type == PACKET_TYPE_CTS){	// CTS packet transmission started

						// Check if notification has been lost due to interferences or weak signal strength
						current_sinr = updateSINR(pw_received_interest, noise_level, max_pw_interference,
								save_node_logs, &node_logger, SimTime(), node_id, node_state);
						loss_reason = isPacketLost(notification, convertPower(LINEAR_TO_DB, current_sinr),
								capture_effect, current_cca, convertPower(PICO_TO_DBM, pw_received_interest),
								constant_PER, hidden_nodes_list, save_node_logs, node_logger, node_id,
								node_state, SimTime());

						if(loss_reason != PACKET_NOT_LOST) {	// If CTS packet IS LOST, send logical Nack

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
									SimTime(), node_id, node_state, LOG_D15, LOG_LVL4, notification.tx_info.packet_id,
									notification.source_id, loss_reason);

							// Send logical NACK to ACK transmitter
							LogicalNack logical_nack = generateLogicalNack(notification.packet_type, notification.tx_info.packet_id,
									node_id, notification.source_id,
									NODE_ID_NONE, loss_reason, BER, current_sinr);

							sendLogicalNack(logical_nack);

							// Do nothing until ACK timeout is triggered

						} else {	// If CTS packet IS NOT LOST (it can be properly received)

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s Reception of CTS %d from N%d CAN be started\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.tx_info.packet_id, notification.source_id);

							// Cancel ACK timeout and go to STATE_RX_ACK while updating receiving info
							trigger_CTS_timeout.Cancel();
							node_state = STATE_RX_CTS;
							receiving_from_node_id = notification.source_id;
							receiving_packet_id = notification.tx_info.packet_id;

							// Change state and update receiving info
							data_duration = notification.tx_info.data_duration;
							ack_duration = notification.tx_info.ack_duration;
							cts_duration = notification.tx_info.cts_duration;

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.tx_info.destination_id);

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s current_sinr = %f dB\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL5, convertPower(LINEAR_TO_DB,current_sinr));

						}

					}  else {	//	Some packet type received that is not CTS
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

			/* STATE_WAIT_DATA:
			 * - If node IS destination and packet is DATA:
			 *   * Check if DATA packet can be received:
			 * - If node IS NOT destination: do nothing
			 */
			case STATE_WAIT_DATA:{

				if(notification.tx_info.destination_id == node_id){	// Node is the destination

					pw_received_interest = power_received_per_node[notification.source_id];
					ongoing_notification = notification;

					if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
							SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, notification.tx_info.destination_id);

					if(notification.packet_type == PACKET_TYPE_DATA){	// DATA packet transmission started

						// Check if notification has been lost due to interferences or weak signal strength
						current_sinr = updateSINR(pw_received_interest, noise_level, max_pw_interference,
								save_node_logs, &node_logger, SimTime(), node_id, node_state);

						loss_reason = isPacketLost(notification, convertPower(LINEAR_TO_DB, current_sinr),
								capture_effect, current_cca, convertPower(PICO_TO_DBM, pw_received_interest),
								constant_PER, hidden_nodes_list, save_node_logs, node_logger, node_id,
								node_state, SimTime());

						if(loss_reason != PACKET_NOT_LOST) {	// If DATA packet IS LOST, send logical Nack

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
									SimTime(), node_id, node_state, LOG_D15, LOG_LVL4, notification.tx_info.packet_id,
									notification.source_id, loss_reason);

							// Send logical NACK to DATA transmitter
							LogicalNack logical_nack = generateLogicalNack(notification.packet_type, notification.tx_info.packet_id,
									node_id, notification.source_id,
									NODE_ID_NONE, loss_reason, BER, current_sinr);

							sendLogicalNack(logical_nack);


						} else {	// If DATA packet IS NOT LOST (it can be properly received)

							if(save_node_logs) fprintf(node_logger.file,
									"%f;N%d;S%d;%s;%s Reception of DATA %d from N%d CAN be started\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.tx_info.packet_id, notification.source_id);

							// Cancel DATA timeout and go to STATE_RX_DATA while updating receiving info
							trigger_DATA_timeout.Cancel();
							node_state = STATE_RX_DATA;
							receiving_from_node_id = notification.source_id;
							receiving_packet_id = notification.tx_info.packet_id;

							// Change state and update receiving info
							data_duration = notification.tx_info.data_duration;
							ack_duration = notification.tx_info.ack_duration;

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
	if(save_node_logs) printOrWriteNodesTransmitting(WRITE_LOG, save_node_logs,
			print_node_logs, node_logger, total_nodes_number, nodes_transmitting);

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

		// Update the power sensed at each channel
		updateChannelsPower(channel_power, power_received_per_node, notification, TX_FINISHED,
				SimTime(), node_logger, save_node_logs, node_id, node_state, central_frequency,
				num_channels_komondor, path_loss_model, cochannel_model);

		updateTimestamptChannelFree(timestampt_channel_becomes_free, channel_power,
				convertPower(DBM_TO_PICO, current_cca), num_channels_komondor, SimTime());

		switch(node_state){

			/* STATE_SENSING:
			 * - handle backoff
			 */
			case STATE_SENSING:{	// Do backoff process

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Attempting to restart backoff.\n",
						SimTime(), node_id, node_state, LOG_E11, LOG_LVL3);

				if(node_is_transmitter) {

					int resume = handleBackoff(RESUME_TIMER, SimTime(), save_node_logs,
							node_logger, node_id, node_state, channel_power, primary_channel,
							current_cca, num_packets_in_buffer);

					// Check if node can restart the BO
					if (resume) trigger_DIFS.Set(SimTime() + DIFS);
				}

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
						node_state = STATE_TX_ACK;
						current_tx_duration = ack_duration;
						current_destination_id = notification.source_id;
						ack_notification = generateNotification(PACKET_TYPE_ACK, current_destination_id, current_tx_duration);
						trigger_SIFS.Set(SimTime() + SIFS); // triggers the sendResponse() function after SIFS

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
						current_CW = handleCW(DECREASE_CW, current_CW, CW_min, CW_max);
						// Restart node (implicitly to STATE_SENSING)
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

			/* STATE_TX_DATA || STATE_TX_ACK:
			 * - Do nothing
			 */
			case STATE_TX_DATA:
			case STATE_TX_ACK:
			case STATE_WAIT_ACK:
			case STATE_TX_RTS:
			case STATE_TX_CTS:
			case STATE_WAIT_CTS:
			case STATE_WAIT_DATA:
			case STATE_NAV:{	// Do nothing
				break;
			}

			/* STATE_RX_RTS:
			 * -
			 */
			case STATE_RX_RTS:{

				if(notification.tx_info.destination_id == node_id){	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_RTS){	// RTS packet transmission finished

						if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s RTS %d reception from N%d is finished successfully.\n",
							SimTime(), node_id, node_state, LOG_E14, LOG_LVL3, notification.tx_info.packet_id,
							notification.source_id);

						// Check channel availability in order to send the CTS
						double distance = computeDistance(x, y, z, notification.tx_info.x, notification.tx_info.y, notification.tx_info.z);
						double pw_received = computePowerReceived(distance, notification.tx_info.tx_power, tx_gain, rx_gain,
								central_frequency, path_loss_model);

						if(save_node_logs) fprintf(node_logger.file,
							"%f;N%d;S%d;%s;%s Checking if CTS can be sent due to channel conditions: "
							"current_sinr = %f dBm, Capture Effect = %f, Power Received = %f pW, CCA = %f dBm.\n",
							SimTime(), node_id, node_state, LOG_E14, LOG_LVL3,
							convertPower(LINEAR_TO_DB, current_sinr), capture_effect,
							pw_received, current_cca);

						// TODO: check max interference sensed in TX channel range instead
						// of doing "convertPower(PICO_TO_DBM, channel_power[primary_channel]) < current_cca"

						if(convertPower(LINEAR_TO_DB, current_sinr) > capture_effect
							&& convertPower(PICO_TO_DBM, channel_power[primary_channel]) < current_cca) {

							if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s Channel(s) is(are) clear! Sending CTS to N%d (STATE = %d) ...\n",
								SimTime(), node_id, node_state, LOG_E14, LOG_LVL3, current_destination_id, node_state);

							node_state = STATE_TX_CTS;
							// Generate and send CTS to transmitter after SIFS
							current_destination_id = notification.source_id;

							current_tx_duration = cts_duration;

							current_nav_time = computeNavTime(node_state, rts_duration, cts_duration, data_duration, ack_duration, SIFS);

							trigger_SIFS.Set(SimTime() + SIFS); // triggers the sendResponse() function after SIFS

							cts_notification = generateNotification(PACKET_TYPE_CTS, current_destination_id, current_tx_duration);

						} else {
							// CANNOT START PACKET TX
							// Generate NACK
							restartNode();
						}

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


			/* STATE_RX_CTS:
			 * -
			 */
			case STATE_RX_CTS:{

				if(notification.tx_info.destination_id == node_id){	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_CTS){	// RTS packet transmission finished

						if(save_node_logs) fprintf(node_logger.file,
								"%f;N%d;S%d;%s;%s CTS %d reception from N%d is finished successfully.\n",
								SimTime(), node_id, node_state, LOG_E14, LOG_LVL3, notification.tx_info.packet_id,
								notification.source_id);

						node_state = STATE_TX_DATA;

						// Generate and send DATA to transmitter after SIFS
						current_destination_id = notification.source_id;
						current_CW = handleCW(DECREASE_CW, current_CW, CW_min, CW_max);

						current_tx_duration = data_duration;	// This duration already computed in endBackoff
						trigger_SIFS.Set(SimTime() + SIFS);

						data_notification = generateNotification(PACKET_TYPE_DATA, current_destination_id, current_tx_duration);


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
void Node :: inportNackReceived(LogicalNack &logical_nack){

	if(save_node_logs) fprintf(node_logger.file,
			"%f;N%d;S%d;%s;%s inportNackReceived(): N%d to N%d (A) and N%d (B)\n",
			SimTime(), node_id, node_state, LOG_H00, LOG_LVL1, logical_nack.source_id,
			logical_nack.node_id_a, logical_nack.node_id_b);

	if(logical_nack.source_id != node_id){	// Node is not the NACK transmitter, process NACK

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s NACK of packet #%d received from N%d sent to a) N%d and b) N%d with reason %d\n",
				SimTime(), node_id, node_state, LOG_H00, LOG_LVL2, logical_nack.packet_id, logical_nack.source_id,
				logical_nack.node_id_a, logical_nack.node_id_b, logical_nack.loss_reason);

		// Process logical NACK for statistics purposes
		processNack(logical_nack, node_id, node_logger, node_state, save_node_logs,
				SimTime(), nacks_received, hidden_nodes_list, potential_hidden_nodes,
				total_nodes_number, nodes_transmitting);

	} else {	// Node is the NACK transmitter, do nothing

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s NACK of packet #%d sent to a) N%d and b) N%d with reason %d\n",
				SimTime(), node_id, node_state, LOG_H00, LOG_LVL2, logical_nack.packet_id,
				logical_nack.node_id_a, logical_nack.node_id_b, logical_nack.loss_reason);

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
		double distance = computeDistance(x, y, z, notification.tx_info.x,
				notification.tx_info.y, notification.tx_info.z);

		double pw_received_interest = computePowerReceived(distance,
				notification.tx_info.tx_power, tx_gain, rx_gain,
				central_frequency, path_loss_model);

		// Select the modulation according to the SINR perceived corresponding to incoming transmitter
		selectMCS(mcs_response, pw_received_interest);

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s mcs_response: ",
			SimTime(), node_id, node_state, LOG_F00, LOG_LVL5);

		printOrWriteArrayInt(mcs_response, 4, WRITE_LOG, save_node_logs,
				print_node_logs, node_logger);

		// Fill and send MCS response
		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Answering MCS\n",
								SimTime(), node_id, node_state, LOG_F00, LOG_LVL1);

		Notification response_mcs  = generateNotification(PACKET_TYPE_MCS_RESPONSE,
				notification.source_id, TX_DURATION_NONE);

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
 * trafficGenerator(): called each time a packet is generated to start a new packet generation
 */
void Node :: trafficGenerator() {

	double time_for_next_packet = 0;

	switch(traffic_model) {

		case TRAFFIC_FULL_BUFFER:{

			num_packets_in_buffer = PACKET_BUFFER_SIZE;

			if(node_is_transmitter){

				int resume = handleBackoff(RESUME_TIMER, SimTime(), save_node_logs,
						node_logger, node_id, node_state, channel_power, primary_channel,
						current_cca, num_packets_in_buffer);

				if (resume) trigger_DIFS.Set(SimTime() + DIFS);
			}

			break;
		}

		case TRAFFIC_POISSON:{
			time_for_next_packet = Exponential(1/lambda);
			trigger_new_packet_generated.Set(SimTime() + time_for_next_packet);
			break;
		}

		case TRAFFIC_DETERMINISTIC:{
			time_for_next_packet = 1/lambda;
			trigger_new_packet_generated.Set(SimTime() + time_for_next_packet);
			break;
		}

		default:{
			printf("Wrong traffic model!\n");
			exit(EXIT_FAILURE);
			break;
		}

	}

}

/*
 * newPacketGenerated(): triggered by trigger_new_packet_generated
 */
void Node :: newPacketGenerated(trigger_t &){

	if(node_is_transmitter){

		if (num_packets_in_buffer < PACKET_BUFFER_SIZE) {

			num_packets_in_buffer++;

//			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s A new packet has been generated (%d/%d)\n",
//					SimTime(), node_id, node_state, LOG_F00, LOG_LVL4, num_packets_in_buffer,PACKET_BUFFER_SIZE);

			// Attempt to restart BO only if node didn't have any packet before a new packet was generated
			if(node_state == STATE_SENSING && num_packets_in_buffer == 1) {

				if(trigger_backoff.Active()) remaining_backoff =
						computeRemainingBackoff(backoff_type, trigger_backoff.GetTime(), SimTime());

				int resume = handleBackoff(RESUME_TIMER, SimTime(), save_node_logs,
						node_logger, node_id, node_state, channel_power, primary_channel,
						current_cca, num_packets_in_buffer);

				if (resume) trigger_DIFS.Set(SimTime() + DIFS);

			}

		} else {
			// Buffer overflow - new packet is lost
		}

	}

	trafficGenerator();

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

	printOrWriteChannelPower(WRITE_LOG, save_node_logs,
			node_logger, print_node_logs, channels_free, channel_power, num_channels_komondor);

	getChannelOccupancyByCCA(channels_free, min_channel_allowed, max_channel_allowed,
			channel_power, current_cca, timestampt_channel_becomes_free, SimTime(), DIFS);

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Channels free: ",
			SimTime(), node_id, node_state, LOG_F02, LOG_LVL2);
	printOrWriteChannelsFree(WRITE_LOG, save_node_logs, print_node_logs, node_logger,
			num_channels_komondor, channels_free);

	// Identify the channel range to TX in depending on the channel bonding scheme and power sensed
	getTxChannelsByChannelBonding(channels_for_tx, channel_bonding_model, channels_free,
			min_channel_allowed, max_channel_allowed, primary_channel);

	if(channels_for_tx[0] == TX_NOT_POSSIBLE){

		num_tx_init_not_possible ++;

	}

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Channels for transmitting: ",
			SimTime(), node_id, node_state, LOG_F02, LOG_LVL2);
	printOrWriteChannelForTx(WRITE_LOG, save_node_logs, print_node_logs, node_logger,
			num_channels_komondor, channels_for_tx);

	if(channels_for_tx[0] != TX_NOT_POSSIBLE){	// Transmission IS POSSIBLE

		// Start RTS/CTS exchange (in case it is active)
		node_state = STATE_TX_RTS;
		// Generate Notification
		current_left_channel = getFirstOrLastTrueElemOfArray(FIRST_ONE_IN_ARRAY, channels_for_tx,  num_channels_komondor);
		current_right_channel = getFirstOrLastTrueElemOfArray(LAST_ONE_IN_ARRAY, channels_for_tx, num_channels_komondor);
		int num_channels_tx = current_right_channel - current_left_channel + 1;

		if(save_node_logs) fprintf(node_logger.file,
			"%f;N%d;S%d;%s;%s Transmission is possible in range: %d - %d\n",
			SimTime(), node_id, node_state, LOG_F04, LOG_LVL3, current_left_channel, current_right_channel);

		// COMPUTE ALL DURATIONS

		int ix_mcs_per_node = current_destination_id - wlan.list_sta_id[0];
		int ix_num_channels_used = log2(num_channels_tx);
		current_modulation = mcs_per_node[ix_mcs_per_node][ix_num_channels_used];
//		current_data_rate =  Mcs_array::mcs_array[ix_num_channels_used][current_modulation-1]
//			 * Mcs_array::coding_rate_array[current_modulation-1];
		current_data_rate =  Mcs_array::mcs_array[ix_num_channels_used][current_modulation-1];

		data_duration = computeTxTime(packet_length * num_packets_aggregated, current_data_rate, pdf_tx_time);
		ack_duration = computeTxTime(ack_length, current_data_rate, pdf_tx_time);
		rts_duration = computeTxTime(rts_length, current_data_rate, pdf_tx_time);
		cts_duration = computeTxTime(cts_length, current_data_rate, pdf_tx_time);

		current_nav_time = computeNavTime(node_state, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
		current_tx_duration = rts_duration;

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Setting NAV time to %f\n",
			SimTime(), node_id, node_state, LOG_F04, LOG_LVL4, current_nav_time);

		Notification notification = generateNotification(PACKET_TYPE_RTS,
				current_destination_id, current_tx_duration);

		if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s Transmission of RTS #%d started\n",
				SimTime(), node_id, node_state, LOG_F04, LOG_LVL3, rts_cts_id);
		// Send RTS notification and trigger to finish transmission
		outportSelfStartTX(notification);
		trigger_toFinishTX.Set(SimTime() + current_tx_duration);

		rts_cts_id++;
		rts_cts_sent ++;

		trigger_DIFS.Cancel();	// Safety instruction

	} else {	// Transmission IS NOT, compute a new backoff.

		// Compute a new backoff and trigger a new DIFS
		remaining_backoff = computeBackoff(pdf_backoff, current_CW, backoff_type);
		trigger_DIFS.Set(SimTime() + DIFS);
		// Remain on STATE_SENSING (redundant)
		node_state = STATE_SENSING;

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Transmission is NOT possible\n",
				SimTime(), node_id, node_state, LOG_F03, LOG_LVL3);

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

			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s DATA tx finished. Waiting for ACK...\n",
						SimTime(), node_id, node_state, LOG_G00, LOG_LVL2);

			break;
		}

		case STATE_TX_ACK:{		// Restart node

			Notification notification = generateNotification(PACKET_TYPE_ACK, current_destination_id, TX_DURATION_NONE);
			outportSelfFinishTX(notification);

			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s ACK tx finished. Restarting node...\n",
						SimTime(), node_id, node_state, LOG_G00, LOG_LVL2);

			restartNode();

			break;
		}

		case STATE_TX_RTS:{		// Wait for CTS

			Notification notification = generateNotification(PACKET_TYPE_RTS, current_destination_id, TX_DURATION_NONE);
			outportSelfFinishTX(notification);

			// Set CTS timeout and change state to STATE_WAIT_CTS
			trigger_CTS_timeout.Set(SimTime() + MIN_VALUE_C_LANGUAGE + SIFS);

			node_state = STATE_WAIT_CTS;

			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s RTS tx finished. Waiting for CTS...\n",
						SimTime(), node_id, node_state, LOG_G00, LOG_LVL2);

			break;
		}

		case STATE_TX_CTS:{		// Wait for Data

			Notification notification = generateNotification(PACKET_TYPE_CTS, current_destination_id, TX_DURATION_NONE);

			outportSelfFinishTX(notification);

			// Set CTS timeout and change state to STATE_WAIT_DATA
			trigger_DATA_timeout.Set(SimTime() + (1 + MIN_VALUE_C_LANGUAGE) * SIFS);
			node_state = STATE_WAIT_DATA;

			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s CTS tx finished. Waiting for DATA...\n",
						SimTime(), node_id, node_state, LOG_G00, LOG_LVL2);

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

	if(node_type == NODE_TYPE_OTHER) {
		// Send request MCS notification
		Notification request_modulation = generateNotification(PACKET_TYPE_MCS_REQUEST, default_destination_id, TX_DURATION_NONE);
		outportAskForTxModulation(request_modulation);

	} else {

		// Send request MCS notification
		Notification request_modulation = generateNotification(PACKET_TYPE_MCS_REQUEST, current_destination_id, TX_DURATION_NONE);
		outportAskForTxModulation(request_modulation);

		int ix_aux = current_destination_id - wlan.list_sta_id[0];	// Auxiliary variable for correcting the node id offset
		// MCS of receiver is not pending anymore
		change_modulation_flag[ix_aux] = FALSE;
	}


	// if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s requestMCS() END\n", SimTime(), node_id, node_state, LOG_G00, LOG_LVL1);

}

/*
 * selectDestination(): pick destination node
 */
void Node :: selectDestination(){

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s selectDestination()\n",
			SimTime(), node_id, node_state, LOG_G00, LOG_LVL1);

	if(node_type == NODE_TYPE_OTHER) {
		current_destination_id = default_destination_id;
	}

	current_destination_id = pickRandomElementFromList(wlan.list_sta_id, wlan.num_stas);

	// if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s selectDestination() END\n", SimTime(), node_id, node_state, LOG_G00, LOG_LVL1);
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

	notification.packet_type = packet_type;
	notification.source_id = node_id;
	notification.left_channel = current_left_channel;
	notification.right_channel = current_right_channel;
	notification.packet_length = -1;
	notification.modulation_id = -1;
	notification.timestampt = SimTime();

	TxInfo tx_info;
	tx_info.setSizeOfMCS(4);

	tx_info.packet_id = packet_id;
	tx_info.destination_id = destination_id;
	tx_info.tx_duration = tx_duration;
	tx_info.data_duration = data_duration;
	tx_info.ack_duration = ack_duration;
	tx_info.rts_duration = rts_duration;
	tx_info.cts_duration = cts_duration;
	int num_channels_tx = current_right_channel - current_left_channel + 1;
	tx_info.tx_power = computeTxPowerPerChannel(current_tpc, num_channels_tx);
	tx_info.tx_gain = tx_gain;
	tx_info.data_rate = current_data_rate;
	tx_info.x = x;
	tx_info.y = y;
	tx_info.z = z;
	tx_info.nav_time = 0;

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

		case PACKET_TYPE_RTS:{
			notification.packet_length = rts_length;
			tx_info.nav_time = current_nav_time;
			break;
		}

		case PACKET_TYPE_CTS:{
			notification.packet_length = cts_length;
			tx_info.nav_time = current_nav_time;
			break;
		}

		default:{
			printf("ERROR: Packet type unknown\n");
			exit(EXIT_FAILURE);
			break;
		}
	}

	notification.tx_info = tx_info;
	return notification;
}

/*
 * sendLogicalNack: Sends a NACK notification
 */
void Node :: sendLogicalNack(LogicalNack logical_nack){

	outportSendLogicalNack(logical_nack);

	if(save_node_logs) fprintf(node_logger.file,
			"%f;N%d;S%d;%s;%s NACK of packet type %d sent to a) N%d and b) N%d with reason %d\n",
			SimTime(), node_id, node_state, LOG_I00, LOG_LVL2, logical_nack.packet_type,
			logical_nack.node_id_a, logical_nack.node_id_b, logical_nack.loss_reason);

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
 * sendResponse(): Sends the response according to the current state (called when SIFS is triggered)
 */
void Node :: sendResponse(trigger_t &){

	switch(node_state){

		case STATE_TX_ACK:{
			if(save_node_logs) fprintf(node_logger.file,
					"%f;N%d;S%d;%s;%s SIFS completed after receiving DATA, sending ACK...\n",
					SimTime(), node_id, node_state, LOG_I00, LOG_LVL2);
			outportSelfStartTX(ack_notification);
			trigger_toFinishTX.Set(SimTime() + current_tx_duration);
			break;
		}

		case STATE_TX_CTS:{
			if(save_node_logs) fprintf(node_logger.file,
					"%f;N%d;S%d;%s;%s SIFS completed after receiving RTS, sending CTS...\n",
					SimTime(), node_id, node_state, LOG_I00, LOG_LVL2);
			outportSelfStartTX(cts_notification);
			trigger_toFinishTX.Set(SimTime() + current_tx_duration);
			break;
		}

		case STATE_TX_DATA:{
			if(save_node_logs) fprintf(node_logger.file,
					"%f;N%d;S%d;%s;%s SIFS completed after receiving CTS, sending DATA...\n",
					SimTime(), node_id, node_state, LOG_I00, LOG_LVL2);
			outportSelfStartTX(data_notification);
			trigger_toFinishTX.Set(SimTime() + current_tx_duration);
			packets_sent++;

			// Remove 1 packet from the queue
			if(traffic_model != TRAFFIC_FULL_BUFFER) num_packets_in_buffer --;

			break;
		}
	}
}

/*********************/
/*********************/
/*     TIMEOUTS      */
/*********************/
/*********************/

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

	handlePacketLoss(PACKET_TYPE_DATA, total_time_lost_in_num_channels, total_time_lost_per_channel,
			packets_lost, rts_cts_lost, current_right_channel, current_left_channel,current_tx_duration);

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Packet lost\n",
					SimTime(), node_id, node_state, LOG_D17, LOG_LVL4);
	current_CW = handleCW(INCREASE_CW, current_CW, CW_min, CW_max);

	restartNode();
}

/*
 * ctsTimeout(): handles ACK timeout. It is called when ACK timeout is triggered.
 */
void Node :: ctsTimeout(trigger_t &){

	if(save_node_logs) fprintf(node_logger.file,
			"%f;N%d;G02; - CTS TIMEOUT! Transmission %d has been lost\n", SimTime(), node_id, packet_id);

	// Quantify lost time during the WAIT status
	// TODO: create CTS tx duration variable
	//current_cts_tx_duration += (1 + MIN_VALUE_C_LANGUAGE) * SIFS;

	handlePacketLoss(PACKET_TYPE_CTS, total_time_lost_in_num_channels, total_time_lost_per_channel,
			packets_lost, rts_cts_lost, current_right_channel, current_left_channel,current_tx_duration);

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Packet lost\n",
					SimTime(), node_id, node_state, LOG_D17, LOG_LVL4);
	current_CW = handleCW(INCREASE_CW, current_CW, CW_min, CW_max);

	restartNode();
}

/*
 * dataTimeout(): handles ACK timeout. It is called when ACK timeout is triggered.
 */
void Node :: dataTimeout(trigger_t &){

	if(save_node_logs) fprintf(node_logger.file,
			"%f;N%d;G02; - DATA TIMEOUT! Transmission %d has been lost\n", SimTime(), node_id, packet_id);

	// Quantify lost time during the WAIT status
	// TODO: create CTS tx duration variable
	//current_cts_tx_duration += (1 + MIN_VALUE_C_LANGUAGE) * SIFS;

	handlePacketLoss(PACKET_TYPE_CTS, total_time_lost_in_num_channels, total_time_lost_per_channel,
			packets_lost, rts_cts_lost, current_right_channel, current_left_channel, current_tx_duration);

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Packet lost\n",
					SimTime(), node_id, node_state, LOG_D17, LOG_LVL4);
	current_CW = handleCW(INCREASE_CW, current_CW, CW_min, CW_max);

	restartNode();
}

/*
 * navTimeout(): handles ACK timeout. It is called when ACK timeout is triggered.
 */
void Node :: navTimeout(trigger_t &){

	if(save_node_logs) fprintf(node_logger.file,
			"%f;N%d;G02; - NAV TIMEOUT! Attempting to resume the backoff\n", SimTime(), node_id);

	node_state = STATE_SENSING;

	if(node_is_transmitter){

		int resume = handleBackoff(RESUME_TIMER, SimTime(), save_node_logs,
				node_logger, node_id, node_state, channel_power, primary_channel,
				current_cca, num_packets_in_buffer);

		if (resume) trigger_DIFS.Set(SimTime() + DIFS);
	}

}

/************************/
/************************/
/*  BACKOFF MANAGEMENT  */
/************************/
/************************/

/*
 * pauseBackoff(): pauses the backoff
 * */
void Node :: pauseBackoff(){

	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Pausing the backoff\n",
		SimTime(), node_id, node_state, LOG_F00, LOG_LVL2);

	trigger_DIFS.Cancel();

	if(trigger_backoff.Active()){	// If backoff trigger is active, freeze it

		remaining_backoff = computeRemainingBackoff(backoff_type,
				trigger_backoff.GetTime(), SimTime());

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
 * resumeBackoff(): resumes the backoff (triggered after DIFS is completed)
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
	total_time_transmitting_in_num_channels[(int)log2(current_right_channel - current_left_channel + 1)] += current_tx_duration;
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

		selectDestination();

		// In case of being an AP
		remaining_backoff = computeBackoff(pdf_backoff, current_CW, backoff_type);

		if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s New backoff computed: %f\n",
						SimTime(), node_id, node_state, LOG_Z00, LOG_LVL3, remaining_backoff);

		// Freeze backoff immediately if primary channel is occupied
		int resume = handleBackoff(RESUME_TIMER, SimTime(), save_node_logs,
				node_logger, node_id, node_state, channel_power, primary_channel,
				current_cca, num_packets_in_buffer);

		// Check if node has to freeze the BO (if it is not already frozen)
		if (resume) trigger_DIFS.Set(SimTime() + DIFS);

	}

	cleanNack();
}

/************************/
/************************/
/*  PRINT INFORMATION   */
/************************/
/************************/

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
		printf("%s tpc_min = %f pW (%f dBm)\n", LOG_LVL4, tpc_min, convertPower(PICO_TO_DBM, tpc_min));
		printf("%s tpc_default = %f pW (%f dBm)\n", LOG_LVL4, tpc_default, convertPower(PICO_TO_DBM, tpc_default));
		printf("%s tpc_max = %f pW (%f dBm)\n", LOG_LVL4, tpc_max, convertPower(PICO_TO_DBM, tpc_max));
		printf("%s cca_min = %f pW (%f dBm)\n", LOG_LVL4, cca_min, convertPower(PICO_TO_DBM, cca_min));
		printf("%s cca_default = %f pW (%f dBm)\n", LOG_LVL4, cca_default, convertPower(PICO_TO_DBM, cca_default));
		printf("%s cca_max = %f pW (%f dBm)\n", LOG_LVL4, cca_max, convertPower(PICO_TO_DBM, cca_max));
		printf("%s tx_gain = %f (%f dBi)\n", LOG_LVL4, tx_gain, convertPower(LINEAR_TO_DB, tx_gain));
		printf("%s rx_gain = %f (%f dBi)\n", LOG_LVL4, rx_gain, convertPower(LINEAR_TO_DB, rx_gain));
		printf("%s modulation_default = %d\n", LOG_LVL4, modulation_default);
		printf("%s central_frequency = %f Hz (%f GHz)\n", LOG_LVL4, central_frequency, central_frequency * pow(10,-9));
		printf("%s lambda = %f packets/s\n", LOG_LVL4, lambda);
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
		fprintf(node_logger.file, "%s - tpc_default = %f pW\n", header_string, tpc_default);
		fprintf(node_logger.file, "%s - cca_default = %f pW\n", header_string, cca_default);
		fprintf(node_logger.file, "%s - lambda = %f\n", header_string, lambda);
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
 * printOrWriteNodeStatistics(): prints (or writes) final statistics at the given node
 */
void Node :: printOrWriteNodeStatistics(int write_or_print){

	// Process statistics
	double packets_lost_percentage = 0;
	double rts_cts_lost_percentage = 0;
	if (packets_sent > 0) {
		packets_lost_percentage = double(packets_lost * 100)/double(packets_sent);
		rts_cts_lost_percentage = double(rts_cts_lost * 100)/double(rts_cts_sent);
	}
	throughput = (((double)(packets_sent-packets_lost) * packet_length * num_packets_aggregated))
			/ SimTime();
	int hidden_nodes_number = 0;
	for(int i = 0; i < total_nodes_number; i++){
		if(hidden_nodes_list[i] == 1) hidden_nodes_number++;
	}

	switch(write_or_print){

		case PRINT_LOG:{

			if (node_is_transmitter && print_node_logs) {
				printf("------- %s (N%d) ------\n", node_code, node_id);

				// Throughput
				printf("%s Throughput = %f Mbps\n", LOG_LVL2, throughput * pow(10,-6));

				// Packets sent and lost
				printf("%s Packets sent = %d - Packets lost = %d  (%.2f %% lost)\n",
						LOG_LVL2, packets_sent, packets_lost, packets_lost_percentage);

				// RTS/CTS sent and lost
				printf("%s RTS/CTS sent = %d - RTS/CTS lost = %d  (%.2f %% lost)\n",
						LOG_LVL2, rts_cts_sent, rts_cts_lost, rts_cts_lost_percentage);

//				// Time EFFECTIVELY transmitting in a given number of channels (no losses)
//				printf("%s Time EFFECTIVELY transmitting in N channels:", LOG_LVL3);
//				for(int n = 0; n < num_channels_allowed; n++){
//					printf("\n%s - %d: %f",
//							LOG_LVL3, (int) pow(2,n), total_time_transmitting_in_num_channels[n] -
//							total_time_lost_in_num_channels[n]);
//				}
//				printf("\n");

				// Time EFFECTIVELY transmitting in each of the channels (no losses)
				printf("%s Time EFFECTIVELY transmitting in each channel:", LOG_LVL3);
				double time_effectively_txing;
				for(int c = 0; c < num_channels_komondor; c++){

					time_effectively_txing = total_time_transmitting_per_channel[c] -
							total_time_lost_per_channel[c];

					printf("\n%s - %d = %.2f s (%.2f %%)",
							LOG_LVL3, c, time_effectively_txing,
							(time_effectively_txing * 100 /SimTime()));
				}
				printf("\n");

				// Time LOST transmitting in a given number of channels
//				printf("%s Time LOST transmitting in N channels:", LOG_LVL3);
//				for(int n = 0; n < num_channels_allowed; n++){
//					printf("\n%s - %d: %f",
//							LOG_LVL3, (int) pow(2,n), total_time_lost_in_num_channels[n]);
//				}
//				printf("\n");

				// Time LOST transmitting in each of the channels
				printf("%s Time LOST transmitting in each channel:", LOG_LVL3);
				for(int c = 0; c < num_channels_komondor; c++){
					printf("\n%s - %d = %.2f s (%.2f %%)",
						LOG_LVL3, c, total_time_lost_per_channel[c],
						(total_time_lost_per_channel[c] * 100 / SimTime()));
				}
				printf("\n");

				// Number of TX initiations that have been not possible due to channel state and DCB model
//				printf("%s num_tx_init_not_possible = %d\n", LOG_LVL2, num_tx_init_not_possible);

				// Hidden nodes
				// printf("%s Total number of hidden nodes: %d\n", LOG_LVL2, hidden_nodes_number);
//				printf("%s Hidden nodes list: ", LOG_LVL2);
//				for(int i = 0; i < total_nodes_number; i++){
//					printf("%d  ", hidden_nodes_list[i]);
//				}
//				printf("\n");

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
							SimTime(), node_id, node_state, LOG_C02, LOG_LVL2, throughput * pow(10,-6));

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

/*****************************/
/*****************************/
/*  VARIABLE INITIALISATION  */
/*****************************/
/*****************************/

/*
 * initializeVariables(): initializes all the necessary variables
 */
void Node :: initializeVariables() {

	current_sinr = 0;

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
	timestampt_channel_becomes_free = (double *) malloc(num_channels_komondor
			* sizeof(*timestampt_channel_becomes_free));

	for(int i = 0; i < num_channels_komondor; i++){
		channel_power[i] = 0;
		total_time_transmitting_per_channel[i] = 0;
		channels_free[i] = FALSE;
		channels_for_tx[i] = FALSE;
		total_time_lost_per_channel[i] = 0;
		timestampt_channel_becomes_free[i] = 0;
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


	node_state = STATE_SENSING;
	current_modulation = modulation_default;
	current_CW = CW_min;
	packet_id = 0;
	rts_cts_id = 0;
	num_packets_in_buffer = 0;
	remaining_backoff = computeBackoff(pdf_backoff, current_CW, backoff_type);
	if(node_type == NODE_TYPE_AP) {
		node_is_transmitter = TRUE;
	} else {
		node_is_transmitter = FALSE;
	}


	current_tpc = tpc_default;
	current_cca = cca_default;
	channel_max_intereference = 0;

	data_duration = 0;
	ack_duration = 0;
	rts_duration = 0;
	cts_duration = 0;

	default_modulation = MODULATION_NONE;

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

	/* NULL notification for Valgrind issues */
	Notification null_notification;

	null_notification.source_id = -1;
	null_notification.packet_type = -1;
	null_notification.left_channel = -1;
	null_notification.right_channel = -1;
	null_notification.packet_length = -1;
	null_notification.modulation_id = -1;
	null_notification.timestampt = -1;

	TxInfo null_tx_info;

	null_tx_info.packet_id = -1;
	null_tx_info.destination_id = -1;
	null_tx_info.tx_duration = -1;
	null_tx_info.data_duration = 0;
	null_tx_info.ack_duration = 0;
	null_tx_info.rts_duration = 0;
	null_tx_info.cts_duration = 0;
	null_tx_info.tx_power = 0;
	null_tx_info.tx_gain = 0;
	null_tx_info.data_rate = 0;
	null_tx_info.setSizeOfMCS(4);
	null_tx_info.x = 0;
	null_tx_info.y = 0;
	null_tx_info.z = 0;
	null_tx_info.nav_time = 0;

	null_notification.tx_info = null_tx_info;

	data_notification = null_notification;
	ack_notification= null_notification;
	ongoing_notification = null_notification;
	rts_notification = null_notification;
	cts_notification = null_notification;


	// Statistics
	packets_sent = 0;
	rts_cts_sent = 0;
	throughput = 0;
	throughput_loss = 0;
	packets_lost = 0;
	rts_cts_lost = 0;
	num_tx_init_not_possible = 0;



}