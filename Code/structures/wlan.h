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
 * wlan.h: this file defines a WLAN and provides basic displaying methods
 */

#ifndef _AUX_WLAN_
#define _AUX_WLAN_

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "../list_of_macros.h"
#include "logger.h"

// WLAN info
struct Wlan
{
	int wlan_id;			///> WLAN ID
	std::string wlan_code;	///> Code of the WLAN (string)
	int num_stas;			///> Number of STAs in the WLAN (AP not included)
	int ap_id;				///> Id of the Access Point
	int *list_sta_id;		///> List of STAs IDs belonging to the WLAN
	int *sta_min_channel;	///> Per-STA min channel (indexed same as list_sta_id)
	int *sta_max_channel;	///> Per-STA max channel (indexed same as list_sta_id)

	int spatial_reuse_enabled;	///> Indicates whether the SR operation is enabled or not

	// MAPC information - multi-group support
	int mapc_enabled;
	int num_mapc_groups;							///> Number of MAPC groups this WLAN belongs to
	int mapc_group_ids[MAX_MAPC_GROUPS_PER_WLAN];	///> MAPC group IDs
	int mapc_method_ids[MAX_MAPC_GROUPS_PER_WLAN];	///> MAPC scheme per group
	int mapc_num_peers[MAX_MAPC_GROUPS_PER_WLAN];	///> Peer count per group
	int *mapc_peer_ap_ids[MAX_MAPC_GROUPS_PER_WLAN];	///> Peer node_ids per group
	int mapc_txop_splits[MAX_MAPC_GROUPS_PER_WLAN];	///> TXOP split method per group
	double mapc_sr_tx_power_dbm[MAX_MAPC_GROUPS_PER_WLAN];	///> Co-SR TX power limit per group [dBm]

	/**
	 * Find index of a given group_id in this WLAN's group list.
	 * Returns index in [0, num_mapc_groups) or -1 if not found.
	 */
	int FindMapcGroupIdx(int group_id) {
		for (int g = 0; g < num_mapc_groups; ++g) {
			if (mapc_group_ids[g] == group_id) return g;
		}
		return -1;
	}

	/**
	 * Set the size of the array list_sta_id
	 * @param "num_stas" [type int]: total number of STAs
	 */
	void SetSizeOfSTAsArray(int num_stas){
		list_sta_id    = new int[num_stas];
		sta_min_channel = new int[num_stas];
		sta_max_channel = new int[num_stas];
		for(int s = 0; s < num_stas; ++s){
			list_sta_id[s]     = NODE_ID_NONE;
			sta_min_channel[s] = -1;
			sta_max_channel[s] = -1;
		}
	}

	/**
	 * Return the channel bounds declared by a specific STA.
	 * Falls back to [0, NUM_CHANNELS_KOMONDOR-1] if the STA is not found.
	 */
	void GetStaChannelBounds(int sta_id, int *min_ch, int *max_ch) {
		for (int s = 0; s < num_stas; ++s) {
			if (list_sta_id[s] == sta_id) {
				*min_ch = sta_min_channel[s];
				*max_ch = sta_max_channel[s];
				return;
			}
		}
		*min_ch = 0;
		*max_ch = NUM_CHANNELS_KOMONDOR - 1;
	}

	/**
	 * Finds an STA in the list
	 * @param "sta_id" [type int]: identifier of the STA to be found
	 */
	int FindStaInList(int sta_id){
		int found(0);
		for(int s = 0; s < num_stas; ++s){
			if(list_sta_id[s] == sta_id) found = 1;
		}
		return found;
	}

	/**
	 * Print the list of STAs IDs belonging to the BSS
	 */
	void PrintStaIds(){
		for(int s = 0; s < num_stas; s++){
			printf("%d  ", list_sta_id[s]);
		}
		printf("\n");
	}

	/**
	 * Write STAs list of IDs to an output file
	 * @param "logger" [type Logger]: logger containing the file to write on
	 */
	void WriteStaIds(Logger logger){
		if (logger.save_logs){
			for(int s = 0; s < num_stas; s++){
				fprintf(logger.file, "%d  ", list_sta_id[s]);
			}
		}
	}

	/**
	 * Print general WLAN information
	 */
	void PrintWlanInfo(){
		printf("%s WLAN %s:\n", LOG_LVL3, wlan_code.c_str());
		printf("%s wlan_id: %d\n", LOG_LVL4, wlan_id);
		printf("%s num_stas: %d\n", LOG_LVL4, num_stas);
		printf("%s ap_id: %d\n", LOG_LVL4, ap_id);
		printf("%s list of STAs IDs: ", LOG_LVL4);
		PrintStaIds();
		if (mapc_enabled) {
			printf("%s MAPC Information:\n", LOG_LVL4);
			printf("%s num_mapc_groups: %d\n", LOG_LVL5, num_mapc_groups);
			for (int g = 0; g < num_mapc_groups; ++g) {
				printf("%s Group %d: id=%d method=%d peers=%d\n",
					LOG_LVL5, g, mapc_group_ids[g], mapc_method_ids[g], mapc_num_peers[g]);
				printf("%s Peers IDs: ", LOG_LVL5);
				for (int i = 0; i < mapc_num_peers[g]; ++i)
					printf("%d ", mapc_peer_ap_ids[g][i]);
				printf("\n");
			}
		}
	}

	/**
	 * Write general WLAN info to an output file
	 * @param "logger" [type Logger]: logger containing the file to write on
	 * @param "header_string" [type std::string]: header string
	 */
	void WriteWlanInfo(Logger logger, std::string header_str){
		if (logger.save_logs){
			fprintf(logger.file, "%s WLAN %s:\n", header_str.c_str(), wlan_code.c_str());
			fprintf(logger.file, "%s - wlan_id: %d\n", header_str.c_str(), wlan_id);
			fprintf(logger.file, "%s - num_stas: %d\n", header_str.c_str(), num_stas);
			fprintf(logger.file, "%s - ap_id: %d\n", header_str.c_str(), ap_id);
			fprintf(logger.file, "%s - list of STAs IDs: ", header_str.c_str());
			WriteStaIds(logger);
			fprintf(logger.file, "\n");
			if (mapc_enabled) {
				fprintf(logger.file, "%s - MAPC Information:\n", header_str.c_str());
				fprintf(logger.file, "%s  * num_mapc_groups: %d\n", header_str.c_str(), num_mapc_groups);
				for (int g = 0; g < num_mapc_groups; ++g) {
					fprintf(logger.file, "%s  * Group %d: id=%d method=%d peers=%d\n",
						header_str.c_str(), g, mapc_group_ids[g], mapc_method_ids[g], mapc_num_peers[g]);
				}
			}
		}
	}
};

#endif
