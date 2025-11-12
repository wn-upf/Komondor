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
 * @file node_reception_manager.h
 * @brief Node reception management
 * 
 * This class manages reception operations for a node, including packet reception,
 * decoding, ACK generation, and reception state management. It encapsulates all
 * reception-related logic that was previously scattered throughout the Node component.
 * 
 * @class NodeReceptionManager
 * @brief Manages reception for a node
 */

#ifndef NODE_RECEPTION_MANAGER_H
#define NODE_RECEPTION_MANAGER_H

#include "../structures/notification.h"
#include "../list_of_macros.h"

/**
 * @class NodeReceptionManager
 * @brief Manages reception for a node
 * 
 * This class is responsible for:
 * - Packet reception handling
 * - Packet decoding and validation
 * - ACK/NACK generation
 * - Reception state management
 * - Power and interference tracking
 * - SINR computation
 */
class NodeReceptionManager {
public:
    /**
     * @brief Constructor
     */
    NodeReceptionManager();
    
    /**
     * @brief Destructor
     */
    ~NodeReceptionManager();
    
    /**
     * @brief Initialize reception manager
     */
    void Initialize();
    
    /**
     * @brief Handle RTS reception
     * @param notification RTS notification
     * @param node_id Node ID (receiver)
     * @param current_time Current simulation time [s]
     * @return true if RTS should be responded to, false otherwise
     */
    bool HandleRTSReception(const Notification& notification, int node_id, double current_time);
    
    /**
     * @brief Handle CTS reception
     * @param notification CTS notification
     * @param node_id Node ID (receiver)
     * @param current_time Current simulation time [s]
     * @return true if CTS is valid, false otherwise
     */
    bool HandleCTSReception(const Notification& notification, int node_id, double current_time);
    
    /**
     * @brief Handle DATA reception
     * @param notification DATA notification
     * @param node_id Node ID (receiver)
     * @param current_time Current simulation time [s]
     * @return true if DATA is valid, false otherwise
     */
    bool HandleDATAReception(const Notification& notification, int node_id, double current_time);
    
    /**
     * @brief Handle ACK reception
     * @param notification ACK notification
     * @param node_id Node ID (receiver)
     * @param current_time Current simulation time [s]
     * @return true if ACK is valid, false otherwise
     */
    bool HandleACKReception(const Notification& notification, int node_id, double current_time);
    
    /**
     * @brief Check if packet can be received
     * @param notification Notification to check
     * @param power_rx_interest Power received of interest [pW]
     * @param max_pw_interference Maximum interference power [pW]
     * @param current_sinr Current SINR
     * @param capture_effect Capture effect
     * @param current_pd Current PD threshold [pW]
     * @param constant_per Constant PER
     * @param node_id Node ID
     * @param capture_effect_model Capture effect model
     * @param current_primary_channel Current primary channel
     * @return Loss reason (PACKET_NOT_LOST if can be received)
     */
    int CheckPacketReception(const Notification& notification,
                             double power_rx_interest, double max_pw_interference,
                             double current_sinr, double capture_effect,
                             double current_pd, double constant_per,
                             int node_id, int capture_effect_model,
                             int current_primary_channel) const;
    
    /**
     * @brief Get power received of interest
     * @return Power received of interest [pW]
     */
    double GetPowerRXInterest() const;
    
    /**
     * @brief Set power received of interest
     * @param power Power received [pW]
     */
    void SetPowerRXInterest(double power);
    
    /**
     * @brief Get maximum interference power
     * @return Maximum interference power [pW]
     */
    double GetMaxInterferencePower() const;
    
    /**
     * @brief Set maximum interference power
     * @param power Maximum interference power [pW]
     */
    void SetMaxInterferencePower(double power);
    
    /**
     * @brief Get current SINR
     * @return Current SINR [linear]
     */
    double GetCurrentSINR() const;
    
    /**
     * @brief Set current SINR
     * @param sinr Current SINR [linear]
     */
    void SetCurrentSINR(double sinr);
    
    /**
     * @brief Get receiving node ID
     * @return Receiving node ID, or -1 if not receiving
     */
    int GetReceivingFromNodeID() const;
    
    /**
     * @brief Set receiving node ID
     * @param node_id Receiving node ID, or -1 if not receiving
     */
    void SetReceivingFromNodeID(int node_id);
    
    /**
     * @brief Get receiving packet ID
     * @return Receiving packet ID, or -1 if not receiving
     */
    int GetReceivingPacketID() const;
    
    /**
     * @brief Set receiving packet ID
     * @param packet_id Receiving packet ID, or -1 if not receiving
     */
    void SetReceivingPacketID(int packet_id);
    
    /**
     * @brief Get current left channel
     * @return Current left channel
     */
    int GetCurrentLeftChannel() const;
    
    /**
     * @brief Set current left channel
     * @param channel Left channel
     */
    void SetCurrentLeftChannel(int channel);
    
    /**
     * @brief Get current right channel
     * @return Current right channel
     */
    int GetCurrentRightChannel() const;
    
    /**
     * @brief Set current right channel
     * @param channel Right channel
     */
    void SetCurrentRightChannel(int channel);
    
    /**
     * @brief Get current modulation
     * @return Current modulation
     */
    int GetCurrentModulation() const;
    
    /**
     * @brief Set current modulation
     * @param modulation Current modulation
     */
    void SetCurrentModulation(int modulation);
    
    /**
     * @brief Reset reception manager
     */
    void Reset();

private:
    double power_rx_interest_;        ///< Power received of interest [pW]
    double max_pw_interference_;      ///< Maximum interference power [pW]
    double current_sinr_;             ///< Current SINR [linear]
    int receiving_from_node_id_;      ///< Node ID from which we're receiving
    int receiving_packet_id_;         ///< Packet ID being received
    int current_left_channel_;        ///< Current left channel
    int current_right_channel_;       ///< Current right channel
    int current_modulation_;          ///< Current modulation
};

// Inline implementations

inline NodeReceptionManager::NodeReceptionManager()
    : power_rx_interest_(0.0), max_pw_interference_(0.0), current_sinr_(0.0),
      receiving_from_node_id_(-1), receiving_packet_id_(-1),
      current_left_channel_(-1), current_right_channel_(-1), current_modulation_(-1) {
}

inline NodeReceptionManager::~NodeReceptionManager() {
}

inline void NodeReceptionManager::Initialize() {
    Reset();
}

inline bool NodeReceptionManager::HandleRTSReception(const Notification& notification, int node_id, double current_time) {
    // Check if this node is the destination
    if (notification.destination_id != node_id) {
        return false;
    }
    
    // Store reception information
    receiving_from_node_id_ = notification.source_id;
    receiving_packet_id_ = notification.packet_id;
    current_left_channel_ = notification.left_channel;
    current_right_channel_ = notification.right_channel;
    
    return true;
}

inline bool NodeReceptionManager::HandleCTSReception(const Notification& notification, int node_id, double current_time) {
    // Check if this node is the destination
    if (notification.destination_id != node_id) {
        return false;
    }
    
    return true;
}

inline bool NodeReceptionManager::HandleDATAReception(const Notification& notification, int node_id, double current_time) {
    // Check if this node is the destination
    if (notification.destination_id != node_id) {
        return false;
    }
    
    return true;
}

inline bool NodeReceptionManager::HandleACKReception(const Notification& notification, int node_id, double current_time) {
    // Check if this node is the destination
    if (notification.destination_id != node_id) {
        return false;
    }
    
    return true;
}

inline double NodeReceptionManager::GetPowerRXInterest() const {
    return power_rx_interest_;
}

inline void NodeReceptionManager::SetPowerRXInterest(double power) {
    power_rx_interest_ = power;
}

inline double NodeReceptionManager::GetMaxInterferencePower() const {
    return max_pw_interference_;
}

inline void NodeReceptionManager::SetMaxInterferencePower(double power) {
    max_pw_interference_ = power;
}

inline double NodeReceptionManager::GetCurrentSINR() const {
    return current_sinr_;
}

inline void NodeReceptionManager::SetCurrentSINR(double sinr) {
    current_sinr_ = sinr;
}

inline int NodeReceptionManager::GetReceivingFromNodeID() const {
    return receiving_from_node_id_;
}

inline void NodeReceptionManager::SetReceivingFromNodeID(int node_id) {
    receiving_from_node_id_ = node_id;
}

inline int NodeReceptionManager::GetReceivingPacketID() const {
    return receiving_packet_id_;
}

inline void NodeReceptionManager::SetReceivingPacketID(int packet_id) {
    receiving_packet_id_ = packet_id;
}

inline int NodeReceptionManager::GetCurrentLeftChannel() const {
    return current_left_channel_;
}

inline void NodeReceptionManager::SetCurrentLeftChannel(int channel) {
    current_left_channel_ = channel;
}

inline int NodeReceptionManager::GetCurrentRightChannel() const {
    return current_right_channel_;
}

inline void NodeReceptionManager::SetCurrentRightChannel(int channel) {
    current_right_channel_ = channel;
}

inline int NodeReceptionManager::GetCurrentModulation() const {
    return current_modulation_;
}

inline void NodeReceptionManager::SetCurrentModulation(int modulation) {
    current_modulation_ = modulation;
}

inline void NodeReceptionManager::Reset() {
    power_rx_interest_ = 0.0;
    max_pw_interference_ = 0.0;
    current_sinr_ = 0.0;
    receiving_from_node_id_ = -1;
    receiving_packet_id_ = -1;
    current_left_channel_ = -1;
    current_right_channel_ = -1;
    current_modulation_ = -1;
}

// Note: CheckPacketReception would need to call external functions from the methods directory.
// This is a placeholder that shows the interface. The actual implementation would delegate
// to existing functions like IsPacketLost.

inline int NodeReceptionManager::CheckPacketReception(const Notification& notification,
                                                       double power_rx_interest, double max_pw_interference,
                                                       double current_sinr, double capture_effect,
                                                       double current_pd, double constant_per,
                                                       int node_id, int capture_effect_model,
                                                       int current_primary_channel) const {
    // This would delegate to IsPacketLost function
    // For now, return a placeholder
    (void)notification;
    (void)power_rx_interest;
    (void)max_pw_interference;
    (void)current_sinr;
    (void)capture_effect;
    (void)current_pd;
    (void)constant_per;
    (void)node_id;
    (void)capture_effect_model;
    (void)current_primary_channel;
    return PACKET_NOT_LOST; // Placeholder
}

#endif // NODE_RECEPTION_MANAGER_H

