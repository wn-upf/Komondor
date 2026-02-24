/***************************/
/* LOG AND DEBUG FUNCTIONS */
/***************************/

/**
 * Print the Komondor's environment general information
 */
void Komondor :: PrintSystemInfo(){
	if (print_system_logs){
		printf("%s total_nodes_number = %d\n", LOG_LVL3, total_nodes_number);
		printf("%s pdf_tx_time = %d\n", LOG_LVL3, pdf_tx_time);
		printf("%s path_loss_model = %d\n", LOG_LVL3, path_loss_model);
		printf("%s adjacent_channel_model = %d\n", LOG_LVL3, adjacent_channel_model);
		printf("%s collisions_model = %d\n", LOG_LVL3, collisions_model);
		printf("\n");
	}
}

/**
 * Write the Komondor's environment general information into an output file
 * @param "logger" [type Logger]: logger object that writes logs into a file
 */
void Komondor :: WriteSystemInfo(Logger logger){
	fprintf(logger.file, "%s total_nodes_number = %d\n", LOG_LVL3, total_nodes_number);
	fprintf(logger.file, "%s pdf_tx_time = %d\n", LOG_LVL3, pdf_tx_time);
	fprintf(logger.file, "%s path_loss_model = %d\n", LOG_LVL3, path_loss_model);
	fprintf(logger.file, "%s adjacent_channel_model = %d\n", LOG_LVL3, adjacent_channel_model);
	fprintf(logger.file, "%s collisions_model = %d\n", LOG_LVL3, collisions_model);
}

/**
 * Print the information of all the nodes
 * @param "info_detail_level" [type int]: level of detail of the written logs
 */
void Komondor :: PrintAllNodesInfo(int info_detail_level){
	for(int n = 0; n < total_nodes_number; ++n ){
		node_container[n].PrintNodeInfo(info_detail_level);
	}
}

/**
 * Print the information of all the WLANs
 */
void Komondor :: PrintAllWlansInfo(){
	for(int w = 0; w < total_wlans_number; ++w){
		wlan_container[w].PrintWlanInfo();
	}
}

/**
 * Print the information of all the agents
 */
void Komondor :: PrintMlOperationInfo(){
	printf("%s Agents generated!\n\n", LOG_LVL2);
	for(int a = 0; a < total_agents_number; ++a ){
		agent_container[a].PrintAgentInfo();
	}
	if (central_controller_flag) {
		printf("%s Central Controller generated!\n\n", LOG_LVL2);
		central_controller[0].PrintControllerInfo();
	}
}

/**
 * Write the information of all the WLANs into a file
 * @param "logger" [type Logger]: logger object that writes logs into a file
 * @param "header_str" [type std::string]: header string
 */
void Komondor :: WriteAllWlansInfo(Logger logger, std::string header_str){
	for(int w = 0; w < total_wlans_number; ++w){
		wlan_container[w].WriteWlanInfo(logger, header_str.c_str());
	}
}

/**
 * Write the information of all the nodes into a file
 * @param "logger" [type Logger]: logger object that writes logs into a file
 * @param "info_detail_level" [type int]: level of detail of the written logs
 * @param "header_str" [type std::string]: header string
 */
void Komondor :: WriteAllNodesInfo(Logger logger, int info_detail_level, std::string header_str){
	for(int n = 0; n < total_nodes_number; ++n){
		node_container[n].WriteNodeInfo(logger, info_detail_level, header_str.c_str());
	}
}

/**
 * Write the information of all the agents into a file
 * @param "logger" [type Logger]: logger object that writes logs into a file
 * @param "header_str" [type std::string]: header string
 */
void Komondor :: WriteAllAgentsInfo(Logger logger, std::string header_str){
	for(int n = 0; n < total_agents_number; ++n){
		agent_container[n].WriteAgentInfo(logger, header_str.c_str());
	}
	if (central_controller_flag) {
		central_controller[0].WriteControllerInfo(logger);
	}
}