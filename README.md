# VaquitaSimulator

To run Vaquita simulator, just build the project by using the build_local script and then execute it by doing:

$ ./VaquitaSimulation.cc SIM_TIME SEED

Where SIM_TIME is the simulation time and SEED is the random seed you wish to use.

There are two input files located at the "input" folder at which you can configure system and nodes parameters, respectively:
	* input_system_conf.csv: define parameters such as the number of total available channels, the CW...
	* input_nodes_conf.csv: define parameters such as the node id, the node location, etc.
	
Regarding the output ("output" folder), some logs and statistics are created at the end of the execution.

Set permissions to the folder: 

chmod -R 777 <dirname>

