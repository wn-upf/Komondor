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
 * edca_methods.h: this file contains functions for EDCA
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>

/**
* Compute a new backoff value
* @param "traffic_type" [type int]: type of traffic for the upcoming channel access
*/
double ComputeBackoffEDCA(int traffic_type) {

    int current_cw_min, current_cw_max;

    // Get standard parameters regarding CW parameters
    switch(traffic_type){
        case AC_VO:{
            current_cw_min = CW_MIN_AC_VO;
            current_cw_max = CW_MAX_AC_VO;
            break;
        }
        case AC_VI:{
            current_cw_min = CW_MIN_AC_VI;
            current_cw_max = CW_MAX_AC_VI;
            break;
        }
        case AC_BE:{
            current_cw_min = CW_MIN_AC_BE;
            current_cw_max = CW_MAX_AC_BE;
            break;
        }
        case AC_BK: default:{
            current_cw_min = CW_MIN_AC_BK;
            current_cw_max = CW_MAX_AC_BK;
            break;
        }
    }
    int num_slots (current_cw_min + (std::rand() % ( current_cw_max - current_cw_min + 1 )));
    return num_slots * SLOT_TIME;

}