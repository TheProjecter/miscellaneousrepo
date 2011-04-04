#include <stdio.h>
#include <string.h>
#include <hash.h>
#include <stdlib.h>

#define LINE_LENGTH 256

int process_line(char* line, size_t len, char* dest, char* comp, char* jump) {
	size_t i;
	int eq = 0, col = 0, d = 0, c = 0, j = 0;

	for (i = 0; i < len; i++) {
		if (line[i] == ' ' || line[i] == '\t') continue;

		if (line[i] == '=') {
			if (eq)
				return 0;
			else
				eq = 1;
		} else if (line[i] == ';') {
			if (col)
				return 0;
			else {
				eq = 1;
				if (!c) {
					strcpy(comp, dest);
					c = d;
					d = 0;
				}
				col = 1;
			}
		} else {
			if (col) {
				jump[j++] = line[i];
			} else if (eq) {
				comp[c++] = line[i];
			} else {
				dest[d++] = line[i];
			}
		}
	}

	if (d && (!c && !j)) return 0;
	if (!j && col) return 0;
	if (!c && eq) return 0;

	jump[j] = 0;
	comp[c] = 0;
	dest[d] = 0;

	return 1;
}

void get_binary(char* str, int len, int num) {
	int i = 0;
	while (num > 0) {
		str[len - (i++) - 1] = '0' + (num%2);
		num>>=1;
	}
}

int get_out_file(char* outfile, char* infile) {
	int len = strlen(infile);
	if (len > 4 && !strcmp(".asm", infile + len - 4)) {
		strncpy(outfile, infile, len - 4);
		outfile[len-4] = 0;
		strcat(outfile, ".hack");
		return 1;
	}

	return 0;
}

int parse_dest(char* dest, char* out) {
	for (; *dest; dest++) {
		if (*dest == 'A') *out = '1';
		else if (*dest == 'D') *(out + 1) = '1';
		else if (*dest == 'M') *(out + 2) = '1';
		else return 0;
	}
	return 1;
}

int parse_comp(char* comp, char* out) {
	if (!strcmp(comp, "0")) {
		strncpy(out, "0101010", 7);
	} else if (!strcmp(comp, "1")) {
		strncpy(out, "0111111", 7);
	} else if (!strcmp(comp, "-1")) {
		strncpy(out, "0111010", 7);
	} else if (!strcmp(comp, "D")) {
		strncpy(out, "0001100", 7);
	} else if (!strcmp(comp, "!D")) {
		strncpy(out, "0001101", 7);
	} else if (!strcmp(comp, "-D")) {
		strncpy(out, "0001111", 7);
	} else if (!strcmp(comp, "D+1") || !strcmp(comp, "1+D")) {
		strncpy(out, "0011111", 7);
	} else if (!strcmp(comp, "D-1")) {
		strncpy(out, "0001110", 7);
	} else if (!strcmp(comp, "A")) {
		strncpy(out, "0110000", 7);
        } else if (!strcmp(comp, "M")) {
                strncpy(out, "1110000", 7);
        } else if (!strcmp(comp, "!A")) {
                strncpy(out, "0110001", 7);
        } else if (!strcmp(comp, "!M")) {
                strncpy(out, "1110001", 7);
        } else if (!strcmp(comp, "-A")) {
                strncpy(out, "0110011", 7);
        } else if (!strcmp(comp, "-M")) {
                strncpy(out, "1110011", 7);
        } else if (!strcmp(comp, "A+1") || !strcmp(comp, "1+A")) {
                strncpy(out, "0110111", 7);
        } else if (!strcmp(comp, "M+1") || !strcmp(comp, "1+M")) {
                strncpy(out, "1110111", 7);
        } else if (!strcmp(comp, "A-1")) {
                strncpy(out, "0110010", 7);
        } else if (!strcmp(comp, "M-1")) {
                strncpy(out, "1110010", 7);
        } else if (!strcmp(comp, "D+A") || !strcmp(comp, "A+D")) {
                strncpy(out, "0000010", 7);
        } else if (!strcmp(comp, "D+M") || !strcmp(comp, "M+D")) {
                strncpy(out, "1000010", 7);
        } else if (!strcmp(comp, "D-A")) {
                strncpy(out, "0010011", 7);
        } else if (!strcmp(comp, "D-M")) {
                strncpy(out, "1010011", 7);
        } else if (!strcmp(comp, "A-D")) {
                strncpy(out, "0000111", 7);
        } else if (!strcmp(comp, "M-D")) {
                strncpy(out, "1000111", 7);	
        } else if (!strcmp(comp, "D&A") || !strcmp(comp, "A&D")) {
                strncpy(out, "0000000", 7);
        } else if (!strcmp(comp, "D&M") || !strcmp(comp, "A&M")) {
                strncpy(out, "1000000", 7);
        } else if (!strcmp(comp, "D|A") || !strcmp(comp, "A|D")) {
                strncpy(out, "0010101", 7);
        } else if (!strcmp(comp, "D|M") || !strcmp(comp, "A|M")) {
                strncpy(out, "1010101", 7);
	} else {
		return 0;
	}

        return 1;
}

int parse_jump(char* jump, char* out) {
	if (!strcmp(jump, "JGT")) {
		*(out + 2) = '1';
	} else if (!strcmp(jump, "JEQ")) {
		*(out + 1) = '1';
        } else if (!strcmp(jump, "JLT")) {
                *out = '1';
        } else if (!strcmp(jump, "JGE")) {
		*(out + 1) = '1';
                *(out + 2) = '1';
        } else if (!strcmp(jump, "JNE")) {
		*out = '1';
                *(out + 2) = '1';
        } else if (!strcmp(jump, "JLE")) {
		*out = '1';
                *(out + 1) = '1';
        } else if (!strcmp(jump, "JMP")) {
		*out = '1';
		*(out + 1) = '1';
                *(out + 2) = '1';
	} else {
		return 0;
	}

        return 1;
}

int main(int argc, char* argv[]) {
	FILE *fd, *outfd;
	int i = 0, icount = 0, symcount = 0;
	char *line, *tmp, outfile[256], *pos, *instr, dest[LINE_LENGTH], comp[LINE_LENGTH], jump[LINE_LENGTH];
        size_t len;
	hash_table* sym;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <file.asm>\n", argv[0]);
		return -1;
	}

	fd = fopen(argv[1], "r");

	if (!get_out_file(outfile, argv[1])) {
		fprintf(stderr, "input file (%s) must have .asm extension\n", argv[1]);
		return -1;
	}

        if (!fd) {
                fprintf(stderr, "could not open file: %s\n", argv[1]);
                return -1;
        }

	outfd = fopen(outfile, "w");

	line = malloc(LINE_LENGTH);
	tmp = line;
	init_hash_table(&sym);
	add_item(sym, "SP", 0);
	add_item(sym, "LCL", 1);
	add_item(sym, "ARG", 2);
	add_item(sym, "THIS", 3);
	add_item(sym, "THAT", 4);
	add_item(sym, "R0", 0);
	add_item(sym, "R1", 1);
	add_item(sym, "R2", 2);
	add_item(sym, "R3", 3);
	add_item(sym, "R4", 4);
	add_item(sym, "R5", 5);
	add_item(sym, "R6", 6);
	add_item(sym, "R7", 7);
	add_item(sym, "R8", 8);
	add_item(sym, "R9", 9);
	add_item(sym, "R10", 10);
	add_item(sym, "R11", 11);
	add_item(sym, "R12", 12);
	add_item(sym, "R13", 13);
	add_item(sym, "R14", 14);
	add_item(sym, "R15", 15);
	add_item(sym, "SCREEN", 0x4000);
	add_item(sym, "KBD", 0x6000);

	// first pass: parse labels
	while (fgets(line, LINE_LENGTH, fd) && ++i) {
		len = strlen(line);
		if (line[len - 1] == '\n' || line[len - 1] == '\r') { len--; line[len] = 0; }
		if (line[len - 1] == '\n' || line[len - 1] == '\r') { len--; line[len] = 0; }

		pos = strstr(line, "//");
		if (pos) {
			len = pos - line;
			line[len] = 0;				
		}

		// skip whitespace
		for (; *line == ' ' || *line == '\t'; line++, len--);

		if (!len) continue;
		
		// predefined: SP(0), LCL(1), ARG(2), THIS(3), THAT(4), R0-R15(0-15),
		// SCREEN(16384), KBD(24576)
		// dest: M(001), D(010), A(100)
		// jump: JGT(001), JEQ(010), JLT(100)
		// comp

		// constant: non-negative int
		// variable: letter, digit, underscore, dot, dollar, colon
		// label: (abc)

		// A-instruction: @abc
                // C-instruction dest=comp;jump
		if (line[0] == '(') {
			instr = line + 1;
			if (len < 2 || instr[len - 2] != ')') {
				fprintf(stderr, "ERROR: incomplete label instruction at line %d: %s\n", i, line);
				return -1;
			}
			instr[len - 2] = 0;
			if (get_item(sym, instr) != -1) {
				fprintf(stderr, "ERROR: label %s was already defined\n", instr);
				return -1;
			}

			add_item(sym, instr, icount);
		} else if (len > 0) {
			icount++;
		}

		line = tmp;
	}

	rewind(fd);

	// second pass, write instructions
	i = 0;
        while (fgets(line, LINE_LENGTH, fd) && ++i) {
		char out[] = "0000000000000000";
                len = strlen(line);
                if (line[len - 1] == '\n' || line[len - 1] == '\r') { len--; line[len] = 0; }
                if (line[len - 1] == '\n' || line[len - 1] == '\r') { len--; line[len] = 0; }

                pos = strstr(line, "//");
                if (pos) {
                        len = pos - line;
                        line[len] = 0;
                }

		// skip whitespace
		for (; *line == ' ' || *line == '\t'; line++, len--);

                if (!len) continue;

		if (line[0] == '@') {
			instr = line + 1;
			printf("A-instruction: %s\n", instr);
			// parse A-instr
			// if first char non numeric, it's a variable or a label
			if (instr[0] < '0' || instr[0] > '9') {
				int id = get_item(sym, instr);
				if (id == -1) {
					id = 16 + symcount;
					add_item(sym, instr, id);
					symcount++;
				}
				get_binary(&out[1], 15, id);
			} else {
				// constant
				get_binary(&out[1], 15, atoi(instr));
			}

			fputs(out, outfd);
			fputs("\r\n", outfd);
		} else if (line[0] == '(') {
			instr = line + 1;
			instr[len - 2] = 0;	
		} else if (process_line(line, len, dest, comp, jump)) {
			printf("%s=%s;%s\n", dest, comp, jump);			
			// parse C-instr
			if (*comp && !parse_comp(comp, &out[16 - 13])) {
				fprintf(stderr, "ERROR: could not parse compute field of C-instruction: %s at line: %d\n", comp, i);
				return -1;
			}
			if (*dest && !parse_dest(dest, &out[16 - 6])) {
				fprintf(stderr, "ERROR: could not parse destination field of C-instruction: %s at line: %d\n", dest, i);
				return -1;
			}
			if (*jump && !parse_jump(jump, &out[16 - 3])) {
				fprintf(stderr, "ERROR: could not parse jump field of C-instruction: %s at line: %d\n", jump, i);
				return -1;
			}
			out[0] = '1';
			out[1] = '1';
			out[2] = '1';
			fputs(out, outfd);
			fputs("\r\n", outfd);
		} else {
			fprintf(stderr, "ERROR: unknown instruction at line %d: %s\n", i, line);
			return -1;
		}

		line = tmp;
	}

	free(tmp);
	fclose(fd);
	fclose(outfd);

	return 0;
}
