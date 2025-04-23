#include <Arduino.h>
#include "extalloc.h"

void fill(volatile void* addr, const uint8_t value, const size_t size) {
	for (size_t i = 0; i < size; i++) {
		static_cast<volatile uint8_t*>(addr)[i] = value;
	}
}

void check(volatile void* addr, const uint8_t value, const size_t size) {
	for (size_t i = 0; i < size; i++) {
		if (static_cast<volatile uint8_t*>(addr)[i] != value) {
			Serial.printf("Check for %hhu failed at %08X : %u (size %u)\n", value, addr, i, size);
		}
	}
	Serial.println("Check finished");
}

void setup()
{
	while (!Serial) ; // wait
	ARM_DEMCR |= ARM_DEMCR_TRCENA;
	ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
	uint32_t c_start = ARM_DWT_CYCCNT;
	uint32_t c_end = ARM_DWT_CYCCNT;

	c_start = ARM_DWT_CYCCNT;
	extalloc::init();
	c_end = ARM_DWT_CYCCNT;
	Serial.printf("(our) init took %lu cycles\n", c_end-c_start);

	void* throwaway = extmem_malloc(128);
	c_start = ARM_DWT_CYCCNT;
	void* test = extmem_malloc(256);
	c_end = ARM_DWT_CYCCNT;
	Serial.printf("theirs took %lu cycles\n", c_end-c_start);
	Serial.printf("sanity check; 0x%08X throwaway 0x%08X real\n", throwaway, test);
	fill(test, 0, 256);

	extmem_free(test);
	extmem_free(throwaway);

	volatile void* throwaway2 = extalloc::alloc(128);
	c_start = ARM_DWT_CYCCNT;
	volatile void* test2 = extalloc::alloc(256);
	c_end = ARM_DWT_CYCCNT;
	Serial.printf("ours took %lu cycles\n", c_end-c_start);
	Serial.printf("sanity check; 0x%08X throwaway 0x%08X real\n", throwaway2, test2);
	fill(test2, 0, 256);
	extalloc::free(test2);
	extalloc::free(throwaway2);
}


void loop()
{

}