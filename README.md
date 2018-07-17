<p align="center"> 
<img src="https://github.com/wn-upf/Komondor/blob/master/Documentation/Images and resources/komondor_logo.png">
</p>

# Komondor: An IEEE 802.11ax Simulator

### Authors
* [Sergio Barrachina-Muñoz](https://github.com/sergiobarra)
* [Francesc Wilhelmi](https://github.com/fwilhelmi)

### Introduction and Project description

The Komondor Simulator has been built to simulate the behavior of overlapping WLANs according to their configuration. Komondor is mainly oriented to simulate the behavior of IEEE 802.11ax WLANs, which has not been considered in the current State-of-the-Art simulators. In addition, it has been prepared for a simple integration of intelligent agents that modify the nodes configuration for improving the network performance. 

Release v1.1 is the first one to include a basic structure to allow the inclusion of decentralized agents. So far, an epsilon-greedy strategy has been developed to tune the frequency channel, the sensitivity threshold and the transmit power, which are key features to enhance spectral efficiency.

The project is structured as follows:
* ```Code```: contains the core files to run Komondor, as well as the input required and the output generated. The main files are "komondor_rts_cts", "node_rts_cts" and "build_local". In addition, there are scripts for running multiple inputs (e.g., "multiple_inputs_script.sh").
* ```/COST```: contains the SENSE simulator libraries, which is built in top of Komondor.
* ```/methods```: contains the necessary classes for performing several opeartions (backoff controlling, notifications management, etc.).
* ```/structures```: contains the definition of the main objects used in Komondor (e.g., wlan).
* ```list_of_macros.h```: file that contains the macros used by Komondor.
* ```/apps```: contains auxiliary applications that contribute to the proper usage of Komondor (e.g., input file generator).
* ```/documentation```: folder containing documentation and additional information.

### Execution instructions

To run Komondor simulator, just build the project by using the build_local script and then execute it by doing:

STEP 0: Set permissions to the folder

```
$ chmod -R 777 <dirname>
```

STEP 1: Build the project

```
$ ./build_local
```

STEP 2: Run Komondor simulator for the given input information

```
$ ./komondor_main INPUT_FILE_SYSTEM_CONFIGURATION INPUT_FILE_NODES OUTPUT_FILE_LOGS FLAG_SAVE_SYSTEM_LOGS FLAG_SAVE_NODE_LOGS FLAG_PRINT_SYSTEM_LOGS FLAG_PRINT_NODE_LOGS SIM_TIME SEED
```

The inputs are further described next:
* ```INPUT_FILE_SYSTEM_CONFIGURATION```: file containing system information (e.g., number of channels available, traffic model, etc.). The file must be a .csv with semicolons as separators.
* ```INPUT_FILE_NODES```: file containing nodes information (e.g., position, channels allowed, etc.).The file must be a .csv with semicolons as separators.
* ```OUTPUT_FILE_LOGS```: path to the output file to which write results at the end of the execution (if the file does not exist, the system will create it).
* ```FLAG_SAVE_SYSTEM_LOGS```: flag to indicate whether to save the system logs into a file (1) or not (0).
* ```FLAG_SAVE_NODE_LOGS```: flag to indicate whether to save the nodes logs into separate files (1) or not (0). If this flag is activated, one file per node will be created.
* ```FLAG_PRINT_SYSTEM_LOGS```: flag to indicate whether to print the system logs (1) or not (0).
* ```FLAG_PRINT_NODE_LOGS```: flag to indicate whether to print the nodes logs (1) or not (0). 
* ```SIM_TIME```: simulation time
* ```SEED```: random seed the user wishes to use

IMPORTANT NOTE (!): Setting ```FLAG_SAVE_SYSTEM_LOGS``` and ```FLAG_SAVE_NODE_LOGS``` to TRUE (1) entails a larger execution time. 

STEP 2-1: Run Komondor simulator with intelligent agents

Alternatively, and in order to indicate the usage of agents, the console input must add the following extra information:

```
$ ./komondor_main INPUT_FILE_SYSTEM_CONFIGURATION INPUT_FILE_NODES INPUT_FILE_AGENTS OUTPUT_FILE_LOGS FLAG_SAVE_SYSTEM_LOGS FLAG_SAVE_NODE_LOGS FLAG_SAVE_AGENT_LOGS FLAG_PRINT_SYSTEM_LOGS FLAG_PRINT_NODE_LOGS FLAG_PRINT_AGENT_LOGS SIM_TIME SEED
```

The new inputs are described next:
* ```INPUT_FILE_AGENTS```: file containing agents information (e.g., wlan code, allowed actions, etc.).The file must be a .csv with semicolons as separators.
* ```FLAG_SAVE_AGENT_LOGS``` :flag to indicate whether to save the agent logs into separate files (1) or not (0). If this flag is activated, one file per agent will be created.
* ```FLAG_PRINT_AGENT_LOGS```: flag to indicate whether to print the agent logs (1) or not (0). 

# Input files

There are two input files located at the "input" folder at which you can configure system and nodes parameters, respectively:
* ```input_system_conf.csv```: define parameters such as the number of total available channels, the CW...
* ```input_nodes_conf.csv```: define parameters such as the node id, the node location, etc.
		
There are two ways of generating nodes:
1) Nodes file: both APs and STAs are individually defined
2) APs file: only APs are defined, and STAs are randomly generated according to a set of input parameters (e.g., min./max. number of associated STAs)
IMPORTANT NOTE (!): a nodes file must contain the keyword "nodes", whereas an APs file must contain the keyword "aps".

In case of including agents, a specific input file must be provided, which contains useful information regarding the agents' operation at the desired WLANs. The most important inputs refer to:
1) WLAN code: a code that must match with WLAN codes provided in node input files
2) Time between requests: an agent is supposed to request data to the AP every inputted period
3) Allowed actions as lists: for each type of modifiable parameter, the user must introduce a list of possible values (e.g. CCA = {-70, -75, -80, -82})

Regarding the output ("output" folder), some logs and statistics are created at the end of the execution.

### Contribute

If you want to contribute, please contact to sergio.barrachina@upf.edu and/or francisco.wilhelmi@upf.edu

### Acknowledgements

This work has been partially supported by a Gift from the Cisco University Research Program (CG\#890107, Towards Deterministic Channel Access in High-Density WLANs) Fund, a corporate advised fund of Silicon Valley Community Foundation.
