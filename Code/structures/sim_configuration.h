// 1. CONFIGURATION: All input parameters and flags
struct SimConfig {
    // Logging
    int save_node_logs;
    int print_node_logs;
    int save_agent_logs;
    int print_agent_logs;
    int print_system_logs;

    // Simulation Settings
    double simulation_time_komondor;
    std::string simulation_code;
    int seed;
    int simulation_index;

    // Feature Flags
    int agents_enabled;
    int mapc_enabled;

    // Scenario
    int total_nodes_number;
    int total_wlans_number;

    // Physical Layer Models
    int pdf_backoff;            // 0: exponential, 1: deterministic
    int pdf_tx_time;
    int path_loss_model;
    int adjacent_channel_model;
    int collisions_model;
    double constant_per;
    int capture_effect_model;

    // Agents 
    int total_agents_number;
    int total_controlled_agents_number;
    
    // Action Spaces
    int num_arms_channel;
    int num_arms_sensitivity;
    int num_arms_tx_power;
    int num_arms_max_bandwidth;

    double* actions_pd;
    double* actions_tx_power;

};