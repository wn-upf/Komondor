<p align="center"> 
<img src="https://github.com/wn-upf/Komondor/blob/master/Documentation/Other/Images and resources/komondor_logo.png">
</p>

# Intelligent Agents in Komondor

## Table of Contents
- [Authors](#authors)
- [Introduction](#introduction)
- [Overview](#overview)
- [Usage](#usage)

## Authors
* [Francesc Wilhelmi](https://github.com/fwilhelmi)
* [Sergio Barrachina-Muñoz](https://github.com/sergiobarra)

## Introduction

Komondor includes integrated Machine Learning (ML) operation, which is implemented based on the [ITU-T Architecture for Future Networks](https://www.itu.int/en/ITU-T/focusgroups/ml5g/Documents/ML5G-delievrables.pdf). In particular, several decentralized and centralized ML mechanisms have been provided to fulfill use cases such as Decentralized Learning-based Spatial Reuse (SR) or Smart Dynamic Channel Bonding (DCB).

For more details on the ML-aware architecture, please refer to Wilhelmi, Francesc, et al. "A Flexible Machine Learning-Aware Architecture for Future WLANs." [arXiv preprint arXiv:1910.03510](https://arxiv.org/pdf/1910.03510.pdf) (2019).
	
## Overview

The modules related to agents operation are as follows:

* ```central_controller.h```: entity that is connected to agents that indicate support for the centralized operation. Allows centralizing procedures such as data gathering or ML output generation. 
* ```agent.h```: entity that is connected to Basic Service Sets (BSSs) through the Access Point (AP). The agents collect data from BSSs and can provide new configurations to them, based on certain optimization mechanism.
* ```learning_modules```: here we find the implementation of ML methods that receive feedback about the networks performance in simulation time. 
* `Code/methods/agent/agent_methods.h` : auxiliary methods used by agents.

An overview of the agents operation is next shown:
<p align="center"> 
<img src="https://github.com/wn-upf/Komondor/blob/master/Documentation/Other/Images and resources/overview_agents.png">
</p>

## Usage

Detailed installation and execution instructions can be found in the [Komondor User's Guide](https://github.com/wn-upf/Komondor/blob/master/Documentation/User%20guide/LaTeX%20files/komondor_user_guide.pdf).

To run Komondor with intelligent agents, pass the `--agents` flag pointing to your agents CSV:

| Flag | Short | Description |
|------|-------|-------------|
| `--nodes <file>` | `-n` | Node input CSV |
| `--agents <file>` | `-a` | Agents input CSV |
| `--save-agent <0/1>` | `-A` | Save per-agent log files (default: 0) |
| `--time <float>` | `-t` | Simulation duration in seconds |
| `--seed <int>` | `-s` | Random seed |

Example:

```bash
cd Code/main
./komondor_main \
    --nodes  ../input/examples/mab_example/input_nodes.csv \
    --agents ../input/examples/mab_example/agents_egreedy.csv \
    --save-agent 1 \
    --time 10.0 --seed 1
```

### Agents Input Files

The agents' operation is ruled by the agents input file. The current parameters are as follows:
1) BSS code: a code that must match with BSS codes provided in node input files. "NULL" indicates that the agent acts as a central controller.
2) Centralized flag: flag that agents use to indicate their support to the centralized system.
3) Time between requests: an agent is supposed to request data to the AP every inputted period. For the central controller, the time between requests is meant for requests to agents.
4) Allowed actions as lists: for each type of modifiable parameter, the user must introduce a list of possible values separated by a comma (e.g., CCA = {-70, -75, -80, -82}). Currently, there is support for lists of channel, CCA, transmission power, and DCB policy.
5) Reward: type of reward to be used by the learning mechanism (e.g., REWARD_TYPE_PACKETS_SUCCESSFUL = 1, REWARD_TYPE_AVERAGE_THROUGHPUT = 2)
6) Learning mechanism: learning framework used by the agent/CC. Supported values:
   * `MONITORING_ONLY = 0` — collect data only, no action updates
   * `MULTI_ARMED_BANDITS = 1` — classic MAB with configurable strategy
   * `RTOT_ALGORITHM = 4` — rate-based TXOP optimization
   * `LEARNING_MECHANISM_EXTERNAL = 7` — offload decisions to an external Python server via Unix socket (see [AIML.md](AIML.md))
7) Selected strategy: extra parameter to differentiate between subtypes of learning approaches. For the MAB framework (`MULTI_ARMED_BANDITS = 1`):
   * `STRATEGY_EGREEDY = 1` — epsilon-greedy
   * `STRATEGY_EXP3 = 2` — Exp3 (adversarial)
   * `STRATEGY_UCB = 3` — Upper Confidence Bound
   * `STRATEGY_TS = 4` — Thompson Sampling

See an example of agents input file [here](Code/input/examples/mab_example/agents_egreedy.csv).

Regarding the output ("output" folder), some logs and statistics are created at the end of the execution.

------
### An example with agents: finding the best primary channel

Assume that an AP implementing dynamic channel bonding wants to find the best primary channel defined as the primary resulting in the highest throughput. Consider the deployment below, where all the BSS's implement single-channel transmissions but AP A, which is allowed to bond channels. Assume also a fully-overlapping setup, where every node listens any transmission falling within its primary channel, and where all APs are fully backlogged. In such a situation, what would be the best primary channel for AP A?

Well, since channels 4 and 5 are always free, thus providing the largest interference-free bondable combination (40 MHz) permitted by IEEE 802.11ac/ax, A should select either 4 or 5 as primary. How would A *learn* that? In this example, we propose a simple combination of the MAB mechanism together with the epsilon-greedy learning strategy. In the charts below, we observe that the agent successfully tends to pick primary 5 since it results in the highest reward (defined as normalized throughput). Notice, that 5 is selected rather than 4 due to pure randomness in the algorithm initialization.

<p align="center"> 
<img src="https://github.com/wn-upf/Komondor/blob/master/Documentation/Other/Images and resources/example_primary_channel_eps_greedy.png">
</p>

You may find the input files for the nodes and the agent of the example in [`Code/input/examples/mab_example/`](Code/input/examples/mab_example/).

For the full ML architecture reference — reward functions, action space encoding, adding new algorithms, and external model integration — see [AIML.md](AIML.md).

[GO BACK TO MAIN PAGE](https://github.com/wn-upf/Komondor)

