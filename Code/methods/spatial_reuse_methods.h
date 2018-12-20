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
 * File description:
 *
 * - This file contains the methods related to "spatial reuse" operations
 */

#include <stddef.h>
#include <math.h>
#include <iostream>

#include "../list_of_macros.h"
#include "auxiliary_methods.h"

/*
 * CheckPacketOrigin(): checks whether the received notification is an intra-BSS,
 * an inter-BSS, an SRG or a non-SRG frame
 * Arguments:
 * - notification: notification to be inspected
 * - bss_color: BSS color of the node inspecting the notification
 * - srg: SRG of the node inspecting the notification
 * Output:
 * - type_of_packet: type of packet
 **/
int CheckPacketOrigin(Notification notification, int bss_color, int srg) {

	int type_of_packet;

	int bss_color_enabled = false;
	int srg_enabled = false;

	if (notification.tx_info.bss_color >= 0 && bss_color >= 0) bss_color_enabled = true;
	if (notification.tx_info.srg >= 0 && srg >= 0) srg_enabled = true;

	if (!bss_color_enabled) {
		type_of_packet = INTRA_BSS_FRAME;
	} else {
		if ( notification.tx_info.bss_color == bss_color ) {
			type_of_packet = INTRA_BSS_FRAME;
		} else {
			if (srg_enabled) {
				if ( notification.tx_info.srg == srg ) {
					type_of_packet = SRG_FRAME;
				} else {
					type_of_packet = NON_SRG_FRAME;
				}
			} else {
				type_of_packet = INTER_BSS_FRAME;
			}
		}
	}

	return type_of_packet;

}

/*
 * GetSensitivitySpatialReuse(): checks whether the received notification is an intra-BSS,
 * an inter-BSS, an SRG or a non-SRG frame
 * Arguments:
 * - packet_type: type of detected frame (according to source)
 * - cca_default: default CST (for intra-BSS communications)
 * - obss_pd: CST to be used for inter-BSS communications (different BSS color)
 * - srg_obss_pd: CST to be used for communications within the same SRG
 * - non_srg_obss_pd: CST to be used for communications within different SRG
 * Output:
 * - cst: CST in pW to be used according to the source of the detected frame
 **/
double GetSensitivitySpatialReuse(int packet_type, double cca_default,
		double obss_pd, double srg_obss_pd, double non_srg_obss_pd) {

	double cst_pw;

	switch(packet_type){
		case INTRA_BSS_FRAME: {
			cst_pw = cca_default;
			break;
		}
		case INTER_BSS_FRAME: {
			cst_pw = obss_pd;
			break;
		}
		case SRG_FRAME: {
			cst_pw = srg_obss_pd;
			break;
		}
		case NON_SRG_FRAME: {
			cst_pw = non_srg_obss_pd;
			break;
		}
		default: {
			printf("Error: Unknown frame type! Using the default CCA (%f dBm)\n",
				ConvertPower(PW_TO_DBM, cca_default));
			cst_pw = cca_default;
			break;
		}
	}

	return cst_pw;

}

/*
 * ApplyTxPowerRestriction(): applies a transmit power restriction, according to the CST used
 * Arguments:
 * - current_cca: default CCA used by the node
 * - current_tpc: default transmit power used by the node
 * Output:
 * - tx_power_pw: transmit power in pW to be used during the next TXOP
 **/
double ApplyTxPowerRestriction(double current_cca, double current_tpc) {

	double tx_power_pw;

	double current_cca_dbm = ConvertPower(PW_TO_DBM, current_cca);

	if (current_cca_dbm <= OBSS_PD_MIN) {
		tx_power_pw = current_tpc; // No restriction
	} else {
		tx_power_pw = ConvertPower(DBM_TO_PW, TX_PWR_REF - (current_cca_dbm - OBSS_PD_MIN));
	}

	return tx_power_pw;

}























/*
 * CheckOBSSPDConstraints(): checks if the proposed obss_pd_level is valid, according to
 * the constraints indicated in the IEEE 802.11ax amendment
 **/
int CheckObssPdConstraints(double current_obss_pd, double obss_pd_min, double obss_pd_max,
		double tx_power_ref, double tx_power) {

	double obss_pd_ref_dbm = std::max(ConvertPower(PW_TO_DBM, obss_pd_min),
			std::min(ConvertPower(PW_TO_DBM, obss_pd_max), ConvertPower(PW_TO_DBM, obss_pd_min)
			+ (ConvertPower(PW_TO_DBM, tx_power_ref) - ConvertPower(PW_TO_DBM,tx_power))));

//	printf("obss_pd_ref = %f (%f pW)\n", obss_pd_ref, ConvertPower(DBM_TO_PW, obss_pd_ref));
//	printf("  - obss_pd_min = %f (%f pW)\n", ConvertPower(PW_TO_DBM, obss_pd_min), obss_pd_min);
//	printf("  - obss_pd_max = %f (%f pW)\n", ConvertPower(PW_TO_DBM, obss_pd_max), obss_pd_max);
//	printf("  - tx_power_ref = %f (%f pW)\n", ConvertPower(PW_TO_DBM, tx_power_ref), tx_power_ref);
//	printf("  - tx_power = %f (%f pW)\n", ConvertPower(PW_TO_DBM,tx_power), tx_power);

	if( ConvertPower(PW_TO_DBM, current_obss_pd) <=  obss_pd_ref_dbm ) {
		//printf("The OBSS_PD level is appropriate!\n");
		return 1;
	} else {
		//printf("ALERT! The OBSS_PD level is NOT appropriate!\n");
		return 0;
	}

}

/*
 * CheckSrgObssPdConstraints(): checks if the proposed srg_obss_pd_level is valid, according to
 * the constraints indicated in the IEEE 802.11ax amendment
 **/
int CheckSrgObssPdConstraints(double current_srg_obss_pd,
		double srg_obss_pd_min, double srg_obss_pd_max,
		double srg_obss_pd_min_offset, double srg_obss_pd_max_offset) {

	if( (ConvertPower(PW_TO_DBM, srg_obss_pd_min_offset) - 82) <= -62 &&
			(ConvertPower(PW_TO_DBM, srg_obss_pd_min_offset) - 82) >= -82 &&
			ConvertPower(PW_TO_DBM, srg_obss_pd_min_offset) <= ConvertPower(PW_TO_DBM, srg_obss_pd_max_offset) &&
			(ConvertPower(PW_TO_DBM, srg_obss_pd_max_offset) - 82) <= -62 &&
			ConvertPower(PW_TO_DBM, current_srg_obss_pd) >= ConvertPower(PW_TO_DBM, srg_obss_pd_min) &&
			ConvertPower(PW_TO_DBM, current_srg_obss_pd) <= ConvertPower(PW_TO_DBM, srg_obss_pd_max)) {
		//printf("The SRG_OBSS_PD level is appropriate!\n");
		return 1;
	} else {
		//printf("ALERT! The SRG_OBSS_PD level is NOT appropriate!\n");
		return 0;
	}

}

/*
 * CheckNonSrgObssPdConstraints(): checks if the proposed non_srg_obss_pd_level is valid, according to
 * the constraints indicated in the IEEE 802.11ax amendment
 **/
int CheckNonSrgObssPdConstraints(double current_non_srg_obss_pd,
		double non_srg_obss_pd_min, double non_srg_obss_pd_max,
		double non_srg_obss_pd_max_offset, double srg_obss_pd_max_offset) {

	if( non_srg_obss_pd_max_offset <= srg_obss_pd_max_offset &&
			ConvertPower(PW_TO_DBM, non_srg_obss_pd_max_offset) -82 <= -62 &&
			current_non_srg_obss_pd >= non_srg_obss_pd_min &&
			current_non_srg_obss_pd <= non_srg_obss_pd_max ) {
		//printf("The NON_SRG_OBSS_PD level is appropriate!\n");
		return 1;
	} else {
		//printf("ALERT! The NON_SRG_OBSS_PD level is NOT appropriate!\n");
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
