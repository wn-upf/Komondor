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
 * ml_model.h: this file contains functions related to the agents' operation
 *
 *  - This file contains the methods used by the ML Model in the Machine Learning (ML) pipeline.
 * 	 In particular, this module applies the actual ML operation
 */

#include "../list_of_macros.h"

#include "../structures/node_configuration.h"
#include "../structures/performance.h"
#include "../structures/controller_report.h"

#include "/network_optimization_methods/centralized_action_banning.h"
#include "/network_optimization_methods/multi_armed_bandits.h"
#include "/network_optimization_methods/rtot_algorithm.h"

#ifndef _AUX_ML_MODEL_
#define _AUX_ML_MODEL_

class MlModel {

	// Public items
	public:

		// Generic
		int agent_id;					///> ID of the agent calling the ML method
		int learning_mechanism;			///> Index of the learning mechanism employed
		int action_selection_strategy;	///> Index of the chosen action-selection strategy

		// Logs
		int save_logs;		///> Flag to indicate whether to save logs or not
		int print_logs;		///> Flag to indicate whether to print logs or not

		// Information of the network
		int agents_number;				///> Number of agents
		int wlans_number;				///> Number of WLANs
		int total_nodes_number;			///> Number of nodes
		int num_channels;				///> Number of channels

		// Variables used by some of the ML methods
		int num_arms;				///> Number of actions (Bandits)
		int max_number_of_actions;		///> Maximum number of actions among all the agents (centralized)
		int num_stas;					///> Number of STAs in a BSS
		double margin_rtot;				///> Margin [dB] used by the RTOT algorithm

	// Private items
	private:

		// Objects belonging to every implemented optimization method
		MultiArmedBandit mab_agent;				///> Multi-Armed Bandit
		RtotAlgorithm rtot_alg;					///> RTOT algorithm
		CentralizedActionBanning action_banner;	///> Centralized action-banning

	// Methods
	public:

		/********************************/
		/********************************/
		/*  CENTRAL CONTROLLER METHODS  */
		/********************************/
		/********************************/

		/**
		* Method for computing the global configuration at a Central Controller (CC)
		* @param "controller_report" [type ControllerReport]: report provided by the CC
		* @param "central_controller_logger" [type Logger]: logger object to write logs
		* @param "sim_time" [type double]: simulation time at the moment of calling the function (for logging purposes)
		*/
		void ComputeGlobalConfiguration(Configuration *configuration_array, ControllerReport &controller_report,
			Logger &central_controller_logger, double sim_time) {

			switch(learning_mechanism) {
				/* ACTION-BANNING */
				case CENTRALIZED_ACTION_BANNING: {
					// Ban configurations based on the observed performance
					action_banner.UpdateConfiguration(configuration_array, controller_report, central_controller_logger, sim_time);
//					controller_report.PrintOrWriteAvailableActions(PRINT_LOG, central_controller_logger, save_logs, sim_time);
					break;
				}
				default: {
					printf("[ML MODEL] ERROR, UNKOWN LEARNING MECHANISM ('%d')\n", learning_mechanism);
					exit(-1);
				}
			}

		}

		/************************************/
		/************************************/
		/*  DECENTRALIZED LEARNING METHODS  */
		/************************************/
		/************************************/

		/**
		* Method for computing an individual configuration (decentralized case)
		* @param "arm_ix" [type int]: index of the current selected arm (bandits)
		* @param "reward" [type Performance*]: array of performances of each AP
		* @param "agent_logger" [type Logger]: logger object to write logs
		* @param "sim_time" [type double]: simulation time at the moment of calling the function (for logging purposes)
		* @return "new_action" [type int]: index of the new selected action
		*/
		int ComputeIndividualConfiguration(int arm_ix, double reward,
			Logger &agent_logger, double sim_time, int *available_arms) {

			int new_action(0);
			switch(learning_mechanism) {
				/* MULTI_ARMED_BANDITS */
				case MULTI_ARMED_BANDITS: {
					// Update the reward of the last played configuration
					mab_agent.UpdateArmStatistics(arm_ix, reward);
					// Select a new action according to the updated information
					new_action = mab_agent.SelectNewAction(available_arms, arm_ix);
					break;
				}
				case RTOT_ALGORITHM: {
					new_action = rtot_alg.UpdateObssPd(reward);
					break;
				}
				case CENTRALIZED_ACTION_BANNING: {
					printf("[ML MODEL] ERROR, Action-Banning is not a decentralized ML method. Use 'CentralizedActionBanning()' instead.\n");
					break;
				}
				default: {
					printf("[ML MODEL] ERROR, UNKOWN LEARNING MECHANISM ('%d')\n", learning_mechanism);
					exit(-1);
				}
			}
			return new_action;
		};

		/******************************/
		/******************************/
		/*  VARIABLES INITIALIZATION  */
		/******************************/
		/******************************/

		/**
		* Initialize variables in the ML model
		*/
		void InitializeVariables() {

			switch(learning_mechanism) {
				/* Monitoring */
				case MONITORING_ONLY:{
					// DO NOTHING
					break;
				}
				/* Centralized action-banning */
				case CENTRALIZED_ACTION_BANNING: {
					action_banner.save_logs = save_logs;
					action_banner.print_logs = print_logs;
					action_banner.agents_number = agents_number;
					action_banner.max_number_of_actions = max_number_of_actions;
					action_banner.InitializeVariables();
					break;
				}
				/* Multi-Armed Bandits */
				case MULTI_ARMED_BANDITS: {
					mab_agent.agent_id = agent_id;
					mab_agent.save_logs = save_logs;
					mab_agent.print_logs = print_logs;
					mab_agent.action_selection_strategy = action_selection_strategy;
					mab_agent.num_arms = num_arms;
					mab_agent.InitializeVariables();
					break;
				}
				/* RTOT Algorithm for decentralized spatial reuse */
				case RTOT_ALGORITHM: {
					rtot_alg.num_stas = num_stas;
					rtot_alg.margin_rtot = margin_rtot;
					rtot_alg.InitializeVariables();
					break;
				}
				//  TODO: provide more learning mechanisms
				// case Q_LEARNING:
				// ...

				/* UNKNOWN */
				default: {
					printf("[ML MODEL] ERROR: '%d' is not a correct learning mechanism\n", learning_mechanism);
					PrintAvailableLearningMechanisms();
					exit(EXIT_FAILURE);
					break;
				}
			}
		}

		/*************************/
		/*************************/
		/*  PRINT/WRITE METHODS  */
		/*************************/
		/*************************/

		/**
		* Print or write statistics of the ML Model
		* @param "write_or_print" [type int]: variable to indicate whether to print on the  console or to write on the the output logs file
		* @param "logger" [type Logger]: logger object to write logs
		* @param "sim_time" [type double]: simulation time at the moment of calling the function (for logging purposes)
		*/
		void PrintOrWriteStatistics(int write_or_print, Logger &logger, double sim_time) {

			switch(learning_mechanism) {
				/* MONITORING */
				case MONITORING_ONLY:{
					// DO NOTHING
					break;
				}
				/* Multi-Armed Bandits */
				case MULTI_ARMED_BANDITS: {
					mab_agent.PrintOrWriteStatistics(write_or_print, logger, sim_time);
					break;
				}
				case RTOT_ALGORITHM: {
					rtot_alg.PrintOrWriteStatistics(write_or_print, logger);
					break;
				}
				case CENTRALIZED_ACTION_BANNING: {
                    action_banner.PrintOrWriteStatistics(write_or_print, logger);
					break;
				}
				/* UNKNOWN */
				default: {
					printf("[ML MODEL] ERROR: '%d' is not a correct learning mechanism\n", learning_mechanism);
					PrintAvailableLearningMechanisms();
					exit(EXIT_FAILURE);
					break;
				}
			}

		}

		/**
		* Print the available ML mechanisms types
		*/
		void PrintAvailableLearningMechanisms(){
			printf("%s Available types of learning mechanisms:\n", LOG_LVL2);
			printf("%s MULTI_ARMED_BANDITS (#%d)\n", LOG_LVL3, MULTI_ARMED_BANDITS);
			printf("%s ACTION_BANNING (#%d)\n", LOG_LVL3, ACTION_BANNING);
			printf("%s RTOT_ALGORITHM (#%d)\n", LOG_LVL3, RTOT_ALGORITHM);
		}

};

#endif
