#include <Arduino.h>

#define _EXTALLOC_DEBUGS_ENABLED

extern "C" uint8_t external_psram_size;

bool memory_enable = false;
uint32_t *memory_begin, *memory_end;

bool memory_ok = false;

void init_extmem() {
	uint8_t size = external_psram_size;
#ifdef _EXTALLOC_DEBUGS_ENABLED
	Serial.printf("EXTALLOC: EXTMEM is %d MiB\n", size);

	// Going to be honest here, not sure what this means at all.
	// Probably useful info though. I think it's the SPI clock speed?
	constexpr float clocks[4] = {396.0f, 720.0f, 664.62f, 528.0f};
	const float frequency = clocks[(CCM_CBCMR >> 8) & 3] / static_cast<float>(((CCM_CBCMR >> 29) & 7) + 1);
	Serial.printf("EXTALLOC: CCM_CBCMR=%08X (%.1f MHz)\n", CCM_CBCMR, frequency);
#endif

	if (size == 0) {
		Serial.println("EXTALLOC: No memory detected (0MiB)! Check solder joints.");
		return;
	}

	memory_begin = reinterpret_cast<uint32_t *>(0x70000000);
	memory_end = reinterpret_cast<uint32_t *>(0x70000000 + size * 1048576);
}

void* extalloc(size_t size) {

}

void free(void* ptr) {

}

void setup()
{
	while (!Serial) ; // wait
	init_extmem();

	memory_ok = true;
}


void loop()
{
	digitalWrite(13, HIGH);
	delay(100);
	if (!memory_ok) digitalWrite(13, LOW); // rapid blink if any test fails
	delay(100);
}