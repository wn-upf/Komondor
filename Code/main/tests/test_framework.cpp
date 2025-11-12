/**
 * @file test_framework.cpp
 * @brief Implementation of simple unit test framework
 */

#include "test_framework.h"
#include <cstdio>

// Global test counters
int test_failed = 0;
int test_passed = 0;
int test_total = 0;

void RunTest(const char* test_name, TestFunction test_func) {
    test_total++;
    test_failed = 0;
    
    printf("Running test: %s... ", test_name);
    fflush(stdout);
    
    test_func();
    
    if (test_failed == 0) {
        test_passed++;
        printf("PASS\n");
    } else {
        printf("FAIL\n");
    }
}

void RunTestSuite(TestSuite* suite, int num_tests) {
    printf("\n=== Running test suite: %s ===\n\n", suite->name);
    
    for (int i = 0; i < num_tests; i++) {
        test_failed = 0;
        RunTest(suite[i].name, suite[i].test_func);
    }
}

void PrintTestResults() {
    printf("\n=== Test Results ===\n");
    printf("Total tests: %d\n", test_total);
    printf("Passed: %d\n", test_passed);
    printf("Failed: %d\n", test_total - test_passed);
    printf("Success rate: %.1f%%\n", (test_passed * 100.0) / test_total);
}

void InitTestFramework() {
    test_failed = 0;
    test_passed = 0;
    test_total = 0;
    printf("Test framework initialized\n");
}

void CleanupTestFramework() {
    PrintTestResults();
}

