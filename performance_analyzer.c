#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h> 
#define MAX_STUDENTS 100
#define NUM_SUBJECTS 3
#define MAX_NAME_LEN 50

struct Student {
    int rollNo;
    char name[MAX_NAME_LEN];
    int marks[NUM_SUBJECTS];
};

/* Function prototypes */
int getValidStudentCount(void);
int getValidRollNumber(void);
void getValidStudentName(char *name, size_t size);
void getValidMarksForStudent(int marks[]);
int isValidName(const char *name);
int isPositiveInt(int num);
void trimSpaces(char *str);

int calcTotal(int marks[]);
float calcAverage(int total);
char calcGrade(float avg);
void showStars(char grade);

void sortRollNumbers(int rollNumbers[], int n);
void printRollNumbersRecursive(int rollNumbers[], int index, int n);

int main(void) {
    int studentCount = getValidStudentCount();
    struct Student students[MAX_STUDENTS];
    int rollNumbers[MAX_STUDENTS];

    /* Student details input */
    for (int studentIndex = 0; studentIndex < studentCount; studentIndex++) {
        printf("\nEnter details for student %d\n", studentIndex + 1);

        students[studentIndex].rollNo = getValidRollNumber();
        getValidStudentName(students[studentIndex].name, sizeof(students[studentIndex].name));
        getValidMarksForStudent(students[studentIndex].marks);
    }

    /* Output section */
    printf("\n--- Student Report ---\n");
    for (int studentIndex = 0; studentIndex < studentCount; studentIndex++) {
        int total = calcTotal(students[studentIndex].marks);
        float avg = calcAverage(total);
        char grade = calcGrade(avg);

        printf("\nRoll No: %d\n", students[studentIndex].rollNo);
        printf("Name: %s\n", students[studentIndex].name);
        printf("Total: %d\n", total);
        printf("Average: %.2f\n", avg);
        printf("Grade: %c\n", grade);

        if (grade != 'F') {
            printf("Performance: ");
            showStars(grade);
            printf("\n");
        }
    }

    /* Sorting roll numbers and printing recursively */
    for (int i = 0; i < studentCount; i++)
        rollNumbers[i] = students[i].rollNo;

    sortRollNumbers(rollNumbers, studentCount);

    printf("\nSorted Roll Numbers: ");
    printRollNumbersRecursive(rollNumbers, 0, studentCount);
    printf("\n");

    return 0;
}

/* ---------- Input / validation helpers ---------- */

int getValidStudentCount(void) {
    int n;
    while (1) {
        printf("Enter number of students (1-%d): ", MAX_STUDENTS);
        if (scanf("%d", &n) != 1) {
            printf("Invalid input! Please enter an integer.\n");
            while (getchar() != '\n'); /* clear invalid input */
            continue;
        }
        if (n < 1 || n > MAX_STUDENTS) {
            printf("Number must be between 1 and %d.\n", MAX_STUDENTS);
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n'); /* consume trailing newline */
        return n;
    }
}

int getValidRollNumber(void) {
    int roll;
    while (1) {
        printf("Roll No: ");
        if (scanf("%d", &roll) != 1 || !isPositiveInt(roll)) {
            printf("Invalid roll number! Please enter a positive integer.\n");
            while (getchar() != '\n'); /* clear */
            continue;
        }
        while (getchar() != '\n'); /* consume newline */
        return roll;
    }
}

void getValidStudentName(char *name, size_t size) {
    while (1) {
        printf("Name: ");
        if (fgets(name, (int)size, stdin) == NULL) {
            /* fgets failure — try again */
            clearerr(stdin);
            continue;
        }
        trimSpaces(name);
        if (strlen(name) == 0) {
            printf("Name cannot be empty.\n");
            continue;
        }
        if (!isValidName(name)) {
            printf("Invalid name! Use letters and spaces only.\n");
            continue;
        }
        return;
    }
}

void getValidMarksForStudent(int marks[]) {
    for (int subjectIndex = 0; subjectIndex < NUM_SUBJECTS; subjectIndex++) {
        while (1) {
            printf("Marks %d (0-100): ", subjectIndex + 1);
            if (scanf("%d", &marks[subjectIndex]) != 1) {
                printf("Invalid input. Enter integer between 0 and 100.\n");
                while (getchar() != '\n');
                continue;
            }
            if (marks[subjectIndex] < 0 || marks[subjectIndex] > 100) {
                printf("Marks must be 0–100.\n");
                while (getchar() != '\n');
                continue;
            }
            while (getchar() != '\n'); /* consume newline */
            break;
        }
    }
}


int isValidName(const char *name) {
    for (int i = 0; name[i] != '\0'; i++) {
        if (!isalpha((unsigned char)name[i]) && name[i] != ' ')
            return 0;
    }
    return 1;
}

int isPositiveInt(int num) {
    return num > 0;
}


void trimSpaces(char *str) {
    if (str == NULL) return;

    int start = 0;
    int end = (int)strlen(str) - 1;

  
    if (end >= 0 && str[end] == '\n') {
        str[end] = '\0';
        end--;
    }

    
    while (end >= 0 && isspace((unsigned char)str[end])) {
        str[end] = '\0';
        end--;
    }

    
    while (str[start] != '\0' && isspace((unsigned char)str[start])) {
        start++;
    }

    if (start > 0) {
        memmove(str, str + start, strlen(str + start) + 1);
    }
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
        default: stars = 0; break;
    }
    for (int i = 0; i < stars; i++) putchar('*');
}


void sortRollNumbers(int rollNumbers[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (rollNumbers[i] > rollNumbers[j]) {
                int tmp = rollNumbers[i];
                rollNumbers[i] = rollNumbers[j];
                rollNumbers[j] = tmp;
            }
        }
    }
}

void printRollNumbersRecursive(int rollNumbers[], int index, int n) {
    if (index == n) return;
    printf("%d ", rollNumbers[index]);
    printRollNumbersRecursive(rollNumbers, index + 1, n);
}
