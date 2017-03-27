#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../ListOfDefines.h"


//int modulation_rates_aux[4][12] = {	// rows: modulation type, colums: number of channels (1, 2, 4, 8)
//	{4,16,24,33,49,65,73,81,98,108,122,135},
//	{8,33,49,65,98,130,146,163,195,217,244,271},
//	{17,68,102,136,204,272,306,340,408,453,510,567},
//	{34,136,204,272,408,544,613,681,817,907,1021,1134}
//};
//
//double getDataRate(int mcs_index, double* mcs_per_node){
//
//
//	if(node_state == STATE_TX_RTS) {
//
//		int ix_aux = current_destination_id - wlan.list_sta_id[0];
//		// TODO: by now, dumb policy: maximum modulation and channels provided by CB model
//		current_modulation = mcs_per_node[ix_aux][ix_num_channels_used];
//		current_data_rate = modulation_rates[ix_num_channels_used][current_modulation-1]
//			 * coding_rate_modulation[current_modulation-1];
//
//	} else if(node_state == STATE_TX_CTS || node_state == STATE_TX_ACK || node_state == STATE_TX_DATA){
//
//		current_data_rate = ongoing_notification.tx_info.data_rate;
//
//	}
//
//	return current_data_rate;
//
//}
///*
// * computeTxTime(): computes the transmission time (just link rate) according to the number of channels used and packet lenght
// * Input arguments:
// * - num_channels_used: number of channels (OR INDEX) used in the transmission
// * - total_bits: number of bits sent in the transmission
// */
//double computeTxTime(int ix_num_channels_used, int total_bits, double current_data_rate, int pdf_tx_time){
//
//	double tx_time;
//
//	// Decide current modulation and channels to be used during transmission
//
//
//
//	// Hardcoded for 1 channel
//	//current_data_rate = data_rate_array[0];
//
////	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Data rate (%d channels and modulation %d): %f Mbps\n",
////				SimTime(), node_id, node_state, LOG_Z01, LOG_LVL3, ix_num_channels_used, current_modulation, current_data_rate*pow(10,-6));
//
//	switch(pdf_tx_time){
//
//		case PDF_DETERMINISTIC:{
//			tx_time = total_bits/current_data_rate;	// 0.01234 s
//			break;
//		}
//
//		case PDF_EXPONENTIAL:{
//			tx_time = Exponential(total_bits/current_data_rate);
//			break;
//		}
//
//		default:{
//			printf("TX time model not found!\n");
//			break;
//		}
//	}
//
////	if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Computed TX duration: %f s\n",
////			SimTime(), node_id, node_state, LOG_Z01, LOG_LVL3, tx_time);
//
//	return tx_time;
//
//}
