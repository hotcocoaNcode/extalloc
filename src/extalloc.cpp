//
// Created by Ember Lee on 4/22/25.
//

#include <Arduino.h>
#include "extalloc.h"

extern "C" uint8_t external_psram_size;

namespace extalloc {
	#define block_int_t uint16_t

	struct block {
		/// Chunked block index. _EXTALLOC_CHUNK_SIZE*index + 0x7... = actual memory index
		block_int_t index = 0;
		/// Chunked block size. _EXTALLOC_CHUNK_SIZE*csize = actual size in bytes
		block_int_t csize = 0;
	};

	class chained_block_arr {
	public:
		chained_block_arr* next_arr = nullptr;
		uint8_t top = 0;
		block blocks[_EXTALLOC_BLOCKS_PER_ARR]{};

		void push(const block b) {
			if (top == _EXTALLOC_BLOCKS_PER_ARR - 1) {
				if (next_arr == nullptr) {
					next_arr = static_cast<chained_block_arr*>(malloc(sizeof(chained_block_arr)));
				}
				next_arr->push(b);
			} else {
				blocks[top++] = b;
			}
		}

		void remove(const uint8_t index) {
			const auto length = top-index;
			if (length > 0) {
				// temporarily store the other part of the array
				block temp[_EXTALLOC_BLOCKS_PER_ARR]; // max size it could be
				memcpy(temp, &blocks[index+1], sizeof(block)*length);
				// copy array backwards one place
				memcpy(&blocks[index], &temp, sizeof(block)*length);
			}
			blocks[top--].csize = 0;
		}
	};

	// In theory, you could probably infer gaps from allocations.
	// However, I do not feel like doing that. I am too "eepy" to think this up.
	chained_block_arr gap_arr{};

	// TODO This should be a BST for quick searches in free()
	chained_block_arr alloc_arr{};

	bool memory_enable = false;
	//uint32_t *memory_begin, *memory_end; // literally never used

	void init() {
		const uint8_t size = external_psram_size;
		if (size == 0) {
			Serial.println("extalloc: No memory detected (0MiB)! Check chip and solder joints.");
			Serial.println("extalloc: Defaulting to normal malloc()!");
			return;
		}

		const unsigned int m_size_bytes = size * 1048576;

#ifdef _EXTALLOC_DEBUGS_ENABLED
		Serial.printf("extalloc: memory is %d MiB (%llu bytes)\n", size, m_size_bytes);

		// Going to be honest here, not sure what this means at all.
		// Probably useful info though. I think it's the SPI clock speed?
		constexpr float clocks[4] = {396.0f, 720.0f, 664.62f, 528.0f};
		const float frequency = clocks[(CCM_CBCMR >> 8) & 3] / static_cast<float>(((CCM_CBCMR >> 29) & 7) + 1);
		Serial.printf("extalloc: CCM_CBCMR=%08X (%.1f MHz)\n", CCM_CBCMR, frequency);
#endif
		memory_enable = true;
		gap_arr.push({0, static_cast<block_int_t>(m_size_bytes/_EXTALLOC_CHUNK_SIZE)});
	}

	void check_for_next_block(chained_block_arr *&chunk, int &i) {
		if (i == chunk->top - 1 && chunk->next_arr != nullptr) {
			chunk = chunk->next_arr;
			i = -1;
		}
	}

	volatile void* alloc(const size_t size) {
		if (memory_enable) {
			// Align upwards, then divide by chunk size.
			const block_int_t csize = ((size-1) - (size-1) % _EXTALLOC_CHUNK_SIZE
                                                   + _EXTALLOC_CHUNK_SIZE)/_EXTALLOC_CHUNK_SIZE;

			chained_block_arr* chunk = &gap_arr;
			block* lowest = &chunk->blocks[0];
			for (int i = 0; i < chunk->top; i++) {
				auto* block = &chunk->blocks[i];
				if (block->csize == csize) { // perfec
					lowest = block;
					break;
				}
				if (block->csize <= lowest->csize && csize < block->csize) { // not perfect, but okay
					lowest = block;
				}
				check_for_next_block(chunk, i);
			}

			lowest->csize -= csize;
			const auto addr = reinterpret_cast<void*>(0x70000000 + lowest->index*128);
			lowest->index += csize;

			alloc_arr.push({lowest->index, csize});

			return addr;
		}
		return malloc(size);
	}

	void free(volatile void* ptr) {
		if (memory_enable) {
			const auto index = static_cast<block_int_t>((*reinterpret_cast<uint32_t*>(&ptr)-0x70000000)/128);
			block b{};

			// Find the block ptr goes to
			chained_block_arr* chunk = &alloc_arr;
			for (int i = 0; i < chunk->top; i++) {
				if (chunk->blocks[i].index == index) {
					b = chunk->blocks[i];
					chunk->remove(i);
					break;
				}
				check_for_next_block(chunk, i);
			}
			if (b.csize == 0) { return; } // Couldn't find it, and can't free a null pointer. Don't keep going

			// Reset chunk so we traverse the gap array
			chunk = &gap_arr;
			for (int i = 0; i < _EXTALLOC_BLOCKS_PER_ARR; i++) {
				// This defragmentation "fix" probably won't in most cases. Still helpful maybe

				// Is this gap's end aligned with our start? If so, merge em
				if (chunk->blocks[i].index + chunk->blocks[i].csize == b.index) {
					chunk->blocks[i].csize += b.csize;
					break;
				}
				// Is this gap's start aligned with our end? If so, merge
				if (b.index + b.csize == chunk->blocks[i].index) {
					chunk->blocks[i].index = b.index;
					chunk->blocks[i].csize += b.csize;
					break;
				}

				// We found a place to put this.
                // Not necessarily the best place (see above), but *a* place.
				if (chunk->blocks[i].csize == 0) {
					chunk->blocks[i] = b;
					if (chunk->top <= i) chunk->top++;
					break;
				}
				check_for_next_block(chunk, i);
			}
		} else {
			::free(const_cast<void*>(ptr));
		}
	}
}