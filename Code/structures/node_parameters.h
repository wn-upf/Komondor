#ifndef NODE_PARAMETERS_H
#define NODE_PARAMETERS_H

#include <string>
#include <cstdio>
#include "../list_of_macros.h"
#include "../methods/channel/power_channel_methods.h"
#include "logger.h"

/**
 * NodeParameters - configuration parameters for a single Node instance.
 * Contains both static input-derived parameters and mutable operational state.
 */
struct NodeParameters {

	// --- Simulation-wide context ---
	double      simulation_time_komondor;	///> Observation time [s]
	int         total_wlans_number;			///> Total number of WLANs
	int         total_nodes_number;			///> Total number of nodes
	int         collisions_model;			///> Collisions model
	double      capture_effect;				///> Capture effect threshold [linear ratio]
	double      constant_per;				///> Constant PER for correct transmissions
	int         save_node_logs;				///> Flag: write node logs to file
	int         print_node_logs;			///> Flag: print node logs to stdout
	std::string simulation_code;			///> Komondor simulation code
	int         capture_effect_model;		///> Capture effect model
	int         nack_activated;				///> Flag: NACK mechanism enabled

	// --- Node identity ---
	std::string wlan_code;					///> WLAN code this node belongs to
	int         node_id;					///> Node identifier
	std::string node_code;					///> Node name (display only)
	int         node_type;					///> Node type (AP, STA, ...)

	// --- Position ---
	double      x;							///> X position [m]
	double      y;							///> Y position [m]
	double      z;							///> Z position [m]

	// --- Packet / traffic ---
	int         pdf_tx_time;				///> TX time distribution (0: exponential, 1: deterministic)
	int         frame_length;				///> Data frame length [bits]
	int         max_num_packets_aggregated;	///> Maximum packets per A-MPDU
	int         traffic_model;				///> Traffic model (0: full buffer, 1: Poisson, 2: deterministic)

	// --- PHY / channel ---
	int         adjacent_channel_model;		///> Co-channel interference model
	int         pifs_activated;				///> PIFS mechanism activation flag
	double      central_frequency;			///> Central frequency [Hz]
	int         path_loss_model;			///> Path loss model (0: free-space, ...)
	int         min_channel_allowed;		///> Min. allowed channel
	int         max_channel_allowed;		///> Max. allowed channel
	int         num_channels_allowed;		///> Max. channels allowed for TX
	double      tx_power_default;			///> Default TX power [pW]
	double      sensitivity_default;		///> Default sensitivity threshold [pW]

	// --- MAC / backoff ---
	int         backoff_type;				///> Backoff type (0: slotted, 1: continuous)
	int         cw_adaptation;				///> CW adaptation (0: constant, 1: BEB)
	int         cw_min_default;				///> Default minimum Contention Window
	int         cw_max_default;				///> Default maximum Contention Window
	int         cw_stage_max;				///> Maximum CW backoff stage
	
	// --- Spatial reuse ---
	int         bss_color;					///> BSS color
	int         srg;						///> Spatial Reuse Group (SRG)

	// --- Current / mutable configuration ---
	// Initialized from input; may be updated during simulation (e.g., by ApplyNewConfiguration).
	int         current_primary_channel;	///> Currently active primary channel
	int         current_max_bandwidth;		///> Currently active max bandwidth [no. of 20-MHz channels]
	int         current_dcb_policy;			///> Currently active channel bonding model
	double      non_srg_obss_pd;			///> Non-SRG OBSS_PD threshold [pW]
	double      srg_obss_pd;				///> SRG OBSS_PD threshold [pW]

	/**
	 * Print the node parameters / capabilities
	 */
	void PrintCapabilities() {
		printf("%s Capabilities of node %d:\n", LOG_LVL3, node_id);
		printf("%s node_type = %d\n", LOG_LVL4, node_type);
		printf("%s position = (%.2f, %.2f, %.2f)\n", LOG_LVL4, x, y, z);
		printf("%s primary_channel = %d\n", LOG_LVL4, current_primary_channel);
		printf("%s min_channel_allowed = %d\n", LOG_LVL4, min_channel_allowed);
		printf("%s max_channel_allowed = %d\n", LOG_LVL4, max_channel_allowed);
		printf("%s current_max_bandwidth = %d\n", LOG_LVL4, current_max_bandwidth);
		printf("%s tx_power_default = %f pW (%f dBm)\n", LOG_LVL4, tx_power_default, ConvertPower(PW_TO_DBM, tx_power_default));
		printf("%s sensitivity_default = %f pW (%f dBm)\n", LOG_LVL4, sensitivity_default, ConvertPower(PW_TO_DBM, sensitivity_default));
		printf("\n");
	}

	/**
	 * Write the node parameters / capabilities to a log file
	 * @param "logger" [type Logger]: logger object
	 * @param "sim_time" [type double]: current simulation time
	 */
	void WriteCapabilities(Logger logger, double sim_time) {
		fprintf(logger.file, "%.15f;N%d;%s;%s WLAN capabilities:\n", sim_time, node_id, LOG_F00, LOG_LVL3);
		fprintf(logger.file, "%.15f;N%d;%s;%s node_type = %d\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, node_type);
		fprintf(logger.file, "%.15f;N%d;%s;%s position = (%.2f, %.2f, %.2f)\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, x, y, z);
		fprintf(logger.file, "%.15f;N%d;%s;%s primary_channel = %d\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, current_primary_channel);
		fprintf(logger.file, "%.15f;N%d;%s;%s min_channel_allowed = %d\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, min_channel_allowed);
		fprintf(logger.file, "%.15f;N%d;%s;%s max_channel_allowed = %d\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, max_channel_allowed);
		fprintf(logger.file, "%.15f;N%d;%s;%s current_max_bandwidth = %d\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, current_max_bandwidth);
		fprintf(logger.file, "%.15f;N%d;%s;%s tx_power_default = %f pW (%f dBm)\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, tx_power_default, ConvertPower(PW_TO_DBM, tx_power_default));
		fprintf(logger.file, "%.15f;N%d;%s;%s sensitivity_default = %f pW (%f dBm)\n",
			sim_time, node_id, LOG_F00, LOG_LVL4, sensitivity_default, ConvertPower(PW_TO_DBM, sensitivity_default));
	}
};

#endif /* NODE_PARAMETERS_H */
