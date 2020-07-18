#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//Struct for command
typedef struct cmd {
	char inst[9];//contains the line as String
	int opcode;
	int rd;
	int rs;
	int rt;
	int imm;
}Command;

////////////////////////////////////////////////////////////////
//Global Parameters:
#define SIZE 4096 //2^12
#define SIZE_OF_DISK 16384 //128*128
char file_arr[SIZE + 1][9]; //array of memin
char disk_out_array[SIZE_OF_DISK + 1][9]; //array of diskout
int reg_arr[16]; //array of all registers
static int memin_array_size;
static int pc = 0;
static int count_inst = 0;
static int is_irq1_run = 0;
static int count_1024 = 0;
int last_line_of_memout = SIZE;
char IOregister[18][9] = { "00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000" };
static int irq = 0;
int ready_to_irq = 1;
int irq2_interrupt_pc[SIZE] = { 0 };// array of all the pc which has irq2in interrupt
int irq2_current_index = 0;
int reti_waiting = 0; //this flag indicates us if we are in the middle of an interrupt
////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
//decleration of all the functions in the program
void read_Data_from_irq2in(FILE * irq2in);

void count_to_1024();

void diskhandle(char diskout[][9]);

char * slice_str(char str[], int start, int end);

int HexCharToInt(char h);

void Int_to_Hex8(int dec_num, char hex_num[9]);

int HexToInt2sComp(char * h);

void FillArray(FILE * memin);

void FillArrayOfdiskout(FILE * diskin);

void RegItOut(FILE * pregout);

void MemItOut(FILE * pmemout);

void DiskItOut(FILE * diskout);

void TraceIt(Command * com, FILE * ptrace);

void TimerHandle();

void irq_status_check();


void hwregtrace(FILE * phwregtrace, int rw, int reg_num);

void BuildCommand(char * command_line, Command * com);

void leds(FILE * pleds);

void display(FILE * pdisplay);

void clk_counter();

void Perform(Command * com, FILE * ptrace, FILE * pcycles, FILE * pmemout, FILE * pregout, FILE * pleds, FILE * pdiskout, FILE * pdisplay, FILE * phwregtrace, FILE * pdiskin);

void InstByLine(FILE * ptrace, FILE * pcycles, FILE * pmemout, FILE * pregout, FILE * pleds, FILE * pdiskout, FILE * pdisplay, FILE * phwregtrace, FILE * pdiskin);

//////////////////////////////////////////////////////////////

//read data from irq2in to array irq2_interrupt_pc and close the irq2in file.
void read_Data_from_irq2in(FILE * irq2in)
{
	for (int k = 0; k < SIZE; k++)
		irq2_interrupt_pc[k] = -1;
	char mline[6];
	int i = 0;
	while (!feof(irq2in))
	{
		fgets(mline, 10, irq2in); //scans the first 8 chars in a line
		irq2_interrupt_pc[i] = atoi(mline);
		i++;
	}
	fclose(irq2in);
}

// if we get disk read/write command, we start counting to 1024 before the next command.
void count_to_1024()
{
	if (is_irq1_run)
		if (count_1024 < 1024)
			count_1024++;
		else
		{
			count_1024 = 0;
			is_irq1_run = 0;
			Int_to_Hex8(0, IOregister[14]);  //diskcmd is now 0
			Int_to_Hex8(0, IOregister[17]);
			Int_to_Hex8(1, IOregister[4]);
		}
}


//this function conatins the logic of the disk handle
void diskhandle(char diskout[][9])
{
	if (!HexToInt2sComp(IOregister[17])) // The disk is free to read/write
	{
		int k = 0;
		char temp[20];
		int sector = HexToInt2sComp(IOregister[15]) * 128;
		int mem_address = HexToInt2sComp(IOregister[16]);
		char str[9];
		switch (HexToInt2sComp(IOregister[14])) {
		case 0: //no command
			break;
		case 1: //read sector
			Int_to_Hex8(1, IOregister[17]);  //Assign disk status to 1
			is_irq1_run = 1; // we get irq1 interupt
			for (k = sector; (k < sector + 128); k++)
			{
				strcpy(file_arr[mem_address], diskout[k]); // read diskin into memin
				if (mem_address + 1 == SIZE)
					mem_address = 0;
				else
					mem_address++;
			}
			break;
		case 2: //write command
			is_irq1_run = 1; // we get irq1 interupt
			Int_to_Hex8(1, IOregister[17]);  //Assign disk status to 1
			for (k = sector; k < sector + 128; k++)
			{
				strcpy(diskout[k], file_arr[mem_address]);
				if (mem_address + 1 == SIZE)
					mem_address = 0;
				else
					mem_address++;
			}
			break;
		}

	}
}

//slice string str from start location to end
char * slice_str(char str[], int start, int end)
{
	static char temp[9];
	int i = 0;
	while (start <= end)
	{
		temp[i] = str[start];
		i++;
		start++;
	}
	temp[i] = '\0';

	return temp;
}

//convert char to int
int HexCharToInt(char h) {
	short res;
	switch (h) {
	case 'A':
		res = 10;
		break;
	case 'B':
		res = 11;
		break;
	case 'C':
		res = 12;
		break;
	case 'D':
		res = 13;
		break;
	case 'E':
		res = 14;
		break;
	case 'F':
		res = 15;
		break;
	case 'a':
		res = 10;
		break;
	case 'b':
		res = 11;
		break;
	case 'c':
		res = 12;
		break;
	case 'd':
		res = 13;
		break;
	case 'e':
		res = 14;
		break;
	case 'f':
		res = 15;
		break;
	default:
		res = atoi(&h); // if char < 10 change it to int
		break;
	}
	return res;
}

//convert int to Hex8
void Int_to_Hex8(int dec_num, char hex_num[9])
{
	if (dec_num < 0) //if dec_num is negative, add 2^32 to it
		dec_num = dec_num + 4294967296; // dec_num = dec_num + 2^32
	sprintf(hex_num, "%08X", dec_num); //set hex_num to be dec_num in signed hex
}

//convert Hex to int in 2's complement
int HexToInt2sComp(char * h) {
	int i;
	int res = 0;
	int len = strlen(h);
	for (i = 0; i < len; i++)
	{
		res += HexCharToInt(h[len - 1 - i]) * (1 << (4 * i)); // change char by char from right to left, and shift it left 4 times (2^4) 
	}
	if ((len < 8) && (res & (1 << (len * 4 - 1)))) // if len is less than 8 and the msb is 1, we want to sign extend the number
	{
		res |= -1 * (1 << (len * 4)); // or of the number with: 1 from the 7th char until the msb of res, then zeros till lsb. -1 for the sign.
	}

	return res;
}

//copy memin to array char "file_arr" 
void FillArray(FILE * memin)
{
	char mline[9];
	int i = 0;
	while (!feof(memin))
	{
		fscanf(memin, "%8[^\n]\n", mline); //scans the first 8 chars in a line
		strcpy(file_arr[i], mline); //fills array[i]
		i++;
	}
	memin_array_size = i;
	while (i < SIZE)
	{
		strcpy(file_arr[i], "00000000"); // paddin with zeros all the empty memory fills array[i]
		i++;
	}
}
//copy the content of diksin into array disk_out_array
void FillArrayOfdiskout(FILE * diskin)
{
	char mline[9];
	int i = 0;
	while (!feof(diskin))
	{
		fscanf(diskin, "%8[^\n]\n", mline); //scans the first 8 chars in a line
		strcpy(disk_out_array[i], mline); //disk_out_array[i]
		i++;
	}
	while (i < SIZE_OF_DISK)
	{
		strcpy(disk_out_array[i], "00000000"); // paddin with zeros all the empty memory disk_out_array[i]
		i++;
	}
}
//write to regout.txt
void RegItOut(FILE *pregout)
{
	int i = 0;
	for (i = 2; i < 16; i++) //go over reg_arr and print it to regout
		fprintf(pregout, "%08X\n", reg_arr[i]);
}

//write to memout.txt
void MemItOut(FILE *pmemout)
{
	int i = 0;
	for (i = 0; i <= last_line_of_memout; i++) //go over file_arr and write it to memout
		fprintf(pmemout, "%s\n", file_arr[i]);
}
//write to diskout.txt
void DiskItOut(FILE *diskout)
{
	int i = 0;
	for (i = 0; i <= SIZE_OF_DISK + 1; i++) //go over file_arr and write it to memout
		fprintf(diskout, "%s\n", disk_out_array[i]);
}
//write to trace.txt
void TraceIt(Command  * com, FILE * ptrace)
{
	char pch[9];
	char inst[9];
	char r0[9];
	char r1[9];
	char r2[9];
	char r3[9];
	char r4[9];
	char r5[9];
	char r6[9];
	char r7[9];
	char r8[9];
	char r9[9];
	char r10[9];
	char r11[9];
	char r12[9];
	char r13[9];
	char r14[9];
	char r15[9];

	strcpy(inst, com->inst);

	char hex_num[9];

	Int_to_Hex8(pc, hex_num);
	strcpy(pch, hex_num);

	Int_to_Hex8(reg_arr[0], hex_num);
	strcpy(r0, hex_num);
	Int_to_Hex8(HexToInt2sComp(slice_str(inst, 5, 7)), hex_num);
	strcpy(r1, hex_num);

	Int_to_Hex8(reg_arr[2], hex_num);
	strcpy(r2, hex_num);

	Int_to_Hex8(reg_arr[3], hex_num);
	strcpy(r3, hex_num);

	Int_to_Hex8(reg_arr[4], hex_num);
	strcpy(r4, hex_num);

	Int_to_Hex8(reg_arr[5], hex_num);
	strcpy(r5, hex_num);

	Int_to_Hex8(reg_arr[6], hex_num);
	strcpy(r6, hex_num);

	Int_to_Hex8(reg_arr[7], hex_num);
	strcpy(r7, hex_num);

	Int_to_Hex8(reg_arr[8], hex_num);
	strcpy(r8, hex_num);

	Int_to_Hex8(reg_arr[9], hex_num);
	strcpy(r9, hex_num);

	Int_to_Hex8(reg_arr[10], hex_num);
	strcpy(r10, hex_num);

	Int_to_Hex8(reg_arr[11], hex_num);
	strcpy(r11, hex_num);

	Int_to_Hex8(reg_arr[12], hex_num);
	strcpy(r12, hex_num);

	Int_to_Hex8(reg_arr[13], hex_num);
	strcpy(r13, hex_num);

	Int_to_Hex8(reg_arr[14], hex_num);
	strcpy(r14, hex_num);

	Int_to_Hex8(reg_arr[15], hex_num);
	strcpy(r15, hex_num);

	fprintf(ptrace, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n", pch, inst, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15);
}

//contain the logic of the timer
void TimerHandle()
{
	if (HexToInt2sComp(IOregister[11]) == 1)
		if (HexToInt2sComp(IOregister[12]) < HexToInt2sComp(IOregister[13])) //if timercurrent is less than timermax
			Int_to_Hex8((HexToInt2sComp(IOregister[12]) + 1), IOregister[12]);

	if ((HexToInt2sComp(IOregister[12]) == HexToInt2sComp(IOregister[13])) && ((HexToInt2sComp(IOregister[13])) != 0))
	{
		Int_to_Hex8(1, IOregister[3]); //if timecurrent is equel to timermax then irq0status is 1
		Int_to_Hex8(0, IOregister[12]); //if timecurrent is equel to timermax then timecurrnt is 0
	}
}
// check if there is an interrupt
void irq_status_check()
{

	if (HexToInt2sComp(IOregister[1]) && HexToInt2sComp(IOregister[4]) && ready_to_irq) {
		is_irq1_run = 1; // we get irq1 interupt
	}
	if ((irq2_interrupt_pc[irq2_current_index] == HexToInt2sComp(IOregister[8]) - 1) && ready_to_irq) {
		Int_to_Hex8(1, IOregister[5]); // this if triggerd the irq2status to 1 when there is intruppt
		irq2_current_index++;
	}
	irq = ((HexToInt2sComp(IOregister[0]) && HexToInt2sComp(IOregister[3])) || ((HexToInt2sComp(IOregister[1])) && HexToInt2sComp(IOregister[4])) || (HexToInt2sComp(IOregister[2]) && HexToInt2sComp(IOregister[5]))) ? 1 : 0;
	Int_to_Hex8(0, IOregister[5]);

}

//this function write to hwregtrace.txt file
void hwregtrace(FILE * phwregtrace, int rw, int reg_num)
{
	char name[50] = "";
	switch (reg_num) {
	case 0:
		strcpy(name, "irq0enable");
		break;
	case 1:
		strcpy(name, "irq1enable");
		break;
	case 2:
		strcpy(name, "irq2enable");
		break;

	case 3:
		strcpy(name, "irq0status");
		break;

	case 4:
		strcpy(name, "irq1status");
		break;

	case 5:
		strcpy(name, "irq2status");
		break;

	case 6:
		strcpy(name, "irqhandler");
		break;

	case 7:
		strcpy(name, "irqreturn");
		break;

	case 8:
		strcpy(name, "clks");

		break;

	case 9:
		strcpy(name, "leds");
		break;

	case 10:
		strcpy(name, "display");
		break;

	case 11:
		strcpy(name, "timerenable");
		break;

	case 12:
		strcpy(name, "timercurrent");
		break;

	case 13:
		strcpy(name, "timermax");
		break;

	case 14:
		strcpy(name, "diskcmd");
		break;

	case 15:
		strcpy(name, "disksector");
		break;

	case 16:
		strcpy(name, "diskbuffer");
		break;

	case 17:
		strcpy(name, "diskstatus");
		break;

	}

	if (rw) //in command
	{
		fprintf(phwregtrace, "%d READ %s %s\n", HexToInt2sComp(IOregister[8]), name, IOregister[reg_num]);
	}
	else
	{
		fprintf(phwregtrace, "%d WRITE %s %s\n", HexToInt2sComp(IOregister[8]), name, IOregister[reg_num]);
	}
}
// gets a command line, and divides it to its components in com
void  BuildCommand(char * command_line, Command * com)
{
	strcpy(com->inst, command_line);

	com->opcode = (int)strtol((char[]) { command_line[0], command_line[1], 0 }, NULL, 16);
	com->rd = (int)strtol((char[]) { command_line[2], 0 }, NULL, 16);
	com->rs = (int)strtol((char[]) { command_line[3], 0 }, NULL, 16);
	com->rt = (int)strtol((char[]) { command_line[4], 0 }, NULL, 16);
	com->imm = (int)strtol((char[]) { command_line[5], command_line[6], command_line[7], 0 }, NULL, 16);
	if (com->imm >= 2048)
		com->imm -= 4096; // if the number is greater then 2048, so the sign bit is on and we need to convert the num;

}
//this function write to led.txt file
void leds(FILE * pleds)
{
	fprintf(pleds, "%d %s\n", HexToInt2sComp(IOregister[8]), IOregister[9]);
}
//this function write to display.txt file
void display(FILE * pdisplay)
{
	fprintf(pdisplay, "%d %s\n", HexToInt2sComp(IOregister[8]), IOregister[10]);

}

//increment the clock every cycle, cyclic clk
void clk_counter()
{

	(HexToInt2sComp(IOregister[8]) == HexToInt2sComp("ffffffff")) ? Int_to_Hex8(0, IOregister[8]) : Int_to_Hex8((HexToInt2sComp(IOregister[8]) + 1), IOregister[8]);
}
//according to the opcode, perform the command, write to trace, manage pc and cycles, and if 'halt' write to files and close them.
// this is the hurt of the program, which incharge to execute the command, manage the clock, the timer and the disk
void Perform(Command * com, FILE * ptrace, FILE * pcycles, FILE * pmemout, FILE * pregout, FILE * pleds, FILE * pdiskout, FILE * pdisplay, FILE * phwregtrace, FILE *pdiskin)
{

	TraceIt(com, ptrace); //write to trace before the command
	int dec_num = 0;
	char hex_num[9];
	char hex_num_temp[9] = "00000000";
	switch (com->opcode) { //cases for the opcode according to the assignment
	case 0: //add
		if (com->rd != 0 && com->rd != 1)
		{
			if (com->rs == 1)
				reg_arr[com->rs] = com->imm;
			if (com->rt == 1)
				reg_arr[com->rt] = com->imm;
			reg_arr[com->rd] = reg_arr[com->rs] + reg_arr[com->rt];
		}
		pc++;
		break;
	case 1: //sub
		if (com->rd != 0 && com->rd != 1)
		{
			if (com->rs == 1)
				reg_arr[com->rs] = com->imm;
			if (com->rt == 1)
				reg_arr[com->rt] = com->imm;
			reg_arr[com->rd] = reg_arr[com->rs] - reg_arr[com->rt];
		}
		pc++;
		break;
	case 2: //and
		if (com->rd != 0 && com->rd != 1)
		{
			if (com->rs == 1)
				reg_arr[com->rs] = com->imm;
			if (com->rt == 1)
				reg_arr[com->rt] = com->imm;
			reg_arr[com->rd] = reg_arr[com->rs] & reg_arr[com->rt];
		}
		pc++;
		break;
	case 3: //or
		if (com->rd != 0 && com->rd != 1)
		{
			if (com->rs == 1)
				reg_arr[com->rs] = com->imm;
			if (com->rt == 1)
				reg_arr[com->rt] = com->imm;
			reg_arr[com->rd] = reg_arr[com->rs] | reg_arr[com->rt];
		}
		pc++;
		break;
	case 4://sll
		if (com->rd != 0 && com->rd != 1)
		{
			if (com->rs == 1)
				reg_arr[com->rs] = com->imm;
			if (com->rt == 1)
				reg_arr[com->rt] = com->imm;
			reg_arr[com->rd] = reg_arr[com->rs] << reg_arr[com->rt];
		}
		pc++;
		break;
	case 5: //sra
		if (com->rd != 0 && com->rd != 1)
		{
			if (com->rs == 1)
				reg_arr[com->rs] = com->imm;
			if (com->rt == 1)
				reg_arr[com->rt] = com->imm;
			reg_arr[com->rd] = reg_arr[com->rs] >> reg_arr[com->rt];
		}
		pc++;
		break;
	case 6: //srl
		if (com->rd != 0 && com->rd != 1)
		{
			if (com->rs == 1)
				reg_arr[com->rs] = com->imm;
			if (com->rt == 1)
				reg_arr[com->rt] = com->imm;
			reg_arr[com->rd] = (reg_arr[com->rs] >> reg_arr[com->rt]) & 0x7fffffff;
		}
		pc++;
		break;
	case 7: //beq
		if (reg_arr[com->rs] == reg_arr[com->rt])
		{
			if (com->rd == 1)
				reg_arr[com->rd] = com->imm;
			pc = reg_arr[com->rd];
		}
		else
			pc++;

		break;
	case 8: //bne
		if (reg_arr[com->rs] != reg_arr[com->rt])
		{
			if (com->rd == 1)
				reg_arr[com->rd] = com->imm;
			pc = reg_arr[com->rd];

		}
		else
			pc++;
		break;
	case 9: //blt
		if (reg_arr[com->rs] < reg_arr[com->rt])
		{
			if (com->rd == 1)
				reg_arr[com->rd] = com->imm;
			pc = reg_arr[com->rd];
		}
		else
			pc++;
		break;
	case 10: //bjt
		if (reg_arr[com->rs] > reg_arr[com->rt])
		{
			if (com->rd == 1)
				reg_arr[com->rd] = com->imm;
			pc = reg_arr[com->rd];
		}
		else
			pc++;
		break;
	case 11: //ble
		if (reg_arr[com->rs] <= reg_arr[com->rt])
		{
			if (com->rd == 1)
				reg_arr[com->rd] = com->imm;
			pc = reg_arr[com->rd];

		}
		else
		{
			pc++;
		}
		break;
	case 12: //bge
		if (reg_arr[com->rs] >= reg_arr[com->rt])
		{
			if (com->rd == 1)
				reg_arr[com->rd] = com->imm;
			pc = reg_arr[com->rd];
		}

		else
			pc++;
		break;
	case 13: //jal
		reg_arr[15] = (pc + 1);
		if (com->rd == 1)
			reg_arr[com->rd] = com->imm;
		pc = reg_arr[com->rd];
		break;
	case 14: //lw
		if (com->rd != 0 && com->rd != 1)
		{
			if (com->rs == 1)
				reg_arr[com->rs] = com->imm;
			if (com->rt == 1)
				reg_arr[com->rt] = com->imm;
			reg_arr[com->rd] = HexToInt2sComp(file_arr[(reg_arr[com->rs]) + reg_arr[com->rt]]);
		}
		pc++;
		break;
	case 15: //sw
		//if ((com->rs + com->rt) != 0 && (com->rs + com->rt) != 1)
		//{
		if (com->rd == 1)
			reg_arr[com->rd] = com->imm;
		if (com->rt == 1)
			reg_arr[com->rt] = com->imm;
		if (com->rs == 1)
			reg_arr[com->rs] = com->imm;

		Int_to_Hex8(reg_arr[com->rd], hex_num_temp);

		strcpy(file_arr[(reg_arr[com->rs] + reg_arr[com->rt])], hex_num_temp);
		//}
		pc++;
		break;
	case 16: //reti
		pc = HexToInt2sComp(IOregister[7]);
		ready_to_irq = 1;
		break;
	case 17: //in
		if (com->rs == 1)
			reg_arr[com->rs] = com->imm;
		if (com->rt == 1)
			reg_arr[com->rt] = com->imm;
		reg_arr[com->rd] = HexToInt2sComp(IOregister[reg_arr[com->rs] + reg_arr[com->rt]]);
		pc++;
		hwregtrace(phwregtrace, 1, reg_arr[com->rs] + reg_arr[com->rt]);

		break;
	case 18: //out
		if (com->rd == 1)
			reg_arr[com->rd] = com->imm;
		if (com->rs == 1)
			reg_arr[com->rs] = com->imm;
		if (com->rt == 1)
			reg_arr[com->rt] = com->imm;
		Int_to_Hex8(reg_arr[com->rd], IOregister[reg_arr[com->rs] + reg_arr[com->rt]]);
		hwregtrace(phwregtrace, 0, reg_arr[com->rs] + reg_arr[com->rt]);
		switch (reg_arr[com->rs] + reg_arr[com->rt]) {
		case 9: //leds
			leds(pleds);
			break;
		case 10: //display
			display(pdisplay);
			break;
		case 14: // diskcmd
			diskhandle(disk_out_array);
			break;
		}
		pc++;
		break;
	case 19: //halt - write files and close them
		clk_counter(); // after the execute of the command we update the clk
		count_inst++;
		fprintf(pcycles, "%d", count_inst);
		RegItOut(pregout);
		DiskItOut(pdiskout);
		MemItOut(pmemout);
		fclose(phwregtrace);
		fclose(pleds);
		fclose(pdisplay);
		fclose(pdiskout);
		fclose(pmemout);
		fclose(pdiskin);
		fclose(pregout);
		fclose(ptrace);
		fclose(pcycles);
		exit(0);
		break;
	}

	clk_counter(); // after the execute of the command we update the clk
	count_to_1024();
	TimerHandle();// evey command we update the timer via the function timer handle
	count_inst++; //count instructions
}

//takes command lines according to the pc and pass it to perfrom.
void InstByLine(FILE * ptrace, FILE * pcycles, FILE * pmemout, FILE * pregout, FILE * pleds, FILE * pdiskout, FILE * pdisplay, FILE * phwregtrace, FILE *pdiskin)
{
	Command curr_com = { NULL, NULL, NULL, NULL, NULL, NULL };
	while (pc < memin_array_size) //perform commands until halt or until end of file
	{
		BuildCommand(file_arr[pc], &curr_com); //takes the command according to the pc
		Perform(&curr_com, ptrace, pcycles, pmemout, pregout, pleds, pdiskout, pdisplay, phwregtrace, pdiskin); //perform the command
		irq_status_check();
		if (irq && ready_to_irq)
		{
			Int_to_Hex8(pc, IOregister[7]);
			pc = HexToInt2sComp(IOregister[6]); //the proccessor is ready to jump to interrupt
			ready_to_irq = 0;
		}
	}
}


int main(int argc, char *argv[])
{
	FILE *memin = fopen(argv[1], "r"), *diskin = fopen(argv[2], "r"), *irq2in = fopen(argv[3], "r"), *memout = fopen(argv[4], "w"), *regout = fopen(argv[5], "w"),
		*trace = fopen(argv[6], "w"), *hwregtrace = fopen(argv[7], "w"), *cycles = fopen(argv[8], "w"), *leds = fopen(argv[9], "w"), *display = fopen(argv[10], "w"), *diskout = fopen(argv[11], "w");
	if (memin == NULL || irq2in == NULL || diskin == NULL || memout == NULL || regout == NULL || trace == NULL || cycles == NULL || hwregtrace == NULL || leds == NULL || display == NULL || diskout == NULL)
	{
		printf("One of the files could not be opened \n ");
		exit(1);
	}
	FillArrayOfdiskout(diskin); //copy diskin into the array that represent diskout 
	read_Data_from_irq2in(irq2in); //read data and then close irq2in
	// build file array
	FillArray(memin);
	fclose(memin);
	// simulator
	InstByLine(trace, cycles, memout, regout, leds, diskout, display, hwregtrace, diskin);
	// write and close files if no halt
	fprintf(cycles, "%d", count_inst /*HexToInt2sComp(IOregister[8])*/);
	DiskItOut(diskout);
	RegItOut(regout);
	MemItOut(memout);
	fclose(memout);
	fclose(regout);
	fclose(trace);
	fclose(hwregtrace);
	fclose(leds);
	fclose(display);
	fclose(diskin);
	fclose(diskout);
	fclose(cycles);
	exit(0);
}


