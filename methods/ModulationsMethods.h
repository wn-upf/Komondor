#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../ListOfDefines.h"

/*
 * selectMCSResponse(): select the proper MCS of transmitter per number of channels
 */
void selectMCS(int *mcs_response, double pw_received_interest) {

	double pw_received_interest_dbm = convertPower(PICO_TO_DBM, pw_received_interest);

	for (int ch_num_ix = 0; ch_num_ix < 4; ch_num_ix ++){	// For 1, 2, 4 and 8 channels

		if(pw_received_interest_dbm < -82 +(ch_num_ix*3)){ mcs_response[ch_num_ix] = MODULATION_FORBIDDEN; }
		else if (pw_received_interest_dbm >= -82 + (ch_num_ix*3) && pw_received_interest_dbm < -79 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_BPSK_1_2;}
		else if (pw_received_interest_dbm >= -79 + (ch_num_ix*3) && pw_received_interest_dbm < -77 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_QPSK_1_2;}
		else if (pw_received_interest_dbm >= -77 + (ch_num_ix*3) && pw_received_interest_dbm < -74 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_QPSK_3_4;}
		else if (pw_received_interest_dbm >= -74 + (ch_num_ix*3) && pw_received_interest_dbm < -70 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_16QAM_1_2;}
		else if (pw_received_interest_dbm >= -70 + (ch_num_ix*3) && pw_received_interest_dbm < -66 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_16QAM_3_4;}
		else if (pw_received_interest_dbm >= -66 + (ch_num_ix*3) && pw_received_interest_dbm < -65 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_64QAM_2_3;}
		else if (pw_received_interest_dbm >= -65 + (ch_num_ix*3) && pw_received_interest_dbm < -64 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_64QAM_3_4;}
		else if (pw_received_interest_dbm >= -64 + (ch_num_ix*3) && pw_received_interest_dbm < -59 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_64QAM_5_6;}
		else if (pw_received_interest_dbm >= -59 + (ch_num_ix*3) && pw_received_interest_dbm < -57 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_256QAM_3_4;}
		else if (pw_received_interest_dbm >= -57 + (ch_num_ix*3) && pw_received_interest_dbm < -54 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_256QAM_5_6;}
		else if (pw_received_interest_dbm >= -54 + (ch_num_ix*3) && pw_received_interest_dbm < -52 +(ch_num_ix*3)){mcs_response[ch_num_ix] = MODULATION_1024QAM_3_4;}
		else { mcs_response[ch_num_ix] = MODULATION_1024QAM_5_6;}

	}

}

/*
 * EbToNoise():
 * "Energy per bit to noise power spectral density ratio"
 * (normalized SNR to compare the BER performance of a digital modulation scheme)
 * Arguments:
 * - SINR: SINR received (pW)
 * - bit_rate: bit rate (bps)
 * - B: channel bandwidth (Hz)
 * - M: number of alternative modulation symbols
 * Output:
 * - Eb_to_N0: Eb_to_N0 value in linear
 */
double EbToNoise(double SINR, double bit_rate, int B, int M){

	double Es_to_N0 = SINR * (bit_rate/B);

	//printf("Es_to_N0 = %f (%f) \n", Es_to_N0, convertPower(LINEAR_TO_DB, Es_to_N0));

	double Eb_to_N0 = Es_to_N0 * log2(M);

	//printf("Eb_to_N0 = %f (%f)\n", Eb_to_N0, convertPower(LINEAR_TO_DB, Eb_to_N0));

	return Eb_to_N0;
}

///*
// * applyModulationProbabilityError():
// * Arguments:
// *
// */
//int Node :: applyModulationProbabilityError(Notification notification){
//
//	//TODO: CHANGE THE WAY BER IS COMPUTED!!!
//
//	int is_packet_lost = PACKET_NOT_LOST;
//	double Eb_to_N0 = 0;
//	double bit_rate;
//	int channels_used;
//	int M;
//	int bw;
//
//	if(convertPower(LINEAR_TO_DB, current_sinr) < capture_effect) {
//
//		BER = 1; //100% lost
//
//	} else {
//
//		// To avoid errors in case modulation is not set
//		if(current_modulation > 0) {
//			channels_used = (notification.right_channel - notification.left_channel) + 1;
//			M = Mcs_array::bits_per_symbol_modulation_array[current_modulation-1];
//			bit_rate = notification.tx_info.data_rate;
//			bw = channels_used*basic_channel_bandwidth*pow(10,6);
//			Eb_to_N0 = EbToNoise(current_sinr, bit_rate, bw, M);
//
//		} else {
//			//ERROR!
//		}
//
//		//printf("Eb_to_N0 = %f W (%f dB)\n",Eb_to_N0, convertPower(W_TO_DB,Eb_to_N0));
//		switch(current_modulation) {
//			case MODULATION_NONE: {
//				break; // Ideal: no notification loss
//			}
//			case MODULATION_BPSK_1_2:
//			case MODULATION_QPSK_1_2:
//			case MODULATION_QPSK_3_4: {
//				BER = erfc(sqrt(2*Eb_to_N0));
//				break;
//			}
//			case MODULATION_16QAM_1_2:
//			case MODULATION_16QAM_3_4:
//			case MODULATION_64QAM_2_3:
//			case MODULATION_64QAM_3_4:
//			case MODULATION_64QAM_5_6:
//			case MODULATION_256QAM_3_4:
//			case MODULATION_256QAM_5_6:
//			case MODULATION_1024QAM_3_4:
//			case MODULATION_1024QAM_5_6: {
//				BER = 4/log2(M) * erfc(sqrt((3*Eb_to_N0*log2(M))/(M-1)));
//				break;
//			}
//			default: {
//				break;
//			}
//		}
//		// Compute packet loss according to the bit error probability
//		PER = 1 - pow((1 - BER), notification.packet_length);
//		is_packet_lost =  ((double) rand() / (RAND_MAX)) < PER;
//
//		if(save_node_logs && is_packet_lost) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Packet has been lost: Modulation used = %d, BER = %f, PER = %f\n",
//				SimTime(), node_id, node_state, LOG_F00, LOG_LVL5, current_modulation, BER, PER);
//
//	}
//
//	return is_packet_lost;
//
//}
