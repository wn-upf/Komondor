#include <vector>
#include <cstdio>
#include <cstdlib> // For exit()

#ifndef INPUT_VALIDATOR
#define INPUT_VALIDATOR

/**
* Identify errors in the introduced input to prevent unexpected situations
* @param "engine" [type Komondor&]: Reference to the main simulation engine
*/
template<typename NodeContainer>
void ValidateInput(int total_nodes_number, NodeContainer& node_container, int print_system_logs) {

    std::vector<int> nodes_ids(total_nodes_number);
    std::vector<double> nodes_x(total_nodes_number);
    std::vector<double> nodes_y(total_nodes_number);
    std::vector<double> nodes_z(total_nodes_number);

    if (print_system_logs) {
        printf("%s Validating input files...\n", LOG_LVL2);
    }

    for (int i = 0; i < total_nodes_number; ++i) {
        // Cache values for duplicate checking later
        nodes_ids[i] = node_container[i].node_params.node_id;
        nodes_x[i]   = node_container[i].node_params.x;
        nodes_y[i]   = node_container[i].node_params.y;
        nodes_z[i]   = node_container[i].node_params.z;

        // Check 1: Transmission Power
        double tx_def = node_container[i].node_params.tx_power_default;
        // Note: ensuring ConvertPower is accessible. If it's a member of Komondor, use engine.ConvertPower
        if (tx_def > ConvertPower(DBM_TO_PW, MAX_TX_POWER_DBM) || 
            tx_def < ConvertPower(DBM_TO_PW, MIN_TX_POWER_DBM)) {
            
            printf("\nERROR: tx_power values are not properly configured at node in line %d\n"
                   "MIN_TX_POWER_DBM = %d\n"
                   "node_container[i].node_params.tx_power_default = %f\n"
                   "MAX_TX_POWER_DBM = %d\n\n",
                   i + 2, MIN_TX_POWER_DBM, ConvertPower(PW_TO_DBM, tx_def), MAX_TX_POWER_DBM);
            exit(-1);
        }

        // Check 2: Sensitivity
        if (node_container[i].node_params.sensitivity_default > ConvertPower(DBM_TO_PW, MAX_SENSITIVITY_DBM)) {
            printf("\nERROR: sensitivity values are not properly configured at node in line %d\n\n", i + 2);
            exit(-1);
        }

        // Check 3: Channels
        if (node_container[i].node_params.current_primary_channel > node_container[i].node_params.max_channel_allowed ||
            node_container[i].node_params.current_primary_channel < node_container[i].node_params.min_channel_allowed ||
            node_container[i].node_params.min_channel_allowed > node_container[i].node_params.max_channel_allowed ||
            node_container[i].node_params.current_primary_channel > NUM_CHANNELS_KOMONDOR ||
            node_container[i].node_params.min_channel_allowed > (NUM_CHANNELS_KOMONDOR - 1) ||
            node_container[i].node_params.max_channel_allowed > (NUM_CHANNELS_KOMONDOR - 1)) {
            
            printf("\nERROR: Channels are not properly configured at node in line %d\n\n", i + 2);
            exit(-1);
        }
    }

    // Check 4: Duplicates
    for (int i = 0; i < total_nodes_number; ++i) {
        for (int j = 0; j < total_nodes_number; ++j) {
            if (i != j && i < j) {
                // Duplicate IDs
                if (nodes_ids[i] == nodes_ids[j]) {
                    printf("\nERROR: Nodes in lines %d and %d have the same ID\n\n", i + 2, j + 2);
                    exit(-1);
                }
                // Duplicate Position
                if (nodes_x[i] == nodes_x[j] && nodes_y[i] == nodes_y[j] && nodes_z[i] == nodes_z[j]) {
                    printf("%s \nERROR: Nodes in lines %d and %d are exactly at the same position\n\n", LOG_LVL2, i + 2, j + 2);
                    exit(-1);
                }
            }
        }
    }

    if (print_system_logs) {
        printf("%s Input files validated!\n", LOG_LVL3);
    }

}

#endif // INPUT_VALIDATOR
