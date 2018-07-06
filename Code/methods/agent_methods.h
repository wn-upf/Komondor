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
 * File description: this file contains functions related to the agents' operation
 *
 * - This file contains the methods related to "time" operations
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include <iostream>

#include "../list_of_macros.h"

#ifndef _AUX_AGENT_METHODS_
#define _AUX_AGENT_METHODS_

/*********************/
/*********************/
/*  MABs (E-GREEDY)  */
/*********************/
/*********************/

/*
 * PickArm():
 */
int PickArmEgreedy(int num_actions, double *reward_per_arm, double epsilon) {

	//int num_actions = sizeof(reward_per_arm)/sizeof(reward_per_arm[0]);

	double rand_number = ((double) rand() / (RAND_MAX));

	//printf("Epsilon = %f - rand_number = %f\n", epsilon, rand_number);

	int arm_index;

	if (rand_number < epsilon) {
		//EXPLORE
		arm_index = rand() % num_actions;
		//printf("EXPLORE: arm_index = %d\n", arm_index);
	} else {
		//EXPLOIT
		double max = 0;
		for (int i = 0; i < num_actions; i ++) {
			// printf("reward_per_arm[%d] = %f\n",i,reward_per_arm[i]);
			if(reward_per_arm[i] >= max) {
				max = reward_per_arm[i];
				arm_index = i;
			}
		}

		//printf("EXPLOIT: arm_index = %d\n", arm_index);
	}


	return arm_index;

}

/***********************/
/***********************/
/*  AUXILIARY METHODS  */
/***********************/
/***********************/

/*
 * index2values(): given different lists of parameters, outputs the index in each list
 * according to the index of the joint action (which considers each parameter)
 * INPUT:
 * 	- indexes: array we want to fill (3 positions, one for each parameter - channel, CCA, Tx power)
 * 	- action_ix: index of the action, which represents a combination of channel, CCA and tx power
 * 	- size_channels: size of channels possibilities
 * 	- size_cca: size of CCA possibilities
 * 	- size_tx_power: size of Tx power possibilities
 * 	- size_dcb_policy: size of the DCB policy possibilities
 * OUTPUT:
 *  - fills "indexes", which indicates the index of the parameter in each of the lists
 */
void index2values(int *indexes, int action_ix, int size_channels, int size_cca, int size_tx_power, int size_dcb_policy) {

//	// Determine channel index
//	int channel_ix = (ix+1) % size_channels;
//	if (channel_ix == 0) { channel_ix = size_channels; }
////	printf("channel_ix = %d\n",channel_ix-1);
//	// Determine CCA index
//	int y = (ix+1) % (size_channels * size_cca);
//	double cca_ix = ceil((double)y/(double)size_channels);
//	if ((int)cca_ix == 0) { cca_ix = size_cca; }
////	printf("cca_ix = %d\n",(int)cca_ix-1);
//	// Determine Tx Power index
//	double tx_power_ix = ceil((double)(ix+1) / (double)(size_channels * size_cca));
//	if (tx_power_ix > size_tx_power) { tx_power_ix = size_tx_power; }
////	printf("tx_power_ix = %d\n",(int)tx_power_ix-1);
//	// Determine DCB policy index
//	int dcb_policy_ix = (ix+1) % size_dcb_policy;
//	if (dcb_policy_ix == 0) { dcb_policy_ix = size_dcb_policy; }
//
//	indexes[0] = channel_ix-1;
//	indexes[1] = cca_ix-1;
//	indexes[2] = (int)tx_power_ix-1;
//	indexes[3] = dcb_policy_ix-1;


	indexes[0] = (int) action_ix/(size_cca * size_tx_power * size_dcb_policy);
	indexes[1] = (int) (action_ix - indexes[0] * (size_cca * size_tx_power * size_dcb_policy))
			/(size_tx_power * size_dcb_policy);
	indexes[2] = (int) (action_ix -  indexes[0] * (size_cca * size_tx_power * size_dcb_policy)
			- indexes[1] * (size_tx_power * size_dcb_policy))
			/(size_dcb_policy);
	indexes[3] = action_ix % size_dcb_policy;

}

void index2valuesADCB(int *indexes, int action_ix) {

	indexes[0] = (int) action_ix/(3 * 4);
	indexes[1] = (int) (action_ix - indexes[0] * (3*4)) / 4;
	indexes[2] = action_ix % 4;

}

/*
 * values2index(): given different indexes of actions, outputs the index of the
 * joint action (which represents a combination of each parameter)
 * INPUT:
 * 	- indexes: array of 3 positions, one for each parameter - channel, CCA, Tx power)
 * 	- size_channels: size of channels possibilities
 * 	- size_cca: size of CCA possibilities
 *	- NOTE: size of tx power elements is not necessary
 * OUTPUT:
 *  - index: index of the action, which represents a combination of channel, CCA and tx power
 */
//int values2index(int *indexes, int size_channels, int size_cca) {
//
//	int index = (indexes[0] + 1) + indexes[1] * size_channels +
//			indexes[2] * size_channels * size_cca -1;
//
//	return index;
//
//}

int values2index(int *indexes, int size_channels, int size_cca, int size_tx_power, int size_dcb_policy) {

	int index = indexes[0] * (size_cca * size_tx_power * size_dcb_policy)
						+ indexes[1] * (size_tx_power * size_dcb_policy)
						+ indexes[2] * size_dcb_policy
						+ indexes[3];

	return index;

}

int values2indexADCB(int *indexes) {

	int index = indexes[0] * (3 * 4)
						+ indexes[1] * (4)
						+ indexes[2];

	return index;

}

/*
 * FindIndexesOfConfiguration(): given a configuration, fills the "indexes_selected_arm", which
 * includes the index of each parameter
 * INPUT:
 * 	- indexes_selected_arm: array to be filled
 * 	- configuration: configuration of the AP
 * 	- size_channels: size of channels possibilities
 * 	- size_cca: size of CCA possibilities
 * 	- size_tx_power: size of Tx power possibilities
 * 	- list_of_channels: list of possible channels
 * 	- list_of_cca_values: list of CCA values
 * 	- list_of_tx_power_values: list of tx power values
 */
void FindIndexesOfConfiguration(int *indexes_selected_arm, Configuration &configuration,
		int size_channels, int size_cca, int size_tx_power, int size_dcb_policy, int *list_of_channels,
		double *list_of_cca_values, double *list_of_tx_power_values, int *list_of_dcb_policy) {

	int index_channel = -1;
	int index_cca = -1;
	int index_tx_power = -1;
	int index_dcb_policy = -1;

	// Channel
	//printf("Channels list: ");
	for(int i = 0; i < size_channels; i ++) {
		//printf("%d ",list_of_channels[i]);
		if(configuration.selected_primary_channel == list_of_channels[i]) {
			index_channel = i;
			break;
		}
	}
	//printf("\n");
	// CCA
	//printf("CCA list: ");
	for(int i = 0; i < size_cca; i ++) {
		//printf("%f ",ConvertPower(PW_TO_DBM,list_of_cca_values[i]));
		if(configuration.selected_cca == list_of_cca_values[i]) {
			index_cca = i;
			break;
		}
	}
	//printf("\n");
	// Tx Power
	//printf("TX Power list: ");
	for(int i = 0; i < size_tx_power; i ++) {
		//printf("%f ",ConvertPower(PW_TO_DBM,list_of_tx_power_values[i]));
		if(configuration.selected_tx_power == list_of_tx_power_values[i]) {
			index_tx_power = i;
			break;
		}
	}
	//printf("\n");
	// DCB policy
	for(int i = 0; i < size_dcb_policy; i ++) {
		if(configuration.selected_dcb_policy == list_of_dcb_policy[i]) {
			index_dcb_policy = i;
			break;
		}
	}
	//printf("\n");

	indexes_selected_arm[0] = index_channel;
	indexes_selected_arm[1] = index_cca;
	indexes_selected_arm[2] = index_tx_power;
	indexes_selected_arm[3] = index_dcb_policy;

	//printf("Indexes: %d, %d, %d\n", indexes_selected_arm[0],indexes_selected_arm[1],indexes_selected_arm[2]);

}

void FindIndexesOfConfigurationADCB(int *indexes_selected_arm, Configuration &configuration) {

	indexes_selected_arm[0] = configuration.a_s2;
	indexes_selected_arm[1] = configuration.a_s3;
	indexes_selected_arm[2] = configuration.a_s4;

	//printf("Indexes: %d, %d, %d\n", indexes_selected_arm[0],indexes_selected_arm[1],indexes_selected_arm[2]);

}


/*
 * printOrWriteNodesTransmitting: prints (or writes) the array representing the transmitting nodes.
 */
void PrintOrWriteRewardPerArm(int write_or_print, int save_agent_logs,
		int print_agent_logs, Logger agent_logger, int size_actions,
		double *reward_per_arm, double *cumulative_reward_per_arm,
		int *times_arm_has_been_selected, int agent_id, double sim_time) {

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_agent_logs){
				printf("Reward per arm: ");
				for(int n = 0; n < size_actions; n++){
					printf("%f  ", reward_per_arm[n]);
				}
				printf("\nCumulative reward per arm: ");
				for(int n = 0; n < size_actions; n++){
					printf("%f  ", cumulative_reward_per_arm[n]);
				}
				printf("\nTimes each arm has been selected: ");
				for(int n = 0; n < size_actions; n++){
					printf("%d  ", times_arm_has_been_selected[n]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s Reward per arm: ",
				sim_time, agent_id, LOG_C00, LOG_LVL3);
			for(int n = 0; n < size_actions; n++){
				 if(save_agent_logs){
					 fprintf(agent_logger.file, "%f  ", reward_per_arm[n]);
				 }
			}
			if(save_agent_logs) fprintf(agent_logger.file, "\n%.15f;A%d;%s;%s Cumulative reward per arm: ",
				sim_time, agent_id, LOG_C00, LOG_LVL3);
			for(int n = 0; n < size_actions; n++){
				 if(save_agent_logs){
					 fprintf(agent_logger.file, "%f  ", cumulative_reward_per_arm[n]);
				 }
			}
			fprintf(agent_logger.file, "\n%.15f;A%d;%s;%s Times each arm has been selected: ",
							sim_time, agent_id, LOG_C00, LOG_LVL3);
			for(int n = 0; n < size_actions; n++){
				if(save_agent_logs){
					fprintf(agent_logger.file, "%d ", times_arm_has_been_selected[n]);
				}
			}
			if(save_agent_logs) fprintf(agent_logger.file, "\n");
			break;
		}
	}
}


#endif
