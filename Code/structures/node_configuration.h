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
 * File description: this is the main Komondor file
 *
 * - This file defines a CONFIGURATION and provides basic displaying methods
 */

#ifndef _AUX_CONFIGURATION_
#define _AUX_CONFIGURATION_

#include "../methods/power_channel_methods.h"

struct Capabilities
{
	std::string node_code;		// Node code
	int node_id;				// Node id
	double x;					// X position coordinate
	double y;					// Y position coordinate
	double z;					// Z position coordinate
	int node_type;				// Node type (e.g., AP, STA, ...)
	int destination_id;			// Destination node id (for nodes not belonging to any WLAN)
	double lambda;				// Average notification generation rate (related to exponential BO) [notification/s]
	double traffic_load;		// Average traffic load of the AP [packets/s]
	int ieee_protocol;			// IEEE protocol type
	int primary_channel;		// Primary channel
	int min_channel_allowed;	// Min. allowed channel
	int max_channel_allowed;	// Max. allowed channel
	int num_channels_allowed;	// Maximum number of channels allowed to TX in
	double tx_power_min;				// Min. power transmission [pW]
	double tx_power_default;			// Default power transmission [pW]
	double tx_power_max;				// Max. power transmission [pW]
	double sensitivity_min;				// Min. pd	("sensitivity" threshold) [pW]
	double sensitivity_default;			// Default pd	("sensitivity" threshold) [pW]
	double sensitivity_max;				// Max. pd ("sensitivity" threshold)
	double tx_gain;				// Antenna transmission gain [linear]
	double rx_gain;				// Antenna reception gain [linear]
	int current_dcb_policy;	// Selected DCB policy
	int modulation_default;		// Default modulation

	// Function to print the node's capabilities
	void PrintCapabilities(){

		printf("%s Capabilities of node %d:\n", LOG_LVL3, node_id);
		printf("%s node_type = %d\n", LOG_LVL4, node_type);
		printf("%s position = (%.2f, %.2f, %.2f)\n", LOG_LVL4, x, y, z);
		printf("%s primary_channel = %d\n", LOG_LVL4, primary_channel);
		printf("%s min_channel_allowed = %d\n", LOG_LVL4, min_channel_allowed);
		printf("%s max_channel_allowed = %d\n", LOG_LVL4, max_channel_allowed);
		printf("%s current_dcb_policy = %d\n", LOG_LVL4, current_dcb_policy);
		printf("%s lambda = %f packets/s\n", LOG_LVL4, lambda);
		printf("%s traffic_load = %.2f packets/s\n", LOG_LVL4, traffic_load);
		printf("%s destination_id = %d\n", LOG_LVL4, destination_id);
		printf("%s tx_power_min = %f pW (%f dBm)\n", LOG_LVL4, tx_power_min, ConvertPower(PW_TO_DBM, tx_power_min));
		printf("%s tx_power_default = %f pW (%f dBm)\n", LOG_LVL4, tx_power_default, ConvertPower(PW_TO_DBM, tx_power_default));
		printf("%s tx_power_max = %f pW (%f dBm)\n", LOG_LVL4, tx_power_max, ConvertPower(PW_TO_DBM, tx_power_max));
		printf("%s sensitivity_min = %f pW (%f dBm)\n", LOG_LVL4, sensitivity_min, ConvertPower(PW_TO_DBM, sensitivity_min));
		printf("%s sensitivity_default = %f pW (%f dBm)\n", LOG_LVL4, sensitivity_default, ConvertPower(PW_TO_DBM, sensitivity_default));
		printf("%s sensitivity_max = %f pW (%f dBm)\n", LOG_LVL4, sensitivity_max, ConvertPower(PW_TO_DBM, sensitivity_max));
		printf("%s tx_gain = %f (%f dBi)\n", LOG_LVL4, tx_gain, ConvertPower(LINEAR_TO_DB, tx_gain));
		printf("%s rx_gain = %f (%f dBi)\n", LOG_LVL4, rx_gain, ConvertPower(LINEAR_TO_DB, rx_gain));
		printf("%s modulation_default = %d\n", LOG_LVL4, modulation_default);

		printf("\n");

	}

	// Function to write the node's capabilities
	void WriteCapabilities(Logger logger, double sim_time){
		fprintf(logger.file, "%.15f;CC;%s;%s WLAN capabilities:\n", sim_time, LOG_F00, LOG_LVL3);
		fprintf(logger.file, "%.15f;CC;%s;%s node_type = %d\n",
			sim_time, LOG_F00, LOG_LVL4, node_type);
		fprintf(logger.file, "%.15f;CC;%s;%s position = (%.2f, %.2f, %.2f)\n",
			sim_time, LOG_F00, LOG_LVL4, x, y, z);
		fprintf(logger.file, "%.15f;CC;%s;%s primary_channel = %d\n",
			sim_time, LOG_F00, LOG_LVL4, primary_channel);
		fprintf(logger.file, "%.15f;CC;%s;%s min_channel_allowed = %d\n",
			sim_time, LOG_F00, LOG_LVL4, min_channel_allowed);
		fprintf(logger.file, "%.15f;CC;%s;%s max_channel_allowed = %d\n",
			sim_time, LOG_F00, LOG_LVL4, max_channel_allowed);
		fprintf(logger.file, "%.15f;CC;%s;%s current_dcb_policy = %d\n",
			sim_time, LOG_F00, LOG_LVL4, current_dcb_policy);
		fprintf(logger.file, "%.15f;CC;%s;%s lambda = %f packets/s\n",
			sim_time, LOG_F00, LOG_LVL4, lambda);
		fprintf(logger.file, "%.15f;CC;%s;%s traffic_load = %.2f packets/s\n",
			sim_time, LOG_F00, LOG_LVL4, traffic_load);
		fprintf(logger.file, "%.15f;CC;%s;%s destination_id = %d\n",
			sim_time, LOG_F00, LOG_LVL4, destination_id);
		fprintf(logger.file, "%.15f;CC;%s;%s tx_power_min = %f pW (%f dBm)\n",
			sim_time, LOG_F00, LOG_LVL4, tx_power_min, ConvertPower(PW_TO_DBM, tx_power_min));
		fprintf(logger.file, "%.15f;CC;%s;%s tx_power_default = %f pW (%f dBm)\n",
			sim_time, LOG_F00, LOG_LVL4, tx_power_default, ConvertPower(PW_TO_DBM, tx_power_default));
		fprintf(logger.file, "%.15f;CC;%s;%s tx_power_max = %f pW (%f dBm)\n",
			sim_time, LOG_F00, LOG_LVL4, tx_power_max, ConvertPower(PW_TO_DBM, tx_power_max));
		fprintf(logger.file, "%.15f;CC;%s;%s sensitivity_min = %f pW (%f dBm)\n",
			sim_time, LOG_F00, LOG_LVL4, sensitivity_min, ConvertPower(PW_TO_DBM, sensitivity_min));
		fprintf(logger.file, "%.15f;CC;%s;%s sensitivity_default = %f pW (%f dBm)\n",
			sim_time, LOG_F00, LOG_LVL4, sensitivity_default, ConvertPower(PW_TO_DBM, sensitivity_default));
		fprintf(logger.file, "%.15f;CC;%s;%s sensitivity_max = %f pW (%f dBm)\n",
			sim_time, LOG_F00, LOG_LVL4, sensitivity_max, ConvertPower(PW_TO_DBM, sensitivity_max));
		fprintf(logger.file, "%.15f;CC;%s;%s tx_gain = %f (%f dBi)\n",
			sim_time, LOG_F00, LOG_LVL4, tx_gain, ConvertPower(LINEAR_TO_DB, tx_gain));
		fprintf(logger.file, "%.15f;CC;%s;%s rx_gain = %f (%f dBi)\n",
			sim_time, LOG_F00, LOG_LVL4, rx_gain, ConvertPower(LINEAR_TO_DB, rx_gain));
		fprintf(logger.file, "%.15f;CC;%s;%s modulation_default = %d\n",
			sim_time, LOG_F00, LOG_LVL4, modulation_default);
	}

};

// Node's configuration
struct Configuration
{
	double timestamp;

	int selected_primary_channel;		// Selected primary channel
	double selected_pd;		// Selected pd ("sensitivity" threshold) [pW]
	double selected_tx_power;	// Selected Tx Power [pW]
	int selected_dcb_policy;	// Selected DCB policy

	// Spatial reuse
	int spatial_reuse_enabled;	// Indicates whether the SR operation is enabled or not
	int bss_color;				// BSS color identifier
	int srg;					// Spatial Reuse Group (SRG) identifier
	double non_srg_obss_pd; 	// Threshold to be used for non-SRG transmissions
	double srg_obss_pd; 		// Threshold to be used for SRG transmissions

	Capabilities capabilities;

	// Function to print the node's configuration
	void PrintConfiguration(int origin){

		if (origin == ORIGIN_AGENT) {
			printf("%s Recommended configuration by the agent:\n", LOG_LVL3);
		} else if (origin == ORIGIN_AP) {
			printf("%s Current configuration of the WLAN:\n", LOG_LVL3);
		} else {
			printf("ERROR: bad origin\n");
		}
		printf("%s selected_primary = %d\n", LOG_LVL4, selected_primary_channel);
		printf("%s pd_default = %f pW (%f dBm)\n", LOG_LVL4, selected_pd, ConvertPower(PW_TO_DBM, selected_pd));
		printf("%s tx_power_default = %f pW (%f dBm)\n", LOG_LVL4, selected_tx_power, ConvertPower(PW_TO_DBM, selected_tx_power));
		printf("%s selected_dcb_policy = %d\n", LOG_LVL4, selected_dcb_policy);
		printf("\n");

	}

	// Function to write the node's configuration
	void WriteConfiguration(Logger logger, double sim_time){
		fprintf(logger.file, "%.15f;CC;%s;%s WLAN configuration:\n", sim_time, LOG_F00, LOG_LVL3);
		fprintf(logger.file, "%.15f;CC;%s;%s selected_primary = %d\n",
			sim_time, LOG_F00, LOG_LVL4, selected_primary_channel);
		fprintf(logger.file, "%.15f;CC;%s;%s pd_default = %f pW (%f dBm)\n",
			sim_time, LOG_F00, LOG_LVL4, selected_pd, ConvertPower(PW_TO_DBM, selected_pd));
		fprintf(logger.file, "%.15f;CC;%s;%s tx_power_default = %f pW (%f dBm)\n",
			sim_time, LOG_F00, LOG_LVL4, selected_tx_power, ConvertPower(PW_TO_DBM, selected_tx_power));
		fprintf(logger.file, "%.15f;CC;%s;%s selected_dcb_policy = %d\n",
			sim_time, LOG_F00, LOG_LVL4, selected_dcb_policy);
	}

};

#endif
