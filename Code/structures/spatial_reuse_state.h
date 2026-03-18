/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * spatial_reuse_state.h: Operational state for the 11ax Spatial Reuse (SR / OBSS-PD) mechanism.
 *
 * These are the private runtime fields that were previously scattered as individual
 * member variables in the Node class (node.h, private section, lines 446-459).
 * Grouping them here:
 *   - documents SR state as a coherent subsystem
 *   - enables future swap-out of the SR policy without touching the Node declaration
 */

#ifndef _AUX_SPATIAL_REUSE_STATE_
#define _AUX_SPATIAL_REUSE_STATE_

struct SpatialReuseState
{
    int    spatial_reuse_enabled;           ///> Indicates whether SR is enabled or not
    int    type_last_sensed_packet;         ///> Type of the last sensed packet (INTRA/INTER_BSS_FRAME)
    double pd_spatial_reuse;               ///> PD threshold [pW] to apply during SR operation
    double tx_power_sr;                    ///> TX power limit [pW] applied during SR operation
    int    txop_sr_identified;             ///> TRUE when a valid SR-based TXOP has been detected
    int    type_ongoing_transmissions_sr[3]; ///> Which frame types are ongoing under SR (3 PD types)
    double next_pd_spatial_reuse;          ///> PD threshold to apply when restarting after SR TXOP
    bool   flag_change_in_tx_power;        ///> TRUE when TX power was modified for SR; triggers MCS refresh
    double potential_obss_pd_threshold;    ///> Candidate OBSS/PD threshold for the next SR opportunity
    double current_obss_pd_threshold;      ///> Active OBSS/PD threshold currently in use
    double next_tx_power_limit;            ///> TX power cap [pW] to apply at the next transmission
    double current_tx_power_sr;            ///> TX power [pW] used within the current SR TXOP
    int    mapc_cosr_active;               ///> TRUE during Co-SR simultaneous DATA phase (bypasses spatial_reuse_enabled)
};

#endif /* _AUX_SPATIAL_REUSE_STATE_ */
