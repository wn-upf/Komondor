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
 * @file node_logging_manager.h
 * @brief Node logging management
 * 
 * This class manages logging functionality for a node, including file logging
 * and console output. It encapsulates all logging logic that was previously
 * scattered throughout the Node component.
 * 
 * @class NodeLoggingManager
 * @brief Manages logging for a node
 */

#ifndef NODE_LOGGING_MANAGER_H
#define NODE_LOGGING_MANAGER_H

#include "../structures/logger.h"
#include "../structures/node_configuration.h"
#include "../structures/wlan.h"
#include "../methods/auxiliary_methods.h"
#include "../methods/power_channel_methods.h"
#include "../list_of_macros.h"
#include <string>
#include <cstdio>
#include <cstdarg>
#include <cmath>

/**
 * @class NodeLoggingManager
 * @brief Manages logging for a node
 * 
 * This class is responsible for:
 * - File logging management
 * - Console output
 * - Log level management
 * - Node information logging
 * - Configuration logging
 * - Event logging
 */
class NodeLoggingManager {
public:
    /**
     * @brief Constructor
     */
    NodeLoggingManager();
    
    /**
     * @brief Destructor
     */
    ~NodeLoggingManager();
    
    /**
     * @brief Initialize logging
     * @param file_path Path to log file
     * @param save_logs Flag to enable/disable logging
     * @return true if initialization successful, false otherwise
     */
    bool InitializeLogging(const std::string& file_path, int save_logs);
    
    /**
     * @brief Close logging
     */
    void CloseLogging();
    
    /**
     * @brief Check if logging is enabled
     * @return true if logging is enabled, false otherwise
     */
    bool IsLoggingEnabled() const;
    
    /**
     * @brief Get logger object
     * @return Logger object
     */
    Logger& GetLogger();
    
    /**
     * @brief Get logger object (const version)
     * @return Logger object
     */
    const Logger& GetLogger() const;
    
    /**
     * @brief Log an event
     * @param format Format string (printf-style)
     * @param ... Variable arguments
     */
    void LogEvent(const char* format, ...) const;
    
    /**
     * @brief Log node information
     * @param node_id Node identifier
     * @param node_code Node code/name
     * @param node_type Node type
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @param current_primary_channel Current primary channel
     * @param min_channel_allowed Minimum channel allowed
     * @param max_channel_allowed Maximum channel allowed
     * @param current_dcb_policy Current DCB policy
     * @param tx_power_default Default transmission power
     * @param sensitivity_default Default sensitivity
     * @param bss_color BSS color
     * @param srg Spatial Reuse Group
     * @param non_srg_obss_pd Non-SRG OBSS-PD threshold
     * @param srg_obss_pd SRG OBSS-PD threshold
     * @param wlan WLAN information
     * @param info_detail_level Detail level for logging
     * @param header_str Header string for log entries
     */
    void LogNodeInfo(int node_id, const std::string& node_code, int node_type,
                     double x, double y, double z,
                     int current_primary_channel, int min_channel_allowed, int max_channel_allowed,
                     int current_dcb_policy,
                     double tx_power_default, double sensitivity_default,
                     int bss_color, int srg, double non_srg_obss_pd, double srg_obss_pd,
                     const Wlan& wlan,
                     int info_detail_level, const std::string& header_str) const;
    
    /**
     * @brief Print node information to console
     * @param node_id Node identifier
     * @param node_code Node code/name
     * @param node_type Node type
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @param current_primary_channel Current primary channel
     * @param min_channel_allowed Minimum channel allowed
     * @param max_channel_allowed Maximum channel allowed
     * @param current_dcb_policy Current DCB policy
     * @param tx_power_default Default transmission power
     * @param sensitivity_default Default sensitivity
     * @param bss_color BSS color
     * @param srg Spatial Reuse Group
     * @param non_srg_obss_pd Non-SRG OBSS-PD threshold
     * @param srg_obss_pd SRG OBSS-PD threshold
     * @param wlan WLAN information
     * @param info_detail_level Detail level for logging
     * @param backoff_type Backoff type
     * @param cw_adaptation CW adaptation
     * @param cw_min_default Minimum CW
     * @param cw_max_default Maximum CW
     * @param cw_stage_max Maximum CW stage
     * @param central_frequency Central frequency
     * @param capture_effect Capture effect
     * @param constant_per Constant PER
     */
    void PrintNodeInfo(int node_id, const std::string& node_code, int node_type,
                       double x, double y, double z,
                       int current_primary_channel, int min_channel_allowed, int max_channel_allowed,
                       int current_dcb_policy,
                       double tx_power_default, double sensitivity_default,
                       int bss_color, int srg, double non_srg_obss_pd, double srg_obss_pd,
                       const Wlan& wlan,
                       int info_detail_level,
                       int backoff_type, int cw_adaptation,
                       int cw_min_default, int cw_max_default, int cw_stage_max,
                       double central_frequency, double capture_effect, double constant_per) const;
    
    /**
     * @brief Log node configuration
     * @param node_code Node code/name
     * @param current_primary_channel Current primary channel
     * @param current_pd Current PD threshold
     * @param current_tx_power Current transmission power
     * @param current_max_bandwidth Current maximum bandwidth
     * @param spatial_reuse_enabled Flag indicating if spatial reuse is enabled
     * @param non_srg_obss_pd Non-SRG OBSS-PD threshold
     * @param header_str Header string for log entries
     */
    void LogNodeConfiguration(const std::string& node_code,
                              int current_primary_channel,
                              double current_pd, double current_tx_power,
                              int current_max_bandwidth,
                              bool spatial_reuse_enabled, double non_srg_obss_pd,
                              const std::string& header_str) const;
    
    /**
     * @brief Print node configuration to console
     * @param node_id Node identifier
     * @param current_pd Current PD threshold
     * @param current_tx_power Current transmission power
     */
    void PrintNodeConfiguration(int node_id, double current_pd, double current_tx_power) const;
    
    /**
     * @brief Log received configuration
     * @param configuration Configuration to log
     * @param header_str Header string for log entries
     */
    void LogReceivedConfiguration(const Configuration& configuration, const std::string& header_str) const;
    
    /**
     * @brief Set header string
     * @param header_str Header string
     */
    void SetHeaderString(const std::string& header_str);
    
    /**
     * @brief Get header string
     * @return Header string
     */
    std::string GetHeaderString() const;

private:
    Logger logger_;           ///< Logger object
    FILE* output_log_file_;   ///< Output log file
    std::string file_path_;   ///< Path to log file
    std::string header_str_;  ///< Header string for log entries
    int save_logs_;           ///< Flag to enable/disable logging
};

// Inline implementations

inline NodeLoggingManager::NodeLoggingManager()
    : logger_(), output_log_file_(NULL), file_path_(""), header_str_(""), save_logs_(FALSE) {
    logger_.save_logs = FALSE;
    logger_.file = NULL;
    logger_.SetVoidHeadString();
}

inline NodeLoggingManager::~NodeLoggingManager() {
    CloseLogging();
}

inline bool NodeLoggingManager::InitializeLogging(const std::string& file_path, int save_logs) {
    file_path_ = file_path;
    save_logs_ = save_logs;
    
    if (save_logs_) {
        // Remove existing file
        remove(file_path_.c_str());
        
        // Open file for appending
        output_log_file_ = fopen(file_path_.c_str(), "at");
        if (output_log_file_ == NULL) {
            return false;
        }
        
        logger_.save_logs = save_logs_;
        logger_.file = output_log_file_;
        logger_.SetVoidHeadString();
    }
    
    return true;
}

inline void NodeLoggingManager::CloseLogging() {
    if (output_log_file_ != NULL) {
        fclose(output_log_file_);
        output_log_file_ = NULL;
    }
    logger_.save_logs = FALSE;
    logger_.file = NULL;
}

inline bool NodeLoggingManager::IsLoggingEnabled() const {
    return (save_logs_ != FALSE && logger_.file != NULL);
}

inline Logger& NodeLoggingManager::GetLogger() {
    return logger_;
}

inline const Logger& NodeLoggingManager::GetLogger() const {
    return logger_;
}

inline void NodeLoggingManager::LogEvent(const char* format, ...) const {
    if (!IsLoggingEnabled()) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vfprintf(logger_.file, format, args);
    va_end(args);
}

inline void NodeLoggingManager::SetHeaderString(const std::string& header_str) {
    header_str_ = header_str;
}

inline std::string NodeLoggingManager::GetHeaderString() const {
    return header_str_;
}

inline void NodeLoggingManager::LogNodeInfo(int node_id, const std::string& node_code, int node_type,
                                             double x, double y, double z,
                                             int current_primary_channel, int min_channel_allowed, int max_channel_allowed,
                                             int current_dcb_policy,
                                             double tx_power_default, double sensitivity_default,
                                             int bss_color, int srg, double non_srg_obss_pd, double srg_obss_pd,
                                             const Wlan& wlan,
                                             int info_detail_level, const std::string& header_str) const {
    if (!IsLoggingEnabled()) {
        return;
    }
    
    fprintf(logger_.file, "%s Node %s info:\n", header_str.c_str(), node_code.c_str());
    fprintf(logger_.file, "%s - node_id = %d\n", header_str.c_str(), node_id);
    fprintf(logger_.file, "%s - node_type = %d\n", header_str.c_str(), node_type);
    fprintf(logger_.file, "%s - position = (%.2f, %.2f, %.2f)\n", header_str.c_str(), x, y, z);
    fprintf(logger_.file, "%s - current_primary_channel = %d\n", header_str.c_str(), current_primary_channel);
    fprintf(logger_.file, "%s - min_channel_allowed = %d\n", header_str.c_str(), min_channel_allowed);
    fprintf(logger_.file, "%s - max_channel_allowed = %d\n", header_str.c_str(), max_channel_allowed);
    fprintf(logger_.file, "%s - current_dcb_policy = %d\n", header_str.c_str(), current_dcb_policy);
    fprintf(logger_.file, "%s - spatial_reuse_enabled = %d\n", header_str.c_str(), (bss_color >= 0));
    
    if (bss_color >= 0) {
        fprintf(logger_.file, "%s bss_color = %d\n", header_str.c_str(), bss_color);
        fprintf(logger_.file, "%s srg = %d\n", header_str.c_str(), srg);
        fprintf(logger_.file, "%s non_srg_obss_pd = %f dBm\n", header_str.c_str(), ConvertPower(PW_TO_DBM, non_srg_obss_pd));
        fprintf(logger_.file, "%s srg_obss_pd = %f dBm\n", header_str.c_str(), ConvertPower(PW_TO_DBM, srg_obss_pd));
    }
    
    if (info_detail_level > INFO_DETAIL_LEVEL_0) {
        // Wlan::WriteWlanInfo is not const, so we need to call it with a non-const logger
        // Create a copy of the logger for this call
        Logger logger_copy = logger_;
        const_cast<Wlan&>(wlan).WriteWlanInfo(logger_copy, header_str);
    }
    
    if (info_detail_level > INFO_DETAIL_LEVEL_1) {
        fprintf(logger_.file, "%s - [cw_min_default - cw_max_default] = [%d-%d]\n", header_str.c_str(), 0, 0); // Placeholder
        fprintf(logger_.file, "%s - cw_stage_max = %d\n", header_str.c_str(), 0); // Placeholder
        fprintf(logger_.file, "%s - tx_power_default = %f pW\n", header_str.c_str(), tx_power_default);
        fprintf(logger_.file, "%s - sensitivity_default = %f pW\n", header_str.c_str(), sensitivity_default);
    }
}

inline void NodeLoggingManager::PrintNodeInfo(int node_id, const std::string& node_code, int node_type,
                                               double x, double y, double z,
                                               int current_primary_channel, int min_channel_allowed, int max_channel_allowed,
                                               int current_dcb_policy,
                                               double tx_power_default, double sensitivity_default,
                                               int bss_color, int srg, double non_srg_obss_pd, double srg_obss_pd,
                                               const Wlan& wlan,
                                               int info_detail_level,
                                               int backoff_type, int cw_adaptation,
                                               int cw_min_default, int cw_max_default, int cw_stage_max,
                                               double central_frequency, double capture_effect, double constant_per) const {
    printf("\n%s Node %s info:\n", LOG_LVL3, node_code.c_str());
    printf("%s node_id = %d\n", LOG_LVL4, node_id);
    printf("%s node_type = %d\n", LOG_LVL4, node_type);
    printf("%s position = (%.2f, %.2f, %.2f)\n", LOG_LVL4, x, y, z);
    printf("%s current_primary_channel = %d (range = [%d - %d])\n",
           LOG_LVL4, current_primary_channel, min_channel_allowed, max_channel_allowed);
    printf("%s current_dcb_policy = %d\n", LOG_LVL4, current_dcb_policy);
    printf("%s tx_power_default = %f pW (%f dBm)\n", LOG_LVL4, tx_power_default, ConvertPower(PW_TO_DBM, tx_power_default));
    printf("%s sensitivity_default = %f pW (%f dBm)\n", LOG_LVL4, sensitivity_default, ConvertPower(PW_TO_DBM, sensitivity_default));
    printf("%s spatial_reuse_enabled = %d\n", LOG_LVL4, (bss_color >= 0));
    
    if (bss_color >= 0) {
        printf("%s bss_color = %d\n", LOG_LVL5, bss_color);
        printf("%s srg = %d\n", LOG_LVL5, srg);
        printf("%s non_srg_obss_pd = %f dBm\n", LOG_LVL5, ConvertPower(PW_TO_DBM, non_srg_obss_pd));
        printf("%s srg_obss_pd = %f dBm\n", LOG_LVL5, ConvertPower(PW_TO_DBM, srg_obss_pd));
    }
    
    if (info_detail_level > INFO_DETAIL_LEVEL_0 && node_type == NODE_TYPE_AP) {
        printf("%s wlan:\n", LOG_LVL4);
        printf("%s wlan code = %s\n", LOG_LVL5, wlan.wlan_code.c_str());
        printf("%s wlan id = %d\n", LOG_LVL5, wlan.wlan_id);
        printf("%s wlan AP id = %d\n", LOG_LVL5, wlan.ap_id);
        printf("%s Identifiers of STAs in WLAN (total number of STAs = %d): ", LOG_LVL5, wlan.num_stas);
        const_cast<Wlan&>(wlan).PrintStaIds();
    }
    
    if (info_detail_level > INFO_DETAIL_LEVEL_1) {
        printf("%s backoff_type = %d\n", LOG_LVL4, backoff_type);
        printf("%s cw_adaptation = %d\n", LOG_LVL4, cw_adaptation);
        printf("%s [cw_min_default - cw_max_default] = [%d-%d]\n", LOG_LVL4, cw_min_default, cw_max_default);
        printf("%s cw_stage_max = %d\n", LOG_LVL4, cw_stage_max);
        printf("%s central_frequency = %f Hz (%f GHz)\n", LOG_LVL4, central_frequency, central_frequency * pow(10, -9));
        printf("%s capture_effect = %f [linear] (%f dB)\n", LOG_LVL4, capture_effect, ConvertPower(LINEAR_TO_DB, capture_effect));
        printf("%s Constant PER = %f\n", LOG_LVL4, constant_per);
    }
}

inline void NodeLoggingManager::LogNodeConfiguration(const std::string& node_code,
                                                      int current_primary_channel,
                                                      double current_pd, double current_tx_power,
                                                      int current_max_bandwidth,
                                                      bool spatial_reuse_enabled, double non_srg_obss_pd,
                                                      const std::string& header_str) const {
    if (!IsLoggingEnabled()) {
        return;
    }
    
    fprintf(logger_.file, "%s Configuration %s info:\n", header_str.c_str(), node_code.c_str());
    fprintf(logger_.file, "%s - current_primary = %d\n", header_str.c_str(), current_primary_channel);
    
    if (spatial_reuse_enabled) {
        fprintf(logger_.file, "%s - current_pd (OBSS/PD) = %f (%f dBm)\n", 
                header_str.c_str(), non_srg_obss_pd, ConvertPower(PW_TO_DBM, non_srg_obss_pd));
    } else {
        fprintf(logger_.file, "%s - current_pd = %f (%f dBm)\n", 
                header_str.c_str(), current_pd, ConvertPower(PW_TO_DBM, current_pd));
    }
    
    fprintf(logger_.file, "%s - current_tx_power = %f (%f dBm)\n", 
            header_str.c_str(), current_tx_power, ConvertPower(PW_TO_DBM, current_tx_power));
    fprintf(logger_.file, "%s - current_max_bandwidth = %d\n", header_str.c_str(), current_max_bandwidth);
}

inline void NodeLoggingManager::PrintNodeConfiguration(int node_id, double current_pd, double current_tx_power) const {
    printf("Node%d - Configuration info:\n", node_id);
    printf(" - current_pd = %f (%f dBm)\n", current_pd, ConvertPower(PW_TO_DBM, current_pd));
    printf(" - current_tx_power = %f (%f dBm)\n", current_tx_power, ConvertPower(PW_TO_DBM, current_tx_power));
}

inline void NodeLoggingManager::LogReceivedConfiguration(const Configuration& configuration, const std::string& header_str) const {
    if (!IsLoggingEnabled()) {
        return;
    }
    
    fprintf(logger_.file, "%s Received Configuration:\n", header_str.c_str());
    fprintf(logger_.file, "%s - selected_primary_channel = %d\n", header_str.c_str(), configuration.selected_primary_channel);
    fprintf(logger_.file, "%s - selected_pd = %f (%f dBm)\n", 
            header_str.c_str(), configuration.selected_pd, ConvertPower(PW_TO_DBM, configuration.selected_pd));
    fprintf(logger_.file, "%s - current_tx_power = %f (%f dBm)\n", 
            header_str.c_str(), configuration.selected_tx_power, ConvertPower(PW_TO_DBM, configuration.selected_tx_power));
    fprintf(logger_.file, "%s - selected_max_bandwidth = %d\n", header_str.c_str(), configuration.selected_max_bandwidth);
}

// Macro for backward compatibility with existing LOGS macro
#define NODE_LOGS(logging_manager, format, ...) \
    do { \
        if ((logging_manager).IsLoggingEnabled()) { \
            (logging_manager).LogEvent(format, ##__VA_ARGS__); \
        } \
    } while(0)

#endif // NODE_LOGGING_MANAGER_H

