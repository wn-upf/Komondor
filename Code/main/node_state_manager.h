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
 * @file node_state_manager.h
 * @brief Node state management
 * 
 * This class manages the internal state of a node, including state transitions
 * and state validation. It encapsulates all state-related logic that was
 * previously scattered throughout the Node component.
 * 
 * @class NodeStateManager
 * @brief Manages node internal state and state transitions
 */

#ifndef NODE_STATE_MANAGER_H
#define NODE_STATE_MANAGER_H

#include "node_internal/node_types.h"
#include "../list_of_macros.h"

/**
 * @class NodeStateManager
 * @brief Manages the internal state of a node
 * 
 * This class is responsible for:
 * - Managing node state transitions
 * - Validating state changes
 * - Providing state query methods
 * - Ensuring state consistency
 */
class NodeStateManager {
public:
    /**
     * @brief Constructor
     * @param initial_state Initial state of the node (default: NODE_STATE_SENSING)
     */
    NodeStateManager(NodeInternal::NodeState initial_state = NodeInternal::NODE_STATE_SENSING);
    
    /**
     * @brief Destructor
     */
    ~NodeStateManager();
    
    /**
     * @brief Set the node state
     * @param new_state The new state to set
     * @return true if state change was successful, false otherwise
     */
    bool SetState(NodeInternal::NodeState new_state);
    
    /**
     * @brief Get the current node state
     * @return Current node state
     */
    NodeInternal::NodeState GetState() const;
    
    /**
     * @brief Check if node is in a specific state
     * @param state The state to check
     * @return true if node is in the specified state, false otherwise
     */
    bool IsInState(NodeInternal::NodeState state) const;
    
    /**
     * @brief Check if node can transmit
     * @return true if node can transmit, false otherwise
     */
    bool CanTransmit() const;
    
    /**
     * @brief Check if node can receive
     * @return true if node can receive, false otherwise
     */
    bool CanReceive() const;
    
    /**
     * @brief Check if node is sensing
     * @return true if node is in sensing state, false otherwise
     */
    bool IsSensing() const;
    
    /**
     * @brief Check if node is transmitting
     * @return true if node is transmitting, false otherwise
     */
    bool IsTransmitting() const;
    
    /**
     * @brief Check if node is receiving
     * @return true if node is receiving, false otherwise
     */
    bool IsReceiving() const;
    
    /**
     * @brief Check if node is in NAV state
     * @return true if node is in NAV state, false otherwise
     */
    bool IsInNAV() const;
    
    /**
     * @brief Check if node is waiting for ACK
     * @return true if node is waiting for ACK, false otherwise
     */
    bool IsWaitingForACK() const;
    
    /**
     * @brief Check if node is waiting for CTS
     * @return true if node is waiting for CTS, false otherwise
     */
    bool IsWaitingForCTS() const;
    
    /**
     * @brief Check if state transition is valid
     * @param from_state Source state
     * @param to_state Destination state
     * @return true if transition is valid, false otherwise
     */
    static bool IsValidTransition(NodeInternal::NodeState from_state, 
                                   NodeInternal::NodeState to_state);
    
    /**
     * @brief Reset state to initial state
     */
    void Reset();
    
    /**
     * @brief Get state as integer (for backward compatibility)
     * @return Current state as integer
     */
    int GetStateAsInt() const;
    
    /**
     * @brief Set state from integer (for backward compatibility)
     * @param state_int State as integer
     * @return true if state change was successful, false otherwise
     */
    bool SetStateFromInt(int state_int);

private:
    NodeInternal::NodeState current_state_;  ///< Current node state
    NodeInternal::NodeState initial_state_;  ///< Initial state
    
    /**
     * @brief Validate state transition
     * @param new_state The new state to validate
     * @return true if transition is valid, false otherwise
     */
    bool ValidateTransition(NodeInternal::NodeState new_state) const;
    
    /**
     * @brief Convert integer to NodeState enum
     * @param state_int State as integer
     * @return NodeState enum value
     */
    static NodeInternal::NodeState IntToNodeState(int state_int);
    
    /**
     * @brief Convert NodeState enum to integer
     * @param state NodeState enum value
     * @return State as integer
     */
    static int NodeStateToInt(NodeInternal::NodeState state);
};

// Inline implementations

inline NodeStateManager::NodeStateManager(NodeInternal::NodeState initial_state)
    : current_state_(initial_state), initial_state_(initial_state) {
    // Constructor initializes state manager with given state
}

inline NodeStateManager::~NodeStateManager() {
}

inline bool NodeStateManager::SetState(NodeInternal::NodeState new_state) {
    if (ValidateTransition(new_state)) {
        current_state_ = new_state;
        return true;
    }
    return false;
}

inline NodeInternal::NodeState NodeStateManager::GetState() const {
    return current_state_;
}

inline bool NodeStateManager::IsInState(NodeInternal::NodeState state) const {
    return current_state_ == state;
}

inline bool NodeStateManager::CanTransmit() const {
    return (current_state_ == NodeInternal::NODE_STATE_SENSING ||
            current_state_ == NodeInternal::NODE_STATE_WAIT_ACK ||
            current_state_ == NodeInternal::NODE_STATE_WAIT_CTS);
}

inline bool NodeStateManager::CanReceive() const {
    return (current_state_ == NodeInternal::NODE_STATE_SENSING ||
            current_state_ == NodeInternal::NODE_STATE_RX_DATA ||
            current_state_ == NodeInternal::NODE_STATE_RX_RTS ||
            current_state_ == NodeInternal::NODE_STATE_RX_CTS ||
            current_state_ == NodeInternal::NODE_STATE_RX_ACK ||
            current_state_ == NodeInternal::NODE_STATE_NAV);
}

inline bool NodeStateManager::IsSensing() const {
    return current_state_ == NodeInternal::NODE_STATE_SENSING;
}

inline bool NodeStateManager::IsTransmitting() const {
    return (current_state_ == NodeInternal::NODE_STATE_TX_DATA ||
            current_state_ == NodeInternal::NODE_STATE_TX_RTS ||
            current_state_ == NodeInternal::NODE_STATE_TX_CTS ||
            current_state_ == NodeInternal::NODE_STATE_TX_ACK);
}

inline bool NodeStateManager::IsReceiving() const {
    return (current_state_ == NodeInternal::NODE_STATE_RX_DATA ||
            current_state_ == NodeInternal::NODE_STATE_RX_RTS ||
            current_state_ == NodeInternal::NODE_STATE_RX_CTS ||
            current_state_ == NodeInternal::NODE_STATE_RX_ACK);
}

inline bool NodeStateManager::IsInNAV() const {
    return current_state_ == NodeInternal::NODE_STATE_NAV;
}

inline bool NodeStateManager::IsWaitingForACK() const {
    return current_state_ == NodeInternal::NODE_STATE_WAIT_ACK;
}

inline bool NodeStateManager::IsWaitingForCTS() const {
    return current_state_ == NodeInternal::NODE_STATE_WAIT_CTS;
}

inline void NodeStateManager::Reset() {
    current_state_ = initial_state_;
}

inline int NodeStateManager::GetStateAsInt() const {
    return NodeStateToInt(current_state_);
}

inline bool NodeStateManager::SetStateFromInt(int state_int) {
    NodeInternal::NodeState new_state = IntToNodeState(state_int);
    return SetState(new_state);
}

inline NodeInternal::NodeState NodeStateManager::IntToNodeState(int state_int) {
    // Convert integer state (from STATE_* macros) to NodeState enum
    // The enum values match the macro values
    switch (state_int) {
        case -1: return NodeInternal::NODE_STATE_UNKNOWN;
        case 0: return NodeInternal::NODE_STATE_SENSING;
        case 1: return NodeInternal::NODE_STATE_TX_DATA;
        case 2: return NodeInternal::NODE_STATE_RX_DATA;
        case 3: return NodeInternal::NODE_STATE_WAIT_ACK;
        case 4: return NodeInternal::NODE_STATE_TX_ACK;
        case 5: return NodeInternal::NODE_STATE_RX_ACK;
        case 6: return NodeInternal::NODE_STATE_TX_RTS;
        case 7: return NodeInternal::NODE_STATE_TX_CTS;
        case 8: return NodeInternal::NODE_STATE_RX_RTS;
        case 9: return NodeInternal::NODE_STATE_RX_CTS;
        case 10: return NodeInternal::NODE_STATE_WAIT_CTS;
        case 11: return NodeInternal::NODE_STATE_WAIT_DATA;
        case 12: return NodeInternal::NODE_STATE_NAV;
        case 13: return NodeInternal::NODE_STATE_SLEEP;
        default: return NodeInternal::NODE_STATE_UNKNOWN;
    }
}

inline int NodeStateManager::NodeStateToInt(NodeInternal::NodeState state) {
    return static_cast<int>(state);
}

inline bool NodeStateManager::ValidateTransition(NodeInternal::NodeState new_state) const {
    // Basic validation: allow all transitions for now
    // Can be extended with state machine rules if needed
    return IsValidTransition(current_state_, new_state);
}

inline bool NodeStateManager::IsValidTransition(NodeInternal::NodeState from_state, 
                                                 NodeInternal::NodeState to_state) {
    // Allow all transitions for now
    // This can be made more strict if needed based on IEEE 802.11 state machine
    // For example, cannot go directly from TX_DATA to RX_DATA
    (void)from_state;  // Suppress unused parameter warning
    (void)to_state;    // Suppress unused parameter warning
    return true;
}

#endif // NODE_STATE_MANAGER_H

