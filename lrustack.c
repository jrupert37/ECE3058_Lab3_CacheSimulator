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
    // Use malloc to dynamically allocate an lru_stack_t
	lru_stack_t* stack = (lru_stack_t*)malloc(sizeof(lru_stack_t));
    // Set the stack size the caller passed in
	stack->size = size;
    // Allocate space for the backing array (based on size)
    stack->stack_arr = (int*)malloc(size * sizeof(int));
    // Initialize lru stack values (doesn't matter how)
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
    return stack->stack_arr[stack->size - 1]; // The LRU sits at index (size - 1) of the backing array
} // lru_stack_get_lru

/**
 * Function to mark the cache block with index <n> as MRU in <stack>. This operation should
 * change/mutate the LRU stack.
 * 
 * @param stack is the stack to run the operation on.
 * @param n the index to promote to MRU.  
 */
void lru_stack_set_mru(lru_stack_t* stack, int n) {
    // Use helper function to find index of n
    int index_of_n = find_array_index(stack->stack_arr, stack->size, n); 
    // Shift everything before that index down by one spot in the array...
    int i;
    for(i = index_of_n; i >= 1; i--) {
        stack->stack_arr[i] = stack->stack_arr[i - 1];
    }
    // ...then set "n" as the new MRU (the element at index 0)
    stack->stack_arr[0] = n;
} // lru_stack_set_mru

/**
 * Helper function to find the index of a specific value within an int array. This function
 * searches through the array until a match to the given value is found. The index
 * of that value is returned.
 * 
 * @param arr the integeger array being searched through to find the index of a given value
 * @param size is the size of the given int array
 * @param value is the specific value this function finds the index of in the given array
 * @return the index of the given value in the given array, or -1 if the value does not exist
 *          
*/
int find_array_index(int arr[], int arr_size, int value) {
    int i;
    for (i = 0; i < arr_size; i++) {
        if (arr[i] == value) {
            return i;                   // If the given value is found, return the index of that value
        }
    }
    return -1;                          // Otherwise, return -1
} // find_array_index

/**
 * Function to free up any memory you dynamically allocated for <stack>
 * 
 * @param stack the stack to free
 */
void lru_stack_cleanup(lru_stack_t* stack) {
    free(stack->stack_arr); // Free the backing array of the stack
    free(stack);            // Free the stack struct itself
} // lru_stack_cleanup