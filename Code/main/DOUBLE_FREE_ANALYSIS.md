# Double-Free Issue Analysis

## Status: **RELEVANT - Needs to be Fixed**

## Problem Summary

The double-free error (`free(): double free detected in tcache 2`) is occurring because **Node and Manager classes are both allocating and freeing the same arrays**.

## Root Cause

### Channel Arrays (CONFIRMED DOUBLE-ALLOCATION)

1. **Node allocates:**
   - `channels_free = new int[NUM_CHANNELS_KOMONDOR];` (line 5680)
   - `channels_for_tx = new int[NUM_CHANNELS_KOMONDOR];` (line 5681)

2. **NodeChannelManager also allocates:**
   - `channels_free_ = new int[num_channels_];` (in `InitializeChannelArrays()`)
   - `channels_for_tx_ = new int[num_channels_];` (in `InitializeChannelArrays()`)

3. **Both try to free:**
   - Node's destructor (or cleanup) tries to free `channels_free` and `channels_for_tx`
   - Channel Manager's destructor tries to free `channels_free_` and `channels_for_tx_`
   - **If Node's pointers point to Manager's arrays, this causes double-free**

### Statistics Arrays (POTENTIAL ISSUE)

1. **Node still allocates:**
   - `total_time_transmitting_per_channel = new double[NUM_CHANNELS_KOMONDOR];` (line 5678)
   - `total_time_lost_per_channel = new double[NUM_CHANNELS_KOMONDOR];` (line 5682)
   - `total_time_channel_busy_per_channel = new double[NUM_CHANNELS_KOMONDOR];` (line 5684)
   - `num_trials_tx_per_num_channels = new int[NUM_CHANNELS_KOMONDOR];` (line 5686)
   - `total_time_transmitting_in_num_channels = new double[NUM_CHANNELS_KOMONDOR];` (line 5703)
   - `total_time_lost_in_num_channels = new double[NUM_CHANNELS_KOMONDOR];` (line 5704)
   - `last_total_time_transmitting_per_channel = new double[NUM_CHANNELS_KOMONDOR];` (line 5679)
   - `last_total_time_lost_per_channel = new double[NUM_CHANNELS_KOMONDOR];` (line 5683)

2. **NodeStatisticsManager declares but doesn't allocate:**
   - These arrays are declared as private members in Statistics Manager
   - But `InitializeArrays()` only allocates per-STA arrays (not channel arrays)
   - **This is currently safe, but if Statistics Manager starts allocating them, we'll have the same issue**

## Current State

### ✅ Fixed (Option 1 Applied)
- `channel_power` - Removed from Node, accessed via `channel_manager_.GetChannelPowersPtr()`
- `mcs_per_node`, `change_modulation_flag`, `mcs_response` - Removed from Node, accessed via MCS Manager
- Per-STA statistics arrays - Removed from Node, accessed via Statistics Manager

### ❌ Still Problematic
- `channels_free` - Node allocates, Channel Manager also allocates
- `channels_for_tx` - Node allocates, Channel Manager also allocates
- Channel-related statistics arrays - Node allocates, Statistics Manager declares (not yet allocated, but could be)

## Impact

- **Build:** ✅ Successful
- **Runtime:** ❌ Double-free error at cleanup
- **Simulation:** ⚠️ Runs but crashes on exit

## Recommendation

**Apply Option 1 to channel arrays:**

1. Remove `channels_free` and `channels_for_tx` member variables from Node
2. Remove their allocations from `Node::InitializeVariables()`
3. Add getter methods to `NodeChannelManager`:
   - `int* GetChannelsFreePtr()`
   - `int* GetChannelsForTxPtr()`
4. Replace all `channels_free[i]` and `channels_for_tx[i]` accesses with manager methods
5. Update `GetTxChannels()` to use internal arrays if needed

**For statistics arrays:**
- Either remove them from Node and let Statistics Manager manage them
- Or ensure Statistics Manager never allocates them (keep them Node-owned)

## Next Steps

1. Fix channel arrays double-allocation (high priority)
2. Review statistics arrays ownership (medium priority)
3. Test and validate double-free is resolved
4. Run full regression tests

