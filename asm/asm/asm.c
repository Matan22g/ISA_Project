#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

//Constants Definitions
#define MEMORY_SIZE 4096
#define MAX_LINE_LENGTH 500 
#define MAX_LABEL_LENGTH 50


typedef char line[MAX_LINE_LENGTH + 1];
//Data structure of the labels.
typedef struct {
	char label_name[MAX_LABEL_LENGTH + 1];
	int location;
} label;

//Data structure of instruction, seperate by fields.
typedef struct {
	char *label;
	char *opcode;
	char *rd;
	char *rs;
	char *rt;
	char *imm;
} instruction_by_fields;

//Define line type. 
typedef enum {
	EMPTY,
	LABEL,
	LABEL_REGULAR, //label and regular instruction in the same one line
	WORD,
	REGULAR, //opcode, rd, rs, rt, imm
} line_type;

//Decleration of all functions in the program.
instruction_by_fields parse_line(char *line);
line_type determine_line_type(instruction_by_fields ins);
void execute(FILE *input_file, FILE *output_file);
void first_pass(FILE *input_file);
int second_pass(FILE *input_file);
int opcode_to_number(char *opcode);
int register_to_number(char *reg);
int immediate_to_number(char *imm);
int encode_instruction(int opcode, int rd, int rs, int rt, int imm);
int HexToInt2sComp(char * h);
int HexCharToInt(char h);


label labels[MEMORY_SIZE]; //global array to store the labels and their location.
int labels_amount = 0;
int memory[MEMORY_SIZE]; //global array to store the memory
int label_regular_counter = 0;

//open the input file and create the output file, then call to the Lower Hierarchy function. 
int main(int argc, char *argv[]) {
	FILE *program = fopen(argv[1], "r"), *output = fopen(argv[2], "w");
	if (program == NULL || output == NULL) {
		printf("Failed to open one of the files, the program will execute with status 1"); //Failed to open one of the files.
		exit(1);
	}
	execute(program, output); //read from the input file and write to the output file
	exit(0);
}

// parses the line into: opcode, rd, rs, rt, imm
instruction_by_fields parse_line(char *line) {
	strtok(line, "#"); // To get rid of comments
	char *line_end = line + strlen(line);
	char *current = line;
	instruction_by_fields result = { 0 };
	char *part = strtok(current, " :\t\n,");
	if (current >= line_end) {
		return result;
	}
	if ((part != NULL) && opcode_to_number(part) != -1)//if not label
	{
		result.label = NULL;
		result.opcode = part;
	}
	else //if label
	{
		result.label = part;
		current = current + strlen(current) + 1;
		if (current >= line_end)
			return result;
		result.opcode = strtok(current, ": \t\n,");
	}

	//in the following segment we check everytime if the line ended. if not we continue to parse the rest of the line

	current = current + strlen(current) + 1;
	if (current >= line_end)
		return result;

	result.rd = strtok(current, " \t\n,");
	current = current + strlen(current) + 1;
	if (current >= line_end)
		return result;

	result.rs = strtok(current, " \t\n,");
	current = current + strlen(current) + 1;
	if (current >= line_end)
		return result;

	result.rt = strtok(current, " \t\n,");
	current = current + strlen(current) + 1;
	if (current >= line_end)
		return result;

	result.imm = strtok(current, " \t\n,");
	return result;
}

// this function determines what kind of line it is.
line_type determine_line_type(instruction_by_fields instrument) {
	if ((instrument.label != NULL && instrument.label[0] == '#') || (instrument.label == NULL && instrument.opcode == NULL))
		return EMPTY;
	if (instrument.label != NULL && instrument.opcode == NULL)
		return LABEL;
	if (instrument.label != NULL && instrument.opcode != NULL && (strcmp(instrument.label, ".word") != 0))
		return LABEL_REGULAR;
	if (instrument.label != NULL && strcmp(instrument.label, ".word") == 0)
		return WORD;
	if (instrument.label == NULL && instrument.opcode != NULL)
		return REGULAR;
}

//the first pass of the array go over the line of the input file and insert label to the array 'labels' 
void first_pass(FILE *input_file) {
	int current_memory_location = 0;
	line temp_line = { 0 };
	while (fgets(temp_line, MAX_LINE_LENGTH, input_file) != NULL) {
		instruction_by_fields ins = parse_line(temp_line); //split the line into instruction by fields.
		line_type lt = determine_line_type(ins);
		if (lt == LABEL) //line type is label, insert the label to the lables array
		{
			strcpy(labels[labels_amount].label_name, ins.label);
			labels[labels_amount].location = current_memory_location - labels_amount + label_regular_counter;
			labels_amount++;
			current_memory_location++;
		}
		else if (lt == LABEL_REGULAR)
		{
			strcpy(labels[labels_amount].label_name, ins.label);
			labels[labels_amount].location = current_memory_location;
			labels_amount++;
			current_memory_location++;
			label_regular_counter++;
		}
		else if (lt == REGULAR)//if the line type is regular we only need to count the lines. 
		{
			current_memory_location++;
		}
	}
}

//go over the file for the second time
int second_pass(FILE *input_file) {
	fseek(input_file, 0, SEEK_SET);
	int current_memory_location = 0;
	int program_length = 0;
	line lne = { 0 };

	while (fgets(lne, MAX_LINE_LENGTH, input_file) != NULL) {
		instruction_by_fields ins = parse_line(lne);
		line_type lt = determine_line_type(ins);
		if (lt == WORD) {
			int address;
			int data;
			if (ins.opcode[0] == '0' && (ins.opcode[1] == 'x' || ins.opcode[1] == 'X')) //check if address is hex 
				address = HexToInt2sComp(ins.opcode);
			else
				address = atoi(ins.opcode);
			if (ins.rd[0] == '0' && (ins.rd[1] == 'x' || ins.rd[1] == 'X')) //check if data is hex
				data = HexToInt2sComp(ins.rd);
			else
				data = atoi(ins.rd);
			memory[address] = data;
			if (address >= program_length)
				program_length = address + 1;
		}
		else if ((lt == REGULAR) || (lt == LABEL_REGULAR)) {
			int opcode = opcode_to_number(ins.opcode);
			int rd = register_to_number(ins.rd);
			int rs = register_to_number(ins.rs);
			int rt = register_to_number(ins.rt);
			int imm = immediate_to_number(ins.imm);
			int encoded = encode_instruction(opcode, rd, rs, rt, imm);
			memory[current_memory_location] = encoded;
			current_memory_location++;
		}
	}
	if (current_memory_location > program_length) {
		program_length = current_memory_location;
	}
	return program_length;
}

// convert opcode to number
int opcode_to_number(char *opcode) {
	char temp[MAX_LINE_LENGTH];
	strcpy(temp, opcode);
	_strlwr(temp); //temp is now in lower case letters
	if (!strcmp(temp, "add"))
		return 0;
	else if (!strcmp(temp, "sub"))
		return 1;
	else if (!strcmp(temp, "and"))
		return 2;
	else if (!strcmp(temp, "or"))
		return 3;
	else if (!strcmp(temp, "sll"))
		return 4;
	else if (!strcmp(temp, "sra"))
		return 5;
	else if (!strcmp(temp, "srl"))
		return 6;
	else if (!strcmp(temp, "beq"))
		return 7;
	else if (!strcmp(temp, "bne"))
		return 8;
	else if (!strcmp(temp, "blt"))
		return 9;
	else if (!strcmp(temp, "bgt"))
		return 10;
	else if (!strcmp(temp, "ble"))
		return 11;
	else if (!strcmp(temp, "bge"))
		return 12;
	else if (!strcmp(temp, "jal"))
		return 13;
	else if (!strcmp(temp, "lw"))
		return 14;
	else if (!strcmp(temp, "sw"))
		return 15;
	else if (!strcmp(temp, "reti"))
		return 16;
	else if (!strcmp(temp, "in"))
		return 17;
	else if (!strcmp(temp, "out"))
		return 18;
	else if (!strcmp(temp, "halt"))
		return 19;
	return -1;
}

//get the register name and return the register number
int register_to_number(char *reg) {
	char temp[MAX_LINE_LENGTH];
	strcpy(temp, reg);
	_strlwr(temp); //temp is now lower letters case
	if (reg == NULL)
		return 0;
	else if (!strcmp(temp, "$zero"))
		return 0;
	else if (!strcmp(temp, "$imm"))
		return 1;
	else if (!strcmp(temp, "$v0"))
		return 2;
	else if (!strcmp(temp, "$a0"))
		return 3;
	else if (!strcmp(temp, "$a1"))
		return 4;
	else if (!strcmp(temp, "$t0"))
		return 5;
	else if (!strcmp(temp, "$t1"))
		return 6;
	else if (!strcmp(temp, "$t2"))
		return 7;
	else if (!strcmp(temp, "$t3"))
		return 8;
	else if (!strcmp(temp, "$s0"))
		return 9;
	else if (!strcmp(temp, "$s1"))
		return 10;
	else if (!strcmp(temp, "$s2"))
		return 11;
	else if (!strcmp(temp, "$gp"))
		return 12;
	else if (!strcmp(temp, "$sp"))
		return 13;
	else if (!strcmp(temp, "$fp"))
		return 14;
	else if (!strcmp(temp, "$ra"))
		return 15;
	return atoi(temp);
}

// convert char imm to number
int immediate_to_number(char *imm) {
	if (isalpha(imm[0])) //label
	{
		for (int i = 0; i < labels_amount; i++)
		{
			if (strcmp(imm, labels[i].label_name) == 0) //check and return the lable location.
				return labels[i].location;
		}
	}
	if (imm[0] == '0' && (imm[1] == 'x' || imm[1] == 'X')) //Hex number
	{
		return HexToInt2sComp(imm);
	}
	return atoi(imm);
}

// Encodeing instruction by shifting the instruction fields to their location in the  32 bits format
int encode_instruction(int opcode, int rd, int rs, int rt, int imm) {
	if (imm >= 0)
		return ((opcode << 24) + (rd << 20) + (rs << 16) + (rt << 12) + (imm));
	imm = 4096 + imm;
	return ((opcode << 24) + (rd << 20) + (rs << 16) + (rt << 12) + (imm));

}

//convert Hex to int in 2's complement
int HexToInt2sComp(char * h) {
	int i;
	int result = 0;
	int len = strlen(h);
	for (i = 0; i < len; i++)
	{
		result += HexCharToInt(h[len - 1 - i]) * (1 << (4 * i)); // change char by char from right to left, and shift it left 4 times
	}
	if ((len <= 4) && (result & (1 << (len * 4 - 1)))) // if len is less than 4 and the msb is 1, we want to sign extend the number
	{
		result |= -1 * (1 << (len * 4)); // or of the number with: 1 from the 3rd char until the msb of result, then zeros till lsb. -1 for the sign.
	}
	return result;
}

//convert char to int
int HexCharToInt(char h) {
	short result;
	switch (h) {
	case 'A':
		result = 10;
		break;
	case 'B':
		result = 11;
		break;
	case 'C':
		result = 12;
		break;
	case 'D':
		result = 13;
		break;
	case 'E':
		result = 14;
		break;
	case 'F':
		result = 15;
		break;
	case 'a':
		result = 10;
		break;
	case 'b':
		result = 11;
		break;
	case 'c':
		result = 12;
		break;
	case 'd':
		result = 13;
		break;
	case 'e':
		result = 14;
		break;
	case 'f':
		result = 15;
		break;
	default:
		result = atoi(&h); // if char < 10 change it to int
		break;
	}
	return result;
}


// execute the first and second pass of the input file, then print it to the output file memin  
void execute(FILE *input_file, FILE *output_file) {
	first_pass(input_file);
	int program_length = second_pass(input_file);
	for (int i = 0; i < program_length; i++) {
		fprintf(output_file, "%08X", memory[i]);//will print to outpot_file the32 bit per register
		fwrite("\n", 1, 1, output_file);// writes \n into the file
	}
}