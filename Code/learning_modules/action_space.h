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
 * action_space.h: Configuration <-> algorithm output codec
 *
 * Owns the discrete action space (channel × PD × TxPower × bandwidth)
 * and converts between a raw Configuration struct and the integer/double
 * output produced by a learning algorithm.  Extracted from pre_processor.h
 * so that reward computation and action encoding are decoupled.
 *
 * Encode: Configuration → arm index (for bandit methods)
 * Decode: algorithm output (int arm index or double pW value) → Configuration
 */

#include "../list_of_macros.h"
#include "../structures/node_configuration.h"
#include "../structures/action.h"

#ifndef _AUX_ACTION_SPACE_
#define _AUX_ACTION_SPACE_

class ActionSpace {

	public:

		/* Action-space dimensions */
		int num_arms;
		int num_arms_channel;
		int num_arms_sensitivity;
		int num_arms_tx_power;
		int num_arms_max_bandwidth;

		/* Parameter lists (pointers set by caller after InitializeVariables) */
		int    *list_of_channels;
		double *list_of_pd_values;
		double *list_of_tx_power_values;
		int    *list_of_max_bandwidth;

		/* Active learning mechanism — needed only for Decode dispatch */
		int learning_mechanism;

	private:

		/* Per-arm decomposition scratch buffer */
		int *indexes_selected_arm;

	public:

		/**********************/
		/*  INITIALIZATION    */
		/**********************/

		/**
		 * Allocate internal scratch buffer.
		 * The four list_of_* arrays must be assigned by the caller afterwards.
		 */
		void InitializeVariables() {
			indexes_selected_arm = new int[NUM_FEATURES_ACTIONS];
			for (int i = 0; i < NUM_FEATURES_ACTIONS; ++i)
				indexes_selected_arm[i] = 0;
		}

		/**
		 * Build and return the full Action array for the current arm space.
		 * Caller takes ownership of the returned array.
		 */
		Action* InitializeActions() {
			Action *action_array = new Action[num_arms];
			int *tmp = new int[NUM_FEATURES_ACTIONS];
			for (int i = 0; i < num_arms; ++i) {
				index2values(tmp, i);
				action_array[i].id           = i;
				action_array[i].channel      = list_of_channels[tmp[0]];
				action_array[i].cca          = list_of_pd_values[tmp[1]];
				action_array[i].tx_power     = list_of_tx_power_values[tmp[2]];
				action_array[i].max_bandwidth = list_of_max_bandwidth[tmp[3]];
				action_array[i].instantaneous_reward                = 0;
				action_array[i].times_played                        = 0;
				action_array[i].average_reward_since_last_cc_request = 0;
				action_array[i].times_played_since_last_cc_request  = 0;
			}
			delete[] tmp;
			return action_array;
		}

		/************************************/
		/*  ENCODE: Configuration → int     */
		/************************************/

		/**
		 * Map the current Configuration to a discrete arm index.
		 * For RTOT and other non-bandit methods this always returns 0
		 * (the arm index is irrelevant; the algorithm uses reward directly).
		 *
		 * @param "config"          current AP configuration
		 * @param "received_from_ap" true when the config was just received from the AP
		 *                           (selects non_srg_obss_pd vs selected_pd when SR is on)
		 * @return arm index in [0, num_arms)
		 */
		int Encode(Configuration config, bool received_from_ap) {

			int index_channel(-1);
			int index_pd(-1);
			int index_tx_power(-1);
			int index_max_bandwidth(-1);

			/* Channel */
			for (int i = 0; i < num_arms_channel; i++) {
				if (config.selected_primary_channel == list_of_channels[i])
					index_channel = i;
			}

			/* PD / CCA threshold */
			double selected_pd;
			if (received_from_ap && config.spatial_reuse_enabled)
				selected_pd = config.non_srg_obss_pd;
			else
				selected_pd = config.selected_pd;

			for (int i = 0; i < num_arms_sensitivity; i++) {
				if (selected_pd == list_of_pd_values[i])
					index_pd = i;
			}

			/* TX power */
			for (int i = 0; i < num_arms_tx_power; i++) {
				if (config.selected_tx_power == list_of_tx_power_values[i])
					index_tx_power = i;
			}

			/* Max bandwidth */
			for (int i = 0; i < num_arms_max_bandwidth; i++) {
				if (config.selected_max_bandwidth == list_of_max_bandwidth[i])
					index_max_bandwidth = i;
			}

			indexes_selected_arm[0] = index_channel;
			indexes_selected_arm[1] = index_pd;
			indexes_selected_arm[2] = index_tx_power;
			indexes_selected_arm[3] = index_max_bandwidth;

			return values2index(indexes_selected_arm);
		}

		/***************************************/
		/*  DECODE: algorithm output → Config  */
		/***************************************/

		/**
		 * Apply the algorithm output to a Configuration struct.
		 *
		 * For MULTI_ARMED_BANDITS:  ml_output is a discrete arm index (stored
		 *   as double for uniformity); the arm's (channel, PD, TxPower, BW)
		 *   tuple is written into *config.
		 *
		 * For RTOT_ALGORITHM: ml_output is a double pW value computed by
		 *   RtotAlgorithm::UpdateObssPd(); written directly to non_srg_obss_pd.
		 *   (Fixes the pre-existing int-cast bug that zeroed this value.)
		 *
		 * @param "ml_output" algorithm return value (double throughout the pipeline)
		 * @param "config"    configuration struct to update in-place
		 */
		void Decode(double ml_output, Configuration *config) {
			switch (learning_mechanism) {
				case MULTI_ARMED_BANDITS: {
					int arm_ix = (int) ml_output;
					index2values(indexes_selected_arm, arm_ix);
					config->selected_primary_channel = list_of_channels[indexes_selected_arm[0]];
					config->selected_pd              = list_of_pd_values[indexes_selected_arm[1]];
					config->selected_tx_power        = list_of_tx_power_values[indexes_selected_arm[2]];
					config->selected_max_bandwidth   = list_of_max_bandwidth[indexes_selected_arm[3]];
					break;
				}
				case RTOT_ALGORITHM: {
					/* ml_output already is the pW OBSS/PD threshold — no cast needed */
					config->non_srg_obss_pd = ml_output;
					break;
				}
				case MONITORING_ONLY: {
					/* No change: keep the current configuration */
					break;
				}
				default: {
					printf("[ActionSpace] ERROR: no Decode rule for learning_mechanism %d\n",
					       learning_mechanism);
					exit(EXIT_FAILURE);
				}
			}
		}

		/************************************/
		/*  AUXILIARY: index ↔ values       */
		/************************************/

		/**
		 * Decompose a joint arm index into per-parameter sub-indices.
		 * Order: [channel, PD, TxPower, MaxBandwidth]
		 */
		void index2values(int *indexes, int action_ix) {
			int s_pd = num_arms_sensitivity;
			int s_tx = num_arms_tx_power;
			int s_bw = num_arms_max_bandwidth;
			indexes[0] = action_ix / (s_pd * s_tx * s_bw);
			indexes[1] = (action_ix - indexes[0] * (s_pd * s_tx * s_bw)) / (s_tx * s_bw);
			indexes[2] = (action_ix - indexes[0] * (s_pd * s_tx * s_bw)
			              - indexes[1] * (s_tx * s_bw)) / s_bw;
			indexes[3] = action_ix % s_bw;
		}

		/**
		 * Compose per-parameter sub-indices into a single joint arm index.
		 */
		int values2index(int *indexes) {
			return indexes[0] * (num_arms_sensitivity * num_arms_tx_power * num_arms_max_bandwidth)
			     + indexes[1] * (num_arms_tx_power    * num_arms_max_bandwidth)
			     + indexes[2] * (num_arms_max_bandwidth)
			     + indexes[3];
		}

		/****************************/
		/*  PRINT / WRITE METHODS   */
		/****************************/

		/**
		 * Print or write the list of currently available actions.
		 */
		void PrintOrWriteAvailableActions(int print_or_write, char *device,
		                                  int save_logs, Logger &logger, double sim_time,
		                                  int *list_of_available_actions) {
			switch (print_or_write) {
				case PRINT_LOG: {
					printf("%s List of available actions: ", device);
					for (int i = 0; i < num_arms; ++i)
						printf("%d ", list_of_available_actions[i]);
					printf("\n");
					break;
				}
				case WRITE_LOG: {
					LOGS(save_logs, logger.file,
					     "%.15f;%s;%s;%s List of available actions: ",
					     sim_time, device, LOG_C00, LOG_LVL2);
					for (int i = 0; i < num_arms; ++i)
						LOGS(save_logs, logger.file, "%d ", list_of_available_actions[i]);
					LOGS(save_logs, logger.file, "\n");
					break;
				}
			}
		}

		/**
		 * Print all supported learning mechanism IDs.
		 */
		void PrintAvailableLearningMechanisms() {
			printf("%s Available learning mechanisms:\n", LOG_LVL2);
			printf("%s MONITORING_ONLY     (%d)\n", LOG_LVL3, MONITORING_ONLY);
			printf("%s MULTI_ARMED_BANDITS (%d)\n", LOG_LVL3, MULTI_ARMED_BANDITS);
			printf("%s RTOT_ALGORITHM      (%d)\n", LOG_LVL3, RTOT_ALGORITHM);
		}

};

#endif /* _AUX_ACTION_SPACE_ */
