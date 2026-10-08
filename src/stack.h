/**
 * @file stack.h
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
 * @return pointer to initialised stack
 * @return nullptr init failed
 */
stack_t stack_init(void);

/**
 * @brief Checks if the stack is empty
 * @param stk Stack to be analysed
 * @return true Stack is empty
 * @return false Stack is not empty
 */
bool stack_is_empty(stack_t stk);

/**
 * @brief Pushes the value to the top of the stack
 * @param stk target stack
 * @param data pointer to data to be saved
 * @return true = success
 * @return false = failure
 */
bool stack_push(stack_t stk, void *data);

/**
 * @brief Returns the top value of stack
 * @param stk target stack
 * @return Pointer to the top value
 * @return exit(1) if stack is empty or non existent, check with stack_is_empty()
 */
void *stack_top(stack_t stk);

/**
 * @brief Removes the top item in stack.
 * Free stored allocated memory before calling stack_pop()!!!
 * @param stk target stack
 * @return true = success
 * @return false = failure
 */
bool stack_pop(stack_t stk);

/**
 * @brief Frees all alocated memory of stack
 * @return true Stack and content memory freed
 * @return false Failure (adress not found)
 */
bool stack_dtor(stack_t stk);

#endif