# KomondorSimulator

To run Komondor simulator, just build the project by using the build_local script and then execute it by doing:

$ ./Komondor.cc SIM_TIME SEED

Where SIM_TIME is the simulation time and SEED is the random seed you wish to use.

There are two input files located at the "input" folder at which you can configure system and nodes parameters, respectively:
	* input_system_conf.csv: define parameters such as the number of total available channels, the CW...
	* input_nodes_conf.csv: define parameters such as the node id, the node location, etc.
	
Regarding the output ("output" folder), some logs and statistics are created at the end of the execution.

Set permissions to the folder: 

chmod -R 777 <dirname>

The project is structured as follows:
* Code_RTS_CTS: contains the core files to run Komondor, as well as the input required and the output generated. The main files are "komondor_rts_cts", "node_rts_cts" and "build_local". In addition, there are scripts for running multiple inputs (e.g., "multiple_inputs_script.sh").
* Code_Basic: contains old files of Komondor that do not apply the RTS/CTS mechanism (now it is deprecated).
* COST: contains the SENSE simulator libraries, which is built in top of Komondor.
* methods: contains the necessary classes for performing several opeartions (backoff controlling, notifications management, etc.).
* structures: contains the definition of the main objects used in Komondor (e.g., wlan).
* list_of_macros.h: file that contains the macros used by Komondor.
* apps: contains auxiliary applications that contribute to the proper usage of Komondor (e.g., input file generator).
* documentation: folder containing documentation and additional information.