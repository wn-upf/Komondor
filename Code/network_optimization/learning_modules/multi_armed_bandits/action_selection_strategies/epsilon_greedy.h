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
 * epsilon_greedy.h: this file contains functions related to the agents' operation
 *
 *  - This file contains the methods used by the e-greedy action-selection strategy (MABs)
 */

#include "../../../../list_of_macros.h"

#ifndef _AUX_EGREEDY_
#define _AUX_EGREEDY_

/**
 * Select an action according to the epsilon-greedy strategy
 * @param "num_actions" [type int]: number of possible actions
 * @param "reward_per_arm" [type double]: array containing the last stored reward for each action
 * @param "epsilon" [type double]: current exploration coefficient
 * @return "arm_index" [type int]: index of the selected action
 */
int PickArmEgreedy(int num_actions, double *reward_per_arm, double epsilon) {

	double rand_number = ((double) rand() / (RAND_MAX));
	int arm_index;

	if (rand_number < epsilon) { //EXPLORE
		arm_index = rand() % num_actions;
//		printf("EXPLORE: arm_index = %d\n", arm_index);
	} else { //EXPLOIT
		double max = 0;
		for (int i = 0; i < num_actions; i ++) {
			if(reward_per_arm[i] >= max) {
				max = reward_per_arm[i];
				arm_index = i;
			}
		}
//		printf("EXPLOIT: arm_index = %d\n", arm_index);
	}

	return arm_index;

}

#endif
