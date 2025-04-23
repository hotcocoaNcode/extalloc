//
// Created by Ember Lee on 4/22/25.
//

#ifndef EXTALLOC_H
#define EXTALLOC_H

//#define _EXTALLOC_DEBUGS_ENABLED
#define _EXTALLOC_BLOCKS_PER_ARR 16
#define _EXTALLOC_CHUNK_SIZE 128

namespace extalloc {
    void init();
    volatile void* alloc(size_t size);
    void free(volatile void* ptr);
}

#endif //EXTALLOC_H
