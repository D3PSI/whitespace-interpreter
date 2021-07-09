#include "map.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// global options
#define STACK_SIZE 65536
#define HEAP_SIZE 524288

#define BUF_SIZE 1024

// lexical tokens
#define SPACE ' '
#define TAB '\t'
#define LINEFEED '\n'

#define NULL_TERM '\0'

char* source;
map_int_t map_label;

struct tokenizer_context {
    int traversed_buf_size;
    int line;
    int col;
};

struct tokenizer_context context = {0, 1, 1};

int* heap;

struct stack_impl {
    int maxsize;
    int top;
    int* items;
};

struct stack_impl* stack;

struct stack_impl* new_stack(int capacity) {
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

    // printf("Inserting %d\n", x);

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

    // printf("Removing %d\n", peek(pt));

    return pt->items[pt->top--];
}

void error() {
    printf("Unexpected token on line [%d], character [%d]", context.line, context.col);
    exit(EXIT_FAILURE);
}

char next_char() {
    char c = source[context.traversed_buf_size++];
    if (c == '\n') {
        context.line += 1;
        context.col = 1;
    } else {
        context.col += 1;
    }
    return c;
}

int number() {
    char c;
    int first = true;
    int positive = false;
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
                positive = true;
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

void push_stack() { push(stack, number()); }

void duplicate_stack() { push(stack, peek(stack)); }

void swap_stack() {
    int top = pop(stack);
    int bot = pop(stack);
    push(stack, top);
    push(stack, bot);
}

void discard_stack() { pop(stack); }

void stack_manipulation() {
    char c;
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
    int right = pop(stack);
    int left = pop(stack);
    push(stack, left + right);
}

void subtraction() {
    int right = pop(stack);
    int left = pop(stack);
    push(stack, left - right);
}

void multiplication() {
    int right = pop(stack);
    int left = pop(stack);
    push(stack, left * right);
}

void int_division() {
    int right = pop(stack);
    int left = pop(stack);
    push(stack, left / right);
}

void modulo() {
    int right = pop(stack);
    int left = pop(stack);
    push(stack, left % right);
}

void arithmetic() {
    char c;
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
    int val = pop(stack);
    int addr = pop(stack);
    heap[addr] = val;
}

void retrieve() {
    int addr = pop(stack);
    push(stack, heap[addr]);
}

void heap_access() {
    char c;
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
    char c;
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

void mark() { map_set(&map_label, label(), context.traversed_buf_size); }

void call() {
    int l = label();
    int pos = map_get(&map_label, l);
    if (pos == NULL) {
        printf("Label not defined [%d]", l);
        exit(EXIT_FAILURE);
    }
    push(stack, context.traversed_buf_size);
    push(stack, l);
    context.traversed_buf_size = pos;
}

void jump() {
    int l = label();
    int pos = map_get(&map_label, l);
    if (pos == NULL) {
        printf("Label not defined [%d]", l);
        exit(EXIT_FAILURE);
    }
    context.traversed_buf_size = pos;
}

void jump_zero() {
    int l = label();
    int pos = map_get(&map_label, l);
    if (pos == NULL) {
        printf("Label not defined [%d]", l);
        exit(EXIT_FAILURE);
    }
    if (peek(stack) == 0) {
        context.traversed_buf_size = pos;
    }
}

void jump_negative() {
    int l = label();
    int pos = map_get(&map_label, l);
    if (pos == NULL) {
        printf("Label not defined [%d]", l);
        exit(EXIT_FAILURE);
    }
    if (peek(stack) < 0) {
        context.traversed_buf_size = pos;
    }
}

void return_to_caller() {
    int top = pop(stack);
    while (map_get(&map_label, top) == NULL) {
        top = pop(stack);
    }
    top = pop(stack);
    context.traversed_buf_size = top;
}

void end() {
    printf("\n\nWhitespace-Routine finished\n");
    exit(EXIT_SUCCESS);
}

void flow_control() {
    char c;
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
            case NULL_TERM:
            case LINEFEED:
                end();
                return;
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

void output_char() { printf("%c", pop(stack)); }

void output_number() { output_char(); }

void input_char() { push(stack, getchar()); }

void input_number() { input_char(); }

void io() {
    char c;
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
    heap = malloc(HEAP_SIZE);
    stack = new_stack(STACK_SIZE);
    map_init(&map_label);
    char c;
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
        case NULL_TERM:
            return;
        default:
            error();
            return;
        }
    }
}

void read() {
    char buffer[BUF_SIZE];
    size_t contentSize = 1;
    char* content = malloc(sizeof(char) * BUF_SIZE);
    if (content == NULL) {
        perror("Failed to allocate content");
        exit(1);
    }
    content[0] = '\0';
    while (fgets(buffer, BUF_SIZE, stdin)) {
        char* old = content;
        contentSize += strlen(buffer);
        content = realloc(content, contentSize);
        if (content == NULL) {
            perror("Failed to reallocate content");
            free(old);
            exit(2);
        }
        strcat(content, buffer);
    }

    if (ferror(stdin)) {
        free(content);
        perror("Error reading from stdin.");
        exit(3);
    }

    source = content;
}

void clean() {
    free(heap);
    free(stack->items);
    free(source);
    map_deinit(&map_label);
}

int main() {
    read();
    interpret();
    clean();
    return 0;
}
