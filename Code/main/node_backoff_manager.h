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
 * @file node_backoff_manager.h
 * @brief Node backoff management
 * 
 * This class manages backoff procedures for a node, including backoff computation,
 * contention window management, and backoff state tracking. It encapsulates all
 * backoff-related logic that was previously scattered throughout the Node component.
 * 
 * @class NodeBackoffManager
 * @brief Manages backoff procedures for a node
 */

#ifndef NODE_BACKOFF_MANAGER_H
#define NODE_BACKOFF_MANAGER_H

#include "../methods/backoff_methods.h"
#include "../list_of_macros.h"
#include <cmath>

/**
 * @class NodeBackoffManager
 * @brief Manages backoff procedures for a node
 * 
 * This class is responsible for:
 * - Backoff computation
 * - Contention window management
 * - Backoff state tracking (pause/resume)
 * - Backoff interruption counting
 * - Backoff value management
 */
class NodeBackoffManager {
public:
    /**
     * @brief Constructor
     */
    NodeBackoffManager();
    
    /**
     * @brief Destructor
     */
    ~NodeBackoffManager();
    
    /**
     * @brief Initialize backoff manager
     * @param pdf_backoff PDF type for backoff (PDF_UNIFORM or PDF_EXPONENTIAL)
     * @param backoff_type Type of backoff (BACKOFF_CUSTOM, etc.)
     * @param cw_min_default Default minimum contention window
     * @param cw_max_default Default maximum contention window
     * @param cw_stage_max Maximum contention window stage
     * @param cw_adaptation CW adaptation type
     */
    void Initialize(int pdf_backoff, int backoff_type,
                    int cw_min_default, int cw_max_default,
                    int cw_stage_max, int cw_adaptation);
    
    /**
     * @brief Compute a new backoff value
     * @param traffic_type Traffic type for EDCA
     * @return Computed backoff value [s]
     */
    double ComputeNewBackoff(int traffic_type = -1);
    
    /**
     * @brief Compute remaining backoff from time remaining
     * @param time_remaining Time remaining until backoff ends [s]
     * @return Remaining backoff value [s]
     */
    double ComputeRemainingBackoffFromTime(double time_remaining) const;
    
    /**
     * @brief Set remaining backoff
     * @param remaining_backoff Remaining backoff value [s]
     */
    void SetRemainingBackoff(double remaining_backoff);
    
    /**
     * @brief Get remaining backoff
     * @return Remaining backoff value [s]
     */
    double GetRemainingBackoff() const;
    
    /**
     * @brief Pause backoff (freeze current backoff value)
     * @param time_remaining Time remaining until backoff would end [s]
     * @return true if backoff was paused, false if already paused
     */
    bool PauseBackoff(double time_remaining);
    
    /**
     * @brief Resume backoff (unfreeze and continue)
     * @return Remaining backoff value to resume with [s]
     */
    double ResumeBackoff();
    
    /**
     * @brief Check if backoff is paused
     * @return true if backoff is paused, false otherwise
     */
    bool IsBackoffPaused() const;
    
    /**
     * @brief Check if backoff is active
     * @param is_difs_active Flag indicating if DIFS trigger is active
     * @param is_backoff_active Flag indicating if backoff trigger is active
     * @return true if backoff is active, false otherwise
     */
    bool IsBackoffActive(bool is_difs_active, bool is_backoff_active) const;
    
    /**
     * @brief End backoff (backoff completed)
     */
    void EndBackoff();
    
    /**
     * @brief Update contention window based on transmission result
     * @param success true if transmission was successful, false otherwise
     */
    void UpdateContentionWindow(bool success);
    
    /**
     * @brief Reset contention window to default values
     */
    void ResetContentionWindow();
    
    /**
     * @brief Get current minimum contention window
     * @return Current CW min
     */
    int GetCurrentCWMin() const;
    
    /**
     * @brief Get current maximum contention window
     * @return Current CW max
     */
    int GetCurrentCWMax() const;
    
    /**
     * @brief Get current contention window stage
     * @return Current CW stage
     */
    int GetCurrentCWStage() const;
    
    /**
     * @brief Get number of backoff interruptions
     * @return Number of interruptions
     */
    int GetNumBackoffInterruptions() const;
    
    /**
     * @brief Increment backoff interruptions counter
     */
    void IncrementBackoffInterruptions();
    
    /**
     * @brief Get previous backoff value
     * @return Previous backoff value [s]
     */
    double GetPreviousBackoff() const;
    
    /**
     * @brief Set previous backoff value
     * @param previous_backoff Previous backoff value [s]
     */
    void SetPreviousBackoff(double previous_backoff);
    
    /**
     * @brief Reset backoff manager to initial state
     */
    void Reset();

private:
    // Backoff configuration
    int pdf_backoff_;              ///< PDF type for backoff
    int backoff_type_;             ///< Type of backoff
    int cw_min_default_;           ///< Default minimum contention window
    int cw_max_default_;           ///< Default maximum contention window
    int cw_stage_max_;             ///< Maximum contention window stage
    int cw_adaptation_;            ///< CW adaptation type
    
    // Current backoff state
    double remaining_backoff_;     ///< Remaining backoff value [s]
    double previous_backoff_;      ///< Previous backoff value [s]
    bool is_paused_;               ///< Flag indicating if backoff is paused
    
    // Contention window state
    int cw_stage_current_;         ///< Current contention window stage
    int current_cw_min_;           ///< Current minimum contention window
    int current_cw_max_;           ///< Current maximum contention window
    
    // Deterministic backoff
    int num_bo_interruptions_;     ///< Number of backoff interruptions
    int base_backoff_deterministic_; ///< Base backoff for deterministic backoff
    int deterministic_bo_active_;  ///< Flag indicating if deterministic BO is active
    
    /**
     * @brief Compute backoff using the configured parameters
     * @param traffic_type Traffic type for EDCA
     * @return Computed backoff value [s]
     */
    double ComputeBackoffInternal(int traffic_type) const;
};

// Inline implementations

inline NodeBackoffManager::NodeBackoffManager()
    : pdf_backoff_(PDF_UNIFORM), backoff_type_(BACKOFF_CUSTOM),
      cw_min_default_(0), cw_max_default_(0), cw_stage_max_(0), cw_adaptation_(0),
      remaining_backoff_(0.0), previous_backoff_(0.0), is_paused_(false),
      cw_stage_current_(0), current_cw_min_(0), current_cw_max_(0),
      num_bo_interruptions_(0), base_backoff_deterministic_(0), deterministic_bo_active_(0) {
}

inline NodeBackoffManager::~NodeBackoffManager() {
}

inline void NodeBackoffManager::Initialize(int pdf_backoff, int backoff_type,
                                           int cw_min_default, int cw_max_default,
                                           int cw_stage_max, int cw_adaptation) {
    pdf_backoff_ = pdf_backoff;
    backoff_type_ = backoff_type;
    cw_min_default_ = cw_min_default;
    cw_max_default_ = cw_max_default;
    cw_stage_max_ = cw_stage_max;
    cw_adaptation_ = cw_adaptation;
    
    ResetContentionWindow();
    Reset();
}

inline double NodeBackoffManager::ComputeNewBackoff(int traffic_type) {
    double backoff = ComputeBackoffInternal(traffic_type);
    previous_backoff_ = backoff;
    remaining_backoff_ = backoff;
    is_paused_ = false;
    return backoff;
}

inline double NodeBackoffManager::ComputeRemainingBackoffFromTime(double time_remaining) const {
    return ComputeRemainingBackoff(backoff_type_, time_remaining);
}

inline void NodeBackoffManager::SetRemainingBackoff(double remaining_backoff) {
    remaining_backoff_ = remaining_backoff;
}

inline double NodeBackoffManager::GetRemainingBackoff() const {
    return remaining_backoff_;
}

inline bool NodeBackoffManager::PauseBackoff(double time_remaining) {
    if (is_paused_) {
        return false; // Already paused
    }
    
    remaining_backoff_ = ComputeRemainingBackoffFromTime(time_remaining);
    is_paused_ = true;
    IncrementBackoffInterruptions();
    return true;
}

inline double NodeBackoffManager::ResumeBackoff() {
    is_paused_ = false;
    return remaining_backoff_;
}

inline bool NodeBackoffManager::IsBackoffPaused() const {
    return is_paused_;
}

inline bool NodeBackoffManager::IsBackoffActive(bool is_difs_active, bool is_backoff_active) const {
    return (is_difs_active || is_backoff_active);
}

inline void NodeBackoffManager::EndBackoff() {
    remaining_backoff_ = 0.0;
    is_paused_ = false;
}

inline void NodeBackoffManager::UpdateContentionWindow(bool success) {
    if (cw_adaptation_ == TRUE) {
        if (success) {
            // Reset to minimum on success
            cw_stage_current_ = 0;
            current_cw_min_ = cw_min_default_;
            current_cw_max_ = cw_max_default_;
        } else {
            // Increase on failure
            if (cw_stage_current_ < cw_stage_max_) {
                cw_stage_current_++;
                current_cw_min_ = cw_min_default_ * (int)pow(2, cw_stage_current_);
                current_cw_max_ = cw_max_default_ * (int)pow(2, cw_stage_current_);
            }
        }
    }
    // For constant CW (cw_adaptation_ == FALSE), do nothing
}

inline void NodeBackoffManager::ResetContentionWindow() {
    cw_stage_current_ = 0;
    current_cw_min_ = cw_min_default_;
    current_cw_max_ = cw_max_default_;
}

inline int NodeBackoffManager::GetCurrentCWMin() const {
    return current_cw_min_;
}

inline int NodeBackoffManager::GetCurrentCWMax() const {
    return current_cw_max_;
}

inline int NodeBackoffManager::GetCurrentCWStage() const {
    return cw_stage_current_;
}

inline int NodeBackoffManager::GetNumBackoffInterruptions() const {
    return num_bo_interruptions_;
}

inline void NodeBackoffManager::IncrementBackoffInterruptions() {
    num_bo_interruptions_++;
}

inline double NodeBackoffManager::GetPreviousBackoff() const {
    return previous_backoff_;
}

inline void NodeBackoffManager::SetPreviousBackoff(double previous_backoff) {
    previous_backoff_ = previous_backoff;
}

inline void NodeBackoffManager::Reset() {
    remaining_backoff_ = 0.0;
    is_paused_ = false;
    num_bo_interruptions_ = 0;
    deterministic_bo_active_ = 0;
}

inline double NodeBackoffManager::ComputeBackoffInternal(int traffic_type) const {
    return ComputeBackoff(pdf_backoff_, current_cw_min_, current_cw_max_, backoff_type_,
                         traffic_type, deterministic_bo_active_, num_bo_interruptions_,
                         base_backoff_deterministic_, previous_backoff_);
}

#endif // NODE_BACKOFF_MANAGER_H

