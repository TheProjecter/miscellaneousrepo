#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// TODO: anything above 6 makes the program crash since it tries all possible solutions.. and memcpy/malloc is way too inefficient
#define NUMBER_COUNT 6 // must be less than or equal to small numbers count + large numbers count
#define LARGE_NUMBERS 25, 50, 75, 100
#define SMALL_NUMBERS 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10
#define MIN_TARGET 100
#define MAX_TARGET 999

enum ops {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_COUNT
};

struct group {
    enum ops operation[NUMBER_COUNT];
    int operand1[NUMBER_COUNT], operand2[NUMBER_COUNT];
};

struct solution {
    struct group groups[NUMBER_COUNT];
    enum ops operation[NUMBER_COUNT];
    int result;
};

struct solutions {
    struct solution* inf;
    struct solutions* next;
};

const char operations[] = {'+', '-', '*', '/'};

int large[] = {LARGE_NUMBERS, 0};
int small[] = {SMALL_NUMBERS, 0};

void select_numbers(unsigned int* selection, unsigned int n_large, unsigned int n_small) {
    unsigned int i, min = n_small < NUMBER_COUNT ? n_small : NUMBER_COUNT;

    printf("Generate a random game using:\n");
    for (i = min; i + n_large >= NUMBER_COUNT; i--) {
        printf("%d) %d small %d large\n", min - i + 1, i, NUMBER_COUNT - i);
    }
    printf("%d) any %d numbers\n\n", min - i + 1, NUMBER_COUNT);
    printf("Press 0 to solve a custom game\n");

    printf("Your pick: ");
    scanf("%d", selection);
}

void generate_game(unsigned int selection, int *numbers, int *target, unsigned int n_large, unsigned int n_small) {
    unsigned int i, n;
    int tmp;
    int p_large = -1, p_small = -1;
    int *both = NULL;
    unsigned int min = n_small < NUMBER_COUNT ? n_small : NUMBER_COUNT;

    if (min - selection + 1 + n_large >= NUMBER_COUNT) {
        p_small = min - selection + 1;
        p_large = NUMBER_COUNT - p_small;
    }

    // shuffle numbers
    if (p_small == -1) {
        both = (int*)malloc(sizeof(int) * (n_large + n_small));
        for (i = 0; i < n_small + n_large; i++) {
            if (i >= n_small)
                both[i] = large[i - n_small];
            else
                both[i] = small[i];
        }

        for (i = 0; i < n_small + n_large; i++) {
            tmp = both[i];
            n = i + rand() % (n_small + n_large - i);
            both[i] = both[n];
            both[n] = tmp;
        }
    } else {
        for (i = 0; i < n_small; i++) {
            tmp = small[i];
            n = i + rand() % (n_small - i);
            small[i] = small[n];
            small[n] = tmp;
        }

        for (i = 0; i < n_large; i++) {
            tmp = large[i];
            n = i + rand() % (n_large - i);
            large[i] = large[n];
            large[n] = tmp;
        }
    }

    for (i = 0; i < NUMBER_COUNT; i++) {
        if (p_small-- > 0) {
            numbers[i] = small[p_small];
        } else if (p_large-- > 0) {
            numbers[i] = large[p_large];
        } else {
            numbers[i] = both[i];
        }
    }

    if (both) {
        free(both);
        both = NULL;
    }

    *target = rand() % (MAX_TARGET - MIN_TARGET + 1) + MIN_TARGET;
}

struct solutions* get_last_solution(struct solutions* solutions) {
    struct solutions* cur;
    for (cur = solutions; cur->next; cur = cur->next);
    if (cur->inf) {
        cur->next = (struct solutions*)malloc(sizeof(struct solutions));
        cur->next->next = NULL;
        cur->next->inf = NULL;
        cur = cur->next;
    }

    return cur;
}

char solutions_are_equal(struct solution* sol1, struct solution* sol2) {
    unsigned int i, j;
    for (i = 0; sol1->groups[i].operand1[0]; i++) {
        if (!sol2->groups[i].operand1[0])
            return 0;

        for (j = 0; sol1->groups[i].operand1[j]; j++) {
            if (!sol2->groups[i].operand1[j])
                return 0;

            if (sol1->groups[i].operand1[j] != sol2->groups[i].operand1[j] || sol1->groups[i].operand2[j] != sol2->groups[i].operand2[j] ||
                sol1->groups[i].operation[j] != sol2->groups[i].operation[j])
                return 0;
        }

        if (sol2->groups[i].operand1[j])
            return 0;
    }

    if (sol2->groups[i].operand1[0])
        return 0;

    return 1;
}

char solution_already_exists(struct solution* sol, struct solutions* solutions) {
    for (; solutions; solutions = solutions->next) {
        if (solutions->inf && solutions_are_equal(solutions->inf, sol))
            return 1;
    }

    return 0;
}

void try_permutation(int* numbers, int target, struct solutions* solutions, struct solution* cur_solution, struct solution** closest_solution, unsigned int n, int result) {
    unsigned int i;
    int tmp;

    for (i = 0; i < OP_COUNT; i++) {
        // continue previous solution
        struct solution* cur = (struct solution*)malloc(sizeof(struct solution));
        enum ops op = (enum ops)i;

        if (cur_solution) {
            memcpy(cur, cur_solution, sizeof(struct solution));
        }

        tmp = result;
        switch (op) {
        case OP_ADD:
            tmp += numbers[n + 1];
            break;
        case OP_SUB:
            tmp -= numbers[n + 1];
            break;
        case OP_MUL:
            tmp *= numbers[n + 1];
            break;
        case OP_DIV:
            // fractions not allowed
            if (tmp % numbers[n + 1])
                return;
            tmp /= numbers[n + 1];
            break;
        }

        cur->groups[0].operand1[n] = result;
        cur->groups[0].operand2[n] = numbers[n + 1];
        cur->groups[0].operation[n] = op;
        cur->groups[0].operand1[n + 1] = 0;
        cur->groups[1].operand1[0] = 0;

        if (tmp == target) {
            struct solutions* sol;

            if (solution_already_exists(cur, solutions)) {
                free(cur);
                cur = NULL;
                return;
            }

            sol = get_last_solution(solutions);
            sol->inf = cur;

            return;
        } else {
            // not a perfect solution, but check if it's closer than any of the solutions we got so far
            if (!solutions->inf) {
                if (!*closest_solution) {
                    *closest_solution = (struct solution*)malloc(sizeof(struct solution));
                    memcpy(*closest_solution, cur, sizeof(struct solution));
                    (*closest_solution)->result = tmp;
                } else if (abs((*closest_solution)->result - target) > abs(tmp - target)) {
                    memcpy(*closest_solution, cur, sizeof(struct solution));
                    (*closest_solution)->result = tmp;
                }
            }
        }

        if (n < NUMBER_COUNT - 2) {
            try_permutation(numbers, target, solutions, cur, closest_solution, n + 1, tmp);
        } else {
            free(cur);
            cur = NULL;
        }
    }
}

void try_permutation_with_groups(int* numbers, int target, struct solutions* solutions, struct solution* cur_solution, struct solution** closest_solution, unsigned int n, int result,
    unsigned int* layout, unsigned int group) {
    unsigned int i;
    int tmp;

    for (i = 0; i < OP_COUNT; i++) {
        // continue previous solution
        struct solution* cur = (struct solution*)malloc(sizeof(struct solution));
        enum ops op = (enum ops)i;

        if (cur_solution) {
            memcpy(cur, cur_solution, sizeof(struct solution));
        }

        tmp = result;
        switch (op) {
        case OP_ADD:
            tmp += numbers[n + 1];
            break;
        case OP_SUB:
            tmp -= numbers[n + 1];
            break;
        case OP_MUL:
            tmp *= numbers[n + 1];
            break;
        case OP_DIV:
            // fractions not allowed
            if (tmp % numbers[n + 1])
                return;
            tmp /= numbers[n + 1];
            break;
        }

        cur->groups[0].operand1[n] = result;
        cur->groups[0].operand2[n] = numbers[n + 1];
        cur->groups[0].operation[n] = op;
        cur->groups[0].operand1[n + 1] = 0;
        cur->groups[1].operand1[0] = 0;

        if (tmp == target) {
            struct solutions* sol;

            if (solution_already_exists(cur, solutions)) {
                free(cur);
                cur = NULL;
                return;
            }

            sol = get_last_solution(solutions);
            sol->inf = cur;

            return;
        } else {
            // not a perfect solution, but check if it's closer than any of the solutions we got so far
            if (!solutions->inf) {
                if (!*closest_solution) {
                    *closest_solution = (struct solution*)malloc(sizeof(struct solution));
                    memcpy(*closest_solution, cur, sizeof(struct solution));
                    (*closest_solution)->result = tmp;
                } else if (abs((*closest_solution)->result - target) > abs(tmp - target)) {
                    memcpy(*closest_solution, cur, sizeof(struct solution));
                    (*closest_solution)->result = tmp;
                }
            }
        }

        if (n < NUMBER_COUNT - 2) {
            try_permutation(numbers, target, solutions, cur, closest_solution, n + 1, tmp);
        } else {
            free(cur);
            cur = NULL;
        }
    }
}

void solve(int* numbers, int target, struct solutions* solutions, struct solution** closest_solution, unsigned int** group_layouts, unsigned int n) {
    unsigned int i, j;
    int tmp;
    static char initial = 0;

    // TODO: implement parantheses
    if (!initial) {
        // try the initial sequence
        try_permutation(numbers, target, solutions, NULL, closest_solution, 0, numbers[0]);
        for (i = 0; group_layouts[i]; i++)
            try_permutation_with_groups(numbers, target, solutions, NULL, closest_solution, 0, numbers[0], group_layouts[i], 0);
        initial = 1;
    }

    // brute force complexity: (4^NUMBER_COUNT) * fact(NUMBER_COUNT)
    if (n < NUMBER_COUNT - 1) {
        solve(numbers, target, solutions, closest_solution, group_layouts, n + 1);
    }

    if (n > 0) {
        for (i = 0; i < NUMBER_COUNT - n; i++) {
            // nth will permute once, n-1th will permute 2 times, and so on
            tmp = numbers[n];
            numbers[n] = numbers[n - 1];
            numbers[n - 1] = tmp;

            try_permutation(numbers, target, solutions, NULL, closest_solution, 0, numbers[0]);
            for (j = 0; group_layouts[j]; j++)
                try_permutation_with_groups(numbers, target, solutions, NULL, closest_solution, 0, numbers[0], group_layouts[j], 0);

            if (n < NUMBER_COUNT - 1) {
                solve(numbers, target, solutions, closest_solution, group_layouts, n + 1);
            }
        }
    }
}

int calculate_result(int n1, enum ops op, int n2) {
    switch(op) {
    case OP_ADD:
    default:
        return n1 + n2;
        break;
    case OP_SUB:
        return n1 - n2;
        break;
    case OP_MUL:
        return n1 * n2;
        break;
    case OP_DIV:
        return n1 / n2;
        break;
    }
}

void print_solution(struct solution* solution) {
    unsigned int i;

    for (i = 0; solution->groups[0].operand1[i]; i++) {
        printf("%d %c %d = %d\n", solution->groups[0].operand1[i], operations[solution->groups[0].operation[i]], solution->groups[0].operand2[i],
            calculate_result(solution->groups[0].operand1[i], solution->groups[0].operation[i], solution->groups[0].operand2[i]));
    }
}

void print_solutions(struct solutions* solutions, struct solution* closest_solution) {
    unsigned int i = 0;

    if (!solutions->inf) {
        printf("No perfect solution was found.\n");
        printf("Closest result was %d. Solution below:\n", closest_solution->result);
        print_solution(closest_solution);
    } else {
        for (; solutions; solutions = solutions->next) {
            struct solution* sol =  solutions->inf;

            printf("\nSOLUTION #%d:\n", ++i);
            print_solution(sol);

            if (solutions->next) {
                printf("\nPress ENTER to see the next solution\n");
                getchar();
            }
        }
    }
}

void clean_up(struct solutions* solutions, struct solution* closest_solution, unsigned int** group_layouts) {
    char initial = 1;
    struct solutions* next = solutions->next;
    unsigned int i;

    for (; solutions; ) {
        if (solutions->inf) {
            free(solutions->inf);
            solutions->inf = NULL;
        }

        if (!initial) {
            next = solutions->next;
            free(solutions);
        } else
            initial = 0;

        solutions = next;
    }

    if (closest_solution) {
        free(closest_solution);
        closest_solution = NULL;
    }

    for (i = 0; group_layouts[i]; i++) {
        free(group_layouts[i]);
        group_layouts[i] = NULL;
    }
}

char layout_exists(unsigned int* layout, unsigned int** group_layouts, unsigned int idx) {
    unsigned int i, j, k;
    unsigned count[NUMBER_COUNT], count2[NUMBER_COUNT];

    for (i = 0; i < idx; i++) {
        char found = 1;

        for (k = 0; k < NUMBER_COUNT; k++) {
            count[k] = 0;
            count2[k] = 0;
        }

        for (j = 0; group_layouts[i][j]; j++) {
            if (!layout[j]) {
                found = 0;
                break;
            }

            count[group_layouts[i][j]]++;
            count2[layout[j]]++;
        }

        if (layout[j]) {
            found = 0;
        }

        if (!found) continue;

        for (k = 0; k < NUMBER_COUNT; k++) {
            if (count[k] != count2[k]) {
                found = 0;
                break;
            }
        }

        if (found)
            return 1;
    }

    return 0;
}

void get_group_layouts(unsigned int** group_layouts) {
    int i, j, k, n, idx = 0, last_count = 1, old_idx, lc, x1, x2;
    if (NUMBER_COUNT < 4) {
        group_layouts[0] = 0;
        return;
    }

    // set up the initial group layouts of size 2
    x1 = NUMBER_COUNT - 2;
    x2 = 2;
    group_layouts[idx] = (unsigned int*)malloc(sizeof(unsigned int) * 3);
    group_layouts[idx][0] = x1;
    group_layouts[idx][1] = x2;
    group_layouts[idx][2] = 0;

    while (--x1 >= ++x2 && x1 > 2) {
        idx++;
        group_layouts[idx] = (unsigned int*)malloc(sizeof(unsigned int) * 3);
        group_layouts[idx][0] = x1;
        group_layouts[idx][1] = x2;
        group_layouts[idx][2] = 0;
        last_count++;
    }

    // calculate group layouts of size 3 and onwards
    for (n = 3; n <= NUMBER_COUNT / 2; n++) {
        lc = last_count;
        last_count = 0;
        old_idx = idx;

        for (i = idx - lc + 1; i <= old_idx; i++) {
            // go through the group layout's elements
            for (j = 0; j < n - 1; j++) {
                unsigned int num = group_layouts[i][j];

                if (num > 3) {
                    unsigned int* layout = (unsigned int*)malloc(sizeof(unsigned int) * (n + 1));
                    char exists = 0;

                    for (k = 0; k < j; k++) layout[k] = group_layouts[i][k];

                    x1 = num - 2;
                    x2 = 2;

                    layout[j] = x1;
                    layout[j + 1] = x2;

                    for (k = j + 1; k < n - 1; k++) layout[k + 1] = group_layouts[i][k];
                    layout[n] = 0;

                    if (layout_exists(layout, group_layouts, idx + 1)) {
                        exists = 1;
                    }

                    while (--x1 >= ++x2 && x1 > 2) {
                        idx++;
                        group_layouts[idx] = (unsigned int*)malloc(sizeof(unsigned int) * (n + 1));
                        memcpy(group_layouts[idx], layout, sizeof(unsigned int) * (n + 1));
                        group_layouts[idx][j] = x1;
                        group_layouts[idx][j + 1] = x2;

                        if (layout_exists(group_layouts[idx], group_layouts, idx)) {
                           free(group_layouts[idx]);
                           group_layouts[idx] = 0;
                           idx--;
                        } else {
                           last_count++;
                        }
                    }

                    if (exists) {
                        free(layout);
                    } else {
                        last_count++;
                        group_layouts[++idx] = layout;
                    }
                }
            }

            // ex: (6,5) -> take first number: (4,2,5), (3,3,5)
            // take second number: (6,3,2)
        }
    }

    group_layouts[++idx] = 0;
}

int main() {
    unsigned int selection, i, n_large, n_small;
    int numbers[NUMBER_COUNT];
    int target;
    struct solutions solutions;
    struct solution* closest_solution = NULL;
    unsigned int* group_layouts[NUMBER_COUNT * NUMBER_COUNT];

    for (n_large = -1; large[++n_large]; );
    for (n_small = -1; small[++n_small]; );

    if (n_large + n_small < NUMBER_COUNT) {
        fprintf(stderr, "NUMBER_COUNT (%d) must be less than or equal to small + large number count (%d)\n", NUMBER_COUNT,
            n_large + n_small);
        exit(1);
    }

    get_group_layouts(group_layouts);

    select_numbers(&selection, n_large, n_small);
    srand(time(NULL));

    if (selection) {
        generate_game(selection, numbers, &target, n_large, n_small);
        printf("Target: %d, Numbers: ", target);

        for (i = 0; i < NUMBER_COUNT; i++)
            printf("%d ", numbers[i]);
    } else {
        printf("Target: ");
        scanf("%d", &target);
        printf("Enter %d numbers: ", NUMBER_COUNT);
        for (i = 0; i < NUMBER_COUNT; i++)
            scanf("%d", &numbers[i]);
    }

    printf("\nPress ENTER to see solutions\n");

    getchar();
    getchar();

    solutions.inf = NULL;
    solutions.next = NULL;
    solve(numbers, target, &solutions, &closest_solution, group_layouts, 0);

    print_solutions(&solutions, closest_solution);
    clean_up(&solutions, closest_solution, group_layouts);

    return 0;
}