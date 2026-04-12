/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
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
 * dcf_methods.h: this file contains functions for DCF
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>

/**
 * Double the contention window (Binary Exponential Backoff), capped at cw_stage_max.
 * Used on each failed transmission attempt for BACKOFF_DCF, BACKOFF_REPEAT_BO, BACKOFF_ECA.
 */
static void BinExpBackoffIncrease(int *cw_stage_current, int *current_cw_min,
		int *current_cw_max, int cw_min_default, int cw_max_default, int cw_stage_max) {
	if (*cw_stage_current < cw_stage_max) {
		*cw_stage_current += 1;
		*current_cw_min = cw_min_default * pow(2, *cw_stage_current);
		*current_cw_max = cw_max_default * pow(2, *cw_stage_current);
	}
}

/**
 * Reset the contention window to its default values.
 * Used on successful transmission for BACKOFF_DCF.
 */
static void DefaultCwReset(int *cw_stage_current, int *current_cw_min,
		int *current_cw_max, int cw_min_default, int cw_max_default) {
	*cw_stage_current = 0;
	*current_cw_min = cw_min_default;
	*current_cw_max = cw_max_default;
}

/**
* Increase or decrease the contention window. CW adaptation: http://article.sapub.org/pdf/10.5923.j.jwnc.20130301.01.pdf
* @param "cw_adaptation" [type int]: boolean indicating whether CW adaptation is enabled or not
* @param "increase_or_reset" [type int]: flag indicating whether the CW should be increased or reseted
* @param "current_cw_min" [type int*]: pointer to the current minimum CW (to be modified by this method)
* @param "current_cw_max" [type int*]: pointer to the current maximum CW (to be modified by this method)
* @param "cw_stage_current" [type int*]: pointer to the current CW stage (to be modified by this method)
* @param "cw_min_default" [type int]: default minimum CW
* @param "cw_max_default" [type int]: default maximum CW
* @param "cw_stage_max" [type int]: maximum CW stage (only for BACKOFF_DCF)
*/
static void HandleBackoffDCF(int cw_adaptation, int increase_or_reset, int *cw_stage_current, int *current_cw_min,
    int *current_cw_max, int cw_min_default, int cw_max_default, int cw_stage_max){
    if (cw_adaptation == TRUE) {
        switch (increase_or_reset) {
            case INCREASE_CW:
                BinExpBackoffIncrease(cw_stage_current, current_cw_min, current_cw_max,
                    cw_min_default, cw_max_default, cw_stage_max);
                break;
            case RESET_CW:
                DefaultCwReset(cw_stage_current, current_cw_min, current_cw_max,
                    cw_min_default, cw_max_default);
                break;
            default:
                printf("ERROR in HandleContentionWindow: Unknown operation on contention window!\n");
                exit(EXIT_FAILURE);
        }
    }
}

/**
* Compute a new backoff value
* @param "current_cw_min" [type int]: current minimum contention window
* @param "current_cw_max" [type int]: current maximum contention window
*/
double ComputeBackoffDCF(int current_cw_min, int current_cw_max) {

    int num_slots (current_cw_min + (std::rand() % ( current_cw_max - current_cw_min + 1 )));
    return num_slots * SLOT_TIME;

}