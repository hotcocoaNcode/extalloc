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
	extalloc::init();
	ARM_DEMCR |= ARM_DEMCR_TRCENA;
	ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;

	volatile void *a = extalloc::alloc(256);
	volatile void *b = extalloc::alloc(256);
	volatile void *c = extalloc::alloc(256);
	volatile void *d = extalloc::alloc(256);

	fill(a, 0, 256);
	fill(b, 1, 256);
	fill(c, 2, 256);
	fill(d, 3, 256);

	check(a, 0, 256);
	check(b, 1, 256);
	check(c, 2, 256);
	check(d, 3, 256);

	Serial.printf("a %08X, b %08X, c %08X, d %08X\n", a, b, c, d);

	extalloc::free(c);



	uint32_t c_start = ARM_DWT_CYCCNT;
	volatile void *e = extalloc::alloc(128);
	uint32_t c_end = ARM_DWT_CYCCNT;
	Serial.printf("e's alloc took %lu cycles\n", c_end-c_start);
	c_start = ARM_DWT_CYCCNT;
	volatile void *f = extalloc::alloc(128);
	c_end = ARM_DWT_CYCCNT;
	Serial.printf("f's alloc took %lu cycles\n", c_end-c_start);

	Serial.printf("e %08X, f %08X\n", e, f);


	fill(e, 4, 128);
	fill(f, 5, 128);

	check(e, 4, 128);
	check(f, 5, 128);

	c_start = ARM_DWT_CYCCNT;
	extalloc::free(a);
	c_end = ARM_DWT_CYCCNT;
	Serial.printf("a's free took %lu cycles\n", c_end-c_start);

	c_start = ARM_DWT_CYCCNT;
	extalloc::free(b);
	extalloc::free(d);
	extalloc::free(e);
	extalloc::free(f);
	c_end = ARM_DWT_CYCCNT;
	Serial.printf("the rest took %lu cycles\n", c_end-c_start);
}


void loop()
{

}