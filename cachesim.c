/**
 * @author ECE 3058 TAs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cachesim.h"

// Statistics you will need to keep track. DO NOT CHANGE THESE.
counter_t accesses = 0;     // Total number of cache accesses
counter_t hits = 0;         // Total number of cache hits
counter_t misses = 0;       // Total number of cache misses
counter_t writebacks = 0;   // Total number of writebacks

/**
 * Function to perform a very basic log2. It is not a full log function, 
 * but it is all that is needed for this assignment. The <math.h> log
 * function causes issues for some people, so we are providing this. 
 * 
 * @param x is the number you want the log of.
 * @returns Techinically, floor(log_2(x)). But for this lab, x should always be a power of 2.
 */
int simple_log_2(int x) {
    int val = 0;
    while (x > 1) {
        x /= 2;
        val++;
    }
    return val; 
} // simple_log_2

//  Here are some global variables you may find useful to get you started.
//  Feel free to add/change anyting here.
cache_set_t* cache;     // Data structure for the cache
int block_size;         // Block size
int cache_size;         // Cache size
int ways;               // Ways
int num_sets;           // Number of sets
int num_offset_bits;    // Number of offset bits
int num_index_bits;     // Number of index bits. 

/**
 * Function to intialize your cache simulator with the given cache parameters. 
 * Note that we will only input valid parameters and all the inputs will always 
 * be a power of 2.
 * 
 * @param _block_size is the block size in bytes
 * @param _cache_size is the cache size in bytes
 * @param _ways is the associativity
 */
void cachesim_init(int _block_size, int _cache_size, int _ways) {
    // Set cache parameters to global variables
    block_size = _block_size;
    cache_size = _cache_size;
    ways = _ways;
    
    // Number of total lines that can be stored in cache = cache_size/block_size. Number of sets = total lines/ways
    num_sets = _cache_size / (_block_size * _ways);
    num_offset_bits = simple_log_2(_block_size); // Number of bits needed for offset depends on how many bytes are in one line (block) of data 
    num_index_bits = simple_log_2(num_sets);     // Number of bits needed for index depends on how many sets we want in the cache

    cache = (cache_set_t*)malloc(num_sets * sizeof(cache_set_t)); // need to allocate the number of cache sets indicated by num_sets

    // Initialize values for members of each set
    int i;
    for (i = 0; i < num_sets; i++) {
        cache[i].size = ways;                   // size of an individual set is equal to the associativity
        cache[i].stack = init_lru_stack(ways);  // initialize an LRU stack for each set
        cache[i].blocks = (cache_block_t*)malloc(ways * sizeof(cache_block_t)); // need to allocate the number of blocks within a set, indicated by ways

        // Set all blocks initially as invalid (valid bit = 0)
        int j;
        for (j = 0; j < cache[i].size; j++) {
            cache[i].blocks[j].valid = 0;
        }
    }
} // cachesim_init

/**
 * Function to perform a SINGLE memory access to your cache. In this function, 
 * you will need to update the required statistics (accesses, hits, misses, writebacks)
 * and update your cache data structure with any changes necessary.
 * 
 * @param physical_addr is the address to use for the memory access. 
 * @param access_type is the type of access - 0 (data read), 1 (data write) or 
 *      2 (instruction read). We have provided macros (MEMREAD, MEMWRITE, IFETCH)
 *      to reflect these values in cachesim.h so you can make your code more readable.
 */
void cachesim_access(addr_t physical_addr, int access_type) {
    int index = parse_index(physical_addr);
    int tag = parse_tag(physical_addr);
    
    int curr = 0;   // index of a specific cache_block_t within a cache_set_t
    cache_block_t* block = &cache[index].blocks[curr]; // the current cache_block, indicated by "curr"

    // Search through the cache blocks of a specific set until either: curr == size -> all blocks have been searched (miss), valid == 0 -> an invalid block is reached (miss), or when the tag bits match (hit!)
    while (curr != cache[index].size && block->valid != 0 && block->tag != tag) {
        curr = curr + 1;                        // Until one of these conditions ^^ are met, increment curr
        block = &cache[index].blocks[curr];     // And move to the next block
    }
    
    switch (access_type) {  // Now that we're done searching, handle the access according to the access_type
        
        // Instr Fetch or Mem Read (treated the same)
        case IFETCH:        // In the case of an IFETCH, just jump to the next case block (MEMREAD), since IFETCH and MEMREAD behave the same
        case MEMREAD: 
        {   
            if (curr == cache[index].size) {    // Read miss! This set is full, need to perform LRU replacement
                int lru_index = lru_stack_get_lru(cache[index].stack);  // Find the LRU block
                if (cache[index].blocks[lru_index].dirty == 1) {        // If the LRU block is dirty -> mem writeback (increment writebacks)
                    writebacks++;
                }  
                cache[index].blocks[lru_index].valid = 1;               // This new block is valid
                cache[index].blocks[lru_index].tag = tag;               // Replace this blocks tag bits with the new tag bits
                cache[index].blocks[lru_index].dirty = 0;               // New block is not dirty (has not been written to)
                lru_stack_set_mru(cache[index].stack, lru_index);       // Now, the (former) LRU becomes the new MRU
                misses++;                                               // Increment misses
            } else if (block->valid == 0) {     // Read miss! No replacement necessary, use empty block
                block->valid = 1;                                       // Set this formally unused (invalid) block to valid
                block->tag = tag;                                       // Save the tag bits
                block->dirty = 0;                                       // New block is not dirty
                lru_stack_set_mru(cache[index].stack, curr);            // Set the new MRU
                misses++;                                               // Increment misses
            } else if (block->tag == tag) {     // Read hit!
                lru_stack_set_mru(cache[index].stack, curr);            // Don't need to do anything except set the new MRU...
                hits++;                                                 // ...and increment hits
            }
            break;
        }

        // Memory Write (store)
        case MEMWRITE:
        {
            if (curr == cache[index].size) {    // Write miss! This set is full, need to perform LRU replacement
                int lru_index = lru_stack_get_lru(cache[index].stack);
                if (cache[index].blocks[lru_index].dirty == 1) {        // Same as IFETCH/MEMREAD above ^^ (fill in empty block, update stack/statistics)...
                    writebacks++;
                }
                cache[index].blocks[lru_index].valid = 1;
                cache[index].blocks[lru_index].tag = tag;
                cache[index].blocks[lru_index].dirty = 1;               // ...BUT NOW, the dirty bit is set to 1 since this line is being written to by the CPU
                lru_stack_set_mru(cache[index].stack, lru_index);
                misses++;
            } else if (block->valid == 0) {     // Write miss! No replacement necessary, use empty block to bring in the line that needs to be updated/written to
                block->valid = 1;
                block->tag = tag;
                block->dirty = 1;                                       // Fill in block, update stack/statistics, set dirty bit to 1
                lru_stack_set_mru(cache[index].stack, curr);
                misses++;
            } else if (block->tag == tag) {     // Write hit!
                block->dirty = 1;                                       // Update stack/statistics, set dirty bit to 1
                lru_stack_set_mru(cache[index].stack, curr);
                hits++;
            }
            break;
        }
    }
    accesses++;                                                         // Increment accesses regardless of access_type
} // cachesim_access

/**
 * Helper function to parse the tag bits from a given memory address. This function simple shifts the address over 
 * by (num_offset_bits + num_index_bits), which will leave behind only the tag bits.
 * 
 * @param physical_addr is the address used for a memory access, from which we want to parse out the tag bits
 * @return the tag bits extracted from the given physical_addr
*/
int parse_tag(addr_t physical_addr) {
    int tag = physical_addr >> (num_offset_bits + num_index_bits); // Shift address so that only tag bits remain
    return tag;
} // parse_tag

/**
 * Helper function to parse the index bits from a given memory address. This function creates a 
 * bit mask to elimated the tag bits from the address. Bit mask is calculated by shifting the 
 * value 1 to the left by the num_offset_bits + num_index_bits, then subtracting 1. This will
 * yield a binary string with as many 1s as there are index bits + offset bits, since these are 
 * the bits we want to keep for now. physical_addr is then ANDed with the bit mask to elimate the tag bits, 
 * yielding a value that will then get shifted to the right by the number of offset bits, leaving 
 * behind only the index bits. 
 * 
 * @param physical_addr is the address used for a memory acces, from which we want to parse out the index bits
 * @return the index bits extracted from the given physical_addr
*/
int parse_index(addr_t physical_addr) {
    int tag_mask = (1 << (num_offset_bits + num_index_bits)) - 1; // Create bit mask string
    int index = physical_addr & tag_mask;                         // Mask out the tag bits from the address
    index = index >> num_offset_bits;                             // Shift address so that only index bits remain
    return index;
} // parse_index

/**
 * Function to free up any dynamically allocated memory you allocated
 */
void cachesim_cleanup() {
    int i;
    for (i = 0; i < num_sets; i++) {        // Loop through each set in the cache
        lru_stack_cleanup(cache[i].stack);  //  For each set, free the lru stack
        free(cache[i].blocks);              //  And free the blocks in that set
    }
    free(cache);                            // Finally, free the cache itself
}

/**
 * Function to print cache statistics
 * DO NOT update what this prints.
 */
void cachesim_print_stats() {
    printf("%llu, %llu, %llu, %llu\n", accesses, hits, misses, writebacks);  
} // cachesim_print_stats

/**
 * Function to open the trace file
 * You do not need to update this function. 
 */
FILE *open_trace(const char *filename) {
    return fopen(filename, "r");
} // open_trace

/**
 * Read in next line of the trace
 * 
 * @param trace is the file handler for the trace
 * @return 0 when error or EOF and 1 otherwise. 
 */
int next_line(FILE* trace) {
    if (feof(trace) || ferror(trace)) return 0;
    else {
        int t;
        unsigned long long address, instr;
        fscanf(trace, "%d %llx %llx\n", &t, &address, &instr);
        cachesim_access(address, t);
    }
    return 1;
} // next_line

/**
 * Main function. See error message for usage. 
 * 
 * @param argc number of arguments
 * @param argv Argument values
 * @returns 0 on success. 
 */
int main(int argc, char **argv) {
    FILE *input;

    if (argc != 5) {
        fprintf(stderr, "Usage:\n  %s <trace> <block size(bytes)>"
                        " <cache size(bytes)> <ways>\n", argv[0]);
        return 1;
    }
    input = open_trace(argv[1]);
    cachesim_init(atol(argv[2]), atol(argv[3]), atol(argv[4]));
    while (next_line(input));
    cachesim_print_stats();
    cachesim_cleanup();
    fclose(input);
    return 0;
} // main
