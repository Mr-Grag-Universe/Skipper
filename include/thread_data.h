#ifndef MY_THREAD_DATA_header
#define MY_THREAD_DATA_header

#include <stdint.h>
#include <stddef.h>


static const size_t MAP_SIZE = 1025;

typedef struct thread_data
{
    uint64_t location;
    uint8_t map[MAP_SIZE];
};


#endif // MY_THREAD_DATA_header