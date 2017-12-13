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
 * File description: Notifications are structures containing information
 * corresponding to communications among nodes such as real packets (e.g.
 * data packets or ACKS) or logical events (e.g. MCS negotiation response).
 *
 * - Bla bla bla...
 */


#ifndef _AUX_NOTIFICATION_
#define _AUX_NOTIFICATION_

// Notification specific info (may be not checked by the other nodes)
struct TxInfo
{
	int packet_id;				// Packet identifier
	int destination_id;			// Destination node of the transmission
	double tx_duration;			// Duration of the transmission

	// For RTS/CTS management
	double data_duration;
	double ack_duration;
	double rts_duration;
	double cts_duration;

	double tx_power;			// Transmission power in [pW]
	double tx_gain;				// Transmission gain [linear ratio]
	double data_rate; 			// Rate at which data is transmitted
	int *modulation_schemes;	// Modulation scheme used
	double x;						// X position of source node
	double y;						// Y position of source node
	double z;						// Z position of source node
	double nav_time;			// RTS/CTS NAV time

	void PrintTxInfo(void){
		printf("packet_id = %d - destination_id = %d - tx_duration = %f - tx_power = %f pw"
				" - position = (%.2f, %.2f, %.2f)\n",
				packet_id, destination_id, tx_duration, tx_power, x, y, z);
	}

	/*
	 * SetSizeOfMCS(): sets the size of the array modulation_schemes
	 */
	void SetSizeOfMCS(int channels_groups){

		modulation_schemes = (int *) malloc(channels_groups * sizeof(*modulation_schemes));

		for(int s = 0; s < channels_groups; s++){
			modulation_schemes[s] = MODULATION_NONE;
		}
	}

};

// Notification info
struct Notification
{
	// ALways read in destination
	int source_id;		// Node id of the source
	int packet_type;	// Type of packet: Data, ACK, etc.
	int left_channel;	// Left channel used in the transmission
	int right_channel;	// Right channel used in the transmission
	int packet_length;	// Size of the packet to transmit
	int modulation_id;	// Modulation being used during the transmission
	double timestampt;

	// Specific transmission info (may not be checked by the others nodes)
	TxInfo tx_info;

	void PrintNotification(void){
		printf("source_id = %d - packet_type = %d - left_channel = %d - right_channel = %d - pkt_length = %d -",
				source_id, packet_type, left_channel, right_channel, packet_length);
		printf("tx_info: ");
		tx_info.PrintTxInfo();
	}
};

#endif
