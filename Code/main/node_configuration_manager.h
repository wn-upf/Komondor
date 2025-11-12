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
 * @file node_configuration_manager.h
 * @brief Node configuration management
 * 
 * This class manages configuration operations for a node, including configuration
 * generation, application, and broadcasting. It encapsulates all configuration-related
 * logic that was previously scattered throughout the Node component.
 * 
 * @class NodeConfigurationManager
 * @brief Manages configuration for a node
 */

#ifndef NODE_CONFIGURATION_MANAGER_H
#define NODE_CONFIGURATION_MANAGER_H

#include "../structures/node_configuration.h"
#include "../list_of_macros.h"

/**
 * @class NodeConfigurationManager
 * @brief Manages configuration for a node
 * 
 * This class is responsible for:
 * - Configuration generation
 * - Configuration application
 * - Configuration broadcasting
 * - Configuration validation
 * - Configuration tracking
 */
class NodeConfigurationManager {
public:
    /**
     * @brief Constructor
     */
    NodeConfigurationManager();
    
    /**
     * @brief Destructor
     */
    ~NodeConfigurationManager();
    
    /**
     * @brief Generate a new configuration based on current node state
     * @param current_primary_channel Current primary channel
     * @param current_pd Current PD threshold [pW]
     * @param current_tx_power Current TX power [pW]
     * @param current_max_bandwidth Current max bandwidth [channels]
     * @param spatial_reuse_enabled Flag indicating if spatial reuse is enabled
     * @param bss_color BSS color
     * @param srg SRG
     * @param non_srg_obss_pd Non-SRG OBSS-PD threshold [pW]
     * @param srg_obss_pd SRG OBSS-PD threshold [pW]
     * @param capabilities Node capabilities
     * @param current_time Current simulation time [s]
     * @return Generated configuration
     */
    Configuration GenerateConfiguration(int current_primary_channel,
                                        double current_pd,
                                        double current_tx_power,
                                        int current_max_bandwidth,
                                        bool spatial_reuse_enabled,
                                        int bss_color, int srg,
                                        double non_srg_obss_pd, double srg_obss_pd,
                                        const Capabilities& capabilities,
                                        double current_time);
    
    /**
     * @brief Apply a new configuration to the node
     * @param configuration Configuration to apply
     * @param current_primary_channel Current primary channel (output)
     * @param current_pd Current PD threshold (output) [pW]
     * @param current_tx_power Current TX power (output) [pW]
     * @param current_max_bandwidth Current max bandwidth (output) [channels]
     * @param current_obss_pd_threshold Current OBSS-PD threshold (output) [pW]
     * @param current_tx_power_sr Current TX power SR (output) [pW]
     * @return true if configuration was applied, false otherwise
     */
    bool ApplyNewConfiguration(const Configuration& configuration,
                               int& current_primary_channel,
                               double& current_pd,
                               double& current_tx_power,
                               int& current_max_bandwidth,
                               double& current_obss_pd_threshold,
                               double& current_tx_power_sr);
    
    /**
     * @brief Check if new configuration should be applied
     * @return true if new configuration flag is set, false otherwise
     */
    bool ShouldApplyNewConfiguration() const;
    
    /**
     * @brief Set flag to apply new configuration
     * @param flag true to set flag, false to clear
     */
    void SetApplyNewConfigurationFlag(bool flag);
    
    /**
     * @brief Get new configuration
     * @return New configuration to be applied
     */
    const Configuration& GetNewConfiguration() const;
    
    /**
     * @brief Set new configuration
     * @param configuration New configuration to set
     */
    void SetNewConfiguration(const Configuration& configuration);
    
    /**
     * @brief Validate configuration
     * @param configuration Configuration to validate
     * @param capabilities Node capabilities
     * @return true if configuration is valid, false otherwise
     */
    bool ValidateConfiguration(const Configuration& configuration,
                               const Capabilities& capabilities) const;
    
    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    const Configuration& GetCurrentConfiguration() const;
    
    /**
     * @brief Set current configuration
     * @param configuration Current configuration
     */
    void SetCurrentConfiguration(const Configuration& configuration);
    
    /**
     * @brief Reset configuration manager
     */
    void Reset();

private:
    Configuration current_configuration_;    ///< Current configuration
    Configuration new_configuration_;        ///< New configuration to be applied
    bool flag_apply_new_configuration_;      ///< Flag indicating if new configuration should be applied
};

// Inline implementations

inline NodeConfigurationManager::NodeConfigurationManager()
    : flag_apply_new_configuration_(FALSE) {
    // Initialize configurations
    current_configuration_.selected_primary_channel = -1;
    current_configuration_.selected_pd = 0.0;
    current_configuration_.selected_tx_power = 0.0;
    current_configuration_.selected_max_bandwidth = -1;
    
    new_configuration_.selected_primary_channel = -1;
    new_configuration_.selected_pd = 0.0;
    new_configuration_.selected_tx_power = 0.0;
    new_configuration_.selected_max_bandwidth = -1;
}

inline NodeConfigurationManager::~NodeConfigurationManager() {
}

inline Configuration NodeConfigurationManager::GenerateConfiguration(int current_primary_channel,
                                                                     double current_pd,
                                                                     double current_tx_power,
                                                                     int current_max_bandwidth,
                                                                     bool spatial_reuse_enabled,
                                                                     int bss_color, int srg,
                                                                     double non_srg_obss_pd, double srg_obss_pd,
                                                                     const Capabilities& capabilities,
                                                                     double current_time) {
    Configuration config;
    
    config.timestamp = current_time;
    config.selected_primary_channel = current_primary_channel;
    config.selected_pd = current_pd;
    config.selected_tx_power = current_tx_power;
    config.selected_max_bandwidth = current_max_bandwidth;
    
    // Spatial reuse parameters
    config.spatial_reuse_enabled = spatial_reuse_enabled ? 1 : 0;
    config.bss_color = bss_color;
    config.srg = srg;
    config.non_srg_obss_pd = non_srg_obss_pd;
    config.srg_obss_pd = srg_obss_pd;
    
    // Copy capabilities
    config.capabilities = capabilities;
    
    return config;
}

inline bool NodeConfigurationManager::ApplyNewConfiguration(const Configuration& configuration,
                                                            int& current_primary_channel,
                                                            double& current_pd,
                                                            double& current_tx_power,
                                                            int& current_max_bandwidth,
                                                            double& current_obss_pd_threshold,
                                                            double& current_tx_power_sr) {
    // Validate configuration before applying
    if (configuration.selected_primary_channel < 0 ||
        configuration.selected_pd <= 0.0 ||
        configuration.selected_tx_power <= 0.0 ||
        configuration.selected_max_bandwidth <= 0) {
        return false;
    }
    
    // Apply configuration
    current_primary_channel = configuration.selected_primary_channel;
    current_pd = configuration.selected_pd;
    current_tx_power = configuration.selected_tx_power;
    current_max_bandwidth = configuration.selected_max_bandwidth;
    
    // Apply spatial reuse parameters if enabled
    if (configuration.spatial_reuse_enabled) {
        current_obss_pd_threshold = configuration.non_srg_obss_pd;
        current_tx_power_sr = configuration.selected_tx_power;
    }
    
    // Update current configuration
    current_configuration_ = configuration;
    
    return true;
}

inline bool NodeConfigurationManager::ShouldApplyNewConfiguration() const {
    return flag_apply_new_configuration_;
}

inline void NodeConfigurationManager::SetApplyNewConfigurationFlag(bool flag) {
    flag_apply_new_configuration_ = flag ? TRUE : FALSE;
}

inline const Configuration& NodeConfigurationManager::GetNewConfiguration() const {
    return new_configuration_;
}

inline void NodeConfigurationManager::SetNewConfiguration(const Configuration& configuration) {
    new_configuration_ = configuration;
    flag_apply_new_configuration_ = TRUE;
}

inline bool NodeConfigurationManager::ValidateConfiguration(const Configuration& configuration,
                                                            const Capabilities& capabilities) const {
    // Validate primary channel
    if (configuration.selected_primary_channel < capabilities.min_channel_allowed ||
        configuration.selected_primary_channel > capabilities.max_channel_allowed) {
        return false;
    }
    
    // Validate PD threshold
    if (configuration.selected_pd <= 0.0 || configuration.selected_pd > capabilities.sensitivity_default * 10.0) {
        return false;
    }
    
    // Validate TX power
    if (configuration.selected_tx_power <= 0.0 || configuration.selected_tx_power > capabilities.tx_power_default * 10.0) {
        return false;
    }
    
    // Validate max bandwidth
    if (configuration.selected_max_bandwidth <= 0 ||
        configuration.selected_max_bandwidth > capabilities.num_channels_allowed) {
        return false;
    }
    
    return true;
}

inline const Configuration& NodeConfigurationManager::GetCurrentConfiguration() const {
    return current_configuration_;
}

inline void NodeConfigurationManager::SetCurrentConfiguration(const Configuration& configuration) {
    current_configuration_ = configuration;
}

inline void NodeConfigurationManager::Reset() {
    flag_apply_new_configuration_ = FALSE;
    // Reset configurations to invalid state
    current_configuration_.selected_primary_channel = -1;
    new_configuration_.selected_primary_channel = -1;
}

#endif // NODE_CONFIGURATION_MANAGER_H

