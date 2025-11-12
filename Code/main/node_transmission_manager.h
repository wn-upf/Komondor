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
 * @file node_transmission_manager.h
 * @brief Node transmission management
 * 
 * This class manages transmission operations for a node, including transmission
 * initiation, RTS generation, channel selection, and transmission state management.
 * It encapsulates all transmission-related logic that was previously scattered
 * throughout the Node component.
 * 
 * @class NodeTransmissionManager
 * @brief Manages transmission for a node
 */

#ifndef NODE_TRANSMISSION_MANAGER_H
#define NODE_TRANSMISSION_MANAGER_H

#include "../structures/notification.h"
#include "../list_of_macros.h"

// Forward declarations
class Node;

/**
 * @class NodeTransmissionManager
 * @brief Manages transmission for a node
 * 
 * This class is responsible for:
 * - Transmission initiation
 * - RTS/CTS/DATA/ACK generation
 * - Channel selection for transmission
 * - Transmission state management
 * - Transmission duration computation
 * - Packet aggregation
 */
class NodeTransmissionManager {
public:
    /**
     * @brief Constructor
     */
    NodeTransmissionManager();
    
    /**
     * @brief Destructor
     */
    ~NodeTransmissionManager();
    
    /**
     * @brief Initialize transmission manager
     * @param frame_length Frame length [bytes]
     * @param max_num_packets_aggregated Maximum number of packets to aggregate
     */
    void Initialize(int frame_length, int max_num_packets_aggregated);
    
    /**
     * @brief Generate RTS notification
     * @param destination_id Destination node ID
     * @param packet_id Packet ID
     * @param num_packets_aggregated Number of packets aggregated
     * @param timestamp_generated Timestamp when packet was generated
     * @param tx_duration Transmission duration [s]
     * @return Generated RTS notification
     */
    Notification GenerateRTSNotification(int destination_id, int packet_id,
                                         int num_packets_aggregated,
                                         double timestamp_generated,
                                         double tx_duration);
    
    /**
     * @brief Generate CTS notification
     * @param destination_id Destination node ID
     * @param packet_id Packet ID
     * @param num_packets_aggregated Number of packets aggregated
     * @param timestamp_generated Timestamp when packet was generated
     * @param tx_duration Transmission duration [s]
     * @return Generated CTS notification
     */
    Notification GenerateCTSNotification(int destination_id, int packet_id,
                                         int num_packets_aggregated,
                                         double timestamp_generated,
                                         double tx_duration);
    
    /**
     * @brief Generate DATA notification
     * @param destination_id Destination node ID
     * @param packet_id Packet ID
     * @param num_packets_aggregated Number of packets aggregated
     * @param timestamp_generated Timestamp when packet was generated
     * @param tx_duration Transmission duration [s]
     * @return Generated DATA notification
     */
    Notification GenerateDATANotification(int destination_id, int packet_id,
                                          int num_packets_aggregated,
                                          double timestamp_generated,
                                          double tx_duration);
    
    /**
     * @brief Generate ACK notification
     * @param destination_id Destination node ID
     * @param packet_id Packet ID
     * @param num_packets_aggregated Number of packets aggregated
     * @param timestamp_generated Timestamp when packet was generated
     * @param tx_duration Transmission duration [s]
     * @return Generated ACK notification
     */
    Notification GenerateACKNotification(int destination_id, int packet_id,
                                         int num_packets_aggregated,
                                         double timestamp_generated,
                                         double tx_duration);
    
    /**
     * @brief Compute number of packets to aggregate
     * @param buffer_size Current buffer size
     * @param max_aggregated Maximum number of packets to aggregate
     * @param traffic_model Traffic model
     * @return Number of packets to aggregate
     */
    int ComputeNumPacketsToAggregate(int buffer_size, int max_aggregated, int traffic_model) const;
    
    /**
     * @brief Compute transmission durations
     * @param rts_duration Output RTS duration [s]
     * @param cts_duration Output CTS duration [s]
     * @param data_duration Output DATA duration [s]
     * @param ack_duration Output ACK duration [s]
     * @param num_channels Number of channels
     * @param modulation Modulation scheme
     * @param num_packets_aggregated Number of packets aggregated
     * @param frame_length Frame length [bytes]
     * @param bits_ofdm_sym Bits per OFDM symbol
     */
    void ComputeFramesDuration(double* rts_duration, double* cts_duration,
                               double* data_duration, double* ack_duration,
                               int num_channels, int modulation,
                               int num_packets_aggregated, int frame_length,
                               double bits_ofdm_sym) const;
    
    /**
     * @brief Compute NAV time
     * @param node_state Current node state
     * @param rts_duration RTS duration [s]
     * @param cts_duration CTS duration [s]
     * @param data_duration DATA duration [s]
     * @param ack_duration ACK duration [s]
     * @param sifs SIFS duration [s]
     * @return NAV time [s]
     */
    double ComputeNAVTime(int node_state, double rts_duration, double cts_duration,
                          double data_duration, double ack_duration, double sifs) const;
    
    /**
     * @brief Check if transmission is possible
     * @param channels_for_tx Array indicating which channels can be used for TX
     * @return true if transmission is possible, false otherwise
     */
    bool IsTransmissionPossible(const int* channels_for_tx) const;
    
    /**
     * @brief Get channel range for transmission
     * @param channels_for_tx Array indicating which channels can be used for TX
     * @param left_channel Output left channel
     * @param right_channel Output right channel
     * @return true if channels are valid, false otherwise
     */
    bool GetChannelRangeForTX(const int* channels_for_tx, int* left_channel, int* right_channel) const;
    
    /**
     * @brief Get current destination ID
     * @return Current destination ID
     */
    int GetCurrentDestinationID() const;
    
    /**
     * @brief Set current destination ID
     * @param destination_id Destination ID
     */
    void SetCurrentDestinationID(int destination_id);
    
    /**
     * @brief Get current transmission duration
     * @return Current transmission duration [s]
     */
    double GetCurrentTXDuration() const;
    
    /**
     * @brief Set current transmission duration
     * @param duration Transmission duration [s]
     */
    void SetCurrentTXDuration(double duration);
    
    /**
     * @brief Get current number of channels used for TX
     * @return Number of channels
     */
    int GetNumChannelsTX() const;
    
    /**
     * @brief Set current number of channels used for TX
     * @param num_channels Number of channels
     */
    void SetNumChannelsTX(int num_channels);
    
    /**
     * @brief Reset transmission manager
     */
    void Reset();

private:
    int current_destination_id_;          ///< Current destination ID
    double current_tx_duration_;          ///< Current transmission duration [s]
    int num_channels_tx_;                 ///< Number of channels used for TX
    int frame_length_;                    ///< Frame length [bytes]
    int max_num_packets_aggregated_;      ///< Maximum number of packets to aggregate
    
    /**
     * @brief Generate a generic notification
     * @param packet_type Packet type
     * @param destination_id Destination node ID
     * @param packet_id Packet ID
     * @param num_packets_aggregated Number of packets aggregated
     * @param timestamp_generated Timestamp when packet was generated
     * @param tx_duration Transmission duration [s]
     * @return Generated notification
     */
    Notification GenerateNotificationInternal(int packet_type, int destination_id,
                                              int packet_id, int num_packets_aggregated,
                                              double timestamp_generated, double tx_duration) const;
};

// Inline implementations

inline NodeTransmissionManager::NodeTransmissionManager()
    : current_destination_id_(-1), current_tx_duration_(0.0),
      num_channels_tx_(0), frame_length_(0), max_num_packets_aggregated_(0) {
}

inline NodeTransmissionManager::~NodeTransmissionManager() {
}

inline void NodeTransmissionManager::Initialize(int frame_length, int max_num_packets_aggregated) {
    frame_length_ = frame_length;
    max_num_packets_aggregated_ = max_num_packets_aggregated;
    Reset();
}

inline Notification NodeTransmissionManager::GenerateRTSNotification(int destination_id, int packet_id,
                                                                     int num_packets_aggregated,
                                                                     double timestamp_generated,
                                                                     double tx_duration) {
    return GenerateNotificationInternal(PACKET_TYPE_RTS, destination_id, packet_id,
                                       num_packets_aggregated, timestamp_generated, tx_duration);
}

inline Notification NodeTransmissionManager::GenerateCTSNotification(int destination_id, int packet_id,
                                                                     int num_packets_aggregated,
                                                                     double timestamp_generated,
                                                                     double tx_duration) {
    return GenerateNotificationInternal(PACKET_TYPE_CTS, destination_id, packet_id,
                                       num_packets_aggregated, timestamp_generated, tx_duration);
}

inline Notification NodeTransmissionManager::GenerateDATANotification(int destination_id, int packet_id,
                                                                      int num_packets_aggregated,
                                                                      double timestamp_generated,
                                                                      double tx_duration) {
    return GenerateNotificationInternal(PACKET_TYPE_DATA, destination_id, packet_id,
                                       num_packets_aggregated, timestamp_generated, tx_duration);
}

inline Notification NodeTransmissionManager::GenerateACKNotification(int destination_id, int packet_id,
                                                                     int num_packets_aggregated,
                                                                     double timestamp_generated,
                                                                     double tx_duration) {
    return GenerateNotificationInternal(PACKET_TYPE_ACK, destination_id, packet_id,
                                       num_packets_aggregated, timestamp_generated, tx_duration);
}

inline int NodeTransmissionManager::ComputeNumPacketsToAggregate(int buffer_size, int max_aggregated, int traffic_model) const {
    if (traffic_model == TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION) {
        return max_aggregated;
    }
    return (buffer_size > max_aggregated) ? max_aggregated : buffer_size;
}

inline bool NodeTransmissionManager::IsTransmissionPossible(const int* channels_for_tx) const {
    if (channels_for_tx == NULL) {
        return false;
    }
    return (channels_for_tx[0] != TX_NOT_POSSIBLE);
}

inline int NodeTransmissionManager::GetCurrentDestinationID() const {
    return current_destination_id_;
}

inline void NodeTransmissionManager::SetCurrentDestinationID(int destination_id) {
    current_destination_id_ = destination_id;
}

inline double NodeTransmissionManager::GetCurrentTXDuration() const {
    return current_tx_duration_;
}

inline void NodeTransmissionManager::SetCurrentTXDuration(double duration) {
    current_tx_duration_ = duration;
}

inline int NodeTransmissionManager::GetNumChannelsTX() const {
    return num_channels_tx_;
}

inline void NodeTransmissionManager::SetNumChannelsTX(int num_channels) {
    num_channels_tx_ = num_channels;
}

inline void NodeTransmissionManager::Reset() {
    current_destination_id_ = -1;
    current_tx_duration_ = 0.0;
    num_channels_tx_ = 0;
}

inline Notification NodeTransmissionManager::GenerateNotificationInternal(int packet_type, int destination_id,
                                                                          int packet_id, int num_packets_aggregated,
                                                                          double timestamp_generated, double tx_duration) const {
    Notification notification;
    notification.packet_type = packet_type;
    notification.destination_id = destination_id;
    notification.packet_id = packet_id;
    notification.tx_info.num_packets_aggregated = num_packets_aggregated;
    notification.timestamp_generated = timestamp_generated;
    notification.tx_duration = tx_duration; // tx_duration is a direct member of Notification
    // Additional fields would be set based on packet type
    return notification;
}

// Note: ComputeFramesDuration, ComputeNAVTime, and GetChannelRangeForTX would
// need to call external functions from the methods directory. These are placeholders
// that show the interface. The actual implementation would delegate to existing functions.

#endif // NODE_TRANSMISSION_MANAGER_H

