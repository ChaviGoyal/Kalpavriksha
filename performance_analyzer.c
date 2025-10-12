#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_STUDENTS 100
#define NUM_SUBJECTS 3

struct Student {
    int rollNo;
    char name[50];
    int marks[NUM_SUBJECTS];
};

int calcTotal(int marks[]);
float calcAverage(int total);
char calcGrade(float avg);
void showStars(char grade);
void printRollRecursive(int rolls[], int index, int n);
void trimSpaces(char *str);
void sortRolls(int rolls[], int n);
int isValidName(char *name);
int isPositiveInt(int num);

int isValidName(char *name) {
    for (int i = 0; name[i] != '\0'; i++) {
        if (!isalpha(name[i]) && name[i] != ' ')
            return 0;
    }
    return 1;
}

int isPositiveInt(int num) {
    return num > 0;
}

int main(void) {
    int n;
    struct Student students[MAX_STUDENTS];
    int rolls[MAX_STUDENTS];

    // number of students input
    while (1) {
        printf("Enter number of students (1-100): ");
        if (scanf("%d", &n) != 1) {
            printf("Invalid input! Please enter an integer.\n");
            while (getchar() != '\n');
            continue;
        }
        if (n < 1 || n > 100) {
            printf("Number must be between 1 and 100.\n");
            continue;
        }
        getchar();
        break;
    }

    // student details input
    for (int i = 0; i < n; i++) {
        printf("\nEnter details for student %d\n", i + 1);

        while (1) {
            printf("Roll No: ");
            if (scanf("%d", &students[i].rollNo) != 1 || !isPositiveInt(students[i].rollNo)) {
                printf("Invalid roll number.\n");
                while (getchar() != '\n');
                continue;
            }
            getchar();
            break;
        }

        while (1) {
            printf("Name: ");
            fgets(students[i].name, sizeof(students[i].name), stdin);
            trimSpaces(students[i].name);
            if (isValidName(students[i].name) && strlen(students[i].name) > 0)
                break;
            printf("Invalid name! Use letters and spaces only.\n");
        }

        // marks for 3 subjects
        for (int j = 0; j < NUM_SUBJECTS; j++) {
            while (1) {
                printf("Marks %d (0-100): ", j + 1);
                if (scanf("%d", &students[i].marks[j]) != 1) {
                    printf("Invalid input.\n");
                    while (getchar() != '\n');
                    continue;
                }
                if (students[i].marks[j] < 0 || students[i].marks[j] > 100) {
                    printf("Marks must be 0–100.\n");
                    continue;
                }
                getchar();
                break;
            }
        }
    }

    // output section
    printf("\n--- Student Report ---\n");
    for (int i = 0; i < n; i++) {
        int total = calcTotal(students[i].marks);
        float avg = calcAverage(total);
        char grade = calcGrade(avg);

        printf("\nRoll No: %d\n", students[i].rollNo);
        printf("Name: %s\n", students[i].name);
        printf("Total: %d\n", total);
        printf("Average: %.2f\n", avg);
        printf("Grade: %c\n", grade);

        if (grade != 'F') {
            printf("Performance: ");
            showStars(grade);
            printf("\n");
        }
    }

    // sorting roll numbers and printing recursively
    for (int i = 0; i < n; i++)
        rolls[i] = students[i].rollNo;

    sortRolls(rolls, n);

    printf("\nSorted Roll Numbers: ");
    printRollRecursive(rolls, 0, n);
    printf("\n");

    return 0;
}

int calcTotal(int marks[]) {
    int total = 0;
    for (int i = 0; i < NUM_SUBJECTS; i++)
        total += marks[i];
    return total;
}

float calcAverage(int total) {
    return (float)total / NUM_SUBJECTS;
}

char calcGrade(float avg) {
    if (avg >= 85) return 'A';
    else if (avg >= 70) return 'B';
    else if (avg >= 50) return 'C';
    else if (avg >= 35) return 'D';
    else return 'F';
}

void showStars(char grade) {
    int stars = 0;
    switch (grade) {
        case 'A': stars = 5; break;
        case 'B': stars = 4; break;
        case 'C': stars = 3; break;
        case 'D': stars = 2; break;
    }
    for (int i = 0; i < stars; i++) printf("*");
}

void printRollRecursive(int rolls[], int index, int n) {
    if (index == n) return;
    printf("%d ", rolls[index]);
    printRollRecursive(rolls, index + 1, n);
}

void trimSpaces(char *str) {
    int start = 0, end;
    int len;
    if (!str) return;
    len = strlen(str);

    end = len - 1;
    while (end >= 0 && isspace((unsigned char)str[end])) {
        str[end] = '\0';
        end--;
    }

    while (str[start] != '\0' && isspace((unsigned char)str[start])) {
        start++;
    }

    if (start > 0)
        memmove(str, str + start, strlen(str + start) + 1);
}

void sortRolls(int rolls[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (rolls[i] > rolls[j]) {
                int tmp = rolls[i];
                rolls[i] = rolls[j];
                rolls[j] = tmp;
            }
        }
    }
}
