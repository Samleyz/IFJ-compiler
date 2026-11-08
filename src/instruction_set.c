/**
 * @file instruction_set.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Instruction set implementation
 * @date 2026-11-08
 */

#include "instruction_set.h"
#include <stdio.h>

typedef struct {
	const char *name;
	int arity;
} inst_info_t;

static void print_operand(operand_t operand);

static FILE *output_file;

static const inst_info_t instruction_table[] = {
	// Arity 0
	[INST_CREATEFRAME] = {"CREATEFRAME", 0},
	[INST_PUSHFRAME] = {"PUSHFRAME", 0},
	[INST_POPFRAME] = {"POPFRAME", 0},
	[INST_RETURN] = {"RETURN", 0},
	[INST_CLEARS] = {"CLEARS", 0},
	[INST_ADDS] = {"ADDS", 0},
	[INST_SUBS] = {"SUBS", 0},
	[INST_MULS] = {"MULS", 0},
	[INST_DIVS] = {"DIVS", 0},
	[INST_IDIVS] = {"IDIVS", 0},
	[INST_LTS] = {"LTS", 0},
	[INST_GTS] = {"GTS", 0},
	[INST_EQS] = {"EQS", 0},
	[INST_ANDS] = {"ANDS", 0},
	[INST_ORS] = {"ORS", 0},
	[INST_NOTS] = {"NOTS", 0},
	[INST_INT2FLOATS] = {"INT2FLOATS", 0},
	[INST_FLOAT2INTS] = {"FLOAT2INTS", 0},
	[INST_INT2CHARS] = {"INT2CHARS", 0},
	[INST_STRI2INTS] = {"STRI2INTS", 0},
	[INST_FLOAT2STRS] = {"FLOAT2STRS", 0},
	[INST_INT2STRS] = {"INT2STRS", 0},

	// Arity 1
	[INST_DEFVAR] = {"DEFVAR", 1},
	[INST_CALL] = {"CALL", 1},
	[INST_PUSHS] = {"PUSHS", 1},
	[INST_POPS] = {"POPS", 1},
	[INST_WRITE] = {"WRITE", 1},
	[INST_LABEL] = {"LABEL", 1},
	[INST_JUMP] = {"JUMP", 1},
	[INST_JUMPIFEQS] = {"JUMPIFEQS", 1},
	[INST_JUMPIFNEQS] = {"JUMPIFNEQS", 1},
	[INST_EXIT] = {"EXIT", 1},

	// Arity 2
	[INST_MOVE] = {"MOVE", 2},
	[INST_NOT] = {"NOT", 2},
	[INST_INT2FLOAT] = {"INT2FLOAT", 2},
	[INST_FLOAT2INT] = {"FLOAT2INT", 2},
	[INST_INT2CHAR] = {"INT2CHAR", 2},
	[INST_FLOAT2STR] = {"FLOAT2STR", 2},
	[INST_INT2STR] = {"INT2STR", 2},
	[INST_READ] = {"READ", 2},
	[INST_STRLEN] = {"STRLEN", 2},

	// Arity 3
	[INST_ADD] = {"ADD", 3},
	[INST_SUB] = {"SUB", 3},
	[INST_MUL] = {"MUL", 3},
	[INST_DIV] = {"DIV", 3},
	[INST_IDIV] = {"IDIV", 3},
	[INST_LT] = {"LT", 3},
	[INST_GT] = {"GT", 3},
	[INST_EQ] = {"EQ", 3},
	[INST_AND] = {"AND", 3},
	[INST_OR] = {"OR", 3},
	[INST_STRI2INT] = {"STRI2INT", 3},
	[INST_CONCAT] = {"CONCAT", 3},
	[INST_GETCHAR] = {"GETCHAR", 3},
	[INST_JUMPIFEQ] = {"JUMPIFEQ", 3},
	[INST_JUMPIFNEQ] = {"JUMPIFNEQ", 3},
};

void instruction_set_init(FILE *out_file) { output_file = out_file ? out_file : stdout; }

static void print_operand(operand_t operand) {
	switch (operand.operand_type) {

	case OP_TYPE_VAR:			 // print variable operand
		switch (operand.frame) { // prints the right frame

		case FT_GLOBAL:
			fprintf(output_file, "GF@%s", operand.value.var_name);
			break;
		case FT_LOCAL:
			fprintf(output_file, "LF@%s", operand.value.var_name);
			break;
		case FT_TEMP:
			fprintf(output_file, "TF@%s", operand.value.var_name);
			break;
		case FT_NO_FRAME:
			break;
		}
		break;

	case OP_TYPE_INT: // print int literal operand
		fprintf(output_file, "int@%d", operand.value.int_val);
		break;

	case OP_TYPE_FLOAT: // print float literal operand
		fprintf(output_file, "float@%a", operand.value.double_val);
		break;

	case OP_TYPE_STRING: // print string literal operand
		fprintf(output_file, "string@");

		for (int i = 0; operand.value.string_val[i] != '\0'; i++) {
			unsigned char character = operand.value.string_val[i];

			if (character <= ' ' || character == '#' ||
				character == '\\') { // if the character has to be escaped
				fprintf(output_file, "\\%03d", character);
			} else {
				putc(character, output_file);
			}
		}
		break;

	case OP_TYPE_BOOL: // print bool literal operand
		fprintf(output_file, "bool@%s", (int)operand.value.bool_val ? "true" : "false");
		break;

	case OP_TYPE_LABEL: // print label operand
		fprintf(output_file, "%s", operand.value.label_name);
		break;

	case OP_TYPE_NIL: // print nil operand
		fprintf(output_file, "nil@nil");
		break;
	}
}

operand_t create_operand(operand_type_t operand_type, value_t value, frame_type_t frame) {
	operand_t operand;
	operand.operand_type = operand_type;
	operand.value = value;
	operand.frame = frame;

	return operand;
}

bool print_inst_0(instruction_t instruction_type) {
	if (instruction_table[instruction_type].arity != 0) {
		return false;
	}

	fprintf(output_file, "%s", instruction_table[instruction_type].name);
	fputc('\n', output_file);

	return true;
}

bool print_inst_1(instruction_t instruction_type, operand_t operand) {
	if (instruction_table[instruction_type].arity != 1) {
		return false;
	}

	fprintf(output_file, "%s", instruction_table[instruction_type].name);
	fputc(' ', output_file);

	print_operand(operand);
	fputc('\n', output_file);

	return true;
}

bool print_inst_2(instruction_t instruction_type, operand_t operand_1, operand_t operand_2) {
	if (instruction_table[instruction_type].arity != 2) {
		return false;
	}

	fprintf(output_file, "%s", instruction_table[instruction_type].name);

	fputc(' ', output_file);
	print_operand(operand_1);
	fputc(' ', output_file);
	print_operand(operand_2);
	fputc('\n', output_file);

	return true;
}

bool print_inst_3(instruction_t instruction_type, operand_t operand_1, operand_t operand_2,
				  operand_t operand_3) {
	if (instruction_table[instruction_type].arity != 3) {
		return false;
	}

	fprintf(output_file, "%s", instruction_table[instruction_type].name);

	fputc(' ', output_file);
	print_operand(operand_1);
	fputc(' ', output_file);
	print_operand(operand_2);
	fputc(' ', output_file);
	print_operand(operand_3);
	fputc('\n', output_file);

	return true;
}
