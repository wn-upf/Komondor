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
 * node.h: this file contains the node component
 */


#include <math.h>
#include <algorithm>
#include <stddef.h>
#include <iostream>
#include <stdlib.h>

#include "../list_of_macros.h"
#include "../methods/utils/auxiliary_methods.h"
#include "../methods/channel/power_channel_methods.h"
#include "../methods/mac/backoff_methods.h"
#include "../methods/frames/notification_methods.h"
#include "../methods/mac/spatial_reuse_methods.h"
#include "../structures/notification.h"
#include "../structures/logical_nack.h"
#include "../structures/wlan.h"
#include "../structures/logger.h"
#include "../structures/FIFO.h"
#include "../structures/node_configuration.h"
#include "../structures/performance.h"
#include "../structures/spatial_reuse_state.h"
#include "../structures/channel_access_state.h"
#include "../structures/node_statistics.h"
#include "../structures/node_parameters.h"
#include "../structures/packet_exchange_sequence.h"

#include "../methods/mac/nack_methods.h"
#include "../methods/mac/packet_loss_methods.h"
#include "../methods/mac/nav_methods.h"

#include "../methods/frames/frame_duration_methods.h"
#include "../methods/frames/packet_aggregation_methods.h"

#define __SAVELOGS__

#ifdef __SAVELOGS__
    #define    LOGS(flag,file,...)    if(flag){fprintf(file, ##__VA_ARGS__);}
#else
    #define    LOGS(flag,file,...)
#endif

// Node component: "TypeII" represents components that are aware of the existence of the simulated time.
component Node : public TypeII{

	// Methods
	public:

		// COST
		void Setup();
		void Start();
		void Stop();

		// Generic
		void InitializeVariables();
		void RestartNode(int called_by_time_out);
		void CallRestartSta();
		void CallSensing();
		void PrintNodeInfo(int info_detail_level);
		void PrintNodeConfiguration();
		void WriteNodeInfo(Logger node_logger, int info_detail_level, std::string header_str);
		void WriteNodeConfiguration(Logger node_logger, std::string header_str);
		void WriteReceivedConfiguration(Logger node_logger, std::string header_str, Configuration new_configuration);
		void PrintOrWriteNodeStatistics(int write_or_print);
		void HandleSlottedBackoffCollision();
		void StartSavingLogs();
		void RecoverFromCtsTimeout();
		void MeasureRho();
		void SaveSimulationPerformance();

		// Packets
		Notification GenerateNotification(int packet_type, int destination_id,
			int packet_id, int num_packets_aggregated, double timestamp_generated, double tx_duration);
		void SelectDestination();
		void SendResponsePacket();
		void AckTimeout();
		void CtsTimeout();
		void DataTimeout();
		void NavTimeout();
		void RequestMCS();
		void StartTransmission();
		void AbortInitialTransmission();
		void PrepareNewTransmission();
		void InitiateBurstPackets();
		void ScheduleTransmission(int first_packet_type = PACKET_TYPE_RTS);

		// NACK
		void SendLogicalNack(LogicalNack logical_nack);

		// Backoff
		void PauseBackoff();
		void ResumeBackoff();
		void ScheduleBackoffAfterDIFS();		

		// Signal helpers
		void UpdateSINRFromNotification(const Notification &notification);

		// Configuration (to be sent to the agent)
		void GenerateConfiguration();
		void ApplyNewConfiguration(Configuration &received_configuration);
		void BroadcastNewConfigurationToStas(Configuration &received_configuration);
		void UpdatePerformanceMeasurements();

		// Spatial Reuse
		void SpatialReuseOpportunityEnds();
		void UpdateSRStateForIncomingFrame(const Notification &notification);
		void TryIdentifySRTXOP(const Notification &notification, int &loss_reason);
		void ComputeNavCollisions(const Notification &notification, int &nav_collision, int &inter_bss_nav_collision);
		void DetectSRTXOPInNavState(const Notification &notification, int loss_reason);
		void DetectSRTXOPWhileTransmitting(const Notification &notification);
		void ApplySRParametersAtBackoffEnd();
		void CheckSRTXOPAtCallSensing();

		// InportSomeNodeStartTX per-state handlers
		void HandleStartTX_StateSensing(const Notification &notification);
		void HandleStartTX_StateTxData(const Notification &notification);
		void HandleStartTX_StateRxData(const Notification &notification);
		void HandleStartTX_StateWaitAck(const Notification &notification);
		void HandleStartTX_StateWaitCts(const Notification &notification);
		void HandleStartTX_StateWaitData(const Notification &notification);
		void HandleStartTX_StateNav(const Notification &notification);
		// MAPC wait-state handlers
		void HandleStartTX_StateWaitIcr(const Notification &notification);
		void HandleStartTX_StateWaitMuRts(const Notification &notification);
		void HandleStartTX_StateWaitTf(const Notification &notification);

		// InportSomeNodeFinishTX per-state handlers
		void HandleFinishTX_StateSensing(const Notification &notification);
		void HandleFinishTX_StateRxData(const Notification &notification);
		void HandleFinishTX_StateTxData(const Notification &notification);
		// MAPC reception handlers
		void HandleFinishTX_StateRxIcf(const Notification &notification);
		void HandleFinishTX_StateRxIcr(const Notification &notification);
		void HandleFinishTX_StateRxMuRts(const Notification &notification);
		void HandleFinishTX_StateRxTf(const Notification &notification);
		void HandleFinishTX_StateWaitAckTf(const Notification &notification);
		void ProceedAfterIcr();
		void ComputeCoSRTxPowers(double &coordinator_pW, double &peer_pW);

	// Public items (entered by nodes constructor in komondor_main)
	public:

		// Specific to a node
		FIFO buffer;					///> FIFO buffer (contains Notifications)
		int last_packet_generated_id;	///> ID of the last packet generated by the source

		// WLAN
		Wlan wlan;					///> Wlan to which the node belongs

		NodeParameters node_params;		///> Static configuration parameters (see structures/node_parameters.h)
		PacketExchangeSequence exchange_sequence;  ///> Active frame exchange protocol (default: 4-WAY RTS/CTS/DATA/ACK)

		// MAPC coordinator state
		int  num_coordinated_aps;				///> # of coordinated APs invited for current TXOP
		int  coordinated_ap_ids[8];				///> Their node_ids
		int  mapc_seq_pos;						///> Current position in exchange_sequence.frame_types
		int  mapc_icr_received_count;			///> # of ICRs received so far
		int  mapc_current_peer_idx;				///> Round-robin counter: index into coordinated_ap_ids[]
		int  mapc_selected_peer_id;				///> node_id of the AP targeted in the current TXOP
		int  mapc_peer_has_data;				///> TRUE if selected peer reported data in its ICR
		double mapc_txop_data_budget;			///> Total DATA duration available for this TXOP (set at ICF time)
		double mapc_txop_per_ap_data_duration;	///> Per-AP DATA duration after TXOP split (set after ICR)
		double mapc_icr_requested_duration;		///> Co-SR/BF: coordinated AP's desired DATA duration from ICR
		Notification mu_rts_notification;		///> MU-RTS/TXS frame (coordinator, Co-TDMA)
		Notification tf_notification;			///> Trigger Frame notification (coordinator, Co-BF/Co-SR)
		Notification ack_tf_notification;		///> ACK TF (coordinator, Co-BF/Co-SR simultaneous ACK request)
		int  mapc_active_group_idx;				///> Index into wlan.mapc_group_ids[] for current TXOP
		int  mapc_coordinator_group_rr;			///> Round-robin counter for group selection (coordinator)

		// MAPC coordinated state
		int  coordinator_ap_id;					///> AP that sent us the ICF
		int  mapc_peer_position;				///> Stagger index for ICR timing (0 = earliest)
		Notification icr_notification;			///> ICR to send back to coordinator

		// Transmissions
		int current_modulation;				///> Current_modulation used by nodes
		int channel_max_intereference;		///> Channel of interest suffering maximum interference

		// Data rate - modulations
		int modulation_rates[4][12];		///> Modulation rates in bps used in IEEE 802.11ax
		int err_prob_modulation[4][12];		///> BER associated to each modulation (TO BE FILLED!!)
		int first_time_requesting_mcs;		///> Flag to indicate if the MCS has been firstly requested or not


		// Token-based channel access
		int *token_order_list;			///> Ordered list of the devices involved in the tokenized channel access
		int token_status;				///> Status of the token (i.e., ID of the device holding the token)
		int distance_to_token;			///> Distance of the node to the token in "token_status_list"
		int *num_missed_tokens_list;	///> Number of missed tokens for each involved device in the tokenized channel access

		double *distances_array;					///> Distance with respect to other nodes
		double *received_power_array;				///> Power received from the other nodes
		double *max_received_power_in_ap_per_wlan;	///> Maximum power received from each WLAN

		double *rssi_per_sta;	///> RSSI per STA in the WLAN

		// Positions of all nodes (initialized at startup; used for per-TXOP beam steering)
		double *all_node_x;		///> X position of every node indexed by node_id
		double *all_node_y;		///> Y position of every node indexed by node_id
		double *all_node_z;		///> Z position of every node indexed by node_id

		// Per-TXOP beamforming state (updated in PrepareNewTransmission, embedded in notifications)
		double current_beam_az_main_rad;				///> Main beam azimuth for current TX [rad]
		double current_beam_null_az_rad[MAX_BEAM_NULLS];///> Null directions for current TX [rad]
		int    current_beam_num_nulls;					///> Number of active nulls for current TX

	// Statistics (accessible when simulation finished through Komondor simulation class)
	public:

		NodeStatistics node_stats;	///> All per-node statistics and measurement state (see structures/node_statistics.h)

		// Store the simulation performance
		Performance simulation_performance;	///> Variable that stores the performance obtained during the simulation

		// Configurations sent/received to/from the agent
		Configuration configuration;		///> Configuration object
		Configuration new_configuration;	///> Auxiliary configuration object, used for changes recommended by agents/CC

		// Spatial Reuse configuration
		Configuration spatial_reuse_configuration;	///> Spatial Reuse configuration

		// Measurements done for agents
		Performance performance_report;	///> Performance report to be delivered to agents or the CC

	// Private items (just for node operation)
	private:

		// Komondor environment
		double *channel_power;				///> Channel power detected in each sub-channel [pW] (Pico watts for resolution issues)
		int *channels_free;					///> Channels that are found free for the beginning TX (i.e. power sensed < pd)
		int *channels_for_tx;				///> Channels that are used in the beginning TX (depend on the channel bonding model)

		// File for writting node logs
		FILE *output_log_file;				///> File for logs in which the node is involved
		char own_file_path[256];			///> Name of the file for node logs
		Logger node_logger;					///> struct containing the attributes needed for writting logs in a file
		std::string header_str;				///> Header string for the logger

		// State and timers
		int node_state;						///> Node's internal state (0: sensing the channel, 1: transmitting, 2: receiving notification)
		int progress_bar_counter;			///> Counter for displaying the progress bar

		// Transmission parameters
		int node_is_transmitter;			///> Flag for determining if node is able to tranmsit packet (e.g., AP in downlink)
		int current_left_channel;			///> Left channel used in current TX
		int current_right_channel;			///> Right channel used in current TX
		double current_tx_power;			///> Transmission power used in current TX [dBm]
		double current_pd;					///> Current pd (variable "sensitivity")	[dBm]
		int current_destination_id;			///> Current destination node ID
		double current_tx_duration;			///> Duration of the TX being done [s]
		double current_nav_time;			///> Current NAV duration
		int packet_id;						///> Notification ID
		double current_sinr;				///> SINR perceived in current TX [linear ratio]
		int loss_reason;					///> Packet loss reason (if any)
		int current_num_packets_aggregated;	///> Num. of packets aggregated in a single PPDU
		int limited_num_packets_aggregated; ///> Num. of limited (due to max PPDU duration) packets aggregated in a single PPDU

		// Notifications
		Notification rts_notification;			///> RTS to be filled before sending it
		Notification cts_notification;			///> CTS to be filled before sending it
		Notification data_notification;			///> DATA notification to be filled before sending it
		Notification ack_notification;			///> ACK to be filled before sending it
		// MAPC notifications
		Notification icf_notification;			///> ICF (MAPC Initial Control Frame) to be filled before sending it
		Notification incoming_notification; 	///> Notification of interest being received
		Notification new_packet;				///> Auxiliar notification object for new packets
		Notification null_notification;			///> Auxiliar notification object for null packets
		Notification nav_notification;			///> Last notification that made the node change state or remain in NAV. It is used for detecting simultaneous events.
		Notification outrange_nav_notification; ///> NAV notification sent in a different primary channel. Store it for detecting BO collisions when using CB.
		TxInfo current_tx_info;					///> Object to store the current transmission information

		// Traffic
		int current_traffic_type;	///> Current type of traffic

		int default_modulation;		///> Default MCS identifier
		double bits_ofdm_sym;		///> Number of bits per OFDM symbol in the data packet according to MCS [bits]

		// Channel access / backoff state (CW management + deterministic BO + ECA)
		ChannelAccessState ca_state;		///> Contention-window, deterministic-BO, and ECA backoff state

		// Packets durations
		double data_duration;		///> Duration of the data packet
		double ack_duration;		///> Duration of the ACK packet
		double rts_duration;		///> Duration of the RTS packet
		double cts_duration;		///> Duration of the CTS packet

		int **mcs_per_node;				///> Modulation selected for each of the nodes (only transmitting nodes)
		int *change_modulation_flag;	///> Flag for changing the MCS of any of the potential receivers
		int *mcs_response;				///> MCS response received from receiver

		// Sensing and Reception parameters
		LogicalNack logical_nack;					///> NACK to be filled in case node is the destination of tx loss
		double max_pw_interference;					///> Maximum interference detected in range of interest [pW]
		int channel_max_interference;				///> Channel of maximum interference detected in range of interest [pW]
		int *nodes_transmitting;					///> IDs of the nodes which are transmitting to any destination
		std::map<int, double>
			power_received_per_node;				///> Map containing the power received from each other node
		double power_rx_interest;					///> Power received from a TX destined to the node [pW]
		int receiving_from_node_id;					///> ID of the node that is transmitting to the node (-1 if node is not receiving)
		int receiving_packet_id;					///> ID of the notification that is being transmitted to me
//		int *hidden_nodes_list;						///> Store nodes that for sure are hidden (1 indicates that node "i" is hidden)
//		int *potential_hidden_nodes;				///> Maintain a list of the times a node participated in a collision by hidden node
//		int collisions_by_hidden_node; 				///> Number of noticed collisions by hidden node (maintained by the transmitter)
		double BER;									///> Bit error rate (deprecated)
		double PER;									///> Packet error rate (deprecated)
		double *timestamp_channel_becomes_free;	///> Timestamp when channel becomes free (when P(channel) < PD threshold)
		double time_to_trigger;						///> Auxiliar time to trigger an specific trigger (used for almost every .Set() function)
		int num_channels_tx;						///> Number of channels used for transmission
		int flag_apply_new_configuration;			///> Flag to determine if there is any new configuration to be applied when doing "RestartNode()"
		int channel_aggregation_cca_model;			///> Flag to determine the type of CCA per bandwidth applied
		int last_transmission_successful;			///> Flag to indicate whether the last transmission was successful or not

		// Burst traffic
		double burst_rate;				///> Average time between two packet generation bursts [bursts/s]
		int num_bursts;					///> Total number of bursts occurred in the simulation

		double time_rand_value;		///> Rand value to avoid synch. issues

		// Spatial Reuse operational state
		SpatialReuseState sr_state;		///> 11ax SR / OBSS-PD operational state

	// Connections and timers
	public:

		// INPORT connections for receiving notifications
		inport void inline InportSomeNodeStartTX(Notification &notification);
		inport void inline InportSomeNodeFinishTX(Notification &notification);
		inport void inline InportNackReceived(LogicalNack &logical_nack_info);

		inport void inline InportMCSRequestReceived(Notification &notification);
		inport void inline InportMCSResponseReceived(Notification &notification);

		inport void inline InportReceivingRequestFromAgent();
		inport void inline InportReceiveConfigurationFromAgent(Configuration &new_configuration);
		inport void inline InportNewWlanConfigurationReceived(Configuration &new_configuration);

		// Traffic generator
		inport void inline InportNewPacketGenerated();

		// Spatial reuse (virtual) notifications
		inport void inline InportRequestSpatialReuseConfiguration();
		inport void inline InportNewSpatialReuseConfiguration(Configuration &new_configuration);

		// OUTPORT connections for sending notifications
		outport void outportSelfStartTX(Notification &notification);
		outport void outportSelfFinishTX(Notification &notification);
		outport void outportSendLogicalNack(LogicalNack &logical_nack_info);

		outport void outportAskForTxModulation(Notification &notification);
		outport void outportAnswerTxModulation(Notification &notification);

		outport void outportAnswerToAgent(Configuration &configuration, Performance &performance);
		outport void outportSetNewWlanConfiguration(Configuration &new_configuration);

		// Spatial reuse (virtual) notifications
		outport void outportRequestSpatialReuseConfiguration();
		outport void outportNewSpatialReuseConfiguration(Configuration &new_configuration);

		// Triggers
		Timer <trigger_t> trigger_sim_time;				// Timer for displaying the exectuion time status (progress bar)
		Timer <trigger_t> trigger_end_backoff; 			// Duration of current trigger_end_backoff. Triggers outportSelfStartTX()
		Timer <trigger_t> trigger_start_backoff;		// Timer for the DIFS
		Timer <trigger_t> trigger_toFinishTX; 			// Duration of current notification transmission. Triggers outportSelfFinishTX()
		Timer <trigger_t> trigger_SIFS;					// Timer for the SIFS
		Timer <trigger_t> trigger_ACK_timeout;			// Trigger when ACK hasn't arrived in time
		Timer <trigger_t> trigger_CTS_timeout;			// Trigger when CTS hasn't arrived in time
		Timer <trigger_t> trigger_DATA_timeout; 		// Trigger when DATA TX could not start due to RTS/CTS failure
		Timer <trigger_t> trigger_NAV_timeout;  		// Trigger for the NAV
		Timer <trigger_t> trigger_inter_bss_NAV_timeout;// Trigger for the inter-BSS NAV (only used in the SR operation)
		Timer <trigger_t> trigger_preoccupancy; 		// Trigger for delaying 1 ps the occupancy of channels after channel range selection
		Timer <trigger_t> trigger_restart_sta; 			// Trigger for retarding the STA restart enough time to handle same time RTS finish events
		Timer <trigger_t> trigger_wait_collisions; 		// Trigger for waiting just in case more RTS collisions are detected at the same time
		Timer <trigger_t> trigger_start_saving_logs; 	// Trigger for starting saving logs
		Timer <trigger_t> trigger_recover_cts_timeout; 	// Trigger for waiting part of EIFS after CTS timeout detected
		Timer <trigger_t> trigger_rho_measurement; 		// Trigger for periodically measuring the rho metric
		Timer <trigger_t> txop_sr_end;					// Trigger to determine the duration of an identified SR-based opportunity

		// Every time the timer expires execute this
		inport inline void EndBackoff(trigger_t& t1);
		inport inline void MyTxFinished(trigger_t& t1);
		inport inline void PrintProgressBar(trigger_t& t1);
		inport inline void ResumeBackoff(trigger_t& t1);
		inport inline void SendResponsePacket(trigger_t& t1);
		inport inline void AckTimeout(trigger_t& t1);
		inport inline void CtsTimeout(trigger_t& t1);
		inport inline void DataTimeout(trigger_t& t1);
		inport inline void NavTimeout(trigger_t& t1);
		inport inline void StartTransmission(trigger_t& t1);
		inport inline void CallRestartSta(trigger_t& t1);
		inport inline void CallSensing(trigger_t& t1);
		inport inline void StartSavingLogs(trigger_t& t1);
		inport inline void RecoverFromCtsTimeout(trigger_t& t1);
		inport inline void MeasureRho(trigger_t& t1);
		inport inline void SpatialReuseOpportunityEnds(trigger_t& t1);

		// Connect timers to methods
		Node () {
			connect trigger_end_backoff.to_component,EndBackoff;
			connect trigger_toFinishTX.to_component,MyTxFinished;
			connect trigger_sim_time.to_component,PrintProgressBar;
			connect trigger_start_backoff.to_component,ResumeBackoff;
			connect trigger_SIFS.to_component,SendResponsePacket;
			connect trigger_ACK_timeout.to_component,AckTimeout;
			connect trigger_CTS_timeout.to_component,CtsTimeout;
			connect trigger_DATA_timeout.to_component,DataTimeout;
			connect trigger_NAV_timeout.to_component,NavTimeout;
			connect trigger_inter_bss_NAV_timeout.to_component,NavTimeout;
			connect trigger_preoccupancy.to_component,StartTransmission;
			connect trigger_restart_sta.to_component,CallRestartSta;
			connect trigger_wait_collisions.to_component,CallSensing;
			connect trigger_start_saving_logs.to_component,StartSavingLogs;
			connect trigger_recover_cts_timeout.to_component,RecoverFromCtsTimeout;
			connect trigger_rho_measurement.to_component,MeasureRho;
			connect txop_sr_end.to_component,SpatialReuseOpportunityEnds;
		}
};

/**
 * Setup()
 */
void Node :: Setup(){
	// Do nothing
};

/**
 * Start()
 */
void Node :: Start(){

	// Initialize variables
	InitializeVariables();

	// Create node logs file if required
	node_logger.file = NULL;
	if(node_params.save_node_logs) {
		// Name node log file accordingly to the node_params.node_id
		snprintf(own_file_path, sizeof(own_file_path), "%s_%s_N%d_%s.txt","../output/logs_output", node_params.simulation_code.c_str(), node_params.node_id, node_params.node_code.c_str());
		remove(own_file_path);
		output_log_file = fopen(own_file_path, "at");
		if(output_log_file == NULL) {
			printf("WARNING: Cannot open node log file %s - disabling node logs\n", own_file_path);
			node_params.save_node_logs = 0;
		} else {
			node_logger.save_logs = node_params.save_node_logs;
			node_logger.file = output_log_file;
			node_logger.SetVoidHeadString();
		}
	}
	LOGS(node_params.save_node_logs, node_logger.file,"%.18f;N%d;S%d;%s;%s Start()\n",
		SimTime(), node_params.node_id, STATE_UNKNOWN, LOG_B00, LOG_LVL1);

	// Write node info and conf.
	std::string header_str;
	header_str.append(ToString(SimTime()));
	if(node_params.save_node_logs) WriteNodeInfo(node_logger, INFO_DETAIL_LEVEL_2, header_str);

	// Start backoff procedure only if node is able to transmit
	if(node_is_transmitter) {
		if (TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION) {
			ScheduleBackoffAfterDIFS();
		}
		// RHO MEASUREMENTS (check if it can be removed)
		// if(node_stats.flag_measure_rho) trigger_rho_measurement.Set(SimTime() + node_stats.delta_measure_rho);
	} else {
		current_destination_id = wlan.ap_id;	// TODO: for uplink traffic. Set STAs destination to the GW
	}

	// Progress bar (trick: it is only printed by node with id 0)
	if(PROGRESS_BAR_DISPLAY){
		if(node_params.node_id == 0){
			if(node_params.print_node_logs) printf("%s PROGRESS BAR:\n", LOG_LVL1);
			trigger_sim_time.Set(SimTime() + PICO_VALUE);
		}
	}

	// ----------------------------------------
	// - For starting saving the nodes from a given timestamp on
	//    node_params.save_node_logs = FALSE;
	//    trigger_start_saving_logs.Set(SimTime() + 3628);
	// ----------------------------------------
	LOGS(node_params.save_node_logs, node_logger.file,"\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	// LOGS(node_params.save_node_logs, node_logger.file, "%f;N%d;S%d;%s;%s Start() END\n", SimTime(), node_params.node_id, node_state, LOG_B01, LOG_LVL1);

};

/**
 * Stop()
 */
void Node :: Stop(){

	LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s Node Stop()\n",
		SimTime(), node_params.node_id, node_state, LOG_C00, LOG_LVL1);

	// Print and write node statistics if required
	if (node_params.print_node_logs) PrintOrWriteNodeStatistics(PRINT_LOG);
	if (node_params.save_node_logs) PrintOrWriteNodeStatistics(WRITE_LOG);

	// Close node logs file
	if(node_params.save_node_logs) fclose(node_logger.file);

	// Save performance into the simulation_performance object
	SaveSimulationPerformance();

	// Save the configuration currently being used by the node
	GenerateConfiguration();

};

/**
 * Handle InportSomeNodeStartTX for STATE_SENSING
 * Called when some node (this one included) starts a transmission
 * @param "notification" [type Notification]: notification containing the information of the transmission start perceived
 */
void Node :: HandleStartTX_StateSensing(const Notification &notification) {

	if(notification.destination_id == node_params.node_id){	// Node IS THE DESTINATION

		// Update power received of interest
		power_rx_interest = power_received_per_node[notification.source_id];

		current_left_channel = notification.left_channel;
		current_right_channel = notification.right_channel;
		current_modulation = notification.modulation_id;

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
			SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3,
			notification.destination_id);

		// Issue #146 RTS/CTS BW indication
		// - If incoming packet is RTS or CTS sent to me, focus first just on the primary channel
		// - If RTS/CTS decodable in the primary, packet not lost.
		// - After decoding the RTS/CTS at the primary, perform CCA assessment in all the range
		// - So, if RTS/CTS: max_pw_interference is now referred just to primary channel interference
		// - Keep max_pw_interference for all range if DATA or ACK.

		if(notification.packet_type == PACKET_TYPE_RTS) {

			// max_pw_interference is interference in primary
			max_pw_interference = channel_power[node_params.current_primary_channel]
				- power_received_per_node[notification.source_id];

		} else {

			// Compute max interference (the highest one perceived in the reception channel range)
			ComputeMaxInterference(&max_pw_interference, &channel_max_intereference,
				notification, node_state, power_received_per_node, &channel_power);
		}

		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %.2f dBm - P_if = %.2f dBm - P_noise = %.2f dBm\n",
			SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
			channel_max_intereference,
			ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
			ConvertPower(PW_TO_DBM, power_rx_interest),
			ConvertPower(PW_TO_DBM, max_pw_interference),
			ConvertPower(PW_TO_DBM, NOISE_LEVEL_DBM));

		if(notification.packet_type == PACKET_TYPE_RTS || notification.packet_type == PACKET_TYPE_ICF) {	// Notification is an RTS or ICF

			current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

			LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s SINR = %.2f dBm\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
				ConvertPower(LINEAR_TO_DB, current_sinr));

			// Check if notification has been lost due to interferences or weak signal strength
			loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification,
					current_sinr, node_params.capture_effect, current_pd,
					power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			if(loss_reason != PACKET_NOT_LOST) {	// If RTS IS LOST, send logical Nack

				// Check if lost due to BO collision
				if(loss_reason == PACKET_LOST_INTERFERENCE){
					if(fabs(outrange_nav_notification.timestamp - notification.timestamp)
						< MAX_DIFFERENCE_SAME_TIME){
						loss_reason = PACKET_LOST_BO_COLLISION;
					}
				}

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
					SimTime(), node_params.node_id, node_state, LOG_D15, LOG_LVL4, notification.packet_id,
					notification.source_id, loss_reason);

				if(node_params.nack_activated) {
					// Send logical NACK to transmitter sending incoming notification for indicating the loss reason
					logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
						node_params.node_id, notification.source_id, NODE_ID_NONE, loss_reason, BER, current_sinr);
					SendLogicalNack(logical_nack);
				}

				if(node_is_transmitter){
					int pause (HandleBackoff(PAUSE_TIMER, &channel_power,
						node_params.current_primary_channel, current_pd, buffer.QueueSize()));
					// Check if node has to freeze the BO (if it is not already frozen)
					if (pause) {
						PauseBackoff();
					}
				}

			} else {	// Data packet IS NOT LOST (it can be properly received)

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of RTS #%d from N%d CAN be started (SINR = %f dB)\n",
					SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id,
					notification.source_id, ConvertPower(LINEAR_TO_DB, current_sinr));

				/*
					* Save incoming notification. This is kept in order to compare new notifications to the current
					* one for determining if it is lost and stuff like that.
					*/
				incoming_notification = notification;

				// Change state and update receiving info
				data_duration = notification.tx_info.data_duration;
				ack_duration = notification.tx_info.ack_duration;
				rts_duration = notification.tx_info.rts_duration;
				cts_duration = notification.tx_info.cts_duration;

				current_left_channel = notification.left_channel;
				current_right_channel = notification.right_channel;

				node_state = STATE_RX_RTS;
				receiving_from_node_id = notification.source_id;
				receiving_packet_id = notification.packet_id;

				// Pause backoff as node has began a reception
				if(node_is_transmitter) PauseBackoff();

			}

		} else if (notification.packet_type == PACKET_TYPE_DATA) {	// 2-way: DATA received directly (no RTS/CTS)

			current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

			loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification,
					current_sinr, node_params.capture_effect, current_pd,
					power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			if(loss_reason != PACKET_NOT_LOST) {

				if(node_params.nack_activated) {
					logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
						node_params.node_id, notification.source_id, NODE_ID_NONE, loss_reason, BER, current_sinr);
					SendLogicalNack(logical_nack);
				}

				if(node_is_transmitter) PauseBackoff();

			} else {

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of DATA #%d from N%d CAN be started (SINR = %f dB)\n",
					SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id,
					notification.source_id, ConvertPower(LINEAR_TO_DB, current_sinr));

				incoming_notification = notification;

				data_duration = notification.tx_info.data_duration;
				ack_duration = notification.tx_info.ack_duration;

				current_left_channel = notification.left_channel;
				current_right_channel = notification.right_channel;

				node_state = STATE_RX_DATA;
				receiving_from_node_id = notification.source_id;
				receiving_packet_id = notification.packet_id;

				if(node_is_transmitter) PauseBackoff();

			}

		} else {	//	Notification does NOT CONTAIN an RTS or DATA
			LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Unexpected packet type (%d) received!\n",
					SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4, notification.packet_type);
		}

	} else {	// Node IS NOT THE DESTINATION

		// MAPC: check for group-addressed ICF targeted at this node's group
		if (notification.packet_type == PACKET_TYPE_ICF
				&& notification.destination_id == NODE_ID_MAPC_BROADCAST
				&& wlan.mapc_enabled
				&& wlan.FindMapcGroupIdx(notification.mapc_group_id) >= 0
				&& notification.source_id != node_params.node_id
				&& node_params.node_type == NODE_TYPE_AP) {

			UpdateSINRFromNotification(notification);
			loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification,
				current_sinr, node_params.capture_effect, current_pd,
				power_rx_interest, node_params.constant_per,
				node_params.node_id, node_params.capture_effect_model);

			if (loss_reason == PACKET_NOT_LOST) {
				incoming_notification = notification;
				data_duration  = notification.tx_info.data_duration;
				ack_duration   = notification.tx_info.ack_duration;
				rts_duration   = notification.tx_info.rts_duration;
				cts_duration   = notification.tx_info.cts_duration;
				// Propagate ICF's NAV so ICR carries a valid nav_time for third-party nodes.
				// Without this, current_nav_time is 0 and AP_C (non-MAPC AP) would try to
				// set trigger_NAV at SimTime()-epsilon => COST trigger crash.
				current_nav_time = notification.tx_info.nav_time;
				current_left_channel  = notification.left_channel;
				current_right_channel = notification.right_channel;
				node_state        = STATE_RX_ICF;
				coordinator_ap_id = notification.source_id;
				++node_stats.num_icf_rx;
				mapc_active_group_idx = wlan.FindMapcGroupIdx(notification.mapc_group_id);
				// Stagger index: count same-group peers with smaller node_id (excl. coordinator)
				mapc_peer_position = 0;
				for (int i = 0; i < wlan.mapc_num_peers[mapc_active_group_idx]; ++i) {
					if (wlan.mapc_peer_ap_ids[mapc_active_group_idx][i] != coordinator_ap_id
							&& wlan.mapc_peer_ap_ids[mapc_active_group_idx][i] < node_params.node_id)
						++mapc_peer_position;
				}
				if (node_is_transmitter) PauseBackoff();
				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Received ICF from N%d -> STATE_RX_ICF (pos=%d)\n",
					SimTime(), node_params.node_id, STATE_RX_ICF, LOG_D07, LOG_LVL2,
					notification.source_id, mapc_peer_position);
			}

		} else if(notification.packet_type == PACKET_TYPE_RTS
			|| notification.packet_type == PACKET_TYPE_CTS
			|| notification.packet_type == PACKET_TYPE_DATA
			|| notification.packet_type == PACKET_TYPE_ACK
			|| notification.packet_type == PACKET_TYPE_ICF
			|| notification.packet_type == PACKET_TYPE_ICR
			|| notification.packet_type == PACKET_TYPE_TF
			|| notification.packet_type == PACKET_TYPE_MU_RTS_TXS) {

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s I am not the TX destination (N%d to N%d). Checking if Frame can be decoded.\n",
				SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL2,
				notification.source_id, notification.destination_id);

			/** Can the packet be decoded? **/
			UpdateSINRFromNotification(notification);
			// 4 - Check if the packet is lost or not
			loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification, current_sinr,
				node_params.capture_effect, current_pd, power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Pmax_intf[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm, sinr = %f dB\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
				channel_max_intereference, ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
				ConvertPower(PW_TO_DBM, power_rx_interest),
				ConvertPower(PW_TO_DBM, max_pw_interference),
				ConvertPower(LINEAR_TO_DB,current_sinr));

			// Spatial Reuse: try to identify SR TXOP; may update loss_reason
			TryIdentifySRTXOP(notification, loss_reason);

			if(loss_reason == PACKET_NOT_LOST) { // RTS/DATA/CTS/ACK can be decoded

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Packet type %d can be decoded\n",
					SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, notification.packet_type);

				// Save NAV notification for comparing timestamps in case of need
				nav_notification = notification;

				int pause (HandleBackoff(PAUSE_TIMER, &channel_power,
					node_params.current_primary_channel, current_pd, buffer.QueueSize()));

				// MAPC: if ICF or TF is for our own MAPC group, stay in STATE_SENSING so we
				// can receive our AP's DATA in the upcoming slot. Covers both STA_A (coord.
				// AP's STA, source == own AP) and STA_B (coordinated AP's STA, source != own AP).
				if ((notification.packet_type == PACKET_TYPE_ICF
							|| notification.packet_type == PACKET_TYPE_TF)
						&& node_params.node_type == NODE_TYPE_STA
						&& wlan.mapc_enabled
						&& wlan.FindMapcGroupIdx(notification.mapc_group_id) >= 0) {
					if (node_is_transmitter) PauseBackoff();
					LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s ICF/TF for own MAPC group %d: stay in SENSING to receive DATA\n",
						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, notification.mapc_group_id);
				// MAPC Co-TDMA: if ICR is for our MAPC group, don't set NAV - DATA will follow
				// for us (STA_A via coordinator AP, STA_B via coordinated AP); stay in SENSING.
				} else if (notification.packet_type == PACKET_TYPE_ICR
						&& node_params.node_type == NODE_TYPE_STA
						&& wlan.mapc_enabled
						&& wlan.FindMapcGroupIdx(notification.mapc_group_id) >= 0) {
					if (node_is_transmitter) PauseBackoff();
					LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s ICR for own MAPC group %d: stay in SENSING to receive upcoming DATA\n",
						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, notification.mapc_group_id);
				// MAPC Co-TDMA: if MU-RTS/TXS is addressed to our own AP, don't set NAV -
				// our AP will send DATA to us in the upcoming slot, so we must stay in STATE_SENSING.
				} else if (notification.packet_type == PACKET_TYPE_MU_RTS_TXS
						&& node_params.node_type == NODE_TYPE_STA
						&& notification.destination_id == wlan.ap_id) {
					if (node_is_transmitter) PauseBackoff();
					LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s MU-RTS/TXS to own AP N%d: stay in SENSING to receive upcoming DATA\n",
						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, wlan.ap_id);
				} else if(pause) {

					// Check if node has to freeze the BO (if it is not already frozen)
					if (node_is_transmitter) {
						PauseBackoff();
					}

					// Update the NAV time according to the frame's info
					current_nav_time = notification.tx_info.nav_time;

					// SERGIO on 28/09/2017:
					// - Ensure NAV TO finishes at same time (or before) than other's WLAN ACK transmission.
					// time_to_trigger = SimTime() + current_nav_time + TIME_OUT_EXTRA_TIME;
					time_to_trigger = SimTime() + current_nav_time - TIME_OUT_EXTRA_TIME;

					// SERGIO_TRIGGER
					// Differentiate between Intra-BSS and Inter-BSS NAV triggers
					if (sr_state.spatial_reuse_enabled && sr_state.type_last_sensed_packet != INTRA_BSS_FRAME) {
						trigger_inter_bss_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
					} else {
						trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
					}

					LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Entering in NAV during %.12f and setting NAV timeout to %.12f\n",
						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3,
						current_nav_time, trigger_NAV_timeout.GetTime());

//							LOGS(node_params.save_node_logs,node_logger.file,
//								"%.15f;N%d;S%d;%s;%s current_nav_time = %.12f\n",
//								SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4,
//								current_nav_time);

					node_state = STATE_NAV;
					node_stats.last_time_not_in_nav = SimTime();
					++node_stats.times_went_to_nav;

				}

			} else { // Frame cannot be decoded.

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Frame sent by N%d could not be decoded for reason %d\n",
					SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3,
					notification.source_id, loss_reason);

				// Save NAV notification for comparing timestamps in case of need
				outrange_nav_notification = notification;

				// Check if DIFS or BO must be stopped
				if(node_is_transmitter){

					LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Checking if BO must be paused...\n",
						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);

					int pause;

					if(sr_state.spatial_reuse_enabled && sr_state.txop_sr_identified) {
						pause = HandleBackoff(PAUSE_TIMER, &channel_power,
							node_params.current_primary_channel, sr_state.current_obss_pd_threshold, buffer.QueueSize());
					} else {
						pause = HandleBackoff(PAUSE_TIMER, &channel_power, node_params.current_primary_channel,
							current_pd, buffer.QueueSize());
					}

					// Check if node has to freeze the BO (if it is not already frozen)
					if (pause) {
						PauseBackoff();
					} else {
						if(trigger_end_backoff.Active()) ca_state.remaining_backoff =
								ComputeRemainingBackoff(node_params.backoff_type, trigger_end_backoff.GetTime() - SimTime());
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s BO must not be paused (%f remaining slots).\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5, ca_state.remaining_backoff/SLOT_TIME);
					}

				}

			}

		}
	}
}


/**
 * Handle InportSomeNodeStartTX for STATE_NAV
 */
void Node :: HandleStartTX_StateNav(const Notification &notification) {

	int nav_collision(0);
	int inter_bss_nav_collision(0);
	ComputeNavCollisions(notification, nav_collision, inter_bss_nav_collision);

	if(notification.destination_id == node_params.node_id){	// Node IS THE DESTINATION

		// DATA from own AP: NAV suppresses transmission, not reception.
		// Always attempt to receive DATA addressed to this node, regardless of whether a
		// simultaneous collision was detected (e.g. Co-SR: peer AP starts 1 event-tick
		// before coordinator, causing this STA to enter NAV from the peer's frame first).
		if(notification.packet_type == PACKET_TYPE_DATA) {

			UpdateSINRFromNotification(notification);

			loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification,
				current_sinr, node_params.capture_effect, current_pd,
				power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			if(loss_reason != PACKET_NOT_LOST) {

				loss_reason = PACKET_LOST_BO_COLLISION;

				if(!node_is_transmitter) {
					trigger_NAV_timeout.Cancel();
					time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;
					trigger_restart_sta.Set(FixTimeOffset(time_to_trigger,13,12));
				}

				if(node_params.nack_activated) {
					logical_nack = GenerateLogicalNack(notification.packet_type,
						notification.packet_id, node_params.node_id, notification.source_id,
						NODE_ID_NONE, loss_reason, BER, current_sinr);
					SendLogicalNack(logical_nack);
				}

			} else {

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of DATA #%d from N%d CAN be started (SINR = %f dB)\n",
					SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id,
					notification.source_id, ConvertPower(LINEAR_TO_DB, current_sinr));

				// Cancel the previous NAV
				if (sr_state.spatial_reuse_enabled) {
					trigger_inter_bss_NAV_timeout.Cancel();
				} else {
					trigger_NAV_timeout.Cancel();
				}

				incoming_notification = notification;

				data_duration = notification.tx_info.data_duration;
				ack_duration = notification.tx_info.ack_duration;

				current_left_channel = notification.left_channel;
				current_right_channel = notification.right_channel;

				node_state = STATE_RX_DATA;
				receiving_from_node_id = notification.source_id;
				receiving_packet_id = notification.packet_id;

				if(node_is_transmitter) PauseBackoff();

			}

		} else if(nav_collision || inter_bss_nav_collision) {	// RTS or other during simultaneous collision

			if(notification.packet_type == PACKET_TYPE_RTS) {	// Notification CONTAINS an RTS PACKET

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s RTS from my AP N%d sent simultaneously\n",
					SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4,
					notification.source_id);

				UpdateSINRFromNotification(notification);

				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm\n",
					SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5, channel_max_intereference,
					ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
					ConvertPower(PW_TO_DBM, power_rx_interest),
					ConvertPower(PW_TO_DBM, max_pw_interference));

				// Check if notification has been lost due to interferences or weak signal strength
				// TODO: method for checking whether the detected transmission can be decoded or not
				loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification,
					current_sinr, node_params.capture_effect, current_pd,
					power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

				if(loss_reason != PACKET_NOT_LOST) {	// If RTS IS LOST, send logical Nack

					loss_reason = PACKET_LOST_BO_COLLISION;

					if(!node_is_transmitter) {

						// Sergio 18/09/2017:
						// NAV is no longer valid. It cannot be decoded due to interference.
						// Wait MAX_DIFFERENCE_SAME_TIME to detect more transmissions sent at the "same" time
						// Trigger the restart then.

						// Sergio on 27/09/2017. Review this case
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s RTS from my AP CANNOT be decoded\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5);

						trigger_NAV_timeout.Cancel();
						time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;

						// trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
						trigger_restart_sta.Set(FixTimeOffset(time_to_trigger,13,12));

					} else {

						printf("ALARM! Should not happen in downlink traffic\n");

					}
					// EOF HandleSlottedBackoffCollision();

					if(node_params.nack_activated) {
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s RTS cannot be decoded (SINR = %f dB) -> Sending NACK corresponding to BO collision to N%d\n",
							SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL5,
							ConvertPower(LINEAR_TO_DB, current_sinr), notification.source_id);
						logical_nack = GenerateLogicalNack(notification.packet_type,
							notification.packet_id, node_params.node_id, notification.source_id,
							NODE_ID_NONE, loss_reason, BER, current_sinr);
						SendLogicalNack(logical_nack);
					}

				} else {	// Data packet IS NOT LOST (it can be properly received)

					LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Reception of RTS #%d from N%d CAN be started (SINR = %f dB)\n",
						SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id,
						notification.source_id, ConvertPower(LINEAR_TO_DB, current_sinr));

					// Cancel the previous NAV
					if ( sr_state.spatial_reuse_enabled ) {
						trigger_inter_bss_NAV_timeout.Cancel(); // Cancel inter-BSS NAV
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s INTER-BSS NAV CANCELLED!\n",
							SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4);
					} else {
						trigger_NAV_timeout.Cancel();			// Cancel intra-BSS NAV (legacy)
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s DEFAULT NAV CANCELLED!\n",
							SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4);
					}

					// Change state and update receiving info
					data_duration = notification.tx_info.data_duration;
					ack_duration = notification.tx_info.ack_duration;
					rts_duration = notification.tx_info.rts_duration;
					cts_duration = notification.tx_info.cts_duration;

					current_left_channel = notification.left_channel;
					current_right_channel = notification.right_channel;

					node_state = STATE_RX_RTS;
					receiving_from_node_id = notification.source_id;
					receiving_packet_id = notification.packet_id;

					incoming_notification = notification;

					// Pause backoff as node has began a reception
					if(node_is_transmitter) PauseBackoff();

				}

			} else {	//	Notification does NOT CONTAIN an RTS or DATA
				LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Unexpected packet type (%d) received!\n",
						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4, notification.packet_type);
			}

		} else { // Notification not detected to happen at the same time

			if (node_params.nack_activated) {
				// Send logical NACK to transmitter sending incoming notification for indicating the loss reason
				logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
					node_params.node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_RX_IN_NAV, BER, current_sinr);
				SendLogicalNack(logical_nack);
			}

		}


	} else { // Node IS NOT THE DESTINATION

		if(notification.packet_type == PACKET_TYPE_RTS
			||  notification.packet_type == PACKET_TYPE_CTS
			|| notification.packet_type == PACKET_TYPE_DATA
			|| notification.packet_type == PACKET_TYPE_ACK) {	// PACKET TYPE RTS OR CTS

			// TODO: determine if can be decoded!

			UpdateSINRFromNotification(notification);
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5, channel_max_intereference,
				ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
				ConvertPower(PW_TO_DBM, power_rx_interest),
				ConvertPower(PW_TO_DBM, max_pw_interference));
			// Check if notification can be decoded
			// TODO: method for checking whether the detected transmission can be decoded or not
			int loss_reason (IsPacketLost(node_params.current_primary_channel, notification, notification,
				current_sinr, node_params.capture_effect, current_pd, power_rx_interest, node_params.constant_per,
				node_params.node_id, node_params.capture_effect_model));

			// NAV collision detected
			if((nav_collision || inter_bss_nav_collision) && loss_reason == PACKET_NOT_LOST)  {

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Updating the NAV according to the last sensed transmission\n",
					SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL2);

				if(!node_is_transmitter) {

					// Cancel the previous NAV and set it again according to the new one
					time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;
					if (sr_state.spatial_reuse_enabled && inter_bss_nav_collision) {
						trigger_inter_bss_NAV_timeout.Cancel(); // Cancel inter-BSS NAV
						trigger_inter_bss_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
						LOGS(node_params.save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s (workaround) setting inter-BSS NAV trigger to %.12f\n",
							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, time_to_trigger);
					} else {
						trigger_NAV_timeout.Cancel();			// Cancel intra-BSS NAV (legacy)
						trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
						LOGS(node_params.save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s (workaround) setting NAV trigger to %.12f\n",
							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, time_to_trigger);
					}

				} else {

					if ( (nav_collision && nav_notification.packet_type == notification.packet_type)
						|| (inter_bss_nav_collision && nav_notification.packet_type == notification.packet_type) ) {

						// if(node_params.save_node_logs) fprintf(node_logger.file,
						//	"%.15f;N%d;S%d;%s;%s Waiting just in case of more collisions.\n",
						//	SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL4);

						// Cancel the previous NAV
						if (sr_state.spatial_reuse_enabled && inter_bss_nav_collision) {
							trigger_inter_bss_NAV_timeout.Cancel();		// Cancel inter-BSS NAV
						} else {
							trigger_NAV_timeout.Cancel();				// Cancel intra-BSS NAV (legacy)
						}

						// Sergio on 27/09/2017
						// - An AP must wait EIFS after the last packet of external RTSs collisions is finished.

						// Sergio on 2018/07/06: EIFS to match Bianchi model
						time_to_trigger =
							SimTime() + notification.tx_info.rts_duration
							+ SIFS + notification.tx_info.cts_duration
							- notification.tx_info.preoccupancy_duration;

						trigger_wait_collisions.Set(FixTimeOffset(time_to_trigger,13,12));

						LOGS(node_params.save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s Recovering from EIFS at %.12f (preoc. = %.12f)\n",
							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL4,
							trigger_wait_collisions.GetTime(),
							notification.tx_info.preoccupancy_duration);

					}
				}

				// Do not send NACK because node is not the destination

			} else { // No collision

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s I am not the TX destination (N%d to N%d). Checking if new RTS/CTS can be decoded.\n",
					SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL2,
					notification.source_id, notification.destination_id);

				// Can the notification be decoded?
				UpdateSINRFromNotification(notification);

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Pmax_intf[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm, sinr = %f dB\n",
					SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
					channel_max_intereference, ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
					ConvertPower(PW_TO_DBM, power_rx_interest),
					ConvertPower(PW_TO_DBM, max_pw_interference),
					ConvertPower(LINEAR_TO_DB,current_sinr));

				// TODO: method for checking whether the detected transmission can be decoded or not
				loss_reason = IsPacketLost(node_params.current_primary_channel, notification, notification,
					current_sinr, node_params.capture_effect, current_pd, power_rx_interest, node_params.constant_per,
					node_params.node_id, node_params.capture_effect_model);

				int power_condition (channel_power[node_params.current_primary_channel] > node_params.sensitivity_default);

				if (loss_reason == PACKET_NOT_LOST && power_condition) {	// Packet IS NOT LOST

					// Spatial Reuse: detect / cancel SR TXOP while in NAV
					DetectSRTXOPInNavState(notification, loss_reason);

				} else {			// Packet IS LOST
					if(notification.packet_type == PACKET_TYPE_RTS
							||  notification.packet_type == PACKET_TYPE_CTS){
						LOGS(node_params.save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s RTS/CTS sent from N%d could not be decoded for reason %d\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3,
							notification.source_id, loss_reason);
					}
				}
			}
		}
	}

}

/**
 * Handle InportSomeNodeStartTX for STATE_TX_DATA
 */
void Node :: HandleStartTX_StateTxData(const Notification &notification) {

	if(notification.destination_id == node_params.node_id){ // Node IS THE DESTINATION

		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
			SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3,
			notification.destination_id);

		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s I am transmitting, packet cannot be received\n",
			SimTime(), node_params.node_id, node_state, LOG_D18, LOG_LVL3);

		if(node_params.nack_activated) {
			// Send logical NACK to incoming notification transmitter due to receiver (node) was already receiving
			logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
				node_params.node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_DESTINATION_TX, BER, current_sinr);

			SendLogicalNack(logical_nack);
		}

	} else {	// Node IS NOT THE DESTINATION, do nothing

//					LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
//						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, notification.destination_id);

	}


	int nav_collision(0);
	int inter_bss_nav_collision(0);
	ComputeNavCollisions(notification, nav_collision, inter_bss_nav_collision);

	// If two or more packets sent at the same time
	if(nav_collision || inter_bss_nav_collision) {

	}
	// Spatial Reuse: detect / cancel SR TXOP while in TX state
	DetectSRTXOPWhileTransmitting(notification);

}

/**
 * Handle InportSomeNodeStartTX for STATE_RX_DATA
 */
void Node :: HandleStartTX_StateRxData(const Notification &notification) {

	if(notification.destination_id == node_params.node_id){	// Node IS THE DESTINATION

//					LOGS(node_params.save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
//							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, notification.destination_id);

		// Update the SINR
		current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

		// Check if ongoing notification has been lost due to interferences caused by new transmission
		loss_reason = IsPacketLost(node_params.current_primary_channel, incoming_notification, notification,
			current_sinr, node_params.capture_effect, current_pd,
			power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

		// TODO: method for checking whether the detected transmission can be decoded or not
		// ...

		switch(node_params.capture_effect_model){

			case CE_DEFAULT:{
				if(loss_reason != PACKET_NOT_LOST
					&& loss_reason != PACKET_LOST_OUTSIDE_CH_RANGE)  {	// If ongoing data packet IS LOST
						// Pure collision (two nodes transmitting to me with enough power)
						LOGS(node_params.save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s Pure collision! Already receiving from N%d\n",
							SimTime(), node_params.node_id, node_state, LOG_D19, LOG_LVL4, receiving_from_node_id);
						loss_reason = PACKET_LOST_PURE_COLLISION;
						// If two or more packets sent at the same time
						if(fabs(notification.timestamp - incoming_notification.timestamp) < MAX_DIFFERENCE_SAME_TIME){
							// SERGIO HandleSlottedBackoffCollision();
							loss_reason = PACKET_LOST_BO_COLLISION;
							if(!node_is_transmitter) {
								time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;
								trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
							} else {
								printf("ALARM! Should not happen in downlink traffic\n");
							}
						}
						if(node_params.nack_activated) {
							// Send NACK to both ongoing transmitter and incoming interferer nodes
							logical_nack = GenerateLogicalNack(notification.packet_type, nav_notification.packet_id,
									node_params.node_id, nav_notification.source_id, notification.source_id, loss_reason, BER, current_sinr);
							SendLogicalNack(logical_nack);
						}

				} else {	// If ongoing data packet IS NOT LOST (incoming transmission does not affect ongoing reception)

					if (node_params.nack_activated) {
						LOGS(node_params.save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s Low strength signal received while already receiving from N%d\n",
							SimTime(), node_params.node_id, node_state, LOG_D20, LOG_LVL4, receiving_from_node_id);

						// Send logical NACK to incoming transmitter indicating that node is already receiving
						logical_nack = GenerateLogicalNack(notification.packet_type, receiving_from_node_id,
								node_params.node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_LOW_SIGNAL_AND_RX, BER, current_sinr);

						SendLogicalNack(logical_nack);
					}

				}
				break;
			}

			case CE_IEEE_802_11:{
				int capture_effect_condition (power_received_per_node[notification.source_id] >
					power_received_per_node[receiving_from_node_id] + node_params.capture_effect);

				if (loss_reason == PACKET_NOT_LOST && capture_effect_condition) {
					if (notification.packet_type == PACKET_TYPE_RTS) {
						// Start decoding the new packet
						incoming_notification = notification;
						// Change state and update receiving info
						data_duration = notification.tx_info.data_duration;
						ack_duration = notification.tx_info.ack_duration;
						rts_duration = notification.tx_info.rts_duration;
						cts_duration = notification.tx_info.cts_duration;
						current_left_channel = notification.left_channel;
						current_right_channel = notification.right_channel;
						node_state = STATE_RX_RTS;
						receiving_from_node_id = notification.source_id;
						receiving_packet_id = notification.packet_id;
						// Pause backoff as node has began a reception
						if(node_is_transmitter) PauseBackoff();
						if (node_params.nack_activated) {
							// Send NACK to both ongoing transmitter and incoming interferer nodes
							logical_nack = GenerateLogicalNack(notification.packet_type, nav_notification.packet_id,
									node_params.node_id, NODE_ID_NONE, notification.source_id, PACKET_LOST_CAPTURE_EFFECT, BER, current_sinr);
							SendLogicalNack(logical_nack);
						}
					}  else {
						// Pure collision (two nodes transmitting to me with enough power)
						LOGS(node_params.save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s Pure collision! Already receiving from N%d\n",
							SimTime(), node_params.node_id, node_state, LOG_D19, LOG_LVL4, receiving_from_node_id);
						loss_reason = PACKET_LOST_PURE_COLLISION;
						// If two or more packets sent at the same time
						if(fabs(notification.timestamp - incoming_notification.timestamp) < MAX_DIFFERENCE_SAME_TIME) {
							loss_reason = PACKET_LOST_BO_COLLISION;
							if(!node_is_transmitter) {
								time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;
								trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
							} else {
								printf("ALARM! Should not happen in downlink traffic\n");
							}
						}
						if(node_params.nack_activated){
							// Send NACK to both ongoing transmitter and incoming interferer nodes
							logical_nack = GenerateLogicalNack(notification.packet_type, nav_notification.packet_id,
									node_params.node_id, nav_notification.source_id, notification.source_id, loss_reason, BER, current_sinr);
							SendLogicalNack(logical_nack);
						}
					}
				} else { // If ongoing data packet IS NOT LOST (incoming transmission does not affect ongoing reception)
					if(node_params.nack_activated){
						LOGS(node_params.save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s Low strength signal received while already receiving from N%d\n",
							SimTime(), node_params.node_id, node_state, LOG_D20, LOG_LVL4, receiving_from_node_id);
						// Send logical NACK to incoming transmitter indicating that node is already receiving
						logical_nack = GenerateLogicalNack(notification.packet_type, receiving_from_node_id,
								node_params.node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_LOW_SIGNAL_AND_RX, BER, current_sinr);
						SendLogicalNack(logical_nack);
					}
				}
				break;
			}
		}

	} else {	// Node is NOT THE DESTINATION

//					LOGS(node_params.save_node_logs, node_logger.file,
//						"%.15f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
//						SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, notification.destination_id);

		// Compute max interference (the highest one perceived in the reception channel range)
		ComputeMaxInterference(&max_pw_interference, &channel_max_intereference,
			incoming_notification, node_state, power_received_per_node, &channel_power);

		// Check if the ongoing reception is affected
		current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

		LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm - current_sinr = %.2f dBm\n",
			SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5, channel_max_intereference,
			ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
			ConvertPower(PW_TO_DBM, power_rx_interest),
			ConvertPower(PW_TO_DBM, max_pw_interference),
			ConvertPower(LINEAR_TO_DB, current_sinr));

		// Check if the notification that was already being received is lost due to new notification
		if (sr_state.spatial_reuse_enabled && sr_state.txop_sr_identified) {
			loss_reason = IsPacketLost(node_params.current_primary_channel, incoming_notification, notification,
				current_sinr, node_params.capture_effect, sr_state.current_obss_pd_threshold,
				power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);
		} else {
			loss_reason = IsPacketLost(node_params.current_primary_channel, incoming_notification, notification,
				current_sinr, node_params.capture_effect, current_pd,
				power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);
		}

		// TODO: method for checking whether the detected transmission can be decoded or not
		// ...

		LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s loss_reason = %d\n",
			SimTime(), node_params.node_id, node_state, LOG_D19, LOG_LVL4, loss_reason);

		if(loss_reason != PACKET_NOT_LOST) { 	// If ongoing packet reception IS LOST

			switch(node_params.capture_effect_model) {

				case CE_DEFAULT:{
					// Collision by hidden node
					LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s Collision by interferences!\n",
						SimTime(), node_params.node_id, node_state, LOG_D19, LOG_LVL4);

						// If two or more packets sent at the same time
					if(node_state == STATE_RX_RTS && notification.packet_type == PACKET_TYPE_RTS){
						if(fabs(notification.timestamp - incoming_notification.timestamp) < MAX_DIFFERENCE_SAME_TIME){
							loss_reason = PACKET_LOST_BO_COLLISION;
						}
					}
					// Send logical NACK to ongoing transmitter
					if (node_params.nack_activated) {
						logical_nack = GenerateLogicalNack(incoming_notification.packet_type,
							incoming_notification.packet_id, node_params.node_id, incoming_notification.source_id,
							NODE_ID_NONE, loss_reason, BER, current_sinr);
						SendLogicalNack(logical_nack);
					}
					RestartNode(FALSE);
					break;
				}

				case CE_IEEE_802_11:{
					int capture_effect_condition = power_received_per_node[notification.source_id] >
						power_received_per_node[receiving_from_node_id] + node_params.capture_effect;
					if (capture_effect_condition) {
						loss_reason = PACKET_LOST_CAPTURE_EFFECT;
						printf("Node %d was in state RX (from %d), and a new notification arrived from %d:\n", node_params.node_id, receiving_from_node_id, notification.source_id);
						printf("	* New RSSI: %f\n", power_received_per_node[notification.source_id]);
						printf("	* Old RSSI: %f:\n", power_received_per_node[receiving_from_node_id]);
						printf("	* CE: %f:\n", node_params.capture_effect);
						printf("	* loss_reason: %d:\n", loss_reason);
						if(node_params.nack_activated){
						// Send NACK to both ongoing transmitter and incoming interferer nodes
						logical_nack = GenerateLogicalNack(notification.packet_type, nav_notification.packet_id,
							node_params.node_id, nav_notification.source_id, notification.source_id, loss_reason, BER, current_sinr);
						SendLogicalNack(logical_nack);
						}
						RestartNode(FALSE);
					}
					break;
				}
			}
		}
	}

}

/**
 * Handle InportSomeNodeStartTX for STATE_WAIT_ACK
 */
void Node :: HandleStartTX_StateWaitAck(const Notification &notification) {

	if(notification.destination_id == node_params.node_id){	// Node is the destination

		power_rx_interest = power_received_per_node[notification.source_id];

		incoming_notification = notification;

//					LOGS(node_params.save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
//							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, notification.destination_id);

		if(notification.packet_type == PACKET_TYPE_ACK){	// ACK packet transmission started

			// Compute max interference (the highest one perceived in the reception channel range)
			ComputeMaxInterference(&max_pw_interference, &channel_max_intereference,
				incoming_notification, node_state, power_received_per_node, &channel_power);

			// Check if notification has been lost due to interferences or weak signal strength
			current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

			// TODO: method for checking whether the detected transmission can be decoded or not
			loss_reason = IsPacketLost(node_params.current_primary_channel, incoming_notification, notification,
					current_sinr, node_params.capture_effect, current_pd,
					power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			if(loss_reason != PACKET_NOT_LOST
					&& loss_reason != PACKET_LOST_OUTSIDE_CH_RANGE) {	// If ACK packet IS LOST, send logical Nack

				LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
						SimTime(), node_params.node_id, node_state, LOG_D15, LOG_LVL4, notification.packet_id,
						notification.source_id, loss_reason);

				if(node_params.nack_activated){
					// Send logical NACK to ACK transmitter
					logical_nack = GenerateLogicalNack(incoming_notification.packet_type, incoming_notification.packet_id,
						node_params.node_id, receiving_from_node_id, NODE_ID_NONE, loss_reason, BER, current_sinr);
					SendLogicalNack(logical_nack);
				}

				// Do nothing until ACK timeout is triggered

			} else {	// If ACK packet IS NOT LOST (it can be properly received)

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of ACK %d from N%d CAN be started\n",
					SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id, notification.source_id);

				// Cancel ACK timeout and go to STATE_RX_ACK while updating receiving info
				trigger_ACK_timeout.Cancel();
				node_state = STATE_RX_ACK;
				receiving_from_node_id = notification.source_id;
				receiving_packet_id = notification.packet_id;

//							LOGS(node_params.save_node_logs, node_logger.file,
//									"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
//									SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.destination_id);

//							LOGS(node_params.save_node_logs, node_logger.file,
//									"%.15f;N%d;S%d;%s;%s current_sinr = %f dB\n",
//									SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL5,
//									ConvertPower(LINEAR_TO_DB,current_sinr));

			}

		}  else {	//	Some packet type received that is not ACK
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s Unexpected packet type received!\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
		}

	} else {	// Node IS NOT THE DESTINATION, do nothing
//
//					LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
//								SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, notification.destination_id);
//
	}

}

/**
 * Handle InportSomeNodeStartTX for STATE_WAIT_CTS
 */
void Node :: HandleStartTX_StateWaitCts(const Notification &notification) {

	if(notification.destination_id == node_params.node_id){	// Node is the destination

		incoming_notification = notification;

//					LOGS(node_params.save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
//							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, notification.destination_id);

		if(notification.packet_type == PACKET_TYPE_CTS){	// CTS packet transmission started

			power_rx_interest = power_received_per_node[notification.source_id];

			// Compute max interference (the highest one perceived in the reception channel range)
			ComputeMaxInterference(&max_pw_interference, &channel_max_intereference,
				incoming_notification, node_state, power_received_per_node, &channel_power);

			// Check if notification has been lost due to interferences or weak signal strength
			current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

//						LOGS(node_params.save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s P_sn = %f dBm (%f pW) - P_st= %f dBm (%f pW)"
//							"- P_if = %f dBm (%f pW)\n",
//							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
//							ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]), channel_power[channel_max_intereference],
//							ConvertPower(PW_TO_DBM, power_rx_interest), power_rx_interest, ConvertPower(PW_TO_DBM, max_pw_interference),
//							max_pw_interference);

			// TODO: method for checking whether the detected transmission can be decoded or not
			loss_reason = IsPacketLost(node_params.current_primary_channel, incoming_notification, notification,
				current_sinr, node_params.capture_effect, current_pd,
				power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			if(loss_reason != PACKET_NOT_LOST
					&& loss_reason != PACKET_LOST_OUTSIDE_CH_RANGE)  {	// If CTS packet IS LOST, send logical Nack

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
					SimTime(), node_params.node_id, node_state, LOG_D15, LOG_LVL4, notification.packet_id,
					notification.source_id, loss_reason);

				if(node_params.nack_activated){
					// Send logical NACK to ACK transmitter
					logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
							node_params.node_id, notification.source_id,
							NODE_ID_NONE, loss_reason, BER, current_sinr);
					SendLogicalNack(logical_nack);
				}

				// Do nothing until ACK timeout is triggered

			} else {	// If CTS packet IS NOT LOST (it can be properly received)

				LOGS(node_params.save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s Reception of CTS #%d from N%d CAN be started\n",
						SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4,
						notification.packet_id, notification.source_id);

				current_left_channel = notification.left_channel;
				current_right_channel = notification.right_channel;

				// Cancel ACK timeout and go to STATE_RX_ACK while updating receiving info
				trigger_CTS_timeout.Cancel();
				node_state = STATE_RX_CTS;
				receiving_from_node_id = notification.source_id;
				receiving_packet_id = notification.packet_id;

				// Change state and update receiving info
				data_duration = notification.tx_info.data_duration;
				ack_duration = notification.tx_info.ack_duration;
				cts_duration = notification.tx_info.cts_duration;

//							LOGS(node_params.save_node_logs, node_logger.file,
//									"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
//									SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.destination_id);

//							LOGS(node_params.save_node_logs, node_logger.file,
//									"%.15f;N%d;S%d;%s;%s current_sinr = %f dB\n",
//									SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL5, ConvertPower(LINEAR_TO_DB,current_sinr));

			}

		}  else {	//	Some packet type received that is not CTS
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s Unexpected packet type received!\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
		}

	} else {	// Node IS NOT THE DESTINATION, do nothing

	}

}

/**
 * Handle InportSomeNodeStartTX for STATE_WAIT_DATA
 */
void Node :: HandleStartTX_StateWaitData(const Notification &notification) {

	if(notification.destination_id == node_params.node_id){	// Node is the destination

		power_rx_interest = power_received_per_node[notification.source_id];
		incoming_notification = notification;

//					LOGS(node_params.save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
//							SimTime(), node_params.node_id, node_state, LOG_D07, LOG_LVL3, notification.destination_id);

		if(notification.packet_type == PACKET_TYPE_DATA){	// DATA packet transmission started

			// Compute max interference (the highest one perceived in the reception channel range)
			ComputeMaxInterference(&max_pw_interference, &channel_max_intereference,
				incoming_notification, node_state, power_received_per_node, &channel_power);

			// Check if notification has been lost due to interferences or weak signal strength
			current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm - current_sinr = %.2f dBm\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5, channel_max_intereference,
				ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
				ConvertPower(PW_TO_DBM, power_rx_interest),
				ConvertPower(PW_TO_DBM, max_pw_interference),
				ConvertPower(LINEAR_TO_DB, current_sinr));

			// TODO: method for checking whether the detected transmission can be decoded or not
			loss_reason = IsPacketLost(node_params.current_primary_channel, incoming_notification, notification,
				current_sinr, node_params.capture_effect, current_pd,
				power_rx_interest, node_params.constant_per, node_params.node_id, node_params.capture_effect_model);

			if(loss_reason != PACKET_NOT_LOST
				&& loss_reason != PACKET_LOST_OUTSIDE_CH_RANGE)  {	// If DATA packet IS LOST, send logical Nack

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
					SimTime(), node_params.node_id, node_state, LOG_D15, LOG_LVL4, notification.packet_id,
					notification.source_id, loss_reason);

				if(node_params.nack_activated){
					// Send logical NACK to DATA transmitter
					logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
							node_params.node_id, notification.source_id,
							NODE_ID_NONE, loss_reason, BER, current_sinr);
					SendLogicalNack(logical_nack);
				}

			} else {	// If DATA packet IS NOT LOST (it can be properly received)

				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s Reception of DATA %d from N%d CAN be started\n",
					SimTime(), node_params.node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id, notification.source_id);

				// Cancel DATA timeout and go to STATE_RX_DATA while updating receiving info
				trigger_DATA_timeout.Cancel();
				node_state = STATE_RX_DATA;
				receiving_from_node_id = notification.source_id;
				receiving_packet_id = notification.packet_id;

				// Change state and update receiving info
				data_duration = notification.tx_info.data_duration;
				ack_duration = notification.tx_info.ack_duration;

			}

		}  else {	//	Some packet type received that is not ACK
			LOGS(node_params.save_node_logs, node_logger.file,
				"%.15f;N%d;S%d;%s;%s Unexpected packet type received!\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
		}

	} else {	// Node IS NOT THE DESTINATION, do nothing

//					LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
//								SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL3, notification.destination_id);

	}

}


void Node :: InportSomeNodeStartTX(Notification &notification){

	LOGS(node_params.save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s InportSomeNodeStartTX(): N%d to N%d sends packet type %d in range %d-%d using a transmit power of %.2f dBm\n",
			SimTime(), node_params.node_id, node_state, LOG_D00, LOG_LVL1,
			notification.source_id, notification.destination_id, notification.packet_type,
			notification.left_channel, notification.right_channel,
			ConvertPower(PW_TO_DBM, notification.tx_info.tx_power));

	LOGS(node_params.save_node_logs,node_logger.file,
	        "%.15f;N%d;S%d;%s;%s Nodes transmitting: ",
			SimTime(), node_params.node_id, node_state, LOG_D00, LOG_LVL3);

	// Identify node that has started the transmission as transmitting node in the array
	nodes_transmitting[notification.source_id] = TRUE;
	PrintOrWriteNodesTransmitting(WRITE_LOG, node_params.save_node_logs,
		node_params.print_node_logs, node_logger, node_params.total_nodes_number, nodes_transmitting);

	// TOKENIZED BO ONLY
	if(node_is_transmitter && node_params.backoff_type == BACKOFF_TOKENIZED) {
		// 1 - Check that the incoming transmission is not originated or directed to the transmitter
		if (notification.source_id != node_params.node_id && notification.destination_id != node_params.node_id) {
			// 2 - Check that the incoming transmission is an RTS or DATA
			if (notification.packet_type == PACKET_TYPE_RTS
				|| notification.packet_type == PACKET_TYPE_DATA) {
				// 3 - Check that the incoming transmission comes from a nearby device
				if (received_power_array[notification.source_id] > current_pd) {
					// Update the list of neighboring devices (if not done)
					if (token_order_list[notification.source_id] == DEVICE_INACTIVE_FOR_TOKEN) {
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token-based channel access operation (update neighbor list):\n",
							SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3);
						LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Token's order list before the update: ",
								SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL5);
							PrintOrWriteTokenList(WRITE_LOG, node_params.save_node_logs, node_logger,
								node_params.print_node_logs, token_order_list, node_params.total_nodes_number);
						UpdateTokenList(ADD_DEVICE_TO_LIST, &token_order_list, notification.source_id);
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token's order list updated: ",
							SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL5);
						PrintOrWriteTokenList(WRITE_LOG, node_params.save_node_logs, node_logger,
							node_params.print_node_logs, token_order_list, node_params.total_nodes_number);
					}
					// Update the status of the token
					if (node_state != STATE_TX_RTS) {
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token-based channel access operation (token ACQUISITION):\n",
							SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL4);
						UpdateTokenStatus(node_params.node_id, TAKE_TOKEN, &token_status, notification.source_id,
							token_order_list, node_params.total_nodes_number, &distance_to_token);
					} else if (node_state == STATE_TX_RTS && node_params.node_id < notification.source_id) {
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token-based channel access operation (token ACQUISITION):\n",
							SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL4);
						UpdateTokenStatus(node_params.node_id, TAKE_TOKEN, &token_status, node_params.node_id,
							token_order_list, node_params.total_nodes_number, &distance_to_token);
					} else {
						// In case of a collision (two simultaneous RTS transmissions occur),
						// solve the conflict by releasing the token (the node with lowest ID gets the token)
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token-based channel access operation (token RELEASE):\n",
							SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL4);
						UpdateTokenStatus(node_params.node_id, RELEASE_TOKEN, &token_status, node_params.node_id,
							token_order_list, node_params.total_nodes_number, &distance_to_token);
					}
					LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Token status updated, the new token holder is %d\n",
						SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL5, token_status);
					// Update the CW parameters
					HandleContentionWindow(
						node_params.cw_adaptation, -1, &ca_state.deterministic_bo_active, &ca_state.current_cw_min, &ca_state.current_cw_max, &ca_state.cw_stage_current,
						node_params.cw_min_default, node_params.cw_max_default, node_params.cw_stage_max, distance_to_token, node_params.backoff_type);
					LOGS(node_params.save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Updated CW parameters (token-based BO) = [%d-%d]\n",
						SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL5, ca_state.current_cw_min, ca_state.current_cw_max);
				}
			}
		}
	}

	if(notification.source_id == node_params.node_id){ // If OWN NODE IS THE TRANSMITTER, do nothing

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s I have started a TX of packet #%d (type %d) to N%d in channels %d - %d of duration %.9f us\n",
			SimTime(), node_params.node_id, node_state, LOG_D02, LOG_LVL2, notification.packet_id,
			notification.packet_type, notification.destination_id,
			notification.left_channel, notification.right_channel, notification.tx_duration * pow(10,6));


	} else {	// If OTHER NODE IS THE TRANSMITTER

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s N%d has started a TX of packet #%d (type %d) to N%d in channels %d - %d\n",
			SimTime(), node_params.node_id, node_state, LOG_D02, LOG_LVL2, notification.source_id,
			notification.packet_id,	notification.packet_type, notification.destination_id,
			notification.left_channel, notification.right_channel);

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Power sensed per channel BEFORE updating [dBm]: ",
			SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3);

		PrintOrWriteChannelPower(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
					&channel_power);

		// Update 'power received' array in case a new tx power is used
		if (notification.tx_info.flag_change_in_tx_power) {
			received_power_array[notification.source_id] =
				ComputePowerReceived(distances_array[notification.source_id],
				notification.tx_info.tx_power, node_params.central_frequency, node_params.path_loss_model);
		}

		// Update the power sensed at each channel
		UpdateChannelsPower(&channel_power, notification, TX_INITIATED,
			node_params.central_frequency, node_params.path_loss_model, node_params.adjacent_channel_model, received_power_array[notification.source_id], node_params.node_id);

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Power sensed per channel [dBm]: ",
			SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3);

		PrintOrWriteChannelPower(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
			&channel_power);

		// Call UpdatePowerSensedPerNode() ONLY for adding power (some node started)
		UpdatePowerSensedPerNode(node_params.current_primary_channel, power_received_per_node, notification,
			node_params.central_frequency, node_params.path_loss_model, received_power_array[notification.source_id], TX_INITIATED,
			node_params.x, node_params.y, node_params.z);

		UpdateTimestampChannelFreeAgain(timestamp_channel_becomes_free, &channel_power,
			current_pd, SimTime());

		// Spatial Reuse: identify frame origin and potential OBSS/PD threshold
		UpdateSRStateForIncomingFrame(notification);

		// Decide action according to current state and Notification initiated
		switch(node_state){

			case STATE_SENSING:{
				HandleStartTX_StateSensing(notification);
				break;
			}
			/* ---------- */
			case STATE_TX_DATA:
			case STATE_TX_ACK:{
				HandleStartTX_StateTxData(notification);
				break;
			}
			/* ---------- */
			case STATE_RX_DATA:
			case STATE_RX_ACK:{
				HandleStartTX_StateRxData(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_ACK:{
				HandleStartTX_StateWaitAck(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_CTS:{
				HandleStartTX_StateWaitCts(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_DATA:{
				HandleStartTX_StateWaitData(notification);
				break;
			}
			/* ---------- */
			case STATE_TX_RTS:
			case STATE_TX_CTS:{
				HandleStartTX_StateTxData(notification);
				break;
			}
			/* ---------- */
			case STATE_RX_RTS:
			case STATE_RX_CTS:{
				HandleStartTX_StateRxData(notification);
				break;
			}
			/* ---------- */
			/* ---------- */
			case STATE_NAV:{
				HandleStartTX_StateNav(notification);
				break;
			}
			case STATE_SLEEP:{
				// do nothing
				break;
			}
			/* ---------- */
			/* MAPC states: TX states ignore incoming (already transmitting) */
			case STATE_TX_ICF:
			case STATE_TX_ICR:
			case STATE_TX_TF:
			case STATE_TX_ACK_TF:{
				HandleStartTX_StateTxData(notification);
				break;
			}
			/* ---------- */
			/* MAPC reception states: treat as RX interference */
			case STATE_RX_ICF:
			case STATE_RX_ICR:
			case STATE_RX_MU_RTS:
			case STATE_RX_TF:{
				HandleStartTX_StateRxData(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_ICR:{
				HandleStartTX_StateWaitIcr(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_MU_RTS:{
				HandleStartTX_StateWaitMuRts(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_TF:{
				HandleStartTX_StateWaitTf(notification);
				break;
			}
			/* ---------- */
			case STATE_WAIT_ACK_TF:{
				// ACK TF is in flight from coordinator; do nothing (handle completion in FinishTX)
				HandleStartTX_StateTxData(notification);
				break;
			}
			/* ---------- */
			default:{
				printf("ERROR: %d is not a correct state\n", node_state);
				exit(EXIT_FAILURE);
			}


		}


	}

	// STATISTICS: compute the time the channel is idle (Node 0 is responsible to monitor this)
	if (node_params.node_id == 0 && node_stats.channel_idle) {
		node_stats.sum_time_channel_idle += (SimTime() - node_stats.last_time_channel_is_idle);
		node_stats.channel_idle = false;
	}

	// LOGS(node_params.save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s InportSomeNodeStartTX() END\n", SimTime(), node_params.node_id, node_state, LOG_D01, LOG_LVL1);
};

/**
 * Helper for InportSomeNodeFinishTX: handles STATE_SENSING case
 */
void Node :: HandleFinishTX_StateSensing(const Notification &notification){

	if(node_is_transmitter) {
		if(!trigger_start_backoff.Active()
			&& !trigger_end_backoff.Active()){	// BO was paused and DIFS not initiated

			int resume (HandleBackoff(RESUME_TIMER, &channel_power, node_params.current_primary_channel, current_pd,
					buffer.QueueSize()));

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm (%f)\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
				node_params.current_primary_channel, ConvertPower(PW_TO_DBM, channel_power[node_params.current_primary_channel]), channel_power[node_params.current_primary_channel]);

			if (resume) {	// BO can be resumed
				// Sergio on 26/09/2017. EIFS vs NAV.
				// - To identify if previous packet lost to trigger the EIFS
				// - If not, just resume the backoff
				time_to_trigger = SimTime() + DIFS;
				// time_to_trigger = SimTime() + SIFS + notification.tx_info.cts_duration + DIFS;
				trigger_start_backoff.Set(FixTimeOffset(time_to_trigger,13,12));
				LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s BO will be resumed after DIFS at %.12f.\n",
					SimTime(), node_params.node_id, node_state, LOG_E11, LOG_LVL4,
					trigger_start_backoff.GetTime());
//							LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s EIFS started.\n",
//														SimTime(), node_params.node_id, node_state, LOG_E11, LOG_LVL4);
			} else {	// BO cannot be resumed
				LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s EIFS/DIFS cannot be started because the channel is busy.\n",
					SimTime(), node_params.node_id, node_state, LOG_E11, LOG_LVL4);

}
		} else {	// BO was already active
			LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s BO was already active.\n",
					SimTime(), node_params.node_id, node_state, LOG_E11, LOG_LVL4);
		}
	}
}

/**
 * Helper for InportSomeNodeFinishTX: handles STATE_RX_DATA case
 */
void Node :: HandleFinishTX_StateRxData(const Notification &notification){

	if(notification.destination_id == node_params.node_id){ 	// Node IS THE DESTINATION

		if(notification.packet_type == PACKET_TYPE_DATA){	// Data packet transmission finished

			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Packet #%d reception from N%d is finished successfully.\n",
				SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3, notification.packet_id,
				notification.source_id);

			// Build ACK notification (sent immediately for normal case, or after ACK TF for Co-BF/Co-SR)
			// NAV must be computed *before* GenerateNotification so that sensing nodes receiving
			// the ACK start notification get the correct nav_time (avoids COST e->time>=m_clock crash).
			current_tx_duration = ack_duration;
			current_destination_id = notification.source_id;
			current_nav_time = ComputeNavTime(STATE_TX_ACK, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
			current_nav_time = FixTimeOffset(current_nav_time, 13, 12);
			ack_notification = GenerateNotification(PACKET_TYPE_ACK, current_destination_id,
					notification.packet_id, notification.tx_info.num_packets_aggregated,
					notification.timestamp_generated, current_tx_duration);

						// Reset the flag that indicates whether the tx power changed or not
						sr_state.flag_change_in_tx_power = FALSE;

						// Set the preoccupancy duration for the ACK
			ack_notification.tx_info.preoccupancy_duration = time_rand_value;


			// ------------------------------------------------------------------------
			// Sergio on 07 Dec 2017: add ACK transmission time to spectrum utilization
			for(int c = current_left_channel; c <= current_right_channel; ++c){
				node_stats.total_time_channel_busy_per_channel[c] = node_stats.total_time_channel_busy_per_channel[c] + current_tx_duration;
			}
			// ------------------------------------------------------------------------

			// Co-BF/Co-SR: check MAPC method (mapc_group_id alone is set for all MAPC modes)
			int _cobf_cosr_g = wlan.FindMapcGroupIdx(notification.mapc_group_id);
			int _cobf_cosr = (_cobf_cosr_g >= 0)
				&& (wlan.mapc_method_ids[_cobf_cosr_g] == CO_BF
					|| wlan.mapc_method_ids[_cobf_cosr_g] == CO_SR);
			if (_cobf_cosr) {
				// Co-BF/Co-SR: coordinator will send ACK TF; hold ACK until then.
				// Timeout = SIFS + rts_duration (ACK TF size) + extra; use rts_duration,
				// NOT cts_duration � ACK TF is the same size as RTS, not CTS.
				node_state = STATE_WAIT_ACK_TF;
				time_to_trigger = SimTime() + SIFS + notification.tx_info.rts_duration + TIME_OUT_EXTRA_TIME;
				trigger_DATA_timeout.Set(FixTimeOffset(time_to_trigger, 13, 12));
				LOGS(node_params.save_node_logs, node_logger.file,
					"%.15f;N%d;S%d;%s;%s DATA from N%d done; waiting for ACK TF from coordinator.\n",
					SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3, notification.source_id);
			} else {
				// Normal case: send ACK directly after SIFS (nav already computed above)
				node_state = STATE_TX_ACK;
				time_to_trigger = SimTime() + SIFS;
				trigger_SIFS.Set(FixTimeOffset(time_to_trigger,13,12));
				LOGS(node_params.save_node_logs,node_logger.file,
					"%.15f;N%d;S%d;%s;%s SIFS will be triggered in %.12f\n",
					SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
					trigger_SIFS.GetTime());
			}

		} else {	// Other packet type transmission finished
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
				SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
		}

	} else {	// Node IS NOT THE DESTINATION, do nothing

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Still locked into the reception of packet #%d from N%d.\n",
			SimTime(), node_params.node_id, node_state, LOG_E15, LOG_LVL3, notification.packet_id,
			notification.source_id);

	}
}

/**
 * Helper for InportSomeNodeFinishTX: handles STATE_TX_DATA/STATE_TX_ACK/STATE_WAIT_ACK/etc. cases
 */
void Node :: HandleFinishTX_StateTxData(const Notification &notification){
	// Do nothing
}

/**
 * Called when some node (this one included) finishes a packet TX (RTS, CTS, Data, or ACK)
 * @param "notification" [type Notification]: notification containing the information of the transmission that has finished
 */
void Node :: InportSomeNodeFinishTX(Notification &notification){

	LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s InportSomeNodeFinishTX(): N%d to N%d (type %d)"
			" at range %d-%d "
			"- nodes transmitting: ",
		SimTime(), node_params.node_id, node_state, LOG_E00, LOG_LVL1,
		notification.source_id, notification.destination_id, notification.packet_type,
		notification.left_channel, notification.right_channel);

	// Identify node that has finished the transmission as non-transmitting node in the array
	nodes_transmitting[notification.source_id] = FALSE;
	PrintOrWriteNodesTransmitting(WRITE_LOG, node_params.save_node_logs,
			node_params.print_node_logs, node_logger, node_params.total_nodes_number, nodes_transmitting);

	// Update the list of neighboring devices
	//UpdateTokenList(ADD_DEVICE_TO_LIST, &token_order_list, notification.source_id);

	// TOKENIZED BO ONLY
	if(node_is_transmitter && node_params.backoff_type == BACKOFF_TOKENIZED &&
			token_order_list[notification.destination_id] == DEVICE_ACTIVE_FOR_TOKEN) {
		// - Check that the incoming transmission is an ACK
		if (notification.packet_type == PACKET_TYPE_ACK) {
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Token-based channel access operation (token RELEASE):\n",
				SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL4);
			// Update the status of the token
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s UPDATING the status of the token (until now, with N%d)\n",
				SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3, token_status);
			UpdateTokenStatus(node_params.node_id, RELEASE_TOKEN, &token_status, notification.destination_id,
					token_order_list, node_params.total_nodes_number, &distance_to_token);
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Token status updated, the new token holder is %d\n",
				SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL5, token_status);
			// Update the CW parameters
			HandleContentionWindow(
				node_params.cw_adaptation, -1, &ca_state.deterministic_bo_active, &ca_state.current_cw_min, &ca_state.current_cw_max, &ca_state.cw_stage_current,
				node_params.cw_min_default, node_params.cw_max_default, node_params.cw_stage_max, distance_to_token, node_params.backoff_type);
			LOGS(node_params.save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Updated CW parameters (token-based BO) = [%d-%d]\n",
				SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL5, ca_state.current_cw_min, ca_state.current_cw_max);
		}
	}

	if(notification.source_id == node_params.node_id){	// Node is the TX source: do nothing

//		LOGS(node_params.save_node_logs,node_logger.file,
//				"%.15f;N%d;S%d;%s;%s I have finished the TX of packet #%d (type %d) in channel range: %d - %d\n",
//				SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL2, notification.packet_id,
//				notification.packet_type, notification.left_channel, notification.right_channel);

	} else {	// Node is not the TX source

//		LOGS(node_params.save_node_logs,node_logger.file,
//				"%.15f;N%d;S%d;%s;%s N%d has finished the TX of packet #%d (type %d) in channel range: %d - %d\n",
//				SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL2, notification.source_id,
//				notification.packet_id, notification.packet_type, notification.left_channel,
//				notification.right_channel);

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Power sensed per channel BEFORE updating [dBm]: ",
			SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3);

		PrintOrWriteChannelPower(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
					&channel_power);

		// Update the power sensed at each channel
		UpdateChannelsPower(&channel_power, notification, TX_FINISHED,
			node_params.central_frequency, node_params.path_loss_model, node_params.adjacent_channel_model, received_power_array[notification.source_id], node_params.node_id);

		// -------------------------
		// Safety condtion. Empty the channel when no node is transmitting
		int num_nodes_transmitting = 0;
		for(int i = 0; i < node_params.total_nodes_number; ++i){
			if(nodes_transmitting[i] == TRUE){
				++num_nodes_transmitting;
			}
		}
		if(num_nodes_transmitting == 0){
			for(int i = 0; i < NUM_CHANNELS_KOMONDOR; ++i){
				channel_power[i] = 0;
			}
		}
		// End of safety condition
		// -------------------------

		// Spatial Reuse: update type of ongoing transmissions
		if(sr_state.spatial_reuse_enabled) {
			UpdateTypeOngoingTransmissions(sr_state.type_ongoing_transmissions_sr,
				notification, node_params.bss_color, node_params.srg, 0);
		}

		LOGS(node_params.save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Power sensed per channel [dBm]: ",
			SimTime(), node_params.node_id, node_state, LOG_E18, LOG_LVL3);

		PrintOrWriteChannelPower(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
				&channel_power);

		// Call UpdatePowerSensedPerNode() ONLY for adding power (some node started)
		UpdatePowerSensedPerNode(node_params.current_primary_channel, power_received_per_node, notification,
			node_params.central_frequency, node_params.path_loss_model, received_power_array[notification.source_id], TX_FINISHED,
			node_params.x, node_params.y, node_params.z);

		UpdateTimestampChannelFreeAgain(timestamp_channel_becomes_free, &channel_power,
			current_pd, SimTime());


		switch(node_state){

			/* STATE_SENSING:
			 * - handle backoff
			 */
			case STATE_SENSING:{
				HandleFinishTX_StateSensing(notification);
				break;
			}

			/* STATE_RX_DATA:
			 * - If node IS the destination and data packet transmission finished:
			 *   * start SIFS and generate ACK
			 *   * state = STATE_TX_ACK
			 * - If node IS NOT the destination: do nothing
			 */
			case STATE_RX_DATA:{
				HandleFinishTX_StateRxData(notification);
				break;
			}

			/* STATE_RX_ACK:
			 * - If node IS the destination and ACK packet transmission finished:
			 *   * decrease contention window and restart node
			 *   * state = STATE_SENSING (implicit on restart)
			 * - If node IS NOT the destination: do nothing
			 */
			case STATE_RX_ACK:{	// Check if the current reception is finished

				if(notification.destination_id == node_params.node_id){	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_ACK){	// ACK packet transmission finished

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s ACK #%d reception from N%d is finished successfully.\n",
							SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3, notification.packet_id,
							notification.source_id);

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s -------------------------------------\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL2);
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s TRANSMISSION #%d SUCCESSFULLY FINISHED!\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL2,
							node_stats.data_packets_acked);
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s -------------------------------------\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL2);

						// Mark the previous transmission as successful
						last_transmission_successful = 1;

						// Whole data packet ACKed
						++node_stats.data_packets_acked;
						++node_stats.data_packets_acked_per_sta[current_destination_id-node_params.node_id-1];
						++performance_report.data_packets_acked;

						current_tx_duration = current_tx_duration + (notification.tx_duration + SIFS);	// Add ACK time to tx_duration

						// Update packet statistics
						for(int i = 0; i < limited_num_packets_aggregated; ++i){

							++node_stats.data_frames_acked;
							++node_stats.data_frames_acked_per_sta[current_destination_id-node_params.node_id-1];
							++performance_report.data_frames_acked;
							++node_stats.num_delay_measurements;
							// Use GetPacketAt(i) so each aggregated frame gets its own timestamp
							double pkt_ts = buffer.GetPacketAt(i).timestamp_generated;
							node_stats.sum_delays = node_stats.sum_delays + (SimTime() - pkt_ts);

							if (pkt_ts > (node_params.simulation_time_komondor - node_stats.last_measurements_window)) {
								++node_stats.last_data_frames_acked;
								++node_stats.last_num_delay_measurements;
								node_stats.last_sum_delays = node_stats.last_sum_delays + (SimTime() - pkt_ts);
							}

							// Update the performance report with delay measurements
							performance_report.sum_delays = performance_report.sum_delays + (SimTime() - pkt_ts);
							++performance_report.num_delay_measurements;
							if ((SimTime() - pkt_ts) > performance_report.max_delay) {
								performance_report.max_delay = (SimTime() - pkt_ts);
							}
							if ((SimTime() - pkt_ts) < performance_report.min_delay) {
								performance_report.min_delay = (SimTime() - pkt_ts);
							}
							LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Packet delay: %f us (generated at %f).\n",
								SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL4,
								(SimTime() - pkt_ts) * pow(10,6),
								pkt_ts);

							// Deletion handled in PrepareNewTransmission (node_packet_methods.h)
							//buffer.DelFirstPacket();

						}

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Handling contention window\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
						LOGS(node_params.save_node_logs,node_logger.file,
									"%.15f;N%d;S%d;%s;%s From CW = [%d-%d], b = %d, m = %d\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
							ca_state.current_cw_min, ca_state.current_cw_max, ca_state.cw_stage_current, node_params.cw_stage_max);
						// - Transmission succeeded ---> reset CW if binary exponential backoff is implemented
						HandleContentionWindow(
							node_params.cw_adaptation, RESET_CW, &ca_state.deterministic_bo_active, &ca_state.current_cw_min, &ca_state.current_cw_max,
							&ca_state.cw_stage_current, node_params.cw_min_default, node_params.cw_max_default, node_params.cw_stage_max, distance_to_token, node_params.backoff_type);

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s To CW = [%d-%d], b = %d, m = %d\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL5,
							ca_state.current_cw_min, ca_state.current_cw_max, ca_state.cw_stage_current, node_params.cw_stage_max);

						// Only the TXOP coordinator sends MU-RTS after its own DATA/ACK.
						// coordinator_ap_id == NODE_ID_NONE identifies the coordinator
						// (reset in EndBackoff; set to peer id when an ICF is received).
						if (wlan.mapc_enabled && wlan.mapc_method_ids[mapc_active_group_idx] == CO_TDMA
								&& coordinator_ap_id == NODE_ID_NONE) {
							if (mapc_peer_has_data) {
								// Peer has data (reported in ICR): grant its TDMA slot via MU-RTS/TXS
								node_state = STATE_TX_RTS;
								current_tx_duration = rts_duration;
								current_destination_id = mapc_selected_peer_id;
								mu_rts_notification = GenerateNotification(PACKET_TYPE_MU_RTS_TXS,
									mapc_selected_peer_id, data_notification.packet_id,
									limited_num_packets_aggregated,
									data_notification.timestamp_generated, current_tx_duration);
								mu_rts_notification.mapc_group_id = wlan.mapc_group_ids[mapc_active_group_idx];
								// MU-RTS NAV covers AP_B DATA + ACK slot (uses coordinator's capped data_duration)
								current_nav_time = SIFS + data_duration + SIFS + ack_duration;
								mu_rts_notification.tx_info.nav_time = current_nav_time;
								// Pass per-AP duration allocation to coordinated AP
								mu_rts_notification.tx_info.mapc_allocated_data_duration = mapc_txop_per_ap_data_duration;
								time_rand_value = 0;
								time_to_trigger = SimTime() + SIFS;
								trigger_SIFS.Set(FixTimeOffset(time_to_trigger, 13, 12));
								LOGS(node_params.save_node_logs,node_logger.file,
									"%.15f;N%d;S%d;%s;%s Sending MU-RTS/TXS to N%d (selected peer) after SIFS\n",
									SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
									current_destination_id);
							} else {
								// Peer has no data (reported in ICR): skip MU-RTS slot
								LOGS(node_params.save_node_logs,node_logger.file,
									"%.15f;N%d;S%d;%s;%s N%d has no data (ICR flag); skipping MU-RTS slot\n",
									SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
									mapc_selected_peer_id);
								++mapc_current_peer_idx;
								RestartNode(FALSE);
							}
						} else {
							// Standard: restart node
							RestartNode(FALSE);
						}

					} else {	// Other packet type transmission finished
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
							SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION

					LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Still receiving packet #%d reception from N%d.\n",
						SimTime(), node_params.node_id, node_state, LOG_E15, LOG_LVL3, incoming_notification.packet_id,
						incoming_notification.source_id);
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
			case STATE_NAV:{
				HandleFinishTX_StateTxData(notification);
				break;
			}

			/* STATE_RX_RTS:
			 * -
			 */
			case STATE_RX_RTS:{

				if(notification.destination_id == node_params.node_id){	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_RTS){	// RTS packet transmission finished

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s RTS #%d reception from N%d is finished successfully.\n",
							SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3, notification.packet_id,
							notification.source_id);

						// Check channel availability in order to send the CTS
						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Checking if CTS can be sent: P_sen = %f dBm, pd = %f dBm.\n",
							SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
							ConvertPower(PW_TO_DBM, channel_power[node_params.current_primary_channel]),
							ConvertPower(PW_TO_DBM, current_pd));


						// Issue #146 RTS/CTS BW indication
						// - If incoming packet is RTS decodable:
						// 1. Perform CCA assessment in full range
						// 2. Derive new operation BW (e.g., if original range was 0 to 7 and 5 is busy, report 0 to 3)
						// 3. Transmit (later) CTS

						int CTS_transmission_possible = FALSE;

						GetChannelOccupancyByCCA(node_params.current_primary_channel, node_params.pifs_activated, channels_free, current_left_channel,
								current_right_channel, &channel_power, current_pd, timestamp_channel_becomes_free, SimTime(), PIFS);

						LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Channels founds free after RTS: ",
								SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL3);

						PrintOrWriteChannelsFree(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
							channels_free);

						GetTxChannels(channels_for_tx, node_params.current_dcb_policy, channels_free,
								current_left_channel, current_right_channel, node_params.current_primary_channel,
								NUM_CHANNELS_KOMONDOR, &channel_power, channel_aggregation_cca_model);
						
						LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Channels for transmitting after RTS: ",
								SimTime(), node_params.node_id, node_state, LOG_F02, LOG_LVL2);

						PrintOrWriteChannelForTx(WRITE_LOG, node_params.save_node_logs, node_params.print_node_logs, node_logger,
							channels_for_tx);

						if(channels_for_tx[0] != TX_NOT_POSSIBLE){

							// Get the transmission channels
							current_left_channel = GetFirstOrLastTrueElemOfArray(FIRST_TRUE_IN_ARRAY,
								channels_for_tx, NUM_CHANNELS_KOMONDOR);
							current_right_channel = GetFirstOrLastTrueElemOfArray(LAST_TRUE_IN_ARRAY,
								channels_for_tx, NUM_CHANNELS_KOMONDOR);

							CTS_transmission_possible = TRUE;

						} else{
							CTS_transmission_possible = FALSE;
						}


						//if(ConvertPower(PW_TO_DBM, channel_power[node_params.current_primary_channel]) < current_pd) {

						if(CTS_transmission_possible){

							LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Channel(s) is (are) clear! Sending CTS to N%d (STATE = %d) ...\n",
								SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3, current_destination_id, node_state);

							node_state = STATE_TX_CTS;
							// Generate and send CTS to transmitter after SIFS
							current_destination_id = notification.source_id;
							current_tx_duration = cts_duration;

							// Compute the NAV time
							bits_ofdm_sym =  GetNumberSubcarriers(current_right_channel - current_left_channel +1) *
								Mcs_array::modulation_bits[notification.modulation_id-1] *
								Mcs_array::coding_rates[notification.modulation_id-1] *
								IEEE_AX_SU_SPATIAL_STREAMS;

							ComputeFramesDuration(&rts_duration, &cts_duration, &data_duration, &ack_duration,
								num_channels_tx, notification.modulation_id, notification.tx_info.num_packets_aggregated,
								node_params.frame_length, bits_ofdm_sym);

							current_nav_time = ComputeNavTime(node_state, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
							current_nav_time = FixTimeOffset(current_nav_time,13,12); // Update the NAV time according to the time offsets

							// ------------------------------------------------------------------------
							// Sergio on 07 Dec 2017: add CTS transmission time to spectrum utilization
							for(int c = current_left_channel; c <= current_right_channel; ++c){
								node_stats.total_time_channel_busy_per_channel[c] = node_stats.total_time_channel_busy_per_channel[c] + current_tx_duration;
							}
							// ------------------------------------------------------------------------

							time_to_trigger = SimTime() + SIFS;
							trigger_SIFS.Set(FixTimeOffset(time_to_trigger,13,12)); // triggers the SendResponsePacket() function after SIFS

							LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s SIFS will be triggered in %.12f\n",
								SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
								trigger_SIFS.GetTime());

							cts_notification = GenerateNotification(PACKET_TYPE_CTS, current_destination_id,
								notification.packet_id, notification.tx_info.num_packets_aggregated,
								notification.timestamp_generated, notification.tx_info.total_tx_power);

							cts_notification.tx_duration = current_tx_duration;

                            // Reset the flag that indicates whether the tx power changed or not
                            sr_state.flag_change_in_tx_power = FALSE;


							// Workaround to solve the e->clock timer issue
							// (occurs when being in NAV and noticing a collision of two or more CTS frames)
							cts_notification.tx_info.preoccupancy_duration = time_rand_value;

						} else {
							// CANNOT START PACKET TX

							LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s CTS TX NOT POSSIBLE\n",
								SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);

							/*
							 * STAs should wait MAX_DIFFERENCE_SAME_TIME in order to avoid entering in NAV when it is not required.
							 * E.g. STA A is sensing and is able to decode a packet from AP A. At the same time AP B transmits and
							 * harms AP A - STA A transmission. STA A is restarted. Again, at the same time AP C transmits. Then,
							 * in order to avoid entering in NAV when in fact a slotted BO collision did happen, STA A should not
							 * listen to AP C packet. After MAX_DIFFERENCE_SAME_TIME, no same time events are ensured and STA A can
							 * start sensing again.
							 */
							if(!node_is_transmitter) {
								time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;
								trigger_restart_sta.Set(FixTimeOffset(time_to_trigger,13,12));
							} else {
								RestartNode(FALSE);
							}
						}

					} else {	// Other packet type transmission finished
						LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
								SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION

					LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Still receiving packet #%d reception from N%d.\n",
							SimTime(), node_params.node_id, node_state, LOG_E15, LOG_LVL3, incoming_notification.packet_id,
							incoming_notification.source_id);
				}

				break;
			}


			/* STATE_RX_CTS:
			 * -
			 */
			case STATE_RX_CTS:{

				if(notification.destination_id == node_params.node_id){	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_CTS){	// CTS packet transmission finished

						LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s CTS #%d reception from N%d is finished successfully.\n",
								SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
								notification.packet_id, notification.source_id);

						node_state = STATE_TX_DATA;

						// Compute the NAV time
						bits_ofdm_sym =  GetNumberSubcarriers(current_right_channel - current_left_channel +1) *
							Mcs_array::modulation_bits[notification.modulation_id-1] *
							Mcs_array::coding_rates[notification.modulation_id-1] *
							IEEE_AX_SU_SPATIAL_STREAMS;

						ComputeFramesDuration(&rts_duration, &cts_duration, &data_duration, &ack_duration,
							num_channels_tx, notification.modulation_id, notification.tx_info.num_packets_aggregated,
							node_params.frame_length, bits_ofdm_sym);

						limited_num_packets_aggregated = notification.tx_info.num_packets_aggregated;

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Transmitting DATA (N_agg = %d) in %d channels using modulation %d (%.0f bits per OFDM symbol ---> %.2f Mbps) \n",
							SimTime(), node_params.node_id, node_state, LOG_F04, LOG_LVL4, limited_num_packets_aggregated,
							(current_right_channel - current_left_channel + 1), current_modulation, bits_ofdm_sym,
							bits_ofdm_sym/IEEE_AX_OFDM_SYMBOL_GI32_DURATION * pow(10,-6));

						// Compute the NAV time
						current_nav_time = ComputeNavTime(node_state, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
						current_nav_time = FixTimeOffset(current_nav_time,13,12); // Update the NAV time according to the time offsets

						// Generate and send DATA to transmitter after SIFS
						current_destination_id = notification.source_id;

						current_tx_duration = data_duration;	// This duration already computed in EndBackoff
						time_to_trigger = SimTime() + SIFS;

						// ------------------------------------------------------------------------
						// Sergio on 07 Dec 2017: add DATA transmission time to spectrum utilization
						for(int c = current_left_channel; c <= current_right_channel; ++c){
							node_stats.total_time_channel_busy_per_channel[c] = node_stats.total_time_channel_busy_per_channel[c] + current_tx_duration;
						}
						// ------------------------------------------------------------------------

						trigger_SIFS.Set(FixTimeOffset(time_to_trigger,13,12));

						LOGS(node_params.save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s SIFS will be triggered in %.12f\n",
							SimTime(), node_params.node_id, node_state, LOG_E14, LOG_LVL3,
							trigger_SIFS.GetTime());

						data_notification = GenerateNotification(PACKET_TYPE_DATA, current_destination_id,
								notification.packet_id, notification.tx_info.num_packets_aggregated,
								notification.timestamp_generated, current_tx_duration);

						// Reset the flag that indicates whether the tx power changed or not
                        sr_state.flag_change_in_tx_power = FALSE;

						data_notification.tx_info.preoccupancy_duration = time_rand_value;



					} else {	// Other packet type transmission finished
						LOGS(node_params.save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
								SimTime(), node_params.node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION

					LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Still receiving packet #%d reception from N%d.\n",
						SimTime(), node_params.node_id, node_state, LOG_E15, LOG_LVL3, incoming_notification.packet_id,
						incoming_notification.source_id);
				}

				break;
			}

			case STATE_SLEEP:{
				// do nothing
				break;
			}

			/* MAPC TX / wait states: do nothing when another node finishes TX */
			case STATE_TX_ICF:
			case STATE_TX_ICR:
			case STATE_TX_TF:
			case STATE_TX_ACK_TF:
			case STATE_WAIT_ICR:
			case STATE_WAIT_MU_RTS:
			case STATE_WAIT_TF:{
				HandleFinishTX_StateTxData(notification);
				break;
			}

			/* MAPC reception states: handle frame reception completion */
			case STATE_RX_ICF:{
				HandleFinishTX_StateRxIcf(notification);
				break;
			}
			case STATE_RX_ICR:{
				HandleFinishTX_StateRxIcr(notification);
				break;
			}
			case STATE_RX_MU_RTS:{
				HandleFinishTX_StateRxMuRts(notification);
				break;
			}
			case STATE_RX_TF:{
				HandleFinishTX_StateRxTf(notification);
				break;
			}
			case STATE_WAIT_ACK_TF:{
				HandleFinishTX_StateWaitAckTf(notification);
				break;
			}

			default:{
				printf("ERROR: %d is not a correct state\n", node_state);
				exit(EXIT_FAILURE);
				break;
			}
		}


	}

	// STATISTICS: compute the time the channel is idle (Node 0 is responsible to monitors this)
	if (node_params.node_id == 0) {
		int num_nodes_transmitting = 0;
		for(int i = 0; i < node_params.total_nodes_number; ++i){
			if(nodes_transmitting[i] == TRUE){
				++ num_nodes_transmitting;
			}
		}
		// Check if nobody is transmitting
		if (num_nodes_transmitting == 0) {
			// If no one is transmitting, set the current SimTime() as the last time the channel has been seen idle
			node_stats.last_time_channel_is_idle = SimTime();
			node_stats.channel_idle = true;
		}
	}

	// LOGS(node_params.save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s InportSomeNodeFinishTX() END",	SimTime(), node_params.node_id, node_state, LOG_E01, LOG_LVL1);
};

// Node::InportNackReceived — see methods/node_mcs_methods.h

// Node::InportMCSRequestReceived — see methods/node_mcs_methods.h

// Node::InportMCSResponseReceived — see methods/node_mcs_methods.h

// Node::InportNewPacketGenerated — see methods/node_mcs_methods.h

// Node::StartTransmission — see methods/node_packet_methods.h

// Node::InitiateBurstPackets — see methods/node/node_packet_methods.h

// Node::ScheduleTransmission — see methods/node/node_packet_methods.h

// Node::PrepareNewTransmission — see methods/node/node_packet_methods.h

// Node::EndBackoff — see methods/node/node_packet_methods.h

// Node::MyTxFinished — see methods/node/node_packet_methods.h

/**
 * Performs a negotiation of the MCS to be used according to the power sensed by the receiver
 */
// Node::RequestMCS, SelectDestination — see methods/node_packet_methods.h

/*********************/
/*********************/
/*  HANDLE PACKETS   */
/* see: node_packet_methods.h */
/*********************/
/*********************/
// Node::GenerateNotification — see methods/node_packet_methods.h

// Node::SendLogicalNack — see methods/node_packet_methods.h

// Node::SendResponsePacket — see methods/node_packet_methods.h

// Node::AbortInitialTransmission — see methods/node_backoff_methods.h

/*********************/
/*********************/
/*     TIMEOUTS      */
/* see: node_timeout_methods.h */
/*********************/
/*********************/
// Node::AckTimeout, CtsTimeout, DataTimeout, NavTimeout — see methods/node_timeout_methods.h

/************************/
/************************/
/*  BACKOFF MANAGEMENT  */
/* see: node_backoff_methods.h */
/************************/
/************************/
// Node::ScheduleBackoffAfterDIFS, UpdateSINRFromNotification,
// Node::PauseBackoff, ResumeBackoff — see methods/node_backoff_methods.h

/*********************/
/*  SPATIAL REUSE    */
/* see: node_spatial_reuse_methods.h */
/*********************/
// Node::SpatialReuseOpportunityEnds, InportRequestSpatialReuseConfiguration,
// Node::InportNewSpatialReuseConfiguration

/*********************/
/*  AGENTS MANAGMENT */
/* see: node_config_methods.h, node_statistics_methods.h */
/*********************/
// Node::GenerateConfiguration, UpdatePerformanceMeasurements,
// Node::InportReceivingRequestFromAgent, InportReceiveConfigurationFromAgent,
// Node::ApplyNewConfiguration, BroadcastNewConfigurationToStas,
// Node::InportNewWlanConfigurationReceived

/***********************/
/***********************/
/* AUXILIARY FUNCTIONS */
/* see: node_backoff_methods.h */
/***********************/
/***********************/
// Node::CallRestartSta, RestartNode, StartSavingLogs — see methods/node_backoff_methods.h

// Node::HandleSlottedBackoffCollision, RecoverFromCtsTimeout, CallSensing — see methods/node_backoff_methods.h
// Node::MeasureRho — see methods/node_statistics_methods.h

/************************/
/*  PRINT INFORMATION   */
/* see: node_statistics_methods.h */
/************************/
// Node::PrintNodeInfo, WriteNodeInfo, WriteNodeConfiguration, WriteReceivedConfiguration,
// Node::PrintNodeConfiguration, PrintProgressBar, PrintOrWriteNodeStatistics

// Node::SaveSimulationPerformance — see methods/node_statistics_methods.h

/*****************************/
/*****************************/
/*  VARIABLE INITIALIZATION  */
/*****************************/
/*****************************/

/*
 * Initialize all the necessary variables
 */
void Node :: InitializeVariables() {

	/*
	 * HARDCODED VARIABLES FOR TESTING PURPOSES
	 * - These variables are initialized in the code itself
	 * - Despite this is not the most efficient approach, it allows us testing new features
	 */
	burst_rate = DEFAULT_BURST_RATE;
	num_bursts = 0;
	// NACK system not activated - to be further extended in future
	node_params.nack_activated = FALSE;
	/*
	 * END OF HARDCODED INITIALIZATION
	 */

	node_params.current_max_bandwidth = node_params.max_channel_allowed - node_params.min_channel_allowed + 1;

	exchange_sequence = IEEE_802_11_COTDMA; // IEEE_802_11_NO_RTS_CTS;

	num_coordinated_aps     = 0;
	mapc_seq_pos            = 0;
	mapc_icr_received_count = 0;
	mapc_current_peer_idx   = 0;
	mapc_selected_peer_id   = NODE_ID_NONE;
	mapc_peer_has_data      = TRUE;          // conservative: assume peer has data until told otherwise
	mapc_txop_data_budget            = 0.0;
	mapc_txop_per_ap_data_duration   = 0.0;
	mapc_icr_requested_duration      = 0.0;
	coordinator_ap_id       = NODE_ID_NONE;
	mapc_peer_position      = 0;
	mapc_active_group_idx      = 0;
	mapc_coordinator_group_rr  = 0;

	node_stats.num_icf_tx = 0;
	node_stats.num_icr_rx = 0;
	node_stats.num_tf_tx  = 0;
	node_stats.num_mu_rts_tx = 0;
	node_stats.num_icf_rx = 0;
	node_stats.num_icr_tx = 0;
	node_stats.num_tf_rx  = 0;
	node_stats.num_mu_rts_rx = 0;
	node_stats.total_mapc_data_duration_own       = 0.0;
	node_stats.total_mapc_data_duration_shared    = 0.0;
	node_stats.total_mapc_data_duration_allocated = 0.0;

	current_sinr = 0;
	max_pw_interference = 0;
	node_stats.rts_lost_slotted_bo = 0;
	last_packet_generated_id = 0;

	node_stats.num_delay_measurements = 0;
	node_stats.sum_delays = 0;
	node_stats.average_delay = -1;
	node_stats.num_packets_generated = 0;
	node_stats.num_packets_dropped = 0;

	num_channels_tx = 0;

	node_stats.flag_measure_rho = TRUE;
	node_stats.delta_measure_rho = 0.00001;
	node_stats.num_measures_rho = 0;
	node_stats.num_measures_rho_accomplished = 0;
	node_stats.average_utilization = 0;
	node_stats.average_rho = 0;
	node_stats.bandwidth_used_txing = 0;

	node_stats.num_measures_utilization = 0;
	node_stats.num_measures_buffer_with_packets = 0;
	node_stats.generation_drop_ratio = 0;

	current_traffic_type = -1;

	// Output file - logger
	node_logger.save_logs = node_params.save_node_logs;
	node_logger.file = NULL;	// File opened in Start() after InitializeVariables() returns

	// Arrays and other
	channel_power = new double[NUM_CHANNELS_KOMONDOR];
	node_params.num_channels_allowed = (node_params.max_channel_allowed - node_params.min_channel_allowed + 1);
	node_stats.total_time_transmitting_per_channel = new double[NUM_CHANNELS_KOMONDOR];
	node_stats.last_total_time_transmitting_per_channel = new double[NUM_CHANNELS_KOMONDOR];
	channels_free = new int[NUM_CHANNELS_KOMONDOR];
	channels_for_tx = new int[NUM_CHANNELS_KOMONDOR];
	node_stats.total_time_lost_per_channel = new double[NUM_CHANNELS_KOMONDOR];
	node_stats.last_total_time_lost_per_channel = new double[NUM_CHANNELS_KOMONDOR];
	node_stats.total_time_channel_busy_per_channel = new double[NUM_CHANNELS_KOMONDOR];
	timestamp_channel_becomes_free = new double[NUM_CHANNELS_KOMONDOR];
	node_stats.num_trials_tx_per_num_channels = new int[NUM_CHANNELS_KOMONDOR];
	for(int i = 0; i < NUM_CHANNELS_KOMONDOR; ++i){
		channel_power[i] = 0;
		node_stats.total_time_transmitting_per_channel[i] = 0;
		node_stats.last_total_time_transmitting_per_channel[i] = 0;
		channels_free[i] = FALSE;
		channels_for_tx[i] = FALSE;
		node_stats.total_time_lost_per_channel[i] = 0;
		node_stats.last_total_time_lost_per_channel[i] = 0;
		timestamp_channel_becomes_free[i] = 0;
		node_stats.num_trials_tx_per_num_channels[i] = 0;
		node_stats.total_time_channel_busy_per_channel[i] = 0;
	}

	node_stats.total_time_transmitting_in_num_channels = new double[NUM_CHANNELS_KOMONDOR];
	node_stats.total_time_lost_in_num_channels = new double[NUM_CHANNELS_KOMONDOR];
	for(int i = 0; i < NUM_CHANNELS_KOMONDOR; ++i){
		node_stats.total_time_transmitting_in_num_channels[i] = 0;
		node_stats.total_time_lost_in_num_channels[i] = 0;
	}

	nodes_transmitting = new int[node_params.total_nodes_number];
//	// List of hidden nodes (1 indicates hidden nodes, 0 indicates the opposite)
//	hidden_nodes_list = new int[node_params.total_nodes_number];
//	// Counter for the times a node was implied in a collision by hidden node
//	potential_hidden_nodes = new int[node_params.total_nodes_number];
	for(int n = 0; n < node_params.total_nodes_number; ++n){
		nodes_transmitting[n] = FALSE;
//		hidden_nodes_list[n] = FALSE;
//		potential_hidden_nodes[n] = 0;
	}

	power_received_per_node.clear();

//	potential_hidden_nodes[node_params.node_id] = -1; // To indicate that the node cannot be hidden from itself
	node_stats.nacks_received = new int[NUM_PACKET_LOST_REASONS];
	for(int i = 0; i < NUM_PACKET_LOST_REASONS; ++i){
		node_stats.nacks_received[i] = 0;
	}

	// Rest of variables
	power_rx_interest = 0;
	progress_bar_counter = 0;
	current_left_channel =  node_params.min_channel_allowed;
	current_right_channel = node_params.max_channel_allowed;
	node_stats.sum_waiting_time = 0;
	node_stats.timestamp_new_trial_started = 0;
	node_stats.num_average_waiting_time_measurements = 0;
	node_stats.expected_backoff = 0;
	ca_state.remaining_backoff = 0;
	node_stats.num_new_backoff_computations = 0;
	last_transmission_successful = 0;

	node_stats.data_packets_acked = 0;
	node_stats.data_frames_acked = 0;

	node_state = STATE_SENSING;
	current_modulation = 1;
	packet_id = 0;

	//CHANNEL ACCESSs
	ca_state.current_cw_min = node_params.cw_min_default; // Initialize the CW min
	ca_state.current_cw_max = node_params.cw_max_default; // Initialize the CW max
	ca_state.cw_stage_current = 0;
	//- Tokenized backoffF
	if (node_params.backoff_type == BACKOFF_TOKENIZED){
		token_status = node_params.node_id;
		distance_to_token = 0;
		token_order_list[node_params.node_id] = DEVICE_ACTIVE_FOR_TOKEN;
	}
	//-  Deterministic backoff
	ca_state.num_bo_interruptions = 0;
	ca_state.base_backoff_deterministic = DEFAULT_BASE_BACKOFF_DETERMINISTIC;
	ca_state.deterministic_bo_active = 0;

	if(node_params.node_type == NODE_TYPE_AP) {
		node_is_transmitter = TRUE;
		ca_state.remaining_backoff = ComputeBackoff(ca_state.current_cw_min, ca_state.current_cw_max, node_params.backoff_type,
				current_traffic_type, ca_state.deterministic_bo_active, ca_state.num_bo_interruptions, ca_state.base_backoff_deterministic, -1);
		ca_state.previous_backoff = ca_state.remaining_backoff;
		node_stats.expected_backoff += ca_state.remaining_backoff;
		node_stats.num_new_backoff_computations++;
		// - compute average waiting time to access the channel
		node_stats.timestamp_new_trial_started = SimTime();
	} else {
		node_is_transmitter = FALSE;
	}

	current_tx_power = node_params.tx_power_default;
	current_pd = node_params.sensitivity_default;
	channel_max_intereference = node_params.current_primary_channel;

	data_duration = 0;
	ack_duration = 0;
	rts_duration = 0;
	cts_duration = 0;

	default_modulation = MODULATION_NONE;

	mcs_response = new int[NUM_OPTIONS_CHANNEL_LENGTH];
	for(int n = 0; n < NUM_OPTIONS_CHANNEL_LENGTH; ++n){
		mcs_response[n] = 0;
	}

	change_modulation_flag = new int[wlan.num_stas];
	for(int n = 0; n < wlan.num_stas; ++n){
		change_modulation_flag[n] = TRUE;
	}
	mcs_per_node = new int *[wlan.num_stas] ;
	for( int i = 0 ; i < wlan.num_stas ; ++i ) mcs_per_node[i] = new int[NUM_OPTIONS_CHANNEL_LENGTH];
	for ( int i=0; i< wlan.num_stas; ++i) {
		for (int j=0; j< NUM_OPTIONS_CHANNEL_LENGTH; ++j) {
			mcs_per_node[i][j] = -1;
		}
	}

	first_time_requesting_mcs = TRUE;

	/* NULL notification for Valgrind issues */
	null_notification.packet_id = -1;
	null_notification.packet_type = -1;
	null_notification.source_id = -1;
	null_notification.destination_id = -1;
	null_notification.tx_duration = -1;
	null_notification.left_channel = -1;
	null_notification.right_channel = -1;
	null_notification.frame_length = -1;
	null_notification.modulation_id = -1;
	null_notification.timestamp = -1;

	TxInfo null_tx_info;

	null_tx_info.data_duration = 0;
	null_tx_info.ack_duration = 0;
	null_tx_info.rts_duration = 0;
	null_tx_info.cts_duration = 0;
	null_tx_info.tx_power = 0;
	null_tx_info.bits_ofdm_sym = 0;
	null_tx_info.SetSizeOfMCS(4);	// TODO: make size dynamic
	null_tx_info.x = 0;
	null_tx_info.y = 0;
	null_tx_info.z = 0;
	null_tx_info.nav_time = 0;

	null_notification.tx_info = null_tx_info;

	data_notification = null_notification;
	ack_notification= null_notification;
	incoming_notification = null_notification;
	rts_notification = null_notification;
	cts_notification = null_notification;
	new_packet = null_notification;

	// Statistics
	node_stats.data_packets_sent = 0;
	node_stats.rts_cts_sent = 0;
	node_stats.throughput = 0;
	node_stats.throughput_loss = 0;
	node_stats.data_packets_lost = 0;
	node_stats.rts_cts_lost = 0;
	node_stats.num_tx_init_not_possible = 0;
	node_stats.num_tx_init_tried = 0;
	node_stats.last_throughput = 0;
	node_stats.last_average_delay = -1;
	node_stats.last_data_frames_acked = 0;
	node_stats.last_sum_delays = 0;
	node_stats.last_num_delay_measurements = 0;

	node_stats.last_measurements_window = DEFAULT_LAST_MEASUREMENTS_WINDOW;	// Time in seconds of the last performance observation window

	node_stats.throughput_per_sta = new double[wlan.num_stas];
	node_stats.data_packets_sent_per_sta = new int[wlan.num_stas];
	node_stats.rts_cts_sent_per_sta = new int[wlan.num_stas];
	node_stats.data_packets_lost_per_sta = new int[wlan.num_stas];
	node_stats.rts_cts_lost_per_sta = new int[wlan.num_stas];
	node_stats.data_packets_acked_per_sta = new int[wlan.num_stas];
	node_stats.data_frames_acked_per_sta = new int[wlan.num_stas];

	for(int i = 0; i < wlan.num_stas; ++i){
		node_stats.throughput_per_sta[i] = 0;
		node_stats.data_packets_sent_per_sta[i] = 0;
		node_stats.rts_cts_sent_per_sta[i] = 0;
		node_stats.data_packets_lost_per_sta[i] = 0;
		node_stats.rts_cts_lost_per_sta[i] = 0;
		node_stats.data_packets_acked_per_sta[i] = 0;
		node_stats.data_frames_acked_per_sta[i] = 0;
	}

	performance_report.SetSizeOfChannelLists(NUM_CHANNELS_KOMONDOR);
	performance_report.SetSizeOfRssiList(node_params.total_wlans_number);

	performance_report.SetSizeOfRssiPerStaList(wlan.num_stas);

	// Measurements to be sent to agents
	RestartPerformanceMetrics(&performance_report, 0, node_params.num_channels_allowed);
	performance_report.SetSizeOfRssiList(node_params.total_wlans_number);

	flag_apply_new_configuration = FALSE;

	// Channel idle measurement
	node_stats.sum_time_channel_idle = 0;
	node_stats.last_time_channel_is_idle = 0;

	/******************************
	 *  SPATIAL REUSE
	 *****************************/
	// Detect whether the SR is enabled or not
	if (node_params.node_type == NODE_TYPE_AP && node_params.bss_color >= 0) {
		sr_state.spatial_reuse_enabled = TRUE;
	} else {
		sr_state.spatial_reuse_enabled = FALSE;
	}
	// In case of being an STA, request the SR configuration to the AP
	if (node_params.node_type == NODE_TYPE_STA) {
		outportRequestSpatialReuseConfiguration();
	}
	// Initialize the TXOP identified to false
	sr_state.txop_sr_identified = FALSE;
	// When leaving Co-SR mode, force MCS re-measurement at standalone TX power
	// for the next TXOP. Without this, nodes reuse the interference-measured MCS
	// from the Co-SR simultaneous phase for their following standalone transmissions.
	if (sr_state.mapc_cosr_active) {
		for (int n = 0; n < wlan.num_stas; ++n) change_modulation_flag[n] = TRUE;
	}
	sr_state.mapc_cosr_active = 0;

	// Initialize beamforming per-TXOP state
	current_beam_az_main_rad = 0.0;
	current_beam_num_nulls = 0;
	for (int _bk = 0; _bk < MAX_BEAM_NULLS; ++_bk)
		current_beam_null_az_rad[_bk] = 0.0;

	// Initialize the type of ongoing transmissions to 0
	for (int i = 0; i < 3; i ++) {
		sr_state.type_ongoing_transmissions_sr[i] = 0;
	}
	sr_state.next_pd_spatial_reuse = current_pd;
	/*****************************/

	if(node_params.node_type == NODE_TYPE_AP) {
		rssi_per_sta = new double[wlan.num_stas];
		for (int i = 0; i < wlan.num_stas; ++i) {
			rssi_per_sta[i] = 0;
		}
	}

	/**************************
	// TODO: change this in DEV to enter type of CCA per BW through config file
	***************************/
	//channel_aggregation_cca_model = CHANNEL_AGGREGATION_CCA_SAME;
	channel_aggregation_cca_model = CHANNEL_AGGREGATION_CCA_11AX;
}

/*****************************/
/*  INCLUDES                 */
/*****************************/
// Node member function implementations -> all pulled in via the aggregator (see Code/methods/node_impl.h for the full list)
#include "../methods/node/node_impl.h"
