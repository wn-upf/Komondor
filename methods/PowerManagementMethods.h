#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../ListOfDefines.h"

/*
 * convertPower(): convert power units
 * Input arguments:
 * - conversion_type: unit conversion type
 * - power: power value
 * Output:
 * - converted_power: power converted to required unit
 */
double convertPower(int conversion_type, double power){
	double converted_power;
	switch(conversion_type){
		// pW to dBm
		case PICO_TO_DBM:{
			converted_power = 10 * log10(power * pow(10,-9));
			break;
		}
		// dBm to pW
		case DBM_TO_PICO:{
			converted_power = pow(10,(power + 90)/10);
			break;
		}
		// mW to dBm
		case MILLI_TO_DBM:{
			converted_power = 10 * log10(power * pow(10,-6));
			break;
		}
		// dBm to mW (dB to linear)
		case DBM_TO_MILLI:
		case DB_TO_LINEAR:
		case DB_TO_W: {
			converted_power = pow(10,power/10);
			break;
		}
		// W to dB
		case W_TO_DB:
		case LINEAR_TO_DB: {
			converted_power = 10 * log10(power);
			break;
		}
		default:{
			printf("Power conversion type not found!\n");
			break;
		}
	}
	return converted_power;
}

/*
 * computeDistance(): returns the distance between 2 points
 * Input arguments:
 * - x1,y1,z1: first point position
 * - x2,y2,z2: second point position
 * Output:
 * - distance: distance [m]
 */
double computeDistance(int x1, int y1, int z1, int x2, int y2, int z2){
	double distance = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
	return distance;
}

/*
 * computePowerReceived() returns the power received in a given distance from the transmitter depending on the path loss model
 * Input arguments:
 * - distance: distance between transceiver and received [m]
 * - tx_power: TX power [dBm]
 * - tx_gain: transmitter gain [dB]
 * - rx_gain: receiver gain [dB]
 * Output arguments:
 * - Power received [dBm]
 */
double computePowerReceived(double distance, double tx_power, int tx_gain, int rx_gain, double central_frequency,
		int path_loss_model) {

	double pw_received = 0;
	double wavelength = 3*pow(10,8)/(central_frequency*pow(10,9));
	double loss;

	switch(path_loss_model){
		// Free space - Calculator: https://www.pasternack.com/t-calculator-fspl.aspx
		case PATH_LOSS_LFS:{
			loss = 32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000);
			pw_received = tx_power + tx_gain + rx_gain - loss;
			break;
		}
		// Okumura-Hata model - Urban areas
		case PATH_LOSS_OKUMURA_HATA:{
			double tx_heigth = 10;		// Transmitter height [m]
			double rx_heigth = 10;		// Receiver height [m]
			double path_loss_A = 69.55 + 26.16 * log10(3*pow(10,8)/wavelength) - 13.82 * log10(tx_heigth);
			double path_loss_B = 44.9 - 6.55 * log10(tx_heigth);
			double path_loss_E = 3.2 * pow(log10(11.7554 * rx_heigth),2) - 4.97;
			double path_loss = path_loss_A + path_loss_B * log10(distance/1000) - path_loss_E;
			pw_received = tx_power + tx_gain + rx_gain - path_loss;
			break;
		}
		// Indoor model (could suite an apartments building scenario)
		case PATH_LOSS_INDOOR: {
			double path_loss_factor = 5;
			double shadowing = 9.5;
			double obstacles = 30;
			double walls_frequency = 5; //  One wall each 5 meters on average
			double shadowing_at_wlan = (((double) rand())/RAND_MAX)*shadowing;
			double obstacles_at_wlan = (((double) rand())/RAND_MAX)*obstacles;
			double alpha = 4.4; // Propagation model
			double path_loss = path_loss_factor + 10*alpha*log10(distance) + shadowing_at_wlan +
					(distance/walls_frequency)*obstacles_at_wlan;
			pw_received = tx_power + tx_gain - path_loss; // Power in dBm
			break;
		}
		// Indoor model without variability
		case PATH_LOSS_INDOOR_2: {
			double path_loss_factor = 5;
			double shadowing = 9.5;
			double obstacles = 30;
			double walls_frequency = 5; //  One wall each 5 meters on average
			double shadowing_at_wlan = 1/2*shadowing;
			double obstacles_at_wlan = 1/2*obstacles;
			double alpha = 4.4; // Propagation model
			double path_loss = path_loss_factor + 10*alpha*log10(distance) + shadowing_at_wlan +
					(distance/walls_frequency)*obstacles_at_wlan;
			pw_received = tx_power + tx_gain - path_loss; // Power in dBm
			break;
		}

		// Residential - 5 dB/wall and 18.3 dB per floor, and 4 dB shadow
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx

		// IEEE 802.11ax uses the TGn channel B path loss model for performance evaluation of simulation scenario #1
		// with extra indoor wall and floor penetration loss.
		case PATH_LOSS_SCENARIO_1_TGax: {

			int n_walls = 10; 	// Wall frequency (n_walls walls each m)
			int n_floors = 3; 	// Floor frequency (n_floors floors each m)
			int L_iw = 5; 		// Penetration for a single wall (dB)

			double LFS = 32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000);

			int d_BP = 5;		// Break-point distance (m)

			if (distance >= d_BP) {
				loss = LFS + 35*log10(distance/d_BP) + 18.3*pow(n_floors,((n_floors+2)/(n_floors+1)) - 0.46) + L_iw*n_walls;
			} else {
				loss = LFS;
			}

			pw_received = tx_power + tx_gain + rx_gain - loss;
			break;

		}

		// Enterprise - 5 dB/wall and 18.3 dB per floor, and 4 dB shadow
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx

		// IEEE 802.11ax uses the TGn channel D path loss model for performance evaluation of simulation scenario #2
		// with extra indoor wall and floor penetration loss.
		case PATH_LOSS_SCENARIO_2_TGax: {

			int n_walls = 2; 	// Wall frequency (n_walls walls each m)
			int n_floors = 3; 		// Floor frequency (n_floors floors each m)
			int L_iw = 7; 	// Penetration for a single wall (dB)

			double LFS = 32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000);

			int d_BP = 10;		// Break-point distance (m)

			if (distance >= d_BP) {
				loss = LFS + 35*log10(distance/d_BP) + 18.3*pow(n_floors,((n_floors+2)/(n_floors+1)) - 0.46) + L_iw*n_walls;
			} else {
				loss = LFS;
			}

			pw_received = tx_power + tx_gain + rx_gain - loss;
			break;

		}

		// Indoor small BSSs
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx

		// IEEE 802.11ax uses the TGn channel D path loss model for performance evaluation
		// of simulation scenario #3.
		case PATH_LOSS_SCENARIO_3_TGax: {

			double LFS = 32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000);

			int d_BP = 10;		// Break-point distance (m)

			if (distance >= d_BP) {
				loss = LFS + 35*log10(distance/d_BP);
			} else {
				loss = LFS;
			}

			pw_received = tx_power + tx_gain + rx_gain - loss;
			break;
		}

		// Outdoor large BSS scenario
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx
		case PATH_LOSS_SCENARIO_4_TGax: {

			double h_AP = 10;		// Height of the AP in m
			double h_STA = 1.5; 	// Height of the STA in m

			double d_BP = (4 * (h_AP - 1) * (h_STA - 1) * central_frequency * pow(10,9)) / SPEED_LIGHT;


			if (distance < d_BP && distance >= 10) {
				loss = 22 * log10(distance) + 28 + 20  *log10(central_frequency);
			} else if (distance >= d_BP && distance < 5000) {
				loss = 40 * log10(distance) + 7.8 + 18 * log10(h_AP - 1) -
						18 * log10(h_STA - 1)	+ 20 * log10(central_frequency);
			}

			pw_received = tx_power + tx_gain - loss; // Power in dBm
			break;
		}

		// Outdoor large BSS scenario + Residential
		// Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx
		case PATH_LOSS_SCENARIO_4a_TGax: {

//			double h_AP = 10;		// Height of the AP in m
//			double h_STA = 1.5; 	// Height of the STA in m

			if (distance < 2000 && distance >= 10) {
				loss = 36.7 * log10(distance) + 22.7 + 26  * log10(central_frequency);
			}

			// Outdoor-to-Indoor building penetration loss
			// TODO: important to consider specifying d_outdoor and d_indoor
			double d_outdoor = 0;
			double d_indoor = 0;
			loss = loss * (d_outdoor + d_indoor) + 20 + 0.5 * d_indoor;

			pw_received = tx_power + tx_gain - loss; // Power in dBm
			break;
		}

		default:{
			printf("Path loss model not found!\n");
			break;
		}

	}
	return pw_received;
}

