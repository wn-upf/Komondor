/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */

/**
 * node_impl.h: Aggregator for all Node:: method implementation fragments.
 *
 * Include this ONCE at the end of node.h, after the Node class definition.
 * Each included file contains the implementation bodies of the Node:: methods
 * declared in the class; they are NOT standalone headers and must not be
 * included anywhere else.
 *
 * Impl-fragment files and their responsibilities:
 *   - node_statistics_methods.h  : performance metrics, print/write helpers, SaveSimulationPerformance
 *   - node_config_methods.h      : agent configuration inports, ApplyNewConfiguration
 *   - node_spatial_reuse_methods.h : OBSS/PD spatial reuse inports and opportunity handling
 *   - node_mcs_methods.h         : SelectMCSResponse, NACK/MCS negotiation inports, traffic inport
 *   - node_timeout_methods.h     : ACK / CTS / Data / NAV timeout handlers
 *   - node_backoff_methods.h     : backoff scheduling, pause/resume, node restart, channel sensing
 *   - node_packet_methods.h      : packet generation, MCS request, GenerateNotification, SendResponsePacket
 */

#ifndef NODE_IMPL_H
#define NODE_IMPL_H

#include "node_statistics_methods.h"
#include "node_config_methods.h"
#include "node_spatial_reuse_methods.h"
#include "node_mcs_methods.h"
#include "node_timeout_methods.h"
#include "node_backoff_methods.h"
#include "node_packet_methods.h"

#endif /* NODE_IMPL_H */
