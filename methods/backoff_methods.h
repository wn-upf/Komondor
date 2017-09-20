/* This is just an sketch of what our Komondor headers should look like.
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
 * File description:
 *
 * - Bla bla bla...
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../list_of_macros.h"

// Exponential redefinition for convenience
double	Random2( double v=1.0)	{ return v*drand48();}
int		Random2( int v)		{ return (int)(v*drand48()); }
double	Exponential2(double mean)	{ return -mean*log(Random2());}

/*
 * ComputeBackoff(): computes a new backoff
 * */
double ComputeBackoff(int pdf_backoff, int cw, int backoff_type){

	double backoff_time;
	double expected_backoff = (double) (cw-1)/2;	// [slots]
	double lambda_backoff =  1/(expected_backoff * SLOT_TIME);

	switch(pdf_backoff){

		case PDF_DETERMINISTIC:{

			if(backoff_type == BACKOFF_SLOTTED) {

				int num_slots = rand() % cw; // Num slots in [0, CW-1]
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

	// HARDCODED BY SERGIO TO TEST
	// backoff_time = SLOT_TIME;
	// END OF HARDCODING

	return backoff_time;
}

/*
 * computeRemainingBackoff(): computes the remaining backoff after some even happens
 * */
double ComputeRemainingBackoff(int backoff_type, double remaining_backoff){

//	printf("----------------------------------\n");
//	printf("remaining_backoff = %f\n", remaining_backoff);

	double updated_remaining_backoff;

	switch(backoff_type){

		case BACKOFF_SLOTTED: {

			int closest_slot = round(remaining_backoff / SLOT_TIME);
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

//	printf("updated = %f (%d)\n", updated_remaining_backoff, (int) (updated_remaining_backoff/ SLOT_TIME));

	return updated_remaining_backoff;
}

/*
 * HandleBackoff(): handles the backoff. It is called when backoff may be paused or resumed.
 * */
int HandleBackoff(int pause_or_resume, double *channel_power, int primary_channel, double cca,
	int packets_in_buffer){

	int handle_success = FALSE;

	switch(pause_or_resume){

		case PAUSE_TIMER:{

			if(channel_power[primary_channel] > cca) handle_success = TRUE;
			break;

		}

		case RESUME_TIMER:{

			if(packets_in_buffer > 0) {
				if(channel_power[primary_channel] <= cca) handle_success =  TRUE;
			}
			break;

		}

		default:{

			printf("Unknown handle backoff mode! (not resume nor pause)\n");
			exit(EXIT_FAILURE);

		}
	}

	return handle_success;
}

/*
 * HandleCongestionWindow(): increase or decrease the contention window.
 **/
void HandleContentionWindow(int cw_adaptation, int increase_or_reset, int* cw_current, int cw_min,
		int *cw_stage_current, int cw_stage_max) {

//	printf("- cw_current = %d - cw_min = %d - cw_stage_current = %d - cw_stage_max = %d\n",
//			*cw_current, cw_min, *cw_stage_current, cw_stage_max);

	// http://article.sapub.org/pdf/10.5923.j.jwnc.20130301.01.pdf

	if(cw_adaptation == TRUE){

		switch(increase_or_reset){

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

		// Constant CW
		// - do nothing: keep cw

	}

}
