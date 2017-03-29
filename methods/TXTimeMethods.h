#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../ListOfDefines.h"

/* Exponential
 *
 */
double	Random( double v=1.0)	{ return v*drand48();}
int	Random( int v)	{ return (int)(v*drand48()); }
double	Exponential(double mean){ return -mean*log(Random());}

/*
 * computeTxTime(): computes the transmission time (just link rate) according to the number of channels used and packet lenght
 * Input arguments:
 * - num_channels_used: number of channels (OR INDEX) used in the transmission
 * - total_bits: number of bits sent in the transmission
 */
double computeTxTime(int total_bits, double data_rate, int pdf_tx_time){

//	printf(" *** computeTxTime(): total_bits = %d - data_rate = %f - pdf_tx_time = %d\n",
//			total_bits, data_rate, pdf_tx_time);

	double tx_time;

	switch(pdf_tx_time){

		case PDF_DETERMINISTIC:{

			tx_time = total_bits/data_rate;
			break;
		}

		case PDF_EXPONENTIAL:{

			tx_time = Exponential(total_bits/data_rate);
			break;
		}

		default:{

			printf("TX time model not found!\n");
			break;
		}
	}

	return tx_time;
}

/* computeNavTime: computes the NAV time for the RTS and CTS packets.
 *
 */
double computeNavTime(int node_state, double rts_duration, double cts_duration, double data_duration, double ack_duration, double sifs){

	double nav_time;

	switch(node_state){

		case STATE_TX_RTS:{

			nav_time = 3 * sifs + rts_duration + cts_duration + data_duration + ack_duration;
			break;
		}

		case STATE_TX_CTS:{

			nav_time = 2 * sifs + cts_duration + data_duration + ack_duration;
			break;
		}

		default:{

			printf("ERROR: Unreachable state\n");
			exit(EXIT_FAILURE);
		}
	}

	return nav_time;
}
