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
 * agent_capabilities.h: this file defines the capabilities of a learning agent
 */

#ifndef _AUX_AGENT_CAPABILITIES_
#define _AUX_AGENT_CAPABILITIES_

#include "../list_of_macros.h"
#include "logger.h"

struct AgentCapabilities
{
	int agent_id;					///> Agent identifier
	double time_between_requests;	///> Time between requests to the AP [s]
	int num_arms;					///> Number of actions (arms) available to the agent
	int *available_actions;			///> Array of available action indices

	/**
	 * Set the size of the available actions array
	 * @param "num_arms" [type int]: total number of actions
	 */
	void SetSizeOfActionsArray(int num_arms){
		this->num_arms = num_arms;
		available_actions = new int[num_arms];
	}

	/**
	 * Function to print the agent's capabilities
	 */
	void PrintAgentCapabilities(){
		printf("%s Information of agent %d:\n", LOG_LVL3, agent_id);
		printf("%s time_between_requests = %f\n", LOG_LVL4, time_between_requests);
		printf("\n");
	}

	/**
	 * Function to write the agent's capabilities
	 * @param "logger" [type Logger]: logger object for printing logs into a file
	 * @param "sim_time" [type double]: current simulation time
	 */
	void WriteAgentCapabilities(Logger logger, double sim_time){
		fprintf(logger.file, "%.15f;A%d;%s;%s Agent information:\n", sim_time, agent_id, LOG_F00, LOG_LVL3);
		fprintf(logger.file, "%.15f;A%d;%s;%s time_between_requests = %f\n",
			sim_time, agent_id, LOG_F00, LOG_LVL4, time_between_requests);
	}
};

#endif
