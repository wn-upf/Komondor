/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 *
 * Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>
 * Everyone is permitted to copy and distribute verbatim copies
 * of this license document, but changing it is not allowed.
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
 * -----------------------------------------------------------------
 *
 * Author  : Sergio Barrachina-Muñoz and Francesc Wilhelmi
 * Created : 2016-12-05
 * Updated : 2024-12-11 (Refactoring)
 *
 * -----------------------------------------------------------------
 */

/**
 * @file node_constants.h
 * @brief Node-specific constants
 * 
 * This file contains constants used by the Node component that were
 * previously hardcoded in the node.h file.
 */

#ifndef NODE_CONSTANTS_H
#define NODE_CONSTANTS_H

// Note: We include list_of_macros.h for constants that are not enums
// For enum-like constants, we define them directly to avoid macro expansion conflicts
#include "../../list_of_macros.h"

/**
 * @namespace NodeInternal
 * @brief Internal constants for Node component
 */
namespace NodeInternal {

// Backoff constants
const int DEFAULT_BASE_BACKOFF_DETERMINISTIC = 5;  ///< Default base backoff for deterministic backoff

// Rho measurement constants
const double DEFAULT_DELTA_MEASURE_RHO = 0.00001;  ///< Default time between rho measurements [s]
const bool DEFAULT_FLAG_MEASURE_RHO = TRUE;        ///< Default flag for rho measurement activation

// Statistics measurement constants
const double DEFAULT_LAST_MEASUREMENTS_WINDOW = 10.0;  ///< Time window for last performance measurements [s]

// MCS constants
const int DEFAULT_MCS_ARRAY_SIZE = 4;  ///< Default size for MCS arrays

// Spatial reuse constants
const int SPATIAL_REUSE_TRANSMISSION_TYPES = 3;  ///< Number of PD types considered for spatial reuse

} // namespace NodeInternal

#endif // NODE_CONSTANTS_H

