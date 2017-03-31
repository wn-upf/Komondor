#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../ListOfDefines.h"

double	Random2( double v=1.0)	{ return v*drand48();}
int		Random2( int v)		{ return (int)(v*drand48()); }
double	Exponential2(double mean)	{ return -mean*log(Random2());}

/*
 * computeBackoff(): computes a new backoff according to its pdf and packet generation rate.
 * Output:
 * - backoff_time: generated backoff
 * */
double computeBackoff(int pdf_backoff, int current_CW, int backoff_type){

	double backoff_time;
	double EB = (double) (current_CW-1)/2;
	double lambda_bo =  1/(EB * SLOT_TIME);

	switch(pdf_backoff){

		case PDF_DETERMINISTIC:{

			if(backoff_type == BACKOFF_SLOTTED) {

				backoff_time = (rand() % current_CW + 1) * SLOT_TIME;

			} else if(backoff_type == BACKOFF_CONTINUOUS) {

				backoff_time = 1/lambda_bo;

			}

			break;
		}

		case PDF_EXPONENTIAL:{

			if(backoff_type == BACKOFF_SLOTTED) {

				backoff_time = round(Exponential2(EB)) * SLOT_TIME;

			} else if(backoff_type == BACKOFF_CONTINUOUS) {

				backoff_time = Exponential2(1/lambda_bo);

			}
			break;
		}

		default:{
			printf("Backoff model not found!\n");
			break;
		}
	}

	return backoff_time;

}

/*
 * computeRemainingBackoff():
 * */
double computeRemainingBackoff(int backoff_type, double remaining_backoff, double SimTime){

	double updatedBO;

	switch(backoff_type){

		case BACKOFF_SLOTTED: {
			updatedBO = round((remaining_backoff - SimTime)/SLOT_TIME) * SLOT_TIME;
			break;
		}

		case BACKOFF_CONTINUOUS: {
			updatedBO = remaining_backoff - SimTime;
			break;
		}

		default:{
			printf("Backoff type not found!\n");
			exit(EXIT_FAILURE);
			break;
		}

	}

	return updatedBO;

}



/*
 * handleBackoff(): handles the backoff. It is called when backoff may be paused or resumed.
 * Arguments:
 * - pause_or_resume: flag for identifying if function must try to pause or resume the backoff
 * - notification: notification info
 * Output:
 * - OK (1) or NOK (0) - Can pause/resume backoff
 * */
int handleBackoff(int pause_or_resume, double SimTime, int save_node_logs, Logger node_logger,
					int node_id, int node_state, double *channel_power,
						int primary_channel, double current_cca,
						int packets_in_buffer){

	switch(pause_or_resume){

		case PAUSE_TIMER:{

			if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s primary_channel (#%d) affected\n",
				SimTime, node_id, node_state, LOG_F00, LOG_LVL2, primary_channel);

			if(save_node_logs) fprintf(node_logger.file,
				"%f;N%d;S%d;%s;%s Power sensed in primary channel:  %f pW\n",
				SimTime, node_id, node_state, LOG_F00, LOG_LVL3, channel_power[primary_channel]);

			if(channel_power[primary_channel] > current_cca){	// CCA exceeded

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s CCA (%f pW) exceeded\n",
						SimTime, node_id, node_state, LOG_F00, LOG_LVL3, current_cca);

				return TRUE;

			} else {	// CCA NOT exceeded

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s CCA (%f pW) NOT exceeded\n",
					SimTime, node_id, node_state, LOG_F00, LOG_LVL3, current_cca);
				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s primary_channel (#%d) NOT affected\n",
					SimTime, node_id, node_state, LOG_F00, LOG_LVL3, primary_channel);

				return FALSE;

			}

			break;

		}

		case RESUME_TIMER:{

			if(packets_in_buffer > 0) {

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Primary_channel (#%d) affected\n",
					SimTime, node_id, node_state, LOG_F00, LOG_LVL2, primary_channel);

				if(save_node_logs) fprintf(node_logger.file,
						"%f;N%d;S%d;%s;%s Power sensed in primary channel:  %f pW\n",
						SimTime, node_id, node_state, LOG_F00, LOG_LVL3, channel_power[primary_channel]);

				if(channel_power[primary_channel] <= current_cca){	// CCA NOT exceeded

					if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s CCA (%f pW) NOT exceeded\n",
						SimTime, node_id, node_state, LOG_F00, LOG_LVL3, current_cca);
					if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s primary_channel (#%d) NOT affected\n",
						SimTime, node_id, node_state, LOG_F00, LOG_LVL3, primary_channel);

					return TRUE;

				} else {	// CCA exceeded

					if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s CCA (%f pW) exceeded\n",
						SimTime, node_id, node_state, LOG_F00, LOG_LVL3, current_cca);

					return FALSE;

				}

				break;
			}

			default:{

				if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Unknown mode %d! (not resume nor pause)\n",
					SimTime, node_id, node_state, LOG_F00, LOG_LVL3, pause_or_resume);

				return FALSE;

				break;

			}
		}
	}
}

/*
 * handleCW(): increase or decrease the CW.
 * - mode: flag for indicating increase or decrease of CW
 */
int handleCW(int mode, int current_CW, int CW_min, int CW_max) {
	// http://article.sapub.org/pdf/10.5923.j.jwnc.20130301.01.pdf

	int new_CW = 0;

	switch(mode){

		case INCREASE_CW:{
			if(2*current_CW < CW_max) {
				new_CW = 2*current_CW;
			} else {
				new_CW = CW_max;
			}
			break;
		}

		case DECREASE_CW:{
			new_CW = CW_min;
			break;
		}

		default:{
			break;
		}

	}

	return new_CW;

}
