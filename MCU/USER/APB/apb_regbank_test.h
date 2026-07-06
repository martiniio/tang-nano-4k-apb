/*
 * apb_regbank_test.h
 *
 *  Created on: Dec 20, 2025
 *      Author: 30698
 */


////////////////////////////////////////////////////////////////////////////////
// File: apb_regbank_test.h
// Description: APB Register Bank Comprehensive Test Suite - Header File
//
// This library provides exhaustive testing for APB slave register banks with
// read-only and read-write register ranges. Tests include bit-level verification,
// security testing, stress testing, and protocol compliance validation.
//
// Usage:
//   1. Include this header in your main.c
//   2. Call test_APB_registers_comprehensive() to run all tests
//   3. Check UART output for results
//
// Author: APB Test Suite
// Version: 1.0
// Date: 2025-12-20
////////////////////////////////////////////////////////////////////////////////

#ifndef USER_APB_APB_REGBANK_TEST_H_
#define USER_APB_APB_REGBANK_TEST_H_

#include "gw1ns4c.h"
#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Configuration - Modify these for your system
////////////////////////////////////////////////////////////////////////////////


#define READ_ONLY_START     0x00        // Start offset of read-only registers
#define READ_ONLY_END       0x7C        // End offset of read-only registers
#define WRITE_REG_START     0x80        // Start offset of read-write registers
#define WRITE_REG_END       0xFC        // End offset of read-write registers
#define NUM_RO_REGS         32          // Number of read-only registers
#define NUM_RW_REGS         32          // Number of read-write registers

////////////////////////////////////////////////////////////////////////////////
// External Dependencies - Must be provided by user
////////////////////////////////////////////////////////////////////////////////

// User must provide these UART functions:
// void UART_SendString(void* uart, const char* str);
// void uint32_to_hex(uint32_t value, char* buf);


extern void uint32_to_hex(uint32_t value, char* buf);


////////////////////////////////////////////////////////////////////////////////
// Test Statistics Structure
////////////////////////////////////////////////////////////////////////////////

typedef struct {
    uint32_t total_tests;   // Total number of tests executed
    uint32_t passed;        // Number of tests passed
    uint32_t failed;        // Number of tests failed
    uint32_t ro_tests;      // Number of read-only tests
    uint32_t ro_passed;     // Number of read-only tests passed
    uint32_t rw_tests;      // Number of read-write tests
    uint32_t rw_passed;     // Number of read-write tests passed
} test_stats_t;

////////////////////////////////////////////////////////////////////////////////
// Public API - Main Test Entry Point
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Run comprehensive APB register bank test suite
 *
 * Executes 10 different test categories covering:
 *   - Bit-level verification
 *   - Pattern testing
 *   - Register independence
 *   - Security (read-only protection)
 *   - Performance (stress testing)
 *   - Boundary conditions
 *   - Data retention
 *   - Protocol compliance
 *
 * @note This function will output results via UART
 * @note Test duration: approximately 5-10 seconds
 */
void test_APB_registers_comprehensive(void);

////////////////////////////////////////////////////////////////////////////////
// Individual Test Functions (can be called separately if needed)
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Test each bit individually by walking a '1' through all positions
 */
void test_walking_ones(void);

/**
 * @brief Test each bit by walking a '0' through all positions (inverted)
 */
void test_walking_zeros(void);

/**
 * @brief Test with checkerboard and alternating bit patterns
 */
void test_checkerboard_patterns(void);

/**
 * @brief Verify that writing to one register doesn't affect others
 */
void test_register_independence(void);

/**
 * @brief Aggressively attempt to write to read-only registers
 */
void test_readonly_protection_stress(void);

/**
 * @brief Perform 1000 rapid back-to-back read/write operations
 */
void test_rapid_readwrite_stress(void);

/**
 * @brief Test first and last registers of each range
 */
void test_address_boundaries(void);

/**
 * @brief Verify written data persists across other operations
 */
void test_data_retention(void);

/**
 * @brief Test with incrementing/decrementing sequences
 */
void test_sequential_patterns(void);

/**
 * @brief Simulate real-world mixed read/write operations
 */
void test_comprehensive_integration(void);

////////////////////////////////////////////////////////////////////////////////
// Helper Functions
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Print formatted test header
 * @param test_name Name of the test being executed
 */
void print_test_header(const char* test_name);

/**
 * @brief Print test result (PASS/FAIL)
 * @param pass True if test passed, false otherwise
 */
void print_test_result(bool pass);

/**
 * @brief Update test statistics
 * @param pass True if test passed
 * @param is_ro_test True if this is a read-only test
 */
void increment_stats(bool pass, bool is_ro_test);

/**
 * @brief Print final test summary with statistics
 */
void print_final_summary(void);

/**
 * @brief Get current test statistics
 * @return Pointer to test statistics structure
 */
test_stats_t* get_test_stats(void);


////////////////////////////////////////////////////////////////////////////////
// End of Header File
////////////////////////////////////////////////////////////////////////////////
#endif /* USER_APB_APB_REGBANK_TEST_H_ */
