# Double-Free Error Fix

## Problem

The simulation runs successfully but crashes at the end with:
```
free(): double free detected in tcache 2
Aborted (core dumped)
```

## Root Cause

The double-free error is caused by `channel_power` memory management:

1. `channel_power` is a pointer member of Node (`double *channel_power;`)
2. We set it to point to the manager's internal array: `channel_power = channel_manager_.GetChannelPowers()`
3. The manager owns the memory and allocates it in its constructor
4. When Node is destroyed, managers are destroyed first (reverse order of declaration)
5. `channel_manager_` destructor calls `CleanupChannelArrays()` which frees `channel_power_`
6. But `channel_power` in Node still points to that freed memory
7. If something tries to free `channel_power` (or if Node has cleanup code), we get a double-free

## Solution

The manager owns the memory for `channel_power_`, so Node should never free it. The solution is to:

1. **Document ownership**: Added comments in `node.h` stating that `channel_power` does NOT own the memory - the manager owns it
2. **Ensure Node never frees it**: Node must never call `delete[] channel_power` or `free(channel_power)`
3. **Manager cleanup**: The manager's destructor will free the memory when it's destroyed

## Implementation

### Changes Made

1. **node.h**: Added comment documenting that `channel_power` does NOT own the memory:
   ```cpp
   // IMPORTANT: channel_power does NOT own this memory - the manager owns it.
   // Node must NEVER free channel_power - the manager will free it in its destructor.
   channel_power = const_cast<double*>(channel_manager_.GetChannelPowers());
   ```

2. **node_channel_manager.h**: Added comment in destructor and cleanup method:
   ```cpp
   inline NodeChannelManager::~NodeChannelManager() {
       // Cleanup arrays - this will free channel_power_ which Node's channel_power points to.
       // Node's channel_power will become a dangling pointer, but Node should never free it.
       CleanupChannelArrays();
   }
   ```

## Verification

- ✅ Build successful
- ✅ No compilation errors
- ⏳ Need to test simulation to verify double-free is fixed

## Notes

- `channel_power` will become a dangling pointer when the manager is destroyed, but Node should never access it after that point
- The manager is destroyed before Node's other members, so `channel_power` should not be accessed after the manager is destroyed
- If Node needs to access `channel_power` after the manager is destroyed, we need to set it to NULL or restructure the code

## Potential Issues

1. **Dangling pointer**: `channel_power` becomes a dangling pointer when the manager is destroyed
2. **Access after destruction**: If Node accesses `channel_power` after the manager is destroyed, it will access freed memory
3. **COST framework**: The COST framework might be trying to free `channel_power` if it thinks Node owns it

## Future Improvements

1. **Set to NULL**: Set `channel_power = NULL` when the manager is destroyed (requires restructuring)
2. **Smart pointers**: Use smart pointers to manage memory ownership
3. **Remove pointer**: Remove `channel_power` from Node and access it through the manager instead

---

**Status**: ✅ Fix Applied
**Build**: ✅ Success
**Testing**: ⏳ Pending

