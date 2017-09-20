/* This is just an sketch of what our Komondor headers should look like.
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
 * File description:
 *
 * - Bla bla bla...
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../list_of_macros.h"

// Exponential redefinition
double	Random( double v=1.0)	{ return v*drand48();}
int	Random( int v)	{ return (int)(v*drand48()); }
double	Exponential(double mean){ return -mean*log(Random());}

/*
 * ComputeTxTime(): computes the transmission time (just link rate) according to the number of channels used and packet lenght
 **/
double ComputeTxTime(int total_bits, double data_rate, int pdf_tx_time){

	double tx_time;

	// TODO: hardcoding rate to match CTMN analysis (MU for one channel)
	data_rate = pow(10,6);

	switch(pdf_tx_time){

		case PDF_DETERMINISTIC:{

			tx_time = total_bits/data_rate;
			break;
		}

		case PDF_EXPONENTIAL:{

			tx_time = Exponential(total_bits/data_rate);
			break;
		}

		default:{

			printf("TX time model not found!\n");
			exit(EXIT_FAILURE);
		}
	}

	// TODO: THIS IS HARDCODED
	// tx_time = 1000 * MICRO_VALUE;
	// END HARCODED

	return tx_time;
}

/*
 * ComputeNavTime: computes the NAV time for the RTS and CTS packets.
 **/
double ComputeNavTime(int node_state, double rts_duration, double cts_duration, double data_duration,
		double ack_duration, double sifs){

	double nav_time;

	switch(node_state){

		case STATE_TX_RTS:{

			// RTS duration taking into account due to trigger is set at the very begining of the RTS reception
			nav_time = 3 * sifs + rts_duration + cts_duration + data_duration + ack_duration;
			break;
		}

		case STATE_TX_CTS:{

			// CTS duration taking into account due to trigger is set at the very begining of the CTS reception
			nav_time = 2 * sifs + cts_duration + data_duration + ack_duration;

			break;
		}

		default:{

			printf("ERROR: Unreachable state\n");
			exit(EXIT_FAILURE);
		}
	}

	return nav_time;
}
