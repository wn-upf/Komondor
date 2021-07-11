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
   static const double mcs_array[4][12];
   static const double coding_rate_array[12];
   static const int bits_per_symbol_modulation_array[12];
   static const int modulation_bits[12];
   static const double coding_rates[12];
};

// THIS IS DEPRECATED
const double Mcs_array::mcs_array[4][12] = {	// rows: modulation type, colums: number of channels (1, 2, 4, 8)
	{4* pow(10,6),16* pow(10,6),24* pow(10,6),33* pow(10,6),49* pow(10,6),65* pow(10,6),73* pow(10,6),81* pow(10,6),
			98* pow(10,6),108* pow(10,6),122* pow(10,6),135* pow(10,6)},
	{8* pow(10,6),33* pow(10,6),49* pow(10,6),65* pow(10,6),98* pow(10,6),130* pow(10,6),146* pow(10,6),163* pow(10,6),
			195* pow(10,6),217* pow(10,6),244* pow(10,6),271* pow(10,6)},
	{17* pow(10,6),68* pow(10,6),102* pow(10,6),136* pow(10,6),204* pow(10,6),272* pow(10,6),306* pow(10,6),
			340*pow(10,6),408* pow(10,6),453* pow(10,6),510 * pow(10,6),567 * pow(10,6)},
	{34 * pow(10,6),136 * pow(10,6),204 * pow(10,6),272 * pow(10,6),408 * pow(10,6),544 * pow(10,6),613 * pow(10,6),
			681 * pow(10,6),817 * pow(10,6),907 * pow(10,6),1021 * pow(10,6),1134 * pow(10,6)}};

const double Mcs_array::coding_rate_array[12] = {1/double(2), 1/double(2), 3/double(4), 1/double(2),
	3/double(4), 2/double(3), 3/double(4), 5/double(6), 3/double(4), 5/double(6), 3/double(4), 5/double(6)};

const int Mcs_array::bits_per_symbol_modulation_array[12] = {2, 4, 4, 16, 16, 64, 64, 64, 256, 256, 1024, 1024};

// Sergio on 5 Oct 2017
// - Include MCS indeces corresponding to IEEE 802.11ax
const int Mcs_array::modulation_bits[12] = {	// row: MCS index, column 1: bits of modulation & column 2: coding rate
	1,	// MCS 0
	2,	// MCS 1
	2,	// MCS 2
	4,	// MCS 3
	4,	// MCS 4
	6,	// MCS 5
	6,	// MCS 6
	6,	// MCS 7
	8,	// MCS 8
	8,	// MCS 9
	10,	// MCS 10
	10	// MCS 11
};

const double Mcs_array::coding_rates[12] = {	// row: MCS index, column 1: bits of modulation & column 2: coding rate
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
	5/double(6)		// MCS 11
};

/**
 *  Provide the number of subcarriers used for each number of channels in the IEEE 802.11ax
 *  @param "num_channels" [type int]: number of channels used for transmission
 *  @return "num_subcarriers" [type int]: number of subcarriers to be used
 */
int getNumberSubcarriers(int num_channels){

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

	}

	return num_subcarriers;

}

#endif
