/**
 * @file test_framework.h
 * @brief Simple unit test framework for Komondor managers
 * 
 * This file provides a lightweight testing framework for unit testing
 * the manager classes in the Komondor simulator.
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>

// Test framework macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, message); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            fprintf(stderr, "FAIL: %s:%d: %s (expected: %d, got: %d)\n", \
                    __FILE__, __LINE__, message, (int)(expected), (int)(actual)); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_DOUBLE_EQ(expected, actual, tolerance, message) \
    do { \
        if (fabs((expected) - (actual)) > (tolerance)) { \
            fprintf(stderr, "FAIL: %s:%d: %s (expected: %.10f, got: %.10f)\n", \
                    __FILE__, __LINE__, message, (expected), (actual)); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_TRUE(condition, message) \
    TEST_ASSERT(condition, message)

#define TEST_ASSERT_FALSE(condition, message) \
    TEST_ASSERT(!(condition), message)

// Test suite structure
struct TestSuite {
    const char* name;
    void (*test_func)();
    int test_failed;
};

// Global test counters
extern int test_failed;
extern int test_passed;
extern int test_total;

// Test function prototype
typedef void (*TestFunction)();

// Run a single test
void RunTest(const char* test_name, TestFunction test_func);

// Run all tests in a suite
void RunTestSuite(TestSuite* suite, int num_tests);

// Print test results
void PrintTestResults();

// Initialize test framework
void InitTestFramework();

// Cleanup test framework
void CleanupTestFramework();

#endif // TEST_FRAMEWORK_H

