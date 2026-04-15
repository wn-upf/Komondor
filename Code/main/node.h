/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
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
#include "../methods/channel/channel_access_methods.h"

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
		int channel_max_interference;		///> Channel of interest suffering maximum interference

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
		int    *all_node_first_sta_id;	///> For AP nodes: first associated STA id; NODE_ID_NONE for STAs

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
		ChannelAccessPolicy channel_access_policy;  ///> Strategy for CCA + channel selection at backoff expiry

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
		int *nodes_transmitting;					///> IDs of the nodes which are transmitting to any destination
		std::map<int, double>
			power_received_per_node;				///> Map containing the power received from each other node
		double power_rx_interest;					///> Power received from a TX destined to the node [pW]
		int receiving_from_node_id;					///> ID of the node that is transmitting to the node (-1 if node is not receiving)
		int receiving_packet_id;					///> ID of the notification that is being transmitted to me
		double BER;									///> Bit error rate (deprecated)
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

		// Preamble puncturing (802.11ax)
		int pp_punctured_bitmap;		///> Bitmask of punctured channels for the current TXOP (set by EndBackoff, used in GenerateNotification)

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


// Node::HandleStartTX_StateSensing  — see methods/node/node_fsm_methods.h
// Node::HandleStartTX_StateNav      — see methods/node/node_fsm_methods.h
// Node::HandleStartTX_StateTxData   — see methods/node/node_fsm_methods.h
// Node::HandleStartTX_StateRxData   — see methods/node/node_fsm_methods.h
// Node::HandleStartTX_StateWaitAck  — see methods/node/node_fsm_methods.h
// Node::HandleStartTX_StateWaitCts  — see methods/node/node_fsm_methods.h
// Node::HandleStartTX_StateWaitData — see methods/node/node_fsm_methods.h
// Node::InportSomeNodeStartTX       — see methods/node/node_fsm_methods.h
// Node::HandleFinishTX_StateSensing — see methods/node/node_fsm_methods.h
// Node::HandleFinishTX_StateRxData  — see methods/node/node_fsm_methods.h
// Node::HandleFinishTX_StateTxData  — see methods/node/node_fsm_methods.h
// Node::InportSomeNodeFinishTX      — see methods/node/node_fsm_methods.h


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

// --- HANDLE PACKETS (see: node_packet_methods.h) ---
// Node::GenerateNotification — see methods/node_packet_methods.h

// Node::SendLogicalNack — see methods/node_packet_methods.h

// Node::SendResponsePacket — see methods/node_packet_methods.h

// Node::AbortInitialTransmission — see methods/node_backoff_methods.h

// --- TIMEOUTS (see: node_timeout_methods.h) ---
// Node::AckTimeout, CtsTimeout, DataTimeout, NavTimeout — see methods/node_timeout_methods.h

// --- BACKOFF MANAGEMENT (see: node_backoff_methods.h) ---
// Node::ScheduleBackoffAfterDIFS, UpdateSINRFromNotification,
// Node::PauseBackoff, ResumeBackoff — see methods/node_backoff_methods.h

// --- SPATIAL REUSE (see: node_spatial_reuse_methods.h) ---
// Node::SpatialReuseOpportunityEnds, InportRequestSpatialReuseConfiguration,
// Node::InportNewSpatialReuseConfiguration

// --- AGENTS MANAGEMENT (see: node_config_methods.h, node_statistics_methods.h) ---
// Node::GenerateConfiguration, UpdatePerformanceMeasurements,
// Node::InportReceivingRequestFromAgent, InportReceiveConfigurationFromAgent,
// Node::ApplyNewConfiguration, BroadcastNewConfigurationToStas,
// Node::InportNewWlanConfigurationReceived

// --- AUXILIARY FUNCTIONS (see: node_backoff_methods.h) ---
// Node::CallRestartSta, RestartNode, StartSavingLogs — see methods/node_backoff_methods.h

// Node::HandleSlottedBackoffCollision, RecoverFromCtsTimeout, CallSensing — see methods/node_backoff_methods.h
// Node::MeasureRho — see methods/node_statistics_methods.h

// --- PRINT INFORMATION (see: node_statistics_methods.h) ---
// Node::PrintNodeInfo, WriteNodeInfo, WriteNodeConfiguration, WriteReceivedConfiguration,
// Node::PrintNodeConfiguration, PrintProgressBar, PrintOrWriteNodeStatistics

// Node::SaveSimulationPerformance — see methods/node_statistics_methods.h

// --- VARIABLE INITIALIZATION ---

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

	exchange_sequence = IEEE_802_11_NO_RTS_CTS;

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

	// Preamble puncturing (802.11ax)
	pp_punctured_bitmap = 0;

	// Channel access policy: default to CSMA/CA; switch to preamble-puncturing if CB_PP_MAX_LOG2
	channel_access_policy.checkAndSelectChannels = CSMA_CA_CheckAndSelectChannels;
	if (node_params.current_dcb_policy == CB_PP_MAX_LOG2)
		channel_access_policy.checkAndSelectChannels = PP_CheckAndSelectChannels;

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
	channel_max_interference = node_params.current_primary_channel;

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
