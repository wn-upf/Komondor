/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 *
 * -----------------------------------------------------------------
 * Author  : Sergio Barrachina-Muñoz and Francesc Wilhelmi
 * Created : 2016-12-05
 * -----------------------------------------------------------------
 */

/**
 * learning_algorithm.h: single-dispatch learning algorithm wrapper for agents
 *
 * Replaces MlModel for the decentralised agent path (agent.h).
 * The central controller continues to use MlModel / ml_model.h which
 * handles centralized_action_banning independently.
 *
 * Design goal: Update() is the ONLY place in the codebase that switches
 * on learning_mechanism.  Adding a new algorithm requires:
 *   1. A new header in network_optimization_methods/
 *   2. A new #define constant in list_of_macros.h
 *   3. One new case each in InitializeVariables(), Update(), and
 *      PrintOrWriteStatistics() below -- nothing else needs to change.
 *
 * Note on return type:
 *   Update() returns double so that:
 *   - Bandit methods can return a discrete arm index (integer stored exactly
 *     as double for any realistic arm count < 2^52).
 *   - RTOT returns its OBSS/PD value in pW directly (double), fixing the
 *     pre-existing silent int-cast bug that always produced 0.
 *
 * Wire protocol for LEARNING_MECHANISM_EXTERNAL (features sent to server):
 *   features[0] = arm_ix    (last arm played)
 *   features[1] = reward    (last reward)
 *   features[2] = num_arms  (total arms, so server never needs it as an arg)
 */

#include "../list_of_macros.h"
#include "../structures/node_configuration.h"
#include "../structures/controller_report.h"

#include "network_optimization_methods/multi_armed_bandits.h"
#include "network_optimization_methods/rtot_algorithm.h"
#include "external_model_client.h"

#ifndef _AUX_LEARNING_ALGORITHM_
#define _AUX_LEARNING_ALGORITHM_

class LearningAlgorithm {

	public:

		/* Identity */
		int agent_id;

		/* Algorithm selection */
		int learning_mechanism;
		int action_selection_strategy;

		/* Algorithm parameters */
		int    num_arms;
		int    num_stas;
		double margin_rtot;

		/* External model parameters (LEARNING_MECHANISM_EXTERNAL only) */
		char external_socket_path[256];

		/* Logging */
		int save_logs;
		int print_logs;

	private:

		MultiArmedBandit     mab_agent;
		RtotAlgorithm        rtot_alg;
		ExternalModelClient  external_client;

		/* Stats tracked for LEARNING_MECHANISM_EXTERNAL */
		int    ext_num_queries;
		double ext_last_arm;

	public:

		/****************************/
		/*  INITIALIZATION          */
		/****************************/

		void InitializeVariables() {
			switch (learning_mechanism) {

				case MONITORING_ONLY: {
					/* nothing to allocate */
					break;
				}

				case MULTI_ARMED_BANDITS: {
					mab_agent.agent_id               = agent_id;
					mab_agent.save_logs              = save_logs;
					mab_agent.print_logs             = print_logs;
					mab_agent.action_selection_strategy = action_selection_strategy;
					mab_agent.num_arms               = num_arms;
					mab_agent.InitializeVariables();
					break;
				}

				case RTOT_ALGORITHM: {
					rtot_alg.num_stas     = num_stas;
					rtot_alg.margin_rtot  = margin_rtot;
					rtot_alg.InitializeVariables();
					break;
				}

				case LEARNING_MECHANISM_EXTERNAL: {
					ext_num_queries = 0;
					ext_last_arm    = 0.0;
					external_client.Connect(external_socket_path);
					break;
				}

				default: {
					printf("[LearningAlgorithm] ERROR: unknown learning_mechanism %d\n",
					       learning_mechanism);
					PrintAvailableLearningMechanisms();
					exit(EXIT_FAILURE);
				}
			}
		}

		/****************************/
		/*  MAIN UPDATE             */
		/****************************/

		/**
		 * Observe the last reward and select the next action.
		 *
		 * @param "arm_ix"         index of the arm played in the previous step
		 * @param "reward"         scalar reward signal from RewardFunction::Compute()
		 * @param "available_arms" bitmask: available_arms[i]==1 means arm i is usable
		 *
		 * @return for bandit methods: next arm index as double
		 *         for RTOT:           new OBSS/PD threshold in pW
		 *         for MONITORING_ONLY: arm_ix unchanged
		 */
		double Update(int arm_ix, double reward, int *available_arms) {

			double result(0);

			switch (learning_mechanism) {

				case MONITORING_ONLY: {
					result = (double) arm_ix;
					break;
				}

				case MULTI_ARMED_BANDITS: {
					mab_agent.UpdateArmStatistics(arm_ix, reward);
					result = (double) mab_agent.SelectNewAction(available_arms, arm_ix);
					break;
				}

				case RTOT_ALGORITHM: {
					/* reward is expected to be the min RSSI in pW */
					result = rtot_alg.UpdateObssPd(reward);
					break;
				}

				case LEARNING_MECHANISM_EXTERNAL: {
					float features[64];
					float out[1];
					features[0] = (float) arm_ix;
					features[1] = (float) reward;
					features[2] = (float) num_arms;
					external_client.Query(features, 3, out, 1);
					result = (double) out[0];
					++ ext_num_queries;
					ext_last_arm = result;
					break;
				}

				default: {
					printf("[LearningAlgorithm] ERROR: unknown learning_mechanism %d\n",
					       learning_mechanism);
					PrintAvailableLearningMechanisms();
					exit(EXIT_FAILURE);
				}
			}

			return result;
		}

		/****************************/
		/*  PRINT / WRITE           */
		/****************************/

		void PrintOrWriteStatistics(int write_or_print, Logger &logger, double sim_time) {
			switch (learning_mechanism) {
				case MONITORING_ONLY: {
					break;
				}
				case MULTI_ARMED_BANDITS: {
					mab_agent.PrintOrWriteStatistics(write_or_print, logger, sim_time);
					break;
				}
				case RTOT_ALGORITHM: {
					rtot_alg.PrintOrWriteStatistics(write_or_print, logger);
					break;
				}
				case LEARNING_MECHANISM_EXTERNAL: {
					printf("%s External model: socket=%s  queries=%d  last_arm=%.0f\n",
					       LOG_LVL3, external_socket_path,
					       ext_num_queries, ext_last_arm);
					break;
				}
				default: {
					printf("[LearningAlgorithm] ERROR: unknown learning_mechanism %d\n",
					       learning_mechanism);
					exit(EXIT_FAILURE);
				}
			}
		}

		/* Close the external socket (no-op for built-in algorithms). */
		void Close() {
			if (learning_mechanism == LEARNING_MECHANISM_EXTERNAL) {
				external_client.Close();
			}
		}

		void PrintAvailableLearningMechanisms() {
			printf("%s Available learning mechanisms:\n", LOG_LVL2);
			printf("%s MONITORING_ONLY          (%d)\n", LOG_LVL3, MONITORING_ONLY);
			printf("%s MULTI_ARMED_BANDITS      (%d)\n", LOG_LVL3, MULTI_ARMED_BANDITS);
			printf("%s RTOT_ALGORITHM           (%d)\n", LOG_LVL3, RTOT_ALGORITHM);
			printf("%s LEARNING_MECHANISM_EXTERNAL (%d)\n", LOG_LVL3, LEARNING_MECHANISM_EXTERNAL);
		}

};

#endif /* _AUX_LEARNING_ALGORITHM_ */
