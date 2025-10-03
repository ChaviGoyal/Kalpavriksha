#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int value_stack[256];
char op_stack[256];
int num_top = -1, op_top = -1;

int get_op_priority(char op) {
    return (op == '*' || op == '/') ? 2 : 1;
}

void calculate(char* expr) {
    char* p = expr;
    int error_flag  = 0; // 0=OK, 1=Invalid, 2=Div by Zero

    while (*p && !error_flag ) {
        if (isspace((char)*p)) {
            p++; // Skipping whitespace
            continue;
        }

        // If current letter is a number, parse and push to the value_stack stack
        if (isdigit((char)*p)) {
            value_stack[++num_top] = strtol(p, (char**)&p, 10);
            continue;
        }

        while (op_top > -1 && get_op_priority(op_stack[op_top]) >= get_op_priority(*p)) {
            if (num_top < 1) { error_flag  = 1; break; }
            int val2 = value_stack[num_top--];
            int val1 = value_stack[num_top--];
            char op = op_stack[op_top--];

            if (op == '/' && val2 == 0) { error_flag  = 2; break; }
            if (op == '+') value_stack[++num_top] = val1 + val2;
            if (op == '-') value_stack[++num_top] = val1 - val2;
            if (op == '*') value_stack[++num_top] = val1 * val2;
            if (op == '/') value_stack[++num_top] = val1 / val2;
        }

        if (strchr("+-*/", *p)) {
            op_stack[++op_top] = *p++;
        } else {
            error_flag  = 1; // Invalid character
        }
    }

    while (op_top > -1 && !error_flag ) {
        if (num_top < 1) { error_flag  = 1; break; }
        int val2 = value_stack[num_top--];
        int val1 = value_stack[num_top--];
        char op = op_stack[op_top--];
        
        if (op == '/' && val2 == 0) { error_flag  = 2; break; }
        if (op == '+') value_stack[++num_top] = val1 + val2;
        if (op == '-') value_stack[++num_top] = val1 - val2;
        if (op == '*') value_stack[++num_top] = val1 * val2;
        if (op == '/') value_stack[++num_top] = val1 / val2;
    }

    if (error_flag  == 1 || num_top != 0 || op_top != -1) {
        printf("Error: Invalid expression.\n");
    } else if (error_flag  == 2) {
        printf("Error: Division by zero.\n");
    } else {
        printf("%d\n", value_stack[0]);
    }
}



int main() {
    char userInput[1024];
    if (fgets(userInput, sizeof(userInput), stdin)) {
        int len = strlen(userInput);
        if (len > 0 && userInput[len - 1] == '\n') {
            userInput[len - 1] = '\0';
        }

        calculate(userInput);
    }
    
    return 0;
}
