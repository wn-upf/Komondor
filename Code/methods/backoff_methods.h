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
* @param "current_cw_min" [type int]: current minimum contention window
* @param "current_cw_max" [type int]: current maximum contention window
* @param "backoff_type" [type int]: type of backoff used (e.g., BACKOFF_CUSTOM)
* @param "traffic_type" [type int]: type of traffic for the upcoming channel access
* @param "deterministic_bo_active" [type int]: flag indicating whether deterministic BO is active or not (Only for BACKOFF_DETERMINISTIC_QUALCOMM)
* @param "num_bo_interruptions" [type int]: number of experienced BO interruptions (Only for BACKOFF_DETERMINISTIC_QUALCOMM)
* @param "base_backoff_deterministic" [type int]: base backoff (Only for BACKOFF_DETERMINISTIC_QUALCOMM)
* @param "previous_backoff" [type double]: previous employed backoff backoff (Only for BACKOFF_REPEAT_BO)
* @return "backoff_time" [type double]: new generated backoff
*/
double ComputeBackoff(int pdf_backoff, int current_cw_min, int current_cw_max, int backoff_type,
		int traffic_type, int deterministic_bo_active, int num_bo_interruptions, int base_backoff_deterministic,
		double previous_backoff){

	double backoff_time;
	double expected_backoff ((double) (current_cw_max-1)/2);	// [slots]

	switch(pdf_backoff){

		case PDF_UNIFORM:{

			// Backoff "Custom" (CW and max. stage manually introduced) or Token-based
			if(backoff_type == BACKOFF_CUSTOM || backoff_type == BACKOFF_TOKENIZED) {
				int num_slots (current_cw_min + (std::rand() % ( current_cw_max - current_cw_min + 1 )));
				backoff_time = num_slots * SLOT_TIME;

			// EDCA for QoS traffic differentiation
			} else if(backoff_type == BACKOFF_EDCA){
				// Get standard parameters regarding CW parameters
				switch(traffic_type){
					case AC_VO:{
						current_cw_min = CW_MIN_AC_VO;
						current_cw_max = CW_MAX_AC_VO;
						break;
					}
					case AC_VI:{
						current_cw_min = CW_MIN_AC_VI;
						current_cw_max = CW_MAX_AC_VI;
						break;
					}
					case AC_BE:{
						current_cw_min = CW_MIN_AC_BE;
						current_cw_max = CW_MAX_AC_BE;
						break;
					}
					case AC_BK: default:{
						current_cw_min = CW_MIN_AC_BK;
						current_cw_max = CW_MAX_AC_BK;
						break;
					}
				}
				int num_slots (current_cw_min + (std::rand() % ( current_cw_max - current_cw_min + 1 )));
				backoff_time = num_slots * SLOT_TIME;

			// Deterministic backoff (Qualcomm)
			} else if(backoff_type == BACKOFF_DETERMINISTIC_QUALCOMM) {
				if (deterministic_bo_active) {
					int num_slots = base_backoff_deterministic +  num_bo_interruptions;
					backoff_time = num_slots * SLOT_TIME;
				} else {
					int num_slots (current_cw_min + (std::rand() % ( current_cw_max - current_cw_min + 1 )));
					backoff_time = num_slots * SLOT_TIME;
				}

			// Repeat backoff (repeat the last backoff if the transmission was successful)
			} else if(backoff_type == BACKOFF_REPEAT_BO) {
				if (deterministic_bo_active && previous_backoff != -1) {
					backoff_time = previous_backoff;
				} else {
					int num_slots (current_cw_min + (std::rand() % ( current_cw_max - current_cw_min + 1 )));
					backoff_time = num_slots * SLOT_TIME;
				}

			// ECA (https://arxiv.org/pdf/1512.02062, https://arxiv.org/pdf/1311.0787
			} else if(backoff_type == BACKOFF_ECA) {
				if (deterministic_bo_active && previous_backoff != -1) {
					backoff_time = base_backoff_deterministic * SLOT_TIME;
				} else {
					int num_slots (current_cw_min + (std::rand() % ( current_cw_max - current_cw_min + 1 )));
					backoff_time = num_slots * SLOT_TIME;
				}

			// SYNCHRONIZED BACKOFF
			} else if(backoff_type == BACKOFF_SYNCHRONIZED) {
				backoff_time = base_backoff_deterministic * SLOT_TIME;

			} else {
				printf("ERROR IN ComputeBackoff: Unknown backoff_type.\n");
				exit(EXIT_FAILURE);
			}

			break;
		}

		case PDF_EXPONENTIAL:{ // ALMOST DEPRECATED - It is recommended to use PDF_UNIFORM instead
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
int HandleBackoff(int pause_or_resume, double **channel_power, int primary_channel, double pd, int packets_in_buffer){

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
* @param "increase_or_reset" [type int]: flag indicating whether the CW should be increased or reseted
* @param "deterministic_bo_active" [type int]: flag indicating whether deterministic BO is actived or not (to be modified by this method, only for BACKOFF_DETERMINISTIC_QUALCOMM)
* @param "current_cw_min" [type int]: pointer to the current minimum CW (to be modified by this method)
* @param "current_cw_max" [type int]: pointer to the current maximum CW (to be modified by this method)
* @param "cw_stage_current" [type int]: pointer to the current CW stage (to be modified by this method)
* @param "cw_min_default" [type int]: default minimum CW
* @param "cw_max_default" [type int]: default maximum CW
* @param "cw_stage_max" [type int]: maximum CW stage (only for BACKOFF_CUSTOM
* @param "backoff_type" [type int]: type of backoff used
*/
void HandleContentionWindow(int cw_adaptation, int increase_or_reset, int *deterministic_bo_active, int *current_cw_min,
		int *current_cw_max, int *cw_stage_current, int cw_min_default, int cw_max_default, int cw_stage_max,
		int distance_to_token, int backoff_type) {

	// Select the CW parameters depending on the traffic to be transmitted
	switch(backoff_type){

		// CUSTOM (DEFAULT)
		case BACKOFF_CUSTOM:{
			// Apply CW adaptation when needed
			if(cw_adaptation == TRUE){
				switch(increase_or_reset){
					case INCREASE_CW:{	// Increase the CW values exponentially
						if(*cw_stage_current < cw_stage_max){
							*cw_stage_current = *cw_stage_current + 1;
							*current_cw_min = cw_min_default * pow(2, *cw_stage_current);
							*current_cw_max = cw_max_default * pow(2, *cw_stage_current);
						}
						break;
					}
					case RESET_CW:{  // Reset the CW to the default values
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
			// Do nothing - No adaptation is required for this backoff model
			break;
		}

		// Token-based BO
		case BACKOFF_TOKENIZED:{
			if (distance_to_token == 0) {
				*current_cw_min = (distance_to_token+1)*cw_max_default - cw_max_default;
				*current_cw_max = (distance_to_token+1)*cw_max_default;
			} else {
				*current_cw_min = (distance_to_token+1)*cw_max_default - cw_max_default + 1;
				*current_cw_max = (distance_to_token+1)*cw_max_default + 1;
			}
			break;
		}

		// Deterministic BO (Qualcomm) - https://mentor.ieee.org/802.11/dcn/24/11-24-0031-00-00bn-deterministic-backoff.pptx
		case BACKOFF_DETERMINISTIC_QUALCOMM:{
			// "Increase or reset" is used as an indication for enabling/disabling the deterministic backoff
			switch(increase_or_reset){
				case INCREASE_CW:{
					*deterministic_bo_active = 0;
					break;
				}
				case RESET_CW:{
					*deterministic_bo_active = 1;
					break;
				}
				default:{
					printf("ERROR in HandleContentionWindow: Unknown operation on contention window!\n");
					exit(EXIT_FAILURE);
					break;
				}
			}
			break;
		}

		// Repeat BO
		// - If the last TX is successful, the same BO value is used, so deterministic_bo_active is set to true (1).
		// - Otherwise, a random backoff is used, so deterministic_bo_active is set to false (0)
		case BACKOFF_REPEAT_BO:{
			// "Increase or reset" is used as an indication for enabling/disabling the deterministic backoff
			switch(increase_or_reset){
				case INCREASE_CW:{
					if(*cw_stage_current < cw_stage_max){
						*cw_stage_current = *cw_stage_current + 1;
						*current_cw_min = cw_min_default * pow(2, *cw_stage_current);
						*current_cw_max = cw_max_default * pow(2, *cw_stage_current);
					}
					*deterministic_bo_active = 0;
					break;
				}
				case RESET_CW:{
					*deterministic_bo_active = 1;
					break;
				}
				default:{
					printf("ERROR in HandleContentionWindow: Unknown operation on contention window!\n");
					exit(EXIT_FAILURE);
					break;
				}
			}
			break;
		}

		// ECA (https://arxiv.org/pdf/1512.02062
		case BACKOFF_ECA:{
			// "Increase or reset" is used as an indication for enabling/disabling the deterministic backoff
			switch(increase_or_reset){
				case INCREASE_CW:{
					if(*cw_stage_current < cw_stage_max){
						*cw_stage_current = *cw_stage_current + 1;
						*current_cw_min = cw_min_default * pow(2, *cw_stage_current);
						*current_cw_max = cw_max_default * pow(2, *cw_stage_current);
					}
					*deterministic_bo_active = 0;
					break;
				}
				case RESET_CW:{
					*deterministic_bo_active = 1;
					break;
				}
				default:{
					printf("ERROR in HandleContentionWindow: Unknown operation on contention window!\n");
					exit(EXIT_FAILURE);
					break;
				}
			}
			break;
		}

		// SYNCHRONIZED BACKOFF
		case BACKOFF_SYNCHRONIZED:{
			// Do nothing
			break;
		}

		default:{
			printf("ERROR in HandleContentionWindow: Unknown backoff type.\n");
			exit(EXIT_FAILURE);
		}
	}

}


/**
* [ONLY FOR BACKOFF_TOKENIZED] Updates the ordered list of devices in the Token-based Channel Access. It is called when neighboring activity is detected.
* @param "add_or_remove" [type int]: flag indicating whether the detected device should be added or removed to/from the list
* @param "token_order_list" [type int*]: reference to "token_order_list", which contains an ordered list of identifiers of sensed neighboring devices
* @param "transmitter_id" [type int]: ID of the detected transmitter
*/
void UpdateTokenList(int add_or_remove, int **token_order_list, int transmitter_id){

	switch(add_or_remove){

		case ADD_DEVICE_TO_LIST:{
			if ((*token_order_list)[transmitter_id] == DEVICE_INACTIVE_FOR_TOKEN) {
				//printf("IF-OK token_order_list[transmitter_id] = %d\n", (*token_order_list)[transmitter_id]);
				(*token_order_list)[transmitter_id] = DEVICE_ACTIVE_FOR_TOKEN;
			}
			break;
		}

		case REMOVE_DEVICE_FROM_LIST:{
			if ((*token_order_list)[transmitter_id] == DEVICE_ACTIVE_FOR_TOKEN) {
				(*token_order_list)[transmitter_id] = DEVICE_INACTIVE_FOR_TOKEN;
			}
			break;
		}

		default: {
			printf("ERROR in UpdateTokenList: Unknown action.\n");
			exit(EXIT_FAILURE);
		}

	}

}

/**
* [ONLY FOR BACKOFF_TOKENIZED] Updates the status of the token. It is called when a new token-based transmission finishes or after a timeout.
* @param "add_or_remove" [type int]: flag indicating whether the detected device should be added or removed to/from the list
* @param "token_status" [type int]: reference to "token_status", which indicates the ID of the device currently holding the token (to be modified by this function)
* @param "transmitter_id" [type int]: ID of the detected transmitter
* @param "token_order_list" [type int*]: contains an ordered list of identifiers of sensed neighboring devices
* @param "length_order_list" [type int]: length of the token list
*/
void UpdateTokenStatus(int node_id, int take_or_release, int *token_status, int transmitter_id,
		int *token_order_list, int length_order_list, int *distance_to_token){

	switch(take_or_release){

		// Update the status of the token
		case TAKE_TOKEN:{
			//printf("------------\n");
			//printf("N%d: Node %d is TAKING the token\n", node_id, transmitter_id);
			*token_status = transmitter_id;
			//printf("= token_status = %d\n", *token_status);
			break;
		}

		// Pass the token to the next device in the list
		case RELEASE_TOKEN:{
			//printf("------------\n");
			//printf("N%d: Node %d is RELEASING the token\n", node_id, transmitter_id);
			if(token_order_list[transmitter_id] == DEVICE_ACTIVE_FOR_TOKEN){
				// Iterate over the order list to pass the token to the next active device
				for (int i = 1; i < length_order_list; ++i) {
					//printf("[(transmitter_id+i) mod length_order_list] = %d\n", (transmitter_id+i) % length_order_list);
					//printf("token_order_list[(transmitter_id+i) mod length_order_list] = %d\n", token_order_list[(transmitter_id+i) % length_order_list]);
					if(token_order_list[(transmitter_id+i) % length_order_list] == DEVICE_ACTIVE_FOR_TOKEN) {
						//printf("--> Updating the token to N%d\n", (transmitter_id+i) % length_order_list);
						*token_status = (transmitter_id+i) % length_order_list;
						break;
					}
				}
			} else {
				// Do not update the token status
			}
			//printf("= token_status = %d\n", *token_status);
			break;
		}

		default: {
			printf("ERROR in UpdateTokenStatus: Unknown action.\n");
			exit(EXIT_FAILURE);
		}

	}

	*distance_to_token = 0;
	// Update the distance of the sensing node to the token
	for(int i = 0; i < length_order_list; ++i) {
		//printf("Next inspected node = %d (absolute value in list = %d)\n", *token_status+i, (*token_status+i)%length_order_list);
		//printf("  - Condition 1: %d\n", (*token_status+i)%length_order_list != node_id);
		//printf("  - Condition 2: %d\n", token_order_list[(*token_status+i)%length_order_list] == DEVICE_ACTIVE_FOR_TOKEN);
		if( (*token_status+i)%length_order_list == node_id ) {
			break;
		} else if (token_order_list[(*token_status+i)%length_order_list] == DEVICE_ACTIVE_FOR_TOKEN ){
			*distance_to_token += 1;
		}
	}
	//printf("distance_to_token = %d\n", *distance_to_token);

}

/**
* [ONLY FOR BACKOFF_TOKENIZED] Print or write the token list for the token-based channel access
* @param "write_or_print" [type int]: indicates whether to print or write
* @param "save_node_logs" [type int]: flag indicating whether to save node logs or not
* @param "node_logger" [type Logger]: logger object to print logs into a file
* @param "token_order_list" [type int*]: contains an ordered list of identifiers of sensed neighboring devices
* @param "length_order_list" [type int]: length of the token list
*/
void PrintOrWriteTokenList(int write_or_print, int save_node_logs, Logger node_logger,
	int print_node_logs, int *token_order_list, int length_order_list){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				for(int i = 0; i < length_order_list; ++i){
					printf("%d  ", token_order_list[i]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int i = 0; i < length_order_list; ++i){
				if(save_node_logs) fprintf(node_logger.file, "%d  ", token_order_list[i]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}
