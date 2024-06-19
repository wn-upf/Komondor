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
 * - This file contains the backoff methods used to simulate the CSMA/CA operation in WLANs
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
* @param "pdf_backoff" [type int]: type of backoff distribution (PDF_UNIFORM or PDF_EXPONENTIAL)
* @param "current_cw" [type int]: current contention window
* @param "backoff_type" [type int]: type of backoff used (e.g., BACKOFF_SLOTTED)
* @return "backoff_time" [type double]: new generated backoff
*/
double ComputeBackoff(int pdf_backoff, int current_cw_min, int current_cw_max, int backoff_type){

	double backoff_time;
	double expected_backoff ((double) (current_cw_max-1)/2);	// [slots]

	switch(pdf_backoff){

		case PDF_UNIFORM:{
			// Backoff "Custom" (CW and max. stage manually introduced) or EDCA
			if(backoff_type == BACKOFF_CUSTOM || backoff_type == BACKOFF_EDCA) {
				int num_slots (current_cw_min + (std::rand() % ( current_cw_max - current_cw_min + 1 )));
				backoff_time = num_slots * SLOT_TIME;
				//int num_slots (rand() % current_cw); // Number of slots in [0, CW-1]
				// printf("num_slots = %d\n", num_slots);
			// TODO: Implement token-based backoff
			} else if(backoff_type == BACKOFF_TOKENIZED) {
				int distance_to_token = 1;
				int cw_range = distance_to_token*current_cw_max - current_cw_max + 1;
				int num_slots = rand() % cw_range + current_cw_max;
				backoff_time = num_slots * SLOT_TIME;
			// TODO: Implement deterministic backoff
			} else if(backoff_type == BACKOFF_DETERMINISTIC_QUALCOMM) {
				int deterministic_phase_active = 0;
				if (deterministic_phase_active) {
					int num_interruptions = 0;
					int base_backoff = 5;
					int num_slots = base_backoff +  num_interruptions;
					backoff_time = num_slots * SLOT_TIME;
				} else {
					int num_slots (rand() % current_cw_max); // Number of slots in [0, CW-1]
					backoff_time = num_slots * SLOT_TIME;
				}
			} else {
				printf("ERROR IN ComputeBackoff: Unknown backoff_type.\n");
				exit(EXIT_FAILURE);
			}

			break;
		}

		case PDF_EXPONENTIAL:{
			if(backoff_type == BACKOFF_CUSTOM) {
				backoff_time = round(Exponential2(expected_backoff)) * SLOT_TIME;
			} else {
				printf("ERROR IN ComputeBackoff: PDF_EXPONENTIAL only works for backoff_type = BACKOFF_CUSTOM.\n");
				exit(EXIT_FAILURE);
			}
			break;
		}

		default:{
			printf("ERROR IN ComputeBackoff: Backoff PDF not found!\n");
			exit(EXIT_FAILURE);
		}
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
int HandleBackoff(int pause_or_resume, double **channel_power, int primary_channel, double pd,
	int packets_in_buffer){

	int backoff_action (FALSE);

	switch(pause_or_resume){

		case PAUSE_TIMER:{
			if((*channel_power)[primary_channel] > pd) backoff_action = TRUE;
			break;
		}

		case RESUME_TIMER:{
			//if(packets_in_buffer > 0) {
			if((*channel_power)[primary_channel] <= pd) backoff_action =  TRUE;
			//}
			break;
		}

		default:{
			printf("ERROR in HandleBackoff: Unknown handle backoff mode! (not resume nor pause)\n");
			exit(EXIT_FAILURE);
		}
	}

	return backoff_action;

}

/**
* Increase or decrease the contention window. CW adaptation: http://article.sapub.org/pdf/10.5923.j.jwnc.20130301.01.pdf
* @param "cw_adaptation" [type int]: boolean indicating whether CW adaptation is enabled or not
* @param "current_cw" [type int*]: pointer to the current CW (to be modified by this method)
* @param "cw_base_max" [type int]: Max base CW value
* @param "cw_base_min" [type int]: Min base CW value
* @param "cw_stage_current" [type int]: pointer to the current CW stage (to be modified by this method)
* @param "cw_stage_max" [type int]: maximum CW stage
*/
void HandleContentionWindow(int cw_adaptation, int increase_or_reset, int *current_cw_min, int *current_cw_max,
		int *cw_stage_current, int cw_min_default, int cw_max_default, int cw_stage_max, int traffic_type, int backoff_type) {

	// Select the CW parameters depending on the traffic to be transmitted
	switch(backoff_type){

		// CUSTOM (DEFAULT)
		case BACKOFF_CUSTOM:{
			current_cw_min = 0; // CW_min is always 0 for this type of backoff
			// Apply CW adaptation when needed
			if(cw_adaptation == TRUE){
				switch(increase_or_reset){
					case INCREASE_CW:{
						if(*cw_stage_current < cw_stage_max){
							*cw_stage_current = *cw_stage_current + 1;
							*current_cw_min = cw_min_default * pow(2, *cw_stage_current);
							*current_cw_max = cw_max_default * pow(2, *cw_stage_current);
						}
						break;
					}
					case RESET_CW:{
						*cw_stage_current = 0;
						*current_cw_min = cw_min_default;
						*current_cw_max = cw_max_default;
						break;
					}
					default:{
						printf("ERROR in HandleContentionWindow: Unknown operation on contention window!\n");
						exit(EXIT_FAILURE);
						break;
					}
				}
			} else {
				// Constant CW - do nothing: keep the CW values
			}
			break;
		}

		// EDCA
		case BACKOFF_EDCA:{
			int cw_min;
			int cw_max;
			// Get standard parameters regarding CW parameters
			switch(traffic_type){
				case AC_VO:{
					cw_min = CW_MIN_AC_VO;
					cw_max = CW_MAX_AC_VO;
					break;
				}
				case AC_VI:{
					cw_min = CW_MIN_AC_VI;
					cw_max = CW_MAX_AC_VI;
					break;
				}
				case AC_BE:{
					cw_min = CW_MIN_AC_BE;
					cw_max = CW_MAX_AC_BE;
					break;
				}
				case AC_BK:{
					cw_min = CW_MIN_AC_BK;
					cw_max = CW_MAX_AC_BK;
					break;
				}
				default:{
					printf("WARNING in HandleContentionWindow: Unknown AC traffic type!\n");
					cw_min = CW_MIN_AC_BK;
					cw_max = CW_MAX_AC_BK;
					break;
				}
			}
			*current_cw_min = cw_min;
			*current_cw_max = cw_max;
			break;
		}

		// Token-based BO
		case BACKOFF_TOKENIZED:{
			// TODO: To be implemented
			break;
		}

		// Token-based BO
		case BACKOFF_DETERMINISTIC_QUALCOMM:{
			// TODO: To be implemented
			break;
		}

		default:{
			printf("ERROR in HandleContentionWindow: Unknown backoff type.\n");
			exit(EXIT_FAILURE);
		}
	}

}
