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
 * - This file defines a WLAN and provides basic displaying methods
 */

#include "../list_of_macros.h"
#include "logger.h"

// Action info
struct Action
{
	int channel;	// Channel selected
	double cca;		// CCA level
	double tx_power;	// Tx Power
	int dcb_policy;
//	int *list_sta_id;	// List of STAs IDs belonging to the WLAN
//
//	/*
//	 * SetSizeOfSTAsArray(): sets the size of the array list_sta_id
//	 */
//	void SetSizeOfSTAsArray(int num_stas){
//
//		list_sta_id = (int *) malloc(num_stas * sizeof(*list_sta_id));
//
//		for(int s = 0; s < num_stas; s++){
//			list_sta_id[s] = NODE_ID_NONE;
//		}
//	}

	/*
	 * PrintStaIds(): prints the list of STAs IDs belonging to the WLAN
	 */
	void PrintAction(){

		printf("------------\n Action:\n");
		printf(" * channel = %d\n", channel);
		printf(" * cca = %f\n", cca);
		printf(" * tx_power = %f\n", tx_power);
		printf(" * dcb_policy = %d\n", dcb_policy);
		printf("------------\n");
	}


};
