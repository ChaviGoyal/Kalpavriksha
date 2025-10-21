#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---------- Function Prototypes ---------- */
int askMatrixSize();
void fillMatrixWithRandomValues(int *matrix, int size);
void printMatrix(int *matrix, int size, const char *title);
void rotateMatrix90Clockwise(int *matrix, int size);
void smoothMatrix3x3(int *matrix, int size);

/* ---------- Main Function ---------- */
int main() {
    int size = askMatrixSize();

    int sonarMatrix[size][size];
    int *ptr = &sonarMatrix[0][0];

    fillMatrixWithRandomValues(ptr, size);
    printMatrix(ptr, size, "\nOriginal Randomly Generated Matrix:");

    rotateMatrix90Clockwise(ptr, size);
    printMatrix(ptr, size, "\nMatrix after 90° Clockwise Rotation:");

    smoothMatrix3x3(ptr, size);
    printMatrix(ptr, size, "\nMatrix after Applying 3×3 Smoothing Filter:");

    return 0;
}



// Ask user for matrix size (integer between 2 and 10)
int askMatrixSize() {
    int size;
    printf("Enter matrix size (2 to 10): ");
    while (1) {
        if (scanf("%d", &size) != 1) {
            printf("Invalid input! Please enter a number: ");
            while (getchar() != '\n'); // clear invalid input
            continue;
        }
        if (size < 2 || size > 10) {
            printf("Matrix size must be between 2 and 10. Try again: ");
            continue;
        }
        break;
    }
    return size;
}

// Fill matrix with random numbers between 0 and 255
void fillMatrixWithRandomValues(int *matrix, int size) {
    srand(time(NULL));
    for (int index = 0; index < size * size; index++) {
        *(matrix + index) = rand() % 256;
    }
}


void printMatrix(int *matrix, int size, const char *title) {
    printf("%s\n", title);
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            printf("%4d", *(matrix + row * size + col));
        }
        printf("\n");
    }
}

// Rotate matrix 90° clockwise in-place using pointers
void rotateMatrix90Clockwise(int *matrix, int size) {
    for (int layer = 0; layer < size / 2; layer++) {
        for (int element = layer; element < size - layer - 1; element++) {
            int topValue = *(matrix + layer * size + element);

            // Move left to top
            *(matrix + layer * size + element) = *(matrix + (size - element - 1) * size + layer);

            // Move bottom to left
            *(matrix + (size - element - 1) * size + layer) = *(matrix + (size - layer - 1) * size + (size - element - 1));

            // Move right to bottom
            *(matrix + (size - layer - 1) * size + (size - element - 1)) = *(matrix + element * size + (size - layer - 1));

            // Move top (saved) to right
            *(matrix + element * size + (size - layer - 1)) = topValue;
        }
    }
}

// Apply 3x3 smoothing filter in-place
void smoothMatrix3x3(int *matrix, int size) {
    int smoothed[size * size]; // temporary buffer

    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            int sum = 0;
            int neighbors = 0;

            for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
                for (int colOffset = -1; colOffset <= 1; colOffset++) {
                    int neighborRow = row + rowOffset;
                    int neighborCol = col + colOffset;

                    if (neighborRow >= 0 && neighborRow < size && neighborCol >= 0 && neighborCol < size) {
                        sum += *(matrix + neighborRow * size + neighborCol);
                        neighbors++;
                    }
                }
            }

            *(smoothed + row * size + col) = sum / neighbors;
        }
    }

    // Copy smoothed values back to original matrix
    for (int i = 0; i < size * size; i++) {
        *(matrix + i) = *(smoothed + i);
    }
}
