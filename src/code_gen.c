/**
 * @file code_gen.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of the functions resposible for skeleton, function and extension generation
 * @date 2026-11-08
 */

#include "code_gen.h"
#include "instruction_set.h"
#include <stdio.h>

static int label_counter;

void code_gen_setup(FILE *output_file) {
	instruction_set_init(output_file);
	label_counter = 0;
}

bool generate_program(ast_node_t root) {
	// TODO
	(void)root;
	return false;
}
