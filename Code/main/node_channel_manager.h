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
 * @file node_channel_manager.h
 * @brief Node channel management
 * 
 * This class manages channel-related operations for a node, including channel
 * power detection, channel availability checking, channel selection, and
 * channel bonding. It encapsulates all channel-related logic that was
 * previously scattered throughout the Node component.
 * 
 * @class NodeChannelManager
 * @brief Manages channel operations for a node
 */

#ifndef NODE_CHANNEL_MANAGER_H
#define NODE_CHANNEL_MANAGER_H

#include "../structures/notification.h"
#include "../methods/power_channel_methods.h"
#include "../list_of_macros.h"
#include <vector>
#include <utility>

// Forward declarations
class Node;

/**
 * @class NodeChannelManager
 * @brief Manages channel operations for a node
 * 
 * This class is responsible for:
 * - Channel power detection and tracking
 * - Channel availability checking
 * - Channel selection based on bonding policies
 * - Channel state management
 * - Channel power updates
 */
class NodeChannelManager {
public:
    /**
     * @brief Constructor
     * @param num_channels Total number of channels in the system
     */
    NodeChannelManager(int num_channels = NUM_CHANNELS_KOMONDOR);
    
    /**
     * @brief Destructor
     */
    ~NodeChannelManager();
    
    /**
     * @brief Initialize channel manager
     * @param min_channel Minimum allowed channel
     * @param max_channel Maximum allowed channel
     */
    void Initialize(int min_channel, int max_channel);
    
    /**
     * @brief Update channel power for a specific channel
     * @param channel Channel index
     * @param power Power level to add/subtract [pW]
     * @param is_increase true to increase power, false to decrease
     */
    void UpdateChannelPower(int channel, double power, bool is_increase = true);
    
    /**
     * @brief Update channel power based on notification
     * @param notification Notification containing transmission information
     * @param update_type TX_INITIATED or TX_FINISHED
     * @param central_frequency Central frequency
     * @param path_loss_model Path loss model
     * @param adjacent_channel_model Adjacent channel interference model
     * @param pw_received Power received
     * @param node_id Node identifier
     */
    void UpdateChannelsPowerFromNotification(const Notification& notification,
                                             int update_type,
                                             double central_frequency,
                                             int path_loss_model,
                                             int adjacent_channel_model,
                                             double pw_received,
                                             int node_id);
    
    /**
     * @brief Get channel power for a specific channel
     * @param channel Channel index
     * @return Channel power [pW]
     */
    double GetChannelPower(int channel) const;
    
    /**
     * @brief Get all channel powers
     * @return Pointer to channel power array
     */
    const double* GetChannelPowers() const;
    
    /**
     * @brief Get all channel powers (non-const version for legacy code)
     * @return Pointer to channel power array
     * @note This returns a non-const pointer for backward compatibility with legacy functions
     *       that take double** or double*. The manager still owns the memory.
     */
    double* GetChannelPowersPtr();
    
    /**
     * @brief Get channels free array (non-const version for legacy code)
     * @return Pointer to channels free array
     * @note This returns a non-const pointer for backward compatibility with legacy functions.
     *       The manager owns the memory.
     */
    int* GetChannelsFreePtr();
    
    /**
     * @brief Get channels for TX array (non-const version for legacy code)
     * @return Pointer to channels for TX array
     * @note This returns a non-const pointer for backward compatibility with legacy functions.
     *       The manager owns the memory.
     */
    int* GetChannelsForTxPtr();
    
    /**
     * @brief Check if a channel is free
     * @param channel Channel index
     * @param pd_threshold PD threshold [pW]
     * @return true if channel is free, false otherwise
     */
    bool IsChannelFree(int channel, double pd_threshold) const;
    
    /**
     * @brief Get available channels
     * @param pd_threshold PD threshold [pW]
     * @param min_channel Minimum channel to check
     * @param max_channel Maximum channel to check
     * @return Vector of available channel indices
     */
    std::vector<int> GetAvailableChannels(double pd_threshold,
                                          int min_channel,
                                          int max_channel) const;
    
    /**
     * @brief Get transmission channels based on bonding policy
     * @param channels_for_tx Output array indicating which channels to use for TX
     * @param channel_bonding_model Channel bonding model
     * @param min_channel_allowed Minimum allowed channel
     * @param max_channel_allowed Maximum allowed channel
     * @param primary_channel Primary channel
     * @param cca_model CCA model (CHANNEL_AGGREGATION_CCA_SAME or CHANNEL_AGGREGATION_CCA_11AX)
     * @param channels_free Optional array of free channels (if NULL, will be computed internally)
     * @return true if transmission is possible, false otherwise
     */
    bool GetTxChannels(int* channels_for_tx,
                       int channel_bonding_model,
                       int min_channel_allowed,
                       int max_channel_allowed,
                       int primary_channel,
                       int cca_model,
                       const int* channels_free = NULL) const;
    
    /**
     * @brief Reset channel state
     */
    void ResetChannelState();
    
    /**
     * @brief Reset channel power for all channels
     */
    void ResetChannelPowers();
    
    /**
     * @brief Get number of channels used for transmission
     * @param channels_for_tx Array indicating which channels are used
     * @return Number of channels used
     */
    static int GetNumChannelsUsed(const int* channels_for_tx);
    
    /**
     * @brief Get channel range for transmission
     * @param channels_for_tx Array indicating which channels are used
     * @return Pair of (left_channel, right_channel), or (-1, -1) if no channels
     */
    static std::pair<int, int> GetChannelRange(const int* channels_for_tx);
    
    /**
     * @brief Check if transmission is possible
     * @param channels_for_tx Array indicating which channels are used
     * @return true if transmission is possible, false otherwise
     */
    static bool IsTransmissionPossible(const int* channels_for_tx);

private:
    double* channel_power_;      ///< Channel power detected in each channel [pW]
    int* channels_free_;         ///< Channels that are found free for TX
    int* channels_for_tx_;       ///< Channels that are used for TX
    int num_channels_;           ///< Total number of channels
    int min_channel_allowed_;    ///< Minimum allowed channel
    int max_channel_allowed_;    ///< Maximum allowed channel
    
    /**
     * @brief Initialize channel arrays
     */
    void InitializeChannelArrays();
    
    /**
     * @brief Clean up channel arrays
     */
    void CleanupChannelArrays();
};

// Inline implementations

inline NodeChannelManager::NodeChannelManager(int num_channels)
    : channel_power_(NULL), channels_free_(NULL), channels_for_tx_(NULL),
      num_channels_(num_channels), min_channel_allowed_(0), max_channel_allowed_(0) {
    InitializeChannelArrays();
}

inline NodeChannelManager::~NodeChannelManager() {
    // Cleanup arrays - this will free channel_power_ which Node's channel_power points to.
    // IMPORTANT: Node's channel_power will become a dangling pointer after this.
    // Node must set channel_power = NULL before the manager is destroyed, or
    // Node must never free channel_power since the manager owns the memory.
    // The manager cannot set Node's channel_power to NULL from here.
    CleanupChannelArrays();
}

inline void NodeChannelManager::Initialize(int min_channel, int max_channel) {
    min_channel_allowed_ = min_channel;
    max_channel_allowed_ = max_channel;
    ResetChannelState();
}

inline void NodeChannelManager::UpdateChannelPower(int channel, double power, bool is_increase) {
    if (channel < 0 || channel >= num_channels_) {
        return;
    }
    
    if (is_increase) {
        channel_power_[channel] += power;
    } else {
        channel_power_[channel] -= power;
        // Avoid near-zero negative values
        if (channel_power_[channel] < 0.000001) {
            channel_power_[channel] = 0;
        }
    }
}

inline double NodeChannelManager::GetChannelPower(int channel) const {
    if (channel < 0 || channel >= num_channels_) {
        return 0.0;
    }
    return channel_power_[channel];
}

inline const double* NodeChannelManager::GetChannelPowers() const {
    return channel_power_;
}

inline double* NodeChannelManager::GetChannelPowersPtr() {
    return channel_power_;
}

inline int* NodeChannelManager::GetChannelsFreePtr() {
    return channels_free_;
}

inline int* NodeChannelManager::GetChannelsForTxPtr() {
    return channels_for_tx_;
}

inline bool NodeChannelManager::IsChannelFree(int channel, double pd_threshold) const {
    if (channel < 0 || channel >= num_channels_) {
        return false;
    }
    return (channel_power_[channel] < pd_threshold);
}

inline void NodeChannelManager::ResetChannelState() {
    ResetChannelPowers();
    if (channels_free_) {
        for (int i = 0; i < num_channels_; ++i) {
            channels_free_[i] = FALSE;
        }
    }
    if (channels_for_tx_) {
        for (int i = 0; i < num_channels_; ++i) {
            channels_for_tx_[i] = FALSE;
        }
    }
}

inline void NodeChannelManager::ResetChannelPowers() {
    if (channel_power_) {
        for (int i = 0; i < num_channels_; ++i) {
            channel_power_[i] = 0.0;
        }
    }
}

inline int NodeChannelManager::GetNumChannelsUsed(const int* channels_for_tx) {
    if (channels_for_tx == NULL) {
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < NUM_CHANNELS_KOMONDOR; ++i) {
        if (channels_for_tx[i] == TRUE) {
            count++;
        }
    }
    return count;
}

inline std::pair<int, int> NodeChannelManager::GetChannelRange(const int* channels_for_tx) {
    if (channels_for_tx == NULL) {
        return std::make_pair(-1, -1);
    }
    
    int left_channel = -1;
    int right_channel = -1;
    
    for (int i = 0; i < NUM_CHANNELS_KOMONDOR; ++i) {
        if (channels_for_tx[i] == TRUE) {
            if (left_channel == -1) {
                left_channel = i;
            }
            right_channel = i;
        }
    }
    
    return std::make_pair(left_channel, right_channel);
}

inline bool NodeChannelManager::IsTransmissionPossible(const int* channels_for_tx) {
    if (channels_for_tx == NULL) {
        return false;
    }
    return (channels_for_tx[0] != TX_NOT_POSSIBLE);
}

inline void NodeChannelManager::InitializeChannelArrays() {
    channel_power_ = new double[num_channels_];
    channels_free_ = new int[num_channels_];
    channels_for_tx_ = new int[num_channels_];
    
    ResetChannelState();
}

inline void NodeChannelManager::CleanupChannelArrays() {
    // Free the arrays owned by the manager
    // Check for NULL before deleting to prevent double-free
    if (channel_power_) {
        delete[] channel_power_;
        channel_power_ = NULL;
    }
    if (channels_free_) {
        delete[] channels_free_;
        channels_free_ = NULL;
    }
    if (channels_for_tx_) {
        delete[] channels_for_tx_;
        channels_for_tx_ = NULL;
    }
}

inline std::vector<int> NodeChannelManager::GetAvailableChannels(double pd_threshold,
                                                                  int min_channel,
                                                                  int max_channel) const {
    std::vector<int> available_channels;
    
    for (int channel = min_channel; channel <= max_channel; ++channel) {
        if (IsChannelFree(channel, pd_threshold)) {
            available_channels.push_back(channel);
        }
    }
    
    return available_channels;
}

inline bool NodeChannelManager::GetTxChannels(int* channels_for_tx,
                                               int channel_bonding_model,
                                               int min_channel_allowed,
                                               int max_channel_allowed,
                                               int primary_channel,
                                               int cca_model,
                                               const int* channels_free) const {
    if (channels_for_tx == NULL) {
        return false;
    }
    
    // Reset channels for transmitting
    for (int c = 0; c < num_channels_; ++c) {
        channels_for_tx[c] = FALSE;
    }
    
    // Use the existing method from power_channel_methods.h
    // This delegates to the existing implementation
    double* channel_power_ptr = const_cast<double*>(channel_power_);
    
    if (cca_model == CHANNEL_AGGREGATION_CCA_11AX) {
        GetTxChannelsByChannelBondingCCA11ax(channels_for_tx, channel_bonding_model,
                                             min_channel_allowed, max_channel_allowed,
                                             primary_channel, &channel_power_ptr);
    } else {
        // For CHANNEL_AGGREGATION_CCA_SAME, use provided channels_free if available,
        // otherwise determine free channels internally
        const int* channels_free_to_use = channels_free;
        if (channels_free_to_use == NULL) {
            // Compute free channels internally with default threshold
            for (int c = min_channel_allowed; c <= max_channel_allowed; ++c) {
                channels_free_[c] = (channel_power_[c] < ConvertPower(DBM_TO_PW, -82)) ? TRUE : FALSE;
            }
            channels_free_to_use = channels_free_;
        }
        GetTxChannelsByChannelBondingCCASame(channels_for_tx, channel_bonding_model,
                                             const_cast<int*>(channels_free_to_use), min_channel_allowed,
                                             max_channel_allowed, primary_channel,
                                             NUM_CHANNELS_KOMONDOR);
    }
    
    return IsTransmissionPossible(channels_for_tx);
}

inline void NodeChannelManager::UpdateChannelsPowerFromNotification(const Notification& notification,
                                                                     int update_type,
                                                                     double central_frequency,
                                                                     int path_loss_model,
                                                                     int adjacent_channel_model,
                                                                     double pw_received,
                                                                     int node_id) {
    double* channel_power_ptr = channel_power_;
    UpdateChannelsPower(&channel_power_ptr, notification, update_type,
                        central_frequency, path_loss_model,
                        adjacent_channel_model, pw_received, node_id);
}

#endif // NODE_CHANNEL_MANAGER_H

