/**
 * @author ECE 3058 TAs
 */

#include <stdlib.h>
#include "lrustack.h"

/**
 * This file contains some starter code to get you started on your LRU implementation. 
 * You are free to implement it however you see fit. You can design it to emulate how this
 * would be implemented in hardware, or design a purely software stack. 
 * 
 * We have broken down the LRU stack's
 * job/interface into two parts:
 *  - get LRU: gets the current index of the LRU block
 *  - set MRU: sets a certain block's index as the MRU. 
 * If you implement it using these suggestions, you will be able to test your LRU implementation
 * using lrustacktest.c
 * 
 * NOTES: 
 *      - You are not required to use this LRU interface. Feel free to design it from scratch if 
 *      that is better for you.
 *      - This will not behave like your traditional LIFO stack  
 */

/**
 * Function to initialize an LRU stack for a cache set with a given <size>. This function
 * creates the LRU stack. 
 * 
 * @param size is the size of the LRU stack to initialize. 
 * @return the dynamically allocated stack. 
 */
lru_stack_t* init_lru_stack(int size) {
    //  Use malloc to dynamically allocate a lru_stack_t
	lru_stack_t* stack = (lru_stack_t*)malloc(sizeof(lru_stack_t));
    //  Set the stack size the caller passed in
	stack->size = size;
    // Malloc space for the backing array (based on size)
    stack->stack_arr = (int*)malloc(size * sizeof(int));
    // Arbitrarily initialize lru stack values
    int i;
    for(i = 0; i < size; i++) {
        stack->stack_arr[i] = i;
    }
	return stack;
} // init_lru_stack

/**
 * Function to get the index of the least recently used cache block, as indicated by <stack>.
 * This operation should not change/mutate your LRU stack. 
 * 
 * @param stack is the stack to run the operation on.
 * @return the index of the LRU cache block.
 */
int lru_stack_get_lru(lru_stack_t* stack) {
    return stack->stack_arr[stack->size - 1];
} // lru_stack_get_lru

/**
 * Function to mark the cache block with index <n> as MRU in <stack>. This operation should
 * change/mutate the LRU stack.
 * 
 * @param stack is the stack to run the operation on.
 * @param n the index to promote to MRU.  
 */
void lru_stack_set_mru(lru_stack_t* stack, int n) {
    int index_of_n = find_array_index(stack->stack_arr, stack->size, n);

    int i;
    for(i = index_of_n; i >= 1; i--) {
        stack->stack_arr[i] = stack->stack_arr[i - 1];
    }
    stack->stack_arr[0] = n;
} // lru_stack_set_mru

int find_array_index(int arr[], int arr_size, int value) {
    int i;
    for (i = 0; i < arr_size; i++) {
        if (arr[i] == value) {
            return i;
        }
    }
    return -1;
} // find_array_index

/**
 * Function to free up any memory you dynamically allocated for <stack>
 * 
 * @param stack the stack to free
 */
void lru_stack_cleanup(lru_stack_t* stack) {
    ////////////////////////////////////////////////////////////////////
    //  TODO: Write any code if you need to do additional heap allocation
    //  cleanup
    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    //  End of your code   
    ////////////////////////////////////////////////////////////////////

    free(stack);        // Free the stack struct we malloc'd
} // lru_stack_cleanup