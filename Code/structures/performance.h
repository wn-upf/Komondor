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
 * performance_metrics.h: this file defines the performance object used for statistics and the agents operation
 */

#ifndef _AUX_PERFORMANCE_
#define _AUX_PERFORMANCE_

struct Performance
{

	// Time of the last measurement
	double timestamp;		        ///> Timestamp of the last measurement
	double sum_time_channel_idle;	///> Sum time the channel has been idle
	double last_measurements_window;

	// Throughput statistics
	double throughput;				///> Throughput
	double throughput_loss;			///> Throughput lost
	double max_bound_throughput;	///> Maximum bound throughput (if no one transmits)
	double last_throughput;

	// Frames statistics
	int data_packets_acked;			///> Number of data packets acknowledged
	int data_frames_acked;			///> Number of frames acknowledged
	int data_packets_sent;			///> Number of data packets sent
	int data_packets_lost;			///> Number of data packets lost
	int rts_cts_sent;				///> Number of RTS/CTS packets sent
	int rts_cts_lost;				///> Number of RTS/CTS packets lost
	int rts_lost_slotted_bo;		///> Number of RTS/CTS packets lost by slotted BO

	// Buffer statistics
	int num_packets_generated;		///> Number of packets generated
	int num_packets_dropped;		///> Number of packets dropped
	int num_delay_measurements;		///> Number of delay measurements
	double sum_delays;				///> Sum of the delays
	double average_delay;			///> Average delay
	double max_delay;				///> Maximum delay
	double min_delay;				///> Minimum delay
	double average_rho;				///> Average rho
	double average_utilization;		///> Average utilization
	double generation_drop_ratio;	///> Average drop ratio
	double last_average_delay;
	double last_average_access_delay;
	double sum_waiting_time; 			//> Sum of channel access delays (contention)
	int num_waiting_time_measurements;	///> Number of contention delay measurements
	double max_waiting_time;			///> Maximum waiting time
	double min_waiting_time;			///> Minimum waiting time

	// Environment statistics
	double *max_received_power_in_ap_per_wlan;
    double *rssi_list;					///> List of RSSI received from each other WLAN
    double *received_power_array;		///> Array containing the power received by each node
    double total_channel_occupancy;
    double successful_channel_occupancy;

	// Channel occupancy
	double expected_backoff;							///> Expected BO
	int num_new_backoff_computations;					///> Number of times a new backoff was computed
	int *num_trials_tx_per_num_channels;				///> Number of attempts for transmitting to each number of channels
	double average_waiting_time;						///> Average waiting time
	double bandwidth_used_txing;						///> Bandwidth used during transmissions
	double *total_time_transmitting_per_channel;		///> Total time transmitting in each channel
	double *total_time_transmitting_in_num_channels;	///> Total time transmitting per each channel width
	double *total_time_lost_per_channel;				///> Total time lost in each channel (e.g. collisions)
	double *total_time_lost_in_num_channels;			///> Total time lost per each channel width
	double *total_time_channel_busy_per_channel;		///> Total time the channel is busy per each channel
	double time_in_nav;									///> Time spent in NAV state
	double *last_total_time_transmitting_per_channel;
	double *last_total_time_lost_per_channel;


	// Per-STA statistics
    int num_stas;                   ///> Number of STAs in the BSS
	double *throughput_per_sta;			///> Array containing the throughput delivered to each STA
	int *data_packets_sent_per_sta;		///> Array containing the number of packets sent to each STA
	int *rts_cts_sent_per_sta;			///> Array containing the number of RTS/CTS sent to each STA
	int *data_packets_lost_per_sta;		///> Array containing the number of packets lost when sent to each STA
	int *rts_cts_lost_per_sta;			///> Array containing the number of RTS/CTS lost when sent to each STA
	int *data_packets_acked_per_sta;	///> Array containing the number of acknowledged packets by each STA
	int *data_frames_acked_per_sta;		///> Array containing the number of acknowledged frames by each STA
    double *rssi_list_per_sta;			///> List of RSSI perceived by each STA in the WLAN

	// Other
	int num_tx_init_tried;				///> Number of transmissions initiated
	int num_tx_init_not_possible;		///> Number of initiated transmissions that were not possible
	double prob_slotted_bo_collision;	///> Probability of suffering collisions by slotted BO

	/**
	 * Set the size of the arrays in which channel-related information is stored
	 * @param "total_channels_number" [type int]: total number of channels
	 */
	void SetSizeOfChannelLists(int total_channels_number){
		num_trials_tx_per_num_channels = new int[total_channels_number];
		total_time_transmitting_per_channel = new double[total_channels_number];
		total_time_transmitting_in_num_channels = new double[total_channels_number];
		total_time_lost_per_channel = new double[total_channels_number];
		total_time_lost_in_num_channels = new double[total_channels_number];
		total_time_channel_busy_per_channel = new double[total_channels_number];
		for(int i = 0; i < total_channels_number; ++i){
			num_trials_tx_per_num_channels[i] = 0;
			total_time_transmitting_per_channel[i] = 0;
			total_time_transmitting_in_num_channels[i] = 0;
			total_time_lost_per_channel[i] = 0;
			total_time_lost_in_num_channels[i] = 0;
			total_time_channel_busy_per_channel[i] = 0;
		}
	}

	/**
	 * Set the size of the arrays in which STA-related information is stored
	 * @param "num_stas" [type int]: total number of STAs in a WLAN
	 */
	void SetSizeOfStaList(int num_stas){
		throughput_per_sta = new double[num_stas];
		data_packets_sent_per_sta = new int[num_stas];
		rts_cts_sent_per_sta = new int[num_stas];
		data_packets_lost_per_sta = new int[num_stas];
		rts_cts_lost_per_sta = new int[num_stas];
		data_packets_acked_per_sta = new int[num_stas];
		data_frames_acked_per_sta = new int[num_stas];
		for(int i = 0; i < num_stas; ++i){
			throughput_per_sta[i] = 0;
			data_packets_sent_per_sta[i] = 0;
			rts_cts_sent_per_sta[i] = 0;
			data_packets_lost_per_sta[i] = 0;
			rts_cts_lost_per_sta[i] = 0;
			data_packets_acked_per_sta[i] = 0;
			data_frames_acked_per_sta[i] = 0;
		}
	}

	/**
	 * Set the size of the array RSSI list
	 * @param "total_wlans_number" [type int]: total number of WLANs
	 */
	void SetSizeOfRssiList(int total_wlans_number){
		rssi_list = new double[total_wlans_number];
		max_received_power_in_ap_per_wlan = new double[total_wlans_number];
		for(int i = 0; i < total_wlans_number; ++i){
			rssi_list[i] = 0;
			max_received_power_in_ap_per_wlan[i] = 0;
		}
	}

	/**
	 * Set the size of the array containing the received power from each node
	 * @param "total_nodes_number" [type int]: total number of nodes
	 */
	void SetSizeOfRxPowerList(int total_nodes_number){
		received_power_array = new double[total_nodes_number];
		for(int i = 0; i < total_nodes_number; ++i){
			received_power_array[i] = 0;
		}
	}

	/**
	 * Set the size of the array containing the RSSI in each STA from the same WLAN
	 * @param "num_stas" [type int]: number of STAs in the WLAN
	 */
	void SetSizeOfRssiPerStaList(int num_stas){
		rssi_list_per_sta = new double[num_stas];
		for(int i = 0; i < num_stas; ++i){
			rssi_list_per_sta[i] = 0;
		}
	}

};

#endif
