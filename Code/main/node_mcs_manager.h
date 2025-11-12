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
 * @file node_mcs_manager.h
 * @brief Node MCS (Modulation and Coding Scheme) management
 * 
 * This class manages MCS operations for a node, including MCS selection,
 * MCS request/response handling, and MCS per destination tracking.
 * It encapsulates all MCS-related logic that was previously scattered
 * throughout the Node component.
 * 
 * @class NodeMCSManager
 * @brief Manages MCS for a node
 */

#ifndef NODE_MCS_MANAGER_H
#define NODE_MCS_MANAGER_H

#include "../methods/modulations_methods.h"
#include "../list_of_macros.h"

/**
 * @class NodeMCSManager
 * @brief Manages MCS for a node
 * 
 * This class is responsible for:
 * - MCS selection based on received power
 * - MCS per destination tracking
 * - MCS request/response management
 * - MCS change flag management
 */
class NodeMCSManager {
public:
    /**
     * @brief Constructor
     * @param num_stas Number of STAs (for MCS tracking, default: 0, must call Initialize() later)
     */
    NodeMCSManager(int num_stas = 0);
    
    /**
     * @brief Initialize MCS manager with number of STAs
     * @param num_stas Number of STAs (for MCS tracking)
     */
    void InitializeWithSTAs(int num_stas);
    
    /**
     * @brief Destructor
     */
    ~NodeMCSManager();
    
    /**
     * @brief Initialize MCS manager
     */
    void Initialize();
    
    /**
     * @brief Select MCS based on received power
     * @param power_rx_interest Power received [pW]
     * @param mcs_response Output array for MCS response (size 4)
     */
    void SelectMCSBasedOnPower(double power_rx_interest, int* mcs_response) const;
    
    /**
     * @brief Get MCS for a specific destination and number of channels
     * @param destination_index Destination index (relative to STA list)
     * @param num_channels Number of channels (1, 2, 4, or 8)
     * @return MCS value, or -1 if not set
     */
    int GetMCSForDestination(int destination_index, int num_channels) const;
    
    /**
     * @brief Set MCS for a specific destination and number of channels
     * @param destination_index Destination index (relative to STA list)
     * @param num_channels Number of channels (1, 2, 4, or 8)
     * @param mcs MCS value to set
     * @param spatial_reuse_enabled Flag indicating if spatial reuse is enabled
     * @param txop_sr_identified Flag indicating if TXOP SR is identified
     */
    void SetMCSForDestination(int destination_index, int num_channels, int mcs,
                              bool spatial_reuse_enabled, bool txop_sr_identified);
    
    /**
     * @brief Check if MCS change is needed for a destination
     * @param destination_index Destination index (relative to STA list)
     * @return true if MCS change is needed, false otherwise
     */
    bool IsMCSChangeNeeded(int destination_index) const;
    
    /**
     * @brief Set MCS change flag for a destination
     * @param destination_index Destination index (relative to STA list)
     * @param flag true to set flag, false to clear
     */
    void SetMCSChangeFlag(int destination_index, bool flag);
    
    /**
     * @brief Clear MCS change flag for a destination
     * @param destination_index Destination index (relative to STA list)
     */
    void ClearMCSChangeFlag(int destination_index);
    
    /**
     * @brief Get MCS response array
     * @return Pointer to MCS response array (size 4)
     */
    int* GetMCSResponse();
    
    /**
     * @brief Get MCS response array (const version)
     * @return Pointer to MCS response array (size 4)
     */
    const int* GetMCSResponse() const;
    
    /**
     * @brief Get MCS per node array
     * @return Pointer to 2D MCS per node array
     */
    int** GetMCSPerNode();
    
    /**
     * @brief Get MCS per node array (const version)
     * @return Pointer to 2D MCS per node array
     */
    const int* const* GetMCSPerNode() const;
    
    /**
     * @brief Get change modulation flag array
     * @return Pointer to change modulation flag array
     * @note Returns non-const pointer for backward compatibility with legacy code
     */
    int* GetChangeModulationFlagPtr();
    
    /**
     * @brief Check if first time requesting MCS
     * @return true if first time, false otherwise
     */
    bool IsFirstTimeRequestingMCS() const;
    
    /**
     * @brief Set first time requesting MCS flag
     * @param flag true if first time, false otherwise
     */
    void SetFirstTimeRequestingMCS(bool flag);
    
    /**
     * @brief Compute maximum achievable throughput
     * @param destination_index Destination index
     * @param min_channel Minimum channel
     * @param max_channel Maximum channel
     * @return Maximum achievable throughput [Mbps]
     */
    double ComputeMaxAchievableThroughput(int destination_index, int min_channel, int max_channel) const;
    
    /**
     * @brief Reset MCS manager
     */
    void Reset();

private:
    int num_stas_;                  ///< Number of STAs
    int** mcs_per_node_;            ///< MCS per node [num_stas][NUM_OPTIONS_CHANNEL_LENGTH]
    int* change_modulation_flag_;   ///< Flag for changing MCS per destination
    int* mcs_response_;             ///< MCS response array [4]
    bool first_time_requesting_mcs_; ///< Flag indicating first MCS request
    
    /**
     * @brief Initialize MCS arrays
     */
    void InitializeMCSArrays();
    
    /**
     * @brief Clean up MCS arrays
     */
    void CleanupMCSArrays();
    
    /**
     * @brief Convert number of channels to index
     * @param num_channels Number of channels (1, 2, 4, or 8)
     * @return Index in MCS array, or -1 if invalid
     */
    static int NumChannelsToIndex(int num_channels);
};

// Inline implementations

inline NodeMCSManager::NodeMCSManager(int num_stas)
    : num_stas_(num_stas), mcs_per_node_(NULL),
      change_modulation_flag_(NULL), mcs_response_(NULL),
      first_time_requesting_mcs_(TRUE) {
    if (num_stas > 0) {
        InitializeMCSArrays();
    }
}

inline void NodeMCSManager::InitializeWithSTAs(int num_stas) {
    if (num_stas_ > 0) {
        CleanupMCSArrays(); // Clean up if already initialized
    }
    num_stas_ = num_stas;
    if (num_stas_ > 0) {
        InitializeMCSArrays();
        Initialize();
    }
}

inline NodeMCSManager::~NodeMCSManager() {
    CleanupMCSArrays();
}

inline void NodeMCSManager::Initialize() {
    Reset();
}

inline void NodeMCSManager::SelectMCSBasedOnPower(double power_rx_interest, int* mcs_response) const {
    if (mcs_response == NULL) {
        return;
    }
    SelectMCSResponse(mcs_response, power_rx_interest);
}

inline int NodeMCSManager::GetMCSForDestination(int destination_index, int num_channels) const {
    if (destination_index < 0 || destination_index >= num_stas_ || mcs_per_node_ == NULL) {
        return -1;
    }
    
    int index = NumChannelsToIndex(num_channels);
    if (index < 0 || index >= NUM_OPTIONS_CHANNEL_LENGTH) {
        return -1;
    }
    
    return mcs_per_node_[destination_index][index];
}

inline void NodeMCSManager::SetMCSForDestination(int destination_index, int num_channels, int mcs,
                                                  bool spatial_reuse_enabled, bool txop_sr_identified) {
    if (destination_index < 0 || destination_index >= num_stas_ || mcs_per_node_ == NULL) {
        return;
    }
    
    int index = NumChannelsToIndex(num_channels);
    if (index < 0 || index >= NUM_OPTIONS_CHANNEL_LENGTH) {
        return;
    }
    
    // Handle spatial reuse case
    if (spatial_reuse_enabled && txop_sr_identified && mcs == MODULATION_FORBIDDEN) {
        mcs_per_node_[destination_index][index] = MODULATION_BPSK_1_2;
    } else {
        mcs_per_node_[destination_index][index] = mcs;
    }
}

inline bool NodeMCSManager::IsMCSChangeNeeded(int destination_index) const {
    if (destination_index < 0 || destination_index >= num_stas_ || change_modulation_flag_ == NULL) {
        return false;
    }
    return (change_modulation_flag_[destination_index] == TRUE);
}

inline void NodeMCSManager::SetMCSChangeFlag(int destination_index, bool flag) {
    if (destination_index < 0 || destination_index >= num_stas_ || change_modulation_flag_ == NULL) {
        return;
    }
    change_modulation_flag_[destination_index] = flag ? TRUE : FALSE;
}

inline void NodeMCSManager::ClearMCSChangeFlag(int destination_index) {
    SetMCSChangeFlag(destination_index, false);
}

inline int* NodeMCSManager::GetMCSResponse() {
    return mcs_response_;
}

inline const int* NodeMCSManager::GetMCSResponse() const {
    return mcs_response_;
}

inline int** NodeMCSManager::GetMCSPerNode() {
    return mcs_per_node_;
}

inline const int* const* NodeMCSManager::GetMCSPerNode() const {
    return const_cast<const int* const*>(mcs_per_node_);
}

inline int* NodeMCSManager::GetChangeModulationFlagPtr() {
    return change_modulation_flag_;
}

inline bool NodeMCSManager::IsFirstTimeRequestingMCS() const {
    return first_time_requesting_mcs_;
}

inline void NodeMCSManager::SetFirstTimeRequestingMCS(bool flag) {
    first_time_requesting_mcs_ = flag;
}

inline void NodeMCSManager::Reset() {
    if (mcs_per_node_) {
        for (int i = 0; i < num_stas_; ++i) {
            for (int j = 0; j < NUM_OPTIONS_CHANNEL_LENGTH; ++j) {
                mcs_per_node_[i][j] = -1;
            }
        }
    }
    
    if (change_modulation_flag_) {
        for (int i = 0; i < num_stas_; ++i) {
            change_modulation_flag_[i] = TRUE;
        }
    }
    
    if (mcs_response_) {
        for (int i = 0; i < 4; ++i) {
            mcs_response_[i] = 0;
        }
    }
    
    first_time_requesting_mcs_ = TRUE;
}

inline int NodeMCSManager::NumChannelsToIndex(int num_channels) {
    switch (num_channels) {
        case 1: return 0;
        case 2: return 1;
        case 4: return 2;
        case 8: return 3;
        default: return -1;
    }
}

inline void NodeMCSManager::InitializeMCSArrays() {
    // Allocate MCS per node array
    mcs_per_node_ = new int*[num_stas_];
    for (int i = 0; i < num_stas_; ++i) {
        mcs_per_node_[i] = new int[NUM_OPTIONS_CHANNEL_LENGTH];
        for (int j = 0; j < NUM_OPTIONS_CHANNEL_LENGTH; ++j) {
            mcs_per_node_[i][j] = -1;
        }
    }
    
    // Allocate change modulation flag array
    change_modulation_flag_ = new int[num_stas_];
    for (int i = 0; i < num_stas_; ++i) {
        change_modulation_flag_[i] = TRUE;
    }
    
    // Allocate MCS response array
    mcs_response_ = new int[4];
    for (int i = 0; i < 4; ++i) {
        mcs_response_[i] = 0;
    }
}

inline void NodeMCSManager::CleanupMCSArrays() {
    if (mcs_per_node_) {
        for (int i = 0; i < num_stas_; ++i) {
            delete[] mcs_per_node_[i];
        }
        delete[] mcs_per_node_;
        mcs_per_node_ = NULL;
    }
    
    if (change_modulation_flag_) {
        delete[] change_modulation_flag_;
        change_modulation_flag_ = NULL;
    }
    
    if (mcs_response_) {
        delete[] mcs_response_;
        mcs_response_ = NULL;
    }
}

inline double NodeMCSManager::ComputeMaxAchievableThroughput(int destination_index, int min_channel, int max_channel) const {
    if (destination_index < 0 || destination_index >= num_stas_ || mcs_per_node_ == NULL) {
        return 0.0;
    }
    
    int num_channels = max_channel - min_channel + 1;
    int index = NumChannelsToIndex(num_channels);
    if (index < 0 || index >= NUM_OPTIONS_CHANNEL_LENGTH) {
        return 0.0;
    }
    
    int mcs = mcs_per_node_[destination_index][index];
    if (mcs < 0) {
        return 0.0;
    }
    
    // Compute maximum achievable bits per OFDM symbol
    double max_achievable_bits_ofdm_sym = getNumberSubcarriers(num_channels) *
                                          Mcs_array::modulation_bits[mcs - 1] *
                                          Mcs_array::coding_rates[mcs - 1] *
                                          IEEE_AX_SU_SPATIAL_STREAMS;
    
    // Compute maximum achievable throughput
    double max_achievable_throughput = max_achievable_bits_ofdm_sym / IEEE_AX_OFDM_SYMBOL_GI32_DURATION;
    
    return max_achievable_throughput;
}

#endif // NODE_MCS_MANAGER_H

