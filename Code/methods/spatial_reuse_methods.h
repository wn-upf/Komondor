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
 * spatial_reuse_methods.h: this file contains functions related to the main Komondor's operation
 *
 * - This file contains the methods related to "spatial reuse" operations
 *
 * For more information regarding the IEEE 802.11ax Spatial Reuse operation, refer to:
 * 	- https://arxiv.org/abs/1907.04141
 * 	- https://arxiv.org/abs/1906.08063
 *
 */

#include <stddef.h>
#include <math.h>
#include <iostream>

#include "../list_of_macros.h"
#include "auxiliary_methods.h"

/**
* Check whether the received notification is an intra-BSS, an inter-BSS, an SRG or a non-SRG frame
* @param "notification" [type Notification]: notification to be inspected
* @param "bss_color" [type int]: BSS color of the node inspecting the notification
* @param "srg" [type int]: SRG of the node inspecting the notification
* @return "type_of_packet" [type int]: type of packet
*/
int CheckPacketOrigin(Notification notification, int bss_color, int srg) {

	int type_of_packet;
	int bss_color_enabled (false);
	if (notification.tx_info.bss_color >= 0 && bss_color >= 0) bss_color_enabled = true;

	if (!bss_color_enabled) {
		type_of_packet = INTRA_BSS_FRAME;
	} else {
		if ( notification.tx_info.bss_color == bss_color && notification.tx_info.bss_color > 0 ) {
			type_of_packet = INTRA_BSS_FRAME;
		} else {
			if ( notification.tx_info.srg == srg && notification.tx_info.srg > 0) {
				type_of_packet = SRG_FRAME;
			} else {
				type_of_packet = NON_SRG_FRAME;
			}
		}
	}

	return type_of_packet;

}

/**
* Compute the PD threshold to be used, according to the type of ongoing transmissions (intra-BSS, inter-BSS, SRG or non-SRG frames)
* @param "type_last_sensed_packet" [type int]: type of packet
* @param "srg_obss_pd" [type double]: SRG OBSS/PD threshold in pW
* @param "non_srg_obss_pd" [type double]: non-SRG OBSS/PD threshold in pW
* @param "pd_default" [type double]: default PD threshold in pW
* @param "power_received" [type double]: power received in pW
* @return "pd_spatial_reuse_pw" [type double]: CST in pW to be used according to the source of the detected frame
*/
double GetSensitivitySpatialReuse( int type_last_sensed_packet, double srg_obss_pd,
	double non_srg_obss_pd, double pd_default, double power_received ) {

	double pd_spatial_reuse_pw;
	// Choose the PD threshold according to the source of the detected transmission
	switch(type_last_sensed_packet) {
		case NON_SRG_FRAME:{
			pd_spatial_reuse_pw = non_srg_obss_pd;
			break;
		}
		case SRG_FRAME:{
			pd_spatial_reuse_pw = srg_obss_pd;
			break;
		}
		default:{
			pd_spatial_reuse_pw = pd_default;
			break;
		}
	}
	return pd_spatial_reuse_pw;

}

/**
* Indicate whether an SR-based opportunity has been detected or not
* @param "power_received" [type double]: RSSI detected from the ongoing transmission
* @param "current_obss_pd" [type double]: OBSS/PD threshold in pW currently being used
* @return "1" or "0" [type int]: boolean indicating whether an SR-based opportunity has been detected or not
*/
int IdentifySpatialReuseOpportunity( double power_received, double current_obss_pd) {
	if (power_received < current_obss_pd) {
		return 1;
	} else {
		return 0;
	}
}

/**
* Apply a transmit power restriction, according to the OBSS/PD threshold used
* @param "current_pd" [type double]: current PD threshold being used by the node
* @param "current_tx_power" [type double]: current transmit power being used by the node
* @return "tx_power_pw" [type int]: limited transmit power (in pW) to be used during the next TXOP
*/
double ApplyTxPowerRestriction(double current_pd, double current_tx_power) {

	double tx_power_pw;

	double current_pd_dbm = ConvertPower(PW_TO_DBM, current_pd);

	if (current_pd_dbm <= OBSS_PD_MIN) {
		tx_power_pw = current_tx_power; // No restriction
	} else {
		tx_power_pw = ConvertPower(DBM_TO_PW, TX_PWR_REF - (current_pd_dbm - OBSS_PD_MIN));
	}

	// Apply general restriction
	if (tx_power_pw > ConvertPower(DBM_TO_PW, MAX_TX_PWR_SR)) {
		tx_power_pw = ConvertPower(DBM_TO_PW, MAX_TX_PWR_SR);
	}

	return tx_power_pw;

}

/**
* Update the array "type_ongoing_transmissions", which stores the type of each sensed ongoing transmissions.
* @param "type_ongoing_transmissions" [type int*]: array of the types of ongoing transmissions (to be updated by this method)
* @param "notification" [type Notification]: last notification received
* @param "bss_color" [type int]: BSS color of the node updating the array
* @param "srg" [type int]: SRG of the node updating the array
* @param "enter_or_leave" [type int]: indicates whether the transmission starts (1) or ends (0)
*/
void UpdateTypeOngoingTransmissions(int *type_ongoing_transmissions,
	Notification notification, int bss_color, int srg, int enter_or_leave) {

	// Identify the type of packet according to the BSS color and the SRG
	int packet_type_source = CheckPacketOrigin(notification, bss_color, srg);

	// Update the type of ongoing transmission for the detected frame
	if (enter_or_leave) {	// if transmission starts
		type_ongoing_transmissions[packet_type_source] = 1;
	} else {				// if transmission ends
		type_ongoing_transmissions[packet_type_source] = 0;
	}

}

/****************************************************************
 * OLD (TO BE REFACTORED/ADAPTED ACCORDING TO NEWER VERSIONS OF THE AMENDMENT)
 ****************************************************************/
/*
 * CheckOBSSPDConstraints(): checks if the proposed obss_pd_level is valid, according to
 * the constraints indicated in the IEEE 802.11ax amendment
 **/
int CheckObssPdConstraints(double current_obss_pd, double obss_pd_min, double obss_pd_max,
		double tx_power_ref, double tx_power) {
	double obss_pd_ref_dbm = std::max(ConvertPower(PW_TO_DBM, obss_pd_min),
		std::min(ConvertPower(PW_TO_DBM, obss_pd_max), ConvertPower(PW_TO_DBM, obss_pd_min)
		+ (ConvertPower(PW_TO_DBM, tx_power_ref) - ConvertPower(PW_TO_DBM,tx_power))));
	if( ConvertPower(PW_TO_DBM, current_obss_pd) <=  obss_pd_ref_dbm ) {
		//printf("The OBSS_PD level is appropriate!\n");
		return 1;
	} else {
		//printf("ALERT! The OBSS_PD level is NOT appropriate!\n");
		return 0;
	}
}

/*
 * CheckPowerConstraints():
 **/
int CheckPowerConstraints(double current_obss_pd, double obss_pd_min, double obss_pd_max) {
	if (current_obss_pd <= obss_pd_min) {
		// Unconstrained
		return 1;
	} else {
		// Apply constraint
		return 0;
	}
}
