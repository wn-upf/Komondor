#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../ListOfDefines.h"

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

//	// Set lambda accordingly
//	double EB = (double) (current_CW-1)/2;
//	lambda =  1/(EB * SLOT_TIME);
}
