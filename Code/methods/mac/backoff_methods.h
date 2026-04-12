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
 * backoff_methods.h: this file contains functions related to the main Komondor's operation
 *
 * - This file contains the backoff methods used to simulate the CSMA/CA operation in WLANs
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include "../../list_of_macros.h"

// Include sub-methods
#include "./channel_access/dcf_methods.h"
#include "./channel_access/edca_methods.h"
#include "./channel_access/tokenized_backoff_methods.h"
#include "./channel_access/deterministic_backoff_methods.h"
#include "./channel_access/eca_methods.h"

// Exponential redefinition for convenience
double	Random2( double v=1.0)	{ return v*drand48();}
int		Random2( int v)		{ return (int)(v*drand48()); }
double	Exponential2(double mean)	{ return -mean*log(Random2());}

/**
* Compute a new backoff value
* @param "current_cw_min" [type int]: current minimum contention window
* @param "current_cw_max" [type int]: current maximum contention window
* @param "backoff_type" [type int]: type of backoff used (e.g., BACKOFF_DCF)
* @param "traffic_type" [type int]: type of traffic for the upcoming channel access
* @param "deterministic_bo_active" [type int]: flag indicating whether deterministic BO is active or not (Only for BACKOFF_DETERMINISTIC_QUALCOMM)
* @param "num_bo_interruptions" [type int]: number of experienced BO interruptions (Only for BACKOFF_DETERMINISTIC_QUALCOMM)
* @param "base_backoff_deterministic" [type int]: base backoff (Only for BACKOFF_DETERMINISTIC_QUALCOMM)
* @param "previous_backoff" [type double]: previous employed backoff backoff (Only for BACKOFF_REPEAT_BO)
* @return "backoff_time" [type double]: new generated backoff
*/
double ComputeBackoff(int current_cw_min, int current_cw_max, int backoff_type,
		int traffic_type, int deterministic_bo_active, int num_bo_interruptions, int base_backoff_deterministic,
		double previous_backoff){

	double backoff_time;

	switch(backoff_type) {

		// Backoff "DCF" (CW and max. stage manually introduced) or Token-based
		case BACKOFF_DCF:
		case BACKOFF_TOKENIZED:
			backoff_time = ComputeBackoffDCF(current_cw_min, current_cw_max);
			break;

		// EDCA for QoS traffic differentiation
		case BACKOFF_EDCA:
			backoff_time = ComputeBackoffEDCA(traffic_type);
			break;

		// Deterministic backoff (Qualcomm)
		case BACKOFF_DETERMINISTIC_QUALCOMM:
			backoff_time = ComputeBackoffDeterministic(deterministic_bo_active, num_bo_interruptions, 
				base_backoff_deterministic, current_cw_min, current_cw_max);
			break;

		// Repeat backoff (repeat the last backoff if the transmission was successful)
		case BACKOFF_REPEAT_BO:
			backoff_time = ComputeBackoffRepeat(deterministic_bo_active, previous_backoff, current_cw_min, current_cw_max);
			break;

		// ECA (https://arxiv.org/pdf/1512.02062, https://arxiv.org/pdf/1311.0787
		case BACKOFF_ECA: 
			backoff_time = ComputeBackoffECA(deterministic_bo_active, previous_backoff, 
				base_backoff_deterministic, current_cw_min, current_cw_max);
			break;

		// SYNCHRONIZED BACKOFF
		case BACKOFF_SYNCHRONIZED:
			backoff_time = base_backoff_deterministic * SLOT_TIME;
			break;

		default:
			printf("ERROR IN ComputeBackoff: Unknown backoff_type.\n");
			exit(EXIT_FAILURE);		

	}

	return backoff_time;

}

/**
* Compute the remaining backoff after some even happens. Particularly useful to compute the closest slot in the BACKOFF_SLOTTED type.
* @param "backoff_type" [type int]: type of backoff used (BACKOFF_SLOTTED) ---> BACKOFF_SLOTTED is recommended
* @param "remaining_backoff" [type double]: remaining backoff
* @return "updated_remaining_backoff" [type double]: updated value of the remaining backoff
*/
double ComputeRemainingBackoff(int backoff_type, double remaining_backoff){

	double updated_remaining_backoff;
	int closest_slot (round(remaining_backoff / SLOT_TIME));
//			printf("- closest_slot = %d\n", closest_slot);
	if(fabs(remaining_backoff - closest_slot * SLOT_TIME) < MAX_DIFFERENCE_SAME_TIME){
		updated_remaining_backoff = closest_slot * SLOT_TIME;
	} else {
		updated_remaining_backoff = ceil(remaining_backoff/SLOT_TIME) * SLOT_TIME;
	}
	return updated_remaining_backoff;

}

/**
* Handle the backoff. It is called when backoff may be paused or resumed.
* @param "pause_or_resume" [type int]: boolean indicating whether to pause or resume the backoff counter
* @param "channel_power" [type double**]: array containing the power sensed in each channel
* @param "primary_channel" [type int]: primary channel used by the node attempting to pause or resume the backoff
* @param "pd" [type double]: packet detect (PD) threshold in pW
* @param "packets_in_buffer" [type int]: number of packets in the buffer
* @return "backoff_action" [type int]: boolean indicating whether to accept or not to pause/resume the backoff counter
*/
int HandleBackoff(int pause_or_resume, double **channel_power, int primary_channel, double pd, int packets_in_buffer){

	int backoff_action (FALSE);

	switch(pause_or_resume){

		case PAUSE_TIMER:
			if((*channel_power)[primary_channel] > pd) backoff_action = TRUE;
			break;

		case RESUME_TIMER:
			//if(packets_in_buffer > 0) {
			if((*channel_power)[primary_channel] <= pd) backoff_action =  TRUE;
			//}
			break;

		default:
			printf("ERROR in HandleBackoff: Unknown handle backoff mode! (not resume nor pause)\n");
			exit(EXIT_FAILURE);

	}

	return backoff_action;

}

/**
* Increase or decrease the contention window. CW adaptation: http://article.sapub.org/pdf/10.5923.j.jwnc.20130301.01.pdf
* @param "cw_adaptation" [type int]: boolean indicating whether CW adaptation is enabled or not
* @param "increase_or_reset" [type int]: flag indicating whether the CW should be increased or reseted
* @param "deterministic_bo_active" [type int*]: flag indicating whether deterministic BO is actived or not (to be modified by this method, only for BACKOFF_DETERMINISTIC_QUALCOMM)
* @param "current_cw_min" [type int*]: pointer to the current minimum CW (to be modified by this method)
* @param "current_cw_max" [type int*]: pointer to the current maximum CW (to be modified by this method)
* @param "cw_stage_current" [type int*]: pointer to the current CW stage (to be modified by this method)
* @param "cw_min_default" [type int]: default minimum CW
* @param "cw_max_default" [type int]: default maximum CW
* @param "cw_stage_max" [type int]: maximum CW stage (only for BACKOFF_DCF)
* @param "distance_to_token" [type int]: distance to token (only for BACKOFF_TOKENIZED)
* @param "backoff_type" [type int]: type of backoff used
*/
void HandleContentionWindow(int cw_adaptation, int increase_or_reset, int *deterministic_bo_active, int *current_cw_min,
		int *current_cw_max, int *cw_stage_current, int cw_min_default, int cw_max_default, int cw_stage_max,
		int distance_to_token, int backoff_type) {

	switch (backoff_type) {

		// DCF (DEFAULT): exponential backoff on failure, reset on success
		case BACKOFF_DCF:
			HandleBackoffDCF(cw_adaptation, increase_or_reset, cw_stage_current, current_cw_min,
    			current_cw_max, cw_min_default, cw_max_default, cw_stage_max);			
			break;

		// EDCA and SYNCHRONIZED: no CW adaptation
		case BACKOFF_EDCA:
		case BACKOFF_SYNCHRONIZED:
			break;

		// Token-based: CW set by position in token ring
		case BACKOFF_TOKENIZED:
			HandleBackoffTokenized(distance_to_token, current_cw_min, current_cw_max, cw_max_default);
			break;

		// Deterministic BO (Qualcomm): increase_or_reset toggles the deterministic flag only
		case BACKOFF_DETERMINISTIC_QUALCOMM:
			HandleBackoffDeterministic(increase_or_reset, deterministic_bo_active);
			break;

		// REPEAT_BO and ECA share identical behavior
		case BACKOFF_REPEAT_BO:
		case BACKOFF_ECA:
			HandleBackoffECA(increase_or_reset, cw_stage_current, current_cw_min, current_cw_max,
    			cw_min_default, cw_max_default, cw_stage_max, deterministic_bo_active);
			break;

		default:
			printf("ERROR in HandleContentionWindow: Unknown backoff type.\n");
			exit(EXIT_FAILURE);
	}

}