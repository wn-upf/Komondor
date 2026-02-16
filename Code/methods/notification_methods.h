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
 * notification_methods.h: this file contains functions related to the main Komondor's operation
 *
 * - This file contains the methods related to "notifications" operations
 * - NACK generation/processing is in nack_methods.h (included below)
 * - Packet loss detection/handling is in packet_loss_methods.h (included below)
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include "../list_of_macros.h"

/**
* Generate the TxInfo struct in a Notification
* @param "num_packets_aggregated" [type int]: primary channel
* @param "data_duration" [type double]: duration of the DATA frame
* @param "ack_duration" [type double]: duration of the ACK frame
* @param "rts_duration" [type double]: duration of the RTS frame
* @param "cts_duration" [type double]: duration of the CTS frame
* @param "current_tx_power" [type double]: transmission power used
* @param "num_channels_tx" [type int]: number of channels for transmitting
* @param "tx_gain" [type double]: transmission gain of the antenna
* @param "bits_ofdm_sym" [type int]: bits of an OFDM symbol
* @param "x" [type double]: x position of the node sending the notification
* @param "y" [type double]: y position of the node sending the notification
* @param "z" [type double]: z position of the node sending the notification
* @return "tx_info" [type TxInfo]: transmission information to be included in a notification
*/
TxInfo GenerateTxInfo(int num_packets_aggregated, double data_duration,	double ack_duration,
		double rts_duration, double cts_duration, double current_tx_power, int num_channels_tx,
		int bits_ofdm_sym, double x, double y, double z, double flag_change_in_tx_power) {

	TxInfo tx_info;
	tx_info.SetSizeOfMCS(4);	// TODO: make size dynamic

	tx_info.num_packets_aggregated = num_packets_aggregated;
	tx_info.data_duration = data_duration;
	tx_info.ack_duration = ack_duration;
	tx_info.rts_duration = rts_duration;
	tx_info.cts_duration = cts_duration;
	tx_info.tx_power = ComputeTxPowerPerChannel(current_tx_power, num_channels_tx);
	tx_info.bits_ofdm_sym = bits_ofdm_sym;
	tx_info.x = x;
	tx_info.y = y;
	tx_info.z = z;
	tx_info.nav_time = 0;
    tx_info.flag_change_in_tx_power = flag_change_in_tx_power;

	return tx_info;

}

#include "nack_methods.h"
#include "packet_loss_methods.h"
