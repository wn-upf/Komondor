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
 * File description: this file contains functions related to the agents' operation
 *
 * - This file contains the methods related to "time" operations
 */

#include "../../list_of_macros.h"
//#include "action_selection_strategies/thompson_sampling.h"

#ifndef _AUX_GRAPH_COLORING_
#define _AUX_GRAPH_COLORING_

class GraphColoring {

	// Public items
	public:

		int save_controller_logs;
		int print_controller_logs;

		int agents_number;
		int num_channels;

	// Private items
	private:

		// Generic variables to all the learning strategies
		int initial_reward;
		int num_iterations;

	// Methods
	public:

		/******************/
		/******************/
		/*  MAIN METHODS  */
		/******************/
		/******************/

		/*
		 * UpdateConfiguration(): main method for updating the configuration according to past experience
		* INPUT:
		 * 	- configuration: current configuration report from the AP
		 * 	- performance: current performance report from the AP
		 * 	- central_controller_logger: logger object to write logs
		 * 	- sim_time: current simulation time
		 * OUTPUT:
		 *  - suggested_configuration: configuration suggested to the AP
		 */
		void UpdateConfiguration(Configuration **configuration_array, Performance *performance_array,
			Logger &central_controller_logger, double sim_time) {

			// Find the action ix according to the AP's configuration
			int new_primary (0);

			// Update the RSSI table
			for (int i = 0; i < agents_number; ++ i) {

				// ...
				new_primary = i % num_channels;
				// Update the configuration
				configuration_array[i]->selected_primary_channel = new_primary;	// Primary

			}

			if(save_controller_logs) fprintf(central_controller_logger.file, "%.15f;%s;%s GraphColoring: "
				"New configurations provided\n", sim_time, LOG_C00, LOG_LVL2);

		};

		/***********************/
		/***********************/
		/*  AUXILIARY METHODS  */
		/***********************/
		/***********************/

		/*
		 * InitializeVariables(): initializes the variables used by the MAB
		 */
		void InitializeVariables(){

			initial_reward = 0;
			num_iterations = 1;

		}

};

#endif
