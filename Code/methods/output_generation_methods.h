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
 * output_generation_methods.h: this file contains functions related to the main Komondor's operation
 *
 * - This file contains the methods for generating the output of a given simulation (process simulation results and provide statistics)
 */

#include <math.h>
#include <stddef.h>
#include <string>
#include <sstream>

#include "../list_of_macros.h"
#include "../structures/performance.h"
#include "../structures/node_configuration.h"
#include "../structures/wlan.h"

#ifndef _OUT_METHODS_
#define _OUT_METHODS_

int total_data_packets_sent (0);				///< Total number of data packets sent
double total_num_packets_generated (0);			///< Total number of packets generated
double total_throughput (0);					///< Sum of the throughput obtained by each WLAN
int ix_wlan_min_throughput (99999);				///< Index of the WLAN experiencing the minimum throughput
double min_throughput (999999999999999999);		///< Minimum throughput across all the WLANs
double max_throughput (0);						///< Maximum throughput across all the WLANs
double proportional_fairness(0);				///< Proportional fairness across all the network
double jains_fairness (0);						///< Jain's fairness across all the network
double jains_fairness_aux (0);					///< Auxiliary value to compute the Jain's fairness
int total_rts_lost_slotted_bo (0);				///< Total number of RTS frames lost by slotted BO collisions
int total_rts_cts_sent (0);						///< Total number of RTS/CTS frames sent
double total_prob_slotted_bo_collision (0);		///< Total probability of noticing collisions by slotted BO
int total_num_tx_init_not_possible (0);			///< Total number transmissions that could not be initiated
double total_delay (0);							///< Total delay in the network
double max_delay (0);							///< Maximum delay across all the WLANs
double min_delay (9999999999);					///< Minimum delay across all the WLANs
double total_bandiwdth_tx (0);					///< Total bandwidth used for transmitting
double av_expected_backoff (0);					///< Average expected backoff across all the WLANs
double av_expected_waiting_time (0);			///< Average expected waiting time across all the WLANs

/**
* Compute the global statistics after finishing the simulation
* @param "performance_report" [type Performance*]: array containing the performance report of each WLAN
* @param "configuration_per_node" [type Configuration*]: array containing the final configuration of each WLAN
* @param "total_nodes_number" [type int]: total number of nodes
* @param "total_wlans_number" [type int]: total number of WLANs
*/
void ComputeSimulationStatistics(Performance *performance_report, Configuration *configuration_per_node,
		int total_nodes_number, int total_wlans_number){

	for(int m=0; m < total_nodes_number; ++m){
		// Take into account only APs (transmitters)
		if( configuration_per_node[m].capabilities.node_type == NODE_TYPE_AP ){
			// Data packets sent in total
			total_data_packets_sent = total_data_packets_sent + performance_report[m].data_packets_sent;
			// Aggregate throughput
			total_throughput = total_throughput + performance_report[m].throughput;
			// Total number of packets generated
			total_num_packets_generated = total_num_packets_generated + performance_report[m].num_packets_generated;
			// Total number of RTS lost due to slotted backoff collisions
			total_rts_lost_slotted_bo = total_rts_lost_slotted_bo + performance_report[m].rts_lost_slotted_bo;
			// Total number of RTS sent
			total_rts_cts_sent = total_rts_cts_sent + performance_report[m].rts_cts_sent;
			// Sum of probabilities of finding a collision by slotted backoff
			total_prob_slotted_bo_collision = total_prob_slotted_bo_collision + performance_report[m].prob_slotted_bo_collision;
			// Total number of trials for transmitting
			total_num_tx_init_not_possible = total_num_tx_init_not_possible + performance_report[m].num_tx_init_not_possible;
			// Proportional fairness metric
			proportional_fairness = proportional_fairness + log10(performance_report[m].throughput);
			// Jain's fairness metric
			jains_fairness_aux = jains_fairness_aux + pow(performance_report[m].throughput, 2);
			// Sum of the average delay experienced by each transmitter
			total_delay = total_delay + performance_report[m].average_delay;
			// Maximum average delay in the network
			if(performance_report[m].average_delay > max_delay) max_delay = performance_report[m].average_delay;
			// Minimum average delay in the network
			if(performance_report[m].average_delay < min_delay) min_delay = performance_report[m].average_delay;
			// Average expected backoff
			av_expected_backoff = av_expected_backoff + performance_report[m].expected_backoff;
			// Average waiting time before transmitting
			av_expected_waiting_time = av_expected_waiting_time + performance_report[m].average_waiting_time;
			// Total bandwidth used in transmissions
			total_bandiwdth_tx = total_bandiwdth_tx + performance_report[m].bandwidth_used_txing;
			// Minimum throughput experienced in the network
			if(performance_report[m].throughput < min_throughput) {
				ix_wlan_min_throughput = m;
				min_throughput = performance_report[m].throughput;
			}
			// Maximum throughput experienced in the network
			if(performance_report[m].throughput > max_throughput) max_throughput = performance_report[m].throughput;
		}
	}
	av_expected_backoff = av_expected_backoff / total_wlans_number;
	av_expected_waiting_time = av_expected_waiting_time / total_wlans_number;
	jains_fairness = pow(total_throughput, 2) / (total_nodes_number/2 * jains_fairness_aux); // Supposing that number_aps = number_nodes/2
}

/**
* Prints and write logs regarding global statistics
* @param "print_system_logs" [type int]: boolean indicating whether to print logs or not
* @param "save_system_logs" [type int]: boolean indicating whether to write logs or not
* @param "logger_simulation" [type Logger]: pointer to the logger that writes logs into a file
* @param "performance_report" [type Performance*]: array containing the performance report of each WLAN
* @param "configuration_per_node" [type Configuration*]: array containing the final configuration of each WLAN
* @param "total_nodes_number" [type int]: total number of nodes
* @param "total_wlans_number" [type int]: total number of WLANs
* @param "frame_length" [type double]: length of a frame in bits
* @param "max_num_packets_aggregated" [type int]: maximum number of packets to be aggregated
* @param "simulation_time_komondor" [type double]: total simulation time
*/
void PrintAndWriteSimulationStatistics(int print_system_logs, int save_system_logs, Logger &logger_simulation,
		Performance *performance_report, Configuration *configuration_per_node, int total_nodes_number,
		int total_wlans_number, double simulation_time_komondor) {

	// Compute global statistics
	ComputeSimulationStatistics(performance_report, configuration_per_node, total_nodes_number, total_wlans_number);

	// Print final statistics in console logs
	if (print_system_logs) {
		printf("\n%s General Statistics (NEW FUNCTION):\n", LOG_LVL1);
		printf("%s Average throughput per WLAN = %.3f Mbps\n",
			LOG_LVL2, (total_throughput * pow(10,-6) / total_wlans_number));
		printf("%s Min. throughput = %.2f Mbps (%.2f pkt/s)\n",
			LOG_LVL3, min_throughput * pow(10,-6), min_throughput / (configuration_per_node[0].frame_length
			* configuration_per_node[0].max_num_packets_aggregated));
		printf("%s Max. throughput = %.2f Mbps (%.2f pkt/s)\n",
			LOG_LVL3, max_throughput * pow(10,-6), max_throughput / (configuration_per_node[0].frame_length
			* configuration_per_node[0].max_num_packets_aggregated));
		printf("%s Total throughput = %.2f Mbps\n", LOG_LVL3, total_throughput * pow(10,-6));
		printf("%s Total number of packets sent = %d\n", LOG_LVL3, total_data_packets_sent);
		printf("%s Average number of data packets successfully sent per WLAN = %.2f\n",
			LOG_LVL4, ((double) total_data_packets_sent/ (double) total_wlans_number));
		printf("%s Average number of RTS packets lost due to slotted BO = %f (%.3f %% loss)\n",
			LOG_LVL4, (double) total_rts_lost_slotted_bo/(double) total_wlans_number,
			((double) total_rts_lost_slotted_bo *100/ (double) total_rts_cts_sent));
		printf("%s Average number of packets sent per WLAN = %d\n", LOG_LVL3, (total_data_packets_sent/total_wlans_number));
		printf("%s Proportional Fairness = %.2f\n", LOG_LVL2, proportional_fairness);
		printf("%s Jain's Fairness = %.2f\n",  LOG_LVL2, jains_fairness);
		printf("%s Prob. collision by slotted BO = %.3f\n", LOG_LVL2, total_prob_slotted_bo_collision / total_wlans_number);
		printf("%s Av. delay = %.2f ms\n", LOG_LVL2, total_delay * pow(10,3) / total_wlans_number);
		printf("%s Max. delay = %.2f ms\n", LOG_LVL3, max_delay * pow(10,3));
		printf("%s Av. expected waiting time = %.2f ms\n", LOG_LVL3, av_expected_waiting_time * pow(10,3));
		printf("%s Average bandwidth used for transmitting = %.2f MHz\n",
			LOG_LVL2, total_bandiwdth_tx / (double) total_wlans_number);
		printf("%s Time channel was idle = %.2f s (%f%%)\n",  LOG_LVL2, performance_report[0].sum_time_channel_idle,
			(100*performance_report[0].sum_time_channel_idle/simulation_time_komondor));
		printf("\n\n");
	}

	printf("\n");

	// Write final statistics in the file containing the console logs
	if (save_system_logs) {
		// Simulation log file
		fprintf(logger_simulation.file,"\n%s General Statistics (NEW FUNCTION):\n", LOG_LVL1);
		fprintf(logger_simulation.file,"%s Average throughput per WLAN = %.2f Mbps\n", LOG_LVL2, (total_throughput * pow(10,-6)/total_wlans_number));
		fprintf(logger_simulation.file,"%s Total throughput = %.2f Mbps\n", LOG_LVL3, total_throughput * pow(10,-6));
		fprintf(logger_simulation.file,"%s Total number of packets sent = %d\n", LOG_LVL3, total_data_packets_sent);
		fprintf(logger_simulation.file,"%s Average number of data packets successfully sent per WLAN = %.2f\n",
			LOG_LVL4, ( (double) total_data_packets_sent/ (double) total_wlans_number));
		fprintf(logger_simulation.file,"%s Average number of RTS packets lost due to slotted BO = %.2f (%.2f %% loss)\n",
			LOG_LVL4,
			(double) total_rts_lost_slotted_bo/(double) total_wlans_number,
			((double) total_rts_lost_slotted_bo *100/ (double) total_rts_cts_sent));
		fprintf(logger_simulation.file,"%s Average number of packets sent per WLAN = %d\n", LOG_LVL3, (total_data_packets_sent/total_wlans_number));
		fprintf(logger_simulation.file,"%s Proportional Fairness = %.2f\n", LOG_LVL2, proportional_fairness);
		fprintf(logger_simulation.file,"%s Jain's Fairness = %.2f\n",  LOG_LVL2, jains_fairness);
		fprintf(logger_simulation.file,"\n");
	}

}

/**
* Generates the script's output (.txt) according to the introduced simulation index
* @param "simulation_index" [type int]: simulation index that indicates which type of logs will be written
* @param "performance_report" [type Performance*]: array containing the performance report of each WLAN
* @param "configuration_per_node" [type Configuration*]: array containing the final configuration of each WLAN
* @param "logger_script" [type Logger]: pointer to the logger that writes logs into a file
* @param "total_wlans_number" [type int]: total number of WLANs
* @param "total_nodes_number" [type int]: total number of nodes
* @param "frame_length" [type double]: length of a frame in bits
* @param "max_num_packets_aggregated" [type int]: maximum number of packets to be aggregated
* @param "wlan_container" [type Wlan*]: array containing each WLAN in the network
* @param "simulation_time_komondor" [type double]: total simulation time
*/
void GenerateScriptOutput(int simulation_index, Performance *performance_report, Configuration *configuration_per_node,
	Logger &logger_script, int total_wlans_number, int	total_nodes_number, Wlan *wlan_container, double simulation_time_komondor) {

	// Generate the content for the "Script output"
	switch(simulation_index){

		case 0:{
			// For toy scenarios
			fprintf(logger_script.file, ";%.2f;%.2f;%f;%f\n",
				performance_report[0].throughput * pow(10,-6),
				performance_report[2].throughput * pow(10,-6),
				performance_report[0].prob_slotted_bo_collision,
				performance_report[2].prob_slotted_bo_collision);
			break;
		}

		case 1:{
			// For large scenarios (Node density vs. throughput)
			fprintf(logger_script.file, ";%.2f;%.2f;%f;%.2f;%d;%.2f\n",
				(total_throughput * pow(10,-6)/total_wlans_number),
				proportional_fairness,
				jains_fairness,
				min_throughput * pow(10,-6),
				ix_wlan_min_throughput,
				total_bandiwdth_tx / (double) total_wlans_number);
			break;
		}

		case 2:{
			// Sergio logs for central WLAN scenario
			fprintf(logger_script.file, ";%.1f;%d;%d;%d;%d;%d;%d;%d\n",
				performance_report[0].throughput * pow(10,-6),
				performance_report[0].rts_cts_sent,
				performance_report[0].rts_cts_lost,
				performance_report[0].rts_lost_slotted_bo,
				performance_report[0].data_packets_sent,
				performance_report[0].data_packets_lost,
				performance_report[0].num_tx_init_tried,
				performance_report[0].num_tx_init_not_possible);
			break;
		}

		case 3:{
			// Bianchi multiple WLANs
			fprintf(logger_script.file, ";%.2f;%.3f;%.5f\n",
				av_expected_backoff / SLOT_TIME,
				(total_throughput * pow(10,-6)/total_wlans_number),
				total_prob_slotted_bo_collision / total_wlans_number);
			break;
		}

		case 4:{
			// DCB validation
			fprintf(logger_script.file, ";%.5f",
				total_prob_slotted_bo_collision / total_wlans_number);
			for(int w = 0; w < total_wlans_number; ++w) {
				fprintf(logger_script.file, ";%.3f", performance_report[w*2].throughput * pow(10,-6));
			}
			fprintf(logger_script.file, "\n");
			break;
		}

		case 5:{
			// Variability of optimal policies
			for(int w = 0; w < total_wlans_number; ++w) {
				fprintf(logger_script.file, ";%.5f", performance_report[w*2].prob_slotted_bo_collision);
			}
			for(int w = 0; w < total_wlans_number; ++w) {
				fprintf(logger_script.file, ";%.3f", performance_report[w*2].throughput * pow(10,-6));
			}
			fprintf(logger_script.file, "\n");
			break;
		}

		case 6:{
			// Sergio logs for Paper #5 Toy Scenario I and II: 2 WLANs overlap scenario, and 3 line scenario
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
				(double) performance_report[0].num_packets_dropped * 100/ performance_report[0].num_packets_generated,
				(double) performance_report[2].num_packets_dropped * 100/ performance_report[2].num_packets_generated,
				(double) (performance_report[0].data_frames_acked / performance_report[0].data_packets_acked),
				(double) (performance_report[2].data_frames_acked / performance_report[2].data_packets_acked));
			break;
		}

		case 7:{
			// Sergio logs for Paper #5: central WLAN scenario
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
				(double) performance_report[0].num_packets_dropped * 100/ performance_report[0].num_packets_generated,
				(double) performance_report[0].data_frames_acked / performance_report[0].data_packets_acked
				);
			break;
		}

		case 8:{
			// Sergio logs for Paper #5: Central WLAN scenario
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
				(double) (performance_report[0].num_packets_dropped * 100/ performance_report[0].num_packets_generated),
				(double) (performance_report[0].data_frames_acked / performance_report[0].data_packets_acked)
				);
			break;
		}

		case 9:{
			// Sergio logs for Paper #5: 6 WLAN random
			fprintf(logger_script.file, ";%.2f;%.2f;%.2f;%d;%.4f;%.4f;%.4f;%.2f;%.2f;%.2f;%f;%f;%f\n",
				total_throughput/(configuration_per_node[0].frame_length *
					configuration_per_node[0].max_num_packets_aggregated * total_wlans_number),
				(total_throughput * pow(10,-6)/total_wlans_number),
				min_throughput/(configuration_per_node[0].frame_length *
					configuration_per_node[0].max_num_packets_aggregated),
				ix_wlan_min_throughput,
				proportional_fairness,
				jains_fairness,
				total_prob_slotted_bo_collision / total_wlans_number,
				total_delay * pow(10,3) / total_wlans_number,
				max_delay * pow(10,3),
				total_bandiwdth_tx / (double) total_wlans_number,
				av_expected_waiting_time * pow(10,3),
				min_delay * pow(10,3),
				max_throughput/(configuration_per_node[0].frame_length *
					configuration_per_node[0].max_num_packets_aggregated)
				);
			break;
		}

		// Regression test (scenarios paper Komondor - Wireless Days 2019)
		case 10:{
//			if (total_nodes_number == 2 || total_nodes_number == 3) {
//				// Basic scenarios (1 WLAN)
//				fprintf(logger_script.file, ";%.2f\n",
//					performance_report[0].throughput * pow(10,-6));
//			} else if (total_nodes_number == 6) {
//				// Complex scenarios (3 WLANs)
//				fprintf(logger_script.file, ";%.2f;%.2f;%.2f\n",
//					performance_report[0].throughput * pow(10,-6),
//					performance_report[2].throughput * pow(10,-6),
//					performance_report[4].throughput * pow(10,-6));
//			} else {
//				printf("Error in 'Komondor :: Stop()' ---> be careful of the desired generated logs (script)\n");
//			}

            //  - Throughput experienced/allocated for each device (AP and STAs)
            char tpt_per_device[250] = "{";
            char aux_tpt_per_device[50];
            // - RSSI in STAs from the associated AP
            char rssi_per_device[250] = "{";
            char aux_rssi_per_device[50];
            int counter_nodes_visited = 0;
            for(int i = 0; i < total_nodes_number; i ++) {
                if(configuration_per_node[i].capabilities.node_type == NODE_TYPE_AP) {
                    // Throughput (Mbps)
                    if(i > 0 && counter_nodes_visited < total_nodes_number) strcat(tpt_per_device, ",");
                    sprintf(aux_tpt_per_device, "%.2f", performance_report[i].throughput * pow(10,-6));
                    strcat(tpt_per_device, aux_tpt_per_device);
                    // RSSI received from the AP (dBm)
                    //strcat(rssi_per_device, "Inf,");
                    // Increase the number of visited nodes
                    ++counter_nodes_visited;
                    for(int w = 0; w < total_wlans_number; w ++) {
                        if(wlan_container[w].ap_id == configuration_per_node[i].capabilities.node_id) {
                            for(int s = 0; s < wlan_container[w].num_stas; s ++) {
                                // Array to store all details of STA
                                char sta_details[250] = "";
                                // RSSI received from the AP
                                sprintf(aux_rssi_per_device, "%.2f", ConvertPower(PW_TO_DBM, performance_report[counter_nodes_visited].received_power_array[i]));
                                strcat(rssi_per_device, aux_rssi_per_device);
                                strcat(sta_details, aux_rssi_per_device);
                                strcat(sta_details, ";");
                                // Increase the number of visited nodes
                                ++counter_nodes_visited;
                                if(counter_nodes_visited < total_nodes_number) {
                                    strcat(rssi_per_device, ",");
                                }
                            }
                        }
                    }
                }
            }
            strcat(tpt_per_device, "}");
            strcat(rssi_per_device, "}");
            // STEP 2: Print the data to the output .csv file
            fprintf(logger_script.file, ";%s;%s\n", tpt_per_device,rssi_per_device);
			break;
		}

		// SPATIAL REUSE - Tutorial
		case 11:{
			if (total_nodes_number == 2 || total_nodes_number == 3) {
				// Basic scenarios
				fprintf(logger_script.file, ";%.2f\n",
					performance_report[0].throughput * pow(10,-6));
			} else if (total_nodes_number == 4) {
				// Toy scenario 1
				fprintf(logger_script.file, ";%.2f;%.2f\n",
					performance_report[0].throughput * pow(10,-6),
					performance_report[2].throughput * pow(10,-6));
			} else if (total_nodes_number == 6) {
				// Toy scenario 2
				fprintf(logger_script.file, ";%.2f;%.2f;%.2f\n",
					performance_report[0].throughput * pow(10,-6),
					performance_report[2].throughput * pow(10,-6),
					performance_report[4].throughput * pow(10,-6));
			} else if (total_nodes_number == 18) {
				// Random scenarios
				fprintf(logger_script.file, ";%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f"
					";%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f"
					";%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\n",
				// Throughput per WLAN (Mbps)
				performance_report[0].throughput * pow(10,-6),
				performance_report[2].throughput * pow(10,-6),
				performance_report[4].throughput * pow(10,-6),
				performance_report[6].throughput * pow(10,-6),
				performance_report[8].throughput * pow(10,-6),
				performance_report[10].throughput * pow(10,-6),
				performance_report[12].throughput * pow(10,-6),
				performance_report[14].throughput * pow(10,-6),
				performance_report[16].throughput * pow(10,-6),
				// Time occupying the channel successfully (%)
				((performance_report[0].total_time_transmitting_in_num_channels[0]
				- performance_report[0].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
				((performance_report[2].total_time_transmitting_in_num_channels[0]
				- performance_report[2].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
				((performance_report[4].total_time_transmitting_in_num_channels[0]
				- performance_report[4].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
				((performance_report[6].total_time_transmitting_in_num_channels[0]
				- performance_report[6].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
				((performance_report[8].total_time_transmitting_in_num_channels[0]
				- performance_report[8].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
				((performance_report[10].total_time_transmitting_in_num_channels[0]
				- performance_report[10].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
				((performance_report[12].total_time_transmitting_in_num_channels[0]
				- performance_report[12].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
				((performance_report[14].total_time_transmitting_in_num_channels[0]
				- performance_report[14].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
				((performance_report[16].total_time_transmitting_in_num_channels[0]
				- performance_report[16].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
				// Average delay (ms)
				performance_report[0].average_waiting_time * pow(10,3),
				performance_report[2].average_waiting_time * pow(10,3),
				performance_report[4].average_waiting_time * pow(10,3),
				performance_report[6].average_waiting_time * pow(10,3),
				performance_report[8].average_waiting_time * pow(10,3),
				performance_report[10].average_waiting_time * pow(10,3),
				performance_report[12].average_waiting_time * pow(10,3),
				performance_report[14].average_waiting_time * pow(10,3),
				performance_report[16].average_waiting_time * pow(10,3));
			} else {
				printf("Error in Komondor :: Stop(): be care of the desired generated logs (script)\n");
			}
			break;
		}

		// SPATIAL REUSE - CSCN
		case 12:{
			// Random scenarios
			fprintf(logger_script.file, ";%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f"
				";%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f"
				";%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\n",
			// Throughput per WLAN (Mbps)
			performance_report[0].throughput * pow(10,-6),
			performance_report[2].throughput * pow(10,-6),
			performance_report[4].throughput * pow(10,-6),
			performance_report[6].throughput * pow(10,-6),
			performance_report[8].throughput * pow(10,-6),
			performance_report[10].throughput * pow(10,-6),
			performance_report[12].throughput * pow(10,-6),
			performance_report[14].throughput * pow(10,-6),
			performance_report[16].throughput * pow(10,-6),
			performance_report[18].throughput * pow(10,-6),
			// Time occupying the channel successfully (%)
			((performance_report[0].total_time_transmitting_in_num_channels[0]
			- performance_report[0].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[2].total_time_transmitting_in_num_channels[0]
			- performance_report[2].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[4].total_time_transmitting_in_num_channels[0]
			- performance_report[4].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[6].total_time_transmitting_in_num_channels[0]
			- performance_report[6].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[8].total_time_transmitting_in_num_channels[0]
			- performance_report[8].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[10].total_time_transmitting_in_num_channels[0]
			- performance_report[10].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[12].total_time_transmitting_in_num_channels[0]
			- performance_report[12].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[14].total_time_transmitting_in_num_channels[0]
			- performance_report[14].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[16].total_time_transmitting_in_num_channels[0]
			- performance_report[16].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			((performance_report[18].total_time_transmitting_in_num_channels[0]
			- performance_report[18].total_time_lost_in_num_channels[0])*100/simulation_time_komondor),
			// Average delay (ms)
			performance_report[0].average_waiting_time * pow(10,3),
			performance_report[2].average_waiting_time * pow(10,3),
			performance_report[4].average_waiting_time * pow(10,3),
			performance_report[6].average_waiting_time * pow(10,3),
			performance_report[8].average_waiting_time * pow(10,3),
			performance_report[10].average_waiting_time * pow(10,3),
			performance_report[12].average_waiting_time * pow(10,3),
			performance_report[14].average_waiting_time * pow(10,3),
			performance_report[16].average_waiting_time * pow(10,3),
			performance_report[18].average_waiting_time * pow(10,3));
			break;
		}

		// FG-ML5G - Students projects (Akshara P)
		case 13: {
			// STEP 1: Concatenate the information obtained from each STA in each WLAN
			//  - Label of each device (AP and STAs)
			char label_per_device[250] = "{";
			//  - Throughput experienced/allocated for each device (AP and STAs)
			char tpt_per_device[250] = "{";
			char aux_tpt_per_device[50];
			// - RSSI in STAs from the associated AP
			char rssi_per_device[250] = "{";
			char aux_rssi_per_device[50];
			// - Packets sent vs packets lost
			char data_loss_ratio_per_device[250] = "{";
			char aux_data_loss_ratio_per_device[50];
			// - RTS/CTS sent vs packets lost
			char rtscts_loss_ratio_per_device[250] = "{";
			char aux_rtscts_loss_ratio_per_device[50];
			// - Time each device is in NAV state
			char time_in_nav_per_device[250] = "{";
			char aux_time_in_nav_per_device[50];
			char aux_time_in_nav_per_sta[50];

			int counter_nodes_visited = 0;
			for(int i = 0; i < total_nodes_number; i ++) {
				// Node id
				strcat(label_per_device, configuration_per_node[i].capabilities.node_code.c_str());
				if (i < total_nodes_number - 1) strcat(label_per_device, ",");
				if(configuration_per_node[i].capabilities.node_type == NODE_TYPE_AP) {
					// Throughput (Mbps)
					sprintf(aux_tpt_per_device, "%.2f,", performance_report[i].throughput * pow(10,-6));
					strcat(tpt_per_device, aux_tpt_per_device);
					// RSSI received from the AP (dBm)
					strcat(rssi_per_device, "Inf,");
					// Data packets sent vs packets lost (%)
					sprintf(aux_data_loss_ratio_per_device, "%.2f,", (double)
						100*performance_report[i].data_packets_lost/performance_report[i].data_packets_sent);
					strcat(data_loss_ratio_per_device, aux_data_loss_ratio_per_device);
					// RTS/CTS packets sent vs packets lost (%)
					sprintf(aux_rtscts_loss_ratio_per_device, "%.2f,", (double)
						100*performance_report[i].rts_cts_lost/performance_report[i].rts_cts_sent);
					strcat(rtscts_loss_ratio_per_device, aux_rtscts_loss_ratio_per_device);
					// Increase the number of visited nodes
					++counter_nodes_visited;
					for(int w = 0; w < total_wlans_number; w ++) {
						if(wlan_container[w].ap_id == configuration_per_node[i].capabilities.node_id) {
							for(int s = 0; s < wlan_container[w].num_stas; s ++) {
								// Array to store all details of STA
								char sta_details[250] = "";
								// Throughput allocated to the STA
								sprintf(aux_tpt_per_device, "%.2f", performance_report[i].throughput_per_sta[s] * pow(10,-6));
								strcat(tpt_per_device, aux_tpt_per_device);
								strcat(sta_details, aux_tpt_per_device);
								strcat(sta_details, ";");
								// RSSI received from the AP
								sprintf(aux_rssi_per_device, "%.2f", ConvertPower(PW_TO_DBM,
									performance_report[counter_nodes_visited].received_power_array[i]));
								strcat(rssi_per_device, aux_rssi_per_device);
								strcat(sta_details, aux_rssi_per_device);
								strcat(sta_details, ";");
								// Data packets sent vs packets lost
								sprintf(aux_data_loss_ratio_per_device, "%.2f", (double)
									100*performance_report[i].data_packets_lost_per_sta[s]/
									performance_report[i].data_packets_sent_per_sta[s]);
								strcat(data_loss_ratio_per_device, aux_data_loss_ratio_per_device);
								strcat(sta_details, aux_data_loss_ratio_per_device);
								strcat(sta_details, ";");
								// RTS/CTS packets sent vs packets lost
								sprintf(aux_rtscts_loss_ratio_per_device, "%.2f", (double)
									100*performance_report[i].rts_cts_lost_per_sta[s]/
									performance_report[i].rts_cts_sent_per_sta[s]);
								strcat(rtscts_loss_ratio_per_device, aux_rtscts_loss_ratio_per_device);
								strcat(sta_details, aux_rtscts_loss_ratio_per_device);
								strcat(sta_details, ";");
								// Time in NAV for STA (%)
								sprintf(aux_time_in_nav_per_sta, "%.2f", performance_report[i+s+1].time_in_nav/simulation_time_komondor*100);
								strcat(sta_details,aux_time_in_nav_per_sta);
								// Increase the number of visited nodes
								++counter_nodes_visited;
								if(counter_nodes_visited < total_nodes_number){
									strcat(tpt_per_device, ",");
									strcat(rssi_per_device, ",");
									strcat(data_loss_ratio_per_device, ",");
									strcat(rtscts_loss_ratio_per_device, ",");}
								// Printing STA details row-wise
//								fprintf(logger_script.file, ";%s\n", sta_details);
							}
						}
					}
				}
				// Time in NAV per device (%)
				sprintf(aux_time_in_nav_per_device, "%.2f", performance_report[i].time_in_nav/simulation_time_komondor*100);
				strcat(time_in_nav_per_device, aux_time_in_nav_per_device);
				if(i < total_nodes_number-1) strcat(time_in_nav_per_device, ",");
			}
			strcat(label_per_device, "}");
			strcat(tpt_per_device, "}");
			strcat(rssi_per_device, "}");
			strcat(data_loss_ratio_per_device, "}");
			strcat(rtscts_loss_ratio_per_device, "}");
			strcat(time_in_nav_per_device, "}");

			// STEP 2: Print the data to the output .csv file
			fprintf(logger_script.file, ";%s;%s;%s;%s;%s;%s\n", label_per_device, tpt_per_device,rssi_per_device,
				data_loss_ratio_per_device,rtscts_loss_ratio_per_device,time_in_nav_per_device);

			break;
		}

		// RTOT algorithm for 11ax SR enhancement
		case 14: {
			//  - Throughput experienced/allocated for each device (AP and STAs)
			char tpt_array[250] = "";
			char aux_tpt[50];
			// Total airtime
			char airtime_array[250] = "";
			char aux_airtime[50];
			// Successful airtime
			char sairtime_array[250] = "";
			char aux_sairtime[50];
			// - RSSI in STAs from the associated AP
			char max_power_in_ap_per_wlan[1000] = "";
			char aux_power_in_ap[250];

			for(int i = 0; i < total_nodes_number; i ++) {
				if (configuration_per_node[i].capabilities.node_type == NODE_TYPE_AP) {
					// Throughput allocated to the STA
					sprintf(aux_tpt, "%.2f", performance_report[i].throughput * pow(10,-6));
					strcat(tpt_array, aux_tpt);
					strcat(tpt_array, ";");
					// Total airtime
					sprintf(aux_airtime, "%.2f", ((performance_report[i].total_time_transmitting_in_num_channels[0]
					 - performance_report[i].total_time_lost_in_num_channels[i])*100/simulation_time_komondor));
					strcat(airtime_array, aux_airtime);
					strcat(airtime_array, ";");
					// Successful airtime
					sprintf(aux_sairtime, "%.2f", (performance_report[i].total_time_transmitting_in_num_channels[0]*100/simulation_time_komondor));
					strcat(sairtime_array, aux_sairtime);
					strcat(sairtime_array, ";");
					// RSSI received from the AP (dBm)
					// Increase the number of visited nodes
					strcat(max_power_in_ap_per_wlan, "{");
					for(int w = 0; w < total_wlans_number; w ++) {
						// Array to store all details of STA
						// RSSI received from the AP
						sprintf(aux_power_in_ap, "%.2f", ConvertPower(PW_TO_DBM,
							performance_report[i].max_received_power_in_ap_per_wlan[w]));
						strcat(max_power_in_ap_per_wlan, aux_power_in_ap);
						// Increase the number of visited nodes
						if (w < total_wlans_number-1) strcat(max_power_in_ap_per_wlan, ",");
					}
					strcat(max_power_in_ap_per_wlan, "};");
				}
			}

			fprintf(logger_script.file, ";%s%s%s%s\n", tpt_array, airtime_array, sairtime_array, max_power_in_ap_per_wlan);
			break;

		}

		// AIML Magazine Paper - MAB-driven SR
		case 15: {
			//  - Throughput experienced/allocated for each device (AP and STAs)
			char tpt_array[250] = "{";
			char aux_tpt[50];
			// Total airtime
			char airtime_array[250] = "{";
			char aux_airtime[50];
			// Successful airtime
			char sairtime_array[250] = "{";
			char aux_sairtime[50];
			// - Packets sent vs packets lost
			char data_loss_ratio[250] = "{";
			char aux_data_loss_ratio[50];
			// - RTS/CTS sent vs packets lost
			char rtscts_loss_ratio[250] = "{";
			char aux_rtscts_loss_ratio[50];
			// - Time each device is in NAV state
			char time_in_nav_per_device[250] = "{";
			char aux_time_in_nav_per_device[50];
			// - Average delay
			char av_delay_per_device[250] = "{";
			char aux_av_delay_per_device[50];
			// - Last throughput
			char last_tpt_array[250] = "{";
			char aux_last_tpt_array[50];
			// - Last Average delay
			char last_av_delay_per_device[250] = "{";
			char aux_last_av_delay_per_device[50];
			// - Last successful airtime delay
			char last_sairtime[250] = "{";
			char aux_last_sairtime[50];
			// - Last airtime
			char last_airtime[250] = "{";
			char aux_last_airtime[50];
			// - Last average access delay
			char last_av_access_delay_per_device[250] = "{";
			char aux_last_av_access_delay_per_device[50];

			int w_count = 0;
			for(int i = 0; i < total_nodes_number; i ++) {
				if (configuration_per_node[i].capabilities.node_type == NODE_TYPE_AP) {
					// Throughput allocated to the STA
					sprintf(aux_tpt, "%.2f", performance_report[i].throughput * pow(10,-6));
					strcat(tpt_array, aux_tpt);
					if (w_count < total_wlans_number-1) {
						strcat(tpt_array, ",");
					} else {
						strcat(tpt_array, "};");
					}
					// Successful airtime
					sprintf(aux_airtime, "%.2f", ((performance_report[i].total_time_transmitting_in_num_channels[0]
					 - performance_report[i].total_time_lost_in_num_channels[i])*100/simulation_time_komondor));
					strcat(airtime_array, aux_airtime);
					if (w_count < total_wlans_number-1) {
						strcat(airtime_array, ",");
					} else {
						strcat(airtime_array, "};");
					}
					// Airtime
					sprintf(aux_sairtime, "%.2f", (performance_report[i].total_time_transmitting_in_num_channels[0]*100/simulation_time_komondor));
					strcat(sairtime_array, aux_sairtime);
					if (w_count < total_wlans_number-1) {
						strcat(sairtime_array, ",");
					} else {
						strcat(sairtime_array, "};");
					}
					// Data packets sent vs packets lost
					sprintf(aux_data_loss_ratio, "%.2f", (double)
						100*performance_report[i].data_packets_lost/
						performance_report[i].data_packets_sent);
					strcat(data_loss_ratio, aux_data_loss_ratio);
					if (w_count < total_wlans_number-1) {
						strcat(data_loss_ratio, ",");
					} else {
						strcat(data_loss_ratio, "};");
					}
					// RTS/CTS packets sent vs packets lost
					sprintf(aux_rtscts_loss_ratio, "%.2f", (double)
						100*performance_report[i].rts_cts_lost/
						performance_report[i].rts_cts_sent);
					strcat(rtscts_loss_ratio, aux_rtscts_loss_ratio);
					if (w_count < total_wlans_number-1) {
						strcat(rtscts_loss_ratio, ",");
					} else {
						strcat(rtscts_loss_ratio, "};");
					}

					// Frames sent vs frames lost
					//sprintf(aux_fame_loss_ratio_per_device, "%.2f", (double)
					//	100*(performance_report[i].data_packets_lost_per_sta[0] + performance_report[i].rts_cts_lost_per_sta[0])/
					//	(performance_report[i].data_packets_sent_per_sta[0] + performance_report[i].rts_cts_sent_per_sta[0]));
					//strcat(frame_loss_ratio_per_sta, aux_fame_loss_ratio_per_device);
					//if (w_count < total_wlans_number-1) {
					//	strcat(frame_loss_ratio_per_sta, ",");
					//} else {
					//	strcat(frame_loss_ratio_per_sta, "};");
					//}


					// Time in NAV per device (%)
					sprintf(aux_time_in_nav_per_device, "%.2f", performance_report[i].time_in_nav/simulation_time_komondor*100);
					strcat(time_in_nav_per_device, aux_time_in_nav_per_device);
					if (w_count < total_wlans_number-1) {
						strcat(time_in_nav_per_device, ",");
					} else {
						strcat(time_in_nav_per_device, "};");
					}
					// Average delay per device (ms)
					sprintf(aux_av_delay_per_device, "%.2f", performance_report[i].average_delay * pow(10,3));
					strcat(av_delay_per_device, aux_av_delay_per_device);
					if (w_count < total_wlans_number-1) {
						strcat(av_delay_per_device, ",");
					} else {
						strcat(av_delay_per_device, "};");
					}
					// LAST throughput per device (Mbps)
					sprintf(aux_last_tpt_array, "%.2f", performance_report[i].last_throughput * pow(10,-6));
					strcat(last_tpt_array, aux_last_tpt_array);
					if (w_count < total_wlans_number-1) {
						strcat(last_tpt_array, ",");
					} else {
						strcat(last_tpt_array, "};");
					}
					// LAST average delay per device (ms)
					sprintf(aux_last_av_delay_per_device, "%.2f", performance_report[i].last_average_delay * pow(10,3));
					strcat(last_av_delay_per_device, aux_last_av_delay_per_device);
					if (w_count < total_wlans_number-1) {
						strcat(last_av_delay_per_device, ",");
					} else {
						strcat(last_av_delay_per_device, "};");
					}
					// LAST successful airtime / total airtime (%)
					double total_airtime(0);
					double successful_airtime(0);
					for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
						total_airtime += performance_report->last_total_time_transmitting_per_channel[c];
						successful_airtime += performance_report->last_total_time_transmitting_per_channel[c]
						                      - performance_report->last_total_time_lost_per_channel[c];
					}
					sprintf(aux_last_sairtime, "%.2f", (successful_airtime*100/performance_report->last_measurements_window));
					strcat(last_sairtime, aux_last_sairtime);
					if (w_count < total_wlans_number-1) {
						strcat(last_sairtime, ",");
					} else {
						strcat(last_sairtime, "};");
					}
					// LAST airtime (%)
					sprintf(aux_last_airtime, "%.2f", (total_airtime*100/performance_report->last_measurements_window));
					strcat(last_airtime, aux_last_airtime);
					if (w_count < total_wlans_number-1) {
						strcat(last_airtime, ",");
					} else {
						strcat(last_airtime, "};");
					}
					// LAST average access delay (ms)
					sprintf(aux_last_av_access_delay_per_device, "%.2f", performance_report[i].last_average_access_delay * pow(10,3));
					strcat(last_av_access_delay_per_device, aux_last_av_access_delay_per_device);
					if (w_count < total_wlans_number-1) {
						strcat(last_av_access_delay_per_device, ",");
					} else {
						strcat(last_av_access_delay_per_device, "};");
					}


					++w_count;

				}
			}

			fprintf(logger_script.file, ";%s%s%s%s%s%s%s%s%s%s%s%s\n", tpt_array, airtime_array, sairtime_array, data_loss_ratio,
					rtscts_loss_ratio, time_in_nav_per_device, av_delay_per_device, last_tpt_array, last_av_delay_per_device,
					last_sairtime, last_airtime, last_av_access_delay_per_device);
			break;

		}

		// TOKENIZED BACKOFF
		case 16: {
			//  - Throughput experienced/allocated for each device (AP and STAs)
			char tpt_array[250] = "{";
			char aux_tpt[50];
			// Total airtime
			char airtime_array[250] = "{";
			char aux_airtime[50];
			// Successful airtime
			char sairtime_array[250] = "{";
			char aux_sairtime[50];
			// - Packets sent vs packets lost
			char data_loss_ratio[250] = "{";
			char aux_data_loss_ratio[50];
			// - RTS/CTS sent vs packets lost
			char rtscts_loss_ratio[250] = "{";
			char aux_rtscts_loss_ratio[50];
			// - Time each device is in NAV state
			char time_in_nav_per_device[250] = "{";
			char aux_time_in_nav_per_device[50];
			// - Average packet delay
			char av_delay_per_device[250] = "{";
			char aux_av_delay_per_device[50];
			// - Average channel access delay
			char av_access_delay_per_device[250] = "{";
			char aux_av_access_delay_per_device[50];
			// - Average backoff
			char av_backoff_per_device[250] = "{";
			char aux_av_backoff_per_device[50];


			int w_count = 0;
			for(int i = 0; i < total_nodes_number; i ++) {
				if (configuration_per_node[i].capabilities.node_type == NODE_TYPE_AP) {
					// Throughput allocated to the STA
					sprintf(aux_tpt, "%.2f", performance_report[i].throughput * pow(10,-6));
					strcat(tpt_array, aux_tpt);
					if (w_count < total_wlans_number-1) {
						strcat(tpt_array, ",");
					} else {
						strcat(tpt_array, "};");
					}
					// Airtime
					sprintf(aux_airtime, "%.2f", (performance_report[i].total_time_transmitting_in_num_channels[0]*100/simulation_time_komondor));
					strcat(airtime_array, aux_airtime);
					if (w_count < total_wlans_number-1) {
						strcat(airtime_array, ",");
					} else {
						strcat(airtime_array, "};");
					}
					// Successful airtime
					sprintf(aux_sairtime, "%.2f", ((performance_report[i].total_time_transmitting_in_num_channels[0]
					 - performance_report[i].total_time_lost_in_num_channels[i])*100/simulation_time_komondor));
					strcat(sairtime_array, aux_sairtime);
					if (w_count < total_wlans_number-1) {
						strcat(sairtime_array, ",");
					} else {
						strcat(sairtime_array, "};");
					}
					// Data packets sent vs packets lost
					sprintf(aux_data_loss_ratio, "%.2f", (double)
						100*performance_report[i].data_packets_lost/
						performance_report[i].data_packets_sent);
					strcat(data_loss_ratio, aux_data_loss_ratio);
					if (w_count < total_wlans_number-1) {
						strcat(data_loss_ratio, ",");
					} else {
						strcat(data_loss_ratio, "};");
					}
					// RTS/CTS packets sent vs packets lost
					sprintf(aux_rtscts_loss_ratio, "%.2f", (double)
						100*performance_report[i].rts_cts_lost/
						performance_report[i].rts_cts_sent);
					strcat(rtscts_loss_ratio, aux_rtscts_loss_ratio);
					if (w_count < total_wlans_number-1) {
						strcat(rtscts_loss_ratio, ",");
					} else {
						strcat(rtscts_loss_ratio, "};");
					}
					// Time in NAV per device (%)
					sprintf(aux_time_in_nav_per_device, "%.2f", performance_report[i].time_in_nav/simulation_time_komondor*100);
					strcat(time_in_nav_per_device, aux_time_in_nav_per_device);
					if (w_count < total_wlans_number-1) {
						strcat(time_in_nav_per_device, ",");
					} else {
						strcat(time_in_nav_per_device, "};");
					}
					// Average delay per device (ms)
					sprintf(aux_av_delay_per_device, "%.2f", performance_report[i].average_delay * pow(10,3));
					strcat(av_delay_per_device, aux_av_delay_per_device);
					if (w_count < total_wlans_number-1) {
						strcat(av_delay_per_device, ",");
					} else {
						strcat(av_delay_per_device, "};");
					}
					// Average access delay (ms)
					sprintf(aux_av_access_delay_per_device, "%.2f", performance_report[i].average_waiting_time * pow(10,3));
					strcat(av_access_delay_per_device, aux_av_access_delay_per_device);
					if (w_count < total_wlans_number-1) {
						strcat(av_access_delay_per_device, ",");
					} else {
						strcat(av_access_delay_per_device, "};");
					}
					// Average backoff (# slots)
					sprintf(aux_av_backoff_per_device, "%.2f", performance_report[i].expected_backoff/SLOT_TIME);
					strcat(av_backoff_per_device, aux_av_backoff_per_device);
					if (w_count < total_wlans_number-1) {
						strcat(av_backoff_per_device, ",");
					} else {
						strcat(av_backoff_per_device, "};");
					}

					++w_count;

				}
			}

			fprintf(logger_script.file, ";%s%s%s%s%s%s%s%s%s\n", tpt_array, airtime_array, sairtime_array, data_loss_ratio,
					rtscts_loss_ratio, time_in_nav_per_device, av_delay_per_device, av_access_delay_per_device, av_backoff_per_device);
			break;

		}

		default:{
		  printf("No simulation type found\n");
		  break;
		}

	}

}

#endif
