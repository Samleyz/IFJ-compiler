/**
 * @file stack.c
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @brief public interface for stack adt
 */

#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

struct stack;
struct stack_item;

typedef struct stack *stack_t;
typedef struct stack_item *stack_item_t;

/**
 * @brief Initialises the stack
 * @return true Stack initialised
 * @return false Failure (malloc failure)
 */
stack_t stack_init();

/**
 * @brief Frees all alocated memory of stack
 * @return true Stack and content memory freed
 * @return false Failure (adress not found)
 */
bool stack_dtor(stack_t stk);

/**
 * @brief  Initialises the stack item
 * @return true Stack Item initialised
 * @return false Failure (malloc failure)
 */
stack_item_t s_item_ctor();

/**
 * @brief Freeing the item memory
 * @param itm target item
 * @return true
 * @return false
 */
bool s_item_dtor(stack_item_t itm);

/**
 * @brief Pushes the value to the top of the stack
 * @param stk target stack
 * @param itm item to be pushed to stack
 * @return true = success
 * @return false = failure
 */
bool stack_push(stack_t stk, stack_item_t itm);

/**
 * @brief Returns the top value of stack
 * @param stk target stack
 * @return Pointer of the top stack item on success
 * @return NULL if stack is empty
 */
stack_item_t *stack_top(stack_t stk);

/**
 * @brief Removes the top item in stack
 * @param stk target stack
 * @return true = success
 * @return false = failure
 */
bool stack_pop(stack_t stk);

#endif