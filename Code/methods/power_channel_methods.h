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
 * power_channel_methods.h: this file contains functions related to the main Komondor's operation
 *
 * - This file contains the methods related to power channel operations
 */

#include <stddef.h>
#include <math.h>
#include <iostream>

#include "../list_of_macros.h"
#include "../structures/modulations.h"
#include "auxiliary_methods.h"

#ifndef _POWER_METHODS_
#define _POWER_METHODS_

/**
* Convert power units
* @param "conversion_type" [type int]: type of conversion
* @param "power_magnitude_in" [type double]: input value to be converted
* @return "converted_power" [type double]: converted value
*/
double ConvertPower(int conversion_type, double power_magnitude_in){

  double converted_power (0);

  switch(conversion_type){
    // pW to dBm
    case PW_TO_DBM:{
      converted_power = 10 * log10(power_magnitude_in * pow(10,-9));
      break;
    }
    // dBm to pW
    case DBM_TO_PW:{
      converted_power = pow(10,(power_magnitude_in + 90)/10);
      break;
    }
    // mW to dBm
    case MW_TO_DBM:{
      converted_power = 10 * log10(power_magnitude_in * pow(10,-6));
      break;
    }
    // dBm to mW (dB to linear)
    case DBM_TO_MW:
    case DB_TO_LINEAR:
    case DBW_TO_W: {
      converted_power = pow(10,power_magnitude_in/10);
      break;
    }
    // W to dBW
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

/**
* Compute the distance between two 3D points
* @param "x1" [type double]: x position of the first object
* @param "y1" [type double]: y position of the first object
* @param "z1" [type double]: z position of the first object
* @param "x2" [type double]: x position of the second object
* @param "y2" [type double]: y position of the second object
* @param "z2" [type double]: z position of the second object
* @return "distance" [type double]: distance in meters
*/
double ComputeDistance(double x1, double y1, double z1, double x2, double y2, double z2){
  double distance (sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2)));
  return distance;
}

/**
* Compute the power received in a given distance from the transmitter depending on the path loss model
* @param "distance" [type double]: distance in meters
* @param "tx_power" [type double]: transmission power used
* @param "central_frequency" [type double]: central frequency
* @param "path_loss_model" [type int]: path-loss model used
* @return "pw_received" [type double]: power received in pW
*/
double ComputePowerReceived(double distance, double tx_power, double central_frequency, int path_loss_model) {

//	printf("    - distance = %f\n", distance);
//	printf("    - tx_power = %f\n", ConvertPower(PW_TO_DBM,tx_power));
//	printf("    - central_frequency = %f\n", central_frequency);
//	printf("    - path_loss_model = %d\n", path_loss_model);

	double tx_power_dbm (ConvertPower(PW_TO_DBM, tx_power));
	double tx_gain (ConvertPower(DB_TO_LINEAR, ANTENNA_TX_GAIN_DB));
	double rx_gain (ConvertPower(DB_TO_LINEAR, ANTENNA_RX_GAIN_DB));
	double pw_received_dbm;
	double wavelength ((double) SPEED_LIGHT/central_frequency);
	double loss;

	double pw_received;	// Power received [pW]

	switch(path_loss_model){
		// Free space - Calculator: https://www.pasternack.com/t-calculator-fspl.aspx (UNITS ARE NOT IN SI!)
		case PATH_LOSS_LFS:{
			pw_received = tx_power * tx_gain * rx_gain * pow(((double) SPEED_LIGHT/(4*M_PI*distance*central_frequency)),2);
			break;
		}
		// Okumura-Hata model - Urban areas
		case PATH_LOSS_OKUMURA_HATA:{
			double tx_heigth (10);    // Transmitter height [m]
			double rx_heigth (10);    // Receiver height [m]
			double path_loss_A (69.55 + 26.16 * log10(3*pow(10,8)/wavelength) - 13.82 * log10(tx_heigth));
			double path_loss_B (44.9 - 6.55 * log10(tx_heigth));
			double path_loss_E (3.2 * pow(log10(11.7554 * rx_heigth),2) - 4.97);
			double path_loss (path_loss_A + path_loss_B * log10(distance/1000) - path_loss_E);
			pw_received_dbm = tx_power_dbm + ANTENNA_TX_GAIN_DB + ANTENNA_RX_GAIN_DB - path_loss;
			pw_received = ConvertPower(DBM_TO_PW, pw_received_dbm);
			break;
		}
		// Indoor model (could suite an apartments building scenario)
		case PATH_LOSS_INDOOR: {
			double path_loss_factor (5);
			double shadowing (9.5);
			double obstacles (30);
			double walls_frequency (5); //  One wall each 5 meters on average
			double shadowing_at_wlan ((((double) rand())/RAND_MAX)*shadowing);
			double obstacles_at_wlan ((((double) rand())/RAND_MAX)*obstacles);
			double alpha (4.4); // Propagation model
			double path_loss (path_loss_factor + 10*alpha*log10(distance) + shadowing_at_wlan +
			  (distance/walls_frequency)*obstacles_at_wlan);
			pw_received_dbm = tx_power_dbm + ANTENNA_TX_GAIN_DB - path_loss; // Power in dBm
			pw_received = ConvertPower(DBM_TO_PW, pw_received_dbm);
			break;
		}
		// Indoor model without variability
		case PATH_LOSS_INDOOR_2: {
			double path_loss_factor (5);
			double shadowing (9.5);
			double obstacles (30);
			double walls_frequency (5); //  One wall each 5 meters on average
			double shadowing_at_wlan (1/2*shadowing);
			double obstacles_at_wlan (1/2*obstacles);
			double alpha (4.4); // Propagation model
			double path_loss (path_loss_factor + 10*alpha*log10(distance) + shadowing_at_wlan +
			  (distance/walls_frequency)*obstacles_at_wlan);
			pw_received_dbm = tx_power_dbm + ANTENNA_TX_GAIN_DB - path_loss; // Power in dBm
			pw_received = ConvertPower(DBM_TO_PW, pw_received_dbm);
			break;
		}

		// Residential - 5 dB/wall and 18.3 dB per floor, and 4 dB shadow
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx
		// IEEE 802.11ax uses the TGn channel B path loss model for performance evaluation of simulation scenario #1
		// with extra indoor wall and floor penetration loss.
		case PATH_LOSS_SCENARIO_1_TGax: {
			int n_walls(10);   // Wall frequency (n_walls walls each m)
			int n_floors(3);   // Floor frequency (n_floors floors each m)
			int L_iw(5);     // Penetration for a single wall (dB)
			double min_d(distance);
			if (distance > 5) { min_d = 5; }
			double central_frequency_ghz(central_frequency / pow(10,9));
			double LFS (40.05 + 20*log10(central_frequency_ghz/2.4) + 20*log10(min_d) +
				  18.3*pow((distance/n_floors),(((distance/n_floors)+2)/((distance/n_floors)+1))
						  - 0.46) + L_iw*(distance/n_walls));
			double d_BP (5);    // Break-point distance (m)
			if (distance >= d_BP) {
			loss = LFS + 35*log10(distance/double(5));
			} else {
			loss = LFS;
			}
			pw_received_dbm = tx_power_dbm + ANTENNA_TX_GAIN_DB + ANTENNA_RX_GAIN_DB - loss;
			pw_received = ConvertPower(DBM_TO_PW, pw_received_dbm);
			break;
		}

		// Enterprise - 5 dB/wall and 18.3 dB per floor, and 4 dB shadow
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx
		// IEEE 802.11ax uses the TGn channel D path loss model for performance evaluation of simulation scenario #2
		// with extra indoor wall and floor penetration loss.
		case PATH_LOSS_SCENARIO_2_TGax: {
			int f_walls(12/20);   // Wall frequency (n_walls walls each m)
			double min_d(distance);
			if (distance > 10) { min_d = 1; }
			double central_frequency_ghz(central_frequency / pow(10,9));
			double shadowing (5);
			double shadowing_at_wlan ((((double) rand())/RAND_MAX)*shadowing);
			double LFS (40.05 + 20*log10(central_frequency_ghz/2.4) + 20*log10(min_d)
				+ 7*(distance*f_walls) + shadowing_at_wlan);
			int d_BP (1);    // Break-point distance (m)
			if (distance >= d_BP) {
				loss = LFS + 35*log10(distance/10);
			} else {
				loss = LFS;
			}
			pw_received_dbm = tx_power_dbm + ANTENNA_TX_GAIN_DB + ANTENNA_RX_GAIN_DB - loss;
			pw_received = ConvertPower(DBM_TO_PW, pw_received_dbm);
			//	  Shadowing: Log-normal with 5 dB standard deviation, iid across all links
			break;
	//	  PL(d) = 40.05 + 20*log10(fc/2.4) + 20*log10(min(d,10)) + (d>10) * 35*log10(d/10) + 7*W
	//	  W = number of office walls traversed in x-direction plus number of office walls traversed in y-direction
	//	  use MCS0 or MCS7 for all transmissions
	//	  APs have 4 rx and tx antennas
		}

		// Indoor small BSSs
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx
		// IEEE 802.11ax uses the TGn channel D path loss model for performance evaluation
		// of simulation scenario #3.
		case PATH_LOSS_SCENARIO_3_TGax: {
		  double LFS (32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000));
		  int d_BP (10);    // Break-point distance (m)
		  if (distance >= d_BP) {
			loss = LFS + 35*log10(distance/d_BP);
		  } else {
			loss = LFS;
		  }
		  pw_received_dbm = tx_power_dbm + ANTENNA_TX_GAIN_DB + ANTENNA_RX_GAIN_DB - loss;
		  pw_received = ConvertPower(DBM_TO_PW, pw_received_dbm);
		  break;
		}

		// Outdoor large BSS scenario
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx
		case PATH_LOSS_SCENARIO_4_TGax: {
		  double h_AP (10);    // Height of the AP in m
		  double h_STA (1.5);   // Height of the STA in m
		  double d_BP ((4 * (h_AP - 1) * (h_STA - 1) * central_frequency) / SPEED_LIGHT);
		  if (distance < d_BP && distance >= 10) {
			loss = 22 * log10(distance) + 28 + 20  *log10(central_frequency * pow(10,-9));
		  } else if (distance >= d_BP && distance < 5000) {
			loss = 40 * log10(distance) + 7.8 + 18 * log10(h_AP - 1) -
				18 * log10(h_STA - 1)  + 20 * log10(central_frequency * pow(10,-9));
		  }
		  pw_received_dbm = tx_power_dbm + ANTENNA_TX_GAIN_DB - loss; // Power in dBm
		  pw_received = ConvertPower(DBM_TO_PW, pw_received_dbm);
		  break;
		}

		// Outdoor large BSS scenario + Residential
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx
		case PATH_LOSS_SCENARIO_4a_TGax: {
		//      double h_AP = 10;    // Height of the AP in m
		//      double h_STA = 1.5;   // Height of the STA in m
		  if (distance < 2000 && distance >= 10) {
			loss = 36.7 * log10(distance) + 22.7 + 26  * log10(central_frequency * pow(10,-9));
		  }
		  // Outdoor-to-Indoor building penetration loss
		  // TODO: important to consider specifying d_outdoor and d_indoor
		  double d_outdoor (0);
		  double d_indoor (0);
		  loss = loss * (d_outdoor + d_indoor) + 20 + 0.5 * d_indoor;
		  pw_received_dbm = tx_power_dbm + ANTENNA_TX_GAIN_DB - loss; // Power in dBm
		  pw_received = ConvertPower(DBM_TO_PW, pw_received_dbm);
		  break;
		}

		/*
		 * Medbo, J., & Berg, J. E. (2000). Simple and accurate path loss modeling at 5 GHz in indoor environments
		 * with corridors. In Vehicular Technology Conference, 2000. IEEE-VTS Fall VTC 2000. 52nd (Vol. 1, pp. 30-36). IEEE.
		 */
		case PATHLOSS_5GHZ_OFFICE_BUILDING:{
			// pl_overall = pl_free_space(d) + alpha * d
			double pl_overall_db;		// Overall path loss
			double pl_free_space_db;	// Pathloss free space
			double alpha (0.44);		// Constant attenuation per unit of path length [dB/m]
			pl_free_space_db = 20 * log10(distance) + 20 * log10(central_frequency) +
					20 * log10((4*M_PI)/((double) SPEED_LIGHT)) -
					ANTENNA_RX_GAIN_DB - ANTENNA_TX_GAIN_DB;
			pl_overall_db = pl_free_space_db + alpha * distance;
			double pw_received_dbm (ConvertPower(PW_TO_DBM, tx_power) - pl_overall_db);
			pw_received = ConvertPower(DBM_TO_PW, pw_received_dbm);
			break;
		}

		/*
		 * Xu et al. Indoor Office Propagation Measurements and Path Loss Models at 5.25 GHz“, IEEE VTC 2007.
		 * one-slope log-distance model in in-room LoS condition
		 */
		case PATHLOSS_INROOM_LOSS_5250KHZ:{
			double pl_overall_db (47.8 + 14.8 * log10(distance));		// Overall path loss
			double pw_received_dbm (ConvertPower(PW_TO_DBM, tx_power) - pl_overall_db);
			pw_received = ConvertPower(DBM_TO_PW, pw_received_dbm);
			break;
		}

		/*
		 * Xu et al. Indoor Office Propagation Measurements and Path Loss Models at 5.25 GHz“, IEEE VTC 2007.
		 * dual-slope log-distance model in room-corridor condition
		 */
		case PATHLOSS_ROOM_CORRIDOR_5250KHZ:{
			double pl_overall_db;
			if(distance <=  9){
				pl_overall_db = 53.2 + 25.8 * log10(distance);		// Overall path loss
			} else {
				pl_overall_db = 56.4 + 29.1 * log10(distance);		// Overall path loss
			}
			double pw_received_dbm (ConvertPower(PW_TO_DBM, tx_power) - pl_overall_db);
			pw_received = ConvertPower(DBM_TO_PW, pw_received_dbm);
			break;
		}

		/*
		 * Adame, T., Carrascosa, M., & Bellalta, B. (2019, April). The TMB path loss model for 5 GHz indoor WiFi scenarios:
		 * On the empirical relationship between RSSI, MCS, and spatial streams. In 2019 Wireless Days (WD) (pp. 1-8). IEEE.
		 */
		case PATHLOSS_TMB:{
			double pl_overall_db;
			pl_overall_db = 54.12 + 10 * 2.06067 * log10(distance) + 5.25 * 0.1467 * distance;
			double pw_received_dbm = ConvertPower(PW_TO_DBM, tx_power) - pl_overall_db;
			pw_received = ConvertPower(DBM_TO_PW, pw_received_dbm);
			break;
		}

		default:{
		  printf("Path loss model not found!\n");
		  break;
		}

	}

	return pw_received;

}

/**
* Compute power sent per channel
* @param "current_tx_power" [type double]: transmission power used
* @param "num_channels_tx" [type int]: number of channels used for transmitting
* @return "tx_power_per_channel" [type double]: transmission power per channel
*/
double ComputeTxPowerPerChannel(double current_tx_power, int num_channels_tx){
	double tx_power_per_channel (current_tx_power);
	int num_channels_tx_ix (log2(num_channels_tx));
	for (int num_ch_ix = 0; num_ch_ix < num_channels_tx_ix; ++num_ch_ix){
		// P_tx issue #113
		tx_power_per_channel =  tx_power_per_channel/2;	// Half the power
	}
	return tx_power_per_channel;
}

/**
* Indicate the channels occupied and free in a binary way
* @param "primary_channel" [type int]: primary channel
* @param "pifs_activated" [type int]: boolean indicating whether PIFS is activated or not
* @param "channels_free" [type int*]: list of channels free (to be updated by this method)
* @param "min_channel_allowed" [type int]: left channel
* @param "max_channel_allowed" [type int]: right channel
* @param "channel_power" [type double*]: power sensed per channel
* @param "pd" [type double]: packet detect (PD) threshold in pW
* @param "timestampt_channel_becomes_free" [type double*]: time at which each channel becomes free (if not further transmissions are detected)
* @param "sim_time" [type double]: current simulation time
* @param "pifs" [type double]: PIFS value
*/
void GetChannelOccupancyByCCA(int primary_channel, int pifs_activated, int *channels_free, int min_channel_allowed,
		int max_channel_allowed, double **channel_power, double pd, double *timestampt_channel_becomes_free,
		double sim_time, double pifs){

	switch(pifs_activated){

		case TRUE:{
			double time_channel_has_been_free;	// Time channel has been free since last P(ch) > CCA
			for(int c = min_channel_allowed; c <= max_channel_allowed; ++c){
				if(c == primary_channel){
					if((*channel_power)[c] < pd) channels_free[c] = CHANNEL_FREE;
				} else {
					time_channel_has_been_free = sim_time - timestampt_channel_becomes_free[c];
					// Sergio on 19 Oct 2017:
					// - Added condidition time_channel_has_been_free < MICRO_VALUE to consider events that happen at the same time.
					// - That is, when the BO expires and other nodes start transmitting PIFS must no be considered, but collision.
					if((*channel_power)[c] < pd && time_channel_has_been_free > pifs){
					  channels_free[c] = CHANNEL_FREE;
					} else {
					  channels_free[c] = CHANNEL_OCCUPIED;

					}
				}
			}
			break;
		}

		case FALSE:{
			for(int c = min_channel_allowed; c <= max_channel_allowed; ++c){
				if((*channel_power)[c] < pd){
				  channels_free[c] = CHANNEL_FREE;
				} else {
				  channels_free[c] = CHANNEL_OCCUPIED;
				}
			}
			break;
		}
	}

}

/**
* Update the power sensed coming from each node in the primary channel (power of interest counted only if transmission implies the primary channel)
* @param "primary_channel" [type int]: primary channel
* @param "power_received_per_node" [type std::map<int,double>]: dynamic map providing the power received by each active node (to be updated by this method)
* @param "notification" [type Notification]: last detected notification
* @param "central_frequency" [type double]: central frequency
* @param "path_loss_model" [type int]: path-loss model used
* @param "pw_received" [type double]: power received in pW
* @param "start_or_finish" [type int]: indicated whether the transmission has started or finishes
*/
void UpdatePowerSensedPerNode(int primary_channel, std::map<int,double> &power_received_per_node,
	Notification notification, double central_frequency, int path_loss_model,
	double pw_received, int start_or_finish) {

	if(primary_channel >= notification.left_channel && primary_channel <= notification.right_channel){

		switch(start_or_finish){

			case TX_INITIATED:{
				power_received_per_node[notification.source_id] = pw_received;
				break;
			}

			case TX_FINISHED:{
				power_received_per_node.erase(notification.source_id);
				break;
			}

			default:{
				printf("ERROR: Unkown start_or_finish instruction");
				exit(EXIT_FAILURE);
				break;
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

/**
* Apply a co-channel interference model
* @param "adjacent_channel_model" [type int]: adjacent channel model
* @param "total_power" [type double]: total power in each channel (to be updated by this method)
* @param "notification" [type Notification]: last detected notification
* @param "central_frequency" [type double]: central frequency
* @param "pw_received" [type double]: power received in pW
* @param "path_loss_model" [type int]: path-loss model used
*/
void ApplyAdjacentChannelInterferenceModel(int adjacent_channel_model, double total_power[],
	Notification notification, double central_frequency, double pw_received, int path_loss_model){

	// Direct power (power of the channels used for transmitting)
	for(int i = notification.left_channel; i <= notification.right_channel; ++i){
		(total_power)[i] = pw_received;
	}

	double pw_loss_db;
	double total_power_dbm;

	// Co-channel interference power
	switch(adjacent_channel_model){

		case ADJACENT_CHANNEL_NONE:{
			// Do nothing
			break;
		}

		// (RECOMMENDED) Boundary co-channel interference: only boundary channels (left and right) used in the TX affect the rest of channels
		case ADJACENT_CHANNEL_BOUNDARY:{
			for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c) {

				if(c < notification.left_channel || c > notification.right_channel){

					if(c < notification.left_channel) {

						pw_loss_db = 20 * abs(c-notification.left_channel);
						total_power_dbm = ConvertPower(PW_TO_DBM, pw_received) - pw_loss_db;
						(total_power)[c] = (total_power)[c] + ConvertPower(DBM_TO_PW, total_power_dbm);

					} else if(c > notification.right_channel) {

						pw_loss_db = 20 * abs(c-notification.right_channel);
						total_power_dbm = ConvertPower(PW_TO_DBM, pw_received) - pw_loss_db;
						(total_power)[c] = (total_power)[c] + ConvertPower(DBM_TO_PW, total_power_dbm);

					}

					if((total_power)[c] < MIN_VALUE_C_LANGUAGE){

						(total_power)[c] = 0;

					}

				} else {
					// Inside TX range --> do nothing
				}
			}
			break;
		}

		case ADJACENT_CHANNEL_EXTREME:{

			for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c) {

				for(int j = notification.left_channel; j <= notification.right_channel; ++j){

					if(c != j) {

						pw_loss_db = 20 * abs(c-j);
						total_power_dbm = ConvertPower(PW_TO_DBM, pw_received) - pw_loss_db;
						(total_power)[c] = (total_power)[c] + ConvertPower(DBM_TO_PW, total_power_dbm);
						if((total_power)[c] < MIN_DOUBLE_VALUE_KOMONDOR) (total_power)[c] = 0;

					}
				}
			}
			break;
		}

		default:{
			printf("ERROR: Unkown cochannel model!");
			exit(EXIT_FAILURE);
			break;
		}
	}

}

/**
* Update the aggregated power sensed by the node in every channel
* @param "channel_power" [type double*]: array containing the power sensed per channel (to be updated by this method)
* @param "notification" [type Notification]: last detected notification
* @param "update_type" [type int]: type of update (TX_INITIATED or TX_FINISHED)
* @param "central_frequency" [type double]: central frequency
* @param "path_loss_model" [type int]: path-loss model used
* @param "adjacent_channel_model" [type int]: adjacent channel model
* @param "pw_received" [type double]: power received in pW
* @param "node_id" [type int]: identifier of the node
*/
void UpdateChannelsPower(double **channel_power, Notification notification,
    int update_type, double central_frequency, int path_loss_model,
	int adjacent_channel_model, double pw_received, int node_id){

	double total_power[NUM_CHANNELS_KOMONDOR];
	memset(total_power, 0, NUM_CHANNELS_KOMONDOR * sizeof(double));

	// Updates total_power array
	ApplyAdjacentChannelInterferenceModel(adjacent_channel_model, total_power,
		notification, central_frequency, pw_received, path_loss_model);

	// Increase/decrease power sensed if TX started/finished
	for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){

		switch(update_type){

			case TX_FINISHED:{

				(*channel_power)[c] = (*channel_power)[c] - total_power[c];

				// Avoid near-zero negative values
				if ((*channel_power)[c] < 0.000001) (*channel_power)[c] = 0;
				break;
			}

			case TX_INITIATED:{
				(*channel_power)[c] = (*channel_power)[c] + total_power[c];
				break;
			}

			default:{}
		}

	}
}

/**
* Update the SINR
* @param "pw_received_interest" [type double]: power of interest
* @param "noise_level" [type double]: noise level
* @param "max_pw_interference" [type double]: maximum interference power
* @return "sinr" [type double]: SINR in dB
*/
double UpdateSINR(double pw_received_interest, double max_pw_interference){
	double sinr (pw_received_interest / (max_pw_interference + ConvertPower(DBM_TO_PW, NOISE_LEVEL_DBM)));
	return sinr;
}

/**
* Compute the maximum interference perceived in the channels of interest
* @param "max_pw_interference" [type double*]: maximum detected interference power (to be updated by this method)
* @param "channel_max_intereference" [type int*]: number of channel with the highest interference (to be updated by this method)
* @param "notification_interest" [type Notification]: last detected notification
* @param "node_state" [type int]: state of the node
* @param "power_received_per_node" [type std::map<int,double>]: dynamic map providing the power received by each active node
* @param "channel_power" [type double*]: array with the power detected per channel
*/
void ComputeMaxInterference(double *max_pw_interference, int *channel_max_intereference,
	Notification notification_interest, int node_state, std::map<int,double> &power_received_per_node,
	double **channel_power) {

	*max_pw_interference = 0;

	for(int c = notification_interest.left_channel; c <= notification_interest.right_channel; ++c){

		if(node_state == STATE_RX_DATA || node_state == STATE_RX_ACK || node_state == STATE_NAV
			|| node_state == STATE_RX_RTS || node_state == STATE_RX_CTS || node_state == STATE_SENSING){

			if(*max_pw_interference <
					((*channel_power)[c] - power_received_per_node[notification_interest.source_id])){

				*max_pw_interference = (*channel_power)[c] - power_received_per_node[notification_interest.source_id];

				*channel_max_intereference = c;

			}
		}
	}
}


void GetTxChannelsByChannelBondingCCA11ax(int *channels_for_tx, int channel_bonding_model, int min_channel_allowed,
		int max_channel_allowed, int primary_channel, double **channel_power){


	int num_channels_allowed = max_channel_allowed - min_channel_allowed + 1;

	// Reset channels for transmitting
	for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
		channels_for_tx[c] = FALSE;
	}

	// Select channels to transmit depending on the sensed power
	switch(channel_bonding_model){

		// Only Primary Channel used if FREE
		case CB_ONLY_PRIMARY:{
			if((*channel_power)[primary_channel] < ConvertPower(DBM_TO_PW,-82)) channels_for_tx[primary_channel] = TRUE;
			break;
		}

		case CB_ALWAYS_MAX_LOG2:{

			switch(num_channels_allowed){

				// 160 MHz allocated
				case 8: {

					int num_ch_tx_possible = 8;

					// Try 160 MHz
					if(primary_channel <=3){

						for(int c = 0; c <= 3; ++c){
							if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_80MHZ)) num_ch_tx_possible = 4;
						}

						for(int c = 4; c <= 7; ++c){
							if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_80MHZ)) num_ch_tx_possible = 4;
						}

					} else {

						for(int c = 0; c <= 3; ++c){
							if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_80MHZ)) num_ch_tx_possible = 4;
						}

						for(int c = 4; c <= 7; ++c){
							if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_80MHZ)) num_ch_tx_possible = 4;
						}

					}

					if (num_ch_tx_possible == 4){
						// Try 80 MHz
						// Detect primary 40 MHz
						if(primary_channel <= 3){

							if(primary_channel <= 1){

								for(int c = 0; c <= 1; ++c){
									if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_40MHZ)) num_ch_tx_possible = 2;
								}

								for(int c = 2; c <= 3; ++c){
									if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
								}

							} else {

								for(int c = 0; c <= 1; ++c){
									if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
								}

								for(int c = 2; c <= 3; ++c){
									if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_40MHZ)) num_ch_tx_possible = 2;
								}

							}

						} else {

							if(primary_channel <= 5){

								for(int c = 4; c <= 5; ++c){
									if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_40MHZ)) num_ch_tx_possible = 2;
								}

								for(int c = 6; c <= 7; ++c){
									if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
								}

							} else {

								for(int c = 4; c <= 5; ++c){
									if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
								}

								for(int c = 6; c <= 7; ++c){
									if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_40MHZ)) num_ch_tx_possible = 2;
								}

							}
						}
					}

					if (num_ch_tx_possible == 2){

						// Try 40 MHz
						// Detect primary 20 MHz
						if(primary_channel == 0){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel+1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 1){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel-1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 2){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel+1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 3){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel-1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 4){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel+1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 5){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel-1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 6){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel+1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 7){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel-1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						}

					}

					if (num_ch_tx_possible == 1){

						channels_for_tx[primary_channel] = TRUE;

					} else if (num_ch_tx_possible == 2){

						if(primary_channel <= 1){
							channels_for_tx[0] = TRUE;
							channels_for_tx[1] = TRUE;
						} else if(primary_channel <= 3){
							channels_for_tx[2] = TRUE;
							channels_for_tx[3] = TRUE;
						} else if(primary_channel <= 5){
							channels_for_tx[4] = TRUE;
							channels_for_tx[5] = TRUE;
						} else if(primary_channel <= 7){
							channels_for_tx[6] = TRUE;
							channels_for_tx[7] = TRUE;
						}

					} else if (num_ch_tx_possible == 4){

						if(primary_channel <= 3){
							channels_for_tx[0] = TRUE;
							channels_for_tx[1] = TRUE;
							channels_for_tx[2] = TRUE;
							channels_for_tx[3] = TRUE;
						} else if(primary_channel <= 7){
							channels_for_tx[4] = TRUE;
							channels_for_tx[5] = TRUE;
							channels_for_tx[6] = TRUE;
							channels_for_tx[7] = TRUE;
						}


					} else if (num_ch_tx_possible == 8){
						for(int c = 0; c <= 7; ++c){
							channels_for_tx[c] = TRUE;
						}
					}
				}

				// 80 MHz
				case 4: {

					int num_ch_tx_possible = 4;

					// Try 80 MHz
					// Detect primary 40 MHz
					if(primary_channel <= 3){

						if(primary_channel <= 1){

							for(int c = 0; c <= 1; ++c){
								if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_40MHZ)) num_ch_tx_possible = 2;
							}

							for(int c = 2; c <= 3; ++c){
								if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
							}

						} else {

							for(int c = 0; c <= 1; ++c){
								if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
							}

							for(int c = 2; c <= 3; ++c){
								if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_40MHZ)) num_ch_tx_possible = 2;
							}

						}

					} else {

						if(primary_channel <= 5){

							for(int c = 4; c <= 5; ++c){
								if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_40MHZ)) num_ch_tx_possible = 2;
							}

							for(int c = 6; c <= 7; ++c){
								if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
							}

						} else {

							for(int c = 4; c <= 5; ++c){
								if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_40MHZ)) num_ch_tx_possible = 2;
							}

							for(int c = 6; c <= 7; ++c){
								if((*channel_power)[c] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_40MHZ)) num_ch_tx_possible = 2;
							}

						}
					}

					if (num_ch_tx_possible == 2){

						// Try 40 MHz
						// Detect primary 20 MHz
						if(primary_channel == 0){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel+1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 1){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel-1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 2){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel+1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 3){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel-1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 4){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel+1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 5){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel-1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 6){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel+1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						} else if(primary_channel == 7){
							if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
							if((*channel_power)[primary_channel-1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
						}

					}

					if (num_ch_tx_possible == 1){

						channels_for_tx[primary_channel] = TRUE;

					} else if (num_ch_tx_possible == 2){

						if(primary_channel <= 1){
							channels_for_tx[0] = TRUE;
							channels_for_tx[1] = TRUE;
						} else if(primary_channel <= 3){
							channels_for_tx[2] = TRUE;
							channels_for_tx[3] = TRUE;
						} else if(primary_channel <= 5){
							channels_for_tx[4] = TRUE;
							channels_for_tx[5] = TRUE;
						} else if(primary_channel <= 7){
							channels_for_tx[6] = TRUE;
							channels_for_tx[7] = TRUE;
						}

					} else if (num_ch_tx_possible == 4){

						if(primary_channel <= 3){
							channels_for_tx[0] = TRUE;
							channels_for_tx[1] = TRUE;
							channels_for_tx[2] = TRUE;
							channels_for_tx[3] = TRUE;
						} else if(primary_channel <= 7){
							channels_for_tx[4] = TRUE;
							channels_for_tx[5] = TRUE;
							channels_for_tx[6] = TRUE;
							channels_for_tx[7] = TRUE;
						}


					} else if (num_ch_tx_possible == 8){
						for(int c = 0; c <= 7; ++c){
							channels_for_tx[c] = TRUE;
						}
					}

					break;
				}

				// 40 MHz
				case 2: {

					int num_ch_tx_possible = 2;

					// Try 40 MHz
					// Detect primary 20 MHz
					if(primary_channel == 0){
						if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
						if((*channel_power)[primary_channel+1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
					} else if(primary_channel == 1){
						if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
						if((*channel_power)[primary_channel-1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
					} else if(primary_channel == 2){
						if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
						if((*channel_power)[primary_channel+1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
					} else if(primary_channel == 3){
						if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
						if((*channel_power)[primary_channel-1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
					} else if(primary_channel == 4){
						if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
						if((*channel_power)[primary_channel+1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
					} else if(primary_channel == 5){
						if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
						if((*channel_power)[primary_channel-1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
					} else if(primary_channel == 6){
						if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
						if((*channel_power)[primary_channel+1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
					} else if(primary_channel == 7){
						if((*channel_power)[primary_channel] > ConvertPower(DBM_TO_PW,CCA_PRIMARY_20MHZ)) num_ch_tx_possible = 1;
						if((*channel_power)[primary_channel-1] > ConvertPower(DBM_TO_PW,CCA_SECONDARY_20MHZ)) num_ch_tx_possible = 1;
					}

					if (num_ch_tx_possible == 1){

						channels_for_tx[primary_channel] = TRUE;

					} else if (num_ch_tx_possible == 2){

						if(primary_channel <= 1){
							channels_for_tx[0] = TRUE;
							channels_for_tx[1] = TRUE;
						} else if(primary_channel <= 3){
							channels_for_tx[2] = TRUE;
							channels_for_tx[3] = TRUE;
						} else if(primary_channel <= 5){
							channels_for_tx[4] = TRUE;
							channels_for_tx[5] = TRUE;
						} else if(primary_channel <= 7){
							channels_for_tx[6] = TRUE;
							channels_for_tx[7] = TRUE;
						}

					} else if (num_ch_tx_possible == 4){

						if(primary_channel <= 3){
							channels_for_tx[0] = TRUE;
							channels_for_tx[1] = TRUE;
							channels_for_tx[2] = TRUE;
							channels_for_tx[3] = TRUE;
						} else if(primary_channel <= 7){
							channels_for_tx[4] = TRUE;
							channels_for_tx[5] = TRUE;
							channels_for_tx[6] = TRUE;
							channels_for_tx[7] = TRUE;
						}


					} else if (num_ch_tx_possible == 8){
						for(int c = 0; c <= 7; ++c){
							channels_for_tx[c] = TRUE;
						}
					}


					break;
				}

				// 20 MHz
				case 1: {

					// Direct assignment to TRUE since the BO has already finished
					channels_for_tx[primary_channel] = TRUE;

					break;
				}

				break;
			}

		}
	}
}

/**
* Identify the channels to TX in depending on the channel_bonding scheme and channel_power state.
* @param "channels_for_tx" [type int*]: list of channels for transmitting (to be updated by this method)
* @param "channel_bonding_model" [type int]: channel bonding model
* @param "channels_free" [type int*]: list of channels free
* @param "min_channel_allowed" [type int]: left channel
* @param "max_channel_allowed" [type int*]: right channel
* @param "primary_channel" [type int]: primary channel
* @param "mcs_per_node" [type int**]: matrix containing the MCS to be used for each node and number of channels
* @param "ix_mcs_per_node" [type int]: index of the MCS used per node
* @param "num_channels_system" [type int]: total number of channels in the system
*/
void GetTxChannelsByChannelBondingCCASame(int *channels_for_tx, int channel_bonding_model, int *channels_free,
    int min_channel_allowed, int max_channel_allowed, int primary_channel, int num_channels_system){

	// Reset channels for transmitting
	for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
		channels_for_tx[c] = FALSE;
	}

	// Get left and right channels available (or free)
	int left_free_ch (0);
	int left_free_ch_is_set (0);	// True if left channel could be set true
	int right_free_ch (0);

	for(int c = min_channel_allowed; c <= max_channel_allowed; ++c){
		if(channels_free[c]){
			if(!left_free_ch_is_set){
				left_free_ch = c;
				left_free_ch_is_set = TRUE;
			}
			if(right_free_ch < c){
				right_free_ch = c;
			}
		}
	}

	int num_available_ch (max_channel_allowed - min_channel_allowed + 1);
	int log2_modulus;	// Auxiliary variable representing a modulus
	int left_tx_ch;		// Left channel to TX
	int right_tx_ch; 	// Right channel to TX

	// SERGIO 18/09/2017:
	// - Modify CB policies. Identify first of all the log2 channel ranges available
	int all_channels_free_in_range ( TRUE );	// auxiliar variable for identifying free channel ranges

	// Boolean array indicating if possible or not to transmit in 1, 2, 4 or 8 channels.
	int possible_channel_ranges_ixs[4] = {FALSE, FALSE, FALSE, FALSE};

	// Check primary
	if(channels_free[primary_channel]) possible_channel_ranges_ixs[0] = TRUE;

	// Check primary and 1 secondary
	if(NUM_CHANNELS_KOMONDOR > 1){
		if(primary_channel % 2 == 1){	// If primary is odd
			if(channels_free[primary_channel - 1]) possible_channel_ranges_ixs[1] = TRUE;
		} else{
			if(channels_free[primary_channel + 1]) possible_channel_ranges_ixs[1] = TRUE;
		}
	}

	// Check primary and 3 secondaries
	if(NUM_CHANNELS_KOMONDOR > 3){
		if(primary_channel > 3){	// primary in channel range 4-7
			for(int c = 0; c < 4; ++c){
				if(!channels_free[4 + c]) all_channels_free_in_range = FALSE;
			}
			if(all_channels_free_in_range) possible_channel_ranges_ixs[2] = TRUE;

		} else { // primary in channel range 0-3
			for(int c = 0; c < 4; ++c){
				if(!channels_free[c]) all_channels_free_in_range = FALSE;
			}
			if(all_channels_free_in_range) possible_channel_ranges_ixs[2] = TRUE;
		}
	}


	// Check primary and 7 secondaries (full system range)
	if(NUM_CHANNELS_KOMONDOR > 7){
		for(int c = 0; c < 8; ++c){
			if(!channels_free[c]) all_channels_free_in_range = FALSE;
		}
		if(all_channels_free_in_range) possible_channel_ranges_ixs[3] = TRUE;
	}

	if(left_free_ch_is_set){

		// Select channels to transmit depending on the sensed power
		switch(channel_bonding_model){

			// Only Primary Channel used if FREE
			case CB_ONLY_PRIMARY:{

				if(primary_channel >= left_free_ch && primary_channel <= right_free_ch){
					channels_for_tx[primary_channel] = TRUE;
				}
				break;
			}

			// SCB: if all channels are FREE, transmit. If not, generate a new backoff.
			case CB_SCB:{

				int tx_possible = TRUE;
				// If all channels are FREE, transmit. If not, generate a new backoff.
				for(int c = min_channel_allowed; c <= max_channel_allowed; ++c){
					if(!channels_free[c]){
						tx_possible = FALSE;
					}
				}

				if(tx_possible){
					left_tx_ch = left_free_ch;
					right_tx_ch = right_free_ch;
					for(int c = min_channel_allowed; c <= max_channel_allowed; ++c){
						channels_for_tx[c] = TRUE;
					}
				} else {
					// TX not possible (code it with negative value)
					channels_for_tx[0] = TX_NOT_POSSIBLE;
				}
				break;
			}

			// SCB log2:  if all channels accepted by the log2 mapping are FREE, transmit. If not, generate a new backoff.
			case CB_SCB_LOG2:{

				while(1){
					// II. If num_free_ch is power of 2
					if(fmod(log10(num_available_ch)/log10(2), 1) == 0){
						log2_modulus = primary_channel % num_available_ch;
						left_tx_ch = primary_channel - log2_modulus;
						right_tx_ch = primary_channel + num_available_ch - log2_modulus - 1;
						// Check if tx channels are inside the free ones
						if((left_tx_ch >= min_channel_allowed) && (right_tx_ch <= max_channel_allowed)){
							// TX channels found!
							break;

						} else {
							--num_available_ch;
						}

					} else{
						--num_available_ch;
					}
				}

				// If all channels accepted by the log2 mapping, transmit. If not, generate a new backoff.
				int tx_possible = TRUE;
				for(int c = left_tx_ch; c <= right_tx_ch; ++c){
					if(!channels_free[c]){
						tx_possible = FALSE;
					}
				}
				if(tx_possible){
					for(int c = left_tx_ch; c <= right_tx_ch; ++c){
						channels_for_tx[c] = TRUE;
					}
				} else {
					// TX not possible (code it with negative value)
					channels_for_tx[0] = TX_NOT_POSSIBLE;
				}
				break;
			}

			// Always-max (DCB): TX in all the free channels contiguous to the primary channel
			// TODO: (skectch) check if it is valid!
			case CB_ALWAYS_MAX:{
				for(int c = left_free_ch; c <= right_free_ch; ++c){
					channels_for_tx[c] = TRUE;
				}
				break;
			}

			// Always-map log2: TX in the larger channel range allowed by the log2 mapping
			// TODO: (skectch) check if it is valid!
			case CB_ALWAYS_MAX_LOG2:{

				int ch_range_ix (GetNumberOfSpecificElementInArray(1, possible_channel_ranges_ixs, 4));

				switch(ch_range_ix){

					case 1:{
						channels_for_tx[primary_channel] = TRUE;
						break;
					}

					case 2:{
						channels_for_tx[primary_channel] = TRUE;
						if(primary_channel % 2 == 1){	// If primary is odd
							channels_for_tx[primary_channel - 1] = TRUE;
						} else{
							channels_for_tx[primary_channel + 1] = TRUE;
						}
						break;
					}

					case 3:{
						// Check primary and 3 secondaries
						if(primary_channel > 3){	// primary in channel range 4-7
							channels_for_tx[4] = TRUE;
							channels_for_tx[5] = TRUE;
							channels_for_tx[6] = TRUE;
							channels_for_tx[7] = TRUE;
						} else { // primary in channel range 0-3
							channels_for_tx[0] = TRUE;
							channels_for_tx[1] = TRUE;
							channels_for_tx[2] = TRUE;
							channels_for_tx[3] = TRUE;
						}
						break;
					}

					case 4:{
						for(int c = 0; c < 8; ++c){
							channels_for_tx[c] = TRUE;
						}
						break;
					}

					default:{
						break;
					}

				}


				break;

			}

			// Always-map (DCB) log2 with optimal MCS: picks the channel range + MCS providing max throughput
			case CB_ALWAYS_MAX_LOG2_MCS:{

				// Deprecated
				printf("Deprecated CB model. Please, use another one.\n");
				exit(-1);

				break;
				}

			// Log2 probabilistic uniform: pick with same probabilty any available channel range
			case CB_PROB_UNIFORM_LOG2:{

				int ch_range_ix = GetNumberOfSpecificElementInArray(1, possible_channel_ranges_ixs, 4);

				int random_value = 1 + rand() % (ch_range_ix);	// 1 to ch_range_ix

				switch(ch_range_ix){

					case 1:{
						channels_for_tx[primary_channel] = TRUE;
						break;
					}

					case 2:{

						channels_for_tx[primary_channel] = TRUE;

						if(random_value > 1){
							if(primary_channel % 2 == 1){	// If primary is odd
								channels_for_tx[primary_channel - 1] = TRUE;
							} else{
								channels_for_tx[primary_channel + 1] = TRUE;
							}
						}
						break;
					}

					case 3:{

						channels_for_tx[primary_channel] = TRUE;

						if(random_value == 2){
							if(primary_channel % 2 == 1){	// If primary is odd
								channels_for_tx[primary_channel - 1] = TRUE;
							} else{
								channels_for_tx[primary_channel + 1] = TRUE;
							}
						} else if( random_value == 3){
							// Check primary and 3 secondaries
							if(primary_channel > 3){	// primary in channel range 4-7

								channels_for_tx[4] = TRUE;
								channels_for_tx[5] = TRUE;
								channels_for_tx[6] = TRUE;
								channels_for_tx[7] = TRUE;

							} else { // primary in channel range 0-3

								channels_for_tx[0] = TRUE;
								channels_for_tx[1] = TRUE;
								channels_for_tx[2] = TRUE;
								channels_for_tx[3] = TRUE;
							}
						}

						break;
					}

					case 4:{

						channels_for_tx[primary_channel] = TRUE;

						if(random_value == 2){
							if(primary_channel % 2 == 1){	// If primary is odd
								channels_for_tx[primary_channel - 1] = TRUE;
							} else{
								channels_for_tx[primary_channel + 1] = TRUE;
							}
						} else if( random_value == 3){
							// Check primary and 3 secondaries
							if(primary_channel > 3){	// primary in channel range 4-7

								channels_for_tx[4] = TRUE;
								channels_for_tx[5] = TRUE;
								channels_for_tx[6] = TRUE;
								channels_for_tx[7] = TRUE;

							} else { // primary in channel range 0-3

								channels_for_tx[0] = TRUE;
								channels_for_tx[1] = TRUE;
								channels_for_tx[2] = TRUE;
								channels_for_tx[3] = TRUE;
							}
						} else if(random_value == 4){
							for(int c = 0; c < 8; ++c ){
								channels_for_tx[c] = TRUE;
							}
						}
						break;
					}

					default:{
						break;
					}

				}


				break;


				break;
			}

			default:{
				printf("channel_bonding_model %d is NOT VALID!\n", channel_bonding_model);
				exit(EXIT_FAILURE);
				break;
			}
		}
	} else {  // No channel is free

	channels_for_tx[0] = TX_NOT_POSSIBLE;

	}

}

/*
 * UpdateTimestampChannelFreeAgain: updates the timestamp at which channels became free again
 **/
void UpdateTimestampChannelFreeAgain(double *timestampt_channel_becomes_free, double **channel_power,
		double current_pd, double sim_time) {

	for(int i = 0; i < NUM_CHANNELS_KOMONDOR; ++i){

		if((*channel_power)[i] > current_pd) {

			timestampt_channel_becomes_free[i] = -1;

		} else if(timestampt_channel_becomes_free[i] == -1){

			timestampt_channel_becomes_free[i] = sim_time;

		}
	}
}

/**
* Print or write the channel_power array representing the power sensed by the node in each subchannel
* @param "write_or_print" [type int]: indicates whether to print or write
* @param "save_node_logs" [type int]: flag indicating whether to save node logs or not
* @param "node_logger" [type Logger]: logger object to print logs into a file
* @param "print_node_logs" [type int]:  flag indicating whether to print node logs or not
* @param "channel_power" [type double*]: power sensed per channel
*/
void PrintOrWriteChannelPower(int write_or_print, int save_node_logs, Logger node_logger,
	int print_node_logs, double **channel_power){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("channel_power [dBm]: ");
				for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
					printf("%f  ", ConvertPower(PW_TO_DBM, (*channel_power)[c]));
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
				if(save_node_logs) fprintf(node_logger.file, "%f  ", ConvertPower(PW_TO_DBM, (*channel_power)[c]));
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

/**
* Print or write the channels_free array representing the channels that are free
* @param "write_or_print" [type int]: indicates whether to print or write
* @param "save_node_logs" [type int]: flag indicating whether to save node logs or not
* @param "print_node_logs" [type int]:  flag indicating whether to print node logs or not
* @param "node_logger" [type Logger]: logger object to print logs into a file
* @param "channels_free" [type int*]: list of channels free
*/
void PrintOrWriteChannelsFree(int write_or_print,
		int save_node_logs, int print_node_logs, Logger node_logger, int *channels_free){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("channels_free: ");
				for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
					printf("%d  ", channels_free[c]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
				 if(save_node_logs) fprintf(node_logger.file, "%d ", channels_free[c]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

/**
* Print or write the array representing the transmitting nodes.
* @param "write_or_print" [type int]: indicates whether to print or write
* @param "save_node_logs" [type int]: flag indicating whether to save node logs or not
* @param "print_node_logs" [type int]:  flag indicating whether to print node logs or not
* @param "node_logger" [type Logger]: logger object to print logs into a file
* @param "total_nodes_number" [type int]: total number of nodes
* @param "nodes_transmitting" [type int*]: list of nodes transmitting
*/
void PrintOrWriteNodesTransmitting(int write_or_print, int save_node_logs, int print_node_logs,
	Logger node_logger, int total_nodes_number,	int *nodes_transmitting){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("Nodes transmitting: ");
				for(int n = 0; n < total_nodes_number; ++n){
					if(nodes_transmitting[n] == TRUE) printf("%d  ", nodes_transmitting[n]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int n = 0; n < total_nodes_number; ++n){
				 if(save_node_logs){
					 if(nodes_transmitting[n])  fprintf(node_logger.file, "N%d ", n);
				 }
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}


/**
* Get minimum and maximum allowed channels given primary and max bandwidth
* @param "primary_channel" [type int]: primary channel
* @param "max_bandwidth" [type int]: max allocated bandwidth [no. of channels]
* @param "min_ch" [type int]:  min channel allocated
* @param "max_ch" [type Logger]: max channel allocated
*/

void GetMinAndMaxAllowedChannels(int& min_ch, int& max_ch, int primary_channel, int max_bandwidth){

	switch(max_bandwidth){

		// 20 MHz
		case 1:{
			min_ch = primary_channel;
			max_ch = primary_channel;
			break;
		}

		//40 MHz
		case 2:{

			if(primary_channel == 0 || primary_channel == 1){
				min_ch = 0;
				max_ch = 1;
			} else if(primary_channel == 2 || primary_channel == 3){
				min_ch = 2;
				max_ch = 3;
			} else if(primary_channel == 4 || primary_channel == 5){
				min_ch = 4;
				max_ch = 5;
			} else if(primary_channel == 6 || primary_channel == 7){
				min_ch = 6;
				max_ch = 7;
			}

			break;
		}

		// 80 MHz
		case 4:{
			if(primary_channel <= 3){
				min_ch = 0;
				max_ch = 3;
			} else {
				min_ch = 4;
				max_ch = 7;
			}
			break;
		}

		// 160 MHz
		case 8:{
			min_ch = 0;
			max_ch = 7;
			break;
		}

	} // End of switch(max_bandwidth)

}

/**
* Get channels where the TX is performed
* @param "channels_for_tx" [type int*]: boolean array indicating whether a channel is used for TX (1) or not (0)
* @param "channel_bonding_model" [type int]: channel bonding model (e.g., 4 DCB)
* @param "channels_free" [type int*]:  array indicating whether a channel is free (1) or not (0)
* @param "min_channel_allowed" [type int]: minimum allocated channel (most to the left)
* @param "max_channel_allowed" [type int]: maximum allocated channel (most to the right)
* @param "primary_channel" [type int]: primary channel
* @param "num_channels_komondor" [type int]: number of channels in the system
* @param "channel_power" [type double**]: array indicating the power perceived per channel
* @param "channel_aggregation_cca_model" [type int]: indicates if CCA is considered to be different per bandwidth
*/

void GetTxChannels(int *channels_for_tx, int channel_bonding_model, int *channels_free,
    int min_channel_allowed, int max_channel_allowed, int primary_channel, int num_channels_komondor,
	double **channel_power, int channel_aggregation_cca_model){

	switch(channel_aggregation_cca_model){

		case CHANNEL_AGGREGATION_CCA_SAME:{
			GetTxChannelsByChannelBondingCCASame(channels_for_tx, channel_bonding_model, channels_free,
					min_channel_allowed, max_channel_allowed, primary_channel, num_channels_komondor);
			break;
		}

		case CHANNEL_AGGREGATION_CCA_11AX:{
			GetTxChannelsByChannelBondingCCA11ax(channels_for_tx, channel_bonding_model, min_channel_allowed,
						max_channel_allowed, primary_channel, channel_power);
			break;
		}

	}

}

/**
* Print or write the channels_for_tx array representing the channels used for TX
* @param "write_or_print" [type int]: indicates whether to print or write
* @param "save_node_logs" [type int]: flag indicating whether to save node logs or not
* @param "print_node_logs" [type int]:  flag indicating whether to print node logs or not
* @param "node_logger" [type Logger]: logger object to print logs into a file
* @param "channels_for_tx" [type int*]: list of channels for transmitting
*/
void PrintOrWriteChannelForTx(int write_or_print, int save_node_logs, int print_node_logs,
	Logger node_logger, int *channels_for_tx){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("channels_for_tx: ");
				for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
					printf("%d  ", channels_for_tx[c]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < NUM_CHANNELS_KOMONDOR; ++c){
				 if(save_node_logs)  fprintf(node_logger.file, "%d  ", channels_for_tx[c]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

#endif
