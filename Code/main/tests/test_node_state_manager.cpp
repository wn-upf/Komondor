/**
 * @file test_node_state_manager.cpp
 * @brief Unit tests for NodeStateManager
 */

#include "test_framework.h"
#include "../node_state_manager.h"
#include "../node_internal/node_types.h"
#include <cstdio>

// Test state transitions
void TestStateTransitions() {
    NodeStateManager manager;
    
    // Test initial state
    TEST_ASSERT_EQ(manager.GetState(), NodeInternal::NODE_STATE_SENSING, 
                   "Initial state should be SENSING");
    
    // Test state transitions
    manager.SetState(NodeInternal::NODE_STATE_TX_RTS);
    TEST_ASSERT_EQ(manager.GetState(), NodeInternal::NODE_STATE_TX_RTS,
                   "State should be TX_RTS after SetState");
    
    manager.SetState(NodeInternal::NODE_STATE_TX_DATA);
    TEST_ASSERT_EQ(manager.GetState(), NodeInternal::NODE_STATE_TX_DATA,
                   "State should be TX_DATA after SetState");
    
    manager.SetState(NodeInternal::NODE_STATE_RX_DATA);
    TEST_ASSERT_EQ(manager.GetState(), NodeInternal::NODE_STATE_RX_DATA,
                   "State should be RX_DATA after SetState");
}

// Test state checks
void TestStateChecks() {
    NodeStateManager manager;
    
    // Test IsSensing
    manager.SetState(NodeInternal::NODE_STATE_SENSING);
    TEST_ASSERT_TRUE(manager.IsSensing(), "Should be sensing");
    TEST_ASSERT_FALSE(manager.IsTransmitting(), "Should not be transmitting");
    TEST_ASSERT_FALSE(manager.IsReceiving(), "Should not be receiving");
    
    // Test IsTransmitting
    manager.SetState(NodeInternal::NODE_STATE_TX_DATA);
    TEST_ASSERT_TRUE(manager.IsTransmitting(), "Should be transmitting");
    TEST_ASSERT_FALSE(manager.IsSensing(), "Should not be sensing");
    TEST_ASSERT_FALSE(manager.IsReceiving(), "Should not be receiving");
    
    // Test IsReceiving
    manager.SetState(NodeInternal::NODE_STATE_RX_DATA);
    TEST_ASSERT_TRUE(manager.IsReceiving(), "Should be receiving");
    TEST_ASSERT_FALSE(manager.IsSensing(), "Should not be sensing");
    TEST_ASSERT_FALSE(manager.IsTransmitting(), "Should not be transmitting");
}

// Test NAV state
void TestNavState() {
    NodeStateManager manager;
    
    // Test IsInNAV
    manager.SetState(NodeInternal::NODE_STATE_NAV);
    TEST_ASSERT_TRUE(manager.IsInNAV(), "Should be in NAV");
    TEST_ASSERT_FALSE(manager.IsSensing(), "Should not be sensing");
    
    // Test state conversion
    manager.SetStateFromInt(12); // STATE_NAV
    TEST_ASSERT_EQ(manager.GetStateAsInt(), 12, "State should be 12 (NAV)");
}

// Test state conversion
void TestStateConversion() {
    NodeStateManager manager;
    
    // Test SetStateFromInt and GetStateAsInt
    manager.SetStateFromInt(0); // STATE_SENSING
    TEST_ASSERT_EQ(manager.GetStateAsInt(), 0, "State should be 0 (SENSING)");
    
    manager.SetStateFromInt(1); // STATE_TX_DATA
    TEST_ASSERT_EQ(manager.GetStateAsInt(), 1, "State should be 1 (TX_DATA)");
    
    manager.SetStateFromInt(2); // STATE_RX_DATA
    TEST_ASSERT_EQ(manager.GetStateAsInt(), 2, "State should be 2 (RX_DATA)");
}

int main() {
    InitTestFramework();
    
    printf("=== NodeStateManager Tests ===\n\n");
    
    RunTest("StateTransitions", TestStateTransitions);
    RunTest("StateChecks", TestStateChecks);
    RunTest("NavState", TestNavState);
    RunTest("StateConversion", TestStateConversion);
    
    CleanupTestFramework();
    
    return (test_passed == test_total) ? 0 : 1;
}

