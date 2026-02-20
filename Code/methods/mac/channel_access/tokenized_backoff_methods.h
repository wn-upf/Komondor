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
 * tokenized_backoff_methods.h: this file contains functions for the token-based backoff
 */

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

 /**
* Increase or decrease the contention window. 
* @param "distance_to_token" [type int]: distance to token (only for BACKOFF_TOKENIZED)
* @param "current_cw_min" [type int*]: pointer to the current minimum CW (to be modified by this method)
* @param "current_cw_max" [type int*]: pointer to the current maximum CW (to be modified by this method)
* @param "cw_max_default" [type int]: default maximum CW
*/
void HandleBackoffTokenized(int distance_to_token, int *current_cw_min, int *current_cw_max, int cw_max_default) {

	if (distance_to_token == 0) {
		*current_cw_min = (distance_to_token+1)*cw_max_default - cw_max_default;
		*current_cw_max = (distance_to_token+1)*cw_max_default;
	} else {
		*current_cw_min = (distance_to_token+1)*cw_max_default - cw_max_default + 1;
		*current_cw_max = (distance_to_token+1)*cw_max_default + 1;
	}

}