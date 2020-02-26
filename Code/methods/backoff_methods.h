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
 * backoff_methods.h: this file contains functions related to the main Komondor's operation
 *
 * - This file contains the backoff methods used to simulated the CSMA/CA operation in WLANs
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include "../list_of_macros.h"

// Exponential redefinition for convenience
double	Random2( double v=1.0)	{ return v*drand48();}
int		Random2( int v)		{ return (int)(v*drand48()); }
double	Exponential2(double mean)	{ return -mean*log(Random2());}

/**
* Compute a new backoff value
* @param "pdf_backoff" [type int]: type of backoff distribution (PDF_DETERMINISTIC or PDF_EXPONENTIAL)
* @param "cw" [type int]: current contention window
* @param "backoff_type" [type int]: type of backoff used (BACKOFF_SLOTTED or BACKOFF_CONTINUOUS) ---> BACKOFF_SLOTTED is highly recommended
* @return "backoff_time" [type double]: new generated backoff
*/
double ComputeBackoff(int pdf_backoff, int cw, int backoff_type){

	double backoff_time;
	double expected_backoff ((double) (cw-1)/2);	// [slots]
	double lambda_backoff (1/(expected_backoff * SLOT_TIME));

	switch(pdf_backoff){

		case PDF_DETERMINISTIC:{
			if(backoff_type == BACKOFF_SLOTTED) {
				int num_slots (rand() % cw); // Num slots in [0, CW-1]
				backoff_time = num_slots * SLOT_TIME;
				// printf("num_slots = %d\n", num_slots);
			} else if(backoff_type == BACKOFF_CONTINUOUS) {
				backoff_time = 1/lambda_backoff;
			}
			break;
		}

		case PDF_EXPONENTIAL:{
			if(backoff_type == BACKOFF_SLOTTED) {
				backoff_time = round(Exponential2(expected_backoff)) * SLOT_TIME;
			} else if(backoff_type == BACKOFF_CONTINUOUS) {
				backoff_time = Exponential2(1/lambda_backoff);
			}
			break;
		}

		default:{
			printf("Backoff model not found!\n");
			break;
		}
	}

	return backoff_time;

}

/**
* Compute the remaining backoff after some even happens. Particularly useful to compute the closest slot in the BACKOFF_SLOTTED type.
* @param "backoff_type" [type int]: type of backoff used (BACKOFF_SLOTTED or BACKOFF_CONTINUOUS) ---> BACKOFF_SLOTTED is highly recommended
* @param "remaining_backoff" [type double]: remaining backoff
* @return "updated_remaining_backoff" [type double]: updated value of the remaining backoff
*/
double ComputeRemainingBackoff(int backoff_type, double remaining_backoff){

	double updated_remaining_backoff;

	switch(backoff_type){

		case BACKOFF_SLOTTED: {
			int closest_slot (round(remaining_backoff / SLOT_TIME));
//			printf("- closest_slot = %d\n", closest_slot);
			if(fabs(remaining_backoff - closest_slot * SLOT_TIME) < MAX_DIFFERENCE_SAME_TIME){
				updated_remaining_backoff = closest_slot * SLOT_TIME;
			} else {
				updated_remaining_backoff = ceil(remaining_backoff/SLOT_TIME) * SLOT_TIME;
			}
			break;
		}

		case BACKOFF_CONTINUOUS: {
			updated_remaining_backoff = remaining_backoff;
			break;
		}

		default:{
			printf("Backoff type not found!\n");
			exit(EXIT_FAILURE);
			break;
		}

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
int HandleBackoff(int pause_or_resume, double **channel_power, int primary_channel, double pd,
	int packets_in_buffer){

	int backoff_action (FALSE);

	switch(pause_or_resume){

		case PAUSE_TIMER:{
			if((*channel_power)[primary_channel] > pd) backoff_action = TRUE;
			break;
		}

		case RESUME_TIMER:{
			if(packets_in_buffer > 0) {
				if((*channel_power)[primary_channel] <= pd) backoff_action =  TRUE;
			}
			break;
		}

		default:{
			printf("Unknown handle backoff mode! (not resume nor pause)\n");
			exit(EXIT_FAILURE);
		}
	}

	return backoff_action;

}

/**
* Increase or decrease the contention window. CW adaptation: http://article.sapub.org/pdf/10.5923.j.jwnc.20130301.01.pdf
* @param "cw_adaptation" [type int]: boolean indicating whether CW adaptation is enabled or not
* @param "pause_or_resume" [type int]: boolean indicating whether to increase or reset the CW
* @param "cw_current" [type int*]: pointer to the current CW (to be modified by this method)
* @param "cw_min" [type int]: minimum CW
* @param "cw_stage_current" [type int]: pointer to the current CW stage (to be modified by this method)
* @param "cw_stage_max" [type int]: maximum CW stage
*/
void HandleContentionWindow(int cw_adaptation, int increase_or_reset, int* cw_current, int cw_min,
		int *cw_stage_current, int cw_stage_max) {
	if(cw_adaptation == TRUE){
		switch(increase_or_reset) {
			case INCREASE_CW:{
				if(*cw_stage_current < cw_stage_max){
					*cw_stage_current = *cw_stage_current + 1;
					*cw_current = cw_min * pow(2, *cw_stage_current);
				}
				break;
			}
			case RESET_CW:{
				*cw_stage_current = 0;
				*cw_current = cw_min;
				break;
			}
			default:{
				printf("Unknown operation on contention window!");
				exit(EXIT_FAILURE);
				break;
			}
		}
	} else {
		// Constant CW - do nothing: keep cw
	}
}
