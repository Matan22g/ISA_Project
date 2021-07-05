# ISA_Project - Computer Structure Course
University Project - Assembler and Simulator for RISC processor
In the project we will implemnet the topics of computer language, I / O, 
Implemention of assembler and simulator (separate programs), and also programs in assembly language for
A RISC processor called SIMP, which is similar to a MIPS processor but simpler than it.
The simulator will simulate the SIMP processor, as well as a number of I / O devices: bulbs, display -7 segment, and a hard disk. 
Each instruction in the processor is performed in one clock cycle, and the processor operates at a frequency of 256 Hz - Performs 256 assembly instructions per second.


# The simulator

The simulator simulates the execute-decode-fetch loop.
Each iteration bring the next instruction in the PC address, then it decode the instruction according to the encoding,
Update the one register by performing an extension sign to the immediate field, then perform the instruction. 
At the end of the instruction, update the PC to a value of 1 + PC unless we have performed a jump instruction
Which updates the PC to a different value. 
The end of the run and exit from the simulator is performed when performing the HALT instruction.
The simulator is written in C language and compiled into an application line command which receives five
parameters line command according to the following execution line:

sim.exe memin.txt diskin.txt irq2in.txt memout.txt regout.txt trace.txt
hwregtrace.txt cycles.txt leds.txt display.txt diskout.txt

The memin.txt file is an input file in text format that contains the contents of the main memory at the beginning
The run. Each line in the file contains word content in memory, starting from zero address, in 8-digit in hexadecimal. 
If the number of lines in the file is less than 4096, it is assumed that the rest of the memory is zero.

The diskin.txt file is an input file, in the same format as memin.txt, that contains the contents of the disk
Hard at the beginning of the run.

The irq2in.txt file is an input file, which contains the numbers of clock cycle in which the outer interrupt line
irq2 rose to 1, each such clock cycle in a separate row in ascending order. The line each time rises to 1 per cycle
Single clock and then drops back to zero) unless another line appears in the file for the clock cycle
the next.

The memout.txt file is an output file, in the same format as memin.txt, that contains the contents of the memory
The main at the end of the run.

The regout.txt file is an output file, which contains the contents of registers R15-R2 at the end of the run.
The trace.txt file is an output file which contains a line of text for each instruction executed by the processor.

The hwregtrace.txt file is an output file, containing a line of text for each read or write to a hardware register 
The cycles.txt file is an output file, which contains the number of clock cycles the program wanted.
The leds.txt file contains the status of the 32 bulbs. In each clock cycle that one of the bulbs changes) turns on or
off
The display.txt file contains the display segment-7.
The diskout.txt file is an output file, in the same format as txt.memin, that contains the contents of the disk
Hard at the end of the run.

# The assembler

To make it convenient to the processor and create the memory image in the memin.txt file, we will write in the project
also the assembler program. The assembler is written in C, and translate the written assembly program to machine language 
The assembler is an command line application with the following execution line:
asm.exe program.asm mem.txt
The program.asm input file contains the assembly program, and the mem.txt output file contains the memory image
The output file of the assembler is later used as the input file of the simulator.
