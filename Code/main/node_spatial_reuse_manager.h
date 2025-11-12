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
 * @file node_spatial_reuse_manager.h
 * @brief Node spatial reuse management
 * 
 * This class manages spatial reuse operations for a node, including BSS color
 * management, SRG (Spatial Reuse Group) handling, OBSS-PD threshold management,
 * and TXOP identification. It encapsulates all spatial reuse-related logic
 * that was previously scattered throughout the Node component.
 * 
 * @class NodeSpatialReuseManager
 * @brief Manages spatial reuse operations for a node
 */

#ifndef NODE_SPATIAL_REUSE_MANAGER_H
#define NODE_SPATIAL_REUSE_MANAGER_H

#include "../structures/notification.h"
#include "../methods/spatial_reuse_methods.h"
#include "../list_of_macros.h"

/**
 * @class NodeSpatialReuseManager
 * @brief Manages spatial reuse operations for a node
 * 
 * This class is responsible for:
 * - BSS color and SRG management
 * - OBSS-PD threshold computation
 * - Packet origin detection (intra-BSS, inter-BSS, SRG, non-SRG)
 * - TXOP SR identification
 * - Spatial reuse opportunity management
 */
class NodeSpatialReuseManager {
public:
    /**
     * @brief Constructor
     */
    NodeSpatialReuseManager();
    
    /**
     * @brief Destructor
     */
    ~NodeSpatialReuseManager();
    
    /**
     * @brief Initialize spatial reuse manager
     * @param bss_color BSS color (-1 if disabled)
     * @param srg Spatial Reuse Group (-1 if disabled)
     * @param non_srg_obss_pd Non-SRG OBSS-PD threshold [pW]
     * @param srg_obss_pd SRG OBSS-PD threshold [pW]
     * @param default_pd Default PD threshold [pW]
     */
    void Initialize(int bss_color, int srg,
                    double non_srg_obss_pd, double srg_obss_pd,
                    double default_pd);
    
    /**
     * @brief Check if spatial reuse is enabled
     * @return true if spatial reuse is enabled, false otherwise
     */
    bool IsSpatialReuseEnabled() const;
    
    /**
     * @brief Get BSS color
     * @return BSS color, or -1 if disabled
     */
    int GetBSSColor() const;
    
    /**
     * @brief Get SRG
     * @return SRG, or -1 if disabled
     */
    int GetSRG() const;
    
    /**
     * @brief Get non-SRG OBSS-PD threshold
     * @return Non-SRG OBSS-PD threshold [pW]
     */
    double GetNonSRGOBSSPD() const;
    
    /**
     * @brief Get SRG OBSS-PD threshold
     * @return SRG OBSS-PD threshold [pW]
     */
    double GetSRGOBSSPD() const;
    
    /**
     * @brief Check packet origin
     * @param notification Notification to check
     * @return Packet type: INTRA_BSS_FRAME, SRG_FRAME, or NON_SRG_FRAME
     * @note This method is not const because it updates internal state
     */
    int CheckPacketOrigin(const Notification& notification);
    
    /**
     * @brief Compute potential OBSS-PD threshold based on packet origin
     * @param notification Notification containing packet information
     * @param power_received Power received from source [pW]
     * @return Potential OBSS-PD threshold [pW]
     * @note This method is not const because it updates internal state
     */
    double ComputePotentialOBSSPDThreshold(const Notification& notification,
                                           double power_received);
    
    /**
     * @brief Get sensitivity for spatial reuse based on packet type
     * @param packet_type Packet type (from CheckPacketOrigin)
     * @param power_received Power received [pW]
     * @return PD threshold to use [pW]
     */
    double GetSensitivitySpatialReuse(int packet_type, double power_received) const;
    
    /**
     * @brief Check if TXOP SR is identified
     * @return true if TXOP SR is identified, false otherwise
     */
    bool IsTXOPSRIdentified() const;
    
    /**
     * @brief Set TXOP SR identified flag
     * @param identified true if identified, false otherwise
     */
    void SetTXOPSRIdentified(bool identified);
    
    /**
     * @brief Get current OBSS-PD threshold
     * @return Current OBSS-PD threshold [pW]
     */
    double GetCurrentOBSSPDThreshold() const;
    
    /**
     * @brief Set current OBSS-PD threshold
     * @param threshold OBSS-PD threshold [pW]
     */
    void SetCurrentOBSSPDThreshold(double threshold);
    
    /**
     * @brief Get next PD for spatial reuse
     * @return Next PD for spatial reuse [pW]
     */
    double GetNextPDSpatialReuse() const;
    
    /**
     * @brief Set next PD for spatial reuse
     * @param pd Next PD for spatial reuse [pW]
     */
    void SetNextPDSpatialReuse(double pd);
    
    /**
     * @brief Get current TX power for spatial reuse
     * @return Current TX power for spatial reuse [pW]
     */
    double GetCurrentTXPowerSR() const;
    
    /**
     * @brief Set current TX power for spatial reuse
     * @param tx_power TX power for spatial reuse [pW]
     */
    void SetCurrentTXPowerSR(double tx_power);
    
    /**
     * @brief Handle spatial reuse opportunity end
     * @param default_pd Default PD threshold [pW]
     * @param default_tx_power Default TX power [pW]
     */
    void HandleSpatialReuseOpportunityEnd(double default_pd, double default_tx_power);
    
    /**
     * @brief Reset spatial reuse manager
     */
    void Reset();
    
    /**
     * @brief Get flag indicating change in TX power
     * @return true if TX power change flag is set, false otherwise
     */
    bool IsTXPowerChangeFlagSet() const;
    
    /**
     * @brief Set flag indicating change in TX power
     * @param flag true to set flag, false to clear
     */
    void SetTXPowerChangeFlag(bool flag);

private:
    int bss_color_;                      ///< BSS color (-1 if disabled)
    int srg_;                            ///< Spatial Reuse Group (-1 if disabled)
    double non_srg_obss_pd_;             ///< Non-SRG OBSS-PD threshold [pW]
    double srg_obss_pd_;                 ///< SRG OBSS-PD threshold [pW]
    double default_pd_;                  ///< Default PD threshold [pW]
    bool spatial_reuse_enabled_;         ///< Flag indicating if spatial reuse is enabled
    bool txop_sr_identified_;            ///< Flag indicating if TXOP SR is identified
    double current_obss_pd_threshold_;   ///< Current OBSS-PD threshold [pW]
    double potential_obss_pd_threshold_; ///< Potential OBSS-PD threshold [pW]
    double next_pd_spatial_reuse_;       ///< Next PD for spatial reuse [pW]
    double current_tx_power_sr_;         ///< Current TX power for spatial reuse [pW]
    bool flag_change_in_tx_power_;       ///< Flag indicating change in TX power
    int type_last_sensed_packet_;        ///< Type of last sensed packet
};

// Inline implementations

inline NodeSpatialReuseManager::NodeSpatialReuseManager()
    : bss_color_(-1), srg_(-1),
      non_srg_obss_pd_(0.0), srg_obss_pd_(0.0), default_pd_(0.0),
      spatial_reuse_enabled_(false), txop_sr_identified_(false),
      current_obss_pd_threshold_(0.0), potential_obss_pd_threshold_(0.0),
      next_pd_spatial_reuse_(0.0), current_tx_power_sr_(0.0),
      flag_change_in_tx_power_(false), type_last_sensed_packet_(INTRA_BSS_FRAME) {
}

inline NodeSpatialReuseManager::~NodeSpatialReuseManager() {
}

inline void NodeSpatialReuseManager::Initialize(int bss_color, int srg,
                                                 double non_srg_obss_pd, double srg_obss_pd,
                                                 double default_pd) {
    bss_color_ = bss_color;
    srg_ = srg;
    non_srg_obss_pd_ = non_srg_obss_pd;
    srg_obss_pd_ = srg_obss_pd;
    default_pd_ = default_pd;
    spatial_reuse_enabled_ = (bss_color >= 0);
    
    Reset();
}

inline bool NodeSpatialReuseManager::IsSpatialReuseEnabled() const {
    return spatial_reuse_enabled_;
}

inline int NodeSpatialReuseManager::GetBSSColor() const {
    return bss_color_;
}

inline int NodeSpatialReuseManager::GetSRG() const {
    return srg_;
}

inline double NodeSpatialReuseManager::GetNonSRGOBSSPD() const {
    return non_srg_obss_pd_;
}

inline double NodeSpatialReuseManager::GetSRGOBSSPD() const {
    return srg_obss_pd_;
}

inline int NodeSpatialReuseManager::CheckPacketOrigin(const Notification& notification) {
    if (!spatial_reuse_enabled_) {
        type_last_sensed_packet_ = INTRA_BSS_FRAME;
        return INTRA_BSS_FRAME;
    }
    
    type_last_sensed_packet_ = ::CheckPacketOrigin(notification, bss_color_, srg_);
    return type_last_sensed_packet_;
}

inline double NodeSpatialReuseManager::ComputePotentialOBSSPDThreshold(const Notification& notification,
                                                                        double power_received) {
    if (!spatial_reuse_enabled_) {
        return default_pd_;
    }
    
    int packet_type = CheckPacketOrigin(notification);
    return GetSensitivitySpatialReuse(packet_type, power_received);
}

inline double NodeSpatialReuseManager::GetSensitivitySpatialReuse(int packet_type, double power_received) const {
    return ::GetSensitivitySpatialReuse(packet_type, srg_obss_pd_, non_srg_obss_pd_,
                                        default_pd_, power_received);
}

inline bool NodeSpatialReuseManager::IsTXOPSRIdentified() const {
    return txop_sr_identified_;
}

inline void NodeSpatialReuseManager::SetTXOPSRIdentified(bool identified) {
    txop_sr_identified_ = identified;
}

inline double NodeSpatialReuseManager::GetCurrentOBSSPDThreshold() const {
    return current_obss_pd_threshold_;
}

inline void NodeSpatialReuseManager::SetCurrentOBSSPDThreshold(double threshold) {
    current_obss_pd_threshold_ = threshold;
}

inline double NodeSpatialReuseManager::GetNextPDSpatialReuse() const {
    return next_pd_spatial_reuse_;
}

inline void NodeSpatialReuseManager::SetNextPDSpatialReuse(double pd) {
    next_pd_spatial_reuse_ = pd;
}

inline double NodeSpatialReuseManager::GetCurrentTXPowerSR() const {
    return current_tx_power_sr_;
}

inline void NodeSpatialReuseManager::SetCurrentTXPowerSR(double tx_power) {
    current_tx_power_sr_ = tx_power;
}

inline void NodeSpatialReuseManager::HandleSpatialReuseOpportunityEnd(double default_pd, double default_tx_power) {
    // Reset SR parameters to default values
    current_obss_pd_threshold_ = default_pd;
    txop_sr_identified_ = false;
    current_tx_power_sr_ = default_tx_power;
    flag_change_in_tx_power_ = false;
    potential_obss_pd_threshold_ = default_pd;
    next_pd_spatial_reuse_ = default_pd;
}

inline void NodeSpatialReuseManager::Reset() {
    txop_sr_identified_ = false;
    current_obss_pd_threshold_ = default_pd_;
    potential_obss_pd_threshold_ = default_pd_;
    next_pd_spatial_reuse_ = default_pd_;
    current_tx_power_sr_ = 0.0;
    flag_change_in_tx_power_ = false;
    type_last_sensed_packet_ = INTRA_BSS_FRAME;
}

inline bool NodeSpatialReuseManager::IsTXPowerChangeFlagSet() const {
    return flag_change_in_tx_power_;
}

inline void NodeSpatialReuseManager::SetTXPowerChangeFlag(bool flag) {
    flag_change_in_tx_power_ = flag;
}

#endif // NODE_SPATIAL_REUSE_MANAGER_H

