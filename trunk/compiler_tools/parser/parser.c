#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "hash.h"
#include "stack.h"

#define LINE_LENGTH 256
#define OUTPUT_EXTENSION ".xml"

static hash_table* grammar; // keys are rule identifiers, values are pointers to rules

// one or more ranges inside the same class
struct range_t {
	char range_start;
	char range_end;
	struct range_t* next;
};

// a class is either a single character or a selection of characters including zero or more ranges i.e. [a-zA-Z0-9_&%]
struct class_t {
	int occurrences; // occurrences for this class, 1 - default, 2 - 0 or more, 3 - 1 or more, 4 - 0 or 1
	struct range_t* ranges; // the ranges in the order they are encountered (even single char are ranges)

	int match_offset; // the offset within the buffer of the first match of this class	
	int matched; // how many times this class has been matched

	struct class_t *prev, *next;
};

// the basic token in a rule, contains one or more classes or is a parent to one or more children rule elements
struct rule_element_t {	
	// if identifier and rules are NULL, this element is not a rule reference, but an expression
	char* identifier; // will be temporarily used, until we resolve the name of the rule
	struct rule_t* rule;

	int occurrences; // occurrences for this rule element (if a parent), 1 - default, 2 - 0 or more, 3 - 1 or more, 4 - 0 or 1; if rule != NULL, occurrences
	// are for the rule instead of the element
	int or; // whether to perform logical OR between this element and the one after it instead of AND (default)
	int allow_whitespace; // whether to allow whitespace (" ", "\r", "\n" or "\t") between this rule element and the one before it
	struct class_t* classes; // the classes in the order they are encountered

	struct rule_element_t* children, *parent;
	struct rule_element_t* next; // next rule element, either a reference to a rule or an expression
	struct rule_element_t* prev;
};

// one or more rule elements
struct rule_t {
	char* name;
	struct rule_element_t* elements;
};

struct rule_t* master_rule = NULL;

#define ONE_OCCURRENCE 			1
#define ONE_OR_MORE_OCCURRENCES		2
#define ZERO_OR_MORE_OCCURRENCES	3
#define ZERO_OR_ONE_OCCURRENCES		4

#define MASTER_RULE "master"

int get_out_file(char* outfile, char* infile, char* base) {
	int len = strlen(infile);
	char *tmp;
	if (len > 5 && !strcmp(".jack", infile + len - 5)) {
		strncpy(outfile, infile, len - 5);
		outfile[len-5] = 0;
		strcat(outfile, OUTPUT_EXTENSION);
	} else {
		return 0;
	}

	for (tmp = infile + len - 1; tmp >= infile; tmp--) {
		if (*tmp == '/') break;
	}
	tmp++;

	strncpy(base, tmp, len - 5 - (tmp - infile));
	base[len - 5 - (tmp - infile)] = 0;

	return 1;
}

inline int is_optional(struct rule_element_t* e) {
	return e->occurrences == ZERO_OR_ONE_OCCURRENCES || e->occurrences == ZERO_OR_MORE_OCCURRENCES;
}

inline int is_required(struct rule_element_t* e) {
	return e->occurrences == ONE_OCCURRENCE || e->occurrences == ONE_OR_MORE_OCCURRENCES;
}

inline int is_class_required(struct class_t* c) {
	return c->occurrences == ONE_OCCURRENCE || c->occurrences == ONE_OR_MORE_OCCURRENCES;
}

inline int is_or_expression(struct rule_element_t* e) {
	return  e->or || (e->prev && e->prev->or);
}

inline int is_special(struct rule_element_t* e) {
	return e->children || e->rule || is_or_expression(e);
}

struct class_match_t {
	struct class_t* class;
	struct rule_element_t* element;

	int position;
	int output_offset;
	int match_number;

	struct stack_t* em_stack;
	struct stack_t* rc;

	struct class_match_t* prev;

	// class_match -> element_match link
	struct element_match_t* prev_em;
};

struct element_match_t {
	struct rule_element_t* element;

	int matches_needed;
	int position;
	int output_offset;
	int match_number;
	int increase_matches;

	struct stack_t* em_stack;
	struct stack_t* rc;

	struct element_match_t* prev, *start;

	// element_match -> class_match link
	struct class_match_t* prev_cm;
};

void add_em(struct element_match_t** em, struct class_match_t** cm, struct rule_element_t* e, int position, FILE* outfd, struct stack_t* rc, struct stack_t* em_stack) {
	struct element_match_t* newem = malloc(sizeof(struct element_match_t));
	newem->match_number = 0;
	newem->position = position;
	newem->output_offset = ftell(outfd);
	newem->matches_needed = is_required(e);
	newem->element = e;
	newem->prev = *em;
	newem->prev_cm = *cm;
	newem->rc = NULL;
	newem->start = NULL;
	copy_stack(&newem->rc, rc);
	newem->em_stack = NULL;
	newem->increase_matches = 0;
	// cm is made NULL when em is added
	*cm = NULL;
	*em = newem;
}

void mark_element_end(struct element_match_t** em, struct class_match_t** cm, struct element_match_t* start) {
	struct element_match_t* newem = malloc(sizeof(struct element_match_t));
	newem->start = start;
	newem->match_number = start->match_number;
	newem->matches_needed = start->matches_needed;
	newem->element = start->element;
	newem->increase_matches = 0;
	if (*em) {
		newem->prev = *em;
		newem->prev_cm = NULL;
		*em = newem;
	} else {
		newem->prev = NULL;
		newem->prev_cm = *cm;
		*em = newem;
		*cm = NULL;
	}
}

inline int is_end_mark(struct element_match_t* em) {
	return em->start != NULL;
}

void next_selection(struct element_match_t* em, struct rule_element_t** e) {
	// go to next element in the OR expression, reset its attributes
	*e = (*e)->next;
	em->element = *e;
	em->matches_needed = is_required(*e);
	em->match_number = 0;
}

void add_cm(struct class_match_t** cm, struct element_match_t** em, struct rule_element_t* e, struct class_t* c, int position, FILE* outfd, struct stack_t* rc, struct stack_t* em_stack) {
	struct class_match_t* newcm = malloc(sizeof(struct class_match_t));
	newcm->class = c;
	newcm->element = e;
	newcm->position = position;
	newcm->output_offset = ftell(outfd);
	newcm->match_number = is_class_required(c);
	newcm->prev = *cm;
	newcm->prev_em = *em;
	newcm->rc = NULL;
	copy_stack(&newcm->rc, rc);
	newcm->em_stack = NULL;
	copy_stack(&newcm->em_stack, em_stack);
	// em is made NULL when cm is added
	*em = NULL;
	*cm = newcm;
}

inline void increase_matches(struct class_match_t* cm, FILE* outfd) {
	cm->match_number++;
	cm->output_offset = ftell(outfd);
}

inline void increase_matches_needed(struct element_match_t* em) {
	em->matches_needed++;
	em->match_number = 0;
}

void copy_stacks(struct class_match_t* cm, struct element_match_t* em, struct stack_t** rc, struct stack_t** em_stack) {
	if (em) {
		copy_stack(rc, em->rc);
		copy_stack(em_stack, em->em_stack);
	} else if (cm) {
		copy_stack(rc, cm->rc);
		copy_stack(em_stack, cm->em_stack);
	}
}

void go_back(struct class_match_t** cm, struct element_match_t** em, struct element_match_t** last_em, struct stack_t** rc, struct stack_t** em_stack) {
	struct element_match_t* tmp_em = NULL;
	struct class_match_t* tmp_cm = NULL;

	// go back one step, skip elements used to mark the end of a match (is_end_mark)
	if (*cm) {
		tmp_cm = *cm;
		*em = (*cm)->prev_em;
		*cm = (*cm)->prev;
	} else if (*em) {
		tmp_em = *em;
		*cm = (*em)->prev_cm;
		*em = (*em)->prev;
	}

	if (tmp_cm) {
		clear_stack(&tmp_cm->rc);
		clear_stack(&tmp_cm->em_stack);
		free(tmp_cm);
	} else {
		clear_stack(&tmp_em->rc);
		clear_stack(&tmp_em->em_stack);
		free(tmp_em);
	}

	while (*em) {
		tmp_em = NULL;

		if (!is_end_mark(*em)) {
			break;
		} else {
			(*em)->start->match_number = (*em)->match_number - 1; // reset match number
			tmp_em = *em;
		}

		*cm = (*em)->prev_cm;
		*em = (*em)->prev;
		if (tmp_em) free(tmp_em);
	}
	
	copy_stacks(*cm, *em, rc, em_stack);
	if (*em) {
		*last_em = *em;
	} else {
		tmp_em = NULL;
		tmp_cm = *cm;
		while (!tmp_em || is_end_mark(tmp_em)) {
			if (tmp_cm) {
				tmp_em = tmp_cm->prev_em;
				tmp_cm = tmp_cm->prev;
			} else if (tmp_em) {
				tmp_cm = tmp_em->prev_cm;
				tmp_em = tmp_em->prev;
			} else {
				break;
			}
		}
		*last_em = tmp_em;
	}
}

inline void reset_position(struct class_match_t* cm, struct element_match_t* em, int* position, FILE* outfd) {
	if (cm) {
		*position = cm->position + cm->match_number;
		fseek(outfd, cm->output_offset, SEEK_SET);
	} else {
		*position = em->position;
		fseek(outfd, em->output_offset, SEEK_SET);
	}
}

#define POP_EM(top)		(struct element_match_t*)pop(top);
#define PUSH_EM(top, em)	push(top, (value_t)em)
#define PEEK_EM(top)		(struct element_match_t*)peek(top)

#define POP_RC(top)		(struct rule_element_t*)pop(top);
#define PUSH_RC(top, e)		push(top, (value_t)e)
#define PEEK_RC(top)		(struct rule_element_t*)peek(top)

int can_go_back(struct element_match_t* em, struct class_match_t* cm) {
	// check if we have a cm/em to go back to
	if (cm) {
		em = cm->prev_em;
		cm = cm->prev;
	} else if (em) {
		cm = em->prev_cm;
		em = em->prev;
	} else {
		return 0;
	}

	while (em) {
		if (!is_end_mark(em)) {
			break;
		}
		cm = em->prev_cm;
		em = em->prev;
	}

	return cm != NULL || em != NULL;
}

int can_skip_whitespace(struct rule_element_t* e) {
	if (e->allow_whitespace) return 1;

	// check that element does not have any element preceding it (other than a parent)
	if (!e->prev) {
		e = e->parent;
		while (e) {
			if (e->prev && !e->allow_whitespace) return 0;
			if (e->allow_whitespace) return 1;
			e = e->parent;
		}
	}

	return 0;
}

inline int can_rematch(struct class_match_t* cm, struct element_match_t* em) {
	// return true if element/class may be attempted a rematch, false otherwise
	if (em) {
		return !(em->element->occurrences == ONE_OCCURRENCE || (em->element->occurrences == ZERO_OR_ONE_OCCURRENCES && em->matches_needed > 0));
	} else {
		return !(cm->class->occurrences == ONE_OCCURRENCE || (cm->class->occurrences == ZERO_OR_ONE_OCCURRENCES && cm->match_number > 0));
	}
}

inline struct element_match_t* get_start(struct element_match_t* em) {
	return em->start ? em->start : em;
}

void clean_up(struct class_match_t* cm, struct element_match_t* em) {
	struct element_match_t* tmp_em = NULL;
	struct class_match_t* tmp_cm = NULL;

	while (cm || em) {
		if (cm) {
			tmp_cm = cm;
			em = cm->prev_em;
			cm = cm->prev;
			clear_stack(&tmp_cm->rc);
			clear_stack(&tmp_cm->em_stack);
			free(tmp_cm);
		} else {
			tmp_em = em;
			cm = em->prev_cm;
			em = em->prev;
			clear_stack(&tmp_em->rc);
			clear_stack(&tmp_em->em_stack);
			free(tmp_em);
		}
	}
}

int compile(FILE* fd, FILE* outfd, char* buffer, int length, char* unexpected) {
	struct rule_element_t* e;
	struct class_t* c;
	struct range_t* r;

	struct element_match_t *em = NULL, *last_em = NULL;
	struct class_match_t *cm = NULL;

	struct stack_t* top = NULL, *rc = NULL;

	int position = 0, furthest = 0;
	char ch;
	char buf[LINE_LENGTH];

	e = master_rule->elements;

	// em only for parents/rule containing elements/elements in an OR expression
	// cm for normal class containing elements not in an OR expression
	// only one of em/cm will be non-null at a time, depending on what is currently being matched

	for (;;) {
		int re_match_element = 0, re_match_class = 0, no_increase = 0;

		match_element:
		re_match_element = em && em->element == e;

		// special elements are tracked thru em's
		if (is_special(e)) {
			if (!re_match_element) {
				// add em if not already present
				if (!cm && !em) { // make sure we have a previous em to link it to
					em = PEEK_EM(top);
				}
				add_em(&em, &cm, e, position, outfd, rc, top);
				last_em = em;
				PUSH_EM(&top, em);
				copy_stack(&em->em_stack, top);
			} else {
				if (em->increase_matches || (can_rematch(cm, em) && !no_increase)) {
					PUSH_EM(&top, get_start(em));
					em->increase_matches = 0;
					copy_stack(&(get_start(em)->em_stack), top);
				} else if (is_or_expression(e)) {
					// go to next element in OR expression
					next_selection(get_start(em), &e);
					re_match_element = 0;
					no_increase = 0;
				}
			}
		}
		if (cm) em = NULL; // destroy temporarily used em if cm is set

		if (re_match_element || is_required(e)) {

			if (e->children) {
				// parent element
				e = e->children;
				goto match_element;
			} else if (e->rule) {
				// rule element
				PUSH_RC(&rc, e);
				sprintf(buf, "<%s>", e->rule->name);
				fputs(buf, outfd);
				e = e->rule->elements;
				goto match_element;
			} else {
				// normal class containing element (terminal)
				// classes of normal elements are tracked thru cm's
				re_match_class = 0;
				c = e->classes;

				while (c) {
					int matched = !is_class_required(c) && !re_match_class;
					int advance = !matched;
					int skip_whitespace;
					ch = buffer[position];
					skip_whitespace = ch == ' ' && can_skip_whitespace(e) && c == e->classes && (!cm || cm->class != c); // skip whitespace if allow_whitespace

					if (advance && position == length) {
						// file ended too soon, while there is remaining stuff to match
						goto backtrack;
					}

					if (skip_whitespace) {
						position++;
						putc(ch, outfd);
						continue;
					}

					if (advance) {
						for (r = c->ranges; r; r = r->next) {
							if (ch >= r->range_start && ch <= r->range_end) {
								matched = 1;
								break;
							}
						}
					}

					// match class
					if (matched) { // also true if optional && !re_match
						if (advance) {
							putc(ch, outfd);
						}

						if (!re_match_class) {
							// add new cm
							if (!cm && !em) { // make sure we have a previous em to link it to
								em = PEEK_EM(top);
							}
							add_cm(&cm, &em, e, c, position, outfd, rc, top);
						} else {
							// modify existing cm
							increase_matches(cm, outfd);
						}
					} else {
						// match failed, backtrack
						int popped;
						if (position > furthest) {
							// remember character where matching failed
							*unexpected = ch;
							furthest = position;
						}

						backtrack:
						popped = 0;

						if (em && is_end_mark(em)) {
 							if (can_go_back(em, cm)) {
								go_back(&cm, &em, &last_em, &rc, &top);
							} else {
								return 0; // could not go back
							}
						}

						if (!cm && !em) {
							popped = 1;
							em = POP_EM(&top); // no class matched successfully, return to em (if any)
						}

						if (cm || em) {
							if ((em && !popped) || (cm && !re_match_class)) {
								// reset element stacks first
								if (em) {
									copy_stack(&rc, em->rc);
									copy_stack(&top, em->em_stack);
								} else if (cm) {
									copy_stack(&rc, cm->rc);
									copy_stack(&top, cm->em_stack);
								}
								goto do_rematch; // do not go back if not rematching or if em was not popped from the stack
							}

							// go back
							if (can_go_back(em, cm)) {
								go_back(&cm, &em, &last_em, &rc, &top);
								goto do_rematch;
							} else {
								return 0; // could not go back
							}							
						} else {
							return 0; // nothing set
						}

						do_rematch:
						if (cm) {
							if (!can_rematch(cm, em)) {
								if (can_go_back(em, cm)) {
									go_back(&cm, &em, &last_em, &rc, &top);
									goto do_rematch;
								} else {
									return 0; // could not go back
								}
							}

							// reset position, output offset
							reset_position(cm, em, &position, outfd);
							c = cm->class;
							e = cm->element;

							re_match_class = 1;

							continue;
						} else if (em) {
							no_increase = 0;
							e = em->element;
							reset_position(cm, em, &position, outfd);

							// if one occurrence we cannot match it again
							if (!can_rematch(cm, em) && !is_or_expression(em->element)) {
								if (can_go_back(em, cm)) {
									go_back(&cm, &em, &last_em, &rc, &top);
									goto do_rematch;
								} else {
									return 0; // could not go back
								}
							}

							// go back if match incomplete and it's not an OR expression
							if (em->match_number < em->matches_needed) {
								no_increase = 1;
								if (!is_or_expression(em->element)) {
									no_increase = 0;
									// element incompletely matched, go back
									if (can_go_back(em, cm)) {
										go_back(&cm, &em, &last_em, &rc, &top);
										goto do_rematch;
									} else {
										return 0; // could not go back
									}
								}
							}

							if (e->prev && e->prev->or && !e->or && (!can_rematch(cm, em) || no_increase)) {
								// in OR expression, but no term after and not rematchable
								if (can_go_back(em, cm)) {
									no_increase = 0;
									go_back(&cm, &em, &last_em, &rc, &top);
									goto do_rematch;
								} else {
									return 0; // could not go back
								}
							}

							if (can_rematch(cm, em) && !no_increase) {
								increase_matches_needed(em);
								em->increase_matches = 1;
								if (PEEK_EM(top) == em) POP_EM(&top); // pop it since it will be readded
							}

							goto match_element;
						}
					}

					re_match_class = 0;
					c = c->next;

					if (advance) {
						position++;
					}	
				} // done matching classes
			} // end of normal, class containing element processing
		}

	
		next:
		// if is in OR expression and is not parent or rule container should pop because push was done for it
		if (PEEK_EM(top) && (PEEK_EM(top))->element == e && ((is_or_expression(e) && !e->children && !e->rule) || (e->children || e->rule))) {
			em = POP_EM(&top);
			if (get_start(em)->match_number < get_start(em)->matches_needed) {
				get_start(em)->match_number++;
				mark_element_end(&em, &cm, get_start(em));
			}			
		}

		if (!em || em->match_number >= em->matches_needed) {
			// element matched enough times, go on to next
			if (cm) em = NULL; // destroy temporarily used em if cm is set

			// already selected this element, so skip the rest in the OR expression
			while (e->or) {
				e = e->next;
			}

			if (e->next) {
				if (!cm && last_em) em = last_em; // done matching this part, fetch last added em
				e = e->next; // visit next element
			} else if (e->parent) {
				e = e->parent;
				goto next;
			} else if (PEEK_RC(rc)) {
				e = POP_RC(&rc); // go up
				sprintf(buf, "</%s>", e->rule->name);
				fputs(buf, outfd);
				goto next;
			} else {
				if (position == length) {
					// perform clean up before continuing to next file
					clean_up(cm, em);
					ftruncate(fileno(outfd), ftell(outfd));
					return 1;
				} else {
					if (!cm && last_em) em = last_em;
					// ran out of classes/elements, file did not end yet, backtrack
					goto backtrack;

				}
			}
		} else {
			// match element one more time
			e = em->element;
		}
	}
}

int process_file(char* path, int directory_caller) {
	char outfile[256], base[256], buf[LINE_LENGTH], *buffer, unexpected[1];
	int buf_len = 0;
	FILE *fd, *outfd;

	if (!get_out_file(outfile, path, base)) {
		if (!directory_caller) { 
			fprintf(stderr, "argument (%s) must be a file with a .jack extension or a directory\n", path);
			return 0;
		}
	
		return -1; // we can skip non .jack files if in a directory
	}

	fd = fopen(path, "r");
	printf("processing file %s\n", path);

        if (!fd) {
                fprintf(stderr, "could not open file: %s\n", path);
                return 0;
        }

	fseek(fd, 0, SEEK_END);
	buffer = malloc(ftell(fd) + 1);
	rewind(fd);

	// process and write to output
	outfd = fopen(outfile, "w");

	// read file into buffer
	buffer[0] = 0;
	while (fgets(buf, LINE_LENGTH, fd)) {
		char *pos, newbuf[LINE_LENGTH];
		int len = strlen(buf), idx = 0, newidx = 0;
		char literal = 0;

                if (buf[len - 1] == '\n' || buf[len - 1] == '\r') { len--; buf[len] = 0; }
                if (buf[len - 1] == '\n' || buf[len - 1] == '\r') { len--; buf[len] = 0; }

		// skip comments
		pos = strstr(buf, "//");
		if (pos) {
			len = pos - buf;
			buf[len] = 0;
		}
		// FIXME: multiline comments not handled

		// skip leading whitespace
		while (buf[idx] == ' ' || buf[idx] == '\t') { idx++; }

		if (idx == len) continue; // blank line

		for (; idx < len; idx++) {
			// more than 1 tab/space at a time not allowed
			if (newidx && !literal && (buf[idx] == ' ' || buf[idx] == '\t') && (buf[idx-1] == ' ' || buf[idx-1] == '\t')) continue;
			if (buf[idx] == '\'' || buf[idx] == '"') {
				if (!literal) literal = buf[idx];
				else if (literal == buf[idx]) literal = 0;
			}

			if (!literal && buf[idx] == '\t') buf[idx] = ' '; // all tabs will be spaces (except when in string literals)
			newbuf[newidx++] = buf[idx];
		}
		newbuf[newidx] = 0;

		// append to buffer
		strcat(buffer, newbuf);
		buf_len += newidx;
	}
	fclose(fd);

	buffer[buf_len] = 0;

	if (!compile(fd, outfd, buffer, buf_len, unexpected)) {
		// FIXME: include line information in error
		fprintf(stderr, "unexpected character '%c'\n", *unexpected);
	} else {
		fprintf(stderr, "success.\n");
	}

	fclose(outfd);
	return 1;
}

int process_directory(char* path) {
	DIR* d = opendir(path);
	struct dirent* de;
	if (!d) return 0; // will signal that we want to try to process it as a file next time

	while ((de = readdir(d)) != NULL) {
		char fpath[256];

		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
			continue;

		strcpy(fpath, path);
		strcat(fpath, "/");
		strcat(fpath, de->d_name);

		if (!process_file(fpath, 1)) {
			return -1; // return -1 to signal that there is no point in trying to parse it as file
		}
	}

	closedir(d);
	return 1;
}

int resolve_references(struct rule_element_t* element) {
	struct rule_element_t* e;

	for (e = element->children; e; e = e->children) {
		if (!resolve_references(e)) return 0;
	}

	for (e = element->next; e; e = e->next) {
		if (!resolve_references(e)) return 0;
	}

	if (element->identifier) {
		// attempt to replace with rule
		value_t ptr = get_item(grammar, element->identifier);
		if (ptr == -1) {
			fprintf(stderr, "ERROR: referenced rule %s was not found!\n", element->identifier);
			return 0;
		}
		element->identifier = NULL;
		element->rule = (struct rule_t*)ptr;
	}

	return 1;
}

int check_range(struct class_t* class, char start, char end, int line_number, struct range_t* range) {
	struct range_t* r;

	if (!class) return 1;

	for (r = class->ranges; r; r=r->next) {
		if (range != r && r->range_start <= end && r->range_end >= start) {
			if (start != end)
				fprintf(stderr, "ERROR: range %c-%c may not intersect other ranges, on line %d\n", start, end, line_number);
			else
				fprintf(stderr, "ERROR: character %c may not intersect other ranges, on line %d\n", start, line_number);
			return 0;
		}
	}

	return 1;
}

int process_grammar(char* path) {
	char line[LINE_LENGTH];
	int i = 0;
	FILE *fd;

	fd = fopen(path, "r");
	printf("processing file %s\n", path);

        if (!fd) {
                fprintf(stderr, "could not open file: %s\n", path);
                return 0;
        }

	init_hash_table(&grammar);
	while (fgets(line, LINE_LENGTH, fd) && ++i) {
		char token[LINE_LENGTH], *pos, identifier[LINE_LENGTH];
                int len = strlen(line);
		int idx = 0;

		int tlen, tmp_idx;
		int element_group = 0, or = 0, create_new_element = 1, append_element = 0;
		struct rule_t* cur_rule;
		struct rule_element_t* cur_element, *prev_element = NULL, *parent_element = NULL, *tmp_element;
		char *space, *tab;

		identifier[0] = 0;

                if (line[len - 1] == '\n' || line[len - 1] == '\r') { len--; line[len] = 0; }
                if (line[len - 1] == '\n' || line[len - 1] == '\r') { len--; line[len] = 0; }

                pos = strstr(line, "//");
                if (pos) {
                        len = pos - line;
                        line[len] = 0;
                }

                if (!len) continue;

		while (idx < len) {
			while (line[idx] == ' ' || line[idx] == '\t') idx++;
			space = strchr(line + idx, ' ');
			tab = strchr(line + idx, '\t');

			if (space && tab) {
				tlen = space > tab ? (tab - line) - idx : (space - line) - idx;
			} else if (space) {
				tlen = (space - line) - idx;
			} else if (tab) {
				tlen = (tab - line) - idx;
			} else {
				tlen = len - idx; // no space or tab, continue until the end of the line
			}
				
			strncpy(token, line + idx, tlen);		
			token[tlen] = 0;
			printf("token = %s len = %d idx = %d\n", token, tlen, idx);

			if (!identifier[0]) {
				// read rule identifier
				if (tlen < 1 || token[tlen-1] != ':') {
					fprintf(stderr, "ERROR: in grammar file %s line %d line must start with <identifier>:\n", path, i);
					return 0;
				}
				strcpy(identifier, token);
				identifier[tlen-1] = 0;
				if (!strcmp(identifier, MASTER_RULE)) {
					// is master rule
					if (master_rule) {
						fprintf(stderr, "ERROR: in grammar file %s line %d duplicate rule\n", path, i);
						return 0;
					}

					master_rule = malloc(sizeof(struct rule_t));
					master_rule->elements = NULL;
					master_rule->name = MASTER_RULE;
					cur_rule = master_rule;
				} else {
					if (get_item(grammar, identifier) != -1) {
						fprintf(stderr, "ERROR: in grammar file %s line %d duplicate rule\n", path, i);
						return 0;
					}

					// create new rule
					cur_rule = malloc(sizeof(struct rule_t));
					cur_rule->elements = NULL;
					cur_rule->name = malloc(tlen-1);
					strcpy(cur_rule->name, identifier);
				}

				idx += tlen;
				continue;
			} else {
				// read rule elements
				char prev;
				struct class_t* prev_class = NULL, *cur_class = NULL;
				struct range_t* prev_range = NULL, *cur_range = NULL;
				char ref[LINE_LENGTH], ref_started = 0;
				int ref_offset;
				int multiple = 0, range = 0, class = 0, escape = 0, occurrence = 0;

				if (create_new_element) {
					cur_element = malloc(sizeof(struct rule_element_t));
					cur_element->identifier = NULL;
					cur_element->rule = NULL;
					cur_element->children = NULL;
					cur_element->next = NULL;
					cur_element->classes = NULL;
					cur_element->or = 0;
					cur_element->occurrences = ONE_OCCURRENCE;
					cur_element->allow_whitespace = 1;
					cur_element->prev = prev_element;

					if (prev_element) {
						prev_element->next = cur_element;
						cur_element->parent = prev_element->parent;
					} else {
						cur_rule->elements = cur_element;
						cur_element->parent = parent_element;
					}
				} else if (prev_element) {
					prev_element->allow_whitespace = 1; // previous element is an OR element created prematurely
				}

				tmp_element = prev_element;
				prev_element = cur_element;
				append_element = 0;
				tmp_idx = idx;

				// TODO: handle exclusion classes
				process_token:
				for (; idx < tmp_idx + tlen; idx++) {
					if (line[idx] != '+' && line[idx] != '*' && line[idx] != '?' &&
						line[idx] != ')' && append_element) {
						// add neighbour after ')' or '}' is encountered (if nested, wait for ')'s to end first)
						cur_element = malloc(sizeof(struct rule_element_t));
						cur_element->identifier = NULL;
						cur_element->rule = NULL;
						cur_element->children = NULL;
						cur_element->next = NULL;
						cur_element->classes = NULL;
						cur_element->or = 0;
						cur_element->occurrences = ONE_OCCURRENCE;
						cur_element->allow_whitespace = 0; // no whitespace allowed, there must be a space between rule elements to indicate that
						cur_element->prev = prev_element;

						if (prev_element) {
							prev_element->next = cur_element;
							cur_element->parent = prev_element->parent;
						} else {
							cur_rule->elements = cur_element;
							cur_element->parent = parent_element;
						}
						cur_class = NULL;
						prev_class = NULL;
						prev_range = NULL;
						tmp_element = prev_element;
						prev_element = cur_element;
						append_element = 0;
					}

					create_new_element = 1;
					
					if (!ref_started && !escape && line[idx] == '[') {
						if (multiple) {
							fprintf(stderr, "ERROR: in grammar file %s line %d character %d unexpected '['\n", path, i, idx+1);
							return 0;
						}

						// start of multiple characters
						multiple = 1;
						or = 0;
						occurrence = 0;
						cur_class = malloc(sizeof(struct class_t));
						cur_class->occurrences = ONE_OCCURRENCE;
						cur_class->next = NULL;
						cur_class->ranges = NULL;
						prev_range = NULL;

						if (prev_class) {
							prev_class->next = cur_class;
						} else {
							cur_element->classes = cur_class;
						}
						prev_class = cur_class;
					} else if  (!ref_started && !escape && (line[idx] == '+' || line[idx] == '*' || line[idx] == '?')) {
						if (or || multiple || !class) {
							fprintf(stderr, "ERROR: in grammar file %s line %d character %d unexpected '%c'\n", path, i, idx+1, line[idx]);
							return 0;
						}

						// + - one or more, * - zero or more, ? - zero or one
						if (cur_class)
							cur_class->occurrences = (line[idx] == '+' ? ONE_OR_MORE_OCCURRENCES : (line[idx] == '*' ? 
								ZERO_OR_MORE_OCCURRENCES : ZERO_OR_ONE_OCCURRENCES));
						else
							tmp_element->occurrences = (line[idx] == '+' ? ONE_OR_MORE_OCCURRENCES : (line[idx] == '*' ? 
								ZERO_OR_MORE_OCCURRENCES : ZERO_OR_ONE_OCCURRENCES));

						class = 0; // class consumed
						occurrence = 1;
					} else if (!ref_started && !escape && line[idx] == ']') {
						if (or || range) {
							fprintf(stderr, "ERROR: in grammar file %s line %d character %d unexpected ']'\n", path, i, idx+1);
							return 0;
						}

						// end of multiple characters
						class = 1; // we now have a class of characters so the + / * operators may be used
						multiple = 0;
					} else if (!ref_started && !escape && line[idx] == '-') {
						if (or || !multiple || !class) {
							fprintf(stderr, "ERROR: in grammar file %s line %d character %d unexpected '-'\n", path, i, idx+1);
							return 0;
						}

						// marks range of characters
						range = prev;
					} else if (!ref_started && !escape && line[idx] == '(') {
						if (multiple) {
							fprintf(stderr, "ERROR: in grammar file %s line %d character %d unexpected '('\n", path, i, idx+1);
							return 0;
						}

						// append child rule element

						// if the current element is not empty, create a new element to hold the child
						if (cur_element->classes || cur_element->identifier || cur_element->rule) {
							prev_element = cur_element;
							cur_element = malloc(sizeof(struct rule_element_t));
							cur_element->identifier = NULL;
							cur_element->rule = NULL;
							cur_element->next = NULL;
							cur_element->children = NULL;
							cur_element->classes = NULL;
							cur_element->or = 0;
							cur_element->occurrences = ONE_OCCURRENCE;
							cur_element->parent = parent_element;
							cur_element->allow_whitespace = 0; // no whitespace allowed, since there was no space between this element and the previous one
							prev_element->next = cur_element;
							cur_element->prev = prev_element;
						}

						parent_element = cur_element;
						cur_element = malloc(sizeof(struct rule_element_t));
						cur_element->identifier = NULL;
						cur_element->rule = NULL;
						cur_element->children = NULL;
						cur_element->next = NULL;
						cur_element->classes = NULL;
						cur_element->or = 0;
						cur_element->occurrences = ONE_OCCURRENCE;
						cur_element->parent = parent_element;
						cur_element->allow_whitespace = 0; // no whitespace allowed, since there was no space between this element and the previous one
						cur_element->prev = NULL;
		
						// for ( (...) (...) ) check if it already has children, if so, append to end
						if (parent_element->children) {
							struct rule_element_t* e;
							for (e = parent_element->children; e->next; e = e->next);
							e->next = cur_element;
						} else {
							parent_element->children = cur_element;
						}

						cur_class = NULL;
						prev_class = NULL;
						prev_range = NULL;
						prev_element = cur_element;
						tmp_element = NULL; // new element has no previous element

						or = 0;
						occurrence = 0;

						element_group++;
					} else if (!ref_started && !escape && line[idx] == ')') {
						if (or || !element_group) {
							fprintf(stderr, "ERROR: in grammar file %s line %d character %d unexpected ')'\n", path, i, idx+1);
							return 0;
						}

						if (!cur_element->identifier && !cur_element->rule && !cur_element->children && !cur_element->classes) {
							fprintf(stderr, "ERROR: in grammar file %s line %d invalid rule element: empty parent\n", path, i);
							return 0;
						}

						append_element = 1;

						tmp_element = cur_element->parent;
						cur_element = cur_element->parent;
						prev_element = cur_element;
						parent_element = cur_element->parent;
						cur_class = NULL;
						class = 1;						

						element_group--;
					} else if (!escape && line[idx] == '{') {
						if (ref_started || range || multiple) {
							fprintf(stderr, "ERROR: in grammar file %s line %d character %d unexpected '{'\n", path, i, idx+1);
							return 0;
						}

						// if element not empty, create neighbour
						if (cur_element->classes || cur_element->identifier || cur_element->rule) {
							cur_element = malloc(sizeof(struct rule_element_t));
							cur_element->identifier = NULL;
							cur_element->rule = NULL;
							cur_element->children = NULL;
							cur_element->next = NULL;
							cur_element->classes = NULL;
							cur_element->or = 0;
							cur_element->occurrences = ONE_OCCURRENCE;
							cur_element->allow_whitespace = 0; // no whitespace allowed, since there was no space between this element and the previous one
							cur_element->prev = prev_element;

							if (prev_element) {
								prev_element->next = cur_element;
								cur_element->parent = prev_element->parent;
							} else {
								cur_rule->elements = cur_element;
								cur_element->parent = parent_element;
							}
							cur_class = NULL;
							prev_class = NULL;
							prev_range = NULL;
							prev_element = cur_element;
						}

						or = 0;
						occurrence = 0;
						ref_started = 1;
						ref_offset = 0;
					} else if (!escape && line[idx] == '}') {
						if (or || !ref_started || !ref_offset) {
							fprintf(stderr, "ERROR: in grammar file %s line %d character %d unexpected '}'\n", path, i, idx+1);
							return 0;
						}

						// reference to another identifier
						// check if we can resolve it
						ref[ref_offset] = 0;
						value_t ptr = get_item(grammar, ref);
						if (ptr != -1) {
							// found
							cur_element->rule = (struct rule_t*)ptr;
						} else {
								// not found
							cur_element->identifier = malloc(ref_offset);
							strcpy(cur_element->identifier, ref);
						}	
						tmp_element = cur_element;
						append_element = 1;						

						ref_started = 0;
						class = 1;
					} else if (!ref_started && !escape && line[idx] == '|') {
						if (or || range || multiple || (!class && !occurrence && !tmp_element)) {
							fprintf(stderr, "ERROR: in grammar file %s line %d character %d unexpected '|'\n", path, i, idx+1);
							return 0;
						}

						// if element not empty, create neighbour to hold further content
						if (cur_element->children || cur_element->classes || cur_element->identifier || cur_element->rule) {
							cur_element = malloc(sizeof(struct rule_element_t));
							cur_element->identifier = NULL;
							cur_element->rule = NULL;
							cur_element->children = NULL;
							cur_element->next = NULL;
							cur_element->classes = NULL;
							cur_element->or = 0;
							cur_element->occurrences = ONE_OCCURRENCE;
							cur_element->allow_whitespace = 0; // no whitespace allowed, since there was no space between this element and the previous one
							cur_element->prev = prev_element;

							if (prev_element) {
								prev_element->next = cur_element;
								cur_element->parent = prev_element->parent;
							} else {
								cur_rule->elements = cur_element;
								cur_element->parent = parent_element;
							}
							cur_class = NULL;
							prev_class = NULL;
							prev_range = NULL;
							prev_element->or = 1; // mark that logical OR is to be used

							tmp_element = prev_element;
							prev_element = cur_element;
						} else if (tmp_element) { // has previous element
							tmp_element->or = 1; // mark that logical OR is to be used
						}

						create_new_element = 0; // do not create a new element for the next token since one was already created
						or = 1;
						occurrence = 0;
					} else if (!ref_started && !escape && line[idx] == '\\') {
						escape = 1;
					} else {
						if (ref_started) {
							ref[ref_offset++] = line[idx];
							continue;
						}

						// literal
						if (range) {
							// valid ranges: a-z, 0-9, A-Z or subgroups
							if (!(line[idx] > range && ((range >= 'a' && line[idx] <= 'z') || (range >= '0' && line[idx] <= '9') || (range >= 'A' && line[idx] <= 'Z')))) {
								fprintf(stderr, "ERROR: in grammar file %s line %d character %d invalid range in regular expression\n", path, i, idx+1);
								return 0;										
							}

							// store it
							if (!check_range(cur_class, cur_range->range_start, line[idx], i, cur_range)) return 0;
							cur_range->range_end = line[idx];
							range = 0;
						} else {
							if (!multiple) {
								// allocate a new class for this single character
								cur_class = malloc(sizeof(struct class_t));
								cur_class->occurrences = ONE_OCCURRENCE;

								cur_class->next = NULL;

								if (prev_class) {
									prev_class->next = cur_class;
								} else {
									cur_element->classes = cur_class;
								}
								prev_class = cur_class;

								cur_class->ranges = malloc(sizeof(struct range_t));
								cur_class->ranges->next = NULL;
								cur_class->ranges->range_start = line[idx];
								cur_class->ranges->range_end = line[idx];
							} else {
								if (!check_range(cur_class, line[idx], line[idx], i, NULL)) return 0;

								// new range for this class
								cur_range = malloc(sizeof(struct range_t));
								cur_range->next = NULL;
								cur_range->range_start = line[idx];
								cur_range->range_end = line[idx];
								if (prev_range) {
									prev_range->next = cur_range;
								} else {
									cur_class->ranges = cur_range;
								}
								prev_range = cur_range;
							}
						}

						class = 1; // we now have a class of characters so the + / * operators may be used
						escape = 0;
						or = 0;
						occurrence = 0;
						prev = line[idx];
					}
				} // end of for loop reading rule element's characters

				if (escape) {
					// if a space or tab follows, the escape is legal
					if (line[idx] == ' ' || line[idx] == '\t') {
						if (!check_range(cur_class, line[idx], line[idx], i, NULL)) return 0;

						if (!multiple) {
							// allocate a new class for this single character
							cur_class = malloc(sizeof(struct class_t));
							cur_class->occurrences = ONE_OCCURRENCE;

							cur_class->next = NULL;

							if (prev_class) {
								prev_class->next = cur_class;
							} else {
								cur_element->classes = cur_class;
							}
							prev_class = cur_class;

							cur_class->ranges = malloc(sizeof(struct range_t));
							cur_class->ranges->next = NULL;
							cur_class->ranges->range_start = line[idx];
							cur_class->ranges->range_end = line[idx];
						} else {
							// new range for this class
							if (!check_range(cur_class, line[idx], line[idx], i, NULL)) return 0;

							cur_range = malloc(sizeof(struct range_t));
							cur_range->next = NULL;
							cur_range->range_start = line[idx];
							cur_range->range_end = line[idx];
							if (prev_range) {
								prev_range->next = cur_range;
							} else {
								cur_class->ranges = cur_range;
							}
							prev_range = cur_range;
						}

						prev = line[idx];
						class = 1; // we now have a class of characters so the + / * operators may be used
						escape = 0;
						or = 0;
						occurrence = 0;

						// set tlen, tmp_idx to new values to continue the rule element
						idx++;
						space = strchr(line + idx, ' ');
						tab = strchr(line + idx, '\t');

						if (space && tab) {
							tlen = space > tab ? (tab - line) - idx : (space - line) - idx;
						} else if (space) {
							tlen = (space - line) - idx;
						} else if (tab) {
							tlen = (tab - line) - idx;
						} else {
							tlen = len - idx; // no space or tab, continue until the end of the line
						}

						if (tlen) {
			
							strncpy(token, line + idx, tlen);		
							token[tlen] = 0;
							printf("token = %s len = %d idx = %d\n", token, tlen, idx);
							tmp_idx = idx;

							// go back to for loop
							goto process_token;
						}
					} else {
						fprintf(stderr, "ERROR: in grammar file %s line %d '\\' must be followed by an expression\n", path, i);
						return 0;
					}
				}

				if (!or && !cur_element->identifier && !cur_element->rule && !cur_element->children && !cur_element->classes) {
					fprintf(stderr, "ERROR: in grammar file %s line %d invalid rule element: empty\n", path, i);
					return 0;
				}

				if (multiple) {
					fprintf(stderr, "ERROR: in grammar file %s line %d unmatched '['\n", path, i);
					return 0;
				}

				if (range) {
					fprintf(stderr, "ERROR: in grammar file %s line %d '-' must specify a character range\n", path, i);
					return 0;
				}

				// advance to next token
			} //  end of if (read rule element)


			// map rule
			add_item(grammar, identifier, (value_t)cur_rule);
		} // end of line processing for loop

		if (element_group > 0) {
			fprintf(stderr, "ERROR: in grammar file %s line %d unmatched '('\n", path, i);
			return 0;
		}

		if (or) {
			fprintf(stderr, "ERROR: in grammar file %s line %d '|' must be followed by an expression\n", path, i);
			return 0;
		}


		if (!cur_rule->elements) {
			fprintf(stderr, "ERROR: in grammar file %s line %d empty rules are not allowed\n", path, i);
			return 0;
		}
	} // end of file processing while loop

	if (!master_rule) {
		fprintf(stderr, "ERROR: in grammar file %s there must be a rule named '%s'\n", path, MASTER_RULE);
		return 0;
	}

	// resolve rule_element identifiers where non-null
	for (i = 0; i < MAX_BUCKETS; i++) {
		bucket *b;
		if (grammar->buckets[i].value == -1) continue;

		for (b = &grammar->buckets[i]; b; b = b->next) {
			if (!resolve_references(((struct rule_t*)b->value)->elements)) return 0;
		}
	}

	fclose(fd);
	return 1;
}

int main(int argc, char* argv[]) {
	int ret;

	if (argc < 3) {
		fprintf(stderr, "usage: %s <grammar_file> <file.jack|directory>\n", argv[0]);
		return -1;
	}

	if (!process_grammar(argv[1])) {
		return -1;
	}

	if (!(ret = process_directory(argv[2]))) {
		process_file(argv[2], 0);
	} else if (ret == -1) {
		return -1;
	}
	
	return 0;
}
