/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
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
 * output_generation_methods.h: functions for computing and writing simulation statistics.
 *
 * GenerateScriptOutput() selects the output format via a simulation_index integer,
 * allowing different projects to extract the metrics they need. Each format is implemented
 * in its own named function below the dispatcher for readability and maintainability.
 */

#include <math.h>
#include <stddef.h>
#include <string>
#include <sstream>
#include <iomanip>

#include "../../list_of_macros.h"
#include "../../structures/performance.h"
#include "../../structures/node_configuration.h"
#include "../../structures/wlan.h"
#include "../channel/power_channel_methods.h"

#ifndef _OUT_METHODS_
#define _OUT_METHODS_

// ===========================================================================
// Aggregated statistics (replaces the former module-scope global variables)
// ===========================================================================

struct SimulationStats {
	int    total_data_packets_sent;
	double total_num_packets_generated;
	double total_throughput;
	int    ix_wlan_min_throughput;
	double min_throughput;
	double max_throughput;
	double proportional_fairness;
	double jains_fairness;
	double jains_fairness_aux;
	int    total_rts_lost_slotted_bo;
	int    total_rts_cts_sent;
	double total_prob_slotted_bo_collision;
	int    total_num_tx_init_not_possible;
	double total_delay;
	double max_delay;
	double min_delay;
	double total_bandiwdth_tx;   ///< typo preserved from original
	double av_expected_backoff;
	double av_expected_waiting_time;

	SimulationStats() :
		total_data_packets_sent(0),
		total_num_packets_generated(0),
		total_throughput(0),
		ix_wlan_min_throughput(99999),
		min_throughput(999999999999999999.0),
		max_throughput(0),
		proportional_fairness(0),
		jains_fairness(0),
		jains_fairness_aux(0),
		total_rts_lost_slotted_bo(0),
		total_rts_cts_sent(0),
		total_prob_slotted_bo_collision(0),
		total_num_tx_init_not_possible(0),
		total_delay(0),
		max_delay(0),
		min_delay(9999999999.0),
		total_bandiwdth_tx(0),
		av_expected_backoff(0),
		av_expected_waiting_time(0)
	{}
};

// ===========================================================================
// Statistics computation
// ===========================================================================

/**
* Compute the global statistics after finishing the simulation.
* @param "stats"                  [type SimulationStats&]:  output — aggregated metrics
* @param "performance_report"     [type Performance*]:      per-node performance
* @param "configuration_per_node" [type Configuration*]:   per-node configuration
* @param "total_nodes_number"     [type int]:               total number of nodes
* @param "total_wlans_number"     [type int]:               total number of WLANs
*/
void ComputeSimulationStatistics(SimulationStats &stats, Performance *performance_report,
		Configuration *configuration_per_node, int total_nodes_number, int total_wlans_number) {

	for (int m = 0; m < total_nodes_number; ++m) {
		if (configuration_per_node[m].capabilities.node_type == NODE_TYPE_AP) {
			stats.total_data_packets_sent         += performance_report[m].data_packets_sent;
			stats.total_throughput                += performance_report[m].throughput;
			stats.total_num_packets_generated     += performance_report[m].num_packets_generated;
			stats.total_rts_lost_slotted_bo       += performance_report[m].rts_lost_slotted_bo;
			stats.total_rts_cts_sent              += performance_report[m].rts_cts_sent;
			stats.total_prob_slotted_bo_collision += performance_report[m].prob_slotted_bo_collision;
			stats.total_num_tx_init_not_possible  += performance_report[m].num_tx_init_not_possible;
			stats.proportional_fairness           += log10(performance_report[m].throughput);
			stats.jains_fairness_aux              += pow(performance_report[m].throughput, 2);
			stats.total_delay                     += performance_report[m].average_delay;
			stats.av_expected_backoff             += performance_report[m].expected_backoff;
			stats.av_expected_waiting_time        += performance_report[m].average_waiting_time;
			stats.total_bandiwdth_tx              += performance_report[m].bandwidth_used_txing;

			if (performance_report[m].average_delay > stats.max_delay)
				stats.max_delay = performance_report[m].average_delay;
			if (performance_report[m].average_delay < stats.min_delay)
				stats.min_delay = performance_report[m].average_delay;
			if (performance_report[m].throughput < stats.min_throughput) {
				stats.ix_wlan_min_throughput = m;
				stats.min_throughput         = performance_report[m].throughput;
			}
			if (performance_report[m].throughput > stats.max_throughput)
				stats.max_throughput = performance_report[m].throughput;
		}
	}
	stats.av_expected_backoff      /= total_wlans_number;
	stats.av_expected_waiting_time /= total_wlans_number;
	// Integer division total_nodes_number/2 is preserved from the original formula
	stats.jains_fairness = pow(stats.total_throughput, 2)
	                     / (total_nodes_number/2 * stats.jains_fairness_aux);
}

/**
* Print and write global statistics to console and log file.
* @param "stats" [type const SimulationStats&]: pre-computed aggregated metrics
*/
void PrintAndWriteSimulationStatistics(int print_system_logs, int save_system_logs,
		Logger &logger_simulation, Performance *performance_report,
		Configuration *configuration_per_node, int total_nodes_number,
		int total_wlans_number, double simulation_time_komondor,
		const SimulationStats &stats) {

	if (print_system_logs) {
		printf("\n%s General Statistics:\n", LOG_LVL1);
		printf("%s Average throughput per WLAN = %.3f Mbps\n",
			LOG_LVL2, (stats.total_throughput * pow(10,-6) / total_wlans_number));
		printf("%s Min. throughput = %.2f Mbps (%.2f pkt/s)\n",
			LOG_LVL3, stats.min_throughput * pow(10,-6),
			stats.min_throughput / (configuration_per_node[0].frame_length
			* configuration_per_node[0].max_num_packets_aggregated));
		printf("%s Max. throughput = %.2f Mbps (%.2f pkt/s)\n",
			LOG_LVL3, stats.max_throughput * pow(10,-6),
			stats.max_throughput / (configuration_per_node[0].frame_length
			* configuration_per_node[0].max_num_packets_aggregated));
		printf("%s Total throughput = %.2f Mbps\n", LOG_LVL3, stats.total_throughput * pow(10,-6));
		printf("%s Total number of packets sent = %d\n", LOG_LVL3, stats.total_data_packets_sent);
		printf("%s Average number of data packets successfully sent per WLAN = %.2f\n",
			LOG_LVL4, ((double) stats.total_data_packets_sent / (double) total_wlans_number));
		printf("%s Average number of RTS packets lost due to slotted BO = %f (%.3f %% loss)\n",
			LOG_LVL4,
			(double) stats.total_rts_lost_slotted_bo / (double) total_wlans_number,
			((double) stats.total_rts_lost_slotted_bo * 100 / (double) stats.total_rts_cts_sent));
		printf("%s Average number of packets sent per WLAN = %d\n",
			LOG_LVL3, (stats.total_data_packets_sent / total_wlans_number));
		printf("%s Proportional Fairness = %.2f\n", LOG_LVL2, stats.proportional_fairness);
		printf("%s Jain's Fairness = %.2f\n",  LOG_LVL2, stats.jains_fairness);
		printf("%s Prob. collision by slotted BO = %.3f\n",
			LOG_LVL2, stats.total_prob_slotted_bo_collision / total_wlans_number);
		printf("%s Av. delay = %.2f ms\n", LOG_LVL2, stats.total_delay * pow(10,3) / total_wlans_number);
		printf("%s Max. delay = %.2f ms\n", LOG_LVL3, stats.max_delay * pow(10,3));
		printf("%s Av. expected waiting time = %.2f ms\n",
			LOG_LVL3, stats.av_expected_waiting_time * pow(10,3));
		printf("%s Average bandwidth used for transmitting = %.2f MHz\n",
			LOG_LVL2, stats.total_bandiwdth_tx / (double) total_wlans_number);
		printf("%s Time channel was idle = %.2f s (%f%%)\n",  LOG_LVL2,
			performance_report[0].sum_time_channel_idle,
			(100 * performance_report[0].sum_time_channel_idle / simulation_time_komondor));
		printf("\n\n");
	}

	printf("\n");

	if (save_system_logs) {
		fprintf(logger_simulation.file, "\n%s General Statistics:\n", LOG_LVL1);
		fprintf(logger_simulation.file, "%s Average throughput per WLAN = %.2f Mbps\n",
			LOG_LVL2, (stats.total_throughput * pow(10,-6) / total_wlans_number));
		fprintf(logger_simulation.file, "%s Total throughput = %.2f Mbps\n",
			LOG_LVL3, stats.total_throughput * pow(10,-6));
		fprintf(logger_simulation.file, "%s Total number of packets sent = %d\n",
			LOG_LVL3, stats.total_data_packets_sent);
		fprintf(logger_simulation.file, "%s Average number of data packets successfully sent per WLAN = %.2f\n",
			LOG_LVL4, ((double) stats.total_data_packets_sent / (double) total_wlans_number));
		fprintf(logger_simulation.file,
			"%s Average number of RTS packets lost due to slotted BO = %.2f (%.2f %% loss)\n",
			LOG_LVL4,
			(double) stats.total_rts_lost_slotted_bo / (double) total_wlans_number,
			((double) stats.total_rts_lost_slotted_bo * 100 / (double) stats.total_rts_cts_sent));
		fprintf(logger_simulation.file, "%s Average number of packets sent per WLAN = %d\n",
			LOG_LVL3, (stats.total_data_packets_sent / total_wlans_number));
		fprintf(logger_simulation.file, "%s Proportional Fairness = %.2f\n",
			LOG_LVL2, stats.proportional_fairness);
		fprintf(logger_simulation.file, "%s Jain's Fairness = %.2f\n",
			LOG_LVL2, stats.jains_fairness);
		fprintf(logger_simulation.file, "\n");
	}
}

// ===========================================================================
// Per-format output functions
// ===========================================================================

// case 0: toy scenarios — throughput and collision probability for 2 WLANs
static void WriteOutput_ToyScenario(Performance *performance_report,
		Configuration *, Logger &logger_script, int, int, Wlan *, double,
		const SimulationStats &) {
	fprintf(logger_script.file, ";%.2f;%.2f;%f;%f\n",
		performance_report[0].throughput * pow(10,-6),
		performance_report[2].throughput * pow(10,-6),
		performance_report[0].prob_slotted_bo_collision,
		performance_report[2].prob_slotted_bo_collision);
}

// case 1: large scenarios — node density vs. throughput
static void WriteOutput_LargeScenario_NodeDensity(Performance *,
		Configuration *, Logger &logger_script, int total_wlans_number, int, Wlan *, double,
		const SimulationStats &stats) {
	fprintf(logger_script.file, ";%.2f;%.2f;%f;%.2f;%d;%.2f\n",
		(stats.total_throughput * pow(10,-6) / total_wlans_number),
		stats.proportional_fairness,
		stats.jains_fairness,
		stats.min_throughput * pow(10,-6),
		stats.ix_wlan_min_throughput,
		stats.total_bandiwdth_tx / (double) total_wlans_number);
}

// case 2: central WLAN scenario — detailed frame counters for one AP
static void WriteOutput_CentralWlan(Performance *performance_report,
		Configuration *, Logger &logger_script, int, int, Wlan *, double,
		const SimulationStats &) {
	fprintf(logger_script.file, ";%.1f;%d;%d;%d;%d;%d;%d;%d\n",
		performance_report[0].throughput * pow(10,-6),
		performance_report[0].rts_cts_sent,
		performance_report[0].rts_cts_lost,
		performance_report[0].rts_lost_slotted_bo,
		performance_report[0].data_packets_sent,
		performance_report[0].data_packets_lost,
		performance_report[0].num_tx_init_tried,
		performance_report[0].num_tx_init_not_possible);
}

// case 3: Bianchi multiple WLANs — backoff, throughput, collision probability
static void WriteOutput_BianchiMultiWlan(Performance *,
		Configuration *, Logger &logger_script, int total_wlans_number, int, Wlan *, double,
		const SimulationStats &stats) {
	fprintf(logger_script.file, ";%.2f;%.3f;%.5f\n",
		stats.av_expected_backoff / SLOT_TIME,
		(stats.total_throughput * pow(10,-6) / total_wlans_number),
		stats.total_prob_slotted_bo_collision / total_wlans_number);
}

// case 4: DCB validation — collision probability and per-WLAN throughput
static void WriteOutput_DcbValidation(Performance *performance_report,
		Configuration *, Logger &logger_script, int total_wlans_number, int, Wlan *, double,
		const SimulationStats &stats) {
	fprintf(logger_script.file, ";%.5f", stats.total_prob_slotted_bo_collision / total_wlans_number);
	for (int w = 0; w < total_wlans_number; ++w)
		fprintf(logger_script.file, ";%.3f", performance_report[w*2].throughput * pow(10,-6));
	fprintf(logger_script.file, "\n");
}

// case 5: variability of optimal policies — collision probability and throughput per WLAN
static void WriteOutput_OptimalPolicyVariability(Performance *performance_report,
		Configuration *, Logger &logger_script, int total_wlans_number, int, Wlan *, double,
		const SimulationStats &) {
	for (int w = 0; w < total_wlans_number; ++w)
		fprintf(logger_script.file, ";%.5f", performance_report[w*2].prob_slotted_bo_collision);
	for (int w = 0; w < total_wlans_number; ++w)
		fprintf(logger_script.file, ";%.3f", performance_report[w*2].throughput * pow(10,-6));
	fprintf(logger_script.file, "\n");
}

// case 6: two-WLAN overlap / 3-WLAN line scenario — extended per-WLAN metrics
static void WriteOutput_TwoWlanOverlap(Performance *performance_report,
		Configuration *configuration_per_node, Logger &logger_script, int, int, Wlan *, double,
		const SimulationStats &) {
	fprintf(logger_script.file, ";%d;%d;%.0f;%.0f;%.2f;%.2f;"
		"%.4f;%.4f;%.2f;%.2f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.2f;%.2f\n",
		performance_report[0].num_packets_generated,
		performance_report[2].num_packets_generated,
		performance_report[0].throughput / (configuration_per_node[0].frame_length *
			configuration_per_node[0].max_num_packets_aggregated),
		performance_report[2].throughput / (configuration_per_node[2].frame_length *
			configuration_per_node[2].max_num_packets_aggregated),
		performance_report[0].average_rho,
		performance_report[2].average_rho,
		performance_report[0].average_delay * pow(10,3),
		performance_report[2].average_delay * pow(10,3),
		performance_report[0].average_utilization,
		performance_report[2].average_utilization,
		performance_report[0].prob_slotted_bo_collision,
		performance_report[2].prob_slotted_bo_collision,
		performance_report[0].average_waiting_time / SLOT_TIME,
		performance_report[2].average_waiting_time / SLOT_TIME,
		(double) performance_report[0].num_packets_dropped * 100 / performance_report[0].num_packets_generated,
		(double) performance_report[2].num_packets_dropped * 100 / performance_report[2].num_packets_generated,
		(double) (performance_report[0].data_frames_acked / performance_report[0].data_packets_acked),
		(double) (performance_report[2].data_frames_acked / performance_report[2].data_packets_acked));
}

// case 7: central WLAN (Paper #5) — extended metrics for one AP
static void WriteOutput_CentralWlan_Paper5(Performance *performance_report,
		Configuration *configuration_per_node, Logger &logger_script, int, int, Wlan *, double,
		const SimulationStats &) {
	fprintf(logger_script.file, ";%.0f;%d;%d;%d;%d;%d;%d;%d;%f;%f;%f;%f;%f\n",
		performance_report[0].throughput / (configuration_per_node[0].frame_length *
			configuration_per_node[0].max_num_packets_aggregated),
		performance_report[0].rts_cts_sent,
		performance_report[0].rts_cts_lost,
		performance_report[0].rts_lost_slotted_bo,
		performance_report[0].data_packets_sent,
		performance_report[0].data_packets_lost,
		performance_report[0].num_tx_init_tried,
		performance_report[0].num_tx_init_not_possible,
		performance_report[0].average_delay * pow(10,3),
		performance_report[0].average_waiting_time / SLOT_TIME,
		performance_report[0].prob_slotted_bo_collision,
		(double) performance_report[0].num_packets_dropped * 100 / performance_report[0].num_packets_generated,
		(double) performance_report[0].data_frames_acked / performance_report[0].data_packets_acked);
}

// case 8: central WLAN (Paper #5) — single-AP load/saturation metrics
static void WriteOutput_CentralWlan_Paper5_Single(Performance *performance_report,
		Configuration *configuration_per_node, Logger &logger_script, int, int, Wlan *, double,
		const SimulationStats &) {
	fprintf(logger_script.file, ";%d;%.0f;%.2f;"
		"%.4f;%.2f;%.4f;%.4f;%.4f;%.4f\n",
		performance_report[0].num_packets_generated,
		performance_report[0].throughput / (configuration_per_node[0].frame_length *
			configuration_per_node[0].max_num_packets_aggregated),
		performance_report[0].average_rho,
		performance_report[0].average_delay * pow(10,3),
		performance_report[0].average_utilization,
		performance_report[0].prob_slotted_bo_collision,
		performance_report[0].average_waiting_time / SLOT_TIME,
		(double) (performance_report[0].num_packets_dropped * 100 / performance_report[0].num_packets_generated),
		(double) (performance_report[0].data_frames_acked / performance_report[0].data_packets_acked));
}

// case 9: 6-WLAN random scenario — comprehensive network-wide metrics
static void WriteOutput_SixWlanRandom(Performance *performance_report,
		Configuration *configuration_per_node, Logger &logger_script, int total_wlans_number, int, Wlan *, double,
		const SimulationStats &stats) {
	fprintf(logger_script.file, ";%.2f;%.2f;%.2f;%d;%.4f;%.4f;%.4f;%.2f;%.2f;%.2f;%f;%f;%f\n",
		stats.total_throughput / (configuration_per_node[0].frame_length *
			configuration_per_node[0].max_num_packets_aggregated * total_wlans_number),
		(stats.total_throughput * pow(10,-6) / total_wlans_number),
		stats.min_throughput / (configuration_per_node[0].frame_length *
			configuration_per_node[0].max_num_packets_aggregated),
		stats.ix_wlan_min_throughput,
		stats.proportional_fairness,
		stats.jains_fairness,
		stats.total_prob_slotted_bo_collision / total_wlans_number,
		stats.total_delay * pow(10,3) / total_wlans_number,
		stats.max_delay * pow(10,3),
		stats.total_bandiwdth_tx / (double) total_wlans_number,
		stats.av_expected_waiting_time * pow(10,3),
		stats.min_delay * pow(10,3),
		stats.max_throughput / (configuration_per_node[0].frame_length *
			configuration_per_node[0].max_num_packets_aggregated));
}

// case 10: regression test — throughput per AP + RSSI per STA
static void WriteOutput_RegressionTest(Performance *performance_report,
		Configuration *configuration_per_node, Logger &logger_script, int total_wlans_number,
		int total_nodes_number, Wlan *wlan_container, double, const SimulationStats &) {

	std::ostringstream tpt_stream, rssi_stream;
	tpt_stream << "{";
	rssi_stream << "{";

	int counter_nodes_visited = 0;
	bool first_ap = true;
	for (int i = 0; i < total_nodes_number; ++i) {
		if (configuration_per_node[i].capabilities.node_type == NODE_TYPE_AP) {
			if (!first_ap) tpt_stream << ",";
			first_ap = false;
			tpt_stream << std::fixed << std::setprecision(2)
			           << (performance_report[i].throughput * pow(10,-6));
			++counter_nodes_visited;
			for (int w = 0; w < total_wlans_number; ++w) {
				if (wlan_container[w].ap_id == configuration_per_node[i].capabilities.node_id) {
					for (int s = 0; s < wlan_container[w].num_stas; ++s) {
						rssi_stream << std::fixed << std::setprecision(2)
						            << ConvertPower(PW_TO_DBM,
						               performance_report[counter_nodes_visited].received_power_array[i]);
						++counter_nodes_visited;
						if (counter_nodes_visited < total_nodes_number) rssi_stream << ",";
					}
				}
			}
		}
	}
	tpt_stream << "}";
	rssi_stream << "}";
	fprintf(logger_script.file, ";%s;%s\n", tpt_stream.str().c_str(), rssi_stream.str().c_str());
}

// case 11: spatial reuse tutorial — throughput, channel occupancy, delay (branch by scenario size)
static void WriteOutput_SpatialReuse_Tutorial(Performance *performance_report,
		Configuration *, Logger &logger_script, int, int total_nodes_number,
		Wlan *, double simulation_time_komondor, const SimulationStats &) {

	if (total_nodes_number == 2 || total_nodes_number == 3) {
		fprintf(logger_script.file, ";%.2f\n",
			performance_report[0].throughput * pow(10,-6));
	} else if (total_nodes_number == 4) {
		fprintf(logger_script.file, ";%.2f;%.2f\n",
			performance_report[0].throughput * pow(10,-6),
			performance_report[2].throughput * pow(10,-6));
	} else if (total_nodes_number == 6) {
		fprintf(logger_script.file, ";%.2f;%.2f;%.2f\n",
			performance_report[0].throughput * pow(10,-6),
			performance_report[2].throughput * pow(10,-6),
			performance_report[4].throughput * pow(10,-6));
	} else if (total_nodes_number == 18) {
		// 9 WLANs: throughput, channel occupancy, average delay
		int ap_ids[9] = {0, 2, 4, 6, 8, 10, 12, 14, 16};
		fprintf(logger_script.file,
			";%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f"
			";%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f"
			";%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\n",
			performance_report[ap_ids[0]].throughput * pow(10,-6),
			performance_report[ap_ids[1]].throughput * pow(10,-6),
			performance_report[ap_ids[2]].throughput * pow(10,-6),
			performance_report[ap_ids[3]].throughput * pow(10,-6),
			performance_report[ap_ids[4]].throughput * pow(10,-6),
			performance_report[ap_ids[5]].throughput * pow(10,-6),
			performance_report[ap_ids[6]].throughput * pow(10,-6),
			performance_report[ap_ids[7]].throughput * pow(10,-6),
			performance_report[ap_ids[8]].throughput * pow(10,-6),
			((performance_report[ap_ids[0]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[0]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[ap_ids[1]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[1]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[ap_ids[2]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[2]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[ap_ids[3]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[3]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[ap_ids[4]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[4]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[ap_ids[5]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[5]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[ap_ids[6]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[6]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[ap_ids[7]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[7]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[ap_ids[8]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[8]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			performance_report[ap_ids[0]].average_waiting_time * pow(10,3),
			performance_report[ap_ids[1]].average_waiting_time * pow(10,3),
			performance_report[ap_ids[2]].average_waiting_time * pow(10,3),
			performance_report[ap_ids[3]].average_waiting_time * pow(10,3),
			performance_report[ap_ids[4]].average_waiting_time * pow(10,3),
			performance_report[ap_ids[5]].average_waiting_time * pow(10,3),
			performance_report[ap_ids[6]].average_waiting_time * pow(10,3),
			performance_report[ap_ids[7]].average_waiting_time * pow(10,3),
			performance_report[ap_ids[8]].average_waiting_time * pow(10,3));
	} else {
		printf("Error in WriteOutput_SpatialReuse_Tutorial: unexpected total_nodes_number=%d\n",
			total_nodes_number);
	}
}

// case 12: spatial reuse CSCN — 10 WLANs: throughput, channel occupancy, delay
static void WriteOutput_SpatialReuse_CSCN(Performance *performance_report,
		Configuration *, Logger &logger_script, int, int, Wlan *,
		double simulation_time_komondor, const SimulationStats &) {

	int ap_ids[10] = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
	fprintf(logger_script.file,
		";%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f"
		";%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f"
		";%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\n",
		performance_report[ap_ids[0]].throughput * pow(10,-6),
		performance_report[ap_ids[1]].throughput * pow(10,-6),
		performance_report[ap_ids[2]].throughput * pow(10,-6),
		performance_report[ap_ids[3]].throughput * pow(10,-6),
		performance_report[ap_ids[4]].throughput * pow(10,-6),
		performance_report[ap_ids[5]].throughput * pow(10,-6),
		performance_report[ap_ids[6]].throughput * pow(10,-6),
		performance_report[ap_ids[7]].throughput * pow(10,-6),
		performance_report[ap_ids[8]].throughput * pow(10,-6),
		performance_report[ap_ids[9]].throughput * pow(10,-6),
		((performance_report[ap_ids[0]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[0]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
		((performance_report[ap_ids[1]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[1]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
		((performance_report[ap_ids[2]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[2]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
		((performance_report[ap_ids[3]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[3]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
		((performance_report[ap_ids[4]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[4]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
		((performance_report[ap_ids[5]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[5]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
		((performance_report[ap_ids[6]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[6]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
		((performance_report[ap_ids[7]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[7]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
		((performance_report[ap_ids[8]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[8]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
		((performance_report[ap_ids[9]].total_time_transmitting_in_num_channels[0] - performance_report[ap_ids[9]].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
		performance_report[ap_ids[0]].average_waiting_time * pow(10,3),
		performance_report[ap_ids[1]].average_waiting_time * pow(10,3),
		performance_report[ap_ids[2]].average_waiting_time * pow(10,3),
		performance_report[ap_ids[3]].average_waiting_time * pow(10,3),
		performance_report[ap_ids[4]].average_waiting_time * pow(10,3),
		performance_report[ap_ids[5]].average_waiting_time * pow(10,3),
		performance_report[ap_ids[6]].average_waiting_time * pow(10,3),
		performance_report[ap_ids[7]].average_waiting_time * pow(10,3),
		performance_report[ap_ids[8]].average_waiting_time * pow(10,3),
		performance_report[ap_ids[9]].average_waiting_time * pow(10,3));
}

// case 13: FG-ML5G student project — per-device labels, tpt, RSSI, loss ratios, NAV time
static void WriteOutput_FgMl5g_StudentProject(Performance *performance_report,
		Configuration *configuration_per_node, Logger &logger_script, int total_wlans_number,
		int total_nodes_number, Wlan *wlan_container, double simulation_time_komondor,
		const SimulationStats &) {

	std::ostringstream label_stream, tpt_stream, rssi_stream;
	std::ostringstream data_loss_stream, rtscts_loss_stream, nav_stream;

	label_stream    << "{";
	tpt_stream      << "{";
	rssi_stream     << "{";
	data_loss_stream   << "{";
	rtscts_loss_stream << "{";
	nav_stream      << "{";

	int counter_nodes_visited = 0;
	for (int i = 0; i < total_nodes_number; ++i) {
		label_stream << configuration_per_node[i].capabilities.node_code;
		if (i < total_nodes_number - 1) label_stream << ",";

		if (configuration_per_node[i].capabilities.node_type == NODE_TYPE_AP) {
			tpt_stream      << std::fixed << std::setprecision(2)
			                << (performance_report[i].throughput * pow(10,-6)) << ",";
			rssi_stream     << "Inf,";
			data_loss_stream   << std::fixed << std::setprecision(2)
			                   << (double) 100*performance_report[i].data_packets_lost
			                              /performance_report[i].data_packets_sent << ",";
			rtscts_loss_stream << std::fixed << std::setprecision(2)
			                   << (double) 100*performance_report[i].rts_cts_lost
			                              /performance_report[i].rts_cts_sent << ",";
			++counter_nodes_visited;

			for (int w = 0; w < total_wlans_number; ++w) {
				if (wlan_container[w].ap_id == configuration_per_node[i].capabilities.node_id) {
					for (int s = 0; s < wlan_container[w].num_stas; ++s) {
						tpt_stream << std::fixed << std::setprecision(2)
						           << (performance_report[i].throughput_per_sta[s] * pow(10,-6));
						rssi_stream << std::fixed << std::setprecision(2)
						            << ConvertPower(PW_TO_DBM,
						               performance_report[counter_nodes_visited].received_power_array[i]);
						data_loss_stream << std::fixed << std::setprecision(2)
						                 << (double) 100*performance_report[i].data_packets_lost_per_sta[s]
						                            /performance_report[i].data_packets_sent_per_sta[s];
						rtscts_loss_stream << std::fixed << std::setprecision(2)
						                   << (double) 100*performance_report[i].rts_cts_lost_per_sta[s]
						                              /performance_report[i].rts_cts_sent_per_sta[s];
						++counter_nodes_visited;
						if (counter_nodes_visited < total_nodes_number) {
							tpt_stream      << ",";
							rssi_stream     << ",";
							data_loss_stream   << ",";
							rtscts_loss_stream << ",";
						}
					}
				}
			}
		}

		nav_stream << std::fixed << std::setprecision(2)
		           << (performance_report[i].time_in_nav / simulation_time_komondor * 100);
		if (i < total_nodes_number - 1) nav_stream << ",";
	}

	label_stream       << "}";
	tpt_stream         << "}";
	rssi_stream        << "}";
	data_loss_stream   << "}";
	rtscts_loss_stream << "}";
	nav_stream         << "}";

	fprintf(logger_script.file, ";%s;%s;%s;%s;%s;%s\n",
		label_stream.str().c_str(), tpt_stream.str().c_str(), rssi_stream.str().c_str(),
		data_loss_stream.str().c_str(), rtscts_loss_stream.str().c_str(), nav_stream.str().c_str());
}

// case 14: RTOT algorithm for 11ax SR — tpt, airtime, successful airtime, max power per WLAN
static void WriteOutput_RtotAlgorithm(Performance *performance_report,
		Configuration *configuration_per_node, Logger &logger_script, int total_wlans_number,
		int total_nodes_number, Wlan *, double simulation_time_komondor, const SimulationStats &) {

	std::ostringstream tpt_stream, airtime_stream, sairtime_stream, power_stream;

	for (int i = 0; i < total_nodes_number; ++i) {
		if (configuration_per_node[i].capabilities.node_type == NODE_TYPE_AP) {
			tpt_stream << std::fixed << std::setprecision(2)
			           << (performance_report[i].throughput * pow(10,-6)) << ";";
			airtime_stream << std::fixed << std::setprecision(2)
			               << ((performance_report[i].total_time_transmitting_in_num_channels[0]
			                    - performance_report[i].total_time_lost_in_num_channels[i])
			                   * 100 / simulation_time_komondor) << ";";
			sairtime_stream << std::fixed << std::setprecision(2)
			                << (performance_report[i].total_time_transmitting_in_num_channels[0]
			                    * 100 / simulation_time_komondor) << ";";

			power_stream << "{";
			for (int w = 0; w < total_wlans_number; ++w) {
				power_stream << std::fixed << std::setprecision(2)
				             << ConvertPower(PW_TO_DBM,
				                performance_report[i].max_received_power_in_ap_per_wlan[w]);
				if (w < total_wlans_number - 1) power_stream << ",";
			}
			power_stream << "};";
		}
	}

	fprintf(logger_script.file, ";%s%s%s%s\n",
		tpt_stream.str().c_str(), airtime_stream.str().c_str(),
		sairtime_stream.str().c_str(), power_stream.str().c_str());
}

// Helper used by WriteOutput_MABDrivenSR and WriteOutput_TokenizedBackoff.
// Appends val (formatted to 2 d.p.) followed by "," or "};" depending on
// whether this is the last WLAN.
static void AppendMetricValue(std::ostringstream &s, double val,
		int w_count, int total_wlans_number) {
	s << std::fixed << std::setprecision(2) << val;
	s << (w_count < total_wlans_number - 1 ? "," : "};");
}

// case 15: AIML Magazine Paper — MAB-driven SR — 12 per-AP metric arrays
static void WriteOutput_MABDrivenSR(Performance *performance_report,
		Configuration *configuration_per_node, Logger &logger_script, int total_wlans_number,
		int total_nodes_number, Wlan *, double simulation_time_komondor, const SimulationStats &) {

	std::ostringstream tpt, airtime, sairtime, data_loss, rtscts_loss, nav, delay;
	std::ostringstream last_tpt, last_delay, last_sairtime, last_airtime, last_access_delay;

	int w_count = 0;
	for (int i = 0; i < total_nodes_number; ++i) {
		if (configuration_per_node[i].capabilities.node_type != NODE_TYPE_AP) continue;

		AppendMetricValue(tpt,    performance_report[i].throughput * pow(10,-6), w_count, total_wlans_number);
		AppendMetricValue(airtime, ((performance_report[i].total_time_transmitting_in_num_channels[0]
		                  - performance_report[i].total_time_lost_in_num_channels[i])
		                 * 100 / simulation_time_komondor), w_count, total_wlans_number);
		AppendMetricValue(sairtime, performance_report[i].total_time_transmitting_in_num_channels[0]
		                 * 100 / simulation_time_komondor, w_count, total_wlans_number);
		AppendMetricValue(data_loss, (double) 100*performance_report[i].data_packets_lost
		                  / performance_report[i].data_packets_sent, w_count, total_wlans_number);
		AppendMetricValue(rtscts_loss, (double) 100*performance_report[i].rts_cts_lost
		                    / performance_report[i].rts_cts_sent, w_count, total_wlans_number);
		AppendMetricValue(nav,   performance_report[i].time_in_nav / simulation_time_komondor * 100, w_count, total_wlans_number);
		AppendMetricValue(delay, performance_report[i].average_delay * pow(10,3), w_count, total_wlans_number);
		AppendMetricValue(last_tpt,   performance_report[i].last_throughput * pow(10,-6), w_count, total_wlans_number);
		AppendMetricValue(last_delay, performance_report[i].last_average_delay * pow(10,3), w_count, total_wlans_number);

		double total_airtime_val = 0, successful_airtime_val = 0;
		for (int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c) {
			total_airtime_val      += performance_report->last_total_time_transmitting_per_channel[c];
			successful_airtime_val += performance_report->last_total_time_transmitting_per_channel[c]
			                       - performance_report->last_total_time_lost_per_channel[c];
		}
		AppendMetricValue(last_sairtime, successful_airtime_val * 100 / performance_report->last_measurements_window, w_count, total_wlans_number);
		AppendMetricValue(last_airtime,  total_airtime_val      * 100 / performance_report->last_measurements_window, w_count, total_wlans_number);
		AppendMetricValue(last_access_delay, performance_report[i].last_average_access_delay * pow(10,3), w_count, total_wlans_number);

		++w_count;
	}

	fprintf(logger_script.file, ";%s%s%s%s%s%s%s%s%s%s%s%s\n",
		("{" + tpt.str()).c_str(), ("{" + airtime.str()).c_str(),
		("{" + sairtime.str()).c_str(), ("{" + data_loss.str()).c_str(),
		("{" + rtscts_loss.str()).c_str(), ("{" + nav.str()).c_str(),
		("{" + delay.str()).c_str(), ("{" + last_tpt.str()).c_str(),
		("{" + last_delay.str()).c_str(), ("{" + last_sairtime.str()).c_str(),
		("{" + last_airtime.str()).c_str(), ("{" + last_access_delay.str()).c_str());
}

// case 16: tokenized backoff — 9 per-AP metric arrays
static void WriteOutput_TokenizedBackoff(Performance *performance_report,
		Configuration *configuration_per_node, Logger &logger_script, int total_wlans_number,
		int total_nodes_number, Wlan *, double simulation_time_komondor, const SimulationStats &) {

	std::ostringstream tpt, airtime, sairtime, data_loss, rtscts_loss, nav, delay, access_delay, backoff;

	int w_count = 0;
	for (int i = 0; i < total_nodes_number; ++i) {
		if (configuration_per_node[i].capabilities.node_type != NODE_TYPE_AP) continue;

		AppendMetricValue(tpt,    performance_report[i].throughput * pow(10,-6), w_count, total_wlans_number);
		AppendMetricValue(airtime, performance_report[i].total_time_transmitting_in_num_channels[0]
		                * 100 / simulation_time_komondor, w_count, total_wlans_number);
		AppendMetricValue(sairtime, ((performance_report[i].total_time_transmitting_in_num_channels[0]
		                   - performance_report[i].total_time_lost_in_num_channels[i])
		                  * 100 / simulation_time_komondor), w_count, total_wlans_number);
		AppendMetricValue(data_loss, (double) 100*performance_report[i].data_packets_lost
		                  / performance_report[i].data_packets_sent, w_count, total_wlans_number);
		AppendMetricValue(rtscts_loss, (double) 100*performance_report[i].rts_cts_lost
		                    / performance_report[i].rts_cts_sent, w_count, total_wlans_number);
		AppendMetricValue(nav,          performance_report[i].time_in_nav / simulation_time_komondor * 100, w_count, total_wlans_number);
		AppendMetricValue(delay,        performance_report[i].average_delay * pow(10,3), w_count, total_wlans_number);
		AppendMetricValue(access_delay, performance_report[i].average_waiting_time * pow(10,3), w_count, total_wlans_number);
		AppendMetricValue(backoff,      performance_report[i].expected_backoff / SLOT_TIME, w_count, total_wlans_number);

		++w_count;
	}

	fprintf(logger_script.file, ";%s%s%s%s%s%s%s%s%s\n",
		("{" + tpt.str()).c_str(), ("{" + airtime.str()).c_str(),
		("{" + sairtime.str()).c_str(), ("{" + data_loss.str()).c_str(),
		("{" + rtscts_loss.str()).c_str(), ("{" + nav.str()).c_str(),
		("{" + delay.str()).c_str(), ("{" + access_delay.str()).c_str(),
		("{" + backoff.str()).c_str());
}

// ===========================================================================
// Dispatcher
// ===========================================================================

/**
* Generate the script output according to the simulation index.
*
* The simulation_index selects which metrics to write to the script output file.
* Each index corresponds to a specific project/experiment format:
*   0  — toy scenarios
*   1  — large scenarios (node density vs. throughput)
*   2  — central WLAN scenario (frame counters)
*   3  — Bianchi multiple WLANs
*   4  — DCB validation
*   5  — variability of optimal policies
*   6  — two-WLAN overlap / 3-WLAN line scenario
*   7  — central WLAN (Paper #5, extended)
*   8  — central WLAN (Paper #5, single-AP)
*   9  — 6-WLAN random scenario
*   10 — regression test (throughput per AP + RSSI per STA)
*   11 — spatial reuse tutorial
*   12 — spatial reuse CSCN
*   13 — FG-ML5G student project (per-device detail)
*   14 — RTOT algorithm for 11ax SR
*   15 — AIML Magazine Paper: MAB-driven SR
*   16 — tokenized backoff
*
* @param "simulation_index"       [type int]:         selects the output format
* @param "performance_report"     [type Performance*]: per-node performance
* @param "configuration_per_node" [type Configuration*]: per-node configuration
* @param "logger_script"          [type Logger]:       output file logger
* @param "total_wlans_number"     [type int]:           total number of WLANs
* @param "total_nodes_number"     [type int]:           total number of nodes
* @param "wlan_container"         [type Wlan*]:         WLAN array
* @param "simulation_time_komondor" [type double]:      total simulation time
*/
void GenerateScriptOutput(int simulation_index, Performance *performance_report,
		Configuration *configuration_per_node, Logger &logger_script,
		int total_wlans_number, int total_nodes_number,
		Wlan *wlan_container, double simulation_time_komondor) {

	// Compute aggregated statistics used by some output formats
	SimulationStats stats;
	ComputeSimulationStatistics(stats, performance_report, configuration_per_node,
		total_nodes_number, total_wlans_number);

	switch (simulation_index) {
		case 0:  WriteOutput_ToyScenario(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 1:  WriteOutput_LargeScenario_NodeDensity(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 2:  WriteOutput_CentralWlan(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 3:  WriteOutput_BianchiMultiWlan(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 4:  WriteOutput_DcbValidation(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 5:  WriteOutput_OptimalPolicyVariability(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 6:  WriteOutput_TwoWlanOverlap(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 7:  WriteOutput_CentralWlan_Paper5(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 8:  WriteOutput_CentralWlan_Paper5_Single(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 9:  WriteOutput_SixWlanRandom(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 10: WriteOutput_RegressionTest(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 11: WriteOutput_SpatialReuse_Tutorial(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 12: WriteOutput_SpatialReuse_CSCN(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 13: WriteOutput_FgMl5g_StudentProject(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 14: WriteOutput_RtotAlgorithm(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 15: WriteOutput_MABDrivenSR(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		case 16: WriteOutput_TokenizedBackoff(performance_report, configuration_per_node, logger_script, total_wlans_number, total_nodes_number, wlan_container, simulation_time_komondor, stats); break;
		default: printf("No simulation type found for index %d\n", simulation_index); break;
	}
}

#endif  // _OUT_METHODS_
