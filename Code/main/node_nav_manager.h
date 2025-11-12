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
 * @file node_nav_manager.h
 * @brief Node NAV (Network Allocation Vector) management
 * 
 * This class manages NAV operations for a node, including NAV state tracking,
 * NAV timeout handling, and inter-BSS NAV management. It encapsulates all
 * NAV-related logic that was previously scattered throughout the Node component.
 * 
 * @class NodeNavManager
 * @brief Manages NAV for a node
 */

#ifndef NODE_NAV_MANAGER_H
#define NODE_NAV_MANAGER_H

#include "../structures/notification.h"
#include "../list_of_macros.h"

/**
 * @class NodeNavManager
 * @brief Manages NAV (Network Allocation Vector) for a node
 * 
 * This class is responsible for:
 * - NAV state management
 * - NAV timeout tracking
 * - Inter-BSS NAV management
 * - NAV collision detection
 * - NAV duration management
 */
class NodeNavManager {
public:
    /**
     * @brief Constructor
     */
    NodeNavManager();
    
    /**
     * @brief Destructor
     */
    ~NodeNavManager();
    
    /**
     * @brief Set NAV with specified duration
     * @param duration NAV duration [s]
     * @param is_inter_bss true if this is an inter-BSS NAV, false otherwise
     * @param current_time Current simulation time [s]
     */
    void SetNAV(double duration, bool is_inter_bss, double current_time);
    
    /**
     * @brief Clear NAV
     * @param current_time Current simulation time [s]
     */
    void ClearNAV(double current_time);
    
    /**
     * @brief Check if node is in NAV state
     * @param current_time Current simulation time [s]
     * @return true if in NAV, false otherwise
     */
    bool IsInNAV(double current_time) const;
    
    /**
     * @brief Get remaining NAV time
     * @param current_time Current simulation time [s]
     * @return Remaining NAV time [s], or 0 if NAV is not active
     */
    double GetRemainingNAVTime(double current_time) const;
    
    /**
     * @brief Get current NAV duration
     * @return Current NAV duration [s]
     */
    double GetCurrentNAVDuration() const;
    
    /**
     * @brief Update NAV based on notification
     * @param notification Notification containing NAV information
     * @param is_inter_bss true if this is an inter-BSS NAV, false otherwise
     * @param current_time Current simulation time [s]
     */
    void UpdateNAV(const Notification& notification, bool is_inter_bss, double current_time);
    
    /**
     * @brief Check if NAV timeout has occurred
     * @param current_time Current simulation time [s]
     * @return true if NAV has timed out, false otherwise
     */
    bool IsNAVTimeout(double current_time) const;
    
    /**
     * @brief Get total time spent in NAV
     * @return Total time in NAV [s]
     */
    double GetTotalTimeInNAV() const;
    
    /**
     * @brief Get number of times node entered NAV
     * @return Number of NAV entries
     */
    int GetTimesWentToNAV() const;
    
    /**
     * @brief Reset NAV manager
     */
    void Reset();
    
    /**
     * @brief Check if inter-BSS NAV is active
     * @param current_time Current simulation time [s]
     * @return true if inter-BSS NAV is active, false otherwise
     */
    bool IsInterBSSNAVActive(double current_time) const;
    
    /**
     * @brief Get NAV notification (last notification that set NAV)
     * @return Last NAV notification
     */
    const Notification& GetNAVNotification() const;
    
    /**
     * @brief Set NAV notification
     * @param notification Notification that sets NAV
     */
    void SetNAVNotification(const Notification& notification);

private:
    double current_nav_time_;      ///< Current NAV duration [s]
    double nav_timeout_time_;      ///< Time when NAV will timeout [s]
    bool is_in_nav_;               ///< Flag indicating if node is in NAV
    bool is_inter_bss_nav_;        ///< Flag indicating if this is inter-BSS NAV
    double last_time_not_in_nav_;  ///< Last time node was not in NAV [s]
    double time_in_nav_;           ///< Total time spent in NAV [s]
    int times_went_to_nav_;        ///< Number of times node entered NAV
    Notification nav_notification_; ///< Last notification that set NAV
    
    /**
     * @brief Update NAV statistics
     * @param current_time Current simulation time [s]
     * @param entering_nav true if entering NAV, false if leaving NAV
     */
    void UpdateNAVStatistics(double current_time, bool entering_nav);
};

// Inline implementations

inline NodeNavManager::NodeNavManager()
    : current_nav_time_(0.0), nav_timeout_time_(0.0),
      is_in_nav_(false), is_inter_bss_nav_(false),
      last_time_not_in_nav_(0.0), time_in_nav_(0.0),
      times_went_to_nav_(0) {
    // Initialize nav_notification_ to a null notification
    nav_notification_.packet_id = -1;
    nav_notification_.packet_type = -1;
    nav_notification_.source_id = -1;
    nav_notification_.destination_id = -1;
}

inline NodeNavManager::~NodeNavManager() {
}

inline void NodeNavManager::SetNAV(double duration, bool is_inter_bss, double current_time) {
    if (is_in_nav_) {
        // Update existing NAV
        UpdateNAVStatistics(current_time, false); // Leaving old NAV
    }
    
    current_nav_time_ = duration;
    nav_timeout_time_ = current_time + duration;
    is_in_nav_ = true;
    is_inter_bss_nav_ = is_inter_bss;
    
    UpdateNAVStatistics(current_time, true); // Entering new NAV
}

inline void NodeNavManager::ClearNAV(double current_time) {
    if (is_in_nav_) {
        UpdateNAVStatistics(current_time, false); // Leaving NAV
    }
    
    current_nav_time_ = 0.0;
    nav_timeout_time_ = 0.0;
    is_in_nav_ = false;
    is_inter_bss_nav_ = false;
}

inline bool NodeNavManager::IsInNAV(double current_time) const {
    if (!is_in_nav_) {
        return false;
    }
    
    // Check if NAV has timed out
    if (current_time >= nav_timeout_time_) {
        return false;
    }
    
    return true;
}

inline double NodeNavManager::GetRemainingNAVTime(double current_time) const {
    if (!IsInNAV(current_time)) {
        return 0.0;
    }
    
    return nav_timeout_time_ - current_time;
}

inline double NodeNavManager::GetCurrentNAVDuration() const {
    return current_nav_time_;
}

inline void NodeNavManager::UpdateNAV(const Notification& notification, bool is_inter_bss, double current_time) {
    double nav_duration = notification.tx_info.nav_time;
    SetNAV(nav_duration, is_inter_bss, current_time);
    SetNAVNotification(notification);
}

inline bool NodeNavManager::IsNAVTimeout(double current_time) const {
    return (is_in_nav_ && current_time >= nav_timeout_time_);
}

inline double NodeNavManager::GetTotalTimeInNAV() const {
    return time_in_nav_;
}

inline int NodeNavManager::GetTimesWentToNAV() const {
    return times_went_to_nav_;
}

inline void NodeNavManager::Reset() {
    current_nav_time_ = 0.0;
    nav_timeout_time_ = 0.0;
    is_in_nav_ = false;
    is_inter_bss_nav_ = false;
    last_time_not_in_nav_ = 0.0;
    time_in_nav_ = 0.0;
    times_went_to_nav_ = 0;
    
    // Reset notification
    nav_notification_.packet_id = -1;
    nav_notification_.packet_type = -1;
    nav_notification_.source_id = -1;
    nav_notification_.destination_id = -1;
}

inline bool NodeNavManager::IsInterBSSNAVActive(double current_time) const {
    return (is_inter_bss_nav_ && IsInNAV(current_time));
}

inline const Notification& NodeNavManager::GetNAVNotification() const {
    return nav_notification_;
}

inline void NodeNavManager::SetNAVNotification(const Notification& notification) {
    nav_notification_ = notification;
}

inline void NodeNavManager::UpdateNAVStatistics(double current_time, bool entering_nav) {
    if (entering_nav) {
        last_time_not_in_nav_ = current_time;
        times_went_to_nav_++;
    } else {
        if (last_time_not_in_nav_ > 0.0) {
            time_in_nav_ += (current_time - last_time_not_in_nav_);
        }
        last_time_not_in_nav_ = current_time;
    }
}

#endif // NODE_NAV_MANAGER_H

