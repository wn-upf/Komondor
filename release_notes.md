# Komondor Release Notes

**Simulator:** Komondor — IEEE 802.11bn (Wi-Fi 8) Event-Driven Simulator  
**Build constraint:** `g++ -std=c++98` (Linux, via `Code/main/Makefile`)  
**Validation suite:** `cd Code && bash validate.sh`

(the release notes were generated with the aid of generative AI tools, but they were carefully reviewed manually)

---

## Table of Contents

1. [Multi-AP Coordination (MAPC): Co-TDMA, Co-SR, Co-BF](#1-multi-ap-coordination-mapc)
2. [Per-TXOP ULA Beamforming](#2-per-txop-ula-beamforming)
3. [Preamble Puncturing (802.11ax)](#3-preamble-puncturing-80211ax)
4. [Per-STA Bandwidth Adaptation](#4-per-sta-bandwidth-adaptation)
5. [EDCA](#5-full-edca-ieee-80211e--80211ax)
6. [Dynamic Subband Operation (DSO)](#6-dynamic-subband-operation--dso-ieee-80211bn-3726)
7. [Non-Primary Channel Access (NPCA)](#7-non-primary-channel-access--npca-ieee-80211bn-3718)
8. [Adaptive ACK Suppression](#8-adaptive-ack-suppression)
9. [External ML Model Integration](#9-external-ml-model-integration)
10. [Bug Fixes](#10-bug-fixes)
11. [Internal Refactoring Summary](#11-internal-refactoring-summary)
12. [Previous Release Notes](#12-previous-release-notes)

---

## 1. Multi-AP Coordination (MAPC): Co-TDMA, Co-SR, Co-BF

### Overview

Implements coordinated multi-AP operation for IEEE 802.11bn. Three coordination modes are supported:

- **Co-TDMA**: the coordinator AP transmits first on its own channel, then signals a coordinated AP to transmit on its channel within the same TXOP. This achieves spatial reuse without interference between the two APs.
- **Co-SR (Option A)**: coordinator and coordinated APs transmit simultaneously at pre-configured power levels. The power levels are set conservatively so that cross-BSS interference stays below the decoding threshold.
- **Co-BF**: coordinator and coordinated APs transmit simultaneously using ULA beamforming (see [Section 2](#2-per-txop-ula-beamforming)). The coordinator steers a null toward the coordinated AP's STA; the coordinated AP steers a null toward the coordinator's STA. This suppresses cross-BSS interference geometrically, allowing full concurrent transmission without power back-off.

The packet exchange sequence for Co-TDMA is:

```
ICF → ICR(s) → DATA/ACK (coordinator) → MU-RTS/TXS → DATA/ACK (coordinated AP)
```

For Co-SR and Co-BF:

```
ICF → ICR(s) → TF → simultaneous DATA/ACK (coordinator + coordinated AP)
```

After simultaneous DATA both STAs ACK on separate sub-channels (OFDMA-like rank assignment) so their ACKs do not collide.

### Configuration

MAPC requires two CSV files: the standard node CSV and a separate MAPC configuration CSV passed via `--mapc`.

#### Node CSV

No new columns are required. Nodes participate in MAPC solely based on their WLAN code and the MAPC config file. Use the standard format.

#### MAPC Config CSV (`--mapc <file>`)

Each row defines one coordination group. Fields are semicolon-delimited:

```
GroupID;Method;CoordinatedBSSIds;ExtraParams
```

| Field | Description |
|-------|-------------|
| `GroupID` | Integer group identifier (1-indexed) |
| `Method` | `CO_TDMA`, `CO_SR`, or `CO_BF` |
| `CoordinatedBSSIds` | Comma-separated WLAN codes of the **coordinated** APs (not the coordinator). Codes match the `wlan_code` field in the node CSV (e.g., `A,B` or `1,2`). |
| `ExtraParams` | For Co-SR: TX power in dBm (e.g., `20.0`). Empty for Co-TDMA and Co-BF. |

**Example — Co-TDMA with WLANs A and B (A is coordinator):**

```
1;CO_TDMA;B;
```

**Example — Co-SR with TX power 20 dBm:**

```
1;CO_SR;B;20.0
```

**Example — Co-BF (beamforming must also be enabled in the node CSV for both APs):**

```
1;CO_BF;B;
```

**Multi-group example (AP B appears in both groups):**

```
1;CO_TDMA;B;
2;CO_TDMA;C;
```

Up to `MAX_MAPC_GROUPS_PER_WLAN = 8` groups per WLAN are supported.

### Running a Co-TDMA Scenario

```bash
cd Code/main
./komondor_main \
  --nodes ../input/examples/mapc_example/input_nodes_mapc_example.csv \
  --mapc  ../input/examples/mapc_example/mapc_cotdma_example.csv \
  --time 10 --seed 1 --logs-sys 1 --save-node 1
```

### TXOP Splitting

The coordinator divides the TXOP budget equally among all active coordinated APs. The number of aggregated packets for each AP's DATA burst is capped so that DATA duration does not exceed the per-AP share. The cap loop runs in `PrepareNewTransmission` just before transmission.

### Traffic Model for Testing (model 99)

For MAPC testing, traffic model 99 (full buffer) pre-fills the coordinator's buffer at ICF time and the coordinated AP's buffer at DATA time. This avoids artificial queueing delays in exchange timing.

### Key Implementation Notes

- ICF and TF are **broadcast** frames (`destination_id = NODE_ID_MAPC_BROADCAST = -2`). Receivers filter by `mapc_group_id`.
- Third-party APs that overhear an ICF receive a valid NAV covering the full MAPC exchange and back off for the duration.
- WLAN codes in the MAPC config are matched as strings; both alphabetic (`A`, `B`) and numeric (`1`, `2`) codes work.

---

## 2. Per-TXOP ULA Beamforming

### Overview

Adds per-TXOP transmit beamforming using a horizontal Uniform Linear Array (ULA). On each new TXOP the transmitter computes a beam weight vector that steers the main lobe toward the intended receiver while placing nulls toward interfering nodes. In a MAPC scenario the coordinator steers nulls toward all peer APs' associated STAs; the coordinated AP steers a null toward the coordinator's STA.

Beamforming is applied only to **DATA frames**. All control frames (ICF, TF, RTS, CTS, ACK) remain omnidirectional to avoid coordination deadlock.

Two beam-weight algorithms are used depending on the mode:

- **Standalone BF** (non-MAPC): Gram-Schmidt projection null-steerer (`ComputeBeamWeights`). Steers the main lobe toward the intended receiver and places nulls toward specified interferers using successive projection.
- **Co-BF** (MAPC): Zero-Forcing (ZF) precoding (`ComputeZFBeamWeights`). Computes `w = H(HᴴH)⁻¹ e₁` via Gaussian elimination with partial pivoting, guaranteeing exact simultaneous nulls toward all peer STAs so both APs can transmit concurrently without cross-BSS interference. Falls back to projection if the number of null constraints exceeds the antenna count or the maximum supported size of 5.

### CSV Configuration

Four optional columns (32–35) are appended to the node CSV. Existing CSVs without these columns default to beamforming disabled.

| Column index | Field | Default | Description |
|---|---|---|---|
| 32 | `bf_enabled` | `0` | `1` to enable beamforming for this node |
| 33 | `bf_N` | — | Number of antenna elements (1–32) |
| 34 | `bf_d` | — | Element spacing in wavelengths (typically 0.5) |
| 35 | `bf_az_deg` | — | Main beam azimuth in degrees (0° = broadside) |

**Example node CSV row (AP with 8-element array, broadside beam):**

```
...<existing fields>...;1;8;0.5;90
```

If `bf_enabled = 0` or `bf_N <= 1`, the gain function returns 1.0 and throughput is identical to the no-BF case.

### Null Steering in MAPC

When beamforming is enabled on a MAPC coordinator, nulls are automatically steered toward the STAs associated with each peer AP in the active coordination group — no extra configuration is required. Null directions are computed from the node position arrays populated at simulation start.

---

## 3. Preamble Puncturing (802.11ax)

### Overview

Preamble puncturing (PP) allows an AP to claim its widest log2-aligned channel block while silencing individual busy 20 MHz secondary sub-channels in the PPDU preamble, rather than falling back to a narrower total bandwidth. The primary channel must be sensed idle. Busy secondary sub-channels are punctured (silenced) up to a maximum of `PP_MAX_PUNCTURED = 2`; if more secondaries are busy than this limit allows, the block is halved and the check repeats.

This is an IEEE 802.11ax feature that removes the primary-channel constraint entirely and uses a different signalling mechanism.

### Configuration

Preamble puncturing is exposed as **channel bonding model 6** (`CB_PP_MAX_LOG2`). No new CSV columns are required; set `channel_bonding_model = 6` in the node CSV for the AP that should use puncturing.

**Example node CSV entry (AP, 80 MHz block, primary channel 0):**

```
...<existing fields>...;6;...
```

where the field position corresponds to `channel_bonding_model`.

### Running a Preamble Puncturing Scenario

```bash
cd Code/main
./komondor_main \
  --nodes ../input/examples/preamble_puncturing_example/input_nodes_pp.csv \
  --time 10 --seed 1 --logs-sys 1 --save-node 1
```

The included scenario has BSS A using model 6 (80 MHz block, primary ch 0) and BSS B using model 4 on channels 2–3, creating secondary-channel interference for A to puncture. Look for `pp_punctured_bitmap` entries in the node logs to confirm which sub-channels were silenced.

---

## 4. Per-STA Bandwidth Adaptation

### Overview

The AP now respects each STA's declared channel range when selecting TX channels. When transmitting to a specific STA, the AP caps its channel selection to the **intersection** of its own range and that STA's declared `min_channel_allowed` / `max_channel_allowed`. All DCB models (CB_ALWAYS_MAX_LOG2, CB_11AX, PP, DSO, etc.) continue to apply, but over the narrower effective range.

This feature is also a prerequisite for DSO and NPCA, both of which require the AP to know each STA's operating bandwidth.

### Configuration

No new CSV columns are required. STAs already declare `min_channel_allowed` and `max_channel_allowed` in the standard node CSV; the AP now reads and uses those values at TX time.

**Example — AP at 160 MHz, STAs at different widths:**

| Node | Primary | Range | Effective TX width from AP |
|------|---------|-------|---------------------------|
| AP_A | 0 | 0–7 (160 MHz) | — |
| STA_A1 | 0 | 0–1 (40 MHz) | 40 MHz |
| STA_A2 | 0 | 0–3 (80 MHz) | 80 MHz |
| STA_A3 | 0 | 0–7 (160 MHz) | 160 MHz |

Existing scenarios where the AP and all STAs have identical ranges produce bit-identical output.

### Verifying Adaptation in Logs

Enable node logs (`--logs-node 1 --save-node 1`) and grep for the effective-range log line:

```bash
grep "Effective TX range" output/logs_nodes/N0*.txt
```

The output shows the per-destination channel range selected before each TXOP.

### Running a Bandwidth Adaptation Scenario

```bash
cd Code/main
./komondor_main \
  --nodes ../input/examples/bandwidth_adaptation_example/input_nodes_bw_adapt.csv \
  --time 10 --seed 1 --logs-node 1 --save-node 1
```

---

## 5. EDCA

### Overview

Completes the EDCA implementation. Per-AC differentiation is now fully active:

- **AIFSN**: VO=2, VI=3, BE=5, BK=7 (IFS = SIFS + AIFSN × slot time)
- **CW bounds**: VO=[8,16], VI=[16,32], BE/BK=[32,1024]
- **Binary Exponential Backoff (BEB)**: CW doubles on collision up to CW_max[AC]; resets to CW_min[AC] on success
- **TXOP limits**: VO=1.504 ms, VI=3.008 ms, BE/BK=0 (single PPDU per TXOP transmission)

### Configuration

Set the access category for each node using optional CSV column 36 (`traffic_type`). If absent, nodes default to AC_BE.

| Value | Access Category |
|-------|----------------|
| `0` | AC_BE (Best Effort) |
| `1` | AC_BK (Background) |
| `2` | AC_VI (Video) |
| `3` | AC_VO (Voice) |

**Example node CSV entry for an AC_VO node:**

```
...<existing fields>...;3
```

### Running an EDCA Differentiation Scenario

```bash
cd Code/main
./komondor_main \
  --nodes ../input/examples/edca_example/input_nodes_edca_2_ac_differentiation.csv \
  --time 10 --seed 1
```

This scenario places four overlapping BSSs in the same area, each using a different AC. Expected ordering in a saturated network: backoff slot count VO < VI < BK < BE; access delay VO < VI < BE ≈ BK.

### Included Test Scenarios

| File | Description |
|------|-------------|
| `input_nodes_edca_1_single_bss.csv` | Single BSS, AC_BE baseline |
| `input_nodes_edca_2_ac_differentiation.csv` | 4 overlapping BSSs, one per AC |
| `input_nodes_edca_3_edca_vs_dcf.csv` | EDCA VO vs. DCF head-to-head |

---

## 6. Dynamic Subband Operation (DSO)

**Validation status: confirmed** (build clean; functional test passed 2026-06-23).

### Overview

DSO is a post-backoff mechanism: after winning a TXOP on the primary channel, an AP redirects its DATA frame to a secondary subband within the target STA's declared bandwidth. The primary channel stays idle during the DATA exchange, freeing it for competing BSSs.

The exchange sequence is:

```
(Win TXOP on primary) → DSO_ICF → DSO_ICR → DATA → ACK
```

The ICR is modelled as a timer (no ICR frame is physically parsed). Total overhead before DATA: approximately 136 µs.

### Configuration

DSO is opt-in per node via optional CSV column 37 (`dso_enabled`). The AP also needs `channel_bonding_model = 7` (`CB_DSO_MAX_LOG2`).

| Column | Field | Value |
|--------|-------|-------|
| `channel_bonding_model` | CB model | `7` (enables DSO channel selection) |
| 37 | `dso_enabled` | `1` to enable |

STAs do not need `dso_enabled = 1`. Each STA must declare its secondary subband via `min_channel_allowed` / `max_channel_allowed` (must **not** overlap the AP's primary channel). Set the STA's `primary_channel` to the left endpoint of its secondary subband.

**Example — AP on 160 MHz (primary ch 0), three STAs each on a 40 MHz secondary:**

| Node | `primary_channel` | `min_channel_allowed` | `max_channel_allowed` | `dso_enabled` |
|------|-------------------|-----------------------|-----------------------|---------------|
| AP_A | 0 | 0 | 7 | 1 |
| STA_A1 | 2 | 2 | 3 | — |
| STA_A2 | 4 | 4 | 5 | — |
| STA_A3 | 6 | 6 | 7 | — |

### Running the DSO Scenario

```bash
cd Code/main
./komondor_main \
  --nodes ../input/examples/dso_npca_example/input_nodes_dso.csv \
  --time 10.0 --seed 1 --logs-sys 1 --save-node 1
```

Expected result: AP_A round-robins across the three STAs, sending each DATA burst on the respective secondary subband (channels 2–3, 4–5, 6–7) while the primary channel (ch 0) remains idle during DATA.

### Key Timing Constants

| Constant | Default | Description |
|----------|---------|-------------|
| `DSO_ICF_DURATION_US` | 52 µs | Duration of the DSO ICF frame |
| `DSO_SWITCH_TIME_US` | 8 µs | Radio retuning time |
| `DSO_ICR_DURATION_US` | 28 µs | Modelled ICR response window |

---

## 7. Non-Primary Channel Access (NPCA)

**Validation status: confirmed** (build clean; functional test passed 2025-05-02).

### Overview

NPCA triggers when an AP in STATE_SENSING (its backoff frozen) decodes an inter-BSS PPDU on its primary channel. Instead of waiting for the channel to clear, it temporarily switches to a secondary subband, runs a short EDCA-like backoff, transmits DATA, and returns to the BSS primary after an `NPCA_TIMER` (default 5 ms).

The flow is:

```
Detect inter-BSS PPDU → freeze primary backoff → switch to secondary subband
→ draw NPCA backoff → ICF → ICR (from STA) → DATA/ACK → (NPCA_TIMER expires) → return to primary
```

After the NPCA ICF the STA transmits a real ICR frame on the secondary channel (state `STATE_TX_NPCA_ICR`). The AP cancels its safety timeout upon ICR reception and proceeds to DATA. A fallback timer fires if the ICR is lost (e.g., due to a collision on the secondary channel). The STA restores its primary channel automatically after the exchange.

### Requirements

- BSS bandwidth must be ≥ 80 MHz. Nodes with narrower ranges have `npca_enabled` forced to 0.
- The NPCA primary channel (the secondary subband centre) defaults to `(min_ch + max_ch + 1) / 2` if not specified.

### Configuration

NPCA uses optional CSV columns 38–43.

| Column | Field | Default | Description |
|--------|-------|---------|-------------|
| 38 | `npca_enabled` | `0` | `1` to enable NPCA |
| 39 | `npca_primary_channel` | auto | Centre channel of the NPCA subband (−1 = auto) |
| 40 | `npca_min_dur_threshold` | 100 µs | Minimum remaining PPDU duration to trigger a switch |
| 41 | `npca_switching_delay` | 16 µs | Radio switching latency |
| 42 | `npca_switch_back_delay` | 16 µs | Latency when returning to primary |
| 43 | `npca_init_qsrc` | — | Initial QSRC (backoff stage) for NPCA backoff |

**CSV column alignment (columns 32–43):**

```
bf_enabled ; bf_N ; bf_d ; bf_az_deg ; traffic_type ; dso_enabled ; npca_enabled ; npca_primary_channel ; ...
```

> **Warning:** If beamforming is not used, the three BF parameter columns must still be present (even if left as zero or empty). A missing semicolon shifts all subsequent column indices by one and silently disables NPCA.

**Minimal NPCA node CSV entry** (BF disabled, AC_BE, DSO off, NPCA on, auto secondary channel):

```
...<cols 0-31>...;0;;;;0;0;1;-1
```

### Running the NPCA Scenario

```bash
cd Code/main
./komondor_main \
  --nodes ../input/examples/dso_npca_example/input_nodes_npca.csv \
  --time 1.0 --seed 1 --logs-sys 1 --save-node 1
```

Scenario: 2 BSSs × 2 nodes, 80 MHz, BSS colors 1+2, `npca_pch = 2`.

**Results (10 s, seed 1 — real ICR frame exchange):**
- WLAN A throughput: 12.4 Mbps | WLAN B throughput: 13.4 Mbps
- Real ICR receptions at AP: 545 | Safety-timeout fires: 0
- Combined DSO+NPCA: WLAN A (DSO) = 226.9 Mbps, WLAN B (NPCA) = 185.9 Mbps

### Key Timing Constants

| Constant | Default | Description |
|----------|---------|-------------|
| `NPCA_TIMER_DEFAULT_US` | 5000 µs | Maximum time on the NPCA subband per episode |
| `NPCA_ICF_DURATION_US` | 52 µs | Duration of the NPCA ICF frame |
| `NPCA_ICR_DURATION_US` | 28 µs | Duration of the STA ICR response frame |
| `NPCA_SWITCH_TIME_US` | 8 µs | Radio retuning time |

---

## 8. Adaptive ACK Suppression

### Overview

The transmitter maintains a per-destination **Exponential Weighted Moving Average (EWMA)** of ACK success rate. Once the estimate converges above a configurable threshold after a minimum number of observed exchanges, it signals the receiver (via a flag in the DATA frame's `TxInfo`) that no ACK is needed. The receiver skips ACK generation; the transmitter skips `STATE_WAIT_ACK` and records the frame as a success, eliminating one SIFS + ACK overhead per TXOP.

If a subsequent ACK timeout occurs (after ACKs are re-enabled via a probing exchange or when the EWMA drops below threshold), the EWMA is driven downward and ACK suppression remains disabled until reliability recovers.

The feature is **disabled by default** to preserve all regression baselines. Enable it by setting `ACK_SUPPRESS_ENABLED = 1` in `Code/list_of_macros.h`.

### Tunable Constants

All constants are in `Code/list_of_macros.h`:

| Macro | Default | Meaning |
|-------|---------|---------|
| `ACK_SUPPRESS_ENABLED` | `0` | Master switch: `0` = disabled, `1` = enabled |
| `ACK_SUPPRESS_THRESHOLD` | `0.95` | Minimum EWMA success rate required to suppress ACK |
| `ACK_SUPPRESS_EWMA_ALPHA` | `0.1` | EWMA forgetting factor (weight of the newest sample) |
| `ACK_SUPPRESS_MIN_SAMPLES` | `5` | Minimum completed ACK exchanges before suppression may start |

### How to Enable and Test

1. Open `Code/list_of_macros.h` and set:
   ```c
   #define ACK_SUPPRESS_ENABLED  1
   ```

2. Rebuild:
   ```bash
   cd Code/main && make
   ```

3. Run a basic scenario:
   ```bash
   ./komondor_main \
     --nodes ../input/validation/basic_scenarios/input_nodes_scenario_1a_aggregation.csv \
     --out /tmp/out_suppress.txt --code suppress_test \
     --logs-sys 1 --logs-node 1 --save-node 1 \
     --time 10.0 --seed 1
   ```

4. Confirm suppression is active in the logs:
   ```bash
   # Transmitter — frames counted as delivered without waiting for ACK:
   grep "ACK suppressed, assuming" output/logs_nodes/N0*.txt | head

   # Receiver — frames accepted without sending ACK:
   grep "ACK suppressed by TX"     output/logs_nodes/N1*.txt | head
   ```

Expected behaviour: the first 5 DATA exchanges use the normal ACK path (EWMA warm-up). From exchange 6 onward (EWMA ≥ 0.95), ACK overhead is eliminated and throughput rises above the 88.25 Mbps baseline.

### EWMA Adaptation Test

To verify that ACK suppression deactivates when the channel degrades:

1. Enable suppression and run a multi-BSS scenario where collisions are likely.
2. After an ACK timeout, `AckTimeout()` drives the EWMA downward.
3. Once EWMA falls below `ACK_SUPPRESS_THRESHOLD`, the transmitter reverts to requiring ACKs.
4. Search the transmitter log for `Waiting for ACK` resuming after a period of suppression.

### Scope Guards

The prediction function always returns 1 (ACK required) in these cases:

- `ACK_SUPPRESS_ENABLED = 0` (master switch off)
- Broadcast or MAPC-broadcast destinations (`dest_id < 0`)
- MAPC-enabled nodes (MAPC uses custom ACK sequences)
- DSO or NPCA DATA paths (`STATE_TX_DATA_DSO` / `STATE_TX_DATA_NPCA`)
- Fewer than `ACK_SUPPRESS_MIN_SAMPLES` completed exchanges with this destination

---

## 9. External ML Model Integration

### Overview

Komondor can offload agent decisions to an external Python process via a POSIX Unix-domain socket. This allows any Python ML framework (PyTorch, TensorFlow, scikit-learn) to be used without modifying the C++98 simulator binary. The C++ client (`Code/learning_modules/external_model_client.h`) sends a feature vector as binary float32 values and blocks for one float32 result. Simulation time is virtual, so wall-clock inference latency has no effect on correctness.

Set `learning_mechanism = 7` (`LEARNING_MECHANISM_EXTERNAL`) in the agent CSV to enable this path.

### Running the External Model Example

Start the server first (in a separate terminal), then launch the simulation:

```bash
# Terminal 1 — start the server
python3 Code/learning_modules/python_servers/ml_server_passthrough.py /tmp/komondor_ml.sock

# Terminal 2 — run the simulation
cd Code/main
./komondor_main \
    --nodes  ../input/examples/external_model_example/input_nodes.csv \
    --agents ../input/examples/external_model_example/agents_external.csv \
    --time 10.0 --seed 1
```

### Python Servers

Three reference implementations are provided in `Code/learning_modules/python_servers/`:

| File | Purpose |
|------|---------|
| `ml_server_passthrough.py` | Smoke-test: echoes the arm index unchanged. Use to verify socket connectivity. |
| `ml_server_pytorch.py` | PyTorch inference server. Load a pre-trained model and run forward passes at each agent step. |
| `ml_server_random.py` | Random action baseline. Samples a random arm each time, independent of the reward signal. |

### Wire Protocol

| Direction | Format |
|-----------|--------|
| C++ → Python | `uint32_t n_feat` header, then `n_feat x float32` feature vector |
| Python → C++ | 1 x `float32` — the selected arm index |

The default feature vector is `[arm_ix, reward, num_arms]` (3 floats). Custom feature vectors can be built by extending `ExternalModelClient::Query()` in `external_model_client.h`.

The server handles multiple simultaneous connections via `select()`, so a single server process can serve all agents in a multi-WLAN simulation.

For full ML architecture documentation (reward functions, action space, adding new algorithms), see [AIML.md](AIML.md).

---

## 10. Bug Fixes

The following bugs were found and corrected. They affect simulation output regardless of whether the new features are used.

| File | Bug | Effect | Fix |
|------|-----|--------|-----|
| `channel_bonding_methods.h` | `case 8` (160 MHz) lacked `break` — fell through to `case 4` and overwrote channel assignments | 160 MHz CB model silently used 80 MHz selection | Added `break` |
| `channel_bonding_methods.h` | `CB_PROB_UNIFORM_LOG2` had a duplicate `break` | Dead code | Removed extra `break` |
| `output_generation_methods.h` | `ComputeSimulationStatistics` was never called — aggregated stats always 0 for output cases 1, 3, 4, 9 | Simulation summary statistics were always zero | Stats now computed inside `GenerateScriptOutput` |
| `nack_methods.h` | `PACKET_LOST_OUTSIDE_CH_RANGE` case assigned the wrong loss reason (`PACKET_LOST_LOW_SIGNAL_AND_RX`) | Packet loss reason was misreported in logs and output | Correct reason now assigned |
| `node.h` | `power_condition` gate used `ConvertPower(PW_TO_DBM, …)` (pW/dBm mismatch) — `DetectSRTXOPInNavState` was never reached | Spatial reuse TXOP detection during NAV state was always disabled | Removed spurious conversion |
| `node_spatial_reuse_methods.h` | `DetectSRTXOPInNavState`: dBm vs. pW mismatch and unreliable `IsPacketLost` used for the SR check | Spatial reuse opportunity detection was unreliable | Replaced with `IdentifySpatialReuseOpportunity` + pW comparison |
| `input_loader.h` | STA channel bounds were read after `node_params` was populated (~50 lines later in the same loop) — bounds were always 0 | AP always capped TX to a single channel regardless of STA range | Read `IX_MIN_CH_ALLOWED` / `IX_MAX_CH_ALLOWED` directly into early variables before the link-logic block |
| `edca_methods.h` | AIFSN macros had a spurious `=` character; TXOP limit for BE/BK incorrectly treated as unlimited | EDCA AC differentiation broken; BE/BK could aggregate beyond a single PPDU | Renamed macros; TXOP=0 now correctly means single PPDU per TXOP transmission |
| `node_packet_methods.h` | `InitiateBurstPackets` added ~40 packets before TXOP-cap reduction — stale old-timestamped packets accumulated each TXOP for VO/VI | Artificial 2+ second delays for VO/VI traffic in full-buffer scenarios | Flush the entire buffer at TXOP start for `TRAFFIC_FULL_BUFFER_NO_DIFFERENTIATION` |
| `node_fsm_methods.h` | In `HandleStartTX_StateRxData` "NOT THE DESTINATION" path, `ComputeMaxInterference` used `channel_power` (raw path-loss, no BF gain) — AP_A's beamforming null toward STA_B1 was invisible to the SINR gate | In Co-BF scenarios where STA_B1 is geometrically closer to AP_A, the full un-nulled AP_A power caused a spurious "Collision by interferences!" and restarted STA_B1, preventing any ACK and collapsing BSS B throughput to 0 | When `notification.tx_info.beamforming_active`, interference is now summed from `power_received_per_node` (BF-corrected) instead of `channel_power`; same fix applied in `HandleStartTX_StateNav` |
| `node_fsm_methods.h` | No bypass existed in the concurrent-reception SINR gate for Co-BF peer-AP DATA — even with the corrected SINR, edge-case geometry could still trigger `IsPacketLost` | Co-BF DATA from the peer AP treated as a collision despite the coordination handshake having deliberately set the STA's NAV | When both the ongoing reception and the arriving DATA share the same Co-BF MAPC group, `loss_reason` is now forced to `PACKET_NOT_LOST`; `HandleFinishTX_StateRxData` has no final decode gate, so the reception always completes |
| `node_spatial_reuse_methods.h` | DSO and NPCA DATA states were not excluded from the SR TXOP-detection gate — an AP in `STATE_TX_DATA_DSO`/`_NPCA` could misidentify a peer's secondary-subband DATA as an SR opportunity | In mixed DSO+NPCA scenarios the SR module attempted a concurrent transmission, breaking NPCA ACK reception and triggering cascading timeouts | `STATE_TX_DATA_DSO` and `STATE_TX_DATA_NPCA` added to the guard list that bypasses SR opportunity detection |
| `node_packet_methods.h` | In a 2-BSS CB_ALWAYS_MAX_LOG2 scenario with RTS/CTS, simultaneous backoff expiry allowed both APs to independently complete their own RTS/CTS exchange and arrive at `SendResponsePacket STATE_TX_DATA` in the same FIFO tick; the FIFO-loser's DATA overlapped the winner's exchange | Winner's STA received a DATA+DATA collision (SINR < 10 dB) → no ACK → ~10% AckTimeout at the winner AP | Added a full CCA re-check across `[current_left_channel, current_right_channel]` before calling `outportSelfStartTX`; the FIFO-loser aborts and calls `RestartNode` if any channel is busy; guard skips MAPC-coordinated APs where concurrent DATA is intentional |

---

## 11. Internal Refactoring Summary

The following structural changes were made to improve codebase maintainability. Observable simulation behaviour is unchanged for all existing scenarios.

### Methods and Structures Reorganization

- `Code/methods/` reorganized into subfolders: `agent/`, `channel/`, `mac/`, `mapc/`, `node/`, `utils/`
- `channel_bonding_methods.h` extracted from `power_channel_methods.h`
- `SimulationStats` struct replaces 19 module-scope globals in `output_generation_methods.h`
- Logging helpers consolidated in `methods/utils/logging_methods.h` with a canonical signature order
- `sprintf`/`strcat` replaced with `std::ostringstream` throughout output generation

### Node FSM Decomposition

`node.h` was reduced from approximately 5,400 lines to approximately 1,000 lines by extracting per-state handlers and data structures into dedicated files:

| Extracted unit | File |
|----------------|------|
| FSM state handlers | `Code/methods/node/node_fsm_methods.h` |
| Packet and TX logic | `Code/methods/node/node_packet_methods.h` |
| Backoff logic | `Code/methods/node/node_backoff_methods.h` |
| Spatial reuse decisions | `Code/methods/node/node_spatial_reuse_methods.h` |
| Statistics print/write | `Code/methods/node/node_statistics_methods.h` |
| Agent and configuration | `Code/methods/node/node_config_methods.h` |
| Node parameters struct | `Code/structures/node_parameters.h` |
| Statistics struct | `Code/structures/node_statistics.h` |
| Channel access state | `Code/structures/channel_access_state.h` |
| Spatial reuse state | `Code/structures/spatial_reuse_state.h` |
| Packet exchange sequence | `Code/structures/packet_exchange_sequence.h` |

A `ChannelAccessPolicy` function-pointer struct in `channel_access_methods.h` allows pluggable channel-selection strategies (CSMA/CA, Preamble Puncturing, DSO, NPCA) without modifying the main backoff loop.

### Bug Fixes in node.h

| Item | Fix |
|------|-----|
| Uninitialized `nav_collision` / `inter_bss_nav_collision` in `STATE_TX_DATA` | Initialized to `0` at declaration |
| `abs()` called on a `double` timestamp difference | Changed to `fabs()` |
| Buffer overflow: `own_file_path[32]` | Expanded to `[256]`; `sprintf` → `snprintf` |
| Self-assignment `node_logger.file = node_logger.file` | Changed to `= NULL` |
| ~310 lines of commented-out dead code | Removed |
| Typo `channel_max_intereference` | Renamed to `channel_max_interference` across 4 files |

---

## 12. Previous Release Notes

### Breaking Changes

* We have changed the way input information is passed to the simulator:
  * The `input_system` file has been deleted.
  * Additional information has been added to the `input_nodes` file.
  * A new file (`/Komondor/Code/config_models`) has been added, which defines which general models are used to describe the global environment (e.g., path-loss, adjacent channel interference, etc.). These models are typically not modified, but Komondor provides flexibility for testing different environmental cases.

* We have provided a first stable implementation of agents, based on the architectural elements defined in Wilhelmi, Francesc, et al. "A Flexible Machine Learning-Aware Architecture for Future WLANs." [arXiv preprint arXiv:1910.03510](https://arxiv.org/pdf/1910.03510.pdf) (2019).

### New Features

- First version of the RTOT algorithm for SR improvement in 802.11ax WLANs. Refer to Ropitault, Tanguy. "Evaluation of RTOT algorithm: A first implementation of OBSS_PD-based SR method for IEEE 802.11 ax." [2018 15th IEEE Annual Consumer Communications & Networking Conference (CCNC). IEEE](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=8319274), 2018.

### Improvements

- Extended the regression script with DCB use cases.
- Improved the way methods are separated.

### Bug Fixes

- Solved a crash related to agents' output log files.
- Fixed an error whereby transmit power was not being updated when bonding more than one channel for transmitting.

---
