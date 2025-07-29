#include "imx8mp_factory_test.h"

uint64_t test_addresses[NUM_TEST_LOCATIONS] = {
    0x40000000ULL,
    0x80000000ULL,
    0xC0000000ULL,
    0x100000000ULL,
    0x140000000ULL,
    0x180000000ULL
};

// Generate test pattern based on address
uint32_t generate_test_pattern(uint64_t base_addr, uint32_t offset)
{
	return (uint32_t)(base_addr >> 8) ^ offset ^ 0xA5A5A5A5;
}

void write_test_data(uint64_t* test_addresses, int num_locations)
{

	for (int loc = 0; loc < num_locations; loc++) {
		volatile uint32_t *test_addr = (uint32_t*)test_addresses[loc];

		uint32_t addr_high = (uint32_t)(test_addresses[loc] >> 32);
		uint32_t addr_low = (uint32_t)(test_addresses[loc] & 0xFFFFFFFF);

		if(addr_high != 0){
			printf("DDRINFO: Writing to location %d: 0x%08x%08x\n", loc + 1, addr_high, addr_low);
		} else {
			printf("DDRINFO: Writing to location %d: 0x%08x\n", loc + 1, addr_low);
		}


		// Write 1MB of test data (256K words)
		for (uint32_t i = 0; i < (TEST_SIZE / 4); i++) {
			uint32_t test_pattern = generate_test_pattern(test_addresses[loc], i);
			test_addr[i] = test_pattern;
		}
		printf(" Location %d write complete\n", loc + 1);
	}
}

void read_and_verify_test_data(uint64_t* test_addresses, int num_locations)
{
	int test_passed = 1;
	uint32_t total_errors = 0;

	for (int loc = 0; loc < num_locations; loc++) {
		volatile uint32_t *test_addr = (uint32_t*)test_addresses[loc];
		uint32_t location_errors = 0;

		uint32_t addr_high = (uint32_t)(test_addresses[loc] >> 32);
		uint32_t addr_low = (uint32_t)(test_addresses[loc] & 0xFFFFFFFF);

		if(addr_high != 0){
			printf("DDRINFO: Verifying location %d: 0x%08x%08x\n", loc + 1, addr_high, addr_low);
		} else {
			printf("DDRINFO: Verifying to location %d: 0x%08x\n", loc + 1, addr_low);
		}
		// Verify 1MB of test data
		for (uint32_t i = 0; i < (TEST_SIZE / 4); i++) {
			uint32_t expected_pattern = generate_test_pattern(test_addresses[loc], i);
			uint32_t read_value = test_addr[i];

			if (read_value != expected_pattern) {
				if (location_errors < 10) {  // Limit error prints
					printf("ERROR at 0x%08x%08x+0x%x: expected 0x%08x, got 0x%08x\n", 
									 addr_high, addr_low, i * 4, expected_pattern, read_value);
				}
				location_errors++;
				test_passed = 0;
			}
		}

		if (location_errors == 0) {
			printf(" Location %d verification PASSED\n", loc + 1);
		} else {
			printf(" Location %d verification FAILED (%d errors)\n", loc + 1, location_errors);
			total_errors += location_errors;
		}
	}
	
	if(test_passed){
		printf("DDRINFO: 1MB DDR test PASSED - All 6 locations verified successfully\n");
	} else {
		printf("DDRINFO: 1MB DDR test FAILED - Total errors: %d\n", total_errors);
	}
}

void run_ddr_test(void)
{
	printf("DDRINFO: Writing 1MB test data at 6 locations...\n");
	write_test_data(test_addresses, NUM_TEST_LOCATIONS);
	printf("DDRINFO: Reading back and verifying test data...\n");
	read_and_verify_test_data(test_addresses, NUM_TEST_LOCATIONS);
}

void run_factory_test(void)
{
	printf("DDRINFO: Starting 1MB DDR test at 1GB intervals\n");
	run_ddr_test();

	// Hang after all the tests are completed
	for(;;);
}
