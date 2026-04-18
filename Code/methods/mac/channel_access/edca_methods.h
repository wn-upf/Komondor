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
 * edca_methods.h: this file contains functions for EDCA
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>

/**
 * Return AIFS duration [s] for a given EDCA access category.
 * AIFS[AC] = SIFS + AIFSN[AC] * SLOT_TIME
 * @param "traffic_type" [type int]: AC_VO, AC_VI, AC_BE, or AC_BK
 */
double ComputeAIFS(int traffic_type) {
    int aifsn;
    switch (traffic_type) {
        case AC_VO: aifsn = AIFSN_VO; break;
        case AC_VI: aifsn = AIFSN_VI; break;
        case AC_BE: aifsn = AIFSN_BE; break;
        case AC_BK: default: aifsn = AIFSN_BK; break;
    }
    return SIFS + aifsn * SLOT_TIME;
}

/**
 * Return the EDCA TXOP_Limit [s] for a given access category.
 * A return value of 0.0 means "single PPDU per channel access" (IEEE 802.11-2020
 * §10.22.2.2) — NOT unlimited.  In Komondor's single-PPDU model this is already
 * the default; the A-MPDU size is bounded by IEEE_AX_MAX_PPDU_DURATION.
 * A positive return value additionally caps the A-MPDU data duration to that limit.
 * @param "traffic_type" [type int]: AC_VO, AC_VI, AC_BE, or AC_BK
 */
double ComputeTxopLimit(int traffic_type) {
    switch (traffic_type) {
        case AC_VO: return TXOP_LIMIT_AC_VO;
        case AC_VI: return TXOP_LIMIT_AC_VI;
        case AC_BE: return TXOP_LIMIT_AC_BE;
        case AC_BK: default: return TXOP_LIMIT_AC_BK;
    }
}

/**
 * Return the per-AC standard CW_min [slots].
 * This is the floor used when resetting after a successful transmission.
 */
static int GetAcCwMin(int traffic_type) {
    switch (traffic_type) {
        case AC_VO: return CW_MIN_AC_VO;
        case AC_VI: return CW_MIN_AC_VI;
        case AC_BE: return CW_MIN_AC_BE;
        case AC_BK: default: return CW_MIN_AC_BK;
    }
}

/**
 * Return the per-AC standard CW_max [slots].
 * This is the ceiling for the BEB doubling sequence.
 */
static int GetAcCwMax(int traffic_type) {
    switch (traffic_type) {
        case AC_VO: return CW_MAX_AC_VO;
        case AC_VI: return CW_MAX_AC_VI;
        case AC_BE: return CW_MAX_AC_BE;
        case AC_BK: default: return CW_MAX_AC_BK;
    }
}

/**
 * EDCA Binary Exponential Backoff (BEB) — IEEE 802.11-2020 §10.22.2.4.
 * current_cw_max represents the live CW value; current_cw_min is always 0.
 *
 * INCREASE_CW (collision):  CW = min((CW+1)*2 - 1, CW_max[AC])
 * RESET_CW    (success):    CW = CW_min[AC]
 */
static void HandleBackoffEDCA(int increase_or_reset, int traffic_type,
        int *cw_stage_current, int *current_cw_min, int *current_cw_max) {
    int cw_max_ac = GetAcCwMax(traffic_type);
    int cw_min_ac = GetAcCwMin(traffic_type);
    switch (increase_or_reset) {
        case INCREASE_CW:
            if (*current_cw_max < cw_max_ac) {
                *current_cw_max = (*current_cw_max + 1) * 2 - 1;
                if (*current_cw_max > cw_max_ac) *current_cw_max = cw_max_ac;
                ++(*cw_stage_current);
            }
            break;
        case RESET_CW:
            *cw_stage_current = 0;
            *current_cw_min   = 0;
            *current_cw_max   = cw_min_ac;
            break;
        default:
            printf("ERROR in HandleBackoffEDCA: Unknown CW operation.\n");
            break;
    }
}

/**
 * Compute a new backoff value for EDCA using the live CW state.
 * Draws uniformly from [current_cw_min, current_cw_max] (in slots).
 * current_cw_min = 0 always for EDCA; current_cw_max doubles on each collision
 * up to CW_max[AC], and resets to CW_min[AC] after a successful transmission.
 *
 * @param "current_cw_min" [type int]: lower bound of the draw range (always 0 for EDCA)
 * @param "current_cw_max" [type int]: current CW size (== live CW value)
 */
double ComputeBackoffEDCA(int current_cw_min, int current_cw_max) {
    int num_slots = current_cw_min + (std::rand() % (current_cw_max - current_cw_min + 1));
    return num_slots * SLOT_TIME;
}