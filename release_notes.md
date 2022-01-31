# Komondor: Release notes

To upgrade Komondor, just download the source code from [https://github.com/wn-upf/Komondor](https://github.com/wn-upf/Komondor)

## Breaking Changes
* We have changed the way input information is passed to the simulator:
	* The input_system file has been deleted.
	* Additional information has been added to input_nodes file.
	* A new file (/Komondor/Code/config_models) has been added, which defines which general models are used to define the global environment (e.g., path-loss, adjacent channel interference, etc.). These models are typically not modified, but Komondor provides certain flexibility for testing different environmental cases.

* We have provided a first stable implementation of agents, which is based on the architectural elements defined in Wilhelmi, Francesc, et al. "A Flexible Machine Learning-Aware Architecture for Future WLANs." [arXiv preprint arXiv:1910.03510](https://arxiv.org/pdf/1910.03510.pdf) (2019).

## New Features
- We implemented a first version of the RTOT algorithm for SR improvement in 11ax WLANs. Refer to Ropitault, Tanguy. "Evaluation of RTOT algorithm: A first implementation of OBSS_PD-based SR method for IEEE 802.11 ax." [2018 15th IEEE Annual Consumer Communications & Networking Conference (CCNC). IEEE](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=8319274), 2018.

## Improvements
- We extended the regression script with DCB use cases.
- We improved the way methods are separated.

## Bug Fixes
- We solved a crash related to agents' output log files.
- We have fixed an error whereby transmit power was not being updated when bonding >1 channels for transmitting.

# Other
- Dataset for the AI Challenge (ITU-T) coming soon...
- A list of academic and educational projects involving Komondor has been added to the README.
