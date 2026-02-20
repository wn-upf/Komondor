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
		void AbortRtsTransmission();
		void PrepareNewTransmission();
		void InitiateBurstPackets();
		void ScheduleTransmission();

//		// NACK
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

		// InportSomeNodeStartTX per-state handlers
		void HandleStartTX_StateSensing(const Notification &notification);
		void HandleStartTX_StateTxData(const Notification &notification);
		void HandleStartTX_StateRxData(const Notification &notification);
		void HandleStartTX_StateWaitAck(const Notification &notification);
		void HandleStartTX_StateWaitCts(const Notification &notification);
		void HandleStartTX_StateWaitData(const Notification &notification);
		void HandleStartTX_StateNav(const Notification &notification);

		// InportSomeNodeFinishTX per-state handlers
		void HandleFinishTX_StateSensing(const Notification &notification);
		void HandleFinishTX_StateRxData(const Notification &notification);
		void HandleFinishTX_StateTxData(const Notification &notification);

	// Public items (entered by nodes constructor in komondor_main)
	public:

		// Specific to a node
		int node_id; 					///> Node identifier
		double x;						///> X position coordinate
		double y;						///> Y position coordinate
		double z;						///> Z position coordinate
		std::string node_code;			///> Name of the Node (only for information displaying purposes)
		int node_type;					///> Node type (e.g., AP, STA, ...)
		int current_primary_channel;	///> Primary channel
		int min_channel_allowed;		///> Min. allowed channel
		int max_channel_allowed;		///> Max. allowed channel
		int num_channels_allowed;		///> Maximum number of channels allowed to TX in
		double tx_power_default;		///> Default power transmission [pW]
		double sensitivity_default;		///> Default pd	("sensitivity" threshold) [pW]
		int current_dcb_policy;			///> Channel bonding model (definition of models in function GetTxChannelsByChannelBonding())
										///> - If set, this parameter determines the min and max channel allowed
		int current_max_bandwidth;		///> Maximum bandwidth allowed [no. of 20-MHz channels]
		FIFO buffer;					///> FIFO buffer (contains Notifications)
		int last_packet_generated_id;	///> ID of the last packet generated by the source

		// Spatial Reuse operation
		int bss_color;			///> BSS color
		int srg;				///> Spatial Reuse Group (SRG)
		double non_srg_obss_pd; ///> non-SRG OBSS_PD threshold
		double srg_obss_pd; 	///> SRG OBSS_PD threshold

		// WLAN
		std::string wlan_code;		///> Code of the WLAN to which the node belongs
		Wlan wlan;					///> Wlan to which the node belongs

		/* Same default in every node (parameters from system input and console arguments) */
		// Generic
		double simulation_time_komondor;	///> Observation time (time when the simulation stops)
		int total_wlans_number;				///> Total number of wlans
		int total_nodes_number;				///> Total number of nodes
		int collisions_model;				///> Collisions model (TODO: implement different collision models)
		double capture_effect;				///> Capture effect threshold [linear ratio]
		double constant_per;				///> Constant PER for correct transmissions
		int save_node_logs;					///> Flag for activating the log writting of nodes
		int print_node_logs;				///> Flag for activating the printing of node logs
		std::string simulation_code;		///> Simulation code
		int capture_effect_model;			///> Capture Effect model
		int nack_activated;					///> Flag for activating the utilization of NACKs

		// Channel
		int adjacent_channel_model;			///> Adjacent channel interference model (definition of models in function UpdateChannelsPower())
		int pifs_activated;					///> PIFS mechanism activation

		// Transmissions
		int current_modulation;				///> Current_modulation used by nodes
		int channel_max_intereference;		///> Channel of interest suffering maximum interference
		double central_frequency;			///> Central frequency (Hz)
		int path_loss_model;				///> Path loss model (0: free-space, 1: Okumura-Hata model - Uban areas)

		// Data rate - modulations
		int modulation_rates[4][12];		///> Modulation rates in bps used in IEEE 802.11ax
		int err_prob_modulation[4][12];		///> BER associated to each modulation (TO BE FILLED!!)
		int first_time_requesting_mcs;		///> Flag to indicate if the MCS has been firstly requested or not

		// Packets
		int pdf_tx_time;					///> Probability distribution type of the transmission time (0: exponential, 1: deterministic)
		int frame_length;					///> Notification length [bits]
		int max_num_packets_aggregated;		///> Number of packets aggregated in one transmission
		int traffic_model;					///> Traffic model (0: full buffer, 1: poisson, 2: deterministic)

		// Channel access parameters
		int backoff_type;					///> Type of Backoff (0: Slotted 1: Continuous)
		int cw_adaptation;					///> CW adaptation (0: constant, 1: bineary exponential backoff)
		int cw_min_default;					///> Minimum Contention Window set by default
		int cw_max_default;					///> Maximum Contention Window set by default
		int cw_stage_max;					///> Backoff maximum Contention Window

		// Token-based channel access
		int *token_order_list;			///> Ordered list of the devices involved in the tokenized channel access
		int token_status;				///> Status of the token (i.e., ID of the device holding the token)
		int distance_to_token;			///> Distance of the node to the token in "token_status_list"
		int *num_missed_tokens_list;	///> Number of missed tokens for each involved device in the tokenized channel access

		double *distances_array;					///> Distance with respect to other nodes
		double *received_power_array;				///> Power received from the other nodes
		double *max_received_power_in_ap_per_wlan;	///> Maximum power received from each WLAN

		double *rssi_per_sta;	///> RSSI per STA in the WLAN

	// Statistics (accessible when simulation finished through Komondor simulation class)
	public:

		int data_packets_sent;								///> Number of data packets sent
		int rts_cts_sent;									///> Number of RTS/CTS packets sent
		double num_packets_generated;						///> Number of packets generated by the AP
		double num_packets_dropped;							///> Number of packets dropped by the AP's buffer
		double *total_time_transmitting_per_channel;		///> Time transmitting per channel;
		double *total_time_transmitting_in_num_channels;	///> Time transmitting in (ix 0: 1 channel, ix 1: 2 channels...)
		double *total_time_lost_per_channel;				///> Time transmitting per channel unsuccessfully;
		double *total_time_lost_in_num_channels;			///> Time transmitting in (ix 0: 1 channel, ix 1: 2 channels...) unsuccessfully
		double *total_time_channel_busy_per_channel;		///> Time that the spectrum is occupied per channel
															///> - Any time a transmissions is done (regardless the packet) the duration of such transmissions is added
		double throughput;									///> Throughput [Mbps]
		double throughput_loss;								///> Throughput of lost packets [Mbps]
		int data_packets_acked;								///> Own data packets (aggregated or not) that have been Acked
		int data_frames_acked;								///> Own data frames that have been Acked
		int data_packets_lost;								///> Own packets that have been collided or lost
		int *num_trials_tx_per_num_channels;				///> Number of txs trials per number of channels
		int rts_cts_lost;
		int *nacks_received;								///> Counter of the type of Nacks received
		int num_tx_init_tried;								///> Number of TX initiations tried (whenever transmitter try to acces the channel)
		int num_tx_init_not_possible;						///> Number of TX initiations that have been not possible due to channel state and DCB model
		int rts_lost_slotted_bo;							///> Number of RTS packets lost due to slotted BO
		double prob_slotted_bo_collision;					///> Probability of slotted BO collision
		double average_waiting_time;						///> Average time between two channel accesses
		double bandwidth_used_txing;						///> Average bandiwdth used for transmitting (RTS, CTS, ACK and DATA packets considered).
															///> - Part of the available bandwidth used in average
															///> - e.g., 135 MHz used in average from the 160 MHz (8 channels) available
		int num_delay_measurements;			///> Number of delay measurements for averaging
		double sum_delays;					///> Sum of delays for averaging
		double average_delay;				///> Average delay from packet generation to ACK
		double average_rho;					///> Average rho metric (prob. of having packets in buffer and channel free)
		double average_utilization;			///> Average buffer utilization
		double generation_drop_ratio;		///> Probability of dropping a packet

		double expected_backoff;		 	///> Average computed BO value
		int num_new_backoff_computations;   ///> Number of backoff computed (generated)

		double sum_time_channel_idle;		///> Variable to measure the time the channel is idle (no one transmits)
		double last_time_channel_is_idle;	///> Auxiliary variable to measure the time the channel is idle
		bool channel_idle;					///> Variable to determine whether the channel is idle or not

		double last_time_not_in_nav;	///> Timestamp referring to the last time the node was in NAV state
		double time_in_nav;				///> Variable to store the time spent in NAV state
		int times_went_to_nav;			///> Variable to store the number of times the node passes to NAV state

		// Statistics in the last part of the simulation
		double last_measurements_window;
		double last_throughput;
		int last_data_frames_acked;
		double last_average_delay;
		double last_sum_delays;
		int last_num_delay_measurements;
		double *last_total_time_transmitting_per_channel;
		double *last_total_time_lost_per_channel;
		double last_sum_waiting_time;
		int last_num_average_waiting_time_measurements;

		// Statistics of each STA
		double *throughput_per_sta;			///> Stores the throughput assigned to each STA (Downlink mode)
		int *data_packets_sent_per_sta;		///> Stores the data packets sent to each STA (Downlink mode)
		int *rts_cts_sent_per_sta;			///> Stores the RTS/CTS packets sent to each STA (Downlink mode)
		int *data_packets_lost_per_sta;		///> Stores the data packets lost for each STA (Downlink mode)
		int *rts_cts_lost_per_sta;			///> Stores the RTS/CTS packets lost for each STA (Downlink mode)
		int *data_packets_acked_per_sta;	///> Stores the data packets acknowledged by each STA (Downlink mode)
		int *data_frames_acked_per_sta;		///> Stores the frames acknowledged by each STA (Downlink mode)

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
		double remaining_backoff;			///> Remaining backoff
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

		// Contention Window (CW) management
		int cw_stage_current;	///> Current Contention Window stage
		int current_cw_min;		///> Minimum Contention Window being used currently
		int current_cw_max;		///> Maximum Contention Window being used currently

		// Deterministic backoff
		int num_bo_interruptions;			///> Number of observed BO interruptions
		int base_backoff_deterministic;		///> Base backoff for the deterministic backoff
		int deterministic_bo_active;		///> Flag to indicate whether the deterministic phase is active or not

		// ECA
		double previous_backoff;				///> Last backoff used

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
		double *timestampt_channel_becomes_free;	///> Timestamp when channel becomes free (when P(channel) < PD threshold)
		double time_to_trigger;						///> Auxiliar time to trigger an specific trigger (used for almost every .Set() function)
		int num_channels_tx;						///> Number of channels used for transmission
		int flag_apply_new_configuration;			///> Flag to determine if there is any new configuration to be applied when doing "RestartNode()"
		int channel_aggregation_cca_model;			///> Flag to determine the type of CCA per bandwidth applied
		int last_transmission_successful;			///> Flag to indicate whether the last transmission was successful or not

		// Rho measurement
		int flag_measure_rho;					///> Flag for activating rho measurement
		double delta_measure_rho;				///> Time [s] between two rho measurements
		int num_measures_rho;					///> Number of measures to get the average rho metric
		int num_measures_rho_accomplished;		///> Number of measures that rho condition (packet in buffer and channel free) is given
		int num_measures_utilization;			///> Number of measures for computing the utilization metric
		int num_measures_buffer_with_packets;	///> Number of measures where the buffer had packets

		// Burst traffic
		double burst_rate;				///> Average time between two packet generation bursts [bursts/s]
		int num_bursts;					///> Total number of bursts occurred in the simulation

		// Time waiting in BO
		double sum_waiting_time;					///> Sum of time waiting before transmitting
		double timestamp_new_trial_started;			///> Variable to store the timestamp at which a new transmission trial starts
		int num_average_waiting_time_measurements;	///> Number of times a waiting time measurement is done (for averaging purposes)
		double time_rand_value;						///> Rand value to avoid synch. issues

		// Spatial Reuse
		int spatial_reuse_enabled;		///> Indicates whether SR is enabled or not
		int type_last_sensed_packet;	///> Stores the type of the last sensed packet
		double pd_spatial_reuse;		///> Stores the PD in pW to be used during the SR operation
		double tx_power_sr; 			///> Stores the limited tx power in pW to be used during the SR operation
		int txop_sr_identified;			///> Indicates whether a TXOP has been identified under the SR operation

		int type_ongoing_transmissions_sr[3]; 	///> Array indicating which types of transmissions are currently being held (3 PD types are considered)
		double next_pd_spatial_reuse; 			///> PD threshold to be used when restarting after detecting a TXOP
		bool flag_change_in_tx_power;			///> Flag to indicate that the tx power has been modified
		double potential_obss_pd_threshold;		///> Potential OBSS/PD threshold to be applied in a SR-based TXOP
		double current_obss_pd_threshold;		///> Current OBSS/PD threshold used by the node
		double next_tx_power_limit;				///> Transmission power limitation to be applied during the next transmission
		double current_tx_power_sr;				///> Current transmission power used within the SR operation

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

	// if(print_node_logs) printf("%s(N%d) Start\n", node_code, node_id);

	// Create node logs file if required
	if(save_node_logs) {
		// Name node log file accordingly to the node_id
		// Sergio on 16 Jan: changed path to adapt to new directory hierarchy
		snprintf(own_file_path, sizeof(own_file_path), "%s_%s_N%d_%s.txt","../output/logs_output", simulation_code.c_str(), node_id, node_code.c_str());
		remove(own_file_path);
		output_log_file = fopen(own_file_path, "at");
		node_logger.save_logs = save_node_logs;
		node_logger.file = output_log_file;
		node_logger.SetVoidHeadString();
	}

	LOGS(save_node_logs, node_logger.file,"%.18f;N%d;S%d;%s;%s Start()\n",
		SimTime(), node_id, STATE_UNKNOWN, LOG_B00, LOG_LVL1);

	// Write node info and conf.
	std::string header_str;
	header_str.append(ToString(SimTime()));
	if(save_node_logs) WriteNodeInfo(node_logger, INFO_DETAIL_LEVEL_2, header_str);

	// Start backoff procedure only if node is able to transmit
	if(node_is_transmitter) {

		if (TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION) {

			//for(int i = 0; i < max_num_packets_aggregated; ++i){
			//	new_packet = null_notification;
			//	new_packet.timestamp_generated = SimTime();
			//	new_packet.packet_id = last_packet_generated_id;
			//	buffer.PutPacket(new_packet);
			//	++last_packet_generated_id;
			//}

			ScheduleBackoffAfterDIFS();

		}

		// if(flag_measure_rho) trigger_rho_measurement.Set(SimTime() + delta_measure_rho);
		if(flag_measure_rho) trigger_rho_measurement.Set(SimTime() + 980);

	} else {
		current_destination_id = wlan.ap_id;	// TODO: for uplink traffic. Set STAs destination to the GW
	}

	// Progress bar (trick: it is only printed by node with id 0)
	if(PROGRESS_BAR_DISPLAY){
		if(node_id == 0){
			if(print_node_logs) printf("%s PROGRESS BAR:\n", LOG_LVL1);
			trigger_sim_time.Set(SimTime() + PICO_VALUE);
		}
	}

	// ----------------------------------------
	// - For starting saving the nodes from a given timestamp on
	//    save_node_logs = FALSE;
	//    trigger_start_saving_logs.Set(SimTime() + 3628);
	// ----------------------------------------

	LOGS(save_node_logs, node_logger.file,"\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	// LOGS(save_node_logs, node_logger.file, "%f;N%d;S%d;%s;%s Start() END\n", SimTime(), node_id, node_state, LOG_B01, LOG_LVL1);
};

/**
 * Stop()
 */
void Node :: Stop(){

	LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s Node Stop()\n",
		SimTime(), node_id, node_state, LOG_C00, LOG_LVL1);

	// Print and write node statistics if required
	if (print_node_logs) PrintOrWriteNodeStatistics(PRINT_LOG);
	if (save_node_logs) PrintOrWriteNodeStatistics(WRITE_LOG);

	// Close node logs file
	if(save_node_logs) fclose(node_logger.file);

	// Save performance into the simulation_performance object
	SaveSimulationPerformance();

	// Save the configuration currently being used by the node
	GenerateConfiguration();

	// LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s Node info:\n", SimTime(), node_id, node_state, LOG_C01, LOG_LVL1);
};

/**
 * Called when some node (this one included) starts a transmission
 * @param "notification" [type Notification]: notification containing the information of the transmission start perceived
 */
/**
 * Handle InportSomeNodeStartTX for STATE_SENSING
 */
void Node :: HandleStartTX_StateSensing(const Notification &notification) {

				if(notification.destination_id == node_id){	// Node IS THE DESTINATION

					// Update power received of interest
					power_rx_interest = power_received_per_node[notification.source_id];

					current_left_channel = notification.left_channel;
					current_right_channel = notification.right_channel;
					current_modulation = notification.modulation_id;

					LOGS(save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
						SimTime(), node_id, node_state, LOG_D07, LOG_LVL3,
						notification.destination_id);

					// Issue #146 RTS/CTS BW indication
					// - If incoming packet is RTS or CTS sent to me, focus first just on the primary channel
					// - If RTS/CTS decodable in the primary, packet not lost.
					// - After decoding the RTS/CTS at the primary, perform CCA assessment in all the range
					// - So, if RTS/CTS: max_pw_interference is now referred just to primary channel interference
					// - Keep max_pw_interference for all range if DATA or ACK.

					if(notification.packet_type == PACKET_TYPE_RTS){

						// max_pw_interference is interference in primary
						max_pw_interference = channel_power[current_primary_channel]
							- power_received_per_node[notification.source_id];

					} else {

						// Compute max interference (the highest one perceived in the reception channel range)
						ComputeMaxInterference(&max_pw_interference, &channel_max_intereference,
							notification, node_state, power_received_per_node, &channel_power);
					}

					LOGS(save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %.2f dBm - P_if = %.2f dBm - P_noise = %.2f dBm\n",
						SimTime(), node_id, node_state, LOG_D08, LOG_LVL5,
						channel_max_intereference,
						ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
						ConvertPower(PW_TO_DBM, power_rx_interest),
						ConvertPower(PW_TO_DBM, max_pw_interference),
						ConvertPower(PW_TO_DBM, NOISE_LEVEL_DBM));

					if(notification.packet_type == PACKET_TYPE_RTS) {	// Notification CONTAINS an RTS PACKET

						current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

						LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s SINR = %.2f dBm\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL5,
							ConvertPower(LINEAR_TO_DB, current_sinr));

						// Check if notification has been lost due to interferences or weak signal strength
						loss_reason = IsPacketLost(current_primary_channel, notification, notification,
								current_sinr, capture_effect, current_pd,
								power_rx_interest, constant_per, node_id, capture_effect_model);

						if(loss_reason != PACKET_NOT_LOST) {	// If RTS IS LOST, send logical Nack

							// Check if lost due to BO collision
							if(loss_reason == PACKET_LOST_INTERFERENCE){
								if(fabs(outrange_nav_notification.timestamp - notification.timestamp)
									< MAX_DIFFERENCE_SAME_TIME){
									loss_reason = PACKET_LOST_BO_COLLISION;
								}
							}

							LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
								SimTime(), node_id, node_state, LOG_D15, LOG_LVL4, notification.packet_id,
								notification.source_id, loss_reason);

							if(nack_activated) {
								// Send logical NACK to transmitter sending incoming notification for indicating the loss reason
								logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
									node_id, notification.source_id, NODE_ID_NONE, loss_reason, BER, current_sinr);
								SendLogicalNack(logical_nack);
							}

							if(node_is_transmitter){

								int pause (HandleBackoff(PAUSE_TIMER, &channel_power,
									current_primary_channel, current_pd, buffer.QueueSize()));
								// Check if node has to freeze the BO (if it is not already frozen)
								if (pause) {
									PauseBackoff();
								}

							}

						} else {	// Data packet IS NOT LOST (it can be properly received)

							LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Reception of RTS #%d from N%d CAN be started (SINR = %f dB)\n",
								SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id,
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

					} else {	//	Notification does NOT CONTAIN an RTS
						LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Unexpected packet type (%d) received!\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL4, notification.packet_type);
					}

				} else {	// Node IS NOT THE DESTINATION

					// See if node has to change to NAV
					if(notification.packet_type == PACKET_TYPE_RTS
						|| notification.packet_type == PACKET_TYPE_CTS
						|| notification.packet_type == PACKET_TYPE_DATA
						|| notification.packet_type == PACKET_TYPE_ACK) {

						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s I am not the TX destination (N%d to N%d). Checking if Frame can be decoded.\n",
							SimTime(), node_id, node_state, LOG_D07, LOG_LVL2,
							notification.source_id, notification.destination_id);

						/** Can the packet be decoded? **/
						UpdateSINRFromNotification(notification);
						// 4 - Check if the packet is lost or not
						loss_reason = IsPacketLost(current_primary_channel, notification, notification, current_sinr,
							capture_effect, current_pd, power_rx_interest, constant_per, node_id, capture_effect_model);

						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Pmax_intf[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm, sinr = %f dB\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL5,
							channel_max_intereference, ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
							ConvertPower(PW_TO_DBM, power_rx_interest),
							ConvertPower(PW_TO_DBM, max_pw_interference),
							ConvertPower(LINEAR_TO_DB,current_sinr));

						/* ****************************************
						/* SPATIAL REUSE OPERATION
						 * *****************************************/
						// If the packet is not lost, check if we can ignore it by applying another pd
						if (spatial_reuse_enabled && loss_reason == PACKET_NOT_LOST) {
							// The incoming packet can be decoded by the default pd
							LOGS(save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s The packet could be decoded with the default pd (%f dBm)...\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL3, ConvertPower(PW_TO_DBM, current_pd));
							// Check if a new SR-based opportunity can be identified to ignore the incoming tranmission
							int new_txop_sr_identified(IdentifySpatialReuseOpportunity(power_rx_interest, potential_obss_pd_threshold));
							// Two cases:
							// (1) An SR-based opportunity was already identified and needs to be overwritten
							// (2) None SR opportunites were previously detected
							LOGS(save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s txop_sr_identified = %d / new_txop_sr_identified = %d\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL4, txop_sr_identified, new_txop_sr_identified);
							if ( (txop_sr_identified && new_txop_sr_identified &&
									potential_obss_pd_threshold <= current_obss_pd_threshold) ||
								(!txop_sr_identified && new_txop_sr_identified) ) {
								// Set the current OBSS/PD threshold for inter-BSS transmissions
								current_obss_pd_threshold = potential_obss_pd_threshold;
								// Update the variable that indicates that an SR-based opportunity has been detected
								txop_sr_identified = new_txop_sr_identified;
								// Indicate that the packet was "lost" in order to continue with the backoff procedure
								loss_reason = PACKET_IGNORED_SPATIAL_REUSE;
								// Define the limited transmission power
								next_tx_power_limit = ApplyTxPowerRestriction(current_obss_pd_threshold, current_tx_power);
								// Start (update) the trigger that indicates the end of the SR-based opportunity
								time_to_trigger = SimTime() + notification.tx_info.nav_time;
								txop_sr_end.Set(FixTimeOffset(time_to_trigger,13,12));
								LOGS(save_node_logs, node_logger.file,
									"%.15f;N%d;S%d;%s;%s An SR TXOP was detected for OBSS_PD = %f dBm "
									"(received RTS/CTS while being in SENSING state.)\n",
									SimTime(), node_id, node_state, LOG_D08, LOG_LVL3,
									ConvertPower(PW_TO_DBM, current_obss_pd_threshold));
							}
						}
						/* **************************************** */

						if(loss_reason == PACKET_NOT_LOST) { // RTS/DATA/CTS/ACK can be decoded

							LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Packet type %d can be decoded\n",
								SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, notification.packet_type);

							// Save NAV notification for comparing timestamps in case of need
							nav_notification = notification;

							int pause (HandleBackoff(PAUSE_TIMER, &channel_power,
								current_primary_channel, current_pd, buffer.QueueSize()));

							if(pause) {

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
								if (spatial_reuse_enabled && type_last_sensed_packet != INTRA_BSS_FRAME) {
									trigger_inter_bss_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
								} else {
									trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
								}

								LOGS(save_node_logs,node_logger.file,
									"%.15f;N%d;S%d;%s;%s Entering in NAV during %.12f and setting NAV timeout to %.12f\n",
									SimTime(), node_id, node_state, LOG_D08, LOG_LVL3,
									current_nav_time, trigger_NAV_timeout.GetTime());

	//							LOGS(save_node_logs,node_logger.file,
	//								"%.15f;N%d;S%d;%s;%s current_nav_time = %.12f\n",
	//								SimTime(), node_id, node_state, LOG_D08, LOG_LVL4,
	//								current_nav_time);

								node_state = STATE_NAV;
								last_time_not_in_nav = SimTime();
								++times_went_to_nav;

							}

						} else { // Frame cannot be decoded.

							LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Frame sent by N%d could not be decoded for reason %d\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL3,
								notification.source_id, loss_reason);

							// Save NAV notification for comparing timestamps in case of need
							outrange_nav_notification = notification;

							// Check if DIFS or BO must be stopped
							if(node_is_transmitter){

								LOGS(save_node_logs,node_logger.file,
									"%.15f;N%d;S%d;%s;%s Checking if BO must be paused...\n",
									SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);

								int pause;

								if(spatial_reuse_enabled && txop_sr_identified) {
									pause = HandleBackoff(PAUSE_TIMER, &channel_power,
										current_primary_channel, current_obss_pd_threshold, buffer.QueueSize());
								} else {
									pause = HandleBackoff(PAUSE_TIMER, &channel_power, current_primary_channel,
										current_pd, buffer.QueueSize());
								}

								// Check if node has to freeze the BO (if it is not already frozen)
								if (pause) {
									PauseBackoff();
								} else {
									if(trigger_end_backoff.Active()) remaining_backoff =
											ComputeRemainingBackoff(backoff_type, trigger_end_backoff.GetTime() - SimTime());
									LOGS(save_node_logs,node_logger.file,
										"%.15f;N%d;S%d;%s;%s BO must not be paused (%f remaining slots).\n",
										SimTime(), node_id, node_state, LOG_D08, LOG_LVL5, remaining_backoff/SLOT_TIME);
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

				/* ****************************************
				/* SPATIAL REUSE OPERATION
				 * *****************************************/
				int nav_collision(0);				// Variable to indicate whether a NAV collision occurred for the current detected notification
				int inter_bss_nav_collision(0);	// Variable to indicate whether an inter-BSS NAV collision occurred for the current detected notification
				// Check if a collision occurred for any of the NAV timers

				if (spatial_reuse_enabled && type_last_sensed_packet != INTRA_BSS_FRAME) {
					inter_bss_nav_collision = fabs(nav_notification.timestamp -
						notification.timestamp) < MAX_DIFFERENCE_SAME_TIME;
				} else {
					nav_collision = fabs(nav_notification.timestamp -
						notification.timestamp) < MAX_DIFFERENCE_SAME_TIME;

				}
				/* **************************************** */

				if(notification.destination_id == node_id){	// Node IS THE DESTINATION

					// If two or more packets sent at the same time
					if(nav_collision || inter_bss_nav_collision) {

						if(notification.packet_type == PACKET_TYPE_RTS) {	// Notification CONTAINS an RTS PACKET

							LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s RTS from my AP N%d sent simultaneously\n",
								SimTime(), node_id, node_state, LOG_D16, LOG_LVL4,
								notification.source_id);

							UpdateSINRFromNotification(notification);

							LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL5, channel_max_intereference,
								ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
								ConvertPower(PW_TO_DBM, power_rx_interest),
								ConvertPower(PW_TO_DBM, max_pw_interference));

							// Check if notification has been lost due to interferences or weak signal strength
							// TODO: method for checking whether the detected transmission can be decoded or not
							loss_reason = IsPacketLost(current_primary_channel, notification, notification,
								current_sinr, capture_effect, current_pd,
								power_rx_interest, constant_per, node_id, capture_effect_model);

							if(loss_reason != PACKET_NOT_LOST) {	// If RTS IS LOST, send logical Nack

								loss_reason = PACKET_LOST_BO_COLLISION;

								if(!node_is_transmitter) {

									// Sergio 18/09/2017:
									// NAV is no longer valid. It cannot be decoded due to interference.
									// Wait MAX_DIFFERENCE_SAME_TIME to detect more transmissions sent at the "same" time
									// Trigger the restart then.

									// Sergio on 27/09/2017. Review this case
									LOGS(save_node_logs,node_logger.file,
										"%.15f;N%d;S%d;%s;%s RTS from my AP CANNOT be decoded\n",
										SimTime(), node_id, node_state, LOG_D08, LOG_LVL5);

									trigger_NAV_timeout.Cancel();
									time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;

									// trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
									trigger_restart_sta.Set(FixTimeOffset(time_to_trigger,13,12));

								} else {

									printf("ALARM! Should not happen in downlink traffic\n");

								}
								// EOF HandleSlottedBackoffCollision();

								if(nack_activated) {
									LOGS(save_node_logs,node_logger.file,
										"%.15f;N%d;S%d;%s;%s RTS cannot be decoded (SINR = %f dB) -> Sending NACK corresponding to BO collision to N%d\n",
										SimTime(), node_id, node_state, LOG_D16, LOG_LVL5,
										ConvertPower(LINEAR_TO_DB, current_sinr), notification.source_id);
									logical_nack = GenerateLogicalNack(notification.packet_type,
										notification.packet_id, node_id, notification.source_id,
										NODE_ID_NONE, loss_reason, BER, current_sinr);
									SendLogicalNack(logical_nack);
								}

							} else {	// Data packet IS NOT LOST (it can be properly received)

								LOGS(save_node_logs,node_logger.file,
									"%.15f;N%d;S%d;%s;%s Reception of RTS #%d from N%d CAN be started (SINR = %f dB)\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id,
									notification.source_id, ConvertPower(LINEAR_TO_DB, current_sinr));

								// Cancel the previous NAV
								if ( spatial_reuse_enabled ) {
									trigger_inter_bss_NAV_timeout.Cancel(); // Cancel inter-BSS NAV
									LOGS(save_node_logs,node_logger.file,
										"%.15f;N%d;S%d;%s;%s INTER-BSS NAV CANCELLED!\n",
										SimTime(), node_id, node_state, LOG_D16, LOG_LVL4);
								} else {
									trigger_NAV_timeout.Cancel();			// Cancel intra-BSS NAV (legacy)
									LOGS(save_node_logs,node_logger.file,
										"%.15f;N%d;S%d;%s;%s DEFAULT NAV CANCELLED!\n",
										SimTime(), node_id, node_state, LOG_D16, LOG_LVL4);
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

						} else {	//	Notification does NOT CONTAIN an RTS
							LOGS(save_node_logs,node_logger.file,
									"%.15f;N%d;S%d;%s;%s Unexpected packet type (%d) received!\n",
									SimTime(), node_id, node_state, LOG_D08, LOG_LVL4, notification.packet_type);
						}

					} else { // Notification not detected to happen at the same time

						if (nack_activated) {
							// Send logical NACK to transmitter sending incoming notification for indicating the loss reason
							logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
								node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_RX_IN_NAV, BER, current_sinr);
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
						LOGS(save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL5, channel_max_intereference,
							ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
							ConvertPower(PW_TO_DBM, power_rx_interest),
							ConvertPower(PW_TO_DBM, max_pw_interference));
						// Check if notification can be decoded
						// TODO: method for checking whether the detected transmission can be decoded or not
						int loss_reason (IsPacketLost(current_primary_channel, notification, notification,
							current_sinr, capture_effect, current_pd, power_rx_interest, constant_per,
							node_id, capture_effect_model));

						// NAV collision detected
						if((nav_collision || inter_bss_nav_collision) && loss_reason == PACKET_NOT_LOST)  {

							LOGS(save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s Updating the NAV according to the last sensed transmission\n",
								SimTime(), node_id, node_state, LOG_D07, LOG_LVL2);

							if(!node_is_transmitter) {

								// Cancel the previous NAV and set it again according to the new one
								time_to_trigger = SimTime() + MAX_DIFFERENCE_SAME_TIME;
								if (spatial_reuse_enabled && inter_bss_nav_collision) {
									trigger_inter_bss_NAV_timeout.Cancel(); // Cancel inter-BSS NAV
									trigger_inter_bss_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
									LOGS(save_node_logs, node_logger.file,
										"%.15f;N%d;S%d;%s;%s (workaround) setting inter-BSS NAV trigger to %.12f\n",
										SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, time_to_trigger);
								} else {
									trigger_NAV_timeout.Cancel();			// Cancel intra-BSS NAV (legacy)
									trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
									LOGS(save_node_logs, node_logger.file,
										"%.15f;N%d;S%d;%s;%s (workaround) setting NAV trigger to %.12f\n",
										SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, time_to_trigger);
								}

							} else {

								if ( (nav_collision && nav_notification.packet_type == notification.packet_type)
									|| (inter_bss_nav_collision && nav_notification.packet_type == notification.packet_type) ) {

									// if(save_node_logs) fprintf(node_logger.file,
									//	"%.15f;N%d;S%d;%s;%s Waiting just in case of more collisions.\n",
									//	SimTime(), node_id, node_state, LOG_D07, LOG_LVL4);

									// Cancel the previous NAV
									if (spatial_reuse_enabled && inter_bss_nav_collision) {
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

									LOGS(save_node_logs, node_logger.file,
										"%.15f;N%d;S%d;%s;%s Recovering from EIFS at %.12f (preoc. = %.12f)\n",
										SimTime(), node_id, node_state, LOG_D07, LOG_LVL4,
										trigger_wait_collisions.GetTime(),
										notification.tx_info.preoccupancy_duration);

								}
							}

							// Do not send NACK because node is not the destination

						} else { // No collision

							LOGS(save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s I am not the TX destination (N%d to N%d). Checking if new RTS/CTS can be decoded.\n",
								SimTime(), node_id, node_state, LOG_D07, LOG_LVL2,
								notification.source_id, notification.destination_id);

							// Can the notification be decoded?
							UpdateSINRFromNotification(notification);

							LOGS(save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s Pmax_intf[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm, sinr = %f dB\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL5,
								channel_max_intereference, ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
								ConvertPower(PW_TO_DBM, power_rx_interest),
								ConvertPower(PW_TO_DBM, max_pw_interference),
								ConvertPower(LINEAR_TO_DB,current_sinr));

							// TODO: method for checking whether the detected transmission can be decoded or not
							loss_reason = IsPacketLost(current_primary_channel, notification, notification,
								current_sinr, capture_effect, current_pd, power_rx_interest, constant_per,
								node_id, capture_effect_model);

							int power_condition (ConvertPower(PW_TO_DBM, channel_power[current_primary_channel]) > sensitivity_default);

							if (loss_reason == PACKET_NOT_LOST && power_condition) {	// Packet IS NOT LOST

								/* ****************************************
								/* SPATIAL REUSE OPERATION
								/* *****************************************/
								// Check if the packet could have been decoded with SR pd
								// This allows transmitting once the NAV is over
								int loss_reason_sr (1);
								int power_condition_sr (1);
								if (spatial_reuse_enabled && type_last_sensed_packet != INTRA_BSS_FRAME && node_is_transmitter) { 	// Check for TXOP
									double power_interference (power_received_per_node[notification.source_id]);
									// TODO: method for checking whether the detected transmission can be decoded or not
									loss_reason_sr = IsPacketLost(current_primary_channel, notification, notification,
										current_sinr, capture_effect, potential_obss_pd_threshold, power_interference, constant_per,
										node_id, capture_effect_model);
									power_condition_sr = ConvertPower(PW_TO_DBM, channel_power[current_primary_channel]) > potential_obss_pd_threshold;
								}
								if (loss_reason_sr != PACKET_NOT_LOST && power_condition_sr) {
									txop_sr_identified = TRUE;	// TXOP identified!
									next_pd_spatial_reuse = potential_obss_pd_threshold;	// Update the pd
									LOGS(save_node_logs, node_logger.file,
										"%.15f;N%d;S%d;%s;%s TXOP detected while being in NAV state\n",
										SimTime(), node_id, node_state, LOG_D08, LOG_LVL3);
								} else {
									if (loss_reason == PACKET_NOT_LOST && txop_sr_identified) txop_sr_identified = FALSE; // Cancel SR TXOP!
								/* *****************************************/
									if (spatial_reuse_enabled && type_last_sensed_packet != INTRA_BSS_FRAME) { // Update inter-BSS NAV trigger
										nav_notification = notification;
										if(trigger_inter_bss_NAV_timeout.GetTime() < notification.tx_info.nav_time) {
											time_to_trigger = SimTime() +  notification.tx_info.nav_time + TIME_OUT_EXTRA_TIME;
											trigger_inter_bss_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
											LOGS(save_node_logs, node_logger.file,
												"%.15f;N%d;S%d;%s;%s Updating inter-BSS NAV timeout to the more restrictive one: From %.12f to %.12f\n",
												SimTime(), node_id, node_state, LOG_D07, LOG_LVL4,
												trigger_inter_bss_NAV_timeout.GetTime(), time_to_trigger);
										}
									} else {	// Update NAV trigger
										nav_notification = notification;
										if(trigger_NAV_timeout.GetTime() < notification.tx_info.nav_time) {
											time_to_trigger = SimTime() +  notification.tx_info.nav_time + TIME_OUT_EXTRA_TIME;
											trigger_NAV_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
											LOGS(save_node_logs, node_logger.file,
												"%.15f;N%d;S%d;%s;%s Updating NAV timeout to the more restrictive one: From %.12f to %.12f\n",
												SimTime(), node_id, node_state, LOG_D07, LOG_LVL4,
												trigger_NAV_timeout.GetTime(), time_to_trigger);
										}
									}
									LOGS(save_node_logs, node_logger.file,
										"%.15f;N%d;S%d;%s;%s New RTS/CTS arrived from (N%d). Setting NAV to new value %.18f\n",
										SimTime(), node_id, node_state, LOG_D07, LOG_LVL3,
										notification.source_id, trigger_NAV_timeout.GetTime());
								}

							} else {			// Packet IS LOST
								if(notification.packet_type == PACKET_TYPE_RTS
										||  notification.packet_type == PACKET_TYPE_CTS){
									LOGS(save_node_logs, node_logger.file,
										"%.15f;N%d;S%d;%s;%s RTS/CTS sent from N%d could not be decoded for reason %d\n",
										SimTime(), node_id, node_state, LOG_D08, LOG_LVL3,
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

				if(notification.destination_id == node_id){ // Node IS THE DESTINATION

					LOGS(save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
						SimTime(), node_id, node_state, LOG_D07, LOG_LVL3,
						notification.destination_id);

					LOGS(save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s I am transmitting, packet cannot be received\n",
						SimTime(), node_id, node_state, LOG_D18, LOG_LVL3);

					if(nack_activated) {
						// Send logical NACK to incoming notification transmitter due to receiver (node) was already receiving
						logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
							node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_DESTINATION_TX, BER, current_sinr);

						SendLogicalNack(logical_nack);
					}

				} else {	// Node IS NOT THE DESTINATION, do nothing

//					LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
//						SimTime(), node_id, node_state, LOG_D08, LOG_LVL3, notification.destination_id);

				}


				/* ****************************************
				/* SPATIAL REUSE OPERATION
				 * *****************************************/
				int nav_collision(0);			// Variable to indicate whether a NAV collision occurred for the current detected notification
				int inter_bss_nav_collision(0);	// Variable to indicate whether an inter-BSS NAV collision occurred for the current detected notification
				// Check if a collision occurred for any of the NAV timers
				if (spatial_reuse_enabled && type_last_sensed_packet != INTRA_BSS_FRAME) {
					inter_bss_nav_collision = fabs(nav_notification.timestamp -
						notification.timestamp) < MAX_DIFFERENCE_SAME_TIME;
				} else {
					nav_collision = fabs(nav_notification.timestamp -
						notification.timestamp) < MAX_DIFFERENCE_SAME_TIME;
				}

				// If two or more packets sent at the same time
				if(nav_collision || inter_bss_nav_collision) {

				}
//				else {
					// Check if new TXOP are detected when transmitting (in order to transmit again when restarting)
					if (spatial_reuse_enabled && type_last_sensed_packet != INTRA_BSS_FRAME && node_is_transmitter) {

						double power_interference (power_received_per_node[notification.source_id]);
						double sinr_interference (UpdateSINR(power_interference, max_pw_interference));

						// Is packet lost with the default pd?
						// TODO: method for checking whether the detected transmission can be decoded or not
						int loss_reason_legacy (IsPacketLost(current_primary_channel, notification, notification,
							sinr_interference, capture_effect, sensitivity_default, power_interference, constant_per,
							node_id, capture_effect_model));
						// Is packet lost with the SR pd?
						// TODO: method for checking whether the detected transmission can be decoded or not
						int loss_reason_sr (IsPacketLost(current_primary_channel, notification, notification,
							sinr_interference, capture_effect, potential_obss_pd_threshold, power_interference, constant_per,
							node_id, capture_effect_model));

						if(save_node_logs && node_id == 0) LOGS(save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s sinr_interference = %f - capture_effect = %f - pd_spatial_reuse = %f"
							" - power_interference = %f)\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL3,
							ConvertPower(LINEAR_TO_DB, sinr_interference), capture_effect,
							ConvertPower(PW_TO_DBM,pd_spatial_reuse),ConvertPower(PW_TO_DBM,power_interference));

						if(save_node_logs && node_id == 0) fprintf(node_logger.file,
							"%.15f;N%d;S%d;%s;%s CHECKING TXOP in TX state (pd_sr = %f - lost = %d)\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL3,
							ConvertPower(PW_TO_DBM,pd_spatial_reuse), loss_reason_sr);

						// If the packet has been ignored due to the OBSS_PD, then detect a TXOP
						if (loss_reason_legacy == PACKET_NOT_LOST && loss_reason_sr != PACKET_NOT_LOST) {
							txop_sr_identified = TRUE;	// TXOP identified!
							current_obss_pd_threshold = potential_obss_pd_threshold;
							// Define the limited transmission power
							next_tx_power_limit = ApplyTxPowerRestriction(current_obss_pd_threshold, current_tx_power);
							// Start (update) the trigger that indicates the end of the SR-based opportunity
							time_to_trigger = SimTime() + notification.tx_info.nav_time;
							txop_sr_end.Set(FixTimeOffset(time_to_trigger,13,12));
							LOGS(save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s TXOP detected while being in TX state\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL3);
						} else if (loss_reason_legacy == PACKET_NOT_LOST && txop_sr_identified) {
							// Cancel SR TXOP
							txop_sr_identified = FALSE;
							LOGS(save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s Cancelling SR TXOP while being in TX state\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL3);
						}
					}
//				}
				/* **************************************** */


}

/**
 * Handle InportSomeNodeStartTX for STATE_RX_DATA
 */
void Node :: HandleStartTX_StateRxData(const Notification &notification) {

				if(notification.destination_id == node_id){	// Node IS THE DESTINATION

//					LOGS(save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
//							SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, notification.destination_id);

					// Update the SINR
					current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

					// Check if ongoing notification has been lost due to interferences caused by new transmission
					loss_reason = IsPacketLost(current_primary_channel, incoming_notification, notification,
						current_sinr, capture_effect, current_pd,
						power_rx_interest, constant_per, node_id, capture_effect_model);

					// TODO: method for checking whether the detected transmission can be decoded or not
					// ...

					switch(capture_effect_model){

						case CE_DEFAULT:{
							if(loss_reason != PACKET_NOT_LOST
								&& loss_reason != PACKET_LOST_OUTSIDE_CH_RANGE)  {	// If ongoing data packet IS LOST
									// Pure collision (two nodes transmitting to me with enough power)
									LOGS(save_node_logs, node_logger.file,
										"%.15f;N%d;S%d;%s;%s Pure collision! Already receiving from N%d\n",
										SimTime(), node_id, node_state, LOG_D19, LOG_LVL4, receiving_from_node_id);
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
									if(nack_activated) {
										// Send NACK to both ongoing transmitter and incoming interferer nodes
										logical_nack = GenerateLogicalNack(notification.packet_type, nav_notification.packet_id,
												node_id, nav_notification.source_id, notification.source_id, loss_reason, BER, current_sinr);
										SendLogicalNack(logical_nack);
									}

							} else {	// If ongoing data packet IS NOT LOST (incoming transmission does not affect ongoing reception)

								if (nack_activated) {
									LOGS(save_node_logs, node_logger.file,
											"%.15f;N%d;S%d;%s;%s Low strength signal received while already receiving from N%d\n",
										SimTime(), node_id, node_state, LOG_D20, LOG_LVL4, receiving_from_node_id);

									// Send logical NACK to incoming transmitter indicating that node is already receiving
									logical_nack = GenerateLogicalNack(notification.packet_type, receiving_from_node_id,
											node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_LOW_SIGNAL_AND_RX, BER, current_sinr);

									SendLogicalNack(logical_nack);
								}

							}
							break;
						}

						case CE_IEEE_802_11:{
							int capture_effect_condition (power_received_per_node[notification.source_id] >
								power_received_per_node[receiving_from_node_id] + capture_effect);

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
									if (nack_activated) {
										// Send NACK to both ongoing transmitter and incoming interferer nodes
										logical_nack = GenerateLogicalNack(notification.packet_type, nav_notification.packet_id,
												node_id, NODE_ID_NONE, notification.source_id, PACKET_LOST_CAPTURE_EFFECT, BER, current_sinr);
										SendLogicalNack(logical_nack);
									}
								}  else {
									// Pure collision (two nodes transmitting to me with enough power)
									LOGS(save_node_logs, node_logger.file,
										"%.15f;N%d;S%d;%s;%s Pure collision! Already receiving from N%d\n",
										SimTime(), node_id, node_state, LOG_D19, LOG_LVL4, receiving_from_node_id);
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
									if(nack_activated){
										// Send NACK to both ongoing transmitter and incoming interferer nodes
										logical_nack = GenerateLogicalNack(notification.packet_type, nav_notification.packet_id,
												node_id, nav_notification.source_id, notification.source_id, loss_reason, BER, current_sinr);
										SendLogicalNack(logical_nack);
									}
								}
							} else { // If ongoing data packet IS NOT LOST (incoming transmission does not affect ongoing reception)
								if(nack_activated){
									LOGS(save_node_logs, node_logger.file,
											"%.15f;N%d;S%d;%s;%s Low strength signal received while already receiving from N%d\n",
										SimTime(), node_id, node_state, LOG_D20, LOG_LVL4, receiving_from_node_id);
									// Send logical NACK to incoming transmitter indicating that node is already receiving
									logical_nack = GenerateLogicalNack(notification.packet_type, receiving_from_node_id,
											node_id, notification.source_id, NODE_ID_NONE, PACKET_LOST_LOW_SIGNAL_AND_RX, BER, current_sinr);
									SendLogicalNack(logical_nack);
								}
							}
							break;
						}
					}

				} else {	// Node is NOT THE DESTINATION

//					LOGS(save_node_logs, node_logger.file,
//						"%.15f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
//						SimTime(), node_id, node_state, LOG_D08, LOG_LVL3, notification.destination_id);

					// Compute max interference (the highest one perceived in the reception channel range)
					ComputeMaxInterference(&max_pw_interference, &channel_max_intereference,
						incoming_notification, node_state, power_received_per_node, &channel_power);

					// Check if the ongoing reception is affected
					current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

					LOGS(save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm - current_sinr = %.2f dBm\n",
						SimTime(), node_id, node_state, LOG_D08, LOG_LVL5, channel_max_intereference,
						ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
						ConvertPower(PW_TO_DBM, power_rx_interest),
						ConvertPower(PW_TO_DBM, max_pw_interference),
						ConvertPower(LINEAR_TO_DB, current_sinr));

					// Check if the notification that was already being received is lost due to new notification
					if (spatial_reuse_enabled && txop_sr_identified) {
						loss_reason = IsPacketLost(current_primary_channel, incoming_notification, notification,
							current_sinr, capture_effect, current_obss_pd_threshold,
							power_rx_interest, constant_per, node_id, capture_effect_model);
					} else {
						loss_reason = IsPacketLost(current_primary_channel, incoming_notification, notification,
							current_sinr, capture_effect, current_pd,
							power_rx_interest, constant_per, node_id, capture_effect_model);
					}

					// TODO: method for checking whether the detected transmission can be decoded or not
					// ...

					LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s loss_reason = %d\n",
						SimTime(), node_id, node_state, LOG_D19, LOG_LVL4, loss_reason);

					if(loss_reason != PACKET_NOT_LOST) { 	// If ongoing packet reception IS LOST

						switch(capture_effect_model) {

							case CE_DEFAULT:{
								// Collision by hidden node
								LOGS(save_node_logs, node_logger.file,
									"%.15f;N%d;S%d;%s;%s Collision by interferences!\n",
									SimTime(), node_id, node_state, LOG_D19, LOG_LVL4);

								 // If two or more packets sent at the same time
								if(node_state == STATE_RX_RTS && notification.packet_type == PACKET_TYPE_RTS){
									if(fabs(notification.timestamp - incoming_notification.timestamp) < MAX_DIFFERENCE_SAME_TIME){
										loss_reason = PACKET_LOST_BO_COLLISION;
									}
								}
								// Send logical NACK to ongoing transmitter
								if (nack_activated) {
									logical_nack = GenerateLogicalNack(incoming_notification.packet_type,
										incoming_notification.packet_id, node_id, incoming_notification.source_id,
										NODE_ID_NONE, loss_reason, BER, current_sinr);
									SendLogicalNack(logical_nack);
								}
								RestartNode(FALSE);
								break;
							}

							case CE_IEEE_802_11:{
								int capture_effect_condition = power_received_per_node[notification.source_id] >
									power_received_per_node[receiving_from_node_id] + capture_effect;
								if (capture_effect_condition) {
									loss_reason = PACKET_LOST_CAPTURE_EFFECT;
									printf("Node %d was in state RX (from %d), and a new notification arrived from %d:\n", node_id, receiving_from_node_id, notification.source_id);
									printf("	* New RSSI: %f\n", power_received_per_node[notification.source_id]);
									printf("	* Old RSSI: %f:\n", power_received_per_node[receiving_from_node_id]);
									printf("	* CE: %f:\n", capture_effect);
									printf("	* loss_reason: %d:\n", loss_reason);
									if(nack_activated){
									// Send NACK to both ongoing transmitter and incoming interferer nodes
									logical_nack = GenerateLogicalNack(notification.packet_type, nav_notification.packet_id,
										node_id, nav_notification.source_id, notification.source_id, loss_reason, BER, current_sinr);
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

				if(notification.destination_id == node_id){	// Node is the destination

					power_rx_interest = power_received_per_node[notification.source_id];

					incoming_notification = notification;

//					LOGS(save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
//							SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, notification.destination_id);

					if(notification.packet_type == PACKET_TYPE_ACK){	// ACK packet transmission started

						// Compute max interference (the highest one perceived in the reception channel range)
						ComputeMaxInterference(&max_pw_interference, &channel_max_intereference,
							incoming_notification, node_state, power_received_per_node, &channel_power);

						// Check if notification has been lost due to interferences or weak signal strength
						current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

						// TODO: method for checking whether the detected transmission can be decoded or not
						loss_reason = IsPacketLost(current_primary_channel, incoming_notification, notification,
								current_sinr, capture_effect, current_pd,
								power_rx_interest, constant_per, node_id, capture_effect_model);

						if(loss_reason != PACKET_NOT_LOST
								&& loss_reason != PACKET_LOST_OUTSIDE_CH_RANGE) {	// If ACK packet IS LOST, send logical Nack

							LOGS(save_node_logs, node_logger.file,
									"%.15f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
									SimTime(), node_id, node_state, LOG_D15, LOG_LVL4, notification.packet_id,
									notification.source_id, loss_reason);

							if(nack_activated){
								// Send logical NACK to ACK transmitter
								logical_nack = GenerateLogicalNack(incoming_notification.packet_type, incoming_notification.packet_id,
									node_id, receiving_from_node_id, NODE_ID_NONE, loss_reason, BER, current_sinr);
								SendLogicalNack(logical_nack);
							}

							// Do nothing until ACK timeout is triggered

						} else {	// If ACK packet IS NOT LOST (it can be properly received)

							LOGS(save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s Reception of ACK %d from N%d CAN be started\n",
								SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id, notification.source_id);

							// Cancel ACK timeout and go to STATE_RX_ACK while updating receiving info
							trigger_ACK_timeout.Cancel();
							node_state = STATE_RX_ACK;
							receiving_from_node_id = notification.source_id;
							receiving_packet_id = notification.packet_id;

//							LOGS(save_node_logs, node_logger.file,
//									"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
//									SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.destination_id);

//							LOGS(save_node_logs, node_logger.file,
//									"%.15f;N%d;S%d;%s;%s current_sinr = %f dB\n",
//									SimTime(), node_id, node_state, LOG_D16, LOG_LVL5,
//									ConvertPower(LINEAR_TO_DB,current_sinr));

						}

					}  else {	//	Some packet type received that is not ACK
						LOGS(save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s Unexpected packet type received!\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION, do nothing
//
//					LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
//								SimTime(), node_id, node_state, LOG_D08, LOG_LVL3, notification.destination_id);
//
				}

}

/**
 * Handle InportSomeNodeStartTX for STATE_WAIT_CTS
 */
void Node :: HandleStartTX_StateWaitCts(const Notification &notification) {

				if(notification.destination_id == node_id){	// Node is the destination

					incoming_notification = notification;

//					LOGS(save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
//							SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, notification.destination_id);

					if(notification.packet_type == PACKET_TYPE_CTS){	// CTS packet transmission started

						power_rx_interest = power_received_per_node[notification.source_id];

						// Compute max interference (the highest one perceived in the reception channel range)
						ComputeMaxInterference(&max_pw_interference, &channel_max_intereference,
							incoming_notification, node_state, power_received_per_node, &channel_power);

						// Check if notification has been lost due to interferences or weak signal strength
						current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

//						LOGS(save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s P_sn = %f dBm (%f pW) - P_st= %f dBm (%f pW)"
//							"- P_if = %f dBm (%f pW)\n",
//							SimTime(), node_id, node_state, LOG_D08, LOG_LVL5,
//							ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]), channel_power[channel_max_intereference],
//							ConvertPower(PW_TO_DBM, power_rx_interest), power_rx_interest, ConvertPower(PW_TO_DBM, max_pw_interference),
//							max_pw_interference);

						// TODO: method for checking whether the detected transmission can be decoded or not
						loss_reason = IsPacketLost(current_primary_channel, incoming_notification, notification,
							current_sinr, capture_effect, current_pd,
							power_rx_interest, constant_per, node_id, capture_effect_model);

						if(loss_reason != PACKET_NOT_LOST
								&& loss_reason != PACKET_LOST_OUTSIDE_CH_RANGE)  {	// If CTS packet IS LOST, send logical Nack

							LOGS(save_node_logs, node_logger.file,
								"%.15f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
								SimTime(), node_id, node_state, LOG_D15, LOG_LVL4, notification.packet_id,
								notification.source_id, loss_reason);

							if(nack_activated){
								// Send logical NACK to ACK transmitter
								logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
										node_id, notification.source_id,
										NODE_ID_NONE, loss_reason, BER, current_sinr);
								SendLogicalNack(logical_nack);
							}

							// Do nothing until ACK timeout is triggered

						} else {	// If CTS packet IS NOT LOST (it can be properly received)

							LOGS(save_node_logs, node_logger.file,
									"%.15f;N%d;S%d;%s;%s Reception of CTS #%d from N%d CAN be started\n",
									SimTime(), node_id, node_state, LOG_D16, LOG_LVL4,
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

//							LOGS(save_node_logs, node_logger.file,
//									"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d)\n",
//									SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.destination_id);

//							LOGS(save_node_logs, node_logger.file,
//									"%.15f;N%d;S%d;%s;%s current_sinr = %f dB\n",
//									SimTime(), node_id, node_state, LOG_D16, LOG_LVL5, ConvertPower(LINEAR_TO_DB,current_sinr));

						}

					}  else {	//	Some packet type received that is not CTS
						LOGS(save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s Unexpected packet type received!\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION, do nothing

				}


}

/**
 * Handle InportSomeNodeStartTX for STATE_WAIT_DATA
 */
void Node :: HandleStartTX_StateWaitData(const Notification &notification) {

			if(notification.destination_id == node_id){	// Node is the destination

				power_rx_interest = power_received_per_node[notification.source_id];
				incoming_notification = notification;

//					LOGS(save_node_logs, node_logger.file,
//							"%.15f;N%d;S%d;%s;%s I am the TX destination (N%d). Checking if notification can be received.\n",
//							SimTime(), node_id, node_state, LOG_D07, LOG_LVL3, notification.destination_id);

				if(notification.packet_type == PACKET_TYPE_DATA){	// DATA packet transmission started

					// Compute max interference (the highest one perceived in the reception channel range)
					ComputeMaxInterference(&max_pw_interference, &channel_max_intereference,
						incoming_notification, node_state, power_received_per_node, &channel_power);

					// Check if notification has been lost due to interferences or weak signal strength
					current_sinr = UpdateSINR(power_rx_interest, max_pw_interference);

					LOGS(save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm - P_st = %f dBm - P_if = %f dBm - current_sinr = %.2f dBm\n",
						SimTime(), node_id, node_state, LOG_D08, LOG_LVL5, channel_max_intereference,
						ConvertPower(PW_TO_DBM, channel_power[channel_max_intereference]),
						ConvertPower(PW_TO_DBM, power_rx_interest),
						ConvertPower(PW_TO_DBM, max_pw_interference),
						ConvertPower(LINEAR_TO_DB, current_sinr));

					// TODO: method for checking whether the detected transmission can be decoded or not
					loss_reason = IsPacketLost(current_primary_channel, incoming_notification, notification,
						current_sinr, capture_effect, current_pd,
						power_rx_interest, constant_per, node_id, capture_effect_model);

					if(loss_reason != PACKET_NOT_LOST
						&& loss_reason != PACKET_LOST_OUTSIDE_CH_RANGE)  {	// If DATA packet IS LOST, send logical Nack

						LOGS(save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s Reception of notification %d from N%d CANNOT be started because of reason %d\n",
							SimTime(), node_id, node_state, LOG_D15, LOG_LVL4, notification.packet_id,
							notification.source_id, loss_reason);

						if(nack_activated){
							// Send logical NACK to DATA transmitter
							logical_nack = GenerateLogicalNack(notification.packet_type, notification.packet_id,
									node_id, notification.source_id,
									NODE_ID_NONE, loss_reason, BER, current_sinr);
							SendLogicalNack(logical_nack);
						}

					} else {	// If DATA packet IS NOT LOST (it can be properly received)

						LOGS(save_node_logs, node_logger.file,
							"%.15f;N%d;S%d;%s;%s Reception of DATA %d from N%d CAN be started\n",
							SimTime(), node_id, node_state, LOG_D16, LOG_LVL4, notification.packet_id, notification.source_id);

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
					LOGS(save_node_logs, node_logger.file,
						"%.15f;N%d;S%d;%s;%s Unexpected packet type received!\n",
						SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
				}

			} else {	// Node IS NOT THE DESTINATION, do nothing

//					LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s I am NOT the TX destination (N%d)\n",
//								SimTime(), node_id, node_state, LOG_D08, LOG_LVL3, notification.destination_id);

			}

			/* ****************************************
			/* SPATIAL REUSE OPERATION
				* *****************************************/
			/* **************************************** */

		}


void Node :: InportSomeNodeStartTX(Notification &notification){

	LOGS(save_node_logs, node_logger.file,
			"%.15f;N%d;S%d;%s;%s InportSomeNodeStartTX(): N%d to N%d sends packet type %d in range %d-%d using a transmit power of %.2f dBm\n",
			SimTime(), node_id, node_state, LOG_D00, LOG_LVL1,
			notification.source_id, notification.destination_id, notification.packet_type,
			notification.left_channel, notification.right_channel,
			ConvertPower(PW_TO_DBM, notification.tx_info.tx_power));

	LOGS(save_node_logs,node_logger.file,
	        "%.15f;N%d;S%d;%s;%s Nodes transmitting: ",
			SimTime(), node_id, node_state, LOG_D00, LOG_LVL3);

	// Identify node that has started the transmission as transmitting node in the array
	nodes_transmitting[notification.source_id] = TRUE;
	PrintOrWriteNodesTransmitting(WRITE_LOG, save_node_logs,
		print_node_logs, node_logger, total_nodes_number, nodes_transmitting);

	// TOKENIZED BO ONLY
	if(node_is_transmitter && backoff_type == BACKOFF_TOKENIZED) {
		// 1 - Check that the incoming transmission is not originated or directed to the transmitter
		if (notification.source_id != node_id && notification.destination_id != node_id) {
			// 2 - Check that the incoming transmission is an RTS or DATA
			if (notification.packet_type == PACKET_TYPE_RTS
				|| notification.packet_type == PACKET_TYPE_DATA) {
				// 3 - Check that the incoming transmission comes from a nearby device
				if (received_power_array[notification.source_id] > current_pd) {
					// Update the list of neighboring devices (if not done)
					if (token_order_list[notification.source_id] == DEVICE_INACTIVE_FOR_TOKEN) {
						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token-based channel access operation (update neighbor list):\n",
							SimTime(), node_id, node_state, LOG_E18, LOG_LVL3);
						LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Token's order list before the update: ",
								SimTime(), node_id, node_state, LOG_E18, LOG_LVL5);
							PrintOrWriteTokenList(WRITE_LOG, save_node_logs, node_logger,
								print_node_logs, token_order_list, total_nodes_number);
						UpdateTokenList(ADD_DEVICE_TO_LIST, &token_order_list, notification.source_id);
						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token's order list updated: ",
							SimTime(), node_id, node_state, LOG_E18, LOG_LVL5);
						PrintOrWriteTokenList(WRITE_LOG, save_node_logs, node_logger,
							print_node_logs, token_order_list, total_nodes_number);
					}
					// Update the status of the token
					if (node_state != STATE_TX_RTS) {
						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token-based channel access operation (token ACQUISITION):\n",
							SimTime(), node_id, node_state, LOG_E18, LOG_LVL4);
						UpdateTokenStatus(node_id, TAKE_TOKEN, &token_status, notification.source_id,
							token_order_list, total_nodes_number, &distance_to_token);
					} else if (node_state == STATE_TX_RTS && node_id < notification.source_id) {
						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token-based channel access operation (token ACQUISITION):\n",
							SimTime(), node_id, node_state, LOG_E18, LOG_LVL4);
						UpdateTokenStatus(node_id, TAKE_TOKEN, &token_status, node_id,
							token_order_list, total_nodes_number, &distance_to_token);
					} else {
						// In case of a collision (two simultaneous RTS transmissions occur),
						// solve the conflict by releasing the token (the node with lowest ID gets the token)
						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Token-based channel access operation (token RELEASE):\n",
							SimTime(), node_id, node_state, LOG_E18, LOG_LVL4);
						UpdateTokenStatus(node_id, RELEASE_TOKEN, &token_status, node_id,
							token_order_list, total_nodes_number, &distance_to_token);
					}
					LOGS(save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Token status updated, the new token holder is %d\n",
						SimTime(), node_id, node_state, LOG_E18, LOG_LVL5, token_status);
					// Update the CW parameters
					HandleContentionWindow(
						cw_adaptation, -1, &deterministic_bo_active, &current_cw_min, &current_cw_max, &cw_stage_current,
						cw_min_default, cw_max_default, cw_stage_max, distance_to_token, backoff_type);
					LOGS(save_node_logs,node_logger.file,
						"%.15f;N%d;S%d;%s;%s Updated CW parameters (token-based BO) = [%d-%d]\n",
						SimTime(), node_id, node_state, LOG_E18, LOG_LVL5, current_cw_min, current_cw_max);
				}
			}
		}
	}

	if(notification.source_id == node_id){ // If OWN NODE IS THE TRANSMITTER, do nothing

		LOGS(save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s I have started a TX of packet #%d (type %d) to N%d in channels %d - %d of duration %.9f us\n",
			SimTime(), node_id, node_state, LOG_D02, LOG_LVL2, notification.packet_id,
			notification.packet_type, notification.destination_id,
			notification.left_channel, notification.right_channel, notification.tx_duration * pow(10,6));


	} else {	// If OTHER NODE IS THE TRANSMITTER

		LOGS(save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s N%d has started a TX of packet #%d (type %d) to N%d in channels %d - %d\n",
			SimTime(), node_id, node_state, LOG_D02, LOG_LVL2, notification.source_id,
			notification.packet_id,	notification.packet_type, notification.destination_id,
			notification.left_channel, notification.right_channel);

		LOGS(save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Power sensed per channel BEFORE updating [dBm]: ",
			SimTime(), node_id, node_state, LOG_E18, LOG_LVL3);

		PrintOrWriteChannelPower(WRITE_LOG, save_node_logs, print_node_logs, node_logger,
					&channel_power);

		// Update 'power received' array in case a new tx power is used
		if (notification.tx_info.flag_change_in_tx_power) {
			received_power_array[notification.source_id] =
				ComputePowerReceived(distances_array[notification.source_id],
				notification.tx_info.tx_power, central_frequency, path_loss_model);
		}

		// Update the power sensed at each channel
		UpdateChannelsPower(&channel_power, notification, TX_INITIATED,
			central_frequency, path_loss_model, adjacent_channel_model, received_power_array[notification.source_id], node_id);

		LOGS(save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Power sensed per channel [dBm]: ",
			SimTime(), node_id, node_state, LOG_E18, LOG_LVL3);

		PrintOrWriteChannelPower(WRITE_LOG, save_node_logs, print_node_logs, node_logger,
			&channel_power);

		// Call UpdatePowerSensedPerNode() ONLY for adding power (some node started)
		UpdatePowerSensedPerNode(current_primary_channel, power_received_per_node, notification,
			central_frequency, path_loss_model, received_power_array[notification.source_id], TX_INITIATED);

		UpdateTimestampChannelFreeAgain(timestampt_channel_becomes_free, &channel_power,
			current_pd, SimTime());

		/* ****************************************
		/* SPATIAL REUSE OPERATION
		 *
		 *  Determine the parameters to be potentially used according to the SR operation.
		 *  - pd_spatial_reuse: PD threshold to be used according to the type of detected frame
		 *  - tx_power_sr: Tx power to be used in case of detecting a TXOP (depends on pd_spatial_reuse)
		 *
		 * *****************************************/
		if (spatial_reuse_enabled) {
			// Identify the source of detected packet
			type_last_sensed_packet = CheckPacketOrigin(notification, bss_color, srg);
			// Obtain the CST to be used
			potential_obss_pd_threshold = GetSensitivitySpatialReuse(type_last_sensed_packet,
				srg_obss_pd, non_srg_obss_pd, current_pd, power_received_per_node[notification.source_id]);
			// In case of detecting an inter-BSS frame, print the information
			if (type_last_sensed_packet != INTRA_BSS_FRAME) {
				LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s SPATIAL REUSE OPERATION: \n",
					SimTime(), node_id, node_state, LOG_F02, LOG_LVL3);
				LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s type_last_sensed_packet = %d\n",
					SimTime(), node_id, node_state, LOG_F02, LOG_LVL4, type_last_sensed_packet);
				LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s Previous current_obss_pd_threshold = %f\n",
					SimTime(), node_id, node_state, LOG_F02, LOG_LVL4, ConvertPower(PW_TO_DBM, current_obss_pd_threshold));
				LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s previous txop_sr_identified = %d\n",
					SimTime(), node_id, node_state, LOG_F02, LOG_LVL4, txop_sr_identified);
				LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s New potential_obss_pd_threshold = %f\n",
					SimTime(), node_id, node_state, LOG_F02, LOG_LVL4, ConvertPower(PW_TO_DBM, potential_obss_pd_threshold));
			}
		}
		/* **************************************** */

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
			default:{
				printf("ERROR: %d is not a correct state\n", node_state);
				exit(EXIT_FAILURE);
			}


		}


	}

	// STATISTICS: compute the time the channel is idle (Node 0 is responsible to monitor this)
	if (node_id == 0 && channel_idle) {
		sum_time_channel_idle += (SimTime() - last_time_channel_is_idle);
		channel_idle = false;
	}

	// LOGS(save_node_logs, node_logger.file, "%.15f;N%d;S%d;%s;%s InportSomeNodeStartTX() END\n", SimTime(), node_id, node_state, LOG_D01, LOG_LVL1);
};

/**
 * Helper for InportSomeNodeFinishTX: handles STATE_SENSING case
 */
void Node :: HandleFinishTX_StateSensing(const Notification &notification){

	if(node_is_transmitter) {
		if(!trigger_start_backoff.Active()
			&& !trigger_end_backoff.Active()){	// BO was paused and DIFS not initiated


			int resume (HandleBackoff(RESUME_TIMER, &channel_power, current_primary_channel, current_pd,
					buffer.QueueSize()));

			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s P[%d] = %f dBm (%f)\n",
				SimTime(), node_id, node_state, LOG_D08, LOG_LVL5,
				current_primary_channel, ConvertPower(PW_TO_DBM, channel_power[current_primary_channel]), channel_power[current_primary_channel]);

			if (resume) {	// BO can be resumed
				// Sergio on 26/09/2017. EIFS vs NAV.
				// - To identify if previous packet lost to trigger the EIFS
				// - If not, just resume the backoff
				time_to_trigger = SimTime() + DIFS;
				// time_to_trigger = SimTime() + SIFS + notification.tx_info.cts_duration + DIFS;
				trigger_start_backoff.Set(FixTimeOffset(time_to_trigger,13,12));
				LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s BO will be resumed after DIFS at %.12f.\n",
					SimTime(), node_id, node_state, LOG_E11, LOG_LVL4,
					trigger_start_backoff.GetTime());
//							LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s EIFS started.\n",
//														SimTime(), node_id, node_state, LOG_E11, LOG_LVL4);
			} else {	// BO cannot be resumed
				LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s EIF/DIFSS cannot be starte because the channel is busyd.\n",
					SimTime(), node_id, node_state, LOG_E11, LOG_LVL4);

}
		} else {	// BO was already active
			LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s BO was already active.\n",
					SimTime(), node_id, node_state, LOG_E11, LOG_LVL4);
		}
	}
}

/**
 * Helper for InportSomeNodeFinishTX: handles STATE_RX_DATA case
 */
void Node :: HandleFinishTX_StateRxData(const Notification &notification){

	if(notification.destination_id == node_id){ 	// Node IS THE DESTINATION

		if(notification.packet_type == PACKET_TYPE_DATA){	// Data packet transmission finished

			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Packet #%d reception from N%d is finished successfully.\n",
				SimTime(), node_id, node_state, LOG_E14, LOG_LVL3, notification.packet_id,
				notification.source_id);

			// Generate and send ACK to transmitter after SIFS
			node_state = STATE_TX_ACK;

			// Compute the NAV time
			current_nav_time = ComputeNavTime(node_state, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
			current_nav_time = FixTimeOffset(current_nav_time,13,12); // Update the NAV time according to the time offsets

			current_tx_duration = ack_duration;
			current_destination_id = notification.source_id;
			ack_notification = GenerateNotification(PACKET_TYPE_ACK, current_destination_id,
					notification.packet_id, notification.tx_info.num_packets_aggregated,
					notification.timestamp_generated, current_tx_duration);

                        // Reset the flag that indicates whether the tx power changed or not
                        flag_change_in_tx_power = FALSE;

                        // Set the preoccupancy duration for the ACK
			ack_notification.tx_info.preoccupancy_duration = time_rand_value;


			// ------------------------------------------------------------------------
			// Sergio on 07 Dec 2017: add ACK transmission time to spectrum utilization
			for(int c = current_left_channel; c <= current_right_channel; ++c){
				total_time_channel_busy_per_channel[c] = total_time_channel_busy_per_channel[c] + current_tx_duration;
			}
			// ------------------------------------------------------------------------

			// triggers the SendResponsePacket() function after SIFS
			time_to_trigger = SimTime() + SIFS;
			trigger_SIFS.Set(FixTimeOffset(time_to_trigger,13,12));

			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s SIFS will be triggered in %.12f\n",
				SimTime(), node_id, node_state, LOG_E14, LOG_LVL3,
				trigger_SIFS.GetTime());

		} else {	// Other packet type transmission finished
			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
				SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
		}

	} else {	// Node IS NOT THE DESTINATION, do nothing

		LOGS(save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Still locked into the reception of packet #%d from N%d.\n",
			SimTime(), node_id, node_state, LOG_E15, LOG_LVL3, notification.packet_id,
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

	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s InportSomeNodeFinishTX(): N%d to N%d (type %d)"
			" at range %d-%d "
			"- nodes transmitting: ",
		SimTime(), node_id, node_state, LOG_E00, LOG_LVL1,
		notification.source_id, notification.destination_id, notification.packet_type,
		notification.left_channel, notification.right_channel);

	// Identify node that has finished the transmission as non-transmitting node in the array
	nodes_transmitting[notification.source_id] = FALSE;
	PrintOrWriteNodesTransmitting(WRITE_LOG, save_node_logs,
			print_node_logs, node_logger, total_nodes_number, nodes_transmitting);

	// Update the list of neighboring devices
	//UpdateTokenList(ADD_DEVICE_TO_LIST, &token_order_list, notification.source_id);

	// TOKENIZED BO ONLY
	if(node_is_transmitter && backoff_type == BACKOFF_TOKENIZED &&
			token_order_list[notification.destination_id] == DEVICE_ACTIVE_FOR_TOKEN) {
		// - Check that the incoming transmission is an ACK
		if (notification.packet_type == PACKET_TYPE_ACK) {
			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Token-based channel access operation (token RELEASE):\n",
				SimTime(), node_id, node_state, LOG_E18, LOG_LVL4);
			// Update the status of the token
			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s UPDATING the status of the token (until now, with N%d)\n",
				SimTime(), node_id, node_state, LOG_E18, LOG_LVL3, token_status);
			UpdateTokenStatus(node_id, RELEASE_TOKEN, &token_status, notification.destination_id,
					token_order_list, total_nodes_number, &distance_to_token);
			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Token status updated, the new token holder is %d\n",
				SimTime(), node_id, node_state, LOG_E18, LOG_LVL5, token_status);
			// Update the CW parameters
			HandleContentionWindow(
				cw_adaptation, -1, &deterministic_bo_active, &current_cw_min, &current_cw_max, &cw_stage_current,
				cw_min_default, cw_max_default, cw_stage_max, distance_to_token, backoff_type);
			LOGS(save_node_logs,node_logger.file,
				"%.15f;N%d;S%d;%s;%s Updated CW parameters (token-based BO) = [%d-%d]\n",
				SimTime(), node_id, node_state, LOG_E18, LOG_LVL5, current_cw_min, current_cw_max);
		}
	}

	if(notification.source_id == node_id){	// Node is the TX source: do nothing

//		LOGS(save_node_logs,node_logger.file,
//				"%.15f;N%d;S%d;%s;%s I have finished the TX of packet #%d (type %d) in channel range: %d - %d\n",
//				SimTime(), node_id, node_state, LOG_E18, LOG_LVL2, notification.packet_id,
//				notification.packet_type, notification.left_channel, notification.right_channel);

	} else {	// Node is not the TX source

//		LOGS(save_node_logs,node_logger.file,
//				"%.15f;N%d;S%d;%s;%s N%d has finished the TX of packet #%d (type %d) in channel range: %d - %d\n",
//				SimTime(), node_id, node_state, LOG_E18, LOG_LVL2, notification.source_id,
//				notification.packet_id, notification.packet_type, notification.left_channel,
//				notification.right_channel);

		LOGS(save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Power sensed per channel BEFORE updating [dBm]: ",
			SimTime(), node_id, node_state, LOG_E18, LOG_LVL3);

		PrintOrWriteChannelPower(WRITE_LOG, save_node_logs, print_node_logs, node_logger,
					&channel_power);

		// Update the power sensed at each channel
		UpdateChannelsPower(&channel_power, notification, TX_FINISHED,
			central_frequency, path_loss_model, adjacent_channel_model, received_power_array[notification.source_id], node_id);

		// -------------------------
		// Safety condtion. Empty the channel when no node is transmitting
		int num_nodes_transmitting = 0;
		for(int i = 0; i < total_nodes_number; ++i){
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

		/* ****************************************
		/* SPATIAL REUSE OPERATION
		 * *****************************************/
		if(spatial_reuse_enabled) {
			//  Update the type of ongoing transmissions
			UpdateTypeOngoingTransmissions(type_ongoing_transmissions_sr,
				notification, bss_color, srg, 0);
		}
		/* **************************************** */

		LOGS(save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Power sensed per channel [dBm]: ",
			SimTime(), node_id, node_state, LOG_E18, LOG_LVL3);

		PrintOrWriteChannelPower(WRITE_LOG, save_node_logs, print_node_logs, node_logger,
				&channel_power);

		// Call UpdatePowerSensedPerNode() ONLY for adding power (some node started)
		UpdatePowerSensedPerNode(current_primary_channel, power_received_per_node, notification,
			central_frequency, path_loss_model, received_power_array[notification.source_id], TX_FINISHED);

		UpdateTimestampChannelFreeAgain(timestampt_channel_becomes_free, &channel_power,
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

				if(notification.destination_id == node_id){	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_ACK){	// ACK packet transmission finished

						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s ACK #%d reception from N%d is finished successfully.\n",
							SimTime(), node_id, node_state, LOG_E14, LOG_LVL3, notification.packet_id,
							notification.source_id);

						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s -------------------------------------\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL2);
						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s TRANSMISSION #%d SUCCESSFULLY FINISHED!\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL2,
							data_packets_acked);
						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s -------------------------------------\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL2);

						// Mark the previous transmission as successful
						last_transmission_successful = 1;

						// Whole data packet ACKed
						++data_packets_acked;
						++data_packets_acked_per_sta[current_destination_id-node_id-1];

						current_tx_duration = current_tx_duration + (notification.tx_duration + SIFS);	// Add ACK time to tx_duration

						// Update packet statistics
						for(int i = 0; i < limited_num_packets_aggregated; ++i){

							++data_frames_acked;
							++data_frames_acked_per_sta[current_destination_id-node_id-1];
							++num_delay_measurements;
							sum_delays = sum_delays + (SimTime() - buffer.GetFirstPacket().timestamp_generated);

							if (buffer.GetFirstPacket().timestamp_generated > (simulation_time_komondor - last_measurements_window)) {
								++last_data_frames_acked;
								++last_num_delay_measurements;
								last_sum_delays = last_sum_delays + (SimTime() - buffer.GetFirstPacket().timestamp_generated);
							}

							// Update the performance report with delay measurements
							performance_report.sum_delays = performance_report.sum_delays + (SimTime() - buffer.GetFirstPacket().timestamp_generated);
							++performance_report.num_delay_measurements;
							if ((SimTime() - buffer.GetFirstPacket().timestamp_generated) > performance_report.max_delay) {
								performance_report.max_delay = (SimTime() - buffer.GetFirstPacket().timestamp_generated);
							}
							if ((SimTime() - buffer.GetFirstPacket().timestamp_generated) < performance_report.min_delay) {
								performance_report.min_delay = (SimTime() - buffer.GetFirstPacket().timestamp_generated);
							}
							LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Packet delay: %f us (generated at %f).\n",
								SimTime(), node_id, node_state, LOG_E14, LOG_LVL4,
								(SimTime() - buffer.GetFirstPacket().timestamp_generated) * pow(10,6),
								buffer.GetFirstPacket().timestamp_generated);

							// Delete all the aggregated frames contained in the ACKed packet
							//buffer.DelFirstPacket();

						}

						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Handling contention window\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
						LOGS(save_node_logs,node_logger.file,
									"%.15f;N%d;S%d;%s;%s From CW = [%d-%d], b = %d, m = %d\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL5,
							current_cw_min, current_cw_max, cw_stage_current, cw_stage_max);
						// - Transmission succeeded ---> reset CW if binary exponential backoff is implemented
						HandleContentionWindow(
							cw_adaptation, RESET_CW, &deterministic_bo_active, &current_cw_min, &current_cw_max,
							&cw_stage_current, cw_min_default, cw_max_default, cw_stage_max, distance_to_token, backoff_type);

						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s To CW = [%d-%d], b = %d, m = %d\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL5,
							current_cw_min, current_cw_max, cw_stage_current, cw_stage_max);
						// Restart node (implicitly to STATE_SENSING)


						// Extra slot for successful transmissions
						RestartNode(FALSE);

					} else {	// Other packet type transmission finished
						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
							SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION

					LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Still receiving packet #%d reception from N%d.\n",
						SimTime(), node_id, node_state, LOG_E15, LOG_LVL3, incoming_notification.packet_id,
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

				if(notification.destination_id == node_id){	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_RTS){	// RTS packet transmission finished

						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s RTS #%d reception from N%d is finished successfully.\n",
							SimTime(), node_id, node_state, LOG_E14, LOG_LVL3, notification.packet_id,
							notification.source_id);

						// Check channel availability in order to send the CTS
						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Checking if CTS can be sent: P_sen = %f dBm, pd = %f dBm.\n",
							SimTime(), node_id, node_state, LOG_E14, LOG_LVL3,
							ConvertPower(PW_TO_DBM, channel_power[current_primary_channel]),
							ConvertPower(PW_TO_DBM, current_pd));


						// Issue #146 RTS/CTS BW indication
						// - If incoming packet is RTS decodable:
						// 1. Perform CCA assessment in full range
						// 2. Derive new operation BW (e.g., if original range was 0 to 7 and 5 is busy, report 0 to 3)
						// 3. Transmit (later) CTS

						int CTS_transmission_possible = FALSE;

						GetChannelOccupancyByCCA(current_primary_channel, pifs_activated, channels_free, current_left_channel,
								current_right_channel, &channel_power, current_pd, timestampt_channel_becomes_free, SimTime(), PIFS);

						LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Channels founds free after RTS: ",
								SimTime(), node_id, node_state, LOG_F02, LOG_LVL3);

						PrintOrWriteChannelsFree(WRITE_LOG, save_node_logs, print_node_logs, node_logger,
							channels_free);

						GetTxChannels(channels_for_tx, current_dcb_policy, channels_free,
								current_left_channel, current_right_channel, current_primary_channel,
								NUM_CHANNELS_KOMONDOR, &channel_power, channel_aggregation_cca_model);
						
						LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Channels for transmitting after RTS: ",
								SimTime(), node_id, node_state, LOG_F02, LOG_LVL2);

						PrintOrWriteChannelForTx(WRITE_LOG, save_node_logs, print_node_logs, node_logger,
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


						//if(ConvertPower(PW_TO_DBM, channel_power[current_primary_channel]) < current_pd) {

						if(CTS_transmission_possible){

							LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Channel(s) is (are) clear! Sending CTS to N%d (STATE = %d) ...\n",
								SimTime(), node_id, node_state, LOG_E14, LOG_LVL3, current_destination_id, node_state);

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
								frame_length, bits_ofdm_sym);

							current_nav_time = ComputeNavTime(node_state, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
							current_nav_time = FixTimeOffset(current_nav_time,13,12); // Update the NAV time according to the time offsets

							// ------------------------------------------------------------------------
							// Sergio on 07 Dec 2017: add CTS transmission time to spectrum utilization
							for(int c = current_left_channel; c <= current_right_channel; ++c){
								total_time_channel_busy_per_channel[c] = total_time_channel_busy_per_channel[c] + current_tx_duration;
							}
							// ------------------------------------------------------------------------

							time_to_trigger = SimTime() + SIFS;
							trigger_SIFS.Set(FixTimeOffset(time_to_trigger,13,12)); // triggers the SendResponsePacket() function after SIFS

							LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s SIFS will be triggered in %.12f\n",
								SimTime(), node_id, node_state, LOG_E14, LOG_LVL3,
								trigger_SIFS.GetTime());

							cts_notification = GenerateNotification(PACKET_TYPE_CTS, current_destination_id,
								notification.packet_id, notification.tx_info.num_packets_aggregated,
								notification.timestamp_generated, notification.tx_info.total_tx_power);

							cts_notification.tx_duration = current_tx_duration;

                            // Reset the flag that indicates whether the tx power changed or not
                            flag_change_in_tx_power = FALSE;


							// Workaround to solve the e->clock timer issue
							// (occurs when being in NAV and noticing a collision of two or more CTS frames)
							cts_notification.tx_info.preoccupancy_duration = time_rand_value;

						} else {
							// CANNOT START PACKET TX

							LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s CTS TX NOT POSSIBLE\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);

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
						LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION

					LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Still receiving packet #%d reception from N%d.\n",
							SimTime(), node_id, node_state, LOG_E15, LOG_LVL3, incoming_notification.packet_id,
							incoming_notification.source_id);
				}

				break;
			}


			/* STATE_RX_CTS:
			 * -
			 */
			case STATE_RX_CTS:{

				if(notification.destination_id == node_id){	// Node IS THE DESTINATION

					if(notification.packet_type == PACKET_TYPE_CTS){	// CTS packet transmission finished

						LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s CTS #%d reception from N%d is finished successfully.\n",
								SimTime(), node_id, node_state, LOG_E14, LOG_LVL3,
								notification.packet_id, notification.source_id);

						node_state = STATE_TX_DATA;

						// Compute the NAV time
						bits_ofdm_sym =  GetNumberSubcarriers(current_right_channel - current_left_channel +1) *
							Mcs_array::modulation_bits[notification.modulation_id-1] *
							Mcs_array::coding_rates[notification.modulation_id-1] *
							IEEE_AX_SU_SPATIAL_STREAMS;

						ComputeFramesDuration(&rts_duration, &cts_duration, &data_duration, &ack_duration,
							num_channels_tx, notification.modulation_id, notification.tx_info.num_packets_aggregated,
							frame_length, bits_ofdm_sym);

						limited_num_packets_aggregated = notification.tx_info.num_packets_aggregated;

						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s Transmitting DATA (N_agg = %d) in %d channels using modulation %d (%.0f bits per OFDM symbol ---> %.2f Mbps) \n",
							SimTime(), node_id, node_state, LOG_F04, LOG_LVL4, limited_num_packets_aggregated,
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
							total_time_channel_busy_per_channel[c] = total_time_channel_busy_per_channel[c] + current_tx_duration;
						}
						// ------------------------------------------------------------------------

						trigger_SIFS.Set(FixTimeOffset(time_to_trigger,13,12));

						LOGS(save_node_logs,node_logger.file,
							"%.15f;N%d;S%d;%s;%s SIFS will be triggered in %.12f\n",
							SimTime(), node_id, node_state, LOG_E14, LOG_LVL3,
							trigger_SIFS.GetTime());

						data_notification = GenerateNotification(PACKET_TYPE_DATA, current_destination_id,
								notification.packet_id, notification.tx_info.num_packets_aggregated,
								notification.timestamp_generated, current_tx_duration);

						// Reset the flag that indicates whether the tx power changed or not
                        flag_change_in_tx_power = FALSE;

						data_notification.tx_info.preoccupancy_duration = time_rand_value;



					} else {	// Other packet type transmission finished
						LOGS(save_node_logs,node_logger.file,
								"%.15f;N%d;S%d;%s;%s Unexpected packet type transmission finished!\n",
								SimTime(), node_id, node_state, LOG_D08, LOG_LVL4);
					}

				} else {	// Node IS NOT THE DESTINATION

					LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Still receiving packet #%d reception from N%d.\n",
						SimTime(), node_id, node_state, LOG_E15, LOG_LVL3, incoming_notification.packet_id,
						incoming_notification.source_id);
				}

				break;
			}

			case STATE_SLEEP:{
				// do nothing
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
	if (node_id == 0) {
		int num_nodes_transmitting = 0;
		for(int i = 0; i < total_nodes_number; ++i){
			if(nodes_transmitting[i] == TRUE){
				++ num_nodes_transmitting;
			}
		}
		// Check if nobody is transmitting
		if (num_nodes_transmitting == 0) {
			// If no one is transmitting, set the current SimTime() as the last time the channel has been seen idle
			last_time_channel_is_idle = SimTime();
			channel_idle = true;
		}
	}

	// LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s InportSomeNodeFinishTX() END",	SimTime(), node_id, node_state, LOG_E01, LOG_LVL1);
};

// Node::InportNackReceived — see methods/node_mcs_methods.h

// Node::InportMCSRequestReceived — see methods/node_mcs_methods.h

// Node::InportMCSResponseReceived — see methods/node_mcs_methods.h

// Node::InportNewPacketGenerated — see methods/node_mcs_methods.h

// Node::StartTransmission — see methods/node_packet_methods.h

/**
 * Helper for EndBackoff: injects burst packets into the buffer
 */
void Node :: InitiateBurstPackets(){

	// - Add another bunch of packets to the buffer if TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION
	//if(traffic_model == TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION && last_transmission_successful) {
	if(traffic_model == TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION) {

		// - Generate the aggregated frames to be sent in the next transmission
		for(int i = 0; i < limited_num_packets_aggregated; ++i){
			new_packet = null_notification;
			new_packet.timestamp_generated = SimTime();
			new_packet.packet_id = last_packet_generated_id;
			buffer.PutPacket(new_packet);
			++last_packet_generated_id;
		}

		// Set "last_transmission_successful" to 0 for the upcoming transmission
		last_transmission_successful = 0;

	}
}

/**
 * Helper for EndBackoff: sets the preoccupancy and toFinishTX triggers
 */
void Node :: ScheduleTransmission(){

        LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s -------------------------------------\n",
		SimTime(), node_id, node_state, LOG_D08, LOG_LVL2);
	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s TRANSMISSION #%d STARTED\n",
		SimTime(), node_id, node_state, LOG_D08, LOG_LVL2,
		rts_cts_sent);
	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s -------------------------------------\n",
		SimTime(), node_id, node_state, LOG_D08, LOG_LVL2);

	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Transmission of RTS #%d started\n",
		SimTime(), node_id, node_state, LOG_F04, LOG_LVL3, rts_notification.packet_id);

	// ------------------------------------------------------------------------
	// Sergio on 07 Dec 2017: add RTS transmission time to spectrum utilization
	for(int c = current_left_channel; c <= current_right_channel; ++c){
		total_time_channel_busy_per_channel[c] += current_tx_duration;
	}
	// ------------------------------------------------------------------------

	// Send RTS notification and trigger to finish transmission
	time_to_trigger = SimTime() + time_rand_value;
	trigger_preoccupancy.Set(FixTimeOffset(time_to_trigger,13,12));
	rts_notification.tx_info.preoccupancy_duration = time_rand_value;


	time_to_trigger = SimTime() + current_tx_duration;

//		LOGS(save_node_logs,node_logger.file,
//			"%.15f;N%d;S%d;%s;%s time_to_trigger = %.12f s - FixTimeOffset = %.12f s\n",
//			SimTime(), node_id, node_state, LOG_F04, LOG_LVL5,
//			time_to_trigger, FixTimeOffset(time_to_trigger,13,12));

	trigger_toFinishTX.Set(FixTimeOffset(time_to_trigger,13,12));
	++rts_cts_sent;
	++rts_cts_sent_per_sta[current_destination_id-node_id-1];
	trigger_start_backoff.Cancel();	// Safety instruction
}

/**
 * Helper for EndBackoff: selects destination, MCS, builds the RTS notification
 */
void Node :: PrepareNewTransmission(){

	// Identify the channel range to TX in depending on the channel bonding scheme and free channels
	int ix_mcs_per_node (current_destination_id - wlan.list_sta_id[0]);

	// Change to state "transmitting RTS"
	node_state = STATE_TX_RTS;

	// Get the transmission channels
	current_left_channel = GetFirstOrLastTrueElemOfArray(FIRST_TRUE_IN_ARRAY,
		channels_for_tx, NUM_CHANNELS_KOMONDOR);
	current_right_channel = GetFirstOrLastTrueElemOfArray(LAST_TRUE_IN_ARRAY,
		channels_for_tx, NUM_CHANNELS_KOMONDOR);

	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Transmission is possible in range: %d - %d\n",
		SimTime(), node_id, node_state, LOG_F04, LOG_LVL3, current_left_channel, current_right_channel);

	int previous_num_channels = num_channels_tx;
	num_channels_tx = current_right_channel - current_left_channel + 1;
	++num_trials_tx_per_num_channels[(int)log2(num_channels_tx)];
	int ix_num_channels_used (log2(num_channels_tx));

	if(previous_num_channels != num_channels_tx) flag_change_in_tx_power = TRUE;

	// Get the current modulation according to the channels selected for transmission
	//int ix_mcs_per_node (current_destination_id - wlan.list_sta_id[0]);
	current_modulation = mcs_per_node[ix_mcs_per_node][ix_num_channels_used];

	// ********************************************************
	// Flexible packet aggregation

	//  - Delete all the aggregated frames contained in the ACKed packet
	if (limited_num_packets_aggregated > 0) {
		for(int i = 0; i < limited_num_packets_aggregated; ++i){
			buffer.DelFirstPacket();
		}
	}
	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Data packet/s removed from buffer (queue: %d/%d).\n",
		SimTime(), node_id, node_state, LOG_E14, LOG_LVL3,
		buffer.QueueSize(), PACKET_BUFFER_SIZE);

	// - Number of packets to be aggregated: min(current buffer size, max num packets aggregated)
	if(buffer.QueueSize() > max_num_packets_aggregated || traffic_model == TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION){
		current_num_packets_aggregated = max_num_packets_aggregated;
	} else {
		current_num_packets_aggregated = buffer.QueueSize();
	}

	// data rate depending on CB and streams: Nsc * ym * yc * SUSS
	bits_ofdm_sym =  GetNumberSubcarriers(num_channels_tx) *
		Mcs_array::modulation_bits[current_modulation-1] *
		Mcs_array::coding_rates[current_modulation-1] *
		IEEE_AX_SU_SPATIAL_STREAMS;

	// Update the number of packets to aggregate (just in case that the max PPDU is exceeded with the current MCS)
	limited_num_packets_aggregated = FindMaximumPacketsAggregated
		(current_num_packets_aggregated, frame_length, bits_ofdm_sym);

	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Num. of packets to aggregate: %d/%d (last_transmission_successful=%d)\n",
		SimTime(), node_id, node_state, LOG_F04, LOG_LVL4,
		limited_num_packets_aggregated, max_num_packets_aggregated, last_transmission_successful);

	// TODO: ADD TRANSMISSION DELAY


	// ********************************************************
	InitiateBurstPackets();
	// ********************************************************

	// Compute all packets durations (RTS, CTS, DATA and ACK) and NAV time
	ComputeFramesDuration(&rts_duration, &cts_duration, &data_duration, &ack_duration,
		num_channels_tx, current_modulation, limited_num_packets_aggregated, frame_length, bits_ofdm_sym);


	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Transmitting (N_agg = %d) in %d channels using modulation %d (%.0f bits per OFDM symbol ---> %.2f Mbps) \n",
		SimTime(), node_id, node_state, LOG_F04, LOG_LVL4, limited_num_packets_aggregated,
		(int) pow(2, ix_num_channels_used), current_modulation, bits_ofdm_sym,
		bits_ofdm_sym/IEEE_AX_OFDM_SYMBOL_GI32_DURATION * pow(10,-6));

	if(spatial_reuse_enabled && txop_sr_identified) {
		LOGS(save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Using tx power = %f dBm \n",
			SimTime(), node_id, node_state, LOG_F04, LOG_LVL4,
			ConvertPower(PW_TO_DBM, current_tx_power_sr));
	} else {
		LOGS(save_node_logs,node_logger.file,
			"%.15f;N%d;S%d;%s;%s Using tx power = %f dBm \n",
			SimTime(), node_id, node_state, LOG_F04, LOG_LVL4,
			ConvertPower(PW_TO_DBM, current_tx_power));
	}

	current_tx_duration = rts_duration;

	// Compute the NAV time
	current_nav_time = ComputeNavTime(node_state, rts_duration, cts_duration, data_duration, ack_duration, SIFS);
	current_nav_time = FixTimeOffset(current_nav_time,13,12); // Update the NAV time according to the time offsets

//		LOGS(save_node_logs,node_logger.file,
//			"%.15f;N%d;S%d;%s;%s RTS duration: %.12f s - NAV duration = %.12f s\n",
//			SimTime(), node_id, node_state, LOG_F04, LOG_LVL5,
//			rts_duration, current_nav_time);

//		printf("current_nav_time = %f\n",current_nav_time);

	/*
	 * IMPORTANT: to avoid synchronization problems in Slotted BO, we put a
	 * random time epsilon trigger before sending the channel occupancy notification.
	 * E.g. If two APs A and B finish their BO at the very same time, both A and B will
	 * find the channel free and will pick the corresponding channels accordingly. This
	 * way we are able to capture slotted BO collisions.
	 */
	time_rand_value = 0;
	int rand_number (2 + rand() % (MAX_NUM_RAND_TIME-2));	// in [2, MAX_NUM_RAND_TIME]
	time_rand_value = (double) rand_number * MAX_DIFFERENCE_SAME_TIME/MAX_NUM_RAND_TIME; // in [FEMTO_SECOND, MAX_DIFFERENCE_SAME_TIME]
	// Sergio on 28/09/2017
	// time_rand_value = RoundToDigits(time_rand_value, 15);
	time_rand_value = FixTimeOffset(time_rand_value,13,12);
	current_nav_time = current_nav_time - time_rand_value;
//			LOGS(save_node_logs,node_logger.file,
//				"%.15f;N%d;S%d;%s;%s time_rand_value = %.12f s - corrected NAV time = %.12f s\n",
//				SimTime(), node_id, node_state, LOG_F04, LOG_LVL5,
//				time_rand_value, current_nav_time);

	// Generate the RTS notification
	Notification first_packet_buffer = buffer.GetFirstPacket();

	rts_notification = GenerateNotification(PACKET_TYPE_RTS, current_destination_id,
		first_packet_buffer.packet_id, limited_num_packets_aggregated,
		first_packet_buffer.timestamp_generated, current_tx_duration);

        // Reset the flag that indicates whether the tx power changed or not
        flag_change_in_tx_power = FALSE;

	ScheduleTransmission();
}

/**
 * Called when backoff finishes (triggered-based operation)
 */
void Node :: EndBackoff(trigger_t &){

	LOGS(save_node_logs,node_logger.file, "\n----------------------------------------------------------\n");
	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s EndBackoff()\n",
			SimTime(), node_id, node_state, LOG_F00, LOG_LVL1);

	/* ****************************************
	/* SPATIAL REUSE OPERATION
	 *
	 *  Determine the parameters to be used according
	 *  to the SR operation (in case of having detected a TXOP).
	 *
	 * *****************************************/
	if (spatial_reuse_enabled) {
		LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s txop_sr_identified = %d\n",
			SimTime(), node_id, node_state, LOG_F00, LOG_LVL1, txop_sr_identified);
		flag_change_in_tx_power = TRUE;
		if(txop_sr_identified) {
		// Apply the transmission power limitation
		current_tx_power_sr = next_tx_power_limit;
		// In order to request a new MCS (the tx power may have changed)
		for(int n = 0; n < wlan.num_stas; n++) {
			change_modulation_flag[n] = TRUE;
		}
	} else {
		// Use default values
	}
	if(save_node_logs) fprintf(node_logger.file, "%.15f;N%d;S%d;%s;%s Intended values for the next TX: "
		"pd = %f dBm, Tx Power = %f dBm\n", SimTime(), node_id, node_state, LOG_F02, LOG_LVL3,
		ConvertPower(PW_TO_DBM, current_obss_pd_threshold), ConvertPower(PW_TO_DBM, current_tx_power_sr));
	}
	/* **************************************** */

	// Sergio on 26th June 2018:
	// - Compute average BO waiting time
	sum_waiting_time = sum_waiting_time + SimTime() - timestamp_new_trial_started;
	++num_average_waiting_time_measurements;

	// Update the performance_report
	performance_report.sum_waiting_time += SimTime() - timestamp_new_trial_started;
	++performance_report.num_waiting_time_measurements;
	if (SimTime() - timestamp_new_trial_started > performance_report.max_waiting_time) {
		performance_report.max_waiting_time = SimTime() - timestamp_new_trial_started;
	}
	if (SimTime() - timestamp_new_trial_started < performance_report.min_waiting_time) {
		performance_report.min_waiting_time = SimTime() - timestamp_new_trial_started;
	}

	// Measurements in the last part of the simulation
	if (SimTime() > (simulation_time_komondor - last_measurements_window)) {
		last_sum_waiting_time = last_sum_waiting_time + SimTime() - timestamp_new_trial_started;
		++last_num_average_waiting_time_measurements;
	}

	// Cancel NAV TO trigger
	trigger_NAV_timeout.Cancel();

	// Cancel trigger for safety
	trigger_recover_cts_timeout.Cancel();

	// Check if MCS already defined for every potential receiver
	for(int n = 0; n < wlan.num_stas; ++n) {
		current_destination_id = wlan.list_sta_id[n];
		// Receive the possible MCS to be used for each number of channels
		if (change_modulation_flag[n]) {
			LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Requesting MCS to N%d\n",
				SimTime(), node_id, node_state, LOG_F02, LOG_LVL2, current_destination_id);
			RequestMCS();
		}
	}

	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Allowed LEFT/RIGHT: %d - %d\n",
		SimTime(), node_id, node_state, LOG_F02, LOG_LVL2, min_channel_allowed, max_channel_allowed);

	// Pick one receiver from the pool of potential receivers
	SelectDestination();

	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Trying to start TX to STA N%d\n",
		SimTime(), node_id, node_state, LOG_F02, LOG_LVL2, current_destination_id);

	// Identify free channels
	++num_tx_init_tried;

	if (spatial_reuse_enabled && txop_sr_identified) {
		GetChannelOccupancyByCCA(current_primary_channel, pifs_activated, channels_free, min_channel_allowed,
			max_channel_allowed, &channel_power, current_obss_pd_threshold, timestampt_channel_becomes_free, SimTime(), PIFS);
	} else {
		GetChannelOccupancyByCCA(current_primary_channel, pifs_activated, channels_free, min_channel_allowed,
			max_channel_allowed, &channel_power, current_pd, timestampt_channel_becomes_free, SimTime(), PIFS);
	}

	LOGS(save_node_logs,node_logger.file,
		"%.15f;N%d;S%d;%s;%s Power sensed per channel [dBm]: ",
		SimTime(), node_id, node_state, LOG_E18, LOG_LVL3);

	PrintOrWriteChannelPower(WRITE_LOG, save_node_logs, print_node_logs, node_logger,
		&channel_power);

	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Channels founds free (mind PIFS if activated): ",
		SimTime(), node_id, node_state, LOG_F02, LOG_LVL3);

	PrintOrWriteChannelsFree(WRITE_LOG, save_node_logs, print_node_logs, node_logger,
		channels_free);

	GetTxChannels(channels_for_tx, current_dcb_policy, channels_free,
			min_channel_allowed, max_channel_allowed, current_primary_channel,
			NUM_CHANNELS_KOMONDOR, &channel_power, channel_aggregation_cca_model);

	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s Channels for transmitting: ",
		SimTime(), node_id, node_state, LOG_F02, LOG_LVL2);

	PrintOrWriteChannelForTx(WRITE_LOG, save_node_logs, print_node_logs, node_logger,
		channels_for_tx);

	// Act according to possible (not possible) transmission
	if(channels_for_tx[0] != TX_NOT_POSSIBLE) {
		//&& current_modulation != MODULATION_FORBIDDEN){	// Transmission IS POSSIBLE
		PrepareNewTransmission();

	} else {	// Transmission IS NOT POSSIBLE, compute a new backoff.
		AbortRtsTransmission();
	}
	// LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s EndBackoff() END\n", SimTime(), node_id, node_state, LOG_F01, LOG_LVL1);
};


/**
 * Called when own transmission is finished (triggered-based operation)
 */
void Node :: MyTxFinished(trigger_t &){

//	LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s MyTxFinished()\n",
//			SimTime(), node_id, node_state, LOG_G00, LOG_LVL1);

	switch(node_state){

		case STATE_TX_RTS:{		// Wait for CTS

			// Set CTS timeout and change state to STATE_WAIT_CTS
			Notification notification = GenerateNotification(PACKET_TYPE_RTS, current_destination_id,
				rts_notification.packet_id, limited_num_packets_aggregated,
				rts_notification.timestamp_generated, TX_DURATION_NONE);

			outportSelfFinishTX(notification);

			// Sergio on 2018/06/22
			// - Time out should be equal to the collision time, i,e., T_c = T_RTS + SIFS + T_CTS minus T_RTS (already txed)

			// time_to_trigger = SimTime() + SIFS + notification.tx_info.cts_duration + DIFS;
			time_to_trigger = SimTime() + SIFS + notification.tx_info.cts_duration;

			trigger_CTS_timeout.Set(FixTimeOffset(time_to_trigger,13,12));

			node_state = STATE_WAIT_CTS;

			LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s RTS #%d tx finished. Waiting for CTS until %.12f\n",
				SimTime(), node_id, node_state, LOG_G00, LOG_LVL2,
				notification.packet_id, trigger_CTS_timeout.GetTime());

			break;
		}

		case STATE_TX_CTS:{		// Wait for Data

			Notification notification = GenerateNotification(PACKET_TYPE_CTS, current_destination_id,
				cts_notification.packet_id, cts_notification.tx_info.num_packets_aggregated,
				cts_notification.timestamp_generated, TX_DURATION_NONE);

			outportSelfFinishTX(notification);

			// Set CTS timeout and change state to STATE_WAIT_DATA
			time_to_trigger = SimTime() + SIFS + TIME_OUT_EXTRA_TIME;
			trigger_DATA_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
			node_state = STATE_WAIT_DATA;

			LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s CTS %d tx finished. Waiting for DATA...\n",
				SimTime(), node_id, node_state, LOG_G00, LOG_LVL2, notification.packet_id);

			break;
		}

		case STATE_TX_DATA:{ 	// Change state to STATE_WAIT_ACK

			Notification notification = GenerateNotification(PACKET_TYPE_DATA, current_destination_id,
				data_notification.packet_id, data_notification.tx_info.num_packets_aggregated,
				data_notification.timestamp_generated, TX_DURATION_NONE);

			outportSelfFinishTX(notification);

			// Set ACK timeout and change state to STATE_WAIT_ACK
			time_to_trigger = SimTime() + SIFS + TIME_OUT_EXTRA_TIME;
			trigger_ACK_timeout.Set(FixTimeOffset(time_to_trigger,13,12));
			node_state = STATE_WAIT_ACK;

			LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s DATA %d tx finished. Waiting for ACK...\n",
				SimTime(), node_id, node_state, LOG_G00, LOG_LVL2, notification.packet_id);

			break;
		}

		case STATE_TX_ACK:{		// Restart node

			Notification notification = GenerateNotification(PACKET_TYPE_ACK, current_destination_id,
				ack_notification.packet_id, ack_notification.tx_info.num_packets_aggregated,
				ack_notification.timestamp_generated, TX_DURATION_NONE);

			outportSelfFinishTX(notification);

			LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;%s ACK %d tx finished. Restarting node...\n",
				SimTime(), node_id, node_state, LOG_G00, LOG_LVL2, notification.packet_id);

			RestartNode(FALSE);

			break;
		}

		default:
			break;
	}

    // Reset the flag that indicates whether the tx power changed or not
    flag_change_in_tx_power = FALSE;

	// LOGS(save_node_logs,node_logger.file, "%.15f;N%d;S%d;%s;  MyTxFinished()\n", SimTime(), node_id, node_state, LOG_G01, LOG_LVL1);
};

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

// Node::AbortRtsTransmission — see methods/node_backoff_methods.h

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
	nack_activated = FALSE;
	/*
	 * END OF HARDCODED INITIALIZATION
	 */

	current_max_bandwidth = max_channel_allowed - min_channel_allowed + 1;

	current_sinr = 0;
	max_pw_interference = 0;
	rts_lost_slotted_bo = 0;
	last_packet_generated_id = 0;

	num_delay_measurements = 0;
	sum_delays = 0;
	average_delay = -1;
	num_packets_generated = 0;
	num_packets_dropped = 0;

	num_channels_tx = 0;

	flag_measure_rho = TRUE;
	delta_measure_rho = 0.00001;
	num_measures_rho = 0;
	num_measures_rho_accomplished = 0;
	average_utilization = 0;
	average_rho = 0;
	bandwidth_used_txing = 0;

	num_measures_utilization = 0;
	num_measures_buffer_with_packets = 0;
	generation_drop_ratio = 0;

	current_traffic_type = -1;

	// Output file - logger
	node_logger.save_logs = save_node_logs;
	node_logger.file = NULL;	// File opened in Start() after InitializeVariables() returns

	// Arrays and other
	channel_power = new double[NUM_CHANNELS_KOMONDOR];
	num_channels_allowed = (max_channel_allowed - min_channel_allowed + 1);
	total_time_transmitting_per_channel = new double[NUM_CHANNELS_KOMONDOR];
	last_total_time_transmitting_per_channel = new double[NUM_CHANNELS_KOMONDOR];
	channels_free = new int[NUM_CHANNELS_KOMONDOR];
	channels_for_tx = new int[NUM_CHANNELS_KOMONDOR];
	total_time_lost_per_channel = new double[NUM_CHANNELS_KOMONDOR];
	last_total_time_lost_per_channel = new double[NUM_CHANNELS_KOMONDOR];
	total_time_channel_busy_per_channel = new double[NUM_CHANNELS_KOMONDOR];
	timestampt_channel_becomes_free = new double[NUM_CHANNELS_KOMONDOR];
	num_trials_tx_per_num_channels = new int[NUM_CHANNELS_KOMONDOR];
	for(int i = 0; i < NUM_CHANNELS_KOMONDOR; ++i){
		channel_power[i] = 0;
		total_time_transmitting_per_channel[i] = 0;
		last_total_time_transmitting_per_channel[i] = 0;
		channels_free[i] = FALSE;
		channels_for_tx[i] = FALSE;
		total_time_lost_per_channel[i] = 0;
		last_total_time_lost_per_channel[i] = 0;
		timestampt_channel_becomes_free[i] = 0;
		num_trials_tx_per_num_channels[i] = 0;
		total_time_channel_busy_per_channel[i] = 0;
	}

	total_time_transmitting_in_num_channels = new double[NUM_CHANNELS_KOMONDOR];
	total_time_lost_in_num_channels = new double[NUM_CHANNELS_KOMONDOR];
	for(int i = 0; i < NUM_CHANNELS_KOMONDOR; ++i){
		total_time_transmitting_in_num_channels[i] = 0;
		total_time_lost_in_num_channels[i] = 0;
	}

	nodes_transmitting = new int[total_nodes_number];
//	// List of hidden nodes (1 indicates hidden nodes, 0 indicates the opposite)
//	hidden_nodes_list = new int[total_nodes_number];
//	// Counter for the times a node was implied in a collision by hidden node
//	potential_hidden_nodes = new int[total_nodes_number];
	for(int n = 0; n < total_nodes_number; ++n){
		nodes_transmitting[n] = FALSE;
//		hidden_nodes_list[n] = FALSE;
//		potential_hidden_nodes[n] = 0;
	}

	power_received_per_node.clear();

//	potential_hidden_nodes[node_id] = -1; // To indicate that the node cannot be hidden from itself
	nacks_received = new int[NUM_PACKET_LOST_REASONS];
	for(int i = 0; i < NUM_PACKET_LOST_REASONS; ++i){
		nacks_received[i] = 0;
	}

	// Rest of variables
	power_rx_interest = 0;
	progress_bar_counter = 0;
	current_left_channel =  min_channel_allowed;
	current_right_channel = max_channel_allowed;
	sum_waiting_time = 0;
	timestamp_new_trial_started = 0;
	num_average_waiting_time_measurements = 0;
	expected_backoff = 0;
	remaining_backoff = 0;
	num_new_backoff_computations = 0;
	last_transmission_successful = 0;

	data_packets_acked = 0;
	data_frames_acked = 0;

	node_state = STATE_SENSING;
	current_modulation = 1;
	packet_id = 0;

	//CHANNEL ACCESSs
	current_cw_min = cw_min_default; // Initialize the CW min
	current_cw_max = cw_max_default; // Initialize the CW max
	cw_stage_current = 0;
	//- Tokenized backoffF
	if (backoff_type == BACKOFF_TOKENIZED){
		token_status = node_id;
		distance_to_token = 0;
		token_order_list[node_id] = DEVICE_ACTIVE_FOR_TOKEN;
	}
	//-  Deterministic backoff
	num_bo_interruptions = 0;
	base_backoff_deterministic = DEFAULT_BASE_BACKOFF_DETERMINISTIC;
	deterministic_bo_active = 0;

	if(node_type == NODE_TYPE_AP) {
		node_is_transmitter = TRUE;
		remaining_backoff = ComputeBackoff(current_cw_min, current_cw_max, backoff_type,
				current_traffic_type, deterministic_bo_active, num_bo_interruptions, base_backoff_deterministic, -1);
		previous_backoff = remaining_backoff;
		expected_backoff += remaining_backoff;
		num_new_backoff_computations++;
		// - compute average waiting time to access the channel
		timestamp_new_trial_started = SimTime();
	} else {
		node_is_transmitter = FALSE;
	}

	current_tx_power = tx_power_default;
	current_pd = sensitivity_default;
	channel_max_intereference = current_primary_channel;

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
	data_packets_sent = 0;
	rts_cts_sent = 0;
	throughput = 0;
	throughput_loss = 0;
	data_packets_lost = 0;
	rts_cts_lost = 0;
	num_tx_init_not_possible = 0;
	num_tx_init_tried = 0;
	last_throughput = 0;
	last_average_delay = -1;
	last_data_frames_acked = 0;
	last_sum_delays = 0;
	last_num_delay_measurements = 0;

	last_measurements_window = DEFAULT_LAST_MEASUREMENTS_WINDOW;	// Time in seconds of the last performance observation window

	throughput_per_sta = new double[wlan.num_stas];
	data_packets_sent_per_sta = new int[wlan.num_stas];
	rts_cts_sent_per_sta = new int[wlan.num_stas];
	data_packets_lost_per_sta = new int[wlan.num_stas];
	rts_cts_lost_per_sta = new int[wlan.num_stas];
	data_packets_acked_per_sta = new int[wlan.num_stas];
	data_frames_acked_per_sta = new int[wlan.num_stas];

	for(int i = 0; i < wlan.num_stas; ++i){
		throughput_per_sta[i] = 0;
		data_packets_sent_per_sta[i] = 0;
		rts_cts_sent_per_sta[i] = 0;
		data_packets_lost_per_sta[i] = 0;
		rts_cts_lost_per_sta[i] = 0;
		data_packets_acked_per_sta[i] = 0;
		data_frames_acked_per_sta[i] = 0;
	}

	performance_report.SetSizeOfChannelLists(NUM_CHANNELS_KOMONDOR);
	performance_report.SetSizeOfRssiList(total_wlans_number);

	performance_report.SetSizeOfRssiPerStaList(wlan.num_stas);

	// Measurements to be sent to agents
	RestartPerformanceMetrics(&performance_report, 0, num_channels_allowed);
	performance_report.SetSizeOfRssiList(total_wlans_number);

	flag_apply_new_configuration = FALSE;

	// Channel idle measurement
	sum_time_channel_idle = 0;
	last_time_channel_is_idle = 0;

	/******************************
	 *  SPATIAL REUSE
	 *****************************/
	// Detect whether the SR is enabled or not
	if (node_type == NODE_TYPE_AP && bss_color >= 0) {
		spatial_reuse_enabled = TRUE;
	} else {
		spatial_reuse_enabled = FALSE;
	}
	// In case of being an STA, request the SR configuration to the AP
	if (node_type == NODE_TYPE_STA) {
		outportRequestSpatialReuseConfiguration();
	}
	// Initialize the TXOP identified to false
	txop_sr_identified = FALSE;
	// Initialize the type of ongoing transmissions to 0
	for (int i = 0; i < 3; i ++) {
		type_ongoing_transmissions_sr[i] = 0;
	}
	next_pd_spatial_reuse = current_pd;
	/*****************************/

	if(node_type == NODE_TYPE_AP) {
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
