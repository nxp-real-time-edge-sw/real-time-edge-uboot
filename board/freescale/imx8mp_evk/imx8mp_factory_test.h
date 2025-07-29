#include <common.h>
#include <hang.h>
#include <init.h>
#include <log.h>

#define TEST_SIZE		0x100000
#define NUM_TEST_LOCATIONS   6

// Test locations at 1GB intervals
extern uint64_t test_addresses[NUM_TEST_LOCATIONS]; 

uint32_t generate_test_pattern(uint64_t base_addr, uint32_t offset);
void write_test_data(uint64_t* test_addresses, int num_locations);
void read_and_verify_test_data(uint64_t* test_addresses, int num_locations);
void run_ddr_test(void);
void run_factory_test(void);
