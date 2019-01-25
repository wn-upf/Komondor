# How to contribute to Komondor

### Bug reporting (guidelines)

1) Before reporting a bug, ensure the bug does not exist in Issues.
2) In case the bug has not been reported, open a new issue and be as clear and concise as possible.
3) In addition to information related to the reported issue, include information about the capabilities (e.g., OS, version, etc.) of the device from which the bug was reproduced.
4) Attach log and input files whenever it is possible.
5) Use the following template:
-----
**Title:** A crash occurs when enabling spatial reuse in large-scale scenarios. 

**System:** Ubuntu 16.04 (64-bits)

**Issue description:** When using the attached input, I noticed a crash (see attached logs) in the simulator. The fact is that the simulation runs properly for the same scenario when disabling spatial reuse.

**Steps to reproduce the issue:** 
1. Use the input_system and input_nodes file attached to this issue
2. Execute Komondor by typing: ./komondor_main /input/input_system.csv /input/input_nodes.csv /output/logs_output.txt 1 1 1 1 50 222
3. Run the simulation until second 43.12, moment at which the application crashes

**What's the expected result?** The simulator should not crash.

**What's the actual result?** The simulator crashes.

-----

### Contribute with new modules

If you want to contribute with new modules, please contact to sergio.barrachina@upf.edu and/or francisco.wilhelmi@upf.edu


Thanks for your contribution! :raised_hands:

Komondor Team
