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
 * time_methods.h: this file contains functions related to the main Komondor's operation
 *
 * - This file contains the methods related to "time" operations
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include "../../list_of_macros.h"

/**
* Compute the NAV time for the RTS and CTS packets
* @param "node_state" [type int]: node state
* @param "rts_duration" [type double]: duration of the RTS packet
* @param "cts_duration" [type double]: duration of the CTS packet
* @param "data_duration" [type double]: duration of the DATA packet
* @param "ack_duration" [type double]: duration of the ACK packet
* @param "sifs" [type double]: duration of the SIFS
* @return "nav_time" [type double]: NAV time
*/
double ComputeNavTime(int node_state, double rts_duration, double cts_duration,
	double data_duration, double ack_duration, double sifs){

	double nav_time;

	switch(node_state){

		case STATE_TX_RTS:{
			// RTS duration taking into account due to trigger is set at the very beginning of the RTS reception
			nav_time = 3 * sifs + rts_duration + cts_duration + data_duration + ack_duration;
			break;
		}

		case STATE_TX_CTS:{
			// CTS duration taking into account due to trigger is set at the very beginning of the CTS reception
			nav_time = 2 * sifs + cts_duration + data_duration + ack_duration;
			break;
		}

		case STATE_TX_DATA:{
			// DATA duration taking into account due to trigger is set at the very beginning of the DATA reception
			nav_time = sifs + data_duration + ack_duration;
			break;
		}

		case STATE_TX_ACK:{
			// ACK duration taking into account due to trigger is set at the very beginning of the ACK reception
			nav_time = ack_duration;
			break;
		}

		case STATE_TX_ICF:{
			// ICF initiates a MAPC exchange; NAV covers ICF+ICR+DATA+ACK (conservative, same formula as RTS)
			nav_time = 3 * sifs + rts_duration + cts_duration + data_duration + ack_duration;
			break;
		}

		case STATE_TX_TF:{
			// TF triggers a DATA+ACK exchange
			nav_time = sifs + data_duration + ack_duration;
			break;
		}

		default:{

			printf("ERROR: Unreachable state\n");
			exit(EXIT_FAILURE);
		}
	}

	return nav_time;

}