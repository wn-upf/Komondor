<p align="center"> 
<img src="https://github.com/wn-upf/Komondor/blob/dev/Documentation/Other/Images%20and%20resources/kom8ndor_logo.png">
</p>

# Komondor: An IEEE 802.11bn-Oriented Simulator
| Agent-related content available [here](https://github.com/wn-upf/Komondor/blob/master/README_agents.md). |
| ----- |

## Table of Contents
- [Authors](#authors)
- [Introduction](#introduction)
- [Features](#features)
- [Overview](#overview)
- [Usage](#usage)
- [Validation](#validation)
- [Contribute](#contribute)
- [Acknowledgements](#acknowledgements)

## Authors
* [Francesc Wilhelmi](https://github.com/fwilhelmi)
* [Sergio Barrachina-Muñoz](https://github.com/sergiobarra)

## Introduction

Komondor is an event-driven wireless network simulator for IEEE 802.11bn (Wi-Fi 8) and beyond WLANs. It covers mechanisms ranging from dynamic channel bonding and spatial reuse to Multi-AP coordination (MAPC), and machine learning-driven channel access, making it suitable for both protocol research and AI/ML benchmarking.
		
Komondor has been conceived as an open source tool that contributes to the ongoing research in wireless networks, especially regarding the implementation of novel functionalities that are not available in other well-known wireless simulators. In addition, it has been prepared for a simple integration with Machine Learning (ML) modules.

The project is structured as follows:
* ```Apps```: contains auxiliary applications that support the Komondor's core operation. For example, in /Apps we find the "Input Generation" project, which contains a Java application that generates inputs for Komondor in an easy and flexible way.
* ```Code```: contains the core files to compile and run Komondor, including input and output folders. 
* ```Documentation```: contains Documentation related to Komondor, such as a Manual, a user's guide, presentations, etc.

## Features

| Feature | Standard | Notes |
|---------|----------|-------|
| Dynamic Channel Bonding (DCB) | 802.11ac/ax | Log2-aligned aggregation; CB models 0-7 including preamble puncturing (model 6) and DSO (model 7) |
| Enhanced Spatial Reuse (SR) | 802.11ax | BSS-color-based OBSS/PD threshold adaptation; SRG and non-SRG variants |
| Multi-AP Coordination (MAPC) | 802.11bn | Co-TDMA, Co-SR (power adaptation), Co-BF (Zero-Forcing beamforming); up to 8 groups per WLAN |
| Per-TXOP ULA Beamforming | 802.11ax/bn | Uniform Linear Array null-steering; ZF precoding for Co-BF, Gram-Schmidt projection for standalone BF |
| Full EDCA | 802.11e/ax | Per-AC AIFSN, contention-window bounds, binary exponential backoff, TXOP limits |
| Dynamic Subband Operation (DSO) | 802.11bn | Post-backoff redirect of DATA to a secondary subband via ICF/ICR exchange |
| Non-Primary Channel Access (NPCA) | 802.11bn | Opportunistic secondary-subband access while the primary channel is busy |
| Per-STA Bandwidth Adaptation | 802.11ax | AP caps TX channel range to each STA's declared capabilities |
| Machine Learning | — | Multi-Armed Bandits, and external Python model via Unix socket |

For a detailed feature changelog and configuration reference, see [release_notes.md](release_notes.md).
For ML architecture and external model integration, see [AIML.md](AIML.md).

## Overview

The code to run simulations is organized as follows:

* ```COST```: constitutes the Komondor's primitive operation. Here we find the CompC++ library that allows generating discrete event simulations. For further information about COST, please refer to its main [website](http://www.ita.cs.rpi.edu/cost.html). 
* ```main```: contains the core files (komondor.cc, node.h, traffic_generator.h, agent.h, and central_controller.h) that are in charge of orchestrating all the simulation. "komondor.cc" is the main component, which initializes all the other components of "Type II". All these modules are aware of the existence of the simulation time. In addition to the core components, here we find "build_local", a bash script that compiles the libraries for executing the code. Note that the file "compcxx_komondor_main.h" is also required to carry out such a compilation.
* ```methods```: independent method headers organized into subfolders by concern: `agent/`, `channel/` (bonding, power, beamforming), `mac/` (NACK, packet loss, spatial reuse), `mapc/` (MAPC frame exchanges), `node/` (FSM handlers, backoff, packet, statistics), `utils/` (output, logging, input parsing), and `frames/` (duration, aggregation). The entry aggregator is `methods/node/node_impl.h`.
* ```structures```: data-only headers shared across modules, including `wlan.h` (WLAN identity and MAPC group membership), `notification.h` (inter-node message format), `node_parameters.h`, `node_statistics.h`, `channel_access_state.h`, `spatial_reuse_state.h`, and `packet_exchange_sequence.h`.
* ```learning_modules```: here we find the implementation of ML methods that receive feedback about the network's performance in simulation time. 
* ```list_of_macros.h```: all the static parameters (e.g., constants) are contained in this file.
* ```input```: contains the input files that allow building the simulation environment.
* ```output```: contains the data generated by Komondor as a result of a given simulation.	
* ```scripts_multiple_executions```: contains bash scripts to perform multiple simulations.

An overview of the current modules available in Komondor is shown next:
<p align="center"> 
<img src="https://github.com/wn-upf/Komondor/blob/master/Documentation/Other/Images and resources/modules_overview.png">
</p>

## Usage

To run Komondor, just build `komondor_main` and then execute it by following the next steps:

### STEP 0: Go to ./Code/main and set permissions of the folder
```
$ cd ./Code/main
$ chmod -R 777 <dirname>
```

### STEP 1: Build Komondor

#### OPTION a: Build Komondor and CompC++ from scratch using make
Following instructions require [make](https://www.gnu.org/software/make/manual/make.html).

```
$ make
```

#### OPTION b: Build Komondor with pre-built CompC++
WARNING: Following instructions use a pre-built CompC++. The binary
`Code/COST/cxx` is platform-dependent and might not work on your machine.

```
$ ./build_local
```

### STEP 2: Run Komondor

#### 2.1 Simulation flags

Komondor uses both short flags (e.g., -n) and long flags (e.g., --nodes).

| Long Flag | Short Flag | Argument | Description | Default |
| :--- | :--- | :--- | :--- | :--- |
| `--nodes` | `-n` | `<file>` | **Required.** Path to the nodes input file. | N/A |
| `--time` | `-t` | `<float>` | Total simulation time in seconds. | `10.0` |
| `--seed` | `-s` | `<int>` | Random seed for stochastic processes. | `1` |
| `--code` | `-c` | `<string>`| Unique simulation identifier code. | `SIM_001` |
| `--out` | `-o` | `<file>` | Path for the output script/results. | `../output/default_output.txt` |

Logging & Debugging (use 1 to enable and 0 to disable):

`--logs-sys <0/1> (-L)`: Print system-level logs to the console (Default: 1).

`--logs-node <0/1> (-l)`: Print node-specific logs to the console (Default: 1).

`--save-node <0/1> (-S)`: Save node logs to a file. IMPORTANT NOTE (!): Setting --save-node to TRUE (1) entails larger execution times. 

Optional modes:

`--agents <file> (-a)`: Enables Agent-based features using the specified input file.

`--save-agent <0/1> (-A)`: Save agent-specific logs to a file.

`--mapc <file> (-m)`: Enables Multi-AP Coordination (MAPC) features using the specified input file.

#### 2.2 Input files

Komondor uses input files to define the simulation setup, including participating nodes, their capabilities, and traffic requirements, to name a few examples.

Input files are located in the `/Komondor/Code/input` folder.

Types of inputs include:
* Node configuration: define parameters such as the node ID, the node location, etc.
    * Example of nodes input file: [```input_nodes.csv```](Code/input/examples/basic_example/input_nodes.csv)
* Agent configuration: define parameters used by the agents' operation. Refer to [README_agents](https://github.com/wn-upf/Komondor/blob/master/README_agents.md).
    * Example of agents input file: [```agents_egreedy.csv```](Code/input/examples/mab_example/agents_egreedy.csv)
* MAPC configuration: define the parameters used for which Multi-Access Point Coordination (MAPC) is enabled.
    * Example of MAPC input file: [`mapc_cotdma_example.csv`](Code/input/examples/mapc_example/mapc_cotdma_example.csv)


#### 2.3 Output 

Regarding the output ("output" folder), some logs and statistics are created at the end of the execution.

#### 2.4 Simulation examples

##### Basic simulation

Run a basic simulation with Komondor for a given nodes configuration file (a .csv file like [this](Code/input/examples/basic_example/input_nodes.csv) containing all the necessary information about the nodes).

From path /Komondor/Code/main, you should run:

```bash
./komondor_main \
    --nodes ../input/examples/basic_example/input_nodes.csv \
    --time 10.0 --seed 1
```

##### Standard simulation

Run a 30-second simulation with a specific seed and custom output name:

```
./komondor_main -n scenario.txt -t 30.0 -s 777 -o ../output/my_results.txt
```

##### Simulation with agents

Run a simulation with Agents:

```bash
./komondor_main \
    --nodes  ../input/examples/basic_example/input_nodes.csv \
    --agents ../input/examples/mab_example/agents_egreedy.csv \
    --save-agent 1 \
    --time 10.0 --seed 1
```

For ML agent configuration details see [README_agents.md](README_agents.md) and [AIML.md](AIML.md).

##### MAPC simulation

```bash
./komondor_main \
    --nodes ../input/examples/mapc_example/input_nodes_mapc_example.csv \
    --mapc  ../input/examples/mapc_example/mapc_cotdma_example.csv \
    --time 10.0 --seed 1
```

### Other installations

Debugging: 

```
$ apt-get install gdb valgrind 
```

## Validation
Komondor v.2.0 has been validated by means of ns-3 and SF-CTMN, and Bianchi analytical models. The presentation of the validation can be found at "S. Barrachina-Muñoz, F. Wilhelmi, I. Selinis & B. Bellalta. Komondor: a Wireless Network Simulator for Next-Generation High-Density WLANs. 2018". Additional resources are available in this repository, in the folder /Documentation/Validation. Files used for each simulation tool can be found:
1) ns-3: execution script and instructions, together with the simulation environment used for validation.
2) Komondor: input "nodes" and "system" files. Release pointing to v.2.0 must be used.
3) SF-CTMN: simulation environment, to be executed through the SF-CTMN framework, available at https://github.com/sergiobarra/SFCTMN
4) Bianchi: Matlab files emulating the Bianchi model ("Bianchi, G., Fratta, L., & Oliveri, M. (1996, October). Performance evaluation and enhancement of the CSMA/CA MAC protocol for 802.11 wireless LANs. In Personal, Indoor and Mobile Radio Communications, 1996. PIMRC'96., Seventh IEEE International Symposium on (Vol. 2, pp. 392-396). IEEE.") can be found, which simulate the throughput achieved by each WLAN in each of the proposed scenarios.

### Regression test
An automated regression test is available to ensure that the development of new features does not affect the previous implementation, which was validated in Barrachina-Muñoz, S., Wilhelmi, F., Selinis, I., & Bellalta, B. (2019, April). Komondor: a Wireless Network Simulator for Next-Generation High-Density WLANs. In 2019 Wireless Days (WD) (pp. 1-8). IEEE.

To run the regression test, build the simulator and execute `validate.sh` from the `Code/` directory:

```bash
cd Code
bash validate.sh
```

The script builds the binary, runs four scenario groups (basic, complex, channel-bonding, and spatial-reuse), and compares throughput and RSSI outputs against known-good baselines. A pass/fail summary is printed at the end. No additional configuration changes are required.

## Academic/Education projects
One of the main purposes of Komondor is to serve as an academic/educational tool. In what follows, we list the projects in which Komondor has been used as a simulation tool:
* Wilhelmi, F., Bellalta, B., Casasnovas, M., Kijanka, A., & Calvo-Fullana, M. (2026). Decentralized Spatial Reuse Optimization in Wi-Fi: An Internal Regret Minimization Approach. (arXiv preprint arXiv:2602.08456)[https://arxiv.org/pdf/2602.08456].
* Wilhelmi, F., Bellalta, B., Szott, S., Kosek-Szott, K., & Barrachina-Muñoz, S. (2025, May). Coordinated Multi-Armed Bandits for Improved Spatial Reuse in Wi-Fi. In 2025 IEEE International Conference on Machine Learning for Communication and Networking (ICMLCN) (pp. 1-6). IEEE. [(Download the paper here)](https://arxiv.org/abs/2412.03076)
* Wilhelmi, F., Galati-Giordano, L., & F., G. (2025, March). “It's Your Turn”: A Novel Channel Contention Mechanism for Improving Wi-Fi's Reliability. In 2025 IEEE Wireless Communications and Networking Conference (WCNC) (pp. 1-6). IEEE. [(Download the paper here)](https://arxiv.org/abs/2410.07874)
* Wilhelmi, F., Szott, S., Kosek-Szott, K., & Bellalta, B. (2024). Machine Learning and Wi-Fi: Unveiling the Path Toward AI/ML-Native IEEE 802.11 Networks. IEEE Communications Magazine. [(Download the paper here)]([https://arxiv.org/abs/2412.03076](https://arxiv.org/abs/2405.11504)
* Wilhelmi, F. & Bellalta, B. (2020). ITU AI for 5G Challenge. [UPF/ITU-T AI Challenge - Improving the capacity of IEEE 802.11 WLANs through Machine Learning](https://www.upf.edu/web/wnrg/ai_challenge).
* Wilhelmi, F., Carrascosa, M., Cano, C., Jonsson, A., Ram, V., & Bellalta, B. (2021). Usage of Network Simulators in Machine-Learning-Assisted 5G/6G Networks. [IEEE Wireless Communications Magazine](https://ieeexplore.ieee.org/abstract/document/9363049).
* Wilhelmi, F., Barrachina-Muñoz, S., Bellalta, B., Cano, C., Jonsson, A., & Ram, V. (2020). A flexible machine-learning-aware architecture for future WLANs. [IEEE Communications Magazine, 58(3), 25-31](https://ieeexplore.ieee.org/abstract/document/9040258/).
* Wilhelmi, Francesc, Sergio Barrachina-Muñoz, and Boris Bellalta. "Spatial Reuse in IEEE 802.11ax WLANs." [Computer Communications](https://www.sciencedirect.com/science/article/abs/pii/S0140366421000499) (2020). 
* Barrachina-Muñoz, Sergio, et al. "Online Primary Channel Selection for Dynamic Channel Bonding in High-Density WLANs." [IEEE Wireless Communications Letters (2019)](https://ieeexplore.ieee.org/abstract/document/8894073/). 
* Barrachina-Muñoz, Sergio, et al. "Komondor: a wireless network simulator for next-generation high-density WLANs." [Wireless Days (WD), 1-8 (2019)](https://ieeexplore.ieee.org/abstract/document/8734225). 
* Barrachina-Muñoz, Sergio, et al. "Dynamic Channel Bonding in Spatially Distributed High-Density WLANs." [IEEE Transactions on Mobile Computing (2019)](https://ieeexplore.ieee.org/abstract/document/8642923). 
* SECTOR, STANDARDIZATION. FOCUS GROUP ON MACHINE LEARNING FOR FUTURE NETWORKS INCLUDING 5G ML5G-I-125-R3. [StandICT](https://www.standict.eu/sites/default/files/ML5G-I-125-R3.pdf).
* Wilhelmi, Francesc, et al. "Potential and pitfalls of multi-armed bandits for decentralized spatial reuse in WLANs." [Journal of Network and Computer Applications 127 (2019)](https://www.sciencedirect.com/science/article/pii/S1084804518303655): 26-42. 
* Barrachina-Muñoz, Sergio, et al. "To overlap or not to overlap: Enabling channel bonding in high-density WLANs." [Computer Networks 152 (2019)](https://www.sciencedirect.com/science/article/pii/S1389128618309745). 
* Wilhelmi, F., Barrachina-Muñoz, S., & Bellalta, B. (2019, October). On the Performance of the Spatial Reuse Operation in IEEE 802.11 ax WLANs. In 2019 IEEE Conference on Standards for Communications and Networking (CSCN) (pp. 1-6). IEEE. [IEEE Conference on Standards for Communications and Networking (CSCN)](https://ieeexplore.ieee.org/abstract/document/8931315/).

## Contribute

If you want to contribute, please contact [francisco.wilhelmi@upf.edu](francisco.wilhelmi@upf.edu)

More details in [CONTRIBUTING.md](https://github.com/wn-upf/Komondor/blob/master/CONTRIBUTING.md)

## Acknowledgements

The release of Kom8ndor has been supported by TRUE-Wi-Fi PID2024-155470NB-I00 (MCIU/AEI/FEDER,UE), by the CHIST-ERA Wireless AI 2022 call MLDR project (ANR-23-CHR4-0005), partially funded by AEI under project PCI2023-145958-2, by Wi-XR PID2021-123995NB-I00, and by MCIN/AEI under the Maria de Maeztu Units of Excellence Programme (CEX2021-001195-M).

Komondor v2.0 (2019-2020) was partially supported by a Gift from the Cisco University Research Program (CG\#890107, Towards Deterministic Channel Access in High-Density WLANs) Fund, a corporate advised fund of Silicon Valley Community Foundation.
