/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 *
 * -----------------------------------------------------------------
 * Author  : Sergio Barrachina-Muñoz and Francesc Wilhelmi
 * Created : 2016-12-05
 * -----------------------------------------------------------------
 */

/**
 * pre_processor.h: thin coordinator for the ML pre-processing stage
 *
 * Owns a RewardFunction and an ActionSpace, and exposes a stable interface
 * to agent.h.  All logic lives in the two sub-modules; this class only
 * wires them together and forwards parameter assignments.
 *
 * To add a new algorithm:
 *   - Extend ActionSpace::Encode/Decode for the new state/action encoding
 *   - Extend RewardFunction::Compute if a new reward type is needed
 *   - No changes required in this file or in agent.h
 */

#include "../list_of_macros.h"
#include "../structures/node_configuration.h"
#include "../structures/performance.h"
#include "../structures/action.h"

#include "reward_function.h"
#include "action_space.h"

#ifndef _AUX_PP_
#define _AUX_PP_

class PreProcessor {

	public:

		/* Active learning mechanism — forwarded to ActionSpace */
		int learning_mechanism;

		/* Action-space dimensions (set by agent, forwarded to ActionSpace) */
		int num_arms;
		int num_arms_channel;
		int num_arms_sensitivity;
		int num_arms_tx_power;
		int num_arms_max_bandwidth;

		/* Parameter lists (set by agent, forwarded to ActionSpace) */
		int    *list_of_channels;
		double *list_of_pd_values;
		double *list_of_tx_power_values;
		int    *list_of_max_bandwidth;

	private:

		RewardFunction reward_fn;
		ActionSpace    action_space;

	public:

		/****************************/
		/*  INITIALIZATION          */
		/****************************/

		/**
		 * Propagate all dimensions and parameter lists into ActionSpace,
		 * then initialise both sub-modules.
		 * Must be called after all public members above are assigned.
		 */
		void InitializeVariables() {
			/* Wire ActionSpace dimensions and algorithm selection */
			action_space.learning_mechanism      = learning_mechanism;
			action_space.num_arms                = num_arms;
			action_space.num_arms_channel        = num_arms_channel;
			action_space.num_arms_sensitivity    = num_arms_sensitivity;
			action_space.num_arms_tx_power       = num_arms_tx_power;
			action_space.num_arms_max_bandwidth  = num_arms_max_bandwidth;
			action_space.InitializeVariables();
			/* NOTE: list_of_* pointer arrays are owned by agent.h and are assigned
			   to pre_processor.list_of_* AFTER this call returns.  Do not forward
			   them here — use ForwardListPointers() once agent.h has set them. */
		}

		/**
		 * Forward the parameter-list pointers from PreProcessor into ActionSpace.
		 * Must be called after agent.h assigns list_of_channels etc. on this object,
		 * and before any call to InitializeActions(), EncodeConfiguration(), or
		 * DecodeAction().  Called by InitializeActions() automatically.
		 */
		void ForwardListPointers() {
			action_space.list_of_channels        = list_of_channels;
			action_space.list_of_pd_values       = list_of_pd_values;
			action_space.list_of_tx_power_values = list_of_tx_power_values;
			action_space.list_of_max_bandwidth   = list_of_max_bandwidth;
		}

		/****************************/
		/*  ENCODE                  */
		/****************************/

		/**
		 * Map the current AP Configuration to a discrete arm index.
		 * Returns 0 for algorithms that do not use arm indexing (e.g. RTOT).
		 */
		int EncodeConfiguration(Configuration config, bool received_from_ap) {
			ForwardListPointers();
			return action_space.Encode(config, received_from_ap);
		}

		/**
		 * 
		 */
		// int EncodeState(Configuration config, Performance performance) {
		// 	ForwardListPointers();
		// 	return action_space.EncodeState(config, performance);
		// }

		/****************************/
		/*  REWARD                  */
		/****************************/

		/**
		 * Compute the scalar reward from the AP's Performance report.
		 */
		double ComputeReward(Performance performance, int type_of_reward) {
			return reward_fn.Compute(type_of_reward, performance);
		}

		/****************************/
		/*  DECODE                  */
		/****************************/

		/**
		 * Apply the learning algorithm's output to a Configuration struct.
		 * For bandit methods ml_output is an arm index; for RTOT it is a pW value.
		 */
		void DecodeAction(double ml_output, Configuration *config) {
			ForwardListPointers();
			action_space.Decode(ml_output, config);
		}

		/****************************/
		/*  ACTIONS ARRAY           */
		/****************************/

		/**
		 * Build and return the full Action array. Caller takes ownership.
		 */
		Action* InitializeActions() {
			ForwardListPointers();
			return action_space.InitializeActions();
		}

		/****************************/
		/*  PRINT / WRITE           */
		/****************************/

		void WritePerformance(Logger &logger, double sim_time, char *device,
		                      Performance performance, int type_of_reward, double reward) {
			reward_fn.Write(logger, sim_time, device, performance, type_of_reward, reward);
		}

		void PrintOrWriteAvailableActions(int print_or_write, char *device,
		                                  int save_logs, Logger &logger, double sim_time,
		                                  int *list_of_available_actions) {
			action_space.PrintOrWriteAvailableActions(print_or_write, device,
			                                          save_logs, logger, sim_time,
			                                          list_of_available_actions);
		}

};

#endif /* _AUX_PP_ */
