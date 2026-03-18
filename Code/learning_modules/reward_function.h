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
 * reward_function.h: algorithm-agnostic reward computation
 *
 * Maps a Performance report to a scalar reward signal and provides
 * the corresponding logging helper. Extracted from pre_processor.h
 * so that any learning algorithm can use it without coupling to the
 * bandit arm-indexing logic.
 */

#include "../list_of_macros.h"
#include "../structures/performance.h"

#ifndef _AUX_REWARD_FUNCTION_
#define _AUX_REWARD_FUNCTION_

class RewardFunction {

	public:

		/**
		 * Compute the scalar reward from a Performance report.
		 * @param "type_of_reward" [type int]: reward type constant (REWARD_TYPE_*)
		 * @param "performance"    [type Performance]: performance report from the AP
		 * @return scalar reward in [0,1] (or an unbounded RSSI value for MIN_RSSI)
		 */
		double Compute(int type_of_reward, Performance performance) {

			double reward(0);

			switch(type_of_reward) {

				case REWARD_TYPE_PACKETS_SUCCESSFUL: {
					reward = (performance.data_packets_sent - performance.data_packets_lost)
					         / (double) performance.data_packets_sent;
					break;
				}

				case REWARD_TYPE_AVERAGE_THROUGHPUT: {
					if (performance.max_bound_throughput == 0) {
						reward = 0;
					} else {
						reward = (double) performance.throughput / performance.max_bound_throughput;
					}
					break;
				}

				case REWARD_TYPE_MIN_RSSI: {
					reward = performance.rssi_list_per_sta[0];
					for (int i = 0; i < performance.num_stas; ++i) {
						if (reward > performance.rssi_list_per_sta[i])
							reward = performance.rssi_list_per_sta[i];
					}
					break;
				}

				case REWARD_TYPE_MAX_DELAY: {
					reward = 1.0 / performance.max_delay;
					break;
				}

				case REWARD_TYPE_MIN_DELAY: {
					reward = 1.0 / performance.min_delay;
					break;
				}

				case REWARD_TYPE_AVERAGE_DELAY: {
					reward = 1.0 / performance.average_delay;
					break;
				}

				case REWARD_TYPE_CHANNEL_OCCUPANCY: {
					reward = performance.successful_channel_occupancy;
					break;
				}

				default: {
					printf("[RewardFunction] ERROR: '%d' is not a valid reward type\n", type_of_reward);
					PrintAvailableTypes();
					exit(EXIT_FAILURE);
				}
			}

			return reward;
		}

		/**
		 * Write the current performance metrics and associated reward to the agent log file.
		 */
		void Write(Logger &logger, double sim_time, char *device,
		           Performance performance, int type_of_reward, double reward) {

			LOGS(TRUE, logger.file, "%.15f;%s;%s;%s Performance:\n",
			     sim_time, device, LOG_C03, LOG_LVL2);

			switch(type_of_reward) {

				case REWARD_TYPE_PACKETS_SUCCESSFUL: {
					LOGS(TRUE, logger.file,
					     "%.15f;%s;%s;%s Packet successful ratio = %f\n",
					     sim_time, device, LOG_C03, LOG_LVL3,
					     (double)((performance.data_packets_sent - performance.data_packets_lost)
					              / performance.data_packets_sent));
					break;
				}

				case REWARD_TYPE_AVERAGE_THROUGHPUT: {
					LOGS(TRUE, logger.file,
					     "%.15f;%s;%s;%s Average throughput = %.2f Mbps\n",
					     sim_time, device, LOG_C03, LOG_LVL3,
					     performance.throughput * 1e-6);
					LOGS(TRUE, logger.file,
					     "%.15f;%s;%s;%s Average delay = %.4f s\n",
					     sim_time, device, LOG_C03, LOG_LVL3, performance.average_delay);
					LOGS(TRUE, logger.file,
					     "%.15f;%s;%s;%s Maximum delay = %.4f s\n",
					     sim_time, device, LOG_C03, LOG_LVL3, performance.max_waiting_time);
					LOGS(TRUE, logger.file,
					     "%.15f;%s;%s;%s Minimum delay = %.4f s\n",
					     sim_time, device, LOG_C03, LOG_LVL3, performance.min_waiting_time);
					break;
				}

				case REWARD_TYPE_MIN_RSSI: {
					LOGS(TRUE, logger.file,
					     "%.15f;%s;%s;%s Min RSSI = %.2f dBm\n",
					     sim_time, device, LOG_C03, LOG_LVL3,
					     performance.rssi_list_per_sta[0]);
					break;
				}

				case REWARD_TYPE_MAX_DELAY: {
					LOGS(TRUE, logger.file,
					     "%.15f;%s;%s;%s Max delay = %.2f ms\n",
					     sim_time, device, LOG_C03, LOG_LVL3,
					     performance.max_delay * 1e-3);
					break;
				}

				case REWARD_TYPE_MIN_DELAY: {
					LOGS(TRUE, logger.file,
					     "%.15f;%s;%s;%s Min delay = %.2f ms\n",
					     sim_time, device, LOG_C03, LOG_LVL3,
					     performance.min_delay * 1e-3);
					break;
				}

				case REWARD_TYPE_AVERAGE_DELAY: {
					LOGS(TRUE, logger.file,
					     "%.15f;%s;%s;%s Average delay = %.2f ms\n",
					     sim_time, device, LOG_C03, LOG_LVL3,
					     performance.average_delay * 1e-3);
					break;
				}

				case REWARD_TYPE_CHANNEL_OCCUPANCY: {
					LOGS(TRUE, logger.file,
					     "%.15f;%s;%s;%s Successful channel occupancy = %.2f\n",
					     sim_time, device, LOG_C03, LOG_LVL3,
					     performance.successful_channel_occupancy);
					break;
				}
			}

			LOGS(TRUE, logger.file,
			     "%.15f;%s;%s;%s Associated reward = %f\n",
			     sim_time, device, LOG_C03, LOG_LVL3, reward);
		}

		/**
		 * Print all available reward type constants.
		 */
		void PrintAvailableTypes() {
			printf("%s Available reward types:\n", LOG_LVL2);
			printf("%s REWARD_TYPE_PACKETS_SUCCESSFUL   (%d)\n", LOG_LVL3, REWARD_TYPE_PACKETS_SUCCESSFUL);
			printf("%s REWARD_TYPE_AVERAGE_THROUGHPUT   (%d)\n", LOG_LVL3, REWARD_TYPE_AVERAGE_THROUGHPUT);
			printf("%s REWARD_TYPE_MIN_RSSI             (%d)\n", LOG_LVL3, REWARD_TYPE_MIN_RSSI);
			printf("%s REWARD_TYPE_MAX_DELAY            (%d)\n", LOG_LVL3, REWARD_TYPE_MAX_DELAY);
			printf("%s REWARD_TYPE_MIN_DELAY            (%d)\n", LOG_LVL3, REWARD_TYPE_MIN_DELAY);
			printf("%s REWARD_TYPE_AVERAGE_DELAY        (%d)\n", LOG_LVL3, REWARD_TYPE_AVERAGE_DELAY);
			printf("%s REWARD_TYPE_CHANNEL_OCCUPANCY    (%d)\n", LOG_LVL3, REWARD_TYPE_CHANNEL_OCCUPANCY);
		}

};

#endif /* _AUX_REWARD_FUNCTION_ */
