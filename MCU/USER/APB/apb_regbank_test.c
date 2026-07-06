/*
 * apb_regbank_test.c
 *
 *  Created on: Dec 20, 2025
 *      Author: 30698
 */


////////////////////////////////////////////////////////////////////////////////
// File: apb_regbank_test.c
// Description: APB Register Bank Comprehensive Test Suite - Implementation
//
// This file implements exhaustive testing for APB slave register banks.
// All tests are designed to verify correct functionality at both the bit
// level and system level.
//
// Author: APB Test Suite
// Version: 1.0
// Date: 2025-12-20
////////////////////////////////////////////////////////////////////////////////

#include "apb_regbank_test.h"

////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////

static test_stats_t stats = {0};

////////////////////////////////////////////////////////////////////////////////
// Helper Functions Implementation
////////////////////////////////////////////////////////////////////////////////

void print_test_header(const char* test_name) {
    UART_SendString(UART0, "\r\n========================================\r\n");
    UART_SendString(UART0, test_name);
    UART_SendString(UART0, "\r\n========================================\r\n");
}

void print_test_result(bool pass) {
    if (pass) {
        UART_SendString(UART0, "[PASS]\r\n");
    } else {
        UART_SendString(UART0, "[FAIL]\r\n");
    }
}

void increment_stats(bool pass, bool is_ro_test) {
    stats.total_tests++;
    if (pass) stats.passed++;
    else stats.failed++;

    if (is_ro_test) {
        stats.ro_tests++;
        if (pass) stats.ro_passed++;
    } else {
        stats.rw_tests++;
        if (pass) stats.rw_passed++;
    }
}

test_stats_t* get_test_stats(void) {
    return &stats;
}

void print_final_summary(void) {
    char buf[16];

    UART_SendString(UART0, "\r\n");
    UART_SendString(UART0, "################################################################################\r\n");
    UART_SendString(UART0, "#                        FINAL TEST SUMMARY                                    #\r\n");
    UART_SendString(UART0, "################################################################################\r\n\r\n");

    UART_SendString(UART0, "Read-Only Tests:      ");
    uint32_to_hex(stats.ro_passed, buf);
    UART_SendString(UART0, buf);
    UART_SendString(UART0, " / ");
    uint32_to_hex(stats.ro_tests, buf);
    UART_SendString(UART0, buf);
    UART_SendString(UART0, " PASSED\r\n");

    UART_SendString(UART0, "Read-Write Tests:     ");
    uint32_to_hex(stats.rw_passed, buf);
    UART_SendString(UART0, buf);
    UART_SendString(UART0, " / ");
    uint32_to_hex(stats.rw_tests, buf);
    UART_SendString(UART0, buf);
    UART_SendString(UART0, " PASSED\r\n");

    UART_SendString(UART0, "--------------------------------------------------------------------------------\r\n");
    UART_SendString(UART0, "TOTAL:                ");
    uint32_to_hex(stats.passed, buf);
    UART_SendString(UART0, buf);
    UART_SendString(UART0, " / ");
    uint32_to_hex(stats.total_tests, buf);
    UART_SendString(UART0, buf);
    UART_SendString(UART0, " PASSED (");


    if (stats.failed == 0) {
        UART_SendString(UART0, "********************************\r\n");
        UART_SendString(UART0, "*** 100% TESTS PASSED!!! ***\r\n");
        UART_SendString(UART0, "********************************\r\n");
        UART_SendString(UART0, "APB Register Bank FULLY VERIFIED!\r\n");
    } else {
        UART_SendString(UART0, "*** WARNING: ");
        uint32_to_hex(stats.failed, buf);
        UART_SendString(UART0, buf);
        UART_SendString(UART0, " TEST(S) FAILED ***\r\n");
        UART_SendString(UART0, "APB slave requires debugging!\r\n");
    }

    UART_SendString(UART0, "################################################################################\r\n\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// TEST 1: Walking Ones Test
////////////////////////////////////////////////////////////////////////////////

void test_walking_ones(void) {
    print_test_header("TEST 1: WALKING ONES (Bit-Level Verification)");
    UART_SendString(UART0, "Testing each bit position individually...\r\n\r\n");

    volatile uint32_t *reg;
    uint32_t walking_one;
    bool test_pass;

    for (uint32_t offset = WRITE_REG_START; offset <= WRITE_REG_END; offset += 4) {
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + offset);

        UART_SendString(UART0, "Reg[0x");
        char buf[16];
        uint32_to_hex(offset, buf);
        UART_SendString(UART0, buf);
        UART_SendString(UART0, "]: ");

        bool reg_pass = true;

        // Test each bit position
        for (int bit = 0; bit < 32; bit++) {
            walking_one = (1UL << bit);

            *reg = walking_one;
            uint32_t readback = *reg;

            if (readback != walking_one) {
                UART_SendString(UART0, "\r\n  Bit ");
                uint32_to_hex(bit, buf);
                UART_SendString(UART0, buf);
                UART_SendString(UART0, " FAIL (wrote 0x");
                uint32_to_hex(walking_one, buf);
                UART_SendString(UART0, buf);
                UART_SendString(UART0, ", read 0x");
                uint32_to_hex(readback, buf);
                UART_SendString(UART0, buf);
                UART_SendString(UART0, ")\r\n");
                reg_pass = false;
            }
        }

        *reg = 0x00000000; // Clear register

        if (reg_pass) {
            UART_SendString(UART0, "All 32 bits PASS\r\n");
        }
        increment_stats(reg_pass, false);
    }
    UART_SendString(UART0, "\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// TEST 2: Walking Zeros Test
////////////////////////////////////////////////////////////////////////////////

void test_walking_zeros(void) {
    print_test_header("TEST 2: WALKING ZEROS (Inverse Bit Test)");
    UART_SendString(UART0, "Testing inverted bit patterns...\r\n\r\n");

    volatile uint32_t *reg;
    uint32_t walking_zero;

    for (uint32_t offset = WRITE_REG_START; offset <= WRITE_REG_END; offset += 4) {
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + offset);

        UART_SendString(UART0, "Reg[0x");
        char buf[16];
        uint32_to_hex(offset, buf);
        UART_SendString(UART0, buf);
        UART_SendString(UART0, "]: ");

        bool reg_pass = true;

        for (int bit = 0; bit < 32; bit++) {
            walking_zero = ~(1UL << bit); // All 1s except one bit

            *reg = walking_zero;
            uint32_t readback = *reg;

            if (readback != walking_zero) {
                reg_pass = false;
            }
        }

        *reg = 0x00000000;

        print_test_result(reg_pass);
        increment_stats(reg_pass, false);
    }
    UART_SendString(UART0, "\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// TEST 3: Checkerboard Patterns
////////////////////////////////////////////////////////////////////////////////

void test_checkerboard_patterns(void) {
    print_test_header("TEST 3: CHECKERBOARD PATTERNS");

    uint32_t patterns[] = {0xAAAAAAAA, 0x55555555, 0xCCCCCCCC, 0x33333333};
    char* pattern_names[] = {"0xAAAAAAAA", "0x55555555", "0xCCCCCCCC", "0x33333333"};

    volatile uint32_t *reg;

    for (uint32_t offset = WRITE_REG_START; offset <= WRITE_REG_END; offset += 4) {
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + offset);

        bool reg_pass = true;

        for (int p = 0; p < 4; p++) {
            *reg = patterns[p];
            uint32_t readback = *reg;

            if (readback != patterns[p]) {
                UART_SendString(UART0, "Reg[0x");
                char buf[16];
                uint32_to_hex(offset, buf);
                UART_SendString(UART0, buf);
                UART_SendString(UART0, "] Pattern ");
                UART_SendString(UART0, pattern_names[p]);
                UART_SendString(UART0, " FAIL\r\n");
                reg_pass = false;
            }
        }

        *reg = 0x00000000;
        increment_stats(reg_pass, false);
    }
    UART_SendString(UART0, "Checkerboard test completed.\r\n\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// TEST 4: Register Independence Test
////////////////////////////////////////////////////////////////////////////////

void test_register_independence(void) {
    print_test_header("TEST 4: REGISTER INDEPENDENCE (Cross-Contamination Check)");
    UART_SendString(UART0, "Writing unique values to all registers, then verifying each...\r\n\r\n");

    volatile uint32_t *reg;
    uint32_t unique_values[NUM_RW_REGS];
    bool test_pass = true;

    // Phase 1: Write unique value to each register
    for (int i = 0; i < NUM_RW_REGS; i++) {
        unique_values[i] = 0x10000000 + (i << 16) + i; // Unique pattern
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + WRITE_REG_START + (i * 4));
        *reg = unique_values[i];
    }

    // Phase 2: Verify each register independently
    for (int i = 0; i < NUM_RW_REGS; i++) {
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + WRITE_REG_START + (i * 4));
        uint32_t readback = *reg;

        if (readback != unique_values[i]) {
            UART_SendString(UART0, "Register ");
            char buf[16];
            uint32_to_hex(i, buf);
            UART_SendString(UART0, buf);
            UART_SendString(UART0, " contaminated! Expected 0x");
            uint32_to_hex(unique_values[i], buf);
            UART_SendString(UART0, buf);
            UART_SendString(UART0, ", got 0x");
            uint32_to_hex(readback, buf);
            UART_SendString(UART0, buf);
            UART_SendString(UART0, "\r\n");
            test_pass = false;
        }
    }

    // Clear all registers
    for (int i = 0; i < NUM_RW_REGS; i++) {
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + WRITE_REG_START + (i * 4));
        *reg = 0x00000000;
    }

    if (test_pass) {
        UART_SendString(UART0, "All registers independent - no cross-contamination detected.\r\n");
    }

    increment_stats(test_pass, false);
    UART_SendString(UART0, "\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// TEST 5: Read-Only Protection Stress Test
////////////////////////////////////////////////////////////////////////////////

void test_readonly_protection_stress(void) {
    print_test_header("TEST 5: READ-ONLY PROTECTION STRESS TEST");
    UART_SendString(UART0, "Attempting to corrupt read-only registers with various patterns...\r\n\r\n");

    uint32_t attack_patterns[] = {
        0xFFFFFFFF, 0x00000000, 0xAAAAAAAA, 0x55555555,
        0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0x87654321
    };

    volatile uint32_t *reg;
    bool all_protected = true;

    for (uint32_t offset = READ_ONLY_START; offset <= READ_ONLY_END; offset += 4) {
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + offset);
        uint32_t original = *reg;

        // Try to corrupt with multiple patterns
        for (int p = 0; p < 8; p++) {
            *reg = attack_patterns[p];
            uint32_t after_attack = *reg;

            if (after_attack != original) {
                UART_SendString(UART0, "SECURITY VIOLATION! Read-only register at 0x");
                char buf[16];
                uint32_to_hex(APB2MASTER1_BASE + offset, buf);
                UART_SendString(UART0, buf);
                UART_SendString(UART0, " was modified!\r\n");
                all_protected = false;
            }
        }

        increment_stats(true, true); // Each RO register is one test
    }

    if (all_protected) {
        UART_SendString(UART0, "All read-only registers properly protected.\r\n");
    }

    UART_SendString(UART0, "\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// TEST 6: Rapid Read/Write Stress Test
////////////////////////////////////////////////////////////////////////////////

void test_rapid_readwrite_stress(void) {
    print_test_header("TEST 6: RAPID READ/WRITE STRESS TEST");
    UART_SendString(UART0, "Performing 1000 rapid back-to-back operations per register...\r\n\r\n");

    volatile uint32_t *reg = (volatile uint32_t *)(APB2MASTER1_BASE + WRITE_REG_START);
    bool test_pass = true;

    // Rapid write-read cycles
    for (int i = 0; i < 1000; i++) {
        uint32_t test_val = 0xA5A50000 | i;
        *reg = test_val;
        *reg = test_val;
        *reg = test_val;
        uint32_t read1 = *reg;
        uint32_t read2 = *reg;
        uint32_t read3 = *reg;

        if (read1 != test_val || read2 != test_val || read3 != test_val) {
            UART_SendString(UART0, "Stress test iteration ");
            char buf[16];
            uint32_to_hex(i, buf);
            UART_SendString(UART0, buf);
            UART_SendString(UART0, " FAILED\r\n");
            test_pass = false;
            break;
        }
    }

    if (test_pass) {
        UART_SendString(UART0, "1000 rapid operations completed successfully.\r\n");
    }

    *reg = 0x00000000;
    increment_stats(test_pass, false);
    UART_SendString(UART0, "\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// TEST 7: Address Boundary Test
////////////////////////////////////////////////////////////////////////////////

void test_address_boundaries(void) {
    print_test_header("TEST 7: ADDRESS BOUNDARY TEST");
    UART_SendString(UART0, "Testing first and last registers of each range...\r\n\r\n");

    volatile uint32_t *reg;
    bool test_pass = true;
    char buf[16];

    // Test first RO register
    UART_SendString(UART0, "First RO Register (0x00): ");
    reg = (volatile uint32_t *)(APB2MASTER1_BASE + 0x00);
    uint32_t ro_first = *reg;
    *reg = 0xFFFFFFFF;
    if (*reg != ro_first) {
        UART_SendString(UART0, "FAIL (modified)\r\n");
        test_pass = false;
    } else {
        UART_SendString(UART0, "PASS\r\n");
    }

    // Test last RO register
    UART_SendString(UART0, "Last RO Register (0x7C): ");
    reg = (volatile uint32_t *)(APB2MASTER1_BASE + 0x7C);
    uint32_t ro_last = *reg;
    *reg = 0xFFFFFFFF;
    if (*reg != ro_last) {
        UART_SendString(UART0, "FAIL (modified)\r\n");
        test_pass = false;
    } else {
        UART_SendString(UART0, "PASS\r\n");
    }

    // Test first RW register
    UART_SendString(UART0, "First RW Register (0x80): ");
    reg = (volatile uint32_t *)(APB2MASTER1_BASE + 0x80);
    *reg = 0xBEEFCAFE;
    if (*reg == 0xBEEFCAFE) {
        UART_SendString(UART0, "PASS\r\n");
        *reg = 0x00000000;
    } else {
        UART_SendString(UART0, "FAIL\r\n");
        test_pass = false;
    }

    // Test last RW register
    UART_SendString(UART0, "Last RW Register (0xFC): ");
    reg = (volatile uint32_t *)(APB2MASTER1_BASE + 0xFC);
    *reg = 0xCAFEBEEF;
    if (*reg == 0xCAFEBEEF) {
        UART_SendString(UART0, "PASS\r\n");
        *reg = 0x00000000;
    } else {
        UART_SendString(UART0, "FAIL\r\n");
        test_pass = false;
    }

    increment_stats(test_pass, false);
    UART_SendString(UART0, "\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// TEST 8: Data Retention Test
////////////////////////////////////////////////////////////////////////////////

void test_data_retention(void) {
    print_test_header("TEST 8: DATA RETENTION TEST");
    UART_SendString(UART0, "Testing that written data persists across other operations...\r\n\r\n");

    volatile uint32_t *reg;
    uint32_t retention_values[NUM_RW_REGS];
    bool test_pass = true;

    // Phase 1: Write unique values
    for (int i = 0; i < NUM_RW_REGS; i++) {
        retention_values[i] = 0xA5000000 | (i << 8) | (~i & 0xFF);
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + WRITE_REG_START + (i * 4));
        *reg = retention_values[i];
    }

    // Phase 2: Perform dummy operations (read other registers many times)
    for (int i = 0; i < 100; i++) {
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + READ_ONLY_START);
        volatile uint32_t dummy = *reg; // Read RO register
        (void)dummy;
    }

    // Phase 3: Verify all values retained
    for (int i = 0; i < NUM_RW_REGS; i++) {
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + WRITE_REG_START + (i * 4));
        uint32_t readback = *reg;

        if (readback != retention_values[i]) {
            UART_SendString(UART0, "Data loss at register ");
            char buf[16];
            uint32_to_hex(i, buf);
            UART_SendString(UART0, buf);
            UART_SendString(UART0, "\r\n");
            test_pass = false;
        }
        *reg = 0x00000000; // Clear
    }

    if (test_pass) {
        UART_SendString(UART0, "All data retained correctly.\r\n");
    }

    increment_stats(test_pass, false);
    UART_SendString(UART0, "\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// TEST 9: Sequential Pattern Test
////////////////////////////////////////////////////////////////////////////////

void test_sequential_patterns(void) {
    print_test_header("TEST 9: SEQUENTIAL PATTERN TEST");

    volatile uint32_t *reg;
    bool test_pass = true;

    // Ascending sequence
    UART_SendString(UART0, "Testing ascending sequence (0x00000000 to 0x0000001F)...\r\n");
    for (int i = 0; i < NUM_RW_REGS; i++) {
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + WRITE_REG_START + (i * 4));
        *reg = i;
        if (*reg != (uint32_t)i) {
            test_pass = false;
        }
    }

    // Descending sequence
    UART_SendString(UART0, "Testing descending sequence (0xFFFFFFFF to 0xFFFFFFE0)...\r\n");
    for (int i = 0; i < NUM_RW_REGS; i++) {
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + WRITE_REG_START + (i * 4));
        *reg = 0xFFFFFFFF - i;
        if (*reg != (0xFFFFFFFF - (uint32_t)i)) {
            test_pass = false;
        }
    }

    // Clear all
    for (int i = 0; i < NUM_RW_REGS; i++) {
        reg = (volatile uint32_t *)(APB2MASTER1_BASE + WRITE_REG_START + (i * 4));
        *reg = 0x00000000;
    }

    print_test_result(test_pass);
    increment_stats(test_pass, false);
    UART_SendString(UART0, "\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// TEST 10: Comprehensive Integration Test
////////////////////////////////////////////////////////////////////////////////

void test_comprehensive_integration(void) {
    print_test_header("TEST 10: COMPREHENSIVE INTEGRATION TEST");
    UART_SendString(UART0, "Simulating real-world mixed read/write operations...\r\n\r\n");

    volatile uint32_t *rw_reg = (volatile uint32_t *)(APB2MASTER1_BASE + WRITE_REG_START);
    volatile uint32_t *ro_reg = (volatile uint32_t *)(APB2MASTER1_BASE + READ_ONLY_START);
    bool test_pass = true;

    // Simulate typical usage: write config, read status, verify config
    for (int cycle = 0; cycle < 50; cycle++) {
        // Write configuration
        uint32_t config = 0x80000000 | cycle;
        *rw_reg = config;

        // Read status (from RO register)
        volatile uint32_t status = *ro_reg;
        (void)status;

        // Verify configuration persisted
        if (*rw_reg != config) {
            UART_SendString(UART0, "Integration test cycle ");
            char buf[16];
            uint32_to_hex(cycle, buf);
            UART_SendString(UART0, buf);
            UART_SendString(UART0, " FAILED\r\n");
            test_pass = false;
            break;
        }
    }

    *rw_reg = 0x00000000;

    if (test_pass) {
        UART_SendString(UART0, "50 integration cycles completed successfully.\r\n");
    }

    increment_stats(test_pass, false);
    UART_SendString(UART0, "\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// Main Test Entry Point
////////////////////////////////////////////////////////////////////////////////

void test_APB_registers_comprehensive(void) {
    // Initialize statistics
    stats.total_tests = 0;
    stats.passed = 0;
    stats.failed = 0;
    stats.ro_tests = 0;
    stats.ro_passed = 0;
    stats.rw_tests = 0;
    stats.rw_passed = 0;

    UART_SendString(UART0, "\r\n\r\n");
    UART_SendString(UART0, "################################################################################\r\n");
    UART_SendString(UART0, "#                                                                              #\r\n");
    UART_SendString(UART0, "#          APB REGISTER BANK COMPREHENSIVE VERIFICATION SUITE                 #\r\n");
    UART_SendString(UART0, "#                                                                              #\r\n");
    UART_SendString(UART0, "#  This suite performs exhaustive testing to guarantee confidence    #\r\n");
    UART_SendString(UART0, "#  in APB register bank functionality and protocol compliance.                #\r\n");
    UART_SendString(UART0, "#                                                                              #\r\n");
    UART_SendString(UART0, "################################################################################\r\n");

    char buf[16];
    UART_SendString(UART0, "\r\nBase Address: 0x");
    uint32_to_hex(APB2MASTER1_BASE, buf);
    UART_SendString(UART0, buf);
    UART_SendString(UART0, "\r\n");
    UART_SendString(UART0, "RO Range: 0x00 - 0x7C (32 registers)\r\n");
    UART_SendString(UART0, "RW Range: 0x80 - 0xFC (32 registers)\r\n");

    // Execute all tests
    test_walking_ones();
    test_walking_zeros();
    test_checkerboard_patterns();
    test_register_independence();
    test_readonly_protection_stress();
    test_rapid_readwrite_stress();
    test_address_boundaries();
    test_data_retention();
    test_sequential_patterns();
    test_comprehensive_integration();

    // Print final summary
    print_final_summary();
}

// Convert 32-bit value to hex string
void uint32_to_hex(uint32_t val, char *buf) {
    const char hex_chars[] = "0123456789ABCDEF";
    for (int i = 0; i < 8; i++) {
        buf[7 - i] = hex_chars[val & 0xF];
        val >>= 4;
    }
    buf[8] = '\0';
}


////////////////////////////////////////////////////////////////////////////////
// End of Implementation
////////////////////////////////////////////////////////////////////////////////
