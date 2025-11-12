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
 * @file node_statistics_manager.h
 * @brief Node statistics management
 * 
 * This class manages statistics collection and reporting for a node, including
 * throughput, delay, packet loss, channel utilization, and per-STA statistics.
 * It encapsulates all statistics-related logic that was previously scattered
 * throughout the Node component.
 * 
 * @class NodeStatisticsManager
 * @brief Manages statistics for a node
 */

#ifndef NODE_STATISTICS_MANAGER_H
#define NODE_STATISTICS_MANAGER_H

#include "../structures/performance.h"
#include "../list_of_macros.h"
#include <cmath>

/**
 * @class NodeStatisticsManager
 * @brief Manages statistics for a node
 * 
 * This class is responsible for:
 * - Throughput measurement
 * - Delay measurement
 * - Packet loss tracking
 * - Channel utilization tracking
 * - Per-STA statistics
 * - Performance reporting
 */
class NodeStatisticsManager {
public:
    /**
     * @brief Constructor
     * @param num_stas Number of STAs (for per-STA statistics, default: 0)
     * @param num_channels Number of channels (default: NUM_CHANNELS_KOMONDOR)
     * @param num_channels_allowed Number of channels allowed (default: 0)
     */
    NodeStatisticsManager(int num_stas = 0, int num_channels = NUM_CHANNELS_KOMONDOR, int num_channels_allowed = 0);
    
    /**
     * @brief Initialize statistics manager with parameters
     * @param num_stas Number of STAs (for per-STA statistics)
     * @param num_channels Number of channels
     * @param num_channels_allowed Number of channels allowed
     */
    void InitializeWithParams(int num_stas, int num_channels, int num_channels_allowed);
    
    /**
     * @brief Destructor
     */
    ~NodeStatisticsManager();
    
    /**
     * @brief Initialize statistics manager
     * @param frame_length Frame length [bytes]
     */
    void Initialize(int frame_length);
    
    /**
     * @brief Record data packet sent
     * @param sta_index STA index (-1 for aggregate)
     */
    void RecordDataPacketSent(int sta_index = -1);
    
    /**
     * @brief Record data packet acked
     * @param num_frames Number of frames acked
     * @param sta_index STA index (-1 for aggregate)
     */
    void RecordDataPacketAcked(int num_frames, int sta_index = -1);
    
    /**
     * @brief Record data packet lost
     * @param sta_index STA index (-1 for aggregate)
     */
    void RecordDataPacketLost(int sta_index = -1);
    
    /**
     * @brief Record RTS/CTS sent
     * @param sta_index STA index (-1 for aggregate)
     */
    void RecordRtsCtsSent(int sta_index = -1);
    
    /**
     * @brief Record RTS/CTS lost
     * @param sta_index STA index (-1 for aggregate)
     */
    void RecordRtsCtsLost(int sta_index = -1);
    
    /**
     * @brief Record RTS lost due to slotted backoff
     */
    void RecordRtsLostSlottedBackoff();
    
    /**
     * @brief Record packet generated
     */
    void RecordPacketGenerated();
    
    /**
     * @brief Record packet dropped
     */
    void RecordPacketDropped();
    
    /**
     * @brief Record TX initiation tried
     */
    void RecordTXInitTried();
    
    /**
     * @brief Record TX initiation not possible
     */
    void RecordTXInitNotPossible();
    
    /**
     * @brief Record transmission time
     * @param duration Transmission duration [s]
     * @param left_channel Left channel
     * @param right_channel Right channel
     * @param is_lost true if transmission was lost, false otherwise
     */
    void RecordTransmissionTime(double duration, int left_channel, int right_channel, bool is_lost);
    
    /**
     * @brief Record channel busy time
     * @param duration Duration [s]
     * @param channel Channel index
     */
    void RecordChannelBusyTime(double duration, int channel);
    
    /**
     * @brief Record delay measurement
     * @param delay Delay [s]
     */
    void RecordDelay(double delay);
    
    /**
     * @brief Record waiting time measurement
     * @param waiting_time Waiting time [s]
     */
    void RecordWaitingTime(double waiting_time);
    
    /**
     * @brief Record backoff computation
     * @param backoff_value Backoff value [s]
     */
    void RecordBackoffComputation(double backoff_value);
    
    /**
     * @brief Record rho measurement
     * @param accomplished true if accomplished, false otherwise
     */
    void RecordRhoMeasurement(bool accomplished);
    
    /**
     * @brief Record utilization measurement
     * @param buffer_has_packets true if buffer has packets, false otherwise
     */
    void RecordUtilizationMeasurement(bool buffer_has_packets);
    
    /**
     * @brief Record time in NAV
     * @param duration Duration in NAV [s]
     */
    void RecordTimeInNAV(double duration);
    
    /**
     * @brief Record times went to NAV
     */
    void RecordTimesWentToNAV();
    
    /**
     * @brief Update statistics for current simulation time
     * @param current_time Current simulation time [s]
     * @param last_measurements_window Last measurements window [s]
     */
    void UpdateStatistics(double current_time, double last_measurements_window);
    
    /**
     * @brief Get throughput
     * @return Throughput [bps]
     */
    double GetThroughput() const;
    
    /**
     * @brief Get average delay
     * @return Average delay [s]
     */
    double GetAverageDelay() const;
    
    /**
     * @brief Get average rho
     * @return Average rho
     */
    double GetAverageRho() const;
    
    /**
     * @brief Get average utilization
     * @return Average utilization
     */
    double GetAverageUtilization() const;
    
    /**
     * @brief Get data packets sent
     * @return Number of data packets sent
     */
    int GetDataPacketsSent() const;
    
    /**
     * @brief Get data packets acked
     * @return Number of data packets acked
     */
    int GetDataPacketsAcked() const;
    
    /**
     * @brief Get data packets lost
     * @return Number of data packets lost
     */
    int GetDataPacketsLost() const;
    
    /**
     * @brief Get data frames acked
     * @return Number of data frames acked
     */
    int GetDataFramesAcked() const;
    
    /**
     * @brief Get RTS/CTS sent
     * @return Number of RTS/CTS sent
     */
    int GetRtsCtsSent() const;
    
    /**
     * @brief Get RTS/CTS lost
     * @return Number of RTS/CTS lost
     */
    int GetRtsCtsLost() const;
    
    /**
     * @brief Get performance report
     * @return Performance report
     */
    const Performance& GetPerformanceReport() const;
    
    /**
     * @brief Get performance report (non-const)
     * @return Performance report
     */
    Performance& GetPerformanceReport();
    
    /**
     * @brief Get throughput per STA array
     * @return Pointer to throughput per STA array
     * @note Returns non-const pointer for backward compatibility with legacy code
     */
    double* GetThroughputPerStaPtr();
    
    /**
     * @brief Get data packets sent per STA array
     * @return Pointer to data packets sent per STA array
     * @note Returns non-const pointer for backward compatibility with legacy code
     */
    int* GetDataPacketsSentPerStaPtr();
    
    /**
     * @brief Get RTS/CTS sent per STA array
     * @return Pointer to RTS/CTS sent per STA array
     * @note Returns non-const pointer for backward compatibility with legacy code
     */
    int* GetRtsCtsSentPerStaPtr();
    
    /**
     * @brief Get data packets lost per STA array
     * @return Pointer to data packets lost per STA array
     * @note Returns non-const pointer for backward compatibility with legacy code
     */
    int* GetDataPacketsLostPerStaPtr();
    
    /**
     * @brief Get RTS/CTS lost per STA array
     * @return Pointer to RTS/CTS lost per STA array
     * @note Returns non-const pointer for backward compatibility with legacy code
     */
    int* GetRtsCtsLostPerStaPtr();
    
    /**
     * @brief Get data packets acked per STA array
     * @return Pointer to data packets acked per STA array
     * @note Returns non-const pointer for backward compatibility with legacy code
     */
    int* GetDataPacketsAckedPerStaPtr();
    
    /**
     * @brief Get data frames acked per STA array
     * @return Pointer to data frames acked per STA array
     * @note Returns non-const pointer for backward compatibility with legacy code
     */
    int* GetDataFramesAckedPerStaPtr();
    
    /**
     * @brief Reset statistics
     */
    void Reset();
    
    /**
     * @brief Reset performance metrics for new measurement window
     * @param current_time Current simulation time [s]
     * @param num_channels_allowed Number of channels allowed
     */
    void RestartPerformanceMetrics(double current_time, int num_channels_allowed);
    
    /**
     * @brief Print statistics
     * @param node_code Node code
     * @param node_id Node ID
     * @param node_is_transmitter Flag indicating if node is transmitter
     * @param print_node_logs Flag indicating if logs should be printed
     * @param wlan_num_stas Number of STAs in WLAN
     * @param simulation_time Simulation time [s]
     * @param num_channels_allowed Number of channels allowed
     * @param bandwidth_used_txing Bandwidth used for transmitting [MHz]
     */
    void PrintStatistics(const std::string& node_code, int node_id, bool node_is_transmitter,
                        bool print_node_logs, int wlan_num_stas, double simulation_time,
                        int num_channels_allowed, double bandwidth_used_txing) const;
    
    /**
     * @brief Write statistics to log file
     * @param logger Logger object
     * @param node_id Node ID
     * @param node_state Node state
     * @param node_is_transmitter Flag indicating if node is transmitter
     * @param save_node_logs Flag indicating if logs should be saved
     * @param simulation_time Simulation time [s]
     * @param num_channels_allowed Number of channels allowed
     * @param bandwidth_used_txing Bandwidth used for transmitting [MHz]
     */
    void WriteStatistics(const Logger& logger, int node_id, int node_state,
                        bool node_is_transmitter, bool save_node_logs,
                        double simulation_time, int num_channels_allowed,
                        double bandwidth_used_txing) const;

private:
    // Statistics counters
    int data_packets_sent_;              ///< Number of data packets sent
    int rts_cts_sent_;                   ///< Number of RTS/CTS packets sent
    double num_packets_generated_;       ///< Number of packets generated
    double num_packets_dropped_;         ///< Number of packets dropped
    double* total_time_transmitting_per_channel_;     ///< Time transmitting per channel
    double* total_time_transmitting_in_num_channels_; ///< Time transmitting in N channels
    double* total_time_lost_per_channel_;            ///< Time lost per channel
    double* total_time_lost_in_num_channels_;        ///< Time lost in N channels
    double* total_time_channel_busy_per_channel_;    ///< Time channel busy per channel
    double throughput_;                  ///< Throughput [bps]
    double throughput_loss_;             ///< Throughput loss [bps]
    int data_packets_acked_;             ///< Data packets acked
    int data_frames_acked_;              ///< Data frames acked
    int data_packets_lost_;              ///< Data packets lost
    int* num_trials_tx_per_num_channels_; ///< Number of TX trials per number of channels
    int rts_cts_lost_;                   ///< RTS/CTS lost
    int rts_lost_slotted_bo_;            ///< RTS lost due to slotted backoff
    double prob_slotted_bo_collision_;   ///< Probability of slotted BO collision
    double average_waiting_time_;        ///< Average waiting time [s]
    double bandwidth_used_txing_;        ///< Bandwidth used for transmitting [MHz]
    int num_delay_measurements_;         ///< Number of delay measurements
    double sum_delays_;                  ///< Sum of delays
    double average_delay_;               ///< Average delay [s]
    double average_rho_;                 ///< Average rho
    double average_utilization_;         ///< Average utilization
    double generation_drop_ratio_;       ///< Generation drop ratio
    double expected_backoff_;            ///< Expected backoff [s]
    int num_new_backoff_computations_;   ///< Number of new backoff computations
    
    // Last measurements window statistics
    double last_measurements_window_;    ///< Last measurements window [s]
    double last_throughput_;             ///< Last throughput [bps]
    int last_data_frames_acked_;         ///< Last data frames acked
    double last_average_delay_;          ///< Last average delay [s]
    double last_sum_delays_;             ///< Last sum of delays
    int last_num_delay_measurements_;    ///< Last number of delay measurements
    double* last_total_time_transmitting_per_channel_; ///< Last total time transmitting per channel
    double* last_total_time_lost_per_channel_;         ///< Last total time lost per channel
    double last_sum_waiting_time_;       ///< Last sum of waiting time
    int last_num_average_waiting_time_measurements_;   ///< Last number of waiting time measurements
    
    // Per-STA statistics
    double* throughput_per_sta_;         ///< Throughput per STA [bps]
    int* data_packets_sent_per_sta_;     ///< Data packets sent per STA
    int* rts_cts_sent_per_sta_;          ///< RTS/CTS sent per STA
    int* data_packets_lost_per_sta_;     ///< Data packets lost per STA
    int* rts_cts_lost_per_sta_;          ///< RTS/CTS lost per STA
    int* data_packets_acked_per_sta_;    ///< Data packets acked per STA
    int* data_frames_acked_per_sta_;     ///< Data frames acked per STA
    
    // Rho and utilization measurements
    int num_measures_rho_;               ///< Number of rho measurements
    int num_measures_rho_accomplished_;  ///< Number of rho measurements accomplished
    int num_measures_utilization_;       ///< Number of utilization measurements
    int num_measures_buffer_with_packets_; ///< Number of utilization measurements with packets
    
    // NAV statistics
    double time_in_nav_;                 ///< Time in NAV [s]
    int times_went_to_nav_;              ///< Times went to NAV
    
    // Performance report
    Performance performance_report_;      ///< Performance report
    
    // Configuration
    int num_stas_;                       ///< Number of STAs
    int num_channels_;                   ///< Number of channels
    int num_channels_allowed_;           ///< Number of channels allowed
    int frame_length_;                   ///< Frame length [bytes]
    int limited_num_packets_aggregated_; ///< Limited number of packets aggregated
    
    /**
     * @brief Initialize statistics arrays
     */
    void InitializeArrays();
    
    /**
     * @brief Clean up statistics arrays
     */
    void CleanupArrays();
    
    /**
     * @brief Compute statistics percentages and ratios
     * @param simulation_time Simulation time [s]
     */
    void ComputeStatistics(double simulation_time);
};

// Inline implementations for simple getters

inline double NodeStatisticsManager::GetThroughput() const {
    return throughput_;
}

inline double NodeStatisticsManager::GetAverageDelay() const {
    return average_delay_;
}

inline double NodeStatisticsManager::GetAverageRho() const {
    return average_rho_;
}

inline double NodeStatisticsManager::GetAverageUtilization() const {
    return average_utilization_;
}

inline int NodeStatisticsManager::GetDataPacketsSent() const {
    return data_packets_sent_;
}

inline int NodeStatisticsManager::GetDataPacketsAcked() const {
    return data_packets_acked_;
}

inline int NodeStatisticsManager::GetDataPacketsLost() const {
    return data_packets_lost_;
}

inline int NodeStatisticsManager::GetDataFramesAcked() const {
    return data_frames_acked_;
}

inline int NodeStatisticsManager::GetRtsCtsSent() const {
    return rts_cts_sent_;
}

inline int NodeStatisticsManager::GetRtsCtsLost() const {
    return rts_cts_lost_;
}

inline const Performance& NodeStatisticsManager::GetPerformanceReport() const {
    return performance_report_;
}

inline Performance& NodeStatisticsManager::GetPerformanceReport() {
    return performance_report_;
}

inline double* NodeStatisticsManager::GetThroughputPerStaPtr() {
    return throughput_per_sta_;
}

inline int* NodeStatisticsManager::GetDataPacketsSentPerStaPtr() {
    return data_packets_sent_per_sta_;
}

inline int* NodeStatisticsManager::GetRtsCtsSentPerStaPtr() {
    return rts_cts_sent_per_sta_;
}

inline int* NodeStatisticsManager::GetDataPacketsLostPerStaPtr() {
    return data_packets_lost_per_sta_;
}

inline int* NodeStatisticsManager::GetRtsCtsLostPerStaPtr() {
    return rts_cts_lost_per_sta_;
}

inline int* NodeStatisticsManager::GetDataPacketsAckedPerStaPtr() {
    return data_packets_acked_per_sta_;
}

inline int* NodeStatisticsManager::GetDataFramesAckedPerStaPtr() {
    return data_frames_acked_per_sta_;
}

// Constructor and initialization implementations
// Note: Initialization order must match member declaration order
inline NodeStatisticsManager::NodeStatisticsManager(int num_stas, int num_channels, int num_channels_allowed)
    : data_packets_sent_(0), rts_cts_sent_(0), num_packets_generated_(0.0), num_packets_dropped_(0.0),
      total_time_transmitting_per_channel_(NULL), total_time_transmitting_in_num_channels_(NULL),
      total_time_lost_per_channel_(NULL), total_time_lost_in_num_channels_(NULL),
      total_time_channel_busy_per_channel_(NULL), throughput_(0.0), throughput_loss_(0.0),
      data_packets_acked_(0), data_frames_acked_(0), data_packets_lost_(0),
      num_trials_tx_per_num_channels_(NULL), rts_cts_lost_(0), rts_lost_slotted_bo_(0),
      prob_slotted_bo_collision_(0.0), average_waiting_time_(0.0), bandwidth_used_txing_(0.0),
      num_delay_measurements_(0), sum_delays_(0.0), average_delay_(0.0), average_rho_(0.0),
      average_utilization_(0.0), generation_drop_ratio_(0.0), expected_backoff_(0.0),
      num_new_backoff_computations_(0), last_measurements_window_(0.0), last_throughput_(0.0),
      last_data_frames_acked_(0), last_average_delay_(0.0), last_sum_delays_(0.0),
      last_num_delay_measurements_(0), last_total_time_transmitting_per_channel_(NULL),
      last_total_time_lost_per_channel_(NULL), last_sum_waiting_time_(0.0),
      last_num_average_waiting_time_measurements_(0), throughput_per_sta_(NULL),
      data_packets_sent_per_sta_(NULL), rts_cts_sent_per_sta_(NULL), data_packets_lost_per_sta_(NULL),
      rts_cts_lost_per_sta_(NULL), data_packets_acked_per_sta_(NULL), data_frames_acked_per_sta_(NULL),
      num_measures_rho_(0), num_measures_rho_accomplished_(0), num_measures_utilization_(0),
      num_measures_buffer_with_packets_(0), time_in_nav_(0.0), times_went_to_nav_(0),
      num_stas_(num_stas), num_channels_(num_channels), num_channels_allowed_(num_channels_allowed),
      frame_length_(0), limited_num_packets_aggregated_(0) {
    if (num_stas > 0 && num_channels > 0) {
        InitializeArrays();
    }
}

inline void NodeStatisticsManager::InitializeWithParams(int num_stas, int num_channels, int num_channels_allowed) {
    if (num_stas_ > 0 || num_channels_ > 0) {
        CleanupArrays(); // Clean up if already initialized
    }
    num_stas_ = num_stas;
    num_channels_ = num_channels;
    num_channels_allowed_ = num_channels_allowed;
    if (num_stas_ > 0 && num_channels_ > 0) {
        InitializeArrays();
    }
}

inline NodeStatisticsManager::~NodeStatisticsManager() {
    CleanupArrays();
}

inline void NodeStatisticsManager::Initialize(int frame_length) {
    frame_length_ = frame_length;
}

inline void NodeStatisticsManager::InitializeArrays() {
    // Allocate per-STA statistics arrays
    if (num_stas_ > 0) {
        throughput_per_sta_ = new double[num_stas_];
        data_packets_sent_per_sta_ = new int[num_stas_];
        rts_cts_sent_per_sta_ = new int[num_stas_];
        data_packets_lost_per_sta_ = new int[num_stas_];
        rts_cts_lost_per_sta_ = new int[num_stas_];
        data_packets_acked_per_sta_ = new int[num_stas_];
        data_frames_acked_per_sta_ = new int[num_stas_];
        
        // Initialize arrays to zero
        for (int i = 0; i < num_stas_; ++i) {
            throughput_per_sta_[i] = 0.0;
            data_packets_sent_per_sta_[i] = 0;
            rts_cts_sent_per_sta_[i] = 0;
            data_packets_lost_per_sta_[i] = 0;
            rts_cts_lost_per_sta_[i] = 0;
            data_packets_acked_per_sta_[i] = 0;
            data_frames_acked_per_sta_[i] = 0;
        }
    }
}

inline void NodeStatisticsManager::CleanupArrays() {
    // Clean up per-STA statistics arrays
    if (throughput_per_sta_) {
        delete[] throughput_per_sta_;
        throughput_per_sta_ = NULL;
    }
    if (data_packets_sent_per_sta_) {
        delete[] data_packets_sent_per_sta_;
        data_packets_sent_per_sta_ = NULL;
    }
    if (rts_cts_sent_per_sta_) {
        delete[] rts_cts_sent_per_sta_;
        rts_cts_sent_per_sta_ = NULL;
    }
    if (data_packets_lost_per_sta_) {
        delete[] data_packets_lost_per_sta_;
        data_packets_lost_per_sta_ = NULL;
    }
    if (rts_cts_lost_per_sta_) {
        delete[] rts_cts_lost_per_sta_;
        rts_cts_lost_per_sta_ = NULL;
    }
    if (data_packets_acked_per_sta_) {
        delete[] data_packets_acked_per_sta_;
        data_packets_acked_per_sta_ = NULL;
    }
    if (data_frames_acked_per_sta_) {
        delete[] data_frames_acked_per_sta_;
        data_frames_acked_per_sta_ = NULL;
    }
}

inline void NodeStatisticsManager::RecordDataPacketSent(int sta_index) {
    data_packets_sent_++;
    if (sta_index >= 0 && data_packets_sent_per_sta_ != NULL) {
        data_packets_sent_per_sta_[sta_index]++;
    }
}

inline void NodeStatisticsManager::RecordDataPacketAcked(int num_frames, int sta_index) {
    data_packets_acked_++;
    data_frames_acked_ += num_frames;
    if (sta_index >= 0 && data_packets_acked_per_sta_ != NULL && data_frames_acked_per_sta_ != NULL) {
        data_packets_acked_per_sta_[sta_index]++;
        data_frames_acked_per_sta_[sta_index] += num_frames;
    }
}

inline void NodeStatisticsManager::RecordDataPacketLost(int sta_index) {
    data_packets_lost_++;
    if (sta_index >= 0 && data_packets_lost_per_sta_ != NULL) {
        data_packets_lost_per_sta_[sta_index]++;
    }
}

inline void NodeStatisticsManager::RecordRtsCtsSent(int sta_index) {
    rts_cts_sent_++;
    if (sta_index >= 0 && rts_cts_sent_per_sta_ != NULL) {
        rts_cts_sent_per_sta_[sta_index]++;
    }
}

inline void NodeStatisticsManager::RecordRtsCtsLost(int sta_index) {
    rts_cts_lost_++;
    if (sta_index >= 0 && rts_cts_lost_per_sta_ != NULL) {
        rts_cts_lost_per_sta_[sta_index]++;
    }
}

inline void NodeStatisticsManager::RecordRtsLostSlottedBackoff() {
    rts_lost_slotted_bo_++;
}

inline void NodeStatisticsManager::RecordPacketGenerated() {
    num_packets_generated_ += 1.0;
}

inline void NodeStatisticsManager::RecordPacketDropped() {
    num_packets_dropped_ += 1.0;
}

inline void NodeStatisticsManager::RecordTXInitTried() {
    // Implementation would increment counter
}

inline void NodeStatisticsManager::RecordTXInitNotPossible() {
    // Implementation would increment counter
}

inline void NodeStatisticsManager::RecordTransmissionTime(double duration, int left_channel, int right_channel, bool is_lost) {
    // Implementation would record transmission time
    (void)duration;
    (void)left_channel;
    (void)right_channel;
    (void)is_lost;
}

inline void NodeStatisticsManager::RecordChannelBusyTime(double duration, int channel) {
    // Implementation would record channel busy time
    (void)duration;
    (void)channel;
}

inline void NodeStatisticsManager::RecordDelay(double delay) {
    num_delay_measurements_++;
    sum_delays_ += delay;
}

inline void NodeStatisticsManager::RecordWaitingTime(double waiting_time) {
    // Implementation would record waiting time
    (void)waiting_time;
}

inline void NodeStatisticsManager::RecordBackoffComputation(double backoff_value) {
    num_new_backoff_computations_++;
    expected_backoff_ += backoff_value;
}

inline void NodeStatisticsManager::RecordRhoMeasurement(bool accomplished) {
    num_measures_rho_++;
    if (accomplished) {
        num_measures_rho_accomplished_++;
    }
}

inline void NodeStatisticsManager::RecordUtilizationMeasurement(bool buffer_has_packets) {
    num_measures_utilization_++;
    if (buffer_has_packets) {
        num_measures_buffer_with_packets_++;
    }
}

inline void NodeStatisticsManager::RecordTimeInNAV(double duration) {
    time_in_nav_ += duration;
}

inline void NodeStatisticsManager::RecordTimesWentToNAV() {
    times_went_to_nav_++;
}

inline void NodeStatisticsManager::UpdateStatistics(double current_time, double last_measurements_window) {
    // Update statistics based on current time
    (void)current_time;
    last_measurements_window_ = last_measurements_window;
    ComputeStatistics(current_time);
}

inline void NodeStatisticsManager::ComputeStatistics(double simulation_time) {
    if (num_delay_measurements_ > 0) {
        average_delay_ = sum_delays_ / (double)num_delay_measurements_;
    }
    if (num_measures_rho_ > 0) {
        average_rho_ = (double)num_measures_rho_accomplished_ / (double)num_measures_rho_;
    }
    if (num_measures_utilization_ > 0) {
        average_utilization_ = (double)num_measures_buffer_with_packets_ / (double)num_measures_utilization_;
    }
    if (simulation_time > 0 && frame_length_ > 0) {
        throughput_ = ((double)data_frames_acked_ * frame_length_) / simulation_time;
    }
    if (num_new_backoff_computations_ > 0) {
        expected_backoff_ = expected_backoff_ / (double)num_new_backoff_computations_;
    }
}

inline void NodeStatisticsManager::Reset() {
    // Reset all statistics
    data_packets_sent_ = 0;
    rts_cts_sent_ = 0;
    num_packets_generated_ = 0.0;
    num_packets_dropped_ = 0.0;
    throughput_ = 0.0;
    data_packets_acked_ = 0;
    data_frames_acked_ = 0;
    data_packets_lost_ = 0;
    rts_cts_lost_ = 0;
    rts_lost_slotted_bo_ = 0;
    num_delay_measurements_ = 0;
    sum_delays_ = 0.0;
    average_delay_ = 0.0;
    average_rho_ = 0.0;
    average_utilization_ = 0.0;
    expected_backoff_ = 0.0;
    num_new_backoff_computations_ = 0;
    time_in_nav_ = 0.0;
    times_went_to_nav_ = 0;
}

inline void NodeStatisticsManager::RestartPerformanceMetrics(double current_time, int num_channels_allowed) {
    last_measurements_window_ = current_time;
    last_throughput_ = 0.0;
    last_data_frames_acked_ = 0;
    last_average_delay_ = 0.0;
    last_sum_delays_ = 0.0;
    last_num_delay_measurements_ = 0;
    last_sum_waiting_time_ = 0.0;
    last_num_average_waiting_time_measurements_ = 0;
    num_channels_allowed_ = num_channels_allowed;
}

inline void NodeStatisticsManager::PrintStatistics(const std::string& node_code, int node_id, bool node_is_transmitter,
                                                    bool print_node_logs, int wlan_num_stas, double simulation_time,
                                                    int num_channels_allowed, double bandwidth_used_txing) const {
    // Placeholder - full implementation would print all statistics
    (void)node_code;
    (void)node_id;
    (void)node_is_transmitter;
    (void)print_node_logs;
    (void)wlan_num_stas;
    (void)simulation_time;
    (void)num_channels_allowed;
    (void)bandwidth_used_txing;
}

inline void NodeStatisticsManager::WriteStatistics(const Logger& logger, int node_id, int node_state,
                                                    bool node_is_transmitter, bool save_node_logs,
                                                    double simulation_time, int num_channels_allowed,
                                                    double bandwidth_used_txing) const {
    // Placeholder - full implementation would write all statistics
    (void)logger;
    (void)node_id;
    (void)node_state;
    (void)node_is_transmitter;
    (void)save_node_logs;
    (void)simulation_time;
    (void)num_channels_allowed;
    (void)bandwidth_used_txing;
}

#endif // NODE_STATISTICS_MANAGER_H

