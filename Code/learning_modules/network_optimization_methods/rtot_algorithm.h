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
 * rtot_algorithm.h: this file contains functions related to the agents' operation
 *
 * - This file contains the implementation of the RTOT algorithm provided in "Ropitault, T. (2018, January).
 *  Evaluation of RTOT algorithm: A first implementation of OBSS_PD-based SR method for IEEE 802.11 ax.
 *  In 2018 15th IEEE Annual Consumer Communications & Networking Conference (CCNC) (pp. 1-7)."
 *  (IEEE.https://ieeexplore.ieee.org/abstract/document/8319274/)
 */

#include "../../list_of_macros.h"

#ifndef _AUX_RTOT_ALG_
#define _AUX_RTOT_ALG_

class RtotAlgorithm {

	// Public items
	public:

		int num_stas;				///> Number of STAs associated to the WLAN
		double *rssi_per_sta;		///> RSSI received at each STA from the AP
		double *obss_pd_per_sta;	///> List of OBSS/PD thresholds to be used for each STA in the WLAN

		double margin_rtot;			///> Margin parameter in the RTOT algorithm

	// Private items
	private:

	// Methods
	public:

		/******************/
		/******************/
		/*  MAIN METHODS  */
		/******************/
		/******************/

		/**
		* Updates the configuration to be used by each WLAN
		* @param "configuration_array" [type Configuration*]: array of configurations of each AP (to be updated by this method)
		*/
		double UpdateObssPd(double rssi) {
			return ComputeObssPdRtotAlgorithm(rssi, margin_rtot);
		}

		/**
		* Updates the configuration to be used by each WLAN
		* @param "configuration_array" [type Configuration*]: array of configurations of each AP (to be updated by this method)
		*/
		double ComputeObssPdRtotAlgorithm(double rssi, double margin_rtot) {

			// Compute the OBSS/PD to be used based on the RSSI
			double obss_pd_dbm(0);
			obss_pd_dbm = floor(ConvertPower(PW_TO_DBM, rssi) - ConvertPower(PW_TO_DBM, margin_rtot));
			if (obss_pd_dbm > OBSS_PD_MAX) obss_pd_dbm = OBSS_PD_MAX;
			else if (obss_pd_dbm < OBSS_PD_MIN) obss_pd_dbm = OBSS_PD_MIN;

			printf("[RTOT] New OBSS/PD computed = %f dBm\n", floor(obss_pd_dbm));

			return ConvertPower(DBM_TO_PW, obss_pd_dbm);

		}

		/***********************/
		/***********************/
		/*  AUXILIARY METHODS  */
		/***********************/
		/***********************/

		/**
		* Initialize the variables used by the RTOT algorithm
		*/
		void InitializeVariables(){
			rssi_per_sta = new double[num_stas];
			obss_pd_per_sta = new double[num_stas];
			for (int i = 0 ; i < num_stas ; ++ i) {
				rssi_per_sta[i] = 0;
				obss_pd_per_sta[i] = 0;
			}
		}

		/**
		* Print or write the statistics of the RTOT algorithm
		* @param "write_or_print" [type int]: variable to indicate whether to print on the  console or to write on the the output logs file
		* @param "logger" [type Logger]: logger object to write on the output file
		*/
		void PrintOrWriteInformation(int write_or_print, Logger &logger) {
			// Write or print according the input parameter "write_or_print"
			switch(write_or_print){
				// Print logs in console
				case PRINT_LOG:{
					printf("%s RTOT algorithm information...\n", LOG_LVL1);
					printf("%s num_stas = %d\n", LOG_LVL2, num_stas);
					printf("%s margin_rtot = %f\n", LOG_LVL2, ConvertPower(PW_TO_DBM, margin_rtot));
					break;
				}
				// Write logs in agent's output file
				case WRITE_LOG:{
					fprintf(logger.file, "RTOT algorithm statistics...\n");
					break;
				}
			}
		}

		/**
		* Print or write the statistics of the RTOT algorithm
		* @param "write_or_print" [type int]: variable to indicate whether to print on the  console or to write on the the output logs file
		* @param "logger" [type Logger]: logger object to write on the output file
		*/
		void PrintOrWriteStatistics(int write_or_print, Logger &logger) {
			// Write or print according the input parameter "write_or_print"
			switch(write_or_print){
				// Print logs in console
				case PRINT_LOG:{
					printf("%s RTOT algorithm statistics...\n", LOG_LVL1);
					printf("%s rssi_per_sta: ", LOG_LVL2);
					for (int i = 0 ; i < num_stas; ++i) {
						printf("%f dBm ", ConvertPower(PW_TO_DBM, rssi_per_sta[i]));
					}
					printf("\n");
					printf("%s obss_pd_per_sta: ", LOG_LVL2);
					for (int i = 0 ; i < num_stas; ++i) {
						printf("%f dBm ", ConvertPower(PW_TO_DBM, obss_pd_per_sta[i]));
					}
					printf("\n");
					break;
				}
				// Write logs in agent's output file
				case WRITE_LOG:{
					fprintf(logger.file, "RTOT algorithm statistics...\n");
					break;
				}
			}
		}


};

#endif
