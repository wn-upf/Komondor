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
 * notification.h: this file defines a NOTIFICATION and provides basic displaying methods
 */

#ifndef _AUX_NOTIFICATION_
#define _AUX_NOTIFICATION_

// Notification specific info (may be not checked by the other nodes)
struct TxInfo
{

	int num_packets_aggregated;				///> Number of frames aggregated
	int *list_id_aggregated;				///> List of frame IDs aggregated
	double *timestamp_frames_aggregated;	///> List of timestamps of the frames aggregated

	// For RTS/CTS management
	double data_duration;		///> Duration of the data packet
	double ack_duration;		///> Duration of the ACK packet
	double rts_duration;		///> Duration of the RTS packet
	double cts_duration;		///> Duration of the CTS packet

	double preoccupancy_duration;	///>
	double total_tx_power;			///> Transmission power [pW]
	double tx_power;				///> Transmission power per 20 MHz channel [pW]
	double tx_gain;					///> Transmission gain [linear ratio]
	double pd;						///> PD threshold in [pW]
	double bits_ofdm_sym; 			///> Bits per OFDM symbol
	double data_rate; 				///> Rate at which data is transmitted
	int modulation_schemes[4];		///> Modulation scheme used
	double x;						///> X position of source node
	double y;						///> Y position of source node
	double z;						///> Z position of source node
	double nav_time;				///> RTS/CTS NAV time

	bool flag_change_in_tx_power;	///> Flag to indicate whether the transmission power was changed (in order to recompute arrays)

	// Spatial Reuse
	int bss_color;				///> BSS color
	int srg;					///> Spatial Reuse Group
	bool txop_sr_identified;	///> Boolean indicating whether an SR-based TXOP has been detected or not

	/**
	 * Function to print the transmission information
	 * @param "packet_id" [type int]: identifier of the packet
	 * @param "destination_id" [type int]: identifier of the destination node
	 * @param "tx_duration" [type double]: transmission duration
	 */
	void PrintTxInfo(int packet_id, int destination_id, double tx_duration){
		printf("packet_id = %d - destination_id = %d - tx_duration = %f - tx_power = %f pw"
			" - position = (%.2f, %.2f, %.2f)\n",
			packet_id, destination_id, tx_duration, tx_power, x, y, z);
	}

	/**
	 * Set the size of the array list_id_aggregated
	 * @param "num_packets_aggregated" [type int]: number of packets aggregated
	 */
	void SetSizeOfIdsAggregatedArray(int num_packets_aggregated){
		list_id_aggregated = new int[num_packets_aggregated];
		for(int t = 0; t < num_packets_aggregated; ++t){
			list_id_aggregated[t] = 0;
		}
	}

	/**
	 * Set the size of the array timestamp_frames_aggregated
	 * @param "num_packets_aggregated" [type int]: number of packets aggregated
	 */
	void SetSizeOfTimestampAggregatedArray(int num_packets_aggregated){
		timestamp_frames_aggregated = new double[num_packets_aggregated];
		for(int t = 0; t < num_packets_aggregated; ++t){
			timestamp_frames_aggregated[t] = 0;
		}
	}

	/**
	 * Set the size of the array modulation_schemes
	 * @param "channels_groups" [type int]: groups of channels that can be used
	 */
	void SetSizeOfMCS(int channels_groups){
		//modulation_schemes = new int[channels_groups];
		for(int s = 0; s < channels_groups; ++s){
			modulation_schemes[s] = MODULATION_NONE;
		}
	}

};

// Notification info
struct Notification
{
	// Always read in destination
	int packet_id;				///> Packet identifier of the first frame
	int packet_type;			///> Type of packet: Data, ACK, etc.
	int source_id;				///> Node id of the source
	int destination_id;			///> Destination node of the transmission
	double tx_duration;			///> Duration of the transmission
	int left_channel;			///> Left channel used in the transmission
	int right_channel;			///> Right channel used in the transmission
	int frame_length;			///> Size of the packet to transmit
	int modulation_id;			///> Modulation being used during the transmission
	double timestamp;			///> Timestamp when notification is sent
	double timestamp_generated;	///> Timestamp when notification was generated
	TxInfo tx_info; 			///> Specific transmission info (may not be checked by the others nodes)

	/**
	 * Function to print the parameters of a notification
	 */
	void PrintNotification(void){
		printf("source_id = %d - packet_type = %d - left_channel = %d - right_channel = %d - pkt_length = %d -",
			source_id, packet_type, left_channel, right_channel, frame_length);
		printf("tx_info: ");
		tx_info.PrintTxInfo(packet_id, destination_id, tx_duration);
	}

};

#endif
