# Double-Free Error Issue

## Problem

The simulation runs successfully but crashes at the end with:
```
free(): double free detected in tcache 2
Aborted (core dumped)
```

## Root Cause Analysis

1. **Original Code**: `channel_power` was allocated with `new double[NUM_CHANNELS_KOMONDOR]` in `InitializeVariables()`
2. **Manager Integration**: We removed the allocation and made `channel_power` point to `channel_manager_.channel_power_`
3. **Memory Ownership**: The manager owns the memory and allocates it in its constructor
4. **Destruction Order**: When Node is destroyed:
   - Managers are destroyed first (reverse order of declaration)
   - `channel_manager_` destructor calls `CleanupChannelArrays()` which frees `channel_power_`
   - `channel_power` in Node becomes a dangling pointer
5. **Double-Free**: Something (possibly COST framework or cleanup code) is trying to free `channel_power` after the manager has already freed it

## Attempted Fixes

1. ✅ **Added comments**: Documented that `channel_power` does NOT own the memory
2. ✅ **Set to NULL in Stop()**: Added `channel_power = NULL;` in `Stop()` method
3. ✅ **Initialize to NULL**: Initialized `channel_power = NULL` in Node constructor
4. ❌ **Still failing**: Double-free error persists

## Current Status

- **Build**: ✅ Success
- **Simulation**: ✅ Runs successfully
- **Cleanup**: ❌ Crashes with double-free error

## Possible Solutions

### Option 1: Don't Share Pointer (Recommended)
Instead of making `channel_power` point to the manager's array, access it through the manager:
- Change all `channel_power[i]` to `channel_manager_.GetChannelPowers()[i]`
- Remove `channel_power` pointer from Node
- This requires changing many locations but is the cleanest solution

### Option 2: Keep Own Copy
Node allocates its own `channel_power` array and syncs with manager:
- Node allocates `channel_power = new double[NUM_CHANNELS_KOMONDOR]`
- Sync `channel_power` with manager's array when needed
- Node frees its own copy in cleanup
- This adds memory overhead but maintains backward compatibility

### Option 3: Smart Pointer
Use `std::shared_ptr` or `std::unique_ptr` to manage memory:
- Manager uses `std::shared_ptr<double[]>` for `channel_power_`
- Node's `channel_power` is also a `shared_ptr`
- Memory is automatically managed
- This requires C++11/14 support

### Option 4: Manager Cleanup Callback
Add a callback mechanism so manager can notify Node when cleaning up:
- Manager calls a callback before freeing memory
- Node sets `channel_power = NULL` in the callback
- This is complex but maintains current architecture

## Recommendation

**Option 1** is the cleanest and most maintainable solution. It eliminates the double-free issue completely by removing the shared pointer. However, it requires changing many locations in the code.

**Option 2** is the quickest fix that maintains backward compatibility. It adds memory overhead but is simple to implement.

## Next Steps

1. Decide on which solution to implement
2. Implement the chosen solution
3. Test to verify the double-free is fixed
4. Update documentation

## Notes

- The error occurs during cleanup/destruction, not during simulation
- The simulation runs successfully, so the issue is purely a cleanup problem
- The COST framework might be trying to free `channel_power` if it detects it was allocated
- Setting `channel_power = NULL` in `Stop()` doesn't prevent the double-free, suggesting the free happens after `Stop()` or in a different code path

---

**Status**: ⚠️ Issue Identified, Fix Pending
**Priority**: High (causes crash on exit)
**Impact**: Simulation works but crashes on cleanup

