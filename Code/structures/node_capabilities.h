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
 * node_capabilities.h: this file defines the static hardware/radio capabilities of a node
 */

#ifndef _AUX_NODE_CAPABILITIES_
#define _AUX_NODE_CAPABILITIES_

#include <string>
#include "../list_of_macros.h"
#include "../methods/channel/power_channel_methods.h"
#include "logger.h"

struct NodeCapabilities
{
	std::string node_code;		///> Node code
	int node_id;				///> Node id
	double x;					///> X position coordinate
	double y;					///> Y position coordinate
	double z;					///> Z position coordinate
	int node_type;				///> Node type (e.g., AP, STA, ...)
	int primary_channel;		///> Primary channel
	int min_channel_allowed;	///> Min. allowed channel
	int max_channel_allowed;	///> Max. allowed channel
	int num_channels_allowed;	///> Maximum number of channels allowed to TX in
	double tx_power_default;	///> Default power transmission [pW]
	double sensitivity_default;	///> Default pd ("sensitivity" threshold) [pW]
	int current_max_bandwidth;	///> Selected max bandwidth [no. of channels]

	/**
	 * Function to print the node's capabilities
	 */
	void PrintCapabilities(){
		printf("%s Capabilities of node %d:\n", LOG_LVL3, node_id);
		printf("%s node_type = %d\n", LOG_LVL4, node_type);
		printf("%s position = (%.2f, %.2f, %.2f)\n", LOG_LVL4, x, y, z);
		printf("%s primary_channel = %d\n", LOG_LVL4, primary_channel);
		printf("%s min_channel_allowed = %d\n", LOG_LVL4, min_channel_allowed);
		printf("%s max_channel_allowed = %d\n", LOG_LVL4, max_channel_allowed);
		printf("%s current_max_bandwidth = %d\n", LOG_LVL4, current_max_bandwidth);
		printf("%s tx_power_default = %f pW (%f dBm)\n", LOG_LVL4, tx_power_default, ConvertPower(PW_TO_DBM, tx_power_default));
		printf("%s sensitivity_default = %f pW (%f dBm)\n", LOG_LVL4, sensitivity_default, ConvertPower(PW_TO_DBM, sensitivity_default));
		printf("\n");
	}

	/**
	 * Function to write the node's capabilities
	 * @param "logger" [type Logger]: logger object for printing logs into a file
	 * @param "sim_time" [type double]: current simulation time
	 */
	void WriteCapabilities(Logger logger, double sim_time){
		fprintf(logger.file, "%.15f;N%d;%s;%s WLAN capabilities:\n", sim_time, node_id, LOG_F00, LOG_LVL3);
		fprintf(logger.file, "%.15f;N%d;%s;%s node_type = %d\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, node_type);
		fprintf(logger.file, "%.15f;N%d;%s;%s position = (%.2f, %.2f, %.2f)\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, x, y, z);
		fprintf(logger.file, "%.15f;N%d;%s;%s primary_channel = %d\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, primary_channel);
		fprintf(logger.file, "%.15f;N%d;%s;%s min_channel_allowed = %d\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, min_channel_allowed);
		fprintf(logger.file, "%.15f;N%d;%s;%s max_channel_allowed = %d\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, max_channel_allowed);
		fprintf(logger.file, "%.15f;N%d;%s;%s current_max_bandwidth = %d\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, current_max_bandwidth);
		fprintf(logger.file, "%.15f;N%d;%s;%s tx_power_default = %f pW (%f dBm)\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, tx_power_default, ConvertPower(PW_TO_DBM, tx_power_default));
		fprintf(logger.file, "%.15f;N%d;%s;%s sensitivity_default = %f pW (%f dBm)\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, sensitivity_default, ConvertPower(PW_TO_DBM, sensitivity_default));
	}

};

#endif
