# Regression Test Guide

## Overview

This guide explains how to run regression tests to validate that the manager integration has not broken existing functionality.

## Prerequisites

1. Komondor must be built successfully
2. Regression test script must be available: `Code/input/script_regression_validation_scenarios.sh`
3. Test scenarios must be configured

## Running Regression Tests

### Quick Test (Single Scenario)

To run a quick test with a single scenario:

```bash
cd /home/sbarrachina/Komondor/Code
./build.sh  # Ensure Komondor is built
cd input
# Run a single test scenario (modify script_regression_validation_scenarios.sh)
bash script_regression_validation_scenarios.sh
```

### Full Regression Test Suite

To run the full regression test suite:

```bash
cd /home/sbarrachina/Komondor/Code/input
bash script_regression_validation_scenarios.sh
```

The script will:
1. Build Komondor
2. Run multiple test scenarios
3. Compare results with baseline values
4. Report any discrepancies

## Test Scenarios

The regression test script includes the following scenarios:

### Basic Scenarios
- **Sce1a (agg)**: Basic scenario with aggregation
- **Sce1a (no agg)**: Basic scenario without aggregation
- **Sce1b (agg)**: Basic scenario variant with aggregation
- **Sce1b (no agg)**: Basic scenario variant without aggregation

### Complex Scenarios
- **Sce2a (agg)**: Complex scenario with aggregation
- **Sce2a (no agg)**: Complex scenario without aggregation
- **Sce2b (agg)**: Complex scenario variant with aggregation
- **Sce2b (no agg)**: Complex scenario variant without aggregation
- **Sce2c (agg)**: Complex scenario variant with aggregation
- **Sce2c (no agg)**: Complex scenario variant without aggregation
- **Sce2d (agg)**: Complex scenario variant with aggregation
- **Sce2d (no agg)**: Complex scenario variant without aggregation

### Channel Bonding Scenarios
- **Sce3a**: 20MHz channel bonding
- **Sce3b**: 40MHz channel bonding
- **Sce3c**: 80MHz channel bonding
- **Sce3d**: 160MHz channel bonding

## Expected Results

### Baseline Values

The regression test script compares results with baseline values:

#### Basic Scenarios
- Sce1a (agg): 88.25 Mbps
- Sce1a (no agg): 22.80 Mbps
- Sce1b (agg): 88.25 Mbps
- Sce1b (no agg): 22.80 Mbps

#### Complex Scenarios
- Sce2a (agg): WLAN_A: 42.11 Mbps, WLAN_B: 41.18 Mbps, WLAN_C: 41.70 Mbps
- Sce2a (no agg): WLAN_A: 9.76 Mbps, WLAN_B: 9.76 Mbps, WLAN_C: 9.76 Mbps
- Sce2b (agg): WLAN_A: 108.83 Mbps, WLAN_B: 1.53 Mbps, WLAN_C: 108.84 Mbps
- Sce2b (no agg): WLAN_A: 20.87 Mbps, WLAN_B: 3.53 Mbps, WLAN_C: 20.87 Mbps
- Sce2c (agg): WLAN_A: 110.21 Mbps, WLAN_B: 67.32 Mbps, WLAN_C: 110.20 Mbps
- Sce2c (no agg): WLAN_A: 23.52 Mbps, WLAN_B: 18.51 Mbps, WLAN_C: 23.53 Mbps
- Sce2d (agg): WLAN_A: 110.21 Mbps, WLAN_B: 110.20 Mbps, WLAN_C: 110.22 Mbps
- Sce2d (no agg): WLAN_A: 23.53 Mbps, WLAN_B: 23.52 Mbps, WLAN_C: 23.53 Mbps

#### Channel Bonding Scenarios
- Sce3a (20MHz): -108.23 dBm
- Sce3b (40MHz): -111.24 dBm
- Sce3c (80MHz): -114.25 dBm
- Sce3d (160MHz): -117.26 dBm

### Tolerance

The regression test script allows for small differences:
- **Throughput tolerance**: 1 Mbps (ALLOWED_ERROR_TPT=1)
- **RSSI tolerance**: 0.1 dBm (ALLOWED_ERROR_RSSI=0.1)

## Validation Process

1. **Build Komondor**: Ensure the latest code is built
2. **Run Test Scenarios**: Execute all test scenarios
3. **Compare Results**: Compare results with baseline values
4. **Check Tolerances**: Verify results are within tolerance
5. **Report Issues**: Report any discrepancies

## Troubleshooting

### Test Failures

If a test fails:
1. Check the error message
2. Verify the build is successful
3. Check input files are correct
4. Verify baseline values are correct
5. Check for any recent changes that might affect results

### Performance Degradation

If performance degrades:
1. Check manager overhead
2. Verify state synchronization is efficient
3. Check for unnecessary copying
4. Profile the code to identify bottlenecks

### Functional Issues

If functionality is broken:
1. Check manager integration
2. Verify state synchronization
3. Check for missing manager calls
4. Verify backward compatibility

## Notes

- Regression tests should be run after each integration phase
- Baseline values may need to be updated if functionality changes
- Tolerance values may need adjustment based on system differences
- Test scenarios should cover all major functionality

## Future Enhancements

1. **Automated Testing**: Automate regression test execution
2. **Continuous Integration**: Integrate regression tests into CI/CD
3. **Performance Testing**: Add performance regression tests
4. **Coverage Analysis**: Add code coverage analysis
5. **Test Reporting**: Improve test reporting and visualization

---

**Last Updated**: Integration Phase 1-3 Complete
**Status**: ✅ Regression Test Guide Created
**Next Step**: Run regression tests to validate functionality

