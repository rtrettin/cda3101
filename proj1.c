/*
Remi Trettin
CDA3101
Project 1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define F_ADD 32
#define F_NOR 39
#define F_SLL 0

#define OP_ADDI 8
#define OP_ORI 13
#define OP_LUI 15
#define OP_SW 43
#define OP_LW 35
#define OP_BNE 5
#define OP_J 2

#define R_SHF_OPCODE 26
#define R_SHF_RS 21
#define R_SHF_RT 16
#define R_SHF_RD 11
#define R_SHF_SHAMT 6

#define I_SHF_OPCODE 26
#define I_SHF_RS 21
#define I_SHF_RT 16

#define J_SHF_OPCODE 26
 
struct Lines {
	char label[100];
	char instruction[100];
	int address;
};

int main() {
	char * tReg[] = { "","","","","","","","","$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7","","","","","","","","" };
	char * sReg[] = { "","","","","","","","","","","","","","","","","$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7" };
	char * zReg[] = { "$0","","","","","","","","","","","","","","","","","","","","","","","" };
	char c[200];
	char str[200];
	str[0] = '\0';
	char str2[200];
	str2[0] = '\0';
	char str3[200];
	str3[0] = '\0';
	struct Lines lines[100];
	int startAddr = 0;
	int arrCounter = 0;
	char search[2] = "la";
	char laPart1[4];
	char * laPart2;

    // Pass 1, assign each instruction and variable an address
	while(fgets(c, 200, stdin) != NULL) {
		if(sscanf(c, "%s\t%s\t%s", str, str2, str3)) {
            // .text or .data directive, throw away
			if(strchr(str, '.') && str2[0] == '\0' && str3[0] == '\0') {
				str[0] = '\0';
				str2[0] = '\0';
				str3[0] = '\0';
				continue;
            // instruction with label
			}else if(strchr(str, ':') && str2[0] != '\0' && str3[0] != '\0') {
				str[strlen(str) - 1] = '\0';
                // la instruction
				if(strstr(str2, search)) {
					laPart2 = strchr(str3, ',');
					laPart2++;
					strncpy(laPart1, str3, 3);
					laPart1[3] = '\0';
					strcpy(lines[arrCounter].label, str);
					strcpy(lines[arrCounter].instruction, "lui");
					strcat(lines[arrCounter].instruction, " ");
					strcat(lines[arrCounter].instruction, laPart1);
					strcat(lines[arrCounter].instruction, ",");
					strcat(lines[arrCounter].instruction, laPart2);
					lines[arrCounter].address = startAddr;
					startAddr = startAddr + 4;
					arrCounter = arrCounter + 1;
					strcpy(lines[arrCounter].label, str);
					strcpy(lines[arrCounter].instruction, "ori");
					strcat(lines[arrCounter].instruction, " ");
					strcat(lines[arrCounter].instruction, laPart1);
					strcat(lines[arrCounter].instruction, ",");
					strcat(lines[arrCounter].instruction, laPart1);
					strcat(lines[arrCounter].instruction, ",");
					strcat(lines[arrCounter].instruction, laPart2);
					lines[arrCounter].address = startAddr;
					startAddr = startAddr + 4;
					arrCounter = arrCounter + 1;
				}else{
					strcpy(lines[arrCounter].label, str);
					strcat(str2, " ");
					strcat(str2, str3);
					strcpy(lines[arrCounter].instruction, str2);
					lines[arrCounter].address = startAddr;
                    if(strstr(str2, "space")) {
                        int bytes = atoi(str3);
                        startAddr = startAddr + (bytes / 4);
                    }else{
                        startAddr = startAddr + 4;
                    }
					arrCounter = arrCounter + 1;
				}
				str[0] = '\0';
				str2[0] = '\0';
				str3[0] = '\0';
				laPart1[0] = '\0';
				laPart2 = 0;
            // instruction without label
			}else{
                // la instruction
				if(strstr(str, search)) {
					laPart2 = strchr(str2, ',');
                    laPart2++;
                    strncpy(laPart1, str2, 3);
                    laPart1[3] = '\0';
                    strcpy(lines[arrCounter].label, "");
                    strcpy(lines[arrCounter].instruction, "lui");
                    strcat(lines[arrCounter].instruction, " ");
                    strcat(lines[arrCounter].instruction, laPart1);
                    strcat(lines[arrCounter].instruction, ",");
                    strcat(lines[arrCounter].instruction, laPart2);
                    lines[arrCounter].address = startAddr;
                    startAddr = startAddr + 4;
                    arrCounter = arrCounter + 1;
                    strcpy(lines[arrCounter].label, "");
                    strcpy(lines[arrCounter].instruction, "ori");
                    strcat(lines[arrCounter].instruction, " ");
                    strcat(lines[arrCounter].instruction, laPart1);
                    strcat(lines[arrCounter].instruction, ",");
                    strcat(lines[arrCounter].instruction, laPart1);
                    strcat(lines[arrCounter].instruction, ",");
                    strcat(lines[arrCounter].instruction, laPart2);
                    lines[arrCounter].address = startAddr;
                    startAddr = startAddr + 4;
                    arrCounter = arrCounter + 1;
				}else{
					strcpy(lines[arrCounter].label, "");
					strcat(str, " ");
					strcat(str, str2);
					strcpy(lines[arrCounter].instruction, str);
					lines[arrCounter].address = startAddr;
					startAddr = startAddr + 4;
					arrCounter = arrCounter + 1;
				}
                str[0] = '\0';
				str2[0] = '\0';
				str3[0] = '\0';
				laPart1[0] = '\0';
				laPart2 = 0;
			}
		}
	}

    // Pass 2, calculate and output machine code for each instruction
	int i;
	for(i = 0; i < arrCounter; i++) {
		int hex = 0;
		int op, rs, rt, immed;
		int rd, shamt, funct;
		int targaddr;
		if(strstr(lines[i].instruction, "addi")) {
			op = OP_ADDI;
			char * r = strchr(lines[i].instruction, ' ');
			r++;
			int x = 0;
			char * p = strtok(r, ",");
			char * arr[3];
			while(p != NULL) {
				arr[x++] = p;
				p = strtok(NULL, ",");
			}
			for(x = 0; x < 24; x++) {
				if(strcmp(arr[0], tReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[0], sReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[0], zReg[x]) == 0) {
					rt = x;
					break;
				}
			}
			for(x = 0; x < 24; x++) {
				if(strcmp(arr[1], tReg[x]) == 0) {
					rs = x;
					break;
				}
				if(strcmp(arr[1], sReg[x]) == 0) {
					rs = x;
					break;
				}
				if(strcmp(arr[1], zReg[x]) == 0) {
					rs = x;
					break;
				}
			}
			immed = atoi(arr[2]);
			op = op << I_SHF_OPCODE;
			rt = rt << I_SHF_RT;
			rs = rs << I_SHF_RS;
			hex = op + rs + rt + immed;
			printf("0x%08x: 0x%08x\n", lines[i].address, hex);
		}else if(strstr(lines[i].instruction, "ori")) {
			op = OP_ORI;
            char * r = strchr(lines[i].instruction, ' ');
            r++;
            int x = 0;
            char * p = strtok(r, ",");
            char * arr[3];
            while(p != NULL) {
                arr[x++] = p;
                p = strtok(NULL, ",");
            }
            for(x = 0; x < 24; x++) {
				if(strcmp(arr[0], tReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[0], sReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[0], zReg[x]) == 0) {
					rt = x;
					break;
				}
			}
            for(x = 0; x < arrCounter; x++) {
                if(strcmp(lines[x].label, arr[2]) == 0) {
                    immed = lines[x].address & 0x0000ffff;
                    break;
                }
            }
            op = op << I_SHF_OPCODE;
            rt = rt << I_SHF_RT;
            rs = rt;
            hex = op + rs + rt + immed;
            printf("0x%08x: 0x%08x\n", lines[i].address, hex);
		}else if(strstr(lines[i].instruction, "lui")) {
			op = OP_LUI;
            char * r = strchr(lines[i].instruction, ' ');
            r++;
            int x = 0;
            char * p = strtok(r, ",");
            char * arr[2];
            while(p != NULL) {
                arr[x++] = p;
                p = strtok(NULL, ",");
            }
            for(x = 0; x < 24; x++) {
				if(strcmp(arr[0], tReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[0], sReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[0], zReg[x]) == 0) {
					rt = x;
					break;
				}
			}
            for(x = 0; x < arrCounter; x++) {
                if(strcmp(lines[x].label, arr[1]) == 0) {
                    immed = lines[x].address >> 16;
                    break;
                }
            }
            op = op << I_SHF_OPCODE;
            rt = rt << I_SHF_RT;
            rs = 0;
            hex = op + rs + rt + immed;
            printf("0x%08x: 0x%08x\n", lines[i].address, hex);
		}else if(strstr(lines[i].instruction, "sw")) {
			op = OP_SW;
            char * r = strchr(lines[i].instruction, ' ');
			r++;
			int x = 0;
			char * p = strtok(r, ",");
			char * arr[2];
			while(p != NULL) {
				arr[x++] = p;
				p = strtok(NULL, ",");
			}
			for(x = 0; x < 24; x++) {
				if(strcmp(arr[0], tReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[0], sReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[0], zReg[x]) == 0) {
					rt = x;
					break;
				}
			}
            int y = 0;
            p = strtok(arr[1], "(");
            char * arr2[2];
            while(p != NULL) {
                arr2[y++] = p;
                p = strtok(NULL, "(");
            }
            arr2[1][strlen(arr2[1]) - 1] = '\0';
            for(x = 0; x < 24; x++) {
				if(strcmp(arr2[1], tReg[x]) == 0) {
					rs = x;
					break;
				}
				if(strcmp(arr2[1], sReg[x]) == 0) {
					rs = x;
					break;
				}
				if(strcmp(arr2[1], zReg[x]) == 0) {
					rs = x;
					break;
				}
			}
			immed = atoi(arr2[0]);
			op = op << I_SHF_OPCODE;
			rt = rt << I_SHF_RT;
			rs = rs << I_SHF_RS;
			hex = op + rs + rt + immed;
			printf("0x%08x: 0x%08x\n", lines[i].address, hex);
		}else if(strstr(lines[i].instruction, "lw")) {
			op = OP_LW;
            char * r = strchr(lines[i].instruction, ' ');
			r++;
			int x = 0;
			char * p = strtok(r, ",");
			char * arr[2];
			while(p != NULL) {
				arr[x++] = p;
				p = strtok(NULL, ",");
			}
			for(x = 0; x < 24; x++) {
				if(strcmp(arr[0], tReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[0], sReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[0], zReg[x]) == 0) {
					rt = x;
					break;
				}
			}
            int y = 0;
            p = strtok(arr[1], "(");
            char * arr2[2];
            while(p != NULL) {
                arr2[y++] = p;
                p = strtok(NULL, "(");
            }
            arr2[1][strlen(arr2[1]) - 1] = '\0';
            for(x = 0; x < 24; x++) {
				if(strcmp(arr2[1], tReg[x]) == 0) {
					rs = x;
					break;
				}
				if(strcmp(arr2[1], sReg[x]) == 0) {
					rs = x;
					break;
				}
				if(strcmp(arr2[1], zReg[x]) == 0) {
					rs = x;
					break;
				}
			}
			immed = atoi(arr2[0]);
			op = op << I_SHF_OPCODE;
			rt = rt << I_SHF_RT;
			rs = rs << I_SHF_RS;
			hex = op + rs + rt + immed;
			printf("0x%08x: 0x%08x\n", lines[i].address, hex);
		}else if(strstr(lines[i].instruction, "bne")) {
			op = OP_BNE;
			char * r = strchr(lines[i].instruction, ' ');
			r++;
			int x = 0;
			char * p = strtok(r, ",");
			char * arr[3];
			while(p != NULL) {
				arr[x++] = p;
				p = strtok(NULL, ",");
			}
			for(x = 0; x < 24; x++) {
				if(strcmp(arr[0], tReg[x]) == 0) {
					rs = x;
					break;
				}
				if(strcmp(arr[0], sReg[x]) == 0) {
					rs = x;
					break;
				}
				if(strcmp(arr[0], zReg[x]) == 0) {
					rs = x;
					break;
				}
			}
			for(x = 0; x < 24; x++) {
				if(strcmp(arr[1], tReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[1], sReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[1], zReg[x]) == 0) {
					rt = x;
					break;
				}
			}
            for(x = 0; x < arrCounter; x++) {
                if(strcmp(lines[x].label, arr[2]) == 0) {
                    int temp = lines[x].address;
                    temp = temp >> 2;
                    immed = temp & 0x0000ffff;
                    break;
                }
            }
			op = op << I_SHF_OPCODE;
			rt = rt << I_SHF_RT;
			rs = rs << I_SHF_RS;
			hex = op + rs + rt + immed;
			printf("0x%08x: 0x%08x\n", lines[i].address, hex);
		}else if(strstr(lines[i].instruction, "add")) {
			op = 0;
            char * r = strchr(lines[i].instruction, ' ');
			r++;
			int x = 0;
			char * p = strtok(r, ",");
			char * arr[3];
			while(p != NULL) {
				arr[x++] = p;
				p = strtok(NULL, ",");
			}
			for(x = 0; x < 24; x++) {
				if(strcmp(arr[0], tReg[x]) == 0) {
					rd = x;
					break;
				}
				if(strcmp(arr[0], sReg[x]) == 0) {
					rd = x;
					break;
				}
				if(strcmp(arr[0], zReg[x]) == 0) {
					rd = x;
					break;
				}
			}
			for(x = 0; x < 24; x++) {
				if(strcmp(arr[1], tReg[x]) == 0) {
					rs = x;
					break;
				}
				if(strcmp(arr[1], sReg[x]) == 0) {
					rs = x;
					break;
				}
				if(strcmp(arr[1], zReg[x]) == 0) {
					rs = x;
					break;
				}
			}
            for(x = 0; x < 24; x++) {
				if(strcmp(arr[2], tReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[2], sReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[2], zReg[x]) == 0) {
					rs = x;
					break;
				}
			}
            shamt = 0;
            funct = F_ADD;
            op = op << R_SHF_OPCODE;
            rs = rs << R_SHF_RS;
            rt = rt << R_SHF_RT;
            rd = rd << R_SHF_RD;
            shamt = shamt << R_SHF_SHAMT;
			hex = op + rs + rt + rd + shamt + funct;
			printf("0x%08x: 0x%08x\n", lines[i].address, hex);
		}else if(strstr(lines[i].instruction, "nor")) {
			op = 0;
            char * r = strchr(lines[i].instruction, ' ');
			r++;
			int x = 0;
			char * p = strtok(r, ",");
			char * arr[3];
			while(p != NULL) {
				arr[x++] = p;
				p = strtok(NULL, ",");
			}
			for(x = 0; x < 24; x++) {
				if(strcmp(arr[0], tReg[x]) == 0) {
					rd = x;
					break;
				}
				if(strcmp(arr[0], sReg[x]) == 0) {
					rd = x;
					break;
				}
				if(strcmp(arr[0], zReg[x]) == 0) {
					rd = x;
					break;
				}
			}
			for(x = 0; x < 24; x++) {
				if(strcmp(arr[1], tReg[x]) == 0) {
					rs = x;
					break;
				}
				if(strcmp(arr[1], sReg[x]) == 0) {
					rs = x;
					break;
				}
				if(strcmp(arr[1], zReg[x]) == 0) {
					rs = x;
					break;
				}
			}
            for(x = 0; x < 24; x++) {
				if(strcmp(arr[2], tReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[2], sReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[2], zReg[x]) == 0) {
					rs = x;
					break;
				}
			}
            shamt = 0;
            funct = F_NOR;
            op = op << R_SHF_OPCODE;
            rs = rs << R_SHF_RS;
            rt = rt << R_SHF_RT;
            rd = rd << R_SHF_RD;
            shamt = shamt << R_SHF_SHAMT;
			hex = op + rs + rt + rd + shamt + funct;
			printf("0x%08x: 0x%08x\n", lines[i].address, hex);
		}else if(strstr(lines[i].instruction, "sll")) {
			op = 0;
            char * r = strchr(lines[i].instruction, ' ');
			r++;
			int x = 0;
			char * p = strtok(r, ",");
			char * arr[3];
			while(p != NULL) {
				arr[x++] = p;
				p = strtok(NULL, ",");
			}
			for(x = 0; x < 24; x++) {
				if(strcmp(arr[0], tReg[x]) == 0) {
					rd = x;
					break;
				}
				if(strcmp(arr[0], sReg[x]) == 0) {
					rd = x;
					break;
				}
				if(strcmp(arr[0], zReg[x]) == 0) {
					rd = x;
					break;
				}
			}
			for(x = 0; x < 24; x++) {
				if(strcmp(arr[1], tReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[1], sReg[x]) == 0) {
					rt = x;
					break;
				}
				if(strcmp(arr[1], zReg[x]) == 0) {
					rt = x;
					break;
				}
			}
            shamt = atoi(arr[2]);
            funct = F_SLL;
            op = op << R_SHF_OPCODE;
            rs = rs << R_SHF_RS;
            rt = rt << R_SHF_RT;
            rd = rd << R_SHF_RD;
            shamt = shamt << R_SHF_SHAMT;
			hex = op + rs + rt + rd + shamt + funct;
			printf("0x%08x: 0x%08x\n", lines[i].address, hex);
		}else if(strstr(lines[i].instruction, "j")) {
			op = OP_J;
            char * r = strchr(lines[i].instruction, ' ');
            r++;
            int x = 0;
            for(x = 0; x < arrCounter; x++) {
                if(strcmp(lines[x].label, r) == 0) {
                    int temp = lines[x].address;
                    temp = temp >> 2;
                    targaddr = temp;
                    break;
                }
            }
            op = op << J_SHF_OPCODE;
            hex = op + targaddr;
            printf("0x%08x: 0x%08x\n", lines[i].address, hex);
		}
	}

	return 0;
}
