# Code Directory Structure Analysis

This document provides a detailed analysis of each subdirectory within the `Code` folder of the Komondor simulator.

## Overview

The `Code` directory contains the core implementation of Komondor, an IEEE 802.11ax wireless network simulator. The codebase is organized into several subdirectories, each serving a specific purpose in the simulation framework.

---

## 1. `compcpp/` - CompC++ Compiler Infrastructure

**Purpose:** Contains the CompC++ compiler infrastructure used to build the simulation engine.

**Contents:**
- `lexer.l` - Lexical analyzer for CompC++
- `parser.cpp` - Parser implementation
- `token.h` / `token.cpp` - Token definitions and handling
- `Makefile` - Build configuration
- `README.md` - Documentation about COST/CompC++

**Functionality:**
- Provides the CompC++ language compiler (`cxx` binary) that enables component-oriented discrete event simulation
- Based on the COST (Component Oriented Simulation Toolkit) framework
- Extracted from the SENSE simulation framework
- Required for compiling Komondor's component-based simulation code

**Key Files:**
- Generates the `cxx` compiler binary used to process `.h` files with component definitions

---

## 2. `COST/` - COST Simulation Framework Core

**Purpose:** Contains the core COST (Component Oriented Simulation Toolkit) library headers that provide the simulation engine primitives.

**Contents:**
- `cost.h` - Main COST framework header with base classes (`CostSimEng`, `TypeII`, etc.)
- `priority_q.h` - Priority queue implementation for event scheduling
- `corsa_alloc.h` - Memory allocation utilities
- `sense.h` - SENSE-related definitions
- `path.h` - Path utilities
- `ether_addr.h` - Ethernet address handling
- `config.h` - Configuration definitions

**Functionality:**
- Provides the discrete event simulation engine foundation
- Defines base component classes (`TypeII` components are time-aware)
- Implements event scheduling and simulation time management
- Provides utility classes for network simulation (queues, timers, etc.)

**Key Concepts:**
- `CostSimEng`: Base simulation engine class
- `TypeII`: Components aware of simulation time
- Event-driven simulation model

---

## 3. `main/` - Core Simulation Components

**Purpose:** Contains the main simulation orchestration components and the entry point of the simulator.

**Contents:**
- `komondor_main.cc` - Main entry point that initializes and runs the simulation
- `komondor_main` - Compiled binary executable
- `node.h` - Node component (APs and STAs) implementing IEEE 802.11ax behavior
- `agent.h` - Intelligent agent component for ML-based optimization
- `traffic_generator.h` - Traffic generation component
- `central_controller.h` - Centralized controller component
- `compcxx_komondor_main.h` - CompC++ compilation helper
- `build_local` - Build script for local compilation
- `Makefile` - Build configuration

**Functionality:**
- **Komondor Component**: Main simulation orchestrator that:
  - Parses input files (nodes configuration, agent configuration)
  - Initializes the wireless network topology
  - Creates and initializes all simulation components (nodes, agents, controllers)
  - Manages simulation execution until simulation time expires
  - Processes and outputs simulation results

- **Node Component**: Implements IEEE 802.11ax node behavior:
  - Access Point (AP) and Station (STA) functionality
  - Channel access (DCF, backoff mechanisms)
  - Packet transmission and reception
  - Channel bonding and spatial reuse operations
  - Modulation and coding scheme (MCS) selection
  - Power control

- **Agent Component**: Implements intelligent agents for:
  - Machine learning-based network optimization
  - Multi-armed bandit algorithms
  - Decentralized decision-making
  - Action selection and learning

- **TrafficGenerator Component**: Generates traffic patterns for nodes

- **CentralController Component**: Provides centralized control and coordination

**Key Files:**
- `komondor_main.cc`: ~1500 lines - Main simulation loop and initialization
- `node.h`: ~5600 lines - Core node behavior implementation

---

## 4. `structures/` - Data Structures and Type Definitions

**Purpose:** Contains header files defining data structures, types, and classes used throughout the simulator.

**Contents:**
- `wlan.h` - WLAN structure (WLAN ID, AP ID, list of STAs, spatial reuse settings)
- `node_configuration.h` - Node configuration parameters
- `notification.h` - Notification structure for inter-component communication
- `performance.h` - Performance metrics and statistics
- `logger.h` - Logging utilities
- `FIFO.h` - First-In-First-Out queue implementation
- `logical_nack.h` - Logical negative acknowledgment structure
- `modulations.h` - Modulation and coding scheme definitions
- `action.h` - Action structure for agents
- `controller_report.h` - Central controller reporting structure

**Functionality:**
- Defines core data structures representing network entities (WLANs, nodes)
- Provides configuration structures for nodes and agents
- Implements communication structures (notifications, reports)
- Defines performance tracking structures
- Provides utility structures (queues, loggers)

**Key Structures:**
- `Wlan`: Represents a WLAN with AP and associated STAs
- `NodeConfiguration`: Configuration parameters for nodes
- `Notification`: Message passing between components
- `Performance`: Performance metrics collection

---

## 5. `methods/` - Utility Methods and Algorithms

**Purpose:** Contains utility methods organized by functionality, following clean architecture principles.

**Contents:**
- `auxiliary_methods.h` - General utility functions
- `backoff_methods.h` - Backoff algorithm implementations for DCF
- `modulations_methods.h` - Modulation and coding scheme methods
- `notification_methods.h` - Notification handling methods
- `output_generation_methods.h` - Output file generation and logging
- `power_channel_methods.h` - Power control and channel management methods
- `spatial_reuse_methods.h` - Spatial reuse operation methods
- `time_methods.h` - Time-related utility methods
- `agent_methods.h` - Agent-related utility methods

**Functionality:**
- Provides reusable methods for common operations
- Implements IEEE 802.11ax specific algorithms (backoff, spatial reuse)
- Handles channel and power management
- Provides output generation and logging utilities
- Implements agent decision-making helpers

**Design Principle:**
- Separation of concerns: Independent methods organized by functionality
- Used by core components (`main/`) but kept separate for maintainability

---

## 6. `learning_modules/` - Machine Learning Components

**Purpose:** Contains machine learning models and optimization algorithms for intelligent network management.

**Contents:**
- `ml_model.h` - Main ML model class that coordinates ML operations
- `pre_processor.h` - Data preprocessing for ML models
- `network_optimization_methods/` - Subdirectory with specific ML algorithms:
  - `multi_armed_bandits.h` - Multi-armed bandit (MAB) algorithm implementation
  - `centralized_action_banning.h` - Centralized action banning algorithm
  - `rtot_algorithm.h` - RTOT (Receive Target Objective Threshold) algorithm

**Functionality:**
- **MlModel Class**: Coordinates ML operations:
  - Receives network performance feedback
  - Applies ML algorithms to optimize network parameters
  - Selects actions based on learning mechanisms
  - Supports multiple learning strategies (MAB, RTOT, centralized)

- **Multi-Armed Bandits**: Implements MAB algorithms for:
  - Decentralized channel selection
  - Power control optimization
  - Spatial reuse parameter tuning

- **RTOT Algorithm**: Implements receive target objective threshold algorithm

- **Centralized Action Banning**: Implements centralized coordination for action selection

- **Pre-processor**: Prepares network state data for ML models

**Integration:**
- Used by `Agent` components in `main/agent.h`
- Receives performance feedback from nodes
- Makes optimization decisions that affect node behavior

---

## 7. `input/` - Input Files and Validation

**Purpose:** Contains input files for simulations and validation scripts.

**Contents:**
- `input_example/` - Example input files
- `input_mab_example/` - Example input files for MAB scenarios
- `validation/` - Validation test scenarios
- `script_regression_validation_scenarios.sh` - Regression test script

**Functionality:**
- Stores CSV input files defining:
  - Node configurations (position, channels, power levels)
  - Agent configurations (learning parameters, action spaces)
  - Network topologies

- Provides validation scenarios for:
  - Regression testing
  - Comparison with ns-3 and analytical models
  - Ensuring code changes don't break existing functionality

**Input File Format:**
- CSV files with semicolon separators
- Node configuration: ID, position, channel assignments, etc.
- Agent configuration: learning mechanism, action selection strategy, etc.

---

## 8. `scripts_multiple_executions/` - Batch Execution Scripts

**Purpose:** Contains bash scripts for running multiple simulations in batch.

**Contents:**
- `multiple_inputs_script.sh` - Script for running multiple simulations
- `multiple_inputs_script_several_seeds.sh` - Script for multiple seeds
- `multiple_inputs_script_agents.sh` - Script for agent-based simulations

**Functionality:**
- Automates execution of simulation campaigns
- Runs simulations with different:
  - Input files
  - Random seeds
  - Parameter configurations
- Facilitates large-scale simulation studies

---

## 9. `list_of_macros.h` - Constants and Macros

**Purpose:** Centralized definition of all constants, macros, and static parameters used throughout the codebase.

**Contents:**
- ~739 lines of macro definitions
- Constants for:
  - IEEE 802.11ax parameters
  - Simulation parameters
  - Physical layer parameters
  - MAC layer parameters
  - Default values
  - Threshold values

**Functionality:**
- Provides a single source of truth for all constants
- Makes it easy to modify simulation parameters
- Includes parameters for:
  - Channel bandwidths
  - Modulation schemes
  - Time slots and inter-frame spacing
  - Power levels
  - Default configurations

**Usage:**
- Included by all major components
- Centralized configuration management

---

## Architecture Overview

### Component Hierarchy

```
Komondor (main component)
├── Node (TypeII component) - APs and STAs
├── Agent (TypeII component) - ML agents
├── TrafficGenerator (TypeII component) - Traffic generation
└── CentralController (TypeII component) - Centralized control
```

### Data Flow

1. **Input**: CSV files → Parsed by `komondor_main.cc`
2. **Initialization**: Components created and configured
3. **Simulation**: Event-driven execution with COST framework
4. **Learning**: Agents receive feedback and make decisions
5. **Output**: Performance metrics written to output files

### Key Design Patterns

- **Component-Oriented**: Uses COST framework for discrete event simulation
- **Separation of Concerns**: Methods separated from core components
- **Clean Architecture**: Independent utility methods in `methods/`
- **ML Integration**: Learning modules integrated via agent components

---

## Summary

The `Code` directory is well-organized with clear separation of concerns:

- **`main/`**: Core simulation components (orchestration, nodes, agents)
- **`structures/`**: Data structures and types
- **`methods/`**: Utility methods organized by functionality
- **`learning_modules/`**: ML algorithms and optimization methods
- **`COST/`**: Simulation engine framework
- **`compcpp/`**: Compiler infrastructure
- **`input/`**: Input files and validation
- **`scripts_multiple_executions/`**: Batch execution scripts
- **`list_of_macros.h`**: Centralized constants

This structure supports:
- IEEE 802.11ax network simulation
- Machine learning-based optimization
- Scalable simulation campaigns
- Maintainable and extensible codebase

