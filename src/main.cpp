#include <Arduino.h>

#define _EXTALLOC_DEBUGS_ENABLED
#define _EXTALLOC_GAP_COUNT 128

extern "C" uint8_t external_psram_size;

namespace extalloc {
	struct gap {
		union {
			void* pointer = nullptr;
			unsigned int pointer_ub_fuckery;
		};
		size_t size = 0;
	};

	struct gap_chunk {
		gap_chunk* next = nullptr;
		gap gaps[_EXTALLOC_GAP_COUNT]{};
	};

	gap_chunk base_chunk{};

	bool memory_enable = false;
	uint32_t *memory_begin, *memory_end;

	void init() {
		const uint8_t size = external_psram_size;
#ifdef _EXTALLOC_DEBUGS_ENABLED
		Serial.printf("extalloc: memory is %d MiB\n", size);

		// Going to be honest here, not sure what this means at all.
		// Probably useful info though. I think it's the SPI clock speed?
		constexpr float clocks[4] = {396.0f, 720.0f, 664.62f, 528.0f};
		const float frequency = clocks[(CCM_CBCMR >> 8) & 3] / static_cast<float>(((CCM_CBCMR >> 29) & 7) + 1);
		Serial.printf("extalloc: CCM_CBCMR=%08X (%.1f MHz)\n", CCM_CBCMR, frequency);
#endif

		if (size == 0) {
			Serial.println("extalloc: No memory detected (0MiB)! Check chip and solder joints.");
			Serial.println("extalloc: Defaulting to normal malloc()!");
			return;
		}

		memory_enable = true;

		const unsigned int m_size_bytes = size * 1048576;

		memory_begin = reinterpret_cast<uint32_t*>(0x70000000); // memory start constant
		memory_end = reinterpret_cast<uint32_t*>(0x70000000 + m_size_bytes);  // m start, 1MiB

		base_chunk.gaps[0].pointer = memory_begin;
		base_chunk.gaps[0].size = m_size_bytes;
	}

	volatile void* alloc(size_t size) {
		if (memory_enable) {
			volatile void* result = nullptr;
			gap_chunk* chunk = &base_chunk;
			for (auto & gap : chunk->gaps) {
				if (gap.size == size) {
					gap.size = 0;
					result = gap.pointer;
				}
				if (gap.size > size) {
					gap.size -= size;
					result = gap.pointer;
					gap.pointer_ub_fuckery += size;
				}
			}
			return result;
		} else {
			return malloc(size);
		}
	}

	void free(void* ptr) {
		if (memory_enable) {

		} else {
			::free(ptr);
		}
	}
}

void setup()
{
	while (!Serial) ; // wait
	extalloc::init();
}


void loop()
{

}