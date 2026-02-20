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
 * eca_methods.h: this file contains functions for the ECA & "repeat BO"
 *  - REPEAT_BO: https://arxiv.org/pdf/1512.02062
 *	- ECA:       https://arxiv.org/pdf/1311.0787
 *  - REPEAT_BO and ECA share identical behavior:
 *      + failure → double CW + disable deterministic mode
 *		+ success → enable deterministic mode (reuse last backoff / half-CW)
 */

 /**
* Increase or decrease the contention window. 
* @param "deterministic_bo_active" [type int*]: flag indicating whether deterministic BO is actived or not (to be modified by this method, only for BACKOFF_DETERMINISTIC_QUALCOMM)
*/
void HandleBackoffECA(int increase_or_reset, int *cw_stage_current, int *current_cw_min, int *current_cw_max,
    int cw_min_default, int cw_max_default, int cw_stage_max, int *deterministic_bo_active) {

    switch (increase_or_reset) {
        case INCREASE_CW:
            BinExpBackoffIncrease(cw_stage_current, current_cw_min, current_cw_max,
                cw_min_default, cw_max_default, cw_stage_max);
            *deterministic_bo_active = 0;
            break;
        case RESET_CW:
            *deterministic_bo_active = 1;
            break;
        default:
            printf("ERROR in HandleContentionWindow: Unknown operation on contention window!\n");
            exit(EXIT_FAILURE);
    }

}

/**
* Compute a new backoff value
* @param "current_cw_min" [type int]: current minimum contention window
* @param "current_cw_max" [type int]: current maximum contention window
* @param "deterministic_bo_active" [type int]: flag indicating whether deterministic BO is active or not (Only for BACKOFF_DETERMINISTIC_QUALCOMM)
* @param "base_backoff_deterministic" [type int]: base backoff (Only for BACKOFF_DETERMINISTIC_QUALCOMM)
* @param "previous_backoff" [type double]: previous employed backoff backoff (Only for BACKOFF_REPEAT_BO)
*/
double ComputeBackoffECA(int deterministic_bo_active, int previous_backoff, 
    int base_backoff_deterministic, int current_cw_min, int current_cw_max) {

    if (deterministic_bo_active && previous_backoff != -1) {
        return base_backoff_deterministic * SLOT_TIME;
    } else {
        int num_slots (current_cw_min + (std::rand() % ( current_cw_max - current_cw_min + 1 )));
        return num_slots * SLOT_TIME;
    }

}

/**
* Compute a new backoff value
* @param "current_cw_min" [type int]: current minimum contention window
* @param "current_cw_max" [type int]: current maximum contention window
* @param "deterministic_bo_active" [type int]: flag indicating whether deterministic BO is active or not (Only for BACKOFF_DETERMINISTIC_QUALCOMM)
* @param "previous_backoff" [type double]: previous employed backoff backoff (Only for BACKOFF_REPEAT_BO)
*/
double ComputeBackoffRepeat(int deterministic_bo_active, int previous_backoff, int current_cw_min, int current_cw_max) {

    if (deterministic_bo_active && previous_backoff != -1) {
        return previous_backoff;
    } else {
        int num_slots (current_cw_min + (std::rand() % ( current_cw_max - current_cw_min + 1 )));
        return num_slots * SLOT_TIME;
    }

}

