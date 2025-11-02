/*
 * Instruction-level simulator for the LC
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMMEMORY 4096 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4 
#define JALR 5
#define HALT 6
#define NOOP 7
#define IDIV 8
#define DEC 9
#define XADD 10
#define SHL 11
#define OR 12
#define NEG 13
#define JMA 14
#define JMNE 15
#define CMP 16
#define JE 17
#define BSR 18
#define POP 19
#define PUSH 20

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
	int zf;
	int stack[32];
	int sp;
} stateType;	

void printState(stateType *);
void run(stateType);
int convertNum(int);

int
main(int argc, char *argv[])
{

    int i;
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) {
	printf("error: usage: %s <machine-code file>\n", argv[0]);
	exit(1);
    }

    /* initialize memories and registers */
    for (i=0; i<NUMMEMORY; i++) {
	state.mem[i] = 0;
    }
    for (i=0; i<NUMREGS; i++) {
	state.reg[i] = 0;
    }

    state.pc=0;
	state.zf = 0;
	state.sp = 32;
	for (i = 0; i < 32; i++) state.stack[i] = 0;
    /* read machine-code file into instruction/data memory (starting at
	address 0) */

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
	printf("error: can't open file %s\n", argv[1]);
	perror("fopen");
	exit(1);
    }

    for (state.numMemory=0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
	state.numMemory++) {
	if (state.numMemory >= NUMMEMORY) {
	    printf("exceeded memory size\n");
	    exit(1);
	}
	if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
	    printf("error in reading address %d\n", state.numMemory);
	    exit(1);
	}
	printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    printf("\n");
    
    /* run never returns */
    run(state);

    return(0);
}

void
run(stateType state)
{
    int arg0, arg1, arg2, addressField;
    int instructions=0;
    int opcode;
    int maxMem=-1;	/* highest memory address touched during run */

    for (; 1; instructions++) { /* infinite loop, exits when it executes halt */
	printState(&state);

	if (state.pc < 0 || state.pc >= NUMMEMORY) {
	    printf("pc went out of the memory range\n");
	    exit(1);
	}

	maxMem = (state.pc > maxMem)?state.pc:maxMem;

	/* this is to make the following code easier to read */
	opcode = state.mem[state.pc] >> 21;
	arg0 = (state.mem[state.pc] >> 18) & 0x7;
	arg1 = (state.mem[state.pc] >> 15) & 0x7;
	arg2 = state.mem[state.pc] & 0x7; 

	addressField = convertNum(state.mem[state.pc] & 0x7FFF); /* for beq,
								    lw, sw */
	state.pc++;	
	if (opcode == ADD) {
	    state.reg[arg2] = state.reg[arg0] + state.reg[arg1];
	} else if (opcode == NAND) {
	    state.reg[arg2] = ~(state.reg[arg0] & state.reg[arg1]);
	} else if (opcode == LW) {
	    if (state.reg[arg0] + addressField < 0 ||
		    state.reg[arg0] + addressField >= NUMMEMORY) {
		printf("address out of bounds\n");
		exit(1);
	    }
	    state.reg[arg1] = state.mem[state.reg[arg0] + addressField];
	    if (state.reg[arg0] + addressField > maxMem) {
		maxMem = state.reg[arg0] + addressField;
	    }
	} else if (opcode == SW) {
	    if (state.reg[arg0] + addressField < 0 ||
		    state.reg[arg0] + addressField >= NUMMEMORY) {
		printf("address out of bounds\n");
		exit(1);
	    }
	    state.mem[state.reg[arg0] + addressField] = state.reg[arg1];
	    if (state.reg[arg0] + addressField > maxMem) {
		maxMem = state.reg[arg0] + addressField;
	    }
	} else if (opcode == BEQ) {
	    if (state.reg[arg0] == state.reg[arg1]) {
		state.pc += addressField;
	    }
	} else if (opcode == JALR) {
	    state.reg[arg1] = state.pc;
            if(arg0 != 0)
 		state.pc = state.reg[arg0];
	    else
		state.pc = 0;
	} else if (opcode == JMA) {
		if((unsigned int)state.reg[arg0] > (unsigned int)state.reg[arg1])
			state.pc = state.pc + addressField;
	} else if (opcode == JMNE) {
		if (state.sp > 30) {
			printf("Stack underflow: not enough operands for JMNE\n");
			exit(1);
		}

		int val1 = state.stack[state.sp];
		state.sp++;
		int val2 = state.stack[state.sp];
		state.sp++;

		if (val1 != val2) {
			state.pc = state.pc + addressField;
		}
	} else if (opcode == NOOP) {
	} else if (opcode == HALT) {
	    printf("machine halted\n");
	    printf("total of %d instructions executed\n", instructions+1);
	    printf("final state of machine:\n");
	    printState(&state);
	    exit(0);
	} else if (opcode == IDIV) {
		if (state.sp >= 31) { 
			printf("Stack underflow error: not enough operands for IDIV\n");
			exit(1);
		}

		int divisor = state.stack[state.sp];
		state.sp++;

		if (state.sp >= 32) {
			printf("Stack underflow error: not enough operands for IDIV\n");
			exit(1);
		}

		int dividend = state.stack[state.sp];
		state.sp++;

		if (divisor == 0) {
			printf("Division by zero error in IDIV\n");
			exit(1);
		}

		int result = dividend / divisor;

		if (state.sp <= 0) {
			printf("Stack overflow error on IDIV result push\n");
			exit(1);
		}
		state.sp--;
		state.stack[state.sp] = result;

	} else if (opcode == DEC) {
		state.reg[arg0] = state.reg[arg0] - 1;
	} else if (opcode == XADD) {
		state.reg[arg2] = state.reg[arg0];
		state.reg[arg0] = state.reg[arg1];
		state.reg[arg1] = state.reg[arg2];
		state.reg[arg2] = state.reg[arg0] + state.reg[arg1];
	} else if (opcode == SHL) {
		state.reg[arg2] = state.reg[arg0] << state.reg[arg1];
	} else if (opcode == OR) {
		state.reg[arg2] = state.reg[arg0] | state.reg[arg1];
	} else if (opcode == NEG) {
		if (state.sp < 32) {
			state.stack[state.sp] = -state.stack[state.sp];
			state.zf = (state.stack[state.sp] == 0);
		}
		else {
			printf("Stack underflow at NEG\n");
			exit(1);
		}
	} else if (opcode == CMP) {
		if ((state.reg[arg0] - state.reg[arg1]) == 0)
			state.zf = 1;
		else
			state.zf = 0;
	} else if (opcode == JE) {
		if (state.zf == 1) {
			state.pc = state.pc + addressField;
		}
	} else if (opcode == BSR) {
		int value = state.reg[arg0];
		int position = -1;

		for (int i = 23; i >= 0; i--) {
			if ((value >> i) & 1) {
				position = i;
				break;
			}
		}
		if (position != -1) {
			state.reg[arg1] = position;
			state.zf = 1;
		}
		else {
			state.reg[arg1] = -1;
			state.zf = 0;
		}
	} else if (opcode == PUSH) {
		if (state.sp <= 0) {
			printf("Stack overflow error\n");
			exit(1);
		}
		state.sp--;
		state.stack[state.sp] = state.reg[arg0];
	} else if (opcode == POP) {
		if (state.sp >= 32) {
		 printf("Stack underflow error\n");
		 exit(1);
		}
		state.reg[arg0] = state.stack[state.sp];
		state.sp++;
	} else {
	    printf("error: illegal opcode 0x%x\n", opcode);
	    exit(1);
	}
        state.reg[0] = 0;
    }
}

void
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
	for (i=0; i<statePtr->numMemory; i++) {
	    printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
	}
    printf("\tregisters:\n");
	for (i=0; i<NUMREGS; i++) {
	    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	}
	printf("\tstack (sp=%d):\n", statePtr->sp);
	for (i = 31; i >= statePtr->sp; i--)
		printf("\t\tstack[%d] = %d\n", i, statePtr->stack[i]);
	printf("\t\tZF: %d\n", statePtr->zf);
    printf("end state\n");
}

int
convertNum(int num)
{
    /* convert a 16-bit number into a 24-bit Sun integer */
    if (num & (1<<14) ) {
	num -= (1<<15);
    }
    return(num);
}
