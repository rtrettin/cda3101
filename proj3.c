/*
Remi Trettin
rt14
CDA3101
Project 3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

typedef struct CacheStruct {
    int validBit;
    int tag;
    int dirtyBit;
    double lastUsed;
} Cache;

int main(int argc, char *argv[]) {
    int opt = 0;
    char *cmdB = NULL;
    char *cmdS = NULL;
    char *cmdN = NULL;
    
    while((opt = getopt(argc, argv, "b:s:n:")) != -1) {
        switch(opt) {
            case 'b':
                cmdB = optarg;
            break;
            case 's':
                cmdS = optarg;
            break;
            case 'n':
                cmdN = optarg;
            break;
            case '?':
                if(optopt == 'b' || optopt == 's' || optopt == 'n') {
                    printf("Missing input option.\n");
                }else{
                    printf("Invalid option.\n");
                }
            break;
        }
    }
    
    int blockSize = atoi(cmdB);
    int sets = atoi(cmdS);
    int assoc = atoi(cmdN);
    Cache wtCache[sets][assoc];
	Cache wbCache[sets][assoc];
    int i, j;
    for(i = 0; i < sets; i++) {
        for(j = 0; j < assoc; j++) {
            wtCache[i][j].validBit = 0;
            wbCache[i][j].validBit = 0;
            wtCache[i][j].tag = 0;
            wbCache[i][j].tag = 0;
            wtCache[i][j].dirtyBit = 0;
            wbCache[i][j].dirtyBit = 0;
            wtCache[i][j].lastUsed = 0;
            wbCache[i][j].lastUsed = 0;
        }
    }
    printf("Block size: %d\n", blockSize);
    printf("Number of sets: %d\n", sets);
    printf("Associativity: %d\n", assoc);
    int offsetBits = log(blockSize) / log(2);
    int indexBits = log(sets) / log(2);
    int tagBits = 32 - indexBits - offsetBits;
    printf("Number of offset bits: %d\n", offsetBits);
    printf("Number of index bits: %d\n", indexBits);
    printf("Number of tag bits: %d\n\n", tagBits);
    
    int wtReferences = 0, wtHits = 0, wtMisses = 0, wtMemReferences = 0;
    int wbReferences = 0, wbHits = 0, wbMisses = 0, wbMemReferences = 0;
    
    char c[200];
    char str[200];
    char str2[200];
    while(fgets(c, 200, stdin) != NULL) {
        struct timeval tv;
        if(sscanf(c, "%s %s", str, str2)) {
            wtReferences++;
            wbReferences++;
            int address = atoi(str2);
            address = address >> offsetBits;
            int index = address & (sets - 1);
            int tag = address >> indexBits;
			int wtfound = -1;
			int wbfound = -1;
			int wtfull = assoc;
			int wbfull = assoc;
			for(j = 0; j < assoc; j++) {
				if(!(wtCache[index][j].validBit == 0 || (wtCache[index][j].tag != tag && wtfound == -1))) {
					wtfound = j;
				}
				if(wtCache[index][j].validBit == 0) {
					wtfull = j;
				}
			}
			for(j = 0; j < assoc; j++) {
				if(!(wbCache[index][j].validBit == 0 || (wbCache[index][j].tag != tag && wbfound == -1))) {
					wbfound = j;
				}
				if(wbCache[index][j].validBit == 0) {
					wbfull = j;
				}
			}
            if(strcmp("R", str) == 0) {
                if(wtfound == -1) {
                    wtMisses++;
                    wtMemReferences++;
                    if(wtfull == assoc) {
                        int replaceIndex = 0;
                        double min = wtCache[index][0].lastUsed;
                        for(j = 1; j <= assoc - 1; j++) {
                            if(wtCache[index][j].lastUsed < min) {
                                min = wtCache[index][j].lastUsed;
                                replaceIndex = j;
                            }
                        }
                        wtCache[index][replaceIndex].validBit = 1;
                        wtCache[index][replaceIndex].tag = tag;
                        gettimeofday(&tv, NULL);
                        wtCache[index][replaceIndex].lastUsed = (double)tv.tv_usec;
                    }else{
                        wtCache[index][wtfull].validBit = 1;
                        wtCache[index][wtfull].tag = tag;
                        gettimeofday(&tv, NULL);
                        wtCache[index][wtfull].lastUsed = (double)tv.tv_usec;
                    }
                }else{
                    wtHits++;
                    gettimeofday(&tv, NULL);
                    wtCache[index][wtfound].lastUsed = (double)tv.tv_usec;
                }
				if(wbfound == -1) {
					wbMisses++;
					wbMemReferences++;
					if(wbfull == assoc) {
                        int replaceIndex = 0;
                        double min = wbCache[index][0].lastUsed;
                        for(j = 1; j <= assoc - 1; j++) {
                            if(wbCache[index][j].lastUsed < min) {
                                min = wbCache[index][j].lastUsed;
                                replaceIndex = j;
                            }
                        }
                        wbCache[index][replaceIndex].validBit = 1;
                        wbCache[index][replaceIndex].tag = tag;
                        gettimeofday(&tv, NULL);
                        wbCache[index][replaceIndex].lastUsed = (double)tv.tv_usec;
                    }else{
                        wbCache[index][wbfull].validBit = 1;
                        wbCache[index][wbfull].tag = tag;
                        gettimeofday(&tv, NULL);
                        wbCache[index][wbfull].lastUsed = (double)tv.tv_usec;
                    }
				}else{
					wbHits++;
					gettimeofday(&tv, NULL);
                    wbCache[index][wbfound].lastUsed = (double)tv.tv_usec;
				}
            }else if(strcmp("W", str) == 0) {
				if(wtfound == -1) {
					wtMisses++;
					wtMemReferences++;
				}else{
					wtHits++;
					wtMemReferences++;
					if(wtfull == assoc) {
						int replaceIndex = 0;
						double min = wtCache[index][0].lastUsed;
						for(j = 1; j <= assoc; j++) {
							if(wtCache[index][j].lastUsed < min) {
								min = wtCache[index][j].lastUsed;
								replaceIndex = j;
							}
						}
						wtCache[index][replaceIndex].validBit = 1;
						wtCache[index][replaceIndex].tag = tag;
						gettimeofday(&tv, NULL);
                        wtCache[index][replaceIndex].lastUsed = (double)tv.tv_usec;
					}else{
						wtCache[index][wtfull].validBit = 1;
						wtCache[index][wtfull].tag = tag;
						gettimeofday(&tv, NULL);
                        wtCache[index][wtfull].lastUsed = (double)tv.tv_usec;
					}
				}
				if(wbfound == -1) {
					wbMisses++;
					wbMemReferences++;
					if(wbfull == assoc) {
						int replaceIndex = 0;
						double min = wbCache[index][0].lastUsed;
						for(j = 1; j <= assoc; j++) {
							if(wbCache[index][j].lastUsed < min) {
								min = wbCache[index][j].lastUsed;
								replaceIndex = j;
							}
						}
						wbCache[index][replaceIndex].validBit = 1;
						wbCache[index][replaceIndex].tag = tag;
						gettimeofday(&tv, NULL);
                        wbCache[index][replaceIndex].lastUsed = (double)tv.tv_usec;
						wbCache[index][replaceIndex].dirtyBit = 1;
					}else{
						wbCache[index][wbfull].validBit = 1;
						wbCache[index][wbfull].tag = tag;
						gettimeofday(&tv, NULL);
                        wbCache[index][wbfull].lastUsed = (double)tv.tv_usec;
						wbCache[index][wbfull].dirtyBit = 1;
					}
				}else{
					wbHits++;
					if(wbfull == assoc) {
						int replaceIndex = 0;
						double min = wbCache[index][0].lastUsed;
						for(j = 1; j <= assoc; j++) {
							if(wbCache[index][j].lastUsed < min) {
								min = wbCache[index][j].lastUsed;
								replaceIndex = j;
							}
						}
						wbCache[index][replaceIndex].validBit = 1;
						wbCache[index][replaceIndex].tag = tag;
						gettimeofday(&tv, NULL);
                        wbCache[index][replaceIndex].lastUsed = (double)tv.tv_usec;
						wbCache[index][replaceIndex].dirtyBit = 1;
					}else{
						wbCache[index][wbfull].validBit = 1;
						wbCache[index][wbfull].tag = tag;
						gettimeofday(&tv, NULL);
                        wbCache[index][wbfull].lastUsed = (double)tv.tv_usec;
						wbCache[index][wbfull].dirtyBit = 1;
					}
				}
            }
        }
    }
    
    printf("****************************************\n");
    printf("Write-through with No Write Allocate\n");
    printf("****************************************\n");
    printf("Total number of references: %d\n", wtReferences);
    printf("Hits: %d\n", wtHits);
    printf("Misses: %d\n", wtMisses);
    printf("Memory References: %d\n\n", wtMemReferences);
    
    printf("****************************************\n");
    printf("Write-back with Write Allocate\n");
    printf("****************************************\n");
    printf("Total number of references: %d\n", wbReferences);
    printf("Hits: %d\n", wbHits);
    printf("Misses: %d\n", wbMisses);
    printf("Memory References: %d\n", wbMemReferences);

    return 0;
}