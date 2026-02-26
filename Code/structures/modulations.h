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
 * modulations.h: this file defines modulations and MCS parameters
 */

#ifndef _MCS_CONFIGURATION_
#define _MCS_CONFIGURATION_

struct Mcs_array {
   static const int modulation_bits[14];
   static const double coding_rates[14];
};

// Sergio on 5 Oct 2017
// - Include MCS indeces corresponding to IEEE 802.11ax
const int Mcs_array::modulation_bits[14] = {	// row: MCS index, column 1: bits of modulation & column 2: coding rate
	1,	// MCS 0: BPSK
	2,	// MCS 1: QPSK
	2,	// MCS 2: QPSK
	4,	// MCS 3: 16-QAM
	4,	// MCS 4: 16-QAM
	6,	// MCS 5: 64-QAM
	6,	// MCS 6: 64-QAM
	6,	// MCS 7: 64-QAM
	8,	// MCS 8: 256-QAM
	8,	// MCS 9: 256-QAM
	10,	// MCS 10: 1024-QAM
	10,	// MCS 11: 1024-QAM
	12, // MCS 12: 4096-QAM (11be)
    12  // MCS 13: 4096-QAM (11be)
};

const double Mcs_array::coding_rates[14] = {	// row: MCS index, column 1: bits of modulation & column 2: coding rate
	1/double(2),	// MCS 0
	1/double(2),	// MCS 1
	3/double(4),	// MCS 2
	1/double(2),	// MCS 3
	3/double(4),	// MCS 4
	1/double(2),	// MCS 5
	2/double(3),	// MCS 6
	3/double(4),	// MCS 7
	3/double(4),	// MCS 8
	5/double(6),	// MCS 9
	3/double(4),	// MCS 10
	5/double(6),	// MCS 11
	3/double(4),	// MCS 12
	5/double(6)		// MCS 13
};

/**
 *  Provide the number of subcarriers used for each number of channels in the IEEE 802.11ax
 *  @param "num_channels" [type int]: number of channels used for transmission
 *  @return "num_subcarriers" [type int]: number of subcarriers to be used
 */
int GetNumberSubcarriers(int num_channels){

	int num_subcarriers;

	switch(num_channels){

		// 1 channel - 20 MHz
		case 1:{
			num_subcarriers = 234;
			break;
		}

		// 2 channels - 40 MHz
		case 2:{
			num_subcarriers = 468;
			break;
		}

		// 4 channels - 80 MHz
		case 4:{
			num_subcarriers = 980;
			break;
		}

		// 8 channels - 160 MHz
		case 8:{
			num_subcarriers = 1960;
			break;
		}

		// 16 channels - 320 MHz
		case 16:{
			num_subcarriers = 3920;
			break;
		}

		default:{
			printf("ERROR: unsupported number of channels (%d)\n", num_channels);
			exit(EXIT_FAILURE);
		}

	}

	return num_subcarriers;

}

#endif
