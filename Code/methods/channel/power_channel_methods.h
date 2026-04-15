/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
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
 * power_channel_methods.h: power calculations, path loss models, channel management,
 * interference, and logging utilities.
 *
 * Channel bonding and channel selection logic has been extracted to channel_bonding_methods.h,
 * which is included at the bottom of this file for backwards compatibility.
 */

#include <stddef.h>
#include <math.h>
#include <iostream>

#include "../../list_of_macros.h"
#include "../../structures/modulations.h"
#include "../utils/auxiliary_methods.h"
#include "beamforming_methods.h"

#ifndef _POWER_METHODS_
#define _POWER_METHODS_

// ===========================================================================
// Power conversion
// ===========================================================================

/**
* Convert power units
* @param "conversion_type" [type int]: type of conversion
* @param "power_magnitude_in" [type double]: input value to be converted
* @return "converted_power" [type double]: converted value
*/
double ConvertPower(int conversion_type, double power_magnitude_in){

  double converted_power (0);

  switch(conversion_type){
    case PW_TO_DBM:{
      converted_power = 10 * log10(power_magnitude_in * pow(10,-9));
      break;
    }
    case DBM_TO_PW:{
      converted_power = pow(10,(power_magnitude_in + 90)/10);
      break;
    }
    case MW_TO_DBM:{
      converted_power = 10 * log10(power_magnitude_in);
      break;
    }
    case DBM_TO_MW:
    case DB_TO_LINEAR:
    case DBW_TO_W: {
      converted_power = pow(10,power_magnitude_in/10);
      break;
    }
    case W_TO_DBW:
    case LINEAR_TO_DB: {
      converted_power = 10 * log10(power_magnitude_in);
      break;
    }
    default:{
      printf("Power conversion type not found!\n");
      break;
    }
  }
  return converted_power;
}

// ===========================================================================
// Distance
// ===========================================================================

/**
* Compute the distance between two 3D points
* @param "x1,y1,z1" [type double]: position of the first object
* @param "x2,y2,z2" [type double]: position of the second object
* @return "distance" [type double]: distance in meters
*/
double ComputeDistance(double x1, double y1, double z1, double x2, double y2, double z2){
  return sqrt(pow(x1-x2,2) + pow(y1-y2,2) + pow(z1-z2,2));
}

// ===========================================================================
// Path loss model helpers (static — used only by ComputePowerReceived)
// ===========================================================================

static double PathLoss_FreeSpace(double tx_power, double distance, double central_frequency) {
	double tx_gain = ConvertPower(DB_TO_LINEAR, ANTENNA_TX_GAIN_DB);
	double rx_gain = ConvertPower(DB_TO_LINEAR, ANTENNA_RX_GAIN_DB);
	return tx_power * tx_gain * rx_gain *
	       pow(((double) SPEED_LIGHT / (4 * M_PI * distance * central_frequency)), 2);
}

static double PathLoss_OkumuraHata(double tx_power, double distance, double wavelength) {
	double tx_power_dbm = ConvertPower(PW_TO_DBM, tx_power);
	double tx_heigth (10), rx_heigth (10);
	double path_loss_A = 69.55 + 26.16 * log10(3*pow(10,8)/wavelength) - 13.82 * log10(tx_heigth);
	double path_loss_B = 44.9 - 6.55 * log10(tx_heigth);
	double path_loss_E = 3.2 * pow(log10(11.7554 * rx_heigth), 2) - 4.97;
	double path_loss   = path_loss_A + path_loss_B * log10(distance/1000) - path_loss_E;
	return ConvertPower(DBM_TO_PW, tx_power_dbm + ANTENNA_TX_GAIN_DB + ANTENNA_RX_GAIN_DB - path_loss);
}

static double PathLoss_Indoor(double tx_power, double distance) {
	double tx_power_dbm = ConvertPower(PW_TO_DBM, tx_power);
	double path_loss_factor (5), shadowing (9.5), obstacles (30), walls_frequency (5), alpha (4.4);
	double shadowing_at_wlan  = (((double) rand())/RAND_MAX) * shadowing;
	double obstacles_at_wlan  = (((double) rand())/RAND_MAX) * obstacles;
	double path_loss = path_loss_factor + 10*alpha*log10(distance)
	                 + shadowing_at_wlan + (distance/walls_frequency)*obstacles_at_wlan;
	return ConvertPower(DBM_TO_PW, tx_power_dbm + ANTENNA_TX_GAIN_DB + ANTENNA_RX_GAIN_DB - path_loss);
}

static double PathLoss_Indoor2(double tx_power, double distance) {
	double tx_power_dbm = ConvertPower(PW_TO_DBM, tx_power);
	double path_loss_factor (5), shadowing (9.5), obstacles (30), walls_frequency (5), alpha (4.4);
	double shadowing_at_wlan  = 1.0/2 * shadowing;
	double obstacles_at_wlan  = 1.0/2 * obstacles;
	double path_loss = path_loss_factor + 10*alpha*log10(distance)
	                 + shadowing_at_wlan + (distance/walls_frequency)*obstacles_at_wlan;
	return ConvertPower(DBM_TO_PW, tx_power_dbm + ANTENNA_TX_GAIN_DB + ANTENNA_RX_GAIN_DB - path_loss);
}

// Residential — TGax scenario #1
// https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx
static double PathLoss_TGax_Scenario1(double tx_power, double distance, double central_frequency) {
	double tx_power_dbm = ConvertPower(PW_TO_DBM, tx_power);
	int n_walls (10), n_floors (3), L_iw (5);
	double min_d = (distance > 5) ? 5.0 : distance;
	double cf_ghz = central_frequency / pow(10, 9);
	double LFS = 40.05 + 20*log10(cf_ghz/2.4) + 20*log10(min_d)
	           + 18.3 * pow((distance/n_floors),
	                        (((distance/n_floors)+2)/((distance/n_floors)+1)) - 0.46)
	           + L_iw * (distance/n_walls);
	double d_BP (5);
	double loss = (distance >= d_BP) ? LFS + 35*log10(distance/5.0) : LFS;
	return ConvertPower(DBM_TO_PW, tx_power_dbm + ANTENNA_TX_GAIN_DB + ANTENNA_RX_GAIN_DB - loss);
}

// Enterprise — TGax scenario #2
static double PathLoss_TGax_Scenario2(double tx_power, double distance, double central_frequency) {
	double tx_power_dbm = ConvertPower(PW_TO_DBM, tx_power);
	double f_walls = 12.0/20;
	double min_d = (distance > 10) ? 10.0 : distance;
	double cf_ghz = central_frequency / pow(10, 9);
	double shadowing (5);
	double shadowing_at_wlan = (((double) rand())/RAND_MAX) * shadowing;
	double LFS = 40.05 + 20*log10(cf_ghz/2.4) + 20*log10(min_d)
	           + 7*(distance*f_walls) + shadowing_at_wlan;
	int d_BP (1);
	double loss = (distance >= d_BP) ? LFS + 35*log10(distance/10.0) : LFS;
	return ConvertPower(DBM_TO_PW, tx_power_dbm + ANTENNA_TX_GAIN_DB + ANTENNA_RX_GAIN_DB - loss);
}

// Indoor small BSSs — TGax scenario #3
static double PathLoss_TGax_Scenario3(double tx_power, double distance, double central_frequency) {
	double tx_power_dbm = ConvertPower(PW_TO_DBM, tx_power);
	double LFS = 32.4 + 20*log10(central_frequency/1e6) + 20*log10(distance/1000);
	int d_BP (10);
	double loss = (distance >= d_BP) ? LFS + 35*log10(distance/double(d_BP)) : LFS;
	return ConvertPower(DBM_TO_PW, tx_power_dbm + ANTENNA_TX_GAIN_DB + ANTENNA_RX_GAIN_DB - loss);
}

// Outdoor large BSS — TGax scenario #4
static double PathLoss_TGax_Scenario4(double tx_power, double distance, double central_frequency) {
	double tx_power_dbm = ConvertPower(PW_TO_DBM, tx_power);
	double h_AP (10), h_STA (1.5);
	double d_BP = (4 * (h_AP - 1) * (h_STA - 1) * central_frequency) / SPEED_LIGHT;
	double loss (0);
	if (distance < d_BP && distance >= 10)
		loss = 22*log10(distance) + 28 + 20*log10(central_frequency * pow(10,-9));
	else if (distance >= d_BP && distance < 5000)
		loss = 40*log10(distance) + 7.8 - 18*log10(h_AP-1) - 18*log10(h_STA-1)
		     + 20*log10(central_frequency * pow(10,-9));
	return ConvertPower(DBM_TO_PW, tx_power_dbm + ANTENNA_TX_GAIN_DB + ANTENNA_RX_GAIN_DB - loss);
}

// Outdoor large BSS + Residential — TGax scenario #4a
static double PathLoss_TGax_Scenario4a(double tx_power, double distance, double central_frequency) {
	double tx_power_dbm = ConvertPower(PW_TO_DBM, tx_power);
	double loss (0);
	if (distance < 2000 && distance >= 10)
		loss = 36.7*log10(distance) + 22.7 + 26*log10(central_frequency * pow(10,-9));
	// Outdoor-to-indoor building penetration loss (additive, d_indoor=0 until geometry is known)
	// TODO: derive d_outdoor and d_indoor from node positions
	double d_indoor (0);
	loss += 20 + 0.5 * d_indoor;
	return ConvertPower(DBM_TO_PW, tx_power_dbm + ANTENNA_TX_GAIN_DB + ANTENNA_RX_GAIN_DB - loss);
}

// 5 GHz office building (Medbo & Berg, VTC 2000)
static double PathLoss_5GHz_Office(double tx_power, double distance, double central_frequency) {
	double alpha (0.44);
	double pl_free_space_db = 20*log10(distance) + 20*log10(central_frequency/1e9)
	                        + 20*log10((4*M_PI)/((double) SPEED_LIGHT))
	                        - ANTENNA_RX_GAIN_DB - ANTENNA_TX_GAIN_DB;
	double pl_overall_db = pl_free_space_db + alpha * distance;
	return ConvertPower(DBM_TO_PW, ConvertPower(PW_TO_DBM, tx_power) - pl_overall_db);
}

// In-room LoS at 5.25 GHz (Xu et al., VTC 2007) — one-slope log-distance model
static double PathLoss_InRoom_5250KHz(double tx_power, double distance) {
	double pl_overall_db = 47.8 + 14.8 * log10(distance);
	return ConvertPower(DBM_TO_PW, ConvertPower(PW_TO_DBM, tx_power) - pl_overall_db);
}

// Room-corridor at 5.25 GHz (Xu et al., VTC 2007) — dual-slope log-distance model
static double PathLoss_RoomCorridor_5250KHz(double tx_power, double distance) {
	double pl_overall_db = (distance <= 9) ? 53.2 + 25.8*log10(distance)
	                                       : 56.4 + 29.1*log10(distance);
	return ConvertPower(DBM_TO_PW, ConvertPower(PW_TO_DBM, tx_power) - pl_overall_db);
}

// TMB model at 5 GHz (Adame et al., Wireless Days 2019)
static double PathLoss_TMB(double tx_power, double distance) {
	double pl_overall_db = 54.12 + 10 * 2.06067*log10(distance) + 5.25 * 0.1467 * distance;
	return ConvertPower(DBM_TO_PW, ConvertPower(PW_TO_DBM, tx_power) - pl_overall_db);
}

// ===========================================================================
// Path loss dispatcher
// ===========================================================================

/**
* Compute the power received at a given distance from the transmitter.
* @param "distance"           [type double]: distance in metres
* @param "tx_power"           [type double]: transmission power in pW
* @param "central_frequency"  [type double]: central frequency in Hz
* @param "path_loss_model"    [type int]:    path loss model identifier
* @return                     [type double]: received power in pW
*/
double ComputePowerReceived(double distance, double tx_power, double central_frequency,
                            int path_loss_model) {
	double wavelength = (double) SPEED_LIGHT / central_frequency;

	switch (path_loss_model) {
		case PATH_LOSS_LFS:
			return PathLoss_FreeSpace(tx_power, distance, central_frequency);
		case PATH_LOSS_OKUMURA_HATA:
			return PathLoss_OkumuraHata(tx_power, distance, wavelength);
		case PATH_LOSS_INDOOR:
			return PathLoss_Indoor(tx_power, distance);
		case PATH_LOSS_INDOOR_2:
			return PathLoss_Indoor2(tx_power, distance);
		case PATH_LOSS_SCENARIO_1_TGax:
			return PathLoss_TGax_Scenario1(tx_power, distance, central_frequency);
		case PATH_LOSS_SCENARIO_2_TGax:
			return PathLoss_TGax_Scenario2(tx_power, distance, central_frequency);
		case PATH_LOSS_SCENARIO_3_TGax:
			return PathLoss_TGax_Scenario3(tx_power, distance, central_frequency);
		case PATH_LOSS_SCENARIO_4_TGax:
			return PathLoss_TGax_Scenario4(tx_power, distance, central_frequency);
		case PATH_LOSS_SCENARIO_4a_TGax:
			return PathLoss_TGax_Scenario4a(tx_power, distance, central_frequency);
		case PATHLOSS_5GHZ_OFFICE_BUILDING:
			return PathLoss_5GHz_Office(tx_power, distance, central_frequency);
		case PATHLOSS_INROOM_LOSS_5250KHZ:
			return PathLoss_InRoom_5250KHz(tx_power, distance);
		case PATHLOSS_ROOM_CORRIDOR_5250KHZ:
			return PathLoss_RoomCorridor_5250KHz(tx_power, distance);
		case PATHLOSS_TMB:
			return PathLoss_TMB(tx_power, distance);
		default:
			printf("Path loss model not found!\n");
			return 0;
	}
}

// ===========================================================================
// Transmission power per channel
// ===========================================================================

/**
* Compute power sent per channel (halved for each additional bonded channel)
* @param "current_tx_power" [type double]: total transmission power in pW
* @param "num_channels_tx"  [type int]:   number of channels used
* @return                   [type double]: transmission power per channel in pW
*/
double ComputeTxPowerPerChannel(double current_tx_power, int num_channels_tx){
	double tx_power_per_channel (current_tx_power);
	int num_ch_ix = (int) log2(num_channels_tx);
	for (int i = 0; i < num_ch_ix; ++i) tx_power_per_channel /= 2;
	return tx_power_per_channel;
}

// ===========================================================================
// Channel occupancy (CCA)
// ===========================================================================

/**
* Indicate the channels occupied and free using CCA (with optional PIFS)
* @param "primary_channel"                   [type int]:     primary channel
* @param "pifs_activated"                    [type int]:     boolean — PIFS active
* @param "channels_free"                     [type int*]:    channel free array (updated in place)
* @param "min_channel_allowed"               [type int]:     left channel bound
* @param "max_channel_allowed"               [type int]:     right channel bound
* @param "channel_power"                     [type double**]: power per channel
* @param "pd"                                [type double]:  PD threshold in pW
* @param "timestampt_channel_becomes_free"   [type double*]: last time each channel became free
* @param "sim_time"                          [type double]:  current simulation time
* @param "pifs"                              [type double]:  PIFS value
*/
void GetChannelOccupancyByCCA(int primary_channel, int pifs_activated, int *channels_free,
		int min_channel_allowed, int max_channel_allowed, double **channel_power, double pd,
		double *timestampt_channel_becomes_free, double sim_time, double pifs){

	switch(pifs_activated){
		case TRUE:{
			for(int c = min_channel_allowed; c <= max_channel_allowed; ++c){
				if(c == primary_channel){
					channels_free[c] = ((*channel_power)[c] < pd) ? CHANNEL_FREE : CHANNEL_OCCUPIED;
				} else {
					double time_free = sim_time - timestampt_channel_becomes_free[c];
					channels_free[c] = ((*channel_power)[c] < pd && time_free > pifs)
					                   ? CHANNEL_FREE : CHANNEL_OCCUPIED;
				}
			}
			break;
		}
		case FALSE:{
			for(int c = min_channel_allowed; c <= max_channel_allowed; ++c){
				channels_free[c] = ((*channel_power)[c] < pd) ? CHANNEL_FREE : CHANNEL_OCCUPIED;
			}
			break;
		}
	}
}

// ===========================================================================
// Power tracking
// ===========================================================================

/**
* Update the power sensed from each node on the primary channel
*/
void UpdatePowerSensedPerNode(int primary_channel, std::map<int,double> &power_received_per_node,
	Notification notification, double central_frequency, int path_loss_model,
	double pw_received, int start_or_finish,
	double rx_x, double rx_y, double rx_z) {

	// Preamble puncturing: if the receiver's primary channel is punctured in this PPDU,
	// the sender contributes zero power there — treat as if out of channel range.
	int primary_in_range = (primary_channel >= notification.left_channel
	                        && primary_channel <= notification.right_channel);
	int primary_punctured = (notification.tx_info.pp_punctured_bitmap & (1 << primary_channel)) != 0;

	if(primary_in_range && !primary_punctured){
		switch(start_or_finish){
			case TX_INITIATED:{
				double effective_power = pw_received;
				if (notification.tx_info.beamforming_active
						&& notification.tx_info.beam_N_elements > 1
						&& notification.packet_type == PACKET_TYPE_DATA) {
					effective_power *= ComputeRxBeamGain(notification.tx_info,
						rx_x, rx_y, rx_z);
				}
				power_received_per_node[notification.source_id] = effective_power;
				break;
			}
			case TX_FINISHED:{
				power_received_per_node.erase(notification.source_id);
				break;
			}
			default:{
				printf("ERROR: Unknown start_or_finish instruction");
				exit(EXIT_FAILURE);
			}
		}
	} else {
		power_received_per_node.erase(notification.source_id);
	}
}

void UpdateRssiPerSta(Wlan wlan, double *rssi_per_sta,
		double *received_power_array, int total_nodes_number){
	int id(0);
	for (int i = 0; i < total_nodes_number; ++i) {
		if (wlan.FindStaInList(i)){
			rssi_per_sta[id] = received_power_array[i];
			++id;
		}
	}
}

// ===========================================================================
// Adjacent channel interference
// ===========================================================================

/**
* Apply a co-channel interference model to compute total power per channel
*/
void ApplyAdjacentChannelInterferenceModel(int adjacent_channel_model, double total_power[],
	Notification notification, double central_frequency, double pw_received, int path_loss_model){

	// Direct power on the TX channels.
	// Preamble puncturing: punctured sub-channels carry no power (bit i=1 → silent).
	for(int i = notification.left_channel; i <= notification.right_channel; ++i){
		if(notification.tx_info.pp_punctured_bitmap & (1 << i)) continue;
		total_power[i] = pw_received;
	}

	double pw_loss_db, total_power_dbm;

	switch(adjacent_channel_model){
		case ADJACENT_CHANNEL_NONE:{
			break;
		}
		case ADJACENT_CHANNEL_BOUNDARY:{
			for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
				if(c < notification.left_channel || c > notification.right_channel){
					int boundary = (c < notification.left_channel) ? notification.left_channel
					                                               : notification.right_channel;
					pw_loss_db      = 20 * abs(c - boundary);
					total_power_dbm = ConvertPower(PW_TO_DBM, pw_received) - pw_loss_db;
					total_power[c] += ConvertPower(DBM_TO_PW, total_power_dbm);
					if(total_power[c] < MIN_VALUE_C_LANGUAGE) total_power[c] = 0;
				}
			}
			break;
		}
		case ADJACENT_CHANNEL_EXTREME:{
			for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
				for(int j = notification.left_channel; j <= notification.right_channel; ++j){
					// Preamble puncturing: no leakage from punctured source sub-channels.
					if(notification.tx_info.pp_punctured_bitmap & (1 << j)) continue;
					if(c != j){
						pw_loss_db      = 20 * abs(c - j);
						total_power_dbm = ConvertPower(PW_TO_DBM, pw_received) - pw_loss_db;
						total_power[c] += ConvertPower(DBM_TO_PW, total_power_dbm);
						if(total_power[c] < MIN_DOUBLE_VALUE_KOMONDOR) total_power[c] = 0;
					}
				}
			}
			break;
		}
		default:{
			printf("ERROR: Unknown cochannel model!");
			exit(EXIT_FAILURE);
		}
	}
}

/**
* Update the aggregated power sensed per channel when a transmission starts or finishes
*/
void UpdateChannelsPower(double **channel_power, Notification notification,
    int update_type, double central_frequency, int path_loss_model,
	int adjacent_channel_model, double pw_received, int node_id){

	double total_power[NUM_CHANNELS_KOMONDOR];
	memset(total_power, 0, NUM_CHANNELS_KOMONDOR * sizeof(double));

	ApplyAdjacentChannelInterferenceModel(adjacent_channel_model, total_power,
		notification, central_frequency, pw_received, path_loss_model);

	for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
		switch(update_type){
			case TX_FINISHED:{
				(*channel_power)[c] -= total_power[c];
				if ((*channel_power)[c] < 0.000001) (*channel_power)[c] = 0;
				break;
			}
			case TX_INITIATED:{
				(*channel_power)[c] += total_power[c];
				break;
			}
		}
	}
}

// ===========================================================================
// SINR and interference
// ===========================================================================

/**
* Compute the SINR (linear, not in dB)
*/
double UpdateSINR(double pw_received_interest, double max_pw_interference){
	return pw_received_interest /
	       (max_pw_interference + ConvertPower(DBM_TO_PW, NOISE_LEVEL_DBM));
}

/**
* Find the maximum interference power across the channels of interest
*/
void ComputeMaxInterference(double *max_pw_interference, int *channel_max_interference,
	Notification notification_interest, int node_state,
	std::map<int,double> &power_received_per_node, double **channel_power) {

	*max_pw_interference = 0;

	for(int c = notification_interest.left_channel; c <= notification_interest.right_channel; ++c){
		if(node_state == STATE_RX_DATA || node_state == STATE_RX_ACK || node_state == STATE_NAV
			|| node_state == STATE_RX_RTS || node_state == STATE_RX_CTS || node_state == STATE_SENSING){

			double interference = (*channel_power)[c]
			                    - power_received_per_node[notification_interest.source_id];
			if(*max_pw_interference < interference){
				*max_pw_interference    = interference;
				*channel_max_interference = c;
			}
		}
	}
}

// ===========================================================================
// Timestamp management
// ===========================================================================

void UpdateTimestampChannelFreeAgain(double *timestampt_channel_becomes_free, double **channel_power,
		double current_pd, double sim_time) {
	for(int i = 0; i < NUM_CHANNELS_KOMONDOR; ++i){
		if((*channel_power)[i] > current_pd){
			timestampt_channel_becomes_free[i] = -1;
		} else if(timestampt_channel_becomes_free[i] == -1){
			timestampt_channel_becomes_free[i] = sim_time;
		}
	}
}

// ===========================================================================
// Logging utilities
// ===========================================================================

void PrintOrWriteChannelPower(int write_or_print, int save_node_logs, int print_node_logs,
	Logger node_logger, double **channel_power){
	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("channel_power [dBm]: ");
				for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c)
					printf("%f  ", ConvertPower(PW_TO_DBM, (*channel_power)[c]));
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c)
				if(save_node_logs) fprintf(node_logger.file, "%f  ", ConvertPower(PW_TO_DBM, (*channel_power)[c]));
			if(save_node_logs) fprintf(node_logger.file, "\n");
			break;
		}
	}
}

void PrintOrWriteChannelsFree(int write_or_print, int save_node_logs, int print_node_logs,
		Logger node_logger, int *channels_free){
	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("channels_free: ");
				for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c) printf("%d  ", channels_free[c]);
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c)
				if(save_node_logs) fprintf(node_logger.file, "%d ", channels_free[c]);
			if(save_node_logs) fprintf(node_logger.file, "\n");
			break;
		}
	}
}

void PrintOrWriteNodesTransmitting(int write_or_print, int save_node_logs, int print_node_logs,
	Logger node_logger, int total_nodes_number, int *nodes_transmitting){
	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("Nodes transmitting: ");
				for(int n = 0; n < total_nodes_number; ++n)
					if(nodes_transmitting[n] == TRUE) printf("N%d  ", n);
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int n = 0; n < total_nodes_number; ++n)
				if(save_node_logs && nodes_transmitting[n]) fprintf(node_logger.file, "N%d ", n);
			if(save_node_logs) fprintf(node_logger.file, "\n");
			break;
		}
	}
}

void PrintOrWriteChannelForTx(int write_or_print, int save_node_logs, int print_node_logs,
	Logger node_logger, int *channels_for_tx){
	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("channels_for_tx: ");
				for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c) printf("%d  ", channels_for_tx[c]);
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c)
				if(save_node_logs) fprintf(node_logger.file, "%d  ", channels_for_tx[c]);
			if(save_node_logs) fprintf(node_logger.file, "\n");
			break;
		}
	}
}

// Channel bonding and channel selection functions are in channel_bonding_methods.h.
// Included here so existing callers of power_channel_methods.h still have access.
#include "channel_bonding_methods.h"

#endif  // _POWER_METHODS_
