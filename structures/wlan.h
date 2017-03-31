/* TODO: DEFINE copyright headers.*/

/* This is just an skecth of what our Komondor headers should look like.
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
 * File description: Wlan contains the information corresponding to the set of
 * nodes composing a WLAN. Is is composed of 1 AP and multiple STAs.
 *
 * - Bla bla bla...
 */


#ifndef _AUX_WLAN_
#define _AUX_WLAN_

#include "../list_of_macros.h"
#include "logger.h"

// WLAN info
struct Wlan
{
	int wlan_id;		// WLAN ID
	char *wlan_code;	// Code of the WLAN (string)
	int num_stas;		// Number of STAs in the WLAN (AP not included)
	int ap_id;			// Id of the Access Point
	int *list_sta_id;	// List of STAs IDs belonging to the WLAN

	/*
	 * setSizeOfSTAsArray(): sets the size of the array list_sta_id
	 */
	void setSizeOfSTAsArray(int num_stas){

		list_sta_id = (int *) malloc(num_stas * sizeof(*list_sta_id));

		for(int s = 0; s < num_stas; s++){
			list_sta_id[s] = NODE_ID_NONE;
		}
	}

	/*
	 * printStaIds(): prints the list of STAs IDs belonging to the WLAN
	 */
	void printStaIds(){

		for(int s = 0; s < num_stas; s++){
			printf("%d  ", list_sta_id[s]);
		}
		printf("\n");
	}


	/*
	 * writeStaIds(): writes STAs list of IDs in a given file
	 * Input arguments:
	 * - logger: logger containing the file to write on
	 */
	void writeStaIds(Logger logger){

		if (logger.save_logs){
			for(int s = 0; s < num_stas; s++){
				fprintf(logger.file, "%d  ", list_sta_id[s]);
			}
		}
	}

	/*
	 * printWlanInfo(): prints general WLAN info
	 */
	void printWlanInfo(){

		printf("%s WLAN %s:\n", LOG_LVL3, wlan_code);
		printf("%s wlan_id: %d\n", LOG_LVL4, wlan_id);
		printf("%s num_stas: %d\n", LOG_LVL4, num_stas);
		printf("%s ap_id: %d\n", LOG_LVL4, ap_id);
		printf("%s list of STAs IDs: ", LOG_LVL4);
		printStaIds();
	}

	/*
	 * writeWlanInfo(): writes general WLAN info in a given file
	 * Input arguments:
	 * - logger: logger containing the file to write on
	 * - header_string: header string
	 */
	void writeWlanInfo(Logger logger, char *header_string){

		if(header_string == NULL){
			header_string = (char *) malloc(1);
			sprintf(header_string, " ");
		}

		if (logger.save_logs){
			fprintf(logger.file, "%s WLAN %s:\n", header_string, wlan_code);
			fprintf(logger.file, "%s - wlan_id: %d\n", header_string, wlan_id);
			fprintf(logger.file, "%s - num_stas: %d\n", header_string, num_stas);
			fprintf(logger.file, "%s - ap_id: %d\n", header_string, ap_id);
			fprintf(logger.file, "%s - list of STAs IDs: ", header_string);
			writeStaIds(logger);
			fprintf(logger.file, "\n");
		}
	}
};

#endif
