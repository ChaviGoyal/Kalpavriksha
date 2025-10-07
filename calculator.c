#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define SIZE 100

int isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/');
}

int checkOverflow(long long value, int *err) {
    if (value > INT_MAX || value < INT_MIN) {
        *err = 3; //integer overflow
        return 0;
    }
    return (int)value;
}

int solve(char *exp, int *err) {
    int nums[SIZE], numCount = 0;
    char operatorList[SIZE];
    int opCount = 0;
    int i = 0, j = 0, k;
    char last = '\0';

    while (exp[i] != '\0') {
        if (exp[i] == ' ') { 
		i++; 
		continue; 
		}

        if (isdigit(exp[i])) {
            long long x = 0;
            while (isdigit(exp[i])) {
                x = x * 10 + (exp[i] - '0');
                if (x > INT_MAX) {
				*err = 3; 
				return 0; 
				} 
                i++;
            }
            nums[numCount++] = (int)x;
            last = 'n';
        }

        else if (isOperator(exp[i])) {
            int look = i + 1;
            while (exp[look] == ' ') 
			look++;
            if (exp[look] == '\0' || !isdigit(exp[look])) {
                *err = 2; 
				return 0;
            }
            operatorList[opCount++] = exp[i];
            last = exp[i];
            i++;
        }

        else {
            *err = 2;
            return 0;
        }
    }

    if (last != 'n') { 
	*err = 2;
	return 0;
	}
	
    // Handle * and /
    while (j < opCount) {
        if (operatorList[j] == '*' || operatorList[j] == '/') {
            long long temp;
            if (operatorList[j] == '/') {
                if (nums[j + 1] == 0) { 
				*err = 1;
				return 0; 
				}
                if (nums[j] == INT_MIN && nums[j + 1] == -1) { 
				*err = 3;
				return 0;
				}
                temp = (long long)nums[j] / nums[j + 1];
            } 
			else {
                temp = (long long)nums[j] * nums[j + 1];
            }
            nums[j] = checkOverflow(temp, err);
            if (*err != 0) 
			return 0;

            for (k = j + 1; k < numCount - 1; k++) 
			nums[k] = nums[k + 1];
            for (k = j; k < opCount - 1; k++) 
			operatorList[k] = operatorList[k + 1];
            numCount--;
            opCount--;
        } 
		else j++;
    }

    // Handle + and -
    long long ans = nums[0];
    for (j = 0; j < opCount; j++) {
        if (operatorList[j] == '+')
            ans += nums[j + 1];
        else
            ans -= nums[j + 1];

        if (ans > INT_MAX || ans < INT_MIN) { 
		*err = 3; 
	    return 0; 
		}
    }

    return (int)ans;
}

int main() {
    char exp[SIZE];
    printf("Enter expression: ");
    fgets(exp, SIZE, stdin);
    exp[strcspn(exp, "\n")] = '\0';

    int err = 0;
    int res = solve(exp, &err);

    if (err == 1) {
        printf("Error: Division is by zero.\n");
    } else if (err == 2) {
        printf("Error: Invalid expression.\n");
    } else if (err == 3) {
        printf("Error: Integer overflow.\n");
    } else {
        printf("%d\n", res);
    }
    return 0;
}
