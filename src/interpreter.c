#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// global options
#define STACK_SIZE 65536
#define HEAP_SIZE 524288

// lexical tokens
#define SPACE ' '
#define TAB '\t'
#define LINEFEED '\n'

struct tokenizer_context {
    int line;
    int col;
};

struct tokenizer_context context = {1, 1};

int heap[HEAP_SIZE] = {};

struct stack_impl {
    int maxsize;
    int top;
    int* items;
};

struct stack_impl stack = {.maxsize = STACK_SIZE};

struct stack_impl* newStack(int capacity) {
    struct stack_impl* pt = (struct stack_impl*)malloc(sizeof(struct stack_impl));

    pt->maxsize = capacity;
    pt->top = -1;
    pt->items = (int*)malloc(sizeof(int) * capacity);

    return pt;
}

int size(struct stack_impl* pt) { return pt->top + 1; }

int isEmpty(struct stack_impl* pt) { return pt->top == -1; }

int isFull(struct stack_impl* pt) { return pt->top == pt->maxsize - 1; }

void push(struct stack_impl* pt, int x) {
    if (isFull(pt)) {
        printf("Overflow\nProgram Terminated\n");
        exit(EXIT_FAILURE);
    }

    printf("Inserting %d\n", x);

    pt->items[++pt->top] = x;
}

int peek(struct stack_impl* pt) {
    if (!isEmpty(pt)) {
        return pt->items[pt->top];
    } else {
        exit(EXIT_FAILURE);
    }
}

int pop(struct stack_impl* pt) {
    if (isEmpty(pt)) {
        printf("Underflow\nProgram Terminated\n");
        exit(EXIT_FAILURE);
    }

    printf("Removing %d\n", peek(pt));

    return pt->items[pt->top--];
}

void error() {
    printf("Unexpected token on line [%d], character [%d]", context.line, context.col);
    exit(EXIT_FAILURE);
}

int next_char() {
    int c = getchar();
    if (c == '\n') {
        context.line += 1;
        context.col = 1;
    } else {
        context.col += 1;
    }
    return getchar();
}

int number() {
    int c;
    int first = true;
    int positive = true;
    int finished = false;
    int number = 0;
    // TODO: handle integer buffer overflow
    while ((c = next_char()) != EOF) {
        switch (c) {
        case SPACE:
            number = number << 1;
            break;
        case TAB:
            if (first) {
                positive = false;
                first = false;
            }
            number = (number << 1) | 1;
            break;
        case LINEFEED:
            finished = true;
            break;
        default:
            error();
            break;
        }
        if (finished) {
            if (!positive) {
                number *= -1;
            }
            return number;
        }
    }
    return 0;
}

void push_stack() { push(&stack, number()); }

void duplicate_stack() { push(&stack, peek(&stack)); }

void swap_stack() {
    int top = pop(&stack);
    int bot = pop(&stack);
    push(&stack, top);
    push(&stack, bot);
}

void discard_stack() { pop(&stack); }

void stack_manipulation() {
    int c;
    while ((c = next_char()) != EOF) {
        switch (c) {
        case SPACE:
            push_stack();
            return;
        case LINEFEED:
            c = next_char();
            switch (c) {
            case SPACE:
                duplicate_stack();
                break;
            case TAB:
                swap_stack();
                break;
            case LINEFEED:
                discard_stack();
                break;
            default:
                error();
                break;
            }
            return;
        case TAB:
        default:
            error();
            return;
        }
    }
}

void addition() {
    int right = pop(&stack);
    int left = pop(&stack);
    push(&stack, left + right);
}

void subtraction() {
    int right = pop(&stack);
    int left = pop(&stack);
    push(&stack, left - right);
}

void multiplication() {
    int right = pop(&stack);
    int left = pop(&stack);
    push(&stack, left * right);
}

void int_division() {
    int right = pop(&stack);
    int left = pop(&stack);
    push(&stack, left / right);
}

void modulo() {
    int right = pop(&stack);
    int left = pop(&stack);
    push(&stack, left % right);
}

void arithmetic() {
    int c;
    while ((c = next_char()) != EOF) {
        switch (c) {
        case SPACE:
            c = next_char();
            switch (c) {
            case SPACE:
                addition();
                break;
            case TAB:
                subtraction();
                break;
            case LINEFEED:
                multiplication();
                break;
            default:
                error();
                break;
            }
            return;
        case TAB:
            c = next_char();
            switch (c) {
            case SPACE:
                int_division();
                break;
            case TAB:
                modulo();
                break;
            case LINEFEED:
            default:
                error();
                break;
            }
            return;
        case LINEFEED:
        default:
            error();
            return;
        }
    }
}

void store() {
    int val = pop(&stack);
    int addr = pop(&stack);
    heap[addr] = val;
}

void retrieve() {
    int addr = pop(&stack);
    push(&stack, heap[addr]);
}

void heap_access() {
    int c;
    while ((c = next_char()) != EOF) {
        switch (c) {
        case SPACE:
            store();
            return;
        case TAB:
            retrieve();
            return;
        case LINEFEED:
        default:
            error();
            return;
        }
    }
}

int label() {
    int c;
    int finished = false;
    int number = 0;
    // TODO: handle integer buffer overflow
    while ((c = next_char()) != EOF) {
        switch (c) {
        case SPACE:
            number = number << 1;
            break;
        case TAB:
            number = (number << 1) | 1;
            break;
        case LINEFEED:
            finished = true;
            break;
        default:
            error();
            break;
        }
        if (finished) {
            return number;
        }
    }
    return 0;
}

// TODO: introduce some sort of call stack
void mark() {
    // TODO: find a solution to count bytes to unread until label
}

void call() {}

void jump() {}

void jump_zero() {}

void jump_negative() {}

void return_to_caller() {}

void end() {
    printf("Whitespace-Routine finished");
    exit(EXIT_SUCCESS);
}

void flow_control() {
    int c;
    while ((c = next_char()) != EOF) {
        switch (c) {
        case SPACE:
            c = next_char();
            switch (c) {
            case SPACE:
                mark();
                break;
            case TAB:
                call();
                break;
            case LINEFEED:
                jump();
                break;
            default:
                error();
                break;
            }
            return;
        case TAB:
            c = next_char();
            switch (c) {
            case SPACE:
                jump_zero();
                break;
            case TAB:
                jump_negative();
                break;
            case LINEFEED:
                return_to_caller();
                break;
            default:
                error();
                break;
            }
            return;
        case LINEFEED:
            c = next_char();
            switch (c) {
            case LINEFEED:
                end();
                break;
            case SPACE:
            case TAB:
            default:
                error();
                return;
            }
            return;
        default:
            error();
            return;
        }
    }
}

void output_char() {
    int addr = pop(&stack);
    printf("%d", heap[addr]);
}

void output_number() { output_char(); }

void input_char() {
    int c = getchar();
    push(&stack, c);
}

void input_number() { input_char(); }

void io() {
    int c;
    while ((c = next_char()) != EOF) {
        switch (c) {
        case SPACE:
            c = next_char();
            switch (c) {
            case SPACE:
                output_char();
                break;
            case TAB:
                output_number();
                break;
            case LINEFEED:
            default:
                error();
                break;
            }
            return;
        case TAB:
            c = next_char();
            switch (c) {
            case SPACE:
                input_char();
                break;
            case TAB:
                input_number();
                break;
            case LINEFEED:
            default:
                error();
                break;
            }
            return;
        case LINEFEED:
        default:
            error();
            return;
        }
    }
}

void interpret() {
    int c;
    while ((c = next_char()) != EOF) {
        switch (c) {
        case SPACE:
            stack_manipulation();
            break;
        case TAB:
            c = next_char();
            switch (c) {
            case SPACE:
                arithmetic();
                break;
            case TAB:
                heap_access();
                break;
            case LINEFEED:
                io();
                break;
            default:
                error();
                return;
            }
            break;
        case LINEFEED:
            flow_control();
            break;
        default:
            error();
            return;
        }
    }
}

int main() {
    interpret();
    return 0;
}
