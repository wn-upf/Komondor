/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * channel_access_state.h: Operational state for CSMA/CA channel access and backoff.
 *
 * These are the private runtime fields that were previously scattered as individual
 * member variables in the Node class (node.h, private section):
 *   - remaining_backoff  (was at "State and timers" block)
 *   - cw_stage_current, current_cw_min, current_cw_max  (CW management block)
 *   - num_bo_interruptions, base_backoff_deterministic, deterministic_bo_active  (deterministic BO block)
 *   - previous_backoff  (ECA block)
 *
 * Grouping them here:
 *   - documents backoff/CW state as a coherent subsystem
 *   - enables future swap-out of the channel-access policy (e.g., OFDMA) without
 *     touching the Node declaration
 */

#ifndef _AUX_CHANNEL_ACCESS_STATE_
#define _AUX_CHANNEL_ACCESS_STATE_

struct ChannelAccessState
{
    // Live backoff countdown
    double remaining_backoff;           ///> Remaining backoff time [s]

    // Contention Window
    int    cw_stage_current;            ///> Current BEB stage (0 = initial, max = cw_stage_max)
    int    current_cw_min;              ///> Minimum CW currently in use
    int    current_cw_max;              ///> Maximum CW currently in use

    // Deterministic backoff (Qualcomm style)
    int    num_bo_interruptions;        ///> Number of BO interruptions observed in current round
    int    base_backoff_deterministic;  ///> Base slot count for deterministic BO
    int    deterministic_bo_active;     ///> TRUE when deterministic BO phase is active

    // ECA / repeat-BO
    double previous_backoff;            ///> Last backoff value drawn (used by BACKOFF_REPEAT_BO / ECA)
};

#endif /* _AUX_CHANNEL_ACCESS_STATE_ */
