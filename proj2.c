/*
Remi Trettin
rt14
CDA3101
Project 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define NUMMEMORY 16
#define NUMREGS 8

#define R 0
#define LW 35
#define SW 43
#define BNE 4
#define HALT 63

#define ADD 32
#define SUB 34
#define NOOP 0

#define STRONGLYTAKEN 3
#define WEAKLYTAKEN 2
#define WEAKLYNOTTAKEN 1
#define STRONGLYNOTTAKEN 0

typedef struct IFIDStruct {
  unsigned int instr;
  int PCPlus4;
} IFIDType;

typedef struct IDEXStruct {
  unsigned int instr;
  int PCPlus4;
  int readData1;
  int readData2;
  int immed;
  int rsReg;
  int rtReg;
  int rdReg;
  int branchTarget;
} IDEXType;

typedef struct EXMEMStruct {
  unsigned int instr;
  int aluResult;
  int writeDataReg;
  int writeReg;
} EXMEMType;

typedef struct MEMWBStruct {
  unsigned int instr;
  int writeDataMem;
  int writeDataALU;
  int writeReg;
} MEMWBType;

typedef struct stateStruct {
  int PC;
  unsigned int instrMem[NUMMEMORY];
  int dataMem[NUMMEMORY];
  int regFile[NUMREGS];
  IFIDType IFID;
  IDEXType IDEX;
  EXMEMType EXMEM;
  MEMWBType MEMWB;
  int cycles;
  int stalls;
  int branches;
  int mispredictions;
  int bpb[NUMMEMORY];
  int ptaken;
  int b;
} stateType;


void run();
void printState(stateType*);
void initState(stateType*);
unsigned int instrToInt(char*, char*);
int get_rs(unsigned int);
int get_rt(unsigned int);
int get_rd(unsigned int);
int get_funct(unsigned int);
int get_immed(unsigned int);
int get_shamt(unsigned int);
int get_opcode(unsigned int);
void printInstruction(unsigned int);

int main(){
    run();
    return(0); 
}

void run(){

  stateType state;
  stateType newState;
  initState(&state);
  int i = 0;
  for(; i < NUMMEMORY; i++) {
    state.bpb[i] = 1;
  }
    
    while (1) {

        printState(&state);


        if (get_opcode(state.MEMWB.instr) == HALT) {
            printf("Total number of cycles executed: %d\n", state.cycles);
            printf("Total number of stalls: %d\n", state.stalls);
            printf("Total number of branches: %d\n", state.branches);
            printf("Total number of mispredicted branches: %d\n", state.mispredictions);
            exit(0);
        }

        newState = state;
        newState.cycles++;


        /* --------------------- WB stage --------------------- */
        if(get_opcode(state.MEMWB.instr) == LW) {
            newState.regFile[state.MEMWB.writeReg] = state.dataMem[state.MEMWB.writeDataALU / 4];
        }else if(get_opcode(state.MEMWB.instr) == R) {
            newState.regFile[state.MEMWB.writeReg] = state.MEMWB.writeDataALU;
        }
        
        /* --------------------- MEM stage --------------------- */
        newState.MEMWB.instr = state.EXMEM.instr;
        newState.MEMWB.writeReg = state.EXMEM.writeReg;
        newState.MEMWB.writeDataALU = state.EXMEM.aluResult;
        if(get_opcode(state.EXMEM.instr) == LW) {
            newState.MEMWB.writeDataMem = state.dataMem[state.EXMEM.aluResult / 4];
        }else if(get_opcode(state.EXMEM.instr) == SW) {
            newState.dataMem[state.EXMEM.aluResult / 4] = newState.regFile[get_rt(state.EXMEM.instr)];
        }
        
        /* --------------------- EX stage --------------------- */
		int DoIWriteToRegister = 0;
        int Forwarded = 0;
		if(get_opcode(state.IDEX.instr) == LW || get_funct(state.IDEX.instr) == ADD || get_funct(state.IDEX.instr) == SUB || get_opcode(state.IDEX.instr) == SW || get_opcode(state.IDEX.instr) == BNE) {
			DoIWriteToRegister = 1;
		}
		if(DoIWriteToRegister && state.EXMEM.writeReg != 0 && state.EXMEM.writeReg == state.IDEX.rsReg) {
			// 1A data hazard
			state.IDEX.readData1 = state.EXMEM.aluResult;
            Forwarded = 1;
		}else if(DoIWriteToRegister && state.EXMEM.writeReg != 0 && state.EXMEM.writeReg == state.IDEX.rtReg) {
			// 1B data hazard
			state.IDEX.readData2 = state.EXMEM.aluResult;
            Forwarded = 1;
		}
		if(DoIWriteToRegister && state.MEMWB.writeReg != 0 && state.MEMWB.writeReg == state.IDEX.rsReg && !(DoIWriteToRegister && state.EXMEM.writeReg != 0 && state.EXMEM.writeReg == state.IDEX.rsReg) && get_opcode(state.MEMWB.instr) != BNE) {
			// 2A data hazard
			if(get_opcode(state.MEMWB.instr) == LW) {
				state.IDEX.readData1 = state.MEMWB.writeDataMem;
                if(state.MEMWB.writeReg == state.IDEX.rtReg) {
                    state.IDEX.readData2 = state.MEMWB.writeDataMem;
                }
			}else{
				state.IDEX.readData1 = newState.MEMWB.writeDataALU;
			}
            Forwarded = 1;
		}else if(DoIWriteToRegister && state.MEMWB.writeReg != 0 && state.MEMWB.writeReg == state.IDEX.rtReg && !(DoIWriteToRegister && state.EXMEM.writeReg != 0 && state.EXMEM.writeReg == state.IDEX.rtReg) && get_opcode(state.MEMWB.instr) != BNE) {
			// 2B data hazard
			if(get_opcode(state.MEMWB.instr) == LW) {
				state.IDEX.readData2 = state.MEMWB.writeDataMem;
			}else{
				state.IDEX.readData2 = newState.MEMWB.writeDataALU;
			}
            Forwarded = 1;
		}
        newState.EXMEM.instr = state.IDEX.instr;
        if(get_opcode(state.IDEX.instr) == HALT) {
            newState.EXMEM.aluResult = 0;
            newState.EXMEM.writeReg = 0;
            newState.EXMEM.writeDataReg = 0;
        }else if(get_opcode(state.IDEX.instr) == LW) {
            newState.EXMEM.aluResult = state.IDEX.readData1 + ((int32_t)(int16_t)state.IDEX.immed);
            newState.EXMEM.writeReg = state.IDEX.rtReg;
        }else if(get_opcode(state.IDEX.instr) == SW) {
            newState.EXMEM.aluResult = state.IDEX.readData1 + ((int32_t)(int16_t)state.IDEX.immed);
            newState.EXMEM.writeDataReg = state.IDEX.readData2;
            newState.EXMEM.writeReg = state.IDEX.rtReg;
        }else if(get_opcode(state.IDEX.instr) == BNE) {
            newState.EXMEM.aluResult = state.IDEX.readData1 - state.IDEX.readData2;
            newState.EXMEM.writeDataReg = state.IDEX.readData2;
            newState.EXMEM.writeReg = state.IDEX.rtReg;
            if(newState.EXMEM.aluResult != 0) {
                // take it
                newState.b = 1;
                if(state.ptaken == 1) {
                    // correct prediction
                    newState.bpb[newState.IDEX.PCPlus4 / 4] = STRONGLYTAKEN;
                }else{
                    // wrong prediction
                    state.IDEX.instr = 0;
                    state.IFID.instr = 0;
                    state.PC = state.IDEX.branchTarget - 4;
                    newState.bpb[newState.IDEX.PCPlus4 / 4] += 1;
                    newState.mispredictions++;
                }
            }else{
                //DONT
                newState.b = 0;
                if(state.ptaken == 1) {
                    // wrong prediction
                    state.IDEX.instr = 0;
                    state.IFID.instr = 0;
                    state.PC = state.IDEX.PCPlus4 - 4;
                    newState.bpb[newState.IDEX.PCPlus4 / 4] -= 1;
                    state.IFID.PCPlus4 = state.IDEX.branchTarget;
                    newState.mispredictions++;
                }else{
                    // correct prediction
                    newState.bpb[newState.IDEX.PCPlus4 / 4] = STRONGLYNOTTAKEN;
                }
            }
        }else if(get_opcode(state.IDEX.instr) == R) {
            if(get_funct(state.IDEX.instr) == ADD) {
                newState.EXMEM.aluResult = state.IDEX.readData1 + state.IDEX.readData2;
                newState.EXMEM.writeDataReg = state.IDEX.readData2;
                newState.EXMEM.writeReg = state.IDEX.rdReg;
            }else if(get_funct(state.IDEX.instr) == SUB) {
                newState.EXMEM.aluResult = state.IDEX.readData1 - state.IDEX.readData2;
                newState.EXMEM.writeDataReg = state.IDEX.readData2;
                newState.EXMEM.writeReg = state.IDEX.rdReg;
            }else{
                newState.EXMEM.aluResult = 0;
                newState.EXMEM.writeReg = 0;
                newState.EXMEM.writeDataReg = 0;
            }
        }
        
        /* --------------------- ID stage --------------------- */
        newState.IDEX.instr = state.IFID.instr;
        newState.IDEX.PCPlus4 = state.IFID.PCPlus4;
        newState.IDEX.readData1 = newState.regFile[get_rs(state.IFID.instr)];
        newState.IDEX.readData2 = newState.regFile[get_rt(state.IFID.instr)];
        newState.IDEX.immed = get_immed(state.IFID.instr);
        newState.IDEX.rsReg = get_rs(state.IFID.instr);
        newState.IDEX.rtReg = get_rt(state.IFID.instr);
        newState.IDEX.rdReg = get_rd(state.IFID.instr);
        newState.IDEX.branchTarget = (((int32_t)(int16_t)newState.IDEX.immed) << 2) + newState.IDEX.PCPlus4;
        // stall detection
        int stalled = 0;
        if(get_opcode(state.IDEX.instr) == LW && ((state.IDEX.rtReg == get_rs(state.IFID.instr)) || (state.IDEX.rtReg == get_rt(state.IFID.instr)))) {
            newState.IDEX.instr = 0;
            newState.IDEX.PCPlus4 = state.PC;
            newState.stalls++;
            stalled = 1;
        }
        newState.ptaken = 0;
        if(get_opcode(newState.IDEX.instr) == BNE) {
            newState.branches++;
            int p = newState.bpb[newState.IDEX.PCPlus4 / 4];
            if(p == WEAKLYTAKEN || p == STRONGLYTAKEN) {
                newState.ptaken = 1;
            }else if(p == WEAKLYNOTTAKEN || p == STRONGLYNOTTAKEN) {
                newState.ptaken = 0;
            }
        }
        
        /* --------------------- IF stage --------------------- */
        if(stalled == 0 && newState.ptaken == 0) {
            if(state.IFID.PCPlus4 == state.IDEX.branchTarget && state.ptaken == 1) {
                newState.IFID.instr = 0;
            }else{
                newState.IFID.instr = state.instrMem[newState.PC / 4];
            }
            if(newState.b == 1 && get_opcode(newState.MEMWB.instr) == BNE && state.ptaken == 0) {
                newState.IFID.PCPlus4 = state.PC + 4;
                newState.b = 0;
                newState.PC = state.PC + 4;
            }else if(newState.b == 1 && state.ptaken == 1) {
                newState.IFID.PCPlus4 = state.IDEX.branchTarget + 4;
                newState.b = 0;
                newState.PC = newState.IFID.PCPlus4;
            }else{
                newState.IFID.PCPlus4 = state.IFID.PCPlus4 + 4;
                newState.PC = state.PC + 4;
            }
        }else if(newState.ptaken == 1) {
            newState.IFID.instr = 0;
            newState.PC = newState.IDEX.branchTarget;
            newState.IFID.PCPlus4 = state.IFID.PCPlus4 + 4;
        }


        state = newState;
    }
}

void initState(stateType *statePtr)
{
    unsigned int dec_inst;
    int data_index = 0;
    int inst_index = 0;
    char line[130];
    char instr[5];
    char args[130];
    char* arg; 

    statePtr->PC = 0;
    statePtr->cycles = 0;
    statePtr->stalls = 0;
    statePtr->branches = 0;
    statePtr->mispredictions = 0;
    statePtr->ptaken = 0;
    statePtr->b = 0;

    memset(statePtr->dataMem, 0, 4*NUMMEMORY);
    memset(statePtr->instrMem, 0, 4*NUMMEMORY);
    memset(statePtr->regFile, 0, 4*NUMREGS);

    while(fgets(line, 130, stdin)){
        if(sscanf(line, "\t.%s %s", instr, args) == 2){
	 
            arg = strtok(args, ",");
            while(arg != NULL){
                statePtr->dataMem[data_index] = atoi(arg);
                data_index += 1;
                arg = strtok(NULL, ","); 
            }  
        }
        else if(sscanf(line, "\t%s %s", instr, args) == 2){
            dec_inst = instrToInt(instr, args);
            statePtr->instrMem[inst_index] = dec_inst;
            inst_index += 1;
        }
    } 

    statePtr->IFID.instr = 0;
    statePtr->IFID.PCPlus4 = 0;
    statePtr->IDEX.instr = 0;
    statePtr->IDEX.PCPlus4 = 0;
    statePtr->IDEX.branchTarget = 0;
    statePtr->IDEX.readData1 = 0;
    statePtr->IDEX.readData2 = 0;
    statePtr->IDEX.immed = 0;
    statePtr->IDEX.rsReg = 0;
    statePtr->IDEX.rtReg = 0;
    statePtr->IDEX.rdReg = 0;
 
    statePtr->EXMEM.instr = 0;
    statePtr->EXMEM.aluResult = 0;
    statePtr->EXMEM.writeDataReg = 0;
    statePtr->EXMEM.writeReg = 0;

    statePtr->MEMWB.instr = 0;
    statePtr->MEMWB.writeDataMem = 0;
    statePtr->MEMWB.writeDataALU = 0;
    statePtr->MEMWB.writeReg = 0;
 }

void printState(stateType *statePtr)
{
    int i;
    printf("\n********************\nState at the beginning of cycle %d:\n", statePtr->cycles+1);
    printf("\tPC = %d\n", statePtr->PC);
    printf("\tData Memory:\n");
    for (i=0; i<(NUMMEMORY/2); i++) {
        printf("\t\tdataMem[%d] = %d\t\tdataMem[%d] = %d\n", 
            i, statePtr->dataMem[i], i+(NUMMEMORY/2), statePtr->dataMem[i+(NUMMEMORY/2)]);
    }
    printf("\tRegisters:\n");
    for (i=0; i<(NUMREGS/2); i++) {
        printf("\t\tregFile[%d] = %d\t\tregFile[%d] = %d\n", 
            i, statePtr->regFile[i], i+(NUMREGS/2), statePtr->regFile[i+(NUMREGS/2)]);
    }
    printf("\tIF/ID:\n");
    printf("\t\tInstruction: ");
    printInstruction(statePtr->IFID.instr);
    printf("\t\tPCPlus4: %d\n", statePtr->IFID.PCPlus4);
    printf("\tID/EX:\n");
    printf("\t\tInstruction: ");
    printInstruction(statePtr->IDEX.instr);
    printf("\t\tPCPlus4: %d\n", statePtr->IDEX.PCPlus4);
    printf("\t\tbranchTarget: %d\n", statePtr->IDEX.branchTarget);
    printf("\t\treadData1: %d\n", statePtr->IDEX.readData1);
    printf("\t\treadData2: %d\n", statePtr->IDEX.readData2);
    printf("\t\timmed: %d\n", statePtr->IDEX.immed);
    printf("\t\trs: %d\n", statePtr->IDEX.rsReg);
    printf("\t\trt: %d\n", statePtr->IDEX.rtReg);
    printf("\t\trd: %d\n", statePtr->IDEX.rdReg);
    printf("\tEX/MEM:\n");
    printf("\t\tInstruction: ");
    printInstruction(statePtr->EXMEM.instr);
    printf("\t\taluResult: %d\n", statePtr->EXMEM.aluResult);
    printf("\t\twriteDataReg: %d\n", statePtr->EXMEM.writeDataReg);
    printf("\t\twriteReg: %d\n", statePtr->EXMEM.writeReg);
    printf("\tMEM/WB:\n");
    printf("\t\tInstruction: ");
    printInstruction(statePtr->MEMWB.instr);
    printf("\t\twriteDataMem: %d\n", statePtr->MEMWB.writeDataMem);
    printf("\t\twriteDataALU: %d\n", statePtr->MEMWB.writeDataALU);
    printf("\t\twriteReg: %d\n", statePtr->MEMWB.writeReg);
}

unsigned int instrToInt(char* inst, char* args){

    int opcode, rs, rt, rd, shamt, funct, immed;
    unsigned int dec_inst;
    
    if((strcmp(inst, "add") == 0) || (strcmp(inst, "sub") == 0)){
        opcode = 0;
        if(strcmp(inst, "add") == 0)
            funct = ADD;
        else
            funct = SUB; 
        shamt = 0; 
        rd = atoi(strtok(args, ",$"));
        rs = atoi(strtok(NULL, ",$"));
        rt = atoi(strtok(NULL, ",$"));
        dec_inst = (opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) + (shamt << 6) + funct;
    } else if((strcmp(inst, "lw") == 0) || (strcmp(inst, "sw") == 0)){
        if(strcmp(inst, "lw") == 0)
            opcode = LW;
        else
            opcode = SW;
        rt = atoi(strtok(args, ",$"));
        immed = atoi(strtok(NULL, ",("));
        rs = atoi(strtok(NULL, "($)"));
        dec_inst = (opcode << 26) + (rs << 21) + (rt << 16) + (immed & 0x0000FFFF);
    } else if(strcmp(inst, "bne") == 0){
        opcode = 4;
        rs = atoi(strtok(args, ",$"));
        rt = atoi(strtok(NULL, ",$"));
        immed = atoi(strtok(NULL, ","));
        dec_inst = (opcode << 26) + (rs << 21) + (rt << 16) + (immed & 0x0000FFFF);   
    } else if(strcmp(inst, "halt") == 0){
        opcode = 63; 
        dec_inst = (opcode << 26);
    } else if(strcmp(inst, "noop") == 0){
        dec_inst = 0;
    }
    return dec_inst;
}

int get_rs(unsigned int instruction){
    return( (instruction>>21) & 0x1F);
}

int get_rt(unsigned int instruction){
    return( (instruction>>16) & 0x1F);
}

int get_rd(unsigned int instruction){
    return( (instruction>>11) & 0x1F);
}

int get_funct(unsigned int instruction){
    return(instruction & 0x3F);
}

int get_immed(unsigned int instruction){
  return((short)(instruction & 0xFFFF));
}

int get_opcode(unsigned int instruction){
  return(instruction>>26);
}

int get_shamt(unsigned int instruction){
  return((instruction>>6) & 0x1F);
}

void printInstruction(unsigned int instr)
{
    char opcodeString[10];
    if (instr == 0){
      printf("NOOP\n");
    } else if (get_opcode(instr) == R) {
        if(get_funct(instr)!=0){
            if(get_funct(instr) == ADD)
                strcpy(opcodeString, "add");
            else
                strcpy(opcodeString, "sub");
            printf("%s $%d,$%d,$%d\n", opcodeString, get_rd(instr), get_rs(instr), get_rt(instr));
        }
        else{
            printf("NOOP\n");
        }
    } else if (get_opcode(instr) == LW) {
        printf("%s $%d,%d($%d)\n", "lw", get_rt(instr), get_immed(instr), get_rs(instr));
    } else if (get_opcode(instr) == SW) {
        printf("%s $%d,%d($%d)\n", "sw", get_rt(instr), get_immed(instr), get_rs(instr));
    } else if (get_opcode(instr) == BNE) {
        printf("%s $%d,$%d,%d\n", "bne", get_rs(instr), get_rt(instr), get_immed(instr));
    } else if (get_opcode(instr) == HALT) {
        printf("%s\n", "halt");
    }
}
