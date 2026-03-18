/**
 * Set up the Komondor environment by reading the system input file (MS-DOS type)
 * @param "system_input_filename" [type char*]: filename of the system input CSV
 */
void Komondor::SetupEnvironmentByReadingConfigFile(const char *config_filename) {
    
    char delim[] = "=";
	char *ptr;
	int ix_param = 0;
	if (print_system_logs) printf("\n%s Reading system configuration file '%s'...\n", LOG_LVL1, config_filename);
	FILE* test_input_config = fopen(config_filename, "r");
	if (!test_input_config){
		printf("%s Test file '%s' not found!\n", LOG_LVL3, config_filename);
		exit(-1);
	}
	char line_system[CHAR_BUFFER_SIZE];
	while (fgets(line_system, CHAR_BUFFER_SIZE, test_input_config)){
		// Ignore lines with comments
		if(line_system[0] == '#') {
			continue;
		}
		// Separate the value of the parameter from the entire line
		ptr = strtok(line_system, delim);
		ptr = strtok(NULL, delim);
		// Store the parameter as a global variable
		if (ix_param == 0){
			// Path-loss model
			path_loss_model = atoi(ptr);
		} else if (ix_param == 1) {
			// Adjacent channel interference model
			adjacent_channel_model = atoi(ptr);
		} else if (ix_param == 2) {
			// Collisions model
			collisions_model = atoi(ptr);
		} else if (ix_param == 3) {
			// PDF tx time model
			pdf_tx_time = atoi(ptr);
		} else if (ix_param == 4) {
			// Simulation index (script's output)
			simulation_index = atoi(ptr);
		}
		ix_param++;
	}
	fclose(test_input_config);

	if (print_system_logs) printf("%s System environment properly set!\n", LOG_LVL2);
}

/**
 * Generate the nodes deterministically, according to the input nodes file
 * @param "nodes_filename" [type char*]: filename of the nodes input CSV
 */
void Komondor::GenerateNodesByReadingInputFile(const char *nodes_filename) {

    if (print_system_logs) printf("\n%s Generating nodes DETERMINISTICALLY through NODES input file...\n", LOG_LVL1);
    if (print_system_logs) printf("%s Reading nodes input file '%s'...\n", LOG_LVL2, nodes_filename);

    // ---------------------------------------------------------
    // 1. WLAN IDENTIFICATION AND ALLOCATION
    // ---------------------------------------------------------
    total_wlans_number = GetNumOfNodes(nodes_filename, NODE_TYPE_AP, ToString(""));
    if (print_system_logs) printf("%s Num. of WLANs detected: %d\n", LOG_LVL3, total_wlans_number);

    wlan_container = new Wlan[total_wlans_number];

    FILE* stream_nodes = fopen(nodes_filename, "r");
    if (!stream_nodes) {
        printf("ERROR: Could not open input file: %s\n", nodes_filename);
        exit(-1);
    }

    char line_nodes[CHAR_BUFFER_SIZE];
    int first_line_skiped_flag = 0;
    int wlan_ix = 0;
    char* tmp_nodes = NULL;

    // First Pass: Identify WLANs
    while (fgets(line_nodes, CHAR_BUFFER_SIZE, stream_nodes)){
        if(!first_line_skiped_flag){
            first_line_skiped_flag = 1;
        } else {
            tmp_nodes = strdup(line_nodes);
            int node_type = atoi(GetField(tmp_nodes, IX_NODE_TYPE));
            
            if(node_type == NODE_TYPE_AP){
                wlan_container[wlan_ix].wlan_id = wlan_ix;
                
                free(tmp_nodes); // Free before reusing
                tmp_nodes = strdup(line_nodes);
                std::string wlan_code_aux = ToString(GetField(tmp_nodes, IX_WLAN_CODE));
                
                wlan_container[wlan_ix].wlan_code = wlan_code_aux;
                wlan_container[wlan_ix].mapc_enabled = 0;
                wlan_container[wlan_ix].num_mapc_groups = 0;
                for (int mapc_g = 0; mapc_g < MAX_MAPC_GROUPS_PER_WLAN; ++mapc_g) {
                    wlan_container[wlan_ix].mapc_group_ids[mapc_g] = 0;
                    wlan_container[wlan_ix].mapc_method_ids[mapc_g] = 0;
                    wlan_container[wlan_ix].mapc_num_peers[mapc_g] = 0;
                    wlan_container[wlan_ix].mapc_peer_ap_ids[mapc_g] = NULL;
                    wlan_container[wlan_ix].mapc_txop_splits[mapc_g] = TXOP_SPLIT_EQUAL;
                    wlan_container[wlan_ix].mapc_sr_tx_power_dbm[mapc_g] = DEFAULT_COSR_TX_POWER_DBM;
                }
                ++wlan_ix;
            }
            free(tmp_nodes);
        }
    }
    fclose(stream_nodes);

    // ---------------------------------------------------------
    // 2. COUNT STAs PER WLAN
    // ---------------------------------------------------------
    for(int w = 0; w < total_wlans_number; ++w){
        int num_stas_in_wlan = GetNumOfNodes(nodes_filename, NODE_TYPE_STA, wlan_container[w].wlan_code);
        wlan_container[w].num_stas = num_stas_in_wlan;
        wlan_container[w].SetSizeOfSTAsArray(num_stas_in_wlan);
    }

    // ---------------------------------------------------------
    // 3. NODE GENERATION (The part that was missing)
    // ---------------------------------------------------------
    if (print_system_logs) printf("%s Generating nodes...\n", LOG_LVL3);
    
    total_nodes_number = GetNumOfNodes(nodes_filename, NODE_TYPE_UNKWNOW, ToString(""));
    node_container.SetSize(total_nodes_number);
    traffic_generator_container.SetSize(total_nodes_number);

    stream_nodes = fopen(nodes_filename, "r");
    int node_ix = 0;
    first_line_skiped_flag = 0;

    while (fgets(line_nodes, CHAR_BUFFER_SIZE, stream_nodes)){

        if(!first_line_skiped_flag){
            first_line_skiped_flag = 1;
        } else {
            // Node ID (auto-assigned)
            node_container[node_ix].node_params.node_id = node_ix;

            // Node code
            tmp_nodes = strdup(line_nodes);
            std::string node_code = ToString(GetField(tmp_nodes, IX_NODE_CODE));
            node_container[node_ix].node_params.node_code = node_code;
            free(tmp_nodes);

            // Node type
            tmp_nodes = strdup(line_nodes);
            int node_type = atoi(GetField(tmp_nodes, IX_NODE_TYPE));
            node_container[node_ix].node_params.node_type = node_type;
            free(tmp_nodes);

            // WLAN code & Linking
            tmp_nodes = strdup(line_nodes);
            const char *wlan_code_aux = GetField(tmp_nodes, IX_WLAN_CODE);
            std::string wlan_code;
            wlan_code.append(ToString(wlan_code_aux));
            node_container[node_ix].node_params.wlan_code = wlan_code;
            free(tmp_nodes);

            // Link logic
            for(int w = 0; w < total_wlans_number; ++w){
                if(strcmp(wlan_code.c_str(), wlan_container[w].wlan_code.c_str()) == 0){
                    if(node_container[node_ix].node_params.node_type == NODE_TYPE_AP){
                        wlan_container[w].ap_id = node_container[node_ix].node_params.node_id;
                    } else if (node_container[node_ix].node_params.node_type == NODE_TYPE_STA){
                        for(int s = 0; s < wlan_container[w].num_stas; ++s){
                            if(wlan_container[w].list_sta_id[s] == NODE_ID_NONE){
                                wlan_container[w].list_sta_id[s] = node_container[node_ix].node_params.node_id;
                                break;
                            }
                        }
                    }
                }
            }

            // Position
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.x = atof(GetField(tmp_nodes, IX_POSITION_X));
            free(tmp_nodes);
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.y = atof(GetField(tmp_nodes, IX_POSITION_Y));
            free(tmp_nodes);
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.z = atof(GetField(tmp_nodes, IX_POSITION_Z));
            free(tmp_nodes);

            // Frequency
            tmp_nodes = strdup(line_nodes);
            const char* central_frequency_char = GetField(tmp_nodes, IX_CENTRAL_FREQ);
            node_container[node_ix].node_params.central_frequency = atof(central_frequency_char) * pow(10,9);
            free(tmp_nodes);

            // Channel Bonding Model
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.current_dcb_policy = atoi(GetField(tmp_nodes, IX_CHANNEL_BONDING_MODEL));
            free(tmp_nodes);

            // Channels
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.current_primary_channel = atoi(GetField(tmp_nodes, IX_PRIMARY_CHANNEL));
            free(tmp_nodes);
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.min_channel_allowed = atoi(GetField(tmp_nodes, IX_MIN_CH_ALLOWED));
            free(tmp_nodes);
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.max_channel_allowed = atoi(GetField(tmp_nodes, IX_MAX_CH_ALLOWED));
            free(tmp_nodes);

            // Powers
            tmp_nodes = strdup(line_nodes);
            double tx_power_default_dbm = atof(GetField(tmp_nodes, IX_TX_POWER_DEFAULT));
            node_container[node_ix].node_params.tx_power_default = ConvertPower(DBM_TO_PW, tx_power_default_dbm);
            free(tmp_nodes);
            
            tmp_nodes = strdup(line_nodes);
            double sensitivity_default_dbm = atoi(GetField(tmp_nodes, IX_PD_DEFAULT));
            node_container[node_ix].node_params.sensitivity_default = ConvertPower(DBM_TO_PW, sensitivity_default_dbm);
            free(tmp_nodes);

            // Traffic
            tmp_nodes = strdup(line_nodes);
            const char* traffic_model_char = GetField(tmp_nodes, IX_TRAFFIC_MODEL);
            int traffic_model_val = atoi(traffic_model_char);
            free(tmp_nodes);

            tmp_nodes = strdup(line_nodes);
            const char* traffic_load_char = GetField(tmp_nodes, IX_TRAFFIC_LOAD);
            double traffic_load_val = atof(traffic_load_char);
            free(tmp_nodes);

            // Packet Length
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.frame_length = atoi(GetField(tmp_nodes, IX_PACKET_LENGTH));
            free(tmp_nodes);

            // Aggregation
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.max_num_packets_aggregated = atoi(GetField(tmp_nodes, IX_NUM_PACKETS_AGG));
            free(tmp_nodes);

            // Capture Effect
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.capture_effect_model = atoi(GetField(tmp_nodes, IX_CAPTURE_EFFECT_MODEL));
            free(tmp_nodes);
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.capture_effect = ConvertPower(DB_TO_LINEAR, atof(GetField(tmp_nodes, IX_CAPTURE_EFFECT_THR)));
            free(tmp_nodes);

            // PER & PIFS
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.constant_per = atof(GetField(tmp_nodes, IX_CONSTANT_PER));
            free(tmp_nodes);
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.pifs_activated = atoi(GetField(tmp_nodes, IX_PIFS_ACTIVATED));
            free(tmp_nodes);

            // Backoff
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.backoff_type = atoi(GetField(tmp_nodes, IX_BACKOFF_TYPE));
            free(tmp_nodes);
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.cw_adaptation = atoi(GetField(tmp_nodes, IX_CW_ADAPTATION_FLAG));
            free(tmp_nodes);
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.cw_min_default = atoi(GetField(tmp_nodes, IX_CW_MIN_DEFAULT));
            free(tmp_nodes);
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.cw_max_default = atoi(GetField(tmp_nodes, IX_CW_MAX_DEFAULT));
            free(tmp_nodes);
            tmp_nodes = strdup(line_nodes);
            node_container[node_ix].node_params.cw_stage_max = atoi(GetField(tmp_nodes, IX_CW_STAGE_MAX));
            free(tmp_nodes);

			// RTS/CTS mode
            tmp_nodes = strdup(line_nodes);
            const char* rts_cts_char = GetField(tmp_nodes, IX_RTS_CTS_ENABLED);
            node_container[node_ix].node_params.rts_cts_enabled = (rts_cts_char != NULL) ? atoi(rts_cts_char) : TRUE;
            free(tmp_nodes);

            // Spatial Reuse
            tmp_nodes = strdup(line_nodes);
            const char* bss_color_char = GetField(tmp_nodes, IX_BSS_COLOR);
            if (bss_color_char != NULL) {
                node_container[node_ix].node_params.bss_color = atoi(bss_color_char);
                free(tmp_nodes);

                tmp_nodes = strdup(line_nodes);
                node_container[node_ix].node_params.srg = atoi(GetField(tmp_nodes, IX_SRG));
                free(tmp_nodes);

                tmp_nodes = strdup(line_nodes);
                node_container[node_ix].node_params.non_srg_obss_pd = ConvertPower(DBM_TO_PW, atof(GetField(tmp_nodes, IX_NON_SRG_OBSS_PD)));
                free(tmp_nodes);

                tmp_nodes = strdup(line_nodes);
                node_container[node_ix].node_params.srg_obss_pd = ConvertPower(DBM_TO_PW, atof(GetField(tmp_nodes, IX_SRG_OBSS_PD)));
                free(tmp_nodes);
            } else {
                node_container[node_ix].node_params.bss_color = -1;
                node_container[node_ix].node_params.srg = -1;
                node_container[node_ix].node_params.non_srg_obss_pd = -1;
                node_container[node_ix].node_params.srg_obss_pd = -1;
                free(tmp_nodes);
            }

            // Beamforming (optional columns; defaults to disabled if absent)
            tmp_nodes = strdup(line_nodes);
            const char* bf_enabled_char = GetField(tmp_nodes, IX_BF_ENABLED);
            if (bf_enabled_char != NULL) {
                node_container[node_ix].node_params.beamforming_enabled = atoi(bf_enabled_char);
                free(tmp_nodes);

                tmp_nodes = strdup(line_nodes);
                node_container[node_ix].node_params.beam_N_elements = atoi(GetField(tmp_nodes, IX_BF_N_ELEMENTS));
                free(tmp_nodes);

                tmp_nodes = strdup(line_nodes);
                node_container[node_ix].node_params.beam_d_spacing = atof(GetField(tmp_nodes, IX_BF_D_SPACING));
                free(tmp_nodes);

                tmp_nodes = strdup(line_nodes);
                node_container[node_ix].node_params.beam_az_main_deg = atof(GetField(tmp_nodes, IX_BF_AZ_MAIN_DEG));
                free(tmp_nodes);
            } else {
                node_container[node_ix].node_params.beamforming_enabled = BEAMFORMING_DISABLED;
                node_container[node_ix].node_params.beam_N_elements     = DEFAULT_BEAM_N_ELEMENTS;
                node_container[node_ix].node_params.beam_d_spacing      = DEFAULT_BEAM_D_SPACING;
                node_container[node_ix].node_params.beam_az_main_deg    = 0.0;
                free(tmp_nodes);
            }

            // Global Models
            node_container[node_ix].node_params.simulation_time_komondor = simulation_time_komondor;
            node_container[node_ix].node_params.total_wlans_number = total_wlans_number;
            node_container[node_ix].node_params.total_nodes_number = total_nodes_number;
            node_container[node_ix].node_params.collisions_model = collisions_model;
            node_container[node_ix].node_params.save_node_logs = save_node_logs;
            node_container[node_ix].node_params.print_node_logs = print_node_logs;
            node_container[node_ix].node_params.adjacent_channel_model = adjacent_channel_model;
            node_container[node_ix].node_params.path_loss_model = path_loss_model;
            node_container[node_ix].node_params.pdf_tx_time = pdf_tx_time;
            node_container[node_ix].node_params.simulation_code = simulation_code;

            // Traffic Generator
            traffic_generator_container[node_ix].node_type = node_type;
            traffic_generator_container[node_ix].node_id = node_ix;
            traffic_generator_container[node_ix].traffic_model = traffic_model_val;
            node_container[node_ix].node_params.traffic_model = traffic_model_val;
            traffic_generator_container[node_ix].traffic_load = traffic_load_val;

            ++node_ix;
        }
    }
    fclose(stream_nodes);

    // ---------------------------------------------------------
    // 4. FINAL LINKING
    // ---------------------------------------------------------
    for(int n = 0; n < total_nodes_number; ++n){
        for(int w = 0; w < total_wlans_number; ++w){
            if (strcmp(node_container[n].node_params.wlan_code.c_str(), wlan_container[w].wlan_code.c_str()) == 0) {
                node_container[n].wlan = wlan_container[w];
            }
        }
    }

    if (print_system_logs) printf("%s Nodes generated!\n", LOG_LVL3);
}

/**
 * Return a field corresponding to a given index from a CSV file
 * @param "line" [type char*]: line of the CSV
 * @param "num" [type int]: field number (index)
 * @return "field" [type char*]: field corresponding to the introduced index
 */
const char* GetField(char* line, int num){
    const char* tok;
    for (tok = strtok(line, ";");
            tok && *tok;
            tok = strtok(NULL, ";\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

/**
 * Return the number of lines of a csv file
 * @param "filename" [type char*]: CSV filename
 * @return "num_lines" [type int]: number of lines in the csv file
 */
int Komondor :: GetNumOfLines(const char *filename){
	int num_lines (0);
	// Nodes file
	FILE* stream = fopen(filename, "r");
	if (!stream){
		printf("Nodes configuration file %s not found!\n", filename);
		exit(-1);
	}
	char line[CHAR_BUFFER_SIZE];
	while (fgets(line, CHAR_BUFFER_SIZE, stream))
	{
		++num_lines;
	}
	num_lines--;
	fclose(stream);
	return num_lines;
}

/**
 * Return the number of nodes of a given type (0: AP, 1: STA, 2: Free Node)
 * @param "nodes_filename" [type char*]: nodes configuration filename
 * @param "node_type" [type int]: type of node to consider in the counting
 * @param "wlan_code" [type std::string]: code of the wlan to consider in the counting
 * @return "num_nodes" [type int]: number of nodes of the introduced type in the indicated WLAN
 */
int Komondor :: GetNumOfNodes(const char *nodes_filename, int node_type, std::string wlan_code){

	int num_nodes(0);
	char line_nodes[CHAR_BUFFER_SIZE];
	char* tmp_nodes;
	first_line_skiped_flag = 0;
	int type_found;
	std::string wlan_code_found;

	FILE* stream_nodes = fopen(nodes_filename, "r");

	if (!stream_nodes){
		printf("[MAIN] ERROR: Nodes configuration file %s not found!\n", nodes_filename);
		exit(-1);
	}

	if(node_type == NODE_TYPE_UNKWNOW){	// Count all type of nodes

		num_nodes = GetNumOfLines(nodes_filename);

	} else {	// Count specific nodes

		while (fgets(line_nodes, CHAR_BUFFER_SIZE, stream_nodes)){

			if(!first_line_skiped_flag){

				first_line_skiped_flag = 1;

			} else{

				// Node type
				tmp_nodes = strdup(line_nodes);
				type_found = atof(GetField(tmp_nodes, IX_NODE_TYPE));
				free(tmp_nodes);

				// WLAN code
				tmp_nodes = strdup(line_nodes);
				wlan_code_found = ToString(GetField(tmp_nodes, IX_WLAN_CODE));
				free(tmp_nodes);

				if(wlan_code.compare(ToString("")) > 0){
					if(type_found == node_type && strcmp(wlan_code_found.c_str(), wlan_code.c_str()) == 0) ++num_nodes;
				} else {
					if(type_found == node_type) ++num_nodes;
				}
			}
		}
	}

	fclose(stream_nodes);
	return num_nodes;
}

/**
* Generate the MAPC configuration, according to the MAPC config file
 * @param "mapc_filename" [type char*]: filename of the MAPC input CSV
*/
void Komondor::GenerateMapcConfiByReadingInputFile(const char *mapc_filename) {

    FILE* stream = fopen(mapc_filename, "r");
    if (!stream) {
        printf("MAPC file %s not found!\n", mapc_filename);
        //exit(-1);
    } else {
		if (print_system_logs) printf("\n%s Reading MAPC configuration file '%s'...\n", LOG_LVL1, mapc_filename);
		char line[CHAR_BUFFER_SIZE];
		int first_line_skipped = 0;
		char* tmp_line;

		while (fgets(line, CHAR_BUFFER_SIZE, stream)) {
			if (!first_line_skipped) { first_line_skipped = 1; continue; } // Skip header
			tmp_line = strdup(line);
			// 1. Parse Group ID and Method
			int group_id = atoi(GetField(tmp_line, IX_MAPC_GROUP_ID));
			// 2. Parse the MAPC scheme
			tmp_line = strdup(line);
			std::string method_str = ToString(GetField(tmp_line, IX_MAPC_METHOD));
			int method_id = 0; 
			if (method_str == "CO_SR") {
				method_id = CO_SR;
			} else if (method_str == "CO_BF") {
				method_id = CO_BF;
			} else if (method_str == "CO_TDMA") {
				method_id = CO_TDMA;
			} else if (method_str == "CO_RTWT") {
				method_id = CO_RTWT;
			}
			// 3. Parse coordinated APs (handle comma-separated list)
			tmp_line = strdup(line);
			std::string coordinated_ids_str = ToString(GetField(tmp_line, IX_MAPC_AP_IDS));
			// Use string comparison so wlan_codes like "A","B" match correctly.
			// atoi("A") = atoi("B") = 0 would cause false matches for all WLANs.
			std::vector<std::string> coordinated_ap_list;
			char* token = strtok((char*)coordinated_ids_str.c_str(), ",");
			while (token != NULL) {
				coordinated_ap_list.push_back(std::string(token));
				token = strtok(NULL, ",");
			}
			// Update MAPC information of involved WLANs
			for (size_t c = 0; c < coordinated_ap_list.size(); ++c) {
				for (int w = 0; w < total_wlans_number; ++w) {
					if (wlan_container[w].wlan_code == coordinated_ap_list[c]) {
						if (wlan_container[w].num_mapc_groups < MAX_MAPC_GROUPS_PER_WLAN) {
							int mapc_gidx = wlan_container[w].num_mapc_groups++;
							wlan_container[w].mapc_enabled = 1;
							wlan_container[w].mapc_group_ids[mapc_gidx] = group_id;
							wlan_container[w].mapc_method_ids[mapc_gidx] = method_id;
							wlan_container[w].mapc_txop_splits[mapc_gidx] = TXOP_SPLIT_EQUAL;
						}
					}
				}
			}
			
			// 4. Parse per-scheme parameters (key-value pairs for extensibility)
			tmp_line = strdup(line);
			std::string params = ToString(GetField(tmp_line, IX_MAPC_EXTRA_PARAM));
			free(tmp_line);

			// --- APPLY LOGIC BASED ON METHOD ---
			if (method_str == "CO_SR") {
				// Option A: read TX power limit from ExtraParams [dBm]; fall back to default if absent
				double sr_pwr = DEFAULT_COSR_TX_POWER_DBM;
				if (!params.empty()) sr_pwr = atof(params.c_str());
				// Apply to every WLAN registered in this group
				for (size_t c = 0; c < coordinated_ap_list.size(); ++c) {
					for (int w = 0; w < total_wlans_number; ++w) {
						if (wlan_container[w].wlan_code == coordinated_ap_list[c]) {
							int g_idx = wlan_container[w].FindMapcGroupIdx(group_id);
							if (g_idx >= 0)
								wlan_container[w].mapc_sr_tx_power_dbm[g_idx] = sr_pwr;
						}
					}
				}

			} else if (method_str == "CO_TDMA") {
				// Configure nodes for Co-TDMA
				// To do

				// Parse specific Co-TDMA params (e.g., SLOT_DURATION)
				// To do

			} else if (method_str == "CO_BF") {
				// ...
			}
		}
		fclose(stream);
		// Build peer AP ID lists for each MAPC-enabled WLAN, per group
		int *tmp_peers = new int[total_wlans_number];
		for (int w = 0; w < total_wlans_number; ++w) {
			if (!wlan_container[w].mapc_enabled) continue;
			for (int g = 0; g < wlan_container[w].num_mapc_groups; ++g) {
				int count = 0;
				for (int w2 = 0; w2 < total_wlans_number; ++w2) {
					if (w2 == w) continue;
					if (wlan_container[w2].FindMapcGroupIdx(wlan_container[w].mapc_group_ids[g]) >= 0) {
						tmp_peers[count++] = wlan_container[w2].ap_id;
					}
				}
				wlan_container[w].mapc_num_peers[g] = count;
				wlan_container[w].mapc_peer_ap_ids[g] = new int[count];
				for (int i = 0; i < count; ++i)
					wlan_container[w].mapc_peer_ap_ids[g][i] = tmp_peers[i];
			}
		}
		delete[] tmp_peers;
		if (print_system_logs) printf("%s MAPC Groups configured!\n", LOG_LVL2);
	}
}

/**
 * Generate the agents deterministically, according to the input agents file
 * @param "agents_filename" [type char*]: filename of the agents input CSV
 */
void Komondor :: GenerateAgents(const char *agents_filename, const char *simulation_code_console) {

	if (print_system_logs) printf("%s Generating agents...\n", LOG_LVL1);
	if (print_system_logs) printf("%s Reading agents input file '%s'...\n", LOG_LVL2, agents_filename);

	// STEP 1: CHECK IF THERE IS A CC AND PARSE ITS INFORMATION DIFFERENTLY THAN FROM AGENTS
	central_controller_flag = CheckCentralController(agents_filename);
	// STEP 2: SET SIZE OF THE AGENTS CONTAINER
	total_agents_number = GetNumOfLines(agents_filename) - central_controller_flag;
	agent_container.SetSize(total_agents_number);
	if (print_system_logs) printf("%s Num. of agents (WLANs): %d/%d\n", LOG_LVL3, total_agents_number, total_wlans_number);
	// STEP 3: read the input file to determine the action space
	if (print_system_logs) printf("%s Setting action space...\n", LOG_LVL4);
	FILE* stream_agents = fopen(agents_filename, "r");
	char line_agents[CHAR_BUFFER_SIZE];
	first_line_skiped_flag = 0;	// Flag for skipping first informative line of input file
	int agent_ix (0);	// Auxiliary index
	while (fgets(line_agents, CHAR_BUFFER_SIZE, stream_agents)){
		if(!first_line_skiped_flag){	// Skip the first line of the .csv file
			first_line_skiped_flag = 1;
		} else{
			char* tmp_agents = strdup(line_agents);
			const char *wlan_code_aux (GetField(tmp_agents, IX_AGENT_WLAN_CODE));
			std::string wlan_code;
			wlan_code.append(ToString(wlan_code_aux));
			// Skip the line in case we find a Central Controller (CC). Otherwise, read it and initialize the agent
			if (strcmp(wlan_code.c_str(), "NULL") == 0) continue;
			// Find the length of the channel actions array
			tmp_agents = strdup(line_agents);
			const char *channel_values_aux (GetField(tmp_agents, IX_AGENT_CHANNEL_VALUES));
			std::string channel_values_text;
			channel_values_text.append(ToString(channel_values_aux));
			const char *channel_aux;
			channel_aux = strtok ((char*)channel_values_text.c_str(),",");
			num_arms_channel = 0;
			while (channel_aux != NULL) {
				channel_aux = strtok (NULL, ",");
				++ num_arms_channel;
			}
			// Set the length of channel actions to agent's field
			agent_container[agent_ix].num_arms_channel = num_arms_channel;
			// Find the length of the pd actions array
			tmp_agents = strdup(line_agents);
			const char *pd_values_aux (GetField(tmp_agents, IX_AGENT_PD_VALUES));
			std::string pd_values_text;
			pd_values_text.append(ToString(pd_values_aux));
			const char *pd_aux;
			pd_aux = strtok ((char*)pd_values_text.c_str(),",");
			num_arms_sensitivity = 0;
			while (pd_aux != NULL) {
				pd_aux = strtok (NULL, ",");
				++ num_arms_sensitivity;
			}
			// Set the length of sensitivity actions to agent's field
			agent_container[agent_ix].num_arms_sensitivity = num_arms_sensitivity;
			// Find the length of the Tx power actions array
			tmp_agents = strdup(line_agents);
			const char *tx_power_values_aux (GetField(tmp_agents, IX_AGENT_TX_POWER_VALUES));
			std::string tx_power_values_text;
			tx_power_values_text.append(ToString(tx_power_values_aux));
			const char *tx_power_aux;
			tx_power_aux = strtok ((char*)tx_power_values_text.c_str(),",");
			num_arms_tx_power = 0;
			while (tx_power_aux != NULL) {
				tx_power_aux = strtok (NULL, ",");
				++ num_arms_tx_power;
			}
			// Set the length of Tx power actions to agent's field
			agent_container[agent_ix].num_arms_tx_power = num_arms_tx_power;
			// Find the length of the DCB actions actions array
			tmp_agents = strdup(line_agents);
			const char *max_bandwidth_values_aux (GetField(tmp_agents, IX_AGENT_MAX_BANDWIDTH));
			std::string max_bandwidth_values_text;
			max_bandwidth_values_text.append(ToString(max_bandwidth_values_aux));
			const char *max_bandwidth_aux;
			max_bandwidth_aux = strtok ((char*)max_bandwidth_values_text.c_str(),",");
			num_arms_max_bandwidth = 0;
			while (max_bandwidth_aux != NULL) {
				max_bandwidth_aux = strtok (NULL, ",");
				++num_arms_max_bandwidth;
			}
			// Set the length of max bandwidth to agent's field
			agent_container[agent_ix].num_arms_max_bandwidth = num_arms_max_bandwidth;

			// Set the length of the total actions in the agent (combinations of parameters)
			agent_container[agent_ix].num_arms = num_arms_channel * num_arms_sensitivity
				* num_arms_tx_power * num_arms_max_bandwidth;

			// Set the simulation code for generating output files
			agent_container[agent_ix].simulation_code.append(ToString(simulation_code_console));

			++agent_ix;
			free(tmp_agents);
		}
	}

	if (print_system_logs) printf("%s Action space set!\n", LOG_LVL4);

	// STEP 3: set agents parameters
	if (print_system_logs) printf("%s Setting agents parameters...\n", LOG_LVL4);
	stream_agents = fopen(agents_filename, "r");
	first_line_skiped_flag = 0;		// Flag for skipping first informative line of input file
	agent_ix = 0;	// Auxiliary index
	total_controlled_agents_number = 0;
	while (fgets(line_agents, CHAR_BUFFER_SIZE, stream_agents)){
		if(!first_line_skiped_flag){	// Skip the first line of the .csv file
			first_line_skiped_flag = 1;
		} else{
			// WLAN code
			char* tmp_agents (strdup(line_agents));
			const char *wlan_code_aux (GetField(tmp_agents, IX_AGENT_WLAN_CODE));
			std::string wlan_code;
			wlan_code.append(ToString(wlan_code_aux));
			// Skip the line in case we find a Central Controller (CC). Otherwise, read it and initialize the agent
			if (strcmp(wlan_code.c_str(), "NULL") == 0) {
				continue;
			} else {
				// Agent ID
				agent_container[agent_ix].agent_id = agent_ix;
				agent_container[agent_ix].wlan_code = wlan_code.c_str();
				// WLAN Id — skip agent if no matching WLAN exists (prevents null outport crash)
				int wlan_found_flag(0);
				for(int w=0; w < total_wlans_number; ++w){
					if(strcmp(wlan_container[w].wlan_code.c_str(), agent_container[agent_ix].wlan_code.c_str()) == 0) {
						agent_container[agent_ix].wlan_id = w;
						wlan_found_flag = 1;
					}
				}
				if (!wlan_found_flag) {
					if (print_system_logs) printf("%s Agent for WLAN '%s' skipped (no matching WLAN)\n",
						LOG_LVL3, wlan_code.c_str());
					--total_agents_number;
					free(tmp_agents);
					continue;
				}
				// Initialize actions and arrays in agents
				agent_container[agent_ix].InitializeAgent();
				//  Agent associated to the Central Controller (CC)
				tmp_agents = strdup(line_agents);
				int agent_centralized (atoi(GetField(tmp_agents, IX_COMMUNICATION_LEVEL)));
				agent_container[agent_ix].agent_centralized = agent_centralized;
				// Check if the central controller has to be created or not
				if(agent_centralized) ++total_controlled_agents_number;
				// Time between requests (in seconds)
				tmp_agents = strdup(line_agents);
				double time_between_requests (atof(GetField(tmp_agents, IX_AGENT_TIME_BW_REQUESTS)));
				agent_container[agent_ix].time_between_requests = time_between_requests;
				// Channel values
				tmp_agents = strdup(line_agents);
				std::string channel_values_text = ToString(GetField(tmp_agents, IX_AGENT_CHANNEL_VALUES));
				// Fill the channel actions array
				char *channel_aux_2;
				char *channel_values_text_char = new char[channel_values_text.length() + 1];
				strcpy(channel_values_text_char, channel_values_text.c_str());
				channel_aux_2 = strtok (channel_values_text_char,",");
				int ix (0);
				while (channel_aux_2 != NULL) {
					int a (atoi(channel_aux_2));
					agent_container[agent_ix].list_of_channels[ix] = a;
					channel_aux_2 = strtok (NULL, ",");
					++ix;
				}
				// If no channel actions are provided, use the primary and range already assigned to the BSS
				if (agent_container[agent_ix].num_arms_channel == 1) {
					int wlan_id_aux(agent_container[agent_ix].wlan_id);
					agent_container[agent_ix].list_of_channels[0] = node_container[wlan_container[wlan_id_aux].ap_id].node_params.current_primary_channel;
				}
				// sensitivity values
				tmp_agents = strdup(line_agents);
				std::string pd_values_text = ToString(GetField(tmp_agents, IX_AGENT_PD_VALUES));
				// Fill the sensitivity actions array
				char *pd_aux_2;
				char *pd_values_text_char = new char[pd_values_text.length() + 1];
				strcpy(pd_values_text_char, pd_values_text.c_str());
				pd_aux_2 = strtok (pd_values_text_char,",");
				ix = 0;
				while (pd_aux_2 != NULL) {
					int a = atoi(pd_aux_2);
					agent_container[agent_ix].list_of_pd_values[ix] = ConvertPower(DBM_TO_PW, a);
					pd_aux_2 = strtok (NULL, ",");
					++ix;
				}
				// Tx Power values
				tmp_agents = strdup(line_agents);
				std::string tx_power_values_text = ToString(GetField(tmp_agents, IX_AGENT_TX_POWER_VALUES));
				// Fill the TX power actions array
				char *tx_power_aux_2;
				char *tx_power_values_text_char = new char[tx_power_values_text.length() + 1];
				strcpy(tx_power_values_text_char, tx_power_values_text.c_str());
				tx_power_aux_2 = strtok (tx_power_values_text_char,",");
				ix = 0;
				while (tx_power_aux_2 != NULL) {
					int a (atoi(tx_power_aux_2));
					agent_container[agent_ix].list_of_tx_power_values[ix] = ConvertPower(DBM_TO_PW, a);
					tx_power_aux_2 = strtok (NULL, ",");
					++ix;
				}
				// Max bandwidth values
				tmp_agents = strdup(line_agents);
				std::string max_bandwidth_values_text = ToString(GetField(tmp_agents, IX_AGENT_MAX_BANDWIDTH));
				// Fill the max bandwidth actions array
				char *max_bandwidth_aux_2;
				char *max_bandwidth_values_text_char = new char[max_bandwidth_values_text.length() + 1];
				strcpy(max_bandwidth_values_text_char, max_bandwidth_values_text.c_str());
				max_bandwidth_aux_2 = strtok (max_bandwidth_values_text_char,",");
				ix = 0;
				while (max_bandwidth_aux_2 != NULL) {
					int a (atoi(max_bandwidth_aux_2));
					agent_container[agent_ix].list_of_max_bandwidth[ix] = a;
					max_bandwidth_aux_2 = strtok (NULL, ",");
					++ix;
				}
				// Type of reward
				tmp_agents = strdup(line_agents);
				int type_of_reward (atoi(GetField(tmp_agents, IX_AGENT_TYPE_OF_REWARD)));
				agent_container[agent_ix].type_of_reward = type_of_reward;
				// Learning mechanism
				tmp_agents = strdup(line_agents);
				int learning_mechanism (atoi(GetField(tmp_agents, IX_AGENT_LEARNING_MECHANISM)));
				agent_container[agent_ix].learning_mechanism = learning_mechanism;
				// Selected strategy
				tmp_agents = strdup(line_agents);
				int action_selection_strategy (atoi(GetField(tmp_agents, IX_AGENT_SELECTED_STRATEGY)));
				agent_container[agent_ix].action_selection_strategy = action_selection_strategy;
				// Other information
				agent_container[agent_ix].save_agent_logs = save_agent_logs;
				agent_container[agent_ix].print_agent_logs = print_agent_logs;
				agent_container[agent_ix].num_stas = wlan_container[agent_container[agent_ix].wlan_id].num_stas;
				// TRICKY - USE THE FIRST ELEMENT INT HE LIST OF PD VALUES AS THE MARGIN
				if(agent_container[agent_ix].learning_mechanism == RTOT_ALGORITHM) {
					agent_container[agent_ix].margin_rtot = agent_container[agent_ix].list_of_pd_values[0];
				}

				//agent_container[agent_ix].PrintAgentInfo();

				++agent_ix;

			}
			free(tmp_agents);
		}
	}
	if (print_system_logs) printf("%s Agents parameters set!\n", LOG_LVL4);

}

/**
 * Generate the central controller (if active), according to the input agents file
 * @param "agents_filename" [type char*]: filename of the agents input CSV
 */
void Komondor :: GenerateCentralController(const char *agents_filename) {

	if (print_system_logs) printf("%s Generating the Central Controller...\n", LOG_LVL1);
	// So far, we consider a single controller. For scalability purposes, the CC must be declared as an array
	if (central_controller_flag) central_controller.SetSize(1);
	if (total_controlled_agents_number > 0) {	// Check that the CC has one or more agents attached
		central_controller[0].controller_on = TRUE;
		central_controller[0].agents_number = total_controlled_agents_number;
		central_controller[0].wlans_number = total_wlans_number;
		int max_number_of_actions(0);
		for (int agent_ix = 0; agent_ix < total_controlled_agents_number; ++agent_ix) {
			if(agent_container[agent_ix].num_arms > max_number_of_actions) max_number_of_actions = agent_container[agent_ix].num_arms;
		}
		central_controller[0].max_number_of_actions = max_number_of_actions;
		// Initialize the CC
		central_controller[0].InitializeCentralController();
		int *agents_list;
		agents_list = new int[total_controlled_agents_number];
		int agent_list_ix (0);					// Index considering the agents attached to the central entity

//		double max_time_between_requests (0);	// To determine the maximum time between requests for agents
		// Check which agents are attached to the central controller
		for (int agent_ix = 0; agent_ix < total_controlled_agents_number; ++agent_ix) {
			if(agent_container[agent_ix].agent_centralized) {
				// Add agent id to list of agents attached to the controller
				agents_list[agent_list_ix] = agent_container[agent_ix].agent_id;
				agent_container[agent_ix].controller_on = central_controller[0].controller_on;
				central_controller[0].num_arms_per_agent[agent_ix] = agent_container[agent_ix].num_arms;
//				double agent_time_between_requests (agent_container[agent_list_ix].time_between_requests);
//				// Store the maximum time between requests
//				if (agent_time_between_requests > max_time_between_requests) {
//					central_controller[0].time_between_requests = agent_time_between_requests;
//				}
				++agent_list_ix;
			}
		}

		// The overall "time between requests" is set to the maximum among all the agents
		central_controller[0].list_of_agents = agents_list;
		// Initialize the CC with parameters from the agents input file
		FILE* stream_cc = fopen(agents_filename, "r");
		char line_agents[CHAR_BUFFER_SIZE];
		char* tmp_cc (strdup(line_agents));
		first_line_skiped_flag = 0;		// Flag for skipping first informative line of input file
		while (fgets(line_agents, CHAR_BUFFER_SIZE, stream_cc)){
			if(!first_line_skiped_flag){
				first_line_skiped_flag = 1;
			} else{
				tmp_cc = strdup(line_agents);
				const char *wlan_code_aux (GetField(tmp_cc, IX_AGENT_WLAN_CODE));
				std::string wlan_code;
				wlan_code.append(ToString(wlan_code_aux));
				// Skip the line in case we find a Central Controller (CC). Otherwise, read it and initialize the agent
				if (strcmp(wlan_code.c_str(), "NULL") == 0) {
					// Time between requests
					tmp_cc = strdup(line_agents);
					double time_between_requests (atoi(GetField(tmp_cc, IX_AGENT_TIME_BW_REQUESTS)));
					central_controller[0].time_between_requests = time_between_requests;
					// Type of reward
					tmp_cc = strdup(line_agents);
					int type_of_reward (atoi(GetField(tmp_cc, IX_AGENT_TYPE_OF_REWARD)));
					central_controller[0].type_of_reward = type_of_reward;
					// Learning mechanism
					tmp_cc = strdup(line_agents);
					int learning_mechanism (atoi(GetField(tmp_cc, IX_AGENT_LEARNING_MECHANISM)));
					central_controller[0].learning_mechanism = learning_mechanism;
					// Selected strategy
					tmp_cc = strdup(line_agents);
					int action_selection_strategy (atoi(GetField(tmp_cc, IX_AGENT_SELECTED_STRATEGY)));
					central_controller[0].action_selection_strategy = action_selection_strategy;
					// Find the length of the channel actions array
					tmp_cc = strdup(line_agents);
					const char *channel_values_aux (GetField(tmp_cc, IX_AGENT_CHANNEL_VALUES));
					std::string channel_values_text;
					channel_values_text.append(ToString(channel_values_aux));
					const char *channels_aux;
					channels_aux = strtok ((char*)channel_values_text.c_str(),",");
					int num_arms_channels = 0;
					while (channels_aux != NULL) {
						channels_aux = strtok (NULL, ",");
						++ num_arms_channels;
					}
					free(tmp_cc);
					break;	// Don't read all the other lines (entailed for agents)
				} else {
					continue; // Keep reading until finding "NULL", which indicated the CC line
				}
			}
		}

		// System logs
		central_controller[0].save_controller_logs = save_agent_logs;
		central_controller[0].print_controller_logs = print_agent_logs;
		central_controller[0].total_nodes_number = total_nodes_number;

	} else {
		printf("%s WARNING: THE CENTRAL CONTROLLER DOES NOT HAVE ANY ATTACHED AGENT! CHECK YOUR AGENTS' INPUT FILE\n", LOG_LVL2);
		central_controller[0].controller_on = FALSE;
	}

	central_controller[0].PrintControllerInfo();

}

/**
 * Return TRUE if there is a Central Controller declared. FALSE, otherwise.
 * The CC is declared in any line by setting the WLAN_CODE field to "NULL"
 * @param "nodes_filename" [type char*]: nodes configuration filename
 * @return "presence_central_cotnroller" [type bool]: flag indicating whether a CC is present or not
 */
int Komondor :: CheckCentralController(const char *agents_filename){
	int presence_central_cotnroller(FALSE);
	FILE* stream_agents = fopen(agents_filename, "r");
	char line_agents[CHAR_BUFFER_SIZE];
	first_line_skiped_flag = 0;	// Flag for skipping first informative line of input file
	while (fgets(line_agents, CHAR_BUFFER_SIZE, stream_agents)){
		if(!first_line_skiped_flag){	// Skip the first line of the .csv file
			first_line_skiped_flag = 1;
		} else{
			// WLAN code
			char* tmp_agents (strdup(line_agents));
			const char *wlan_code_aux (GetField(tmp_agents, IX_AGENT_WLAN_CODE));
			std::string wlan_code;
			wlan_code.append(ToString(wlan_code_aux));
			// Skip the line in case we find a Central Controller (CC). Otherwise, read it and initialize the agent
			if (strcmp(wlan_code.c_str(), "NULL") == 0) {
				presence_central_cotnroller = TRUE;
			}
		}
	}
	return presence_central_cotnroller;
}