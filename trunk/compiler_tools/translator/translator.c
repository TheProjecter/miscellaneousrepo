#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <hash.h>

#define LINE_LENGTH 256

#define PUSH		0
#define POP		1
#define AND		2
#define OR		3
#define NOT		4
#define EQ		5
#define NEG		6
#define LT		7
#define GT		8
#define SUB		9
#define ADD		10
#define LABEL		11
#define GOTO		12
#define IFGOTO		13
#define FUNCTION	14
#define RETURN		15
#define CALL		16

// argument count for each instruction
static int arguments[] = {
	2, // push
	2, // pop
	0, // and
	0, // or
	0, // not
	0, // eq
	0, // neg
	0, // lt
	0, // gt
	0, // sub
	0, // add
	1, // label
	1, // goto
	1, // if-goto
	2, // function
	0, // return
	2  // call
};

#define ARGUMENT	0
#define LOCAL		1
#define	STATIC		2
#define CONSTANT	3
#define	THIS		4
#define THAT		5
#define POINTER		6
#define TEMP		7

// call Sys.init (with 0 arguments) / assume it won't modify any segments, since they all point to location 0 by default; also no need to save state since it's the root function and it never returns
#define INIT		"@256\r\nD=A\r\n@SP\r\nM=D\r\n@Sys.init\r\n0;JMP\r\n"

// we assume there is always a Sys.init function in the code
// if there isn't the program wil just be stuck in a continuous loop

static int eq = 0;
static int lt = 0;
static int gt = 0;

// optimization: this avoids useless code when we have a pop after a push
// and would increase SP only to decrease it afterwards
// so when this is set to true and we are processing an instruction that pulls
// an element off the stack then we do not change the position SP is pointing at
static int inc_sp = 0;

// TODO: more possible optimizations i.e. keep track of what the location stored in A or D registers would be at any point and do not refetch it if it's already there
// ex: push stores @SP in A at the end, pop first fetches @SP into A to get M = useless

static char current_function[256];

static hash_table* sym;

static int calls = 0;

void process_pushpop(int segment, int index, char* out, int push, char* filename) {
	char buf[10];

	// back up D into R13
	if (!push && index) {
		switch(segment) {
			case ARGUMENT:
			case LOCAL:
			case STATIC:
			case THIS:
			case THAT:
			case POINTER:
			case TEMP:
				strcat(out, "@R13\r\nM=D\r\n");
			break;
		}
	}

	switch (segment) {
		case ARGUMENT:
			strcat(out, "@ARG\r\n");
		break;
		case LOCAL:
			strcat(out, "@LCL\r\n");
		break;
		case STATIC:			
			strcat(out, "@");
			strcat(out, filename);
			strcat(out, ".");
			sprintf(buf, "%d", index);
			strcat(out, buf);
			strcat(out, "\r\n");
		break;
		case CONSTANT:
			strcat(out, "@");
			sprintf(buf, "%d", index);
			strcat(out, buf);
			strcat(out, "\r\n");
		break;
		case THIS:
			strcat(out, "@THIS\r\n");
		break;
		case THAT:
			strcat(out, "@THAT\r\n");
		break;
		case POINTER:
			strcat(out, "@R3\r\n");
		break;
		case TEMP:
			strcat(out, "@R5\r\n");
		break;
	}

	switch (segment) {
		case ARGUMENT:
		case LOCAL:
		case THIS:
		case THAT:
			if (push && index) {
				strcat(out, "D=M\r\n@");				
				sprintf(buf, "%d", index);
				strcat(out, buf);
				strcat(out, "\r\nA=A+D\r\n");
			} else {
				if (index) {
					strcat(out, "D=M\r\n@");
					sprintf(buf, "%d", index);
					strcat(out, buf);
					strcat(out, "\r\nD=D+A\r\n");
					strcat(out, "@R14\r\nM=D\r\n");
					strcat(out, "@R13\r\nD=M\r\n@R14\r\nA=M\r\n");
				} else {
					strcat(out, "A=M\r\n");
				}
			}
			strcat(out, push ? "D=M" : "M=D");
		break;

		case POINTER:
		case TEMP:
			if (push && index) {
				strcat(out, "D=A\r\n@");				
				sprintf(buf, "%d", index);
				strcat(out, buf);
				strcat(out, "\r\nA=A+D\r\n");
			} else {
				if (index) {
					strcat(out, "D=A\r\n@");
					sprintf(buf, "%d", index);
					strcat(out, buf);
					strcat(out, "\r\nD=D+A\r\n");
					strcat(out, "@R14\r\nM=D\r\n");
					strcat(out, "@R13\r\nD=M\r\n@R14\r\nA=M\r\n");
				}
			}
			strcat(out, push ? "D=M" : "M=D");
		break;

		case STATIC:
			strcat(out, push ? "D=M" : "M=D");
		break;

		case CONSTANT:
			// pop constant x is illegal
			strcat(out, "D=A");
		break;
	}
}

#define E_SUCCESS			0
#define E_NOT_INSIDE_FUNCTION		1
#define E_LABEL_ALREADY_DEFINED		2
#define E_FUNCTION_ALREADY_DEFINED	3
#define E_FUNCTION_NOT_DEFINED		4

int process_instruction(int func, int segment, char* arg1, int index, char* out, char* filename) {
	char buf[256];
	int  locals;

	if (func != FUNCTION && !current_function[0]) {
		return E_NOT_INSIDE_FUNCTION;
	}

	switch (func) {
		case PUSH:
			if (inc_sp)
				strcpy(out, "@SP\r\nM=M+1\r\n");
			else
				strcpy(out, "");

			process_pushpop(segment, index, out, 1, filename);
			strcat(out, "\r\n@SP\r\nA=M\r\nM=D");
			inc_sp = 1;
		break;

		case POP:
			if (inc_sp)
				strcpy(out, "@SP\r\nA=M\r\nD=M\r\n");
			else
				strcpy(out, "@SP\r\nM=M-1\r\nA=M\r\nD=M\r\n");
			inc_sp = 0;

			process_pushpop(segment, index, out, 0, filename);
		break;

		case AND:
			if (inc_sp)
				strcpy(out, "@SP\r\n");
			else
				strcpy(out, "@SP\r\nM=M-1\r\n");
			inc_sp = 0;
							
			strcat(out, "A=M\r\nD=M\r\n@SP\r\nA=M-1\r\nM=M&D");
		break;

		case OR:
			if (inc_sp)
				strcpy(out, "@SP\r\n");
			else
				strcpy(out, "@SP\r\nM=M-1\r\n");
			inc_sp = 0;

			strcat(out, "A=M\r\nD=M\r\n@SP\r\nA=M-1\r\nM=M|D");
		break;

		case NOT:
			if (inc_sp)
				strcpy(out, "@SP\r\nM=M+1\r\nA=M-1\r\n");
			else
				strcpy(out, "@SP\r\nA=M-1\r\n");
			inc_sp = 0;

			strcat(out, "M=!M");
		break;


		case NEG:
			if (inc_sp)
				strcpy(out, "@SP\r\nM=M+1\r\nA=M-1\r\n");
			else
				strcpy(out, "@SP\r\nA=M-1\r\n");
			inc_sp = 0;

			strcat(out, "M=-M");
		break;

		case LT:
			if (inc_sp)
				strcpy(out, "@SP\r\n");
			else
				strcpy(out, "@SP\r\nM=M-1\r\n");
			inc_sp = 0;

			strcat(out, "A=M\r\nD=M\r\n@SP\r\nM=M-1\r\nA=M\r\nD=M-D\r\n@JLT_");
			sprintf(buf, "%d", lt++);
			strcat(out, buf);
			strcat(out, "\r\nD;JLT\r\n@SP\r\nA=M\r\nM=0\r\n@JLT_");
			strcat(out, buf);
			strcat(out, "_END\r\n0;JMP\r\n(JLT_");
			strcat(out, buf);
			strcat(out, ")\r\n@SP\r\nA=M\r\nM=-1\r\n(JLT_");
			strcat(out, buf);
			strcat(out, "_END)");
			inc_sp = 1;
		break;

		case EQ:
			if (inc_sp)
				strcpy(out, "@SP\r\n");
			else
				strcpy(out, "@SP\r\nM=M-1\r\n");
			inc_sp = 0;

			strcat(out, "A=M\r\nD=M\r\n@SP\r\nM=M-1\r\nA=M\r\nD=M-D\r\n@JEQ_");
			sprintf(buf, "%d", eq++);
			strcat(out, buf);
			strcat(out, "\r\nD;JEQ\r\n@SP\r\nA=M\r\nM=0\r\n@JEQ_");
			strcat(out, buf);
			strcat(out, "_END\r\n0;JMP\r\n(JEQ_");
			strcat(out, buf);
			strcat(out, ")\r\n@SP\r\nA=M\r\nM=-1\r\n(JEQ_");
			strcat(out, buf);
			strcat(out, "_END)");
			inc_sp = 1;
		break;

		case GT:
			if (inc_sp)
				strcpy(out, "@SP\r\n");
			else
				strcpy(out, "@SP\r\nM=M-1\r\n");
			inc_sp = 0;

			strcat(out, "A=M\r\nD=M\r\n@SP\r\nM=M-1\r\nA=M\r\nD=M-D\r\n@JGT_");
			sprintf(buf, "%d", gt++);
			strcat(out, buf);
			strcat(out, "\r\nD;JGT\r\n@SP\r\nA=M\r\nM=0\r\n@JGT_");
			strcat(out, buf);
			strcat(out, "_END\r\n0;JMP\r\n(JGT_");
			strcat(out, buf);
			strcat(out, ")\r\n@SP\r\nA=M\r\nM=-1\r\n(JGT_");
			strcat(out, buf);
			strcat(out, "_END)");
			inc_sp = 1;
		break;

		case SUB:
			if (inc_sp)
				strcpy(out, "@SP\r\n");
			else
				strcpy(out, "@SP\r\nM=M-1\r\n");
			inc_sp = 0;

			strcat(out, "A=M\r\nD=M\r\n@SP\r\nA=M-1\r\nM=M-D");
		break;

		case ADD:
			if (inc_sp)
				strcpy(out, "@SP\r\n");
			else
				strcpy(out, "@SP\r\nM=M-1\r\n");
			inc_sp = 0;

			strcat(out, "A=M\r\nD=M\r\n@SP\r\nA=M-1\r\nM=M+D");
		break;

		case LABEL:
			if (inc_sp)
				strcpy(out, "@SP\r\nM=M+1\r\n");
			else
				strcpy(out, "");
			inc_sp = 0;

			strcat(out, "(");
			strcat(out, current_function);
			strcat(out, "$");
			strcat(out, arg1);
			strcat(out, ")");
		break;

		case GOTO:
			if (inc_sp)
				strcpy(out, "@SP\r\nM=M+1\r\n");
			else
				strcpy(out, "");
			inc_sp = 0;

			strcat(out, "@");
			strcat(out, current_function);
			strcat(out, "$");
			strcat(out, arg1);
			strcat(out, "\r\n0;JMP");
		break;

		case IFGOTO:
			if (inc_sp)
				strcpy(out, "@SP\r\nA=M\r\nD=M\r\n");
			else
				strcpy(out, "@SP\r\nM=M-1\r\nA=M\r\nD=M\r\n");
			inc_sp = 0;

			strcat(out, "@");
			strcat(out, current_function);
			strcat(out, "$");
			strcat(out, arg1);
			strcat(out, "\r\nD;JNE");
		break;

		case FUNCTION:
			// enter function
			strcpy(current_function, arg1);

			if (inc_sp)
				strcpy(out, "@SP\r\nM=M+1\r\n");
			else
				strcpy(out, "");
			inc_sp = 0;

			// insert label
			strcat(out, "(");
			strcat(out, arg1);
			strcat(out, ")");
		break;

		case RETURN:
			locals = get_item(sym, current_function);

			if (inc_sp)
				strcpy(out, "@SP\r\n");
			else
				strcpy(out, "@SP\r\nM=M-1\r\n");

			// decrease SP past number of local variables
			// and return value (we assume it's always there, the first value on the top of the stack)

			// first get return value and store it into R13
			strcat(out, "A=M\r\nD=M\r\n@R13\r\nM=D\r\n");
			
			// now decrease to before local variables
			if (locals > 0) {
				strcat(out, "@");
				sprintf(buf, "%d", locals);
				strcat(out, buf);
				strcat(out, "\r\nD=A\r\n@SP\r\nM=M-D\r\n");
			}

			// store ARG value into R14
			strcat(out, "@ARG\r\nD=M\r\n@R14\r\nM=D\r\n");

			// pop THAT
			strcat(out, "@SP\r\nM=M-1\r\nA=M\r\nD=M\r\n@THAT\r\nM=D\r\n");
			// pop THIS
			strcat(out, "@SP\r\nM=M-1\r\nA=M\r\nD=M\r\n@THIS\r\nM=D\r\n");
			// pop ARG
			strcat(out, "@SP\r\nM=M-1\r\nA=M\r\nD=M\r\n@ARG\r\nM=D\r\n");
			// pop LCL
			strcat(out, "@SP\r\nM=M-1\r\nA=M\r\nD=M\r\n@LCL\r\nM=D\r\n");
			// pop return address into R15
			strcat(out, "@SP\r\nM=M-1\r\nA=M\r\nD=M\r\n@R15\r\nM=D\r\n");

			// set SP to ARG old value (R14) and put return value (R13) into it, then increase SP
			strcat(out, "@R14\r\nD=M\r\n@SP\r\nM=D\r\n@R13\r\nD=M\r\n@SP\r\nM=M+1\r\nA=M-1\r\nM=D\r\n");

			// go to return address (R15)
			strcat(out, "@R15\r\nA=M\r\n0;JMP");
			inc_sp = 0; // SP was increased (prefer to do it here rather than in each call)
		break;

		case CALL:
			locals = get_item(sym, arg1);
			if (locals == -1) {
				return E_FUNCTION_NOT_DEFINED;
			}

			if (inc_sp)
				strcpy(out, "@SP\r\nM=M+1\r\n");
			else
				strcpy(out, "");
			inc_sp = 0;

			// backup initial SP value into R13

			// decrease by number of arguments that were pushed onto SP
			strcat(out, "@SP\r\nD=M\r\n@R13\r\nM=D\r\n");
			sprintf(buf, "%d", index);
			strcat(out, "@");
			strcat(out, buf);
			strcat(out, "\r\nD=A\r\n@R13\r\nM=M-D\r\n");

			// push return address
			sprintf(buf, "%d", calls);
			strcat(out, "@RETURN_ADDRESS_");
			strcat(out, buf);
			strcat(out, "\r\nD=A\r\n@SP\r\nA=M\r\nM=D\r\n@SP\r\nM=M+1\r\n");
			// push LCL
			strcat(out, "@LCL\r\nD=M\r\n@SP\r\nA=M\r\nM=D\r\n@SP\r\nM=M+1\r\n");
			// push ARG
			strcat(out, "@ARG\r\nD=M\r\n@SP\r\nA=M\r\nM=D\r\n@SP\r\nM=M+1\r\n");
			// push THIS
			strcat(out, "@THIS\r\nD=M\r\n@SP\r\nA=M\r\nM=D\r\n@SP\r\nM=M+1\r\n");
			// push THAT
			strcat(out, "@THAT\r\nD=M\r\n@SP\r\nA=M\r\nM=D\r\n@SP\r\nMD=M+1\r\n");

			// set LCL to new value (SP)
			strcat(out, "@LCL\r\nM=D\r\n");

			// increase SP past number of local variables
			if (locals > 0) {
				strcat(out, "@");
				sprintf(buf, "%d", locals);
				strcat(out, buf);
				strcat(out, "\r\nD=A\r\n@SP\r\nM=M+D\r\n");
			}

			// set ARG to new value
			strcat(out, "@R13\r\nD=M\r\n@ARG\r\nM=D\r\n");			

			// jump to function
			strcat(out, "@");
			strcat(out, arg1);
			strcat(out, "\r\n0;JMP\r\n");			

			// insert return address label
			sprintf(buf, "%d", calls);
			strcat(out, "(RETURN_ADDRESS_");
			strcat(out, buf);
			strcat(out, ")");
			calls++;
		break;				
	}

	return E_SUCCESS;
}

int process_number(char* token, int* n) {
	for (; *token; token++) {
		int val = *token-'0';
		if (val < 0 || val > 9) return 0;
		*n=(*n)*10 + val;
	}
	return 1;
}

int get_out_file(char* outfile, char* infile, char* base) {
	int len = strlen(infile);
	char *tmp;
	if (len > 3 && !strcmp(".vm", infile + len - 3)) {
		strncpy(outfile, infile, len - 3);
		outfile[len-3] = 0;
		strcat(outfile, ".asm");
	} else {
		return 0;
	}

	for (tmp = infile + len - 1; tmp >= infile; tmp--) {
		if (*tmp == '/') break;
	}
	tmp++;

	strncpy(base, tmp, len - 3 - (tmp - infile));
	base[len - 3 - (tmp - infile)] = 0;

	return 1;
}

int process_file(char* path, FILE* ofd) {
	char outfile[256], line[LINE_LENGTH], base[256];
	int i = 0;
	FILE *fd, *outfd;

	if (!get_out_file(outfile, path, base)) {
		if (ofd) return 1;
		fprintf(stderr, "argument (%s) must be a file with a .vm extension or a directory\n", path);
		return 0;
	}

	fd = fopen(path, "r");
	printf("processing file %s\n", path);

        if (!fd) {
                fprintf(stderr, "could not open file: %s\n", path);
                return 0;
        }

	// process and write to output
	if (ofd)
		outfd = ofd;
	else {
		outfd = fopen(outfile, "w");
		fputs(INIT, outfd); // bootstrap code
	}

	// we start not inside any function, user must declare one
	current_function[0] = 0;
	while (fgets(line, LINE_LENGTH, fd) && ++i) {
		char *token, *pos, args = 0, out[8192], arg1char[LINE_LENGTH];
                int len = strlen(line), n = 0, arg1, arg2, err, func = 0;

                pos = strstr(line, "//");
                if (pos) {
                        len = pos - line;
                        line[len] = 0;
                }

                if (!len) continue;

		token = strtok(line, " \t\r\n");
		if (token) {
			// 2-arg functions: push segment index, pop segment index, function name local_variable_count, call function argument_count
			// 1-arg functions: label name, goto label, if-goto label
			// 0-arg functions (add result on stack): add, sub, neg, or, and, not, lt, gt, eq, return
			printf("token = %s\n", token);
			if (!strcmp(token, "push")) {
				func = PUSH;
			} else if (!strcmp(token, "pop")) {
				func = POP;
			} else if (!strcmp(token, "add")) {
				func = ADD;
			} else if (!strcmp(token, "sub")) {
				func = SUB;
			} else if (!strcmp(token, "neg")) {
				func = NEG;
			} else if (!strcmp(token, "or")) {
				func = OR;
			} else if (!strcmp(token, "and")) {
				func = AND;
			} else if (!strcmp(token, "not")) {
				func = NOT;
			} else if (!strcmp(token, "lt")) {
				func = LT;
			} else if (!strcmp(token, "gt")) {
				func = GT;
			} else if (!strcmp(token, "eq")) {
				func = EQ;
			} else if (!strcmp(token, "label")) {
				func = LABEL;
			} else if (!strcmp(token, "goto")) {
				func = GOTO;
			} else if (!strcmp(token, "if-goto")) {
				func = IFGOTO;
			} else if (!strcmp(token, "function")) {
				func = FUNCTION;
			} else if (!strcmp(token, "return")) {
				func = RETURN;
			} else if (!strcmp(token, "call")) {
				func = CALL;
			} else {
				fprintf(stderr, "ERROR: unknown instruction in file %s, line %d: %s\n", path, i, token);
				return 0;
			}

			while ((token = strtok(NULL, " \t\r\n"))) {
				printf("token = %s\n", token);
				args++;

				if (arguments[func] < args) {
					fprintf(stderr, "ERROR: in file %s, line %d: wrong argument count for instruction, should be %d\n", path, i, arguments[func]);
					return 0;			
				}

				if (args == 1) {
					if (func == PUSH || func == POP) {
						if (!strcmp(token, "argument")) {
							arg1 = ARGUMENT;
						} else if (!strcmp(token, "local")) {
							arg1 = LOCAL;
						} else if (!strcmp(token, "static")) {
							arg1 = STATIC;		
						} else if (!strcmp(token, "constant")) {
							arg1 = CONSTANT;	
						} else if (!strcmp(token, "this")) {
							arg1 = THIS;	
						} else if (!strcmp(token, "that")) {
							arg1 = THAT;	
						} else if (!strcmp(token, "pointer")) {
							arg1 = POINTER;	
						} else if (!strcmp(token, "temp")) {
							arg1 = TEMP;	
						} else {
							fprintf(stderr, "ERROR: unknown segment in file %s, line %d: %s\n", path, i, token);
							return 0;
						}	
					} else {
						strcpy(arg1char, token);
					}		
				} else if (args == 2) {				
					if (!process_number(token, &n)) {
						fprintf(stderr, "ERROR: invalid number in file %s, line %d: %s\n", path, i, token);
						return 0;
					}

					arg2 = n;
				}
			}

			if (arguments[func] != args) {
				fprintf(stderr, "ERROR: in file %s, line %d: wrong argument count (%d) for instruction, should be %d\n", path, i, args, arguments[func]);
				return 0;			
			}

			if ((err = process_instruction(func, arg1, arg1char, arg2, out, base)) != E_SUCCESS) {
				switch (err) {
					case E_NOT_INSIDE_FUNCTION:
						fprintf(stderr, "ERROR: in file %s, line %d: all instructions must be inside a function\n", path, i);						
					break;

					case E_FUNCTION_NOT_DEFINED:
						fprintf(stderr, "ERROR: in file %s, line %d: function %s is not defined\n", path, i, arg1char);						
					break;
				}

				return 0;
			}

			fputs(out, outfd);
			fputs("\r\n", outfd);
		}
	}

	if (inc_sp) {
		fputs("@SP\r\nM=M+1\r\n", outfd);
		inc_sp = 0;
	}

	if (!ofd)
		fclose(outfd);

	fclose(fd);
	return 1;
}

int process_directory(char* path) {
	DIR* d = opendir(path);
	FILE* outfd;
	char outfile[256];
	int len = strlen(path);
	struct dirent* de;
	if (!d) return 0; // will signal that we want to try to process it as a file next time

	strcpy(outfile, path);
	if (outfile[len - 1] == '/') outfile[len - 1] = 0; // trim trailing slash

	strcat(outfile, ".asm");
	outfd = fopen(outfile, "w");
	fputs(INIT, outfd); // bootstrap code

	while ((de = readdir(d)) != NULL) {
		char fpath[256];

		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
			continue;

		strcpy(fpath, path);
		strcat(fpath, "/");
		strcat(fpath, de->d_name);

		if (!process_file(fpath, outfd)) {
			return -1; // return -1 to signal that there is no point in trying to parse it as file
		}
	}

	fclose(outfd);
	closedir(d);
	return 1;
}

int process_lf(int func, char* arg1, int arg2) {
	char buf[256];

	switch (func) {
		case FUNCTION:
			// enter function
			strcpy(current_function, arg1);

			// add number of local variables to symbols hash table
			if (get_item(sym, arg1) != -1) {
				return E_FUNCTION_ALREADY_DEFINED;
			}
			add_item(sym, arg1, arg2);
		break;

		case LABEL:
			if (!current_function[0]) {
				return E_NOT_INSIDE_FUNCTION;
			}

			// add label to symbols hash table
			sprintf(buf, "%s$%s", current_function, arg1);
			if (get_item(sym, buf) != -1) {
				return E_LABEL_ALREADY_DEFINED;
			}
			add_item(sym, buf, 0);
		break;
	}

	return E_SUCCESS;
}

int process_lf_file(char* path, int process_directory) {
	char line[LINE_LENGTH];
	int i = 0;
	FILE *fd;

	fd = fopen(path, "r");
        if (!fd) {
                fprintf(stderr, "could not open file: %s\n", path);
                return 0;
        }

	if (!process_directory) {
		init_hash_table(&sym); // symbol table that maps labels, functions
	}

	// we start not inside any function, user must declare one
	current_function[0] = 0;
	while (fgets(line, LINE_LENGTH, fd) && ++i) {
		char *token, *pos, args = 0, arg1[LINE_LENGTH];
                int len = strlen(line), n = 0, arg2, err, func = 0;

                pos = strstr(line, "//");
                if (pos) {
                        len = pos - line;
                        line[len] = 0;
                }

                if (!len) continue;

		token = strtok(line, " \t\r\n");
		if (token) {
			if (!strcmp(token, "function")) {
				func = FUNCTION;
			} else if (!strcmp(token, "label")) {
				func = LABEL;
			} else if (!strcmp(token, "push") || !strcmp(token, "pop") || !strcmp(token, "add") || !strcmp(token, "sub") || !strcmp(token, "neg") || !strcmp(token, "or") || !strcmp(token, "and") || !strcmp(token, "not") || !strcmp(token, "lt") || !strcmp(token, "gt") || !strcmp(token, "eq")|| !strcmp(token, "goto") || !strcmp(token, "if-goto") || !strcmp(token, "return") || !strcmp(token, "call")) {
				continue; // ignore
			} else {
				fprintf(stderr, "ERROR: unknown instruction in file %s, line %d: %s\n", path, i, token);
				return 0;
			}

			while ((token = strtok(NULL, " \t\r\n"))) {
				args++;

				if (arguments[func] < args) {
					fprintf(stderr, "ERROR: in file %s, line %d: wrong argument count for instruction, should be %d\n", path, i, arguments[func]);
					return 0;			
				}

				if (args == 1) {
					strcpy(arg1, token);		
				} else if (args == 2) {				
					if (!process_number(token, &n)) {
						fprintf(stderr, "ERROR: invalid number in file %s, line %d: %s\n", path, i, token);
						return 0;
					}

					arg2 = n;
				}
			}

			if (arguments[func] != args) {
				fprintf(stderr, "ERROR: in file %s, line %d: wrong argument count (%d) for instruction, should be %d\n", path, i, args, arguments[func]);
				return 0;			
			}

			if ((err = process_lf(func, arg1, arg2)) != E_SUCCESS) {
				switch (err) {
					case E_NOT_INSIDE_FUNCTION:
						fprintf(stderr, "ERROR: in file %s, line %d: all instructions must be inside a function\n", path, i);						
					break;

					case E_LABEL_ALREADY_DEFINED:
						fprintf(stderr, "ERROR: in file %s, line %d: label %s already defined inside function\n", path, i, arg1);						
					break;

					case E_FUNCTION_ALREADY_DEFINED:
						fprintf(stderr, "ERROR: in file %s, line %d: function %s is already defined\n", path, i, arg1);						
					break;
				}

				return 0;
			}
		}
	}

	fclose(fd);
	return 1;
}

int process_lf_directory(char* path) {
	DIR* d = opendir(path);
	struct dirent* de;
	if (!d) return 0; // will signal that we want to try to process it as a file next time

	init_hash_table(&sym); // symbol table that maps labels, functions

	while ((de = readdir(d)) != NULL) {
		char fpath[256];

		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
			continue;

		strcpy(fpath, path);
		strcat(fpath, "/");
		strcat(fpath, de->d_name);

		if (!process_lf_file(fpath, 1)) { 
			return -1; // return -1 to signal that there is no point in trying to parse it as file
		}
	}

	closedir(d);
	return 1;
}

int main(int argc, char* argv[]) {
	int ret;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <file.vm|directory>\n", argv[0]);
		return -1;
	}

	// first pass, process all function and label names
	if (!(ret = process_lf_directory(argv[1]))) {
		process_lf_file(argv[1], 0);
	} else if (ret == -1) {
		return -1;
	}

	// second pass, translate stuff
	if (!(ret = process_directory(argv[1]))) {
		process_file(argv[1], 0);
	} else if (ret == -1) {
		return -1;
	}
	
	return 0;
}
