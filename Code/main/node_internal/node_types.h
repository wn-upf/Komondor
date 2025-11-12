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
 * @file node_types.h
 * @brief Type definitions and enums for Node component
 * 
 * This file contains type definitions, enums, and constants used internally
 * by the Node component and its managers.
 */

#ifndef NODE_TYPES_H
#define NODE_TYPES_H

/**
 * @namespace NodeInternal
 * @brief Internal types and utilities for Node component
 * 
 * Note: We use numeric values directly to avoid macro expansion conflicts with list_of_macros.h.
 * The enum values match the STATE_* macros defined in list_of_macros.h.
 */
namespace NodeInternal {

/**
 * @enum NodeState
 * @brief Internal state of a node in the simulation
 * 
 * Represents the current operational state of a node, indicating what
 * activity the node is currently performing (sensing, transmitting, receiving, etc.)
 * 
 * These values match the STATE_* macros defined in list_of_macros.h:
 * STATE_UNKNOWN=-1, STATE_SENSING=0, STATE_TX_DATA=1, etc.
 */
enum NodeState {
    NODE_STATE_UNKNOWN = -1,    ///< Unknown state (e.g., at the beginning of simulation)
    NODE_STATE_SENSING = 0,     ///< Sensing the channel and decreasing backoff counter
    NODE_STATE_TX_DATA = 1,     ///< Transmitting data packet
    NODE_STATE_RX_DATA = 2,     ///< Receiving a data packet from another node
    NODE_STATE_WAIT_ACK = 3,    ///< Waiting for ACK after data transmission
    NODE_STATE_TX_ACK = 4,      ///< Transmitting ACK packet
    NODE_STATE_RX_ACK = 5,      ///< Receiving ACK packet
    NODE_STATE_TX_RTS = 6,      ///< Transmitting RTS packet
    NODE_STATE_TX_CTS = 7,      ///< Transmitting CTS packet
    NODE_STATE_RX_RTS = 8,      ///< Receiving RTS packet
    NODE_STATE_RX_CTS = 9,      ///< Receiving CTS packet
    NODE_STATE_WAIT_CTS = 10,   ///< Waiting for CTS packet after RTS
    NODE_STATE_WAIT_DATA = 11,  ///< Waiting for data packet after CTS
    NODE_STATE_NAV = 12,        ///< Virtual Carrier Sense (NAV active)
    NODE_STATE_SLEEP = 13       ///< Sleep state (not currently used)
};

/**
 * @enum NodeType
 * @brief Type of node in the network
 * 
 * These values match the NODE_TYPE_* macros defined in list_of_macros.h
 */
enum NodeType {
    NODE_INT_TYPE_UNKNOWN = -1,  ///< Unknown node type
    NODE_INT_TYPE_AP = 0,        ///< Access Point
    NODE_INT_TYPE_STA = 1,       ///< Station
    NODE_INT_TYPE_OTHER = 2      ///< Other type of device
};

/**
 * @enum PacketLossReason
 * @brief Reasons why a packet may be lost
 * 
 * These values match the PACKET_* macros defined in list_of_macros.h
 */
enum PacketLossReason {
    NODE_PACKET_NOT_LOST = 0,                    ///< Packet was successfully received
    NODE_PACKET_LOST_COLLISION = 1,              ///< Packet lost due to collision
    NODE_PACKET_LOST_INTERFERENCE = 2,           ///< Packet lost due to interference
    NODE_PACKET_LOST_SINR = 3,                   ///< Packet lost due to low SINR
    NODE_PACKET_LOST_CAPTURE_EFFECT = 4,         ///< Packet lost due to capture effect
    NODE_PACKET_IGNORED_SPATIAL_REUSE = 5,       ///< Packet ignored for spatial reuse
    NODE_PACKET_LOST_NAV_COLLISION = 6,          ///< Packet lost due to NAV collision
    NODE_PACKET_LOST_INTER_BSS_NAV_COLLISION = 7 ///< Packet lost due to inter-BSS NAV collision
};

/**
 * @enum PacketType
 * @brief Types of packets in the simulation
 * 
 * These values match the PACKET_TYPE_* macros defined in list_of_macros.h
 */
enum PacketType {
    NODE_PACKET_TYPE_RTS = 0,  ///< Request to Send
    NODE_PACKET_TYPE_CTS = 1,  ///< Clear to Send
    NODE_PACKET_TYPE_DATA = 2, ///< Data packet
    NODE_PACKET_TYPE_ACK = 3   ///< Acknowledgment
};

/**
 * @brief Convert NodeState enum to string for logging
 * @param state The node state to convert
 * @return String representation of the state
 */
inline const char* NodeStateToString(NodeState state) {
    switch (state) {
        case NODE_STATE_UNKNOWN: return "UNKNOWN";
        case NODE_STATE_SENSING: return "SENSING";
        case NODE_STATE_TX_DATA: return "TX_DATA";
        case NODE_STATE_RX_DATA: return "RX_DATA";
        case NODE_STATE_WAIT_ACK: return "WAIT_ACK";
        case NODE_STATE_TX_ACK: return "TX_ACK";
        case NODE_STATE_RX_ACK: return "RX_ACK";
        case NODE_STATE_TX_RTS: return "TX_RTS";
        case NODE_STATE_TX_CTS: return "TX_CTS";
        case NODE_STATE_RX_RTS: return "RX_RTS";
        case NODE_STATE_RX_CTS: return "RX_CTS";
        case NODE_STATE_WAIT_CTS: return "WAIT_CTS";
        case NODE_STATE_WAIT_DATA: return "WAIT_DATA";
        case NODE_STATE_NAV: return "NAV";
        case NODE_STATE_SLEEP: return "SLEEP";
        default: return "INVALID";
    }
}

} // namespace NodeInternal

#endif // NODE_TYPES_H

