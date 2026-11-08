/**
 * @file instruction_set.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Instruction interface for code generation
 * @date 2026-11-07
 */

#ifndef INSTRUCTION_SET_H
#define INSTRUCTION_SET_H

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Memory frame types supported in IFJcode26
 */
typedef enum {
	FT_NO_FRAME, /**< No frame (for literals and labels) */
	FT_GLOBAL,	 /**< Global frame (GF@) */
	FT_LOCAL,	 /**< Local frame (LF@) */
	FT_TEMP,	 /**< Temporary frame (TF@) */
} frame_type_t;

/**
 * @brief Types of instruction operands
 */
typedef enum operand_type {
	OP_TYPE_VAR,	/**< Unique variable identifier */
	OP_TYPE_INT,	/**< Integer literal */
	OP_TYPE_FLOAT,	/**< Floating-point literal */
	OP_TYPE_STRING, /**< String literal */
	OP_TYPE_BOOL,	/**< Boolean literal */
	OP_TYPE_LABEL,	/**< Jump or call target label */
	OP_TYPE_NIL,	/**< Nil constant */
} operand_type_t;

/**
 * @brief Supported IFJcode26 instruction opcodes
 */
typedef enum instruction_type {
	INST_MOVE,
	INST_CREATEFRAME,
	INST_PUSHFRAME,
	INST_POPFRAME,
	INST_DEFVAR,
	INST_CALL,
	INST_RETURN,
	INST_PUSHS,
	INST_POPS,
	INST_CLEARS,
	INST_ADD,
	INST_SUB,
	INST_MUL,
	INST_DIV,
	INST_IDIV,
	INST_ADDS,
	INST_SUBS,
	INST_MULS,
	INST_DIVS,
	INST_IDIVS,
	INST_LT,
	INST_GT,
	INST_EQ,
	INST_LTS,
	INST_GTS,
	INST_EQS,
	INST_AND,
	INST_OR,
	INST_NOT,
	INST_ANDS,
	INST_ORS,
	INST_NOTS,
	INST_INT2FLOAT,
	INST_FLOAT2INT,
	INST_INT2CHAR,
	INST_STRI2INT,
	INST_FLOAT2STR,
	INST_INT2STR,
	INST_INT2FLOATS,
	INST_FLOAT2INTS,
	INST_INT2CHARS,
	INST_STRI2INTS,
	INST_FLOAT2STRS,
	INST_INT2STRS,
	INST_READ,
	INST_WRITE,
	INST_CONCAT,
	INST_STRLEN,
	INST_GETCHAR,
	INST_LABEL,
	INST_JUMP,
	INST_JUMPIFEQ,
	INST_JUMPIFNEQ,
	INST_JUMPIFEQS,
	INST_JUMPIFNEQS,
	INST_EXIT,
} instruction_t;

typedef union value {
	const char *var_name;	/**< Variable identifier */
	const char *label_name; /**< Label identifier */
	const char *string_val; /**< String constant */
	int32_t int_val;		/**< Integer constant */
	double double_val;		/**< Floating-point constant */
	bool bool_val;			/**< Boolean constant */
} value_t;

/**
 * @brief Instruction operand representation
 */
typedef struct operand {
	operand_type_t operand_type; /**< Type of the operand */
	frame_type_t frame;			 /**< Memory frame */
	value_t value;
} operand_t;

/**
 * @brief Set the instruction set output_file
 *
 * @param[in] output_file output file to be set
 */
void instruction_set_init(FILE *out_file);

/**
 * @brief Creates an instruction operand structure
 *
 * @param[in] operand_type Type of the operand to create
 * @param[in] value Data union containing the operand value
 * @param[in] frame Target memory frame
 * @return operand_t Initialized operand structure
 */
operand_t create_operand(operand_type_t operand_type, value_t value, frame_type_t frame);

/**
 * @brief Prints an instruction with 0 operands
 *
 * @param[in] instruction_type Opcode of the instruction
 * @return true if successful, otherwise false
 */
bool print_inst_0(instruction_t instruction_type);

/**
 * @brief Prints an instruction with 1 operand
 *
 * @param[in] instruction_type Opcode of the instruction
 * @param[in] operand First operand
 * @return true if successful, otherwise false
 */
bool print_inst_1(instruction_t instruction_type, operand_t operand);

/**
 * @brief Prints an instruction with 2 operands
 *
 * @param[in] instruction_type Opcode of the instruction
 * @param[in] operand_1 First operand
 * @param[in] operand_2 Second operand
 * @return true if successful, otherwise false
 */
bool print_inst_2(instruction_t instruction_type, operand_t operand_1, operand_t operand_2);

/**
 * @brief Prints an instruction with 3 operands
 *
 * @param[in] instruction_type Opcode of the instruction
 * @param[in] operand_1 First operand
 * @param[in] operand_2 Second operand
 * @param[in] operand_3 Third operand
 * @return true if successful, otherwise false
 */
bool print_inst_3(instruction_t instruction_type, operand_t operand_1, operand_t operand_2,
				  operand_t operand_3);

#endif
