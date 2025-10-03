#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct User {
    int id;
    char name[50];
    int age;
};

// CRUD functions
void addUser();
void displayUsers();
void updateUser();
void deleteUser();

int main() {
    int choice;

    while (1) {
        printf("\n--- Simple User Manager ---\n");
        printf("1. Add User\n");
        printf("2. Display All Users\n");
        printf("3. Update a User\n");
        printf("4. Delete a User\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addUser();
                break;
            case 2:
                displayUsers();
                break;
            case 3:
                updateUser();
                break;
            case 4:
                deleteUser();
                break;
            case 5:
                printf("Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}

void addUser() {        // this will add a new user
    struct User userToAdd;
    FILE *fptr;

    fptr = fopen("users.txt", "a");
    if (fptr == NULL) {
        printf("Error: Cannot open file.\n");
        return;
    }

    printf("Enter User ID: ");
    scanf("%d", &userToAdd.id);
    printf("Enter Name: ");
    scanf(" %[^\n]s", userToAdd.name);
    printf("Enter Age: ");
    scanf("%d", &userToAdd.age);
    
    for (int i = 0; i < strlen(userToAdd.name); i++) {
        if (userToAdd.name[i] == ' ') {
            userToAdd.name[i] = '_';
        }
    }

    fprintf(fptr, "%d %s %d\n", userToAdd.id, userToAdd.name, userToAdd.age);

    fclose(fptr);
    printf("User added successfully!\n");
}


void displayUsers() {
    struct User record;
    FILE *fptr;

    fptr = fopen("users.txt", "r");
    if (fptr == NULL) {
        printf("No users found. Please add a user first.\n");
        return;
    }

    printf("\n--- User List ---\n");
    printf("ID\tName\t\tAge\n");
    printf("---------------------------\n");

    while (fscanf(fptr, "%d %s %d", &record.id, record.name, &record.age) != EOF) {
        // before diplaying, i am converting underscores with spaces 
        for (int i = 0; i < strlen(record.name); i++) {
            if (record.name[i] == '_') {
                record.name[i] = ' ';
            }
        }
        printf("%d\t%s\t\t%d\n", record.id, record.name, record.age);
    }

    fclose(fptr);
}


void updateUser() {         // this will update a specific user record
    int updateID;
    int recordFound = 0;
    struct User record;
    FILE *oldFile, *tempFile; 

    printf("Enter the ID of the user to update: ");
    scanf("%d", &updateID);

    oldFile = fopen("users.txt", "r");
    tempFile = fopen("temp.txt", "w");

    if (oldFile == NULL) {
        printf("No users to update.\n");
        if (tempFile) fclose(tempFile);
        remove("temp.txt");
        return;
    }

    while (fscanf(oldFile, "%d %s %d", &record.id, record.name, &record.age) != EOF) {
        if (record.id == updateID) {
            recordFound = 1;
            printf("Enter new Name: ");
            scanf(" %[^\n]s", record.name);
            printf("Enter new Age: ");
            scanf("%d", &record.age);
        }
        fprintf(tempFile, "%d %s %d\n", record.id, record.name, record.age);
    }

    fclose(oldFile);
    fclose(tempFile);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (recordFound) {
        printf("User updated successfully!\n");
    } else {
        printf("User ID not found.\n");
    }
}


void deleteUser() {         // this will delete the specific user record
    int deleteID;
    int recordFound = 0;
    struct User record;
    FILE *oldFile, *tempFile;

    printf("Enter the ID of the user to delete: ");
    scanf("%d", &deleteID);

    oldFile = fopen("users.txt", "r");
    tempFile = fopen("temp.txt", "w");

    if (oldFile == NULL) {
        printf("No users to delete.\n");
        if (tempFile) fclose(tempFile);
        remove("temp.txt");
        return;
    }

    while (fscanf(oldFile, "%d %s %d", &record.id, record.name, &record.age) != EOF) {
        if (record.id != deleteID) {
            fprintf(tempFile, "%d %s %d\n", record.id, record.name, record.age);
        } else {
            recordFound = 1;
        }
    }

    fclose(oldFile);
    fclose(tempFile);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (recordFound) {
        printf("User deleted successfully!\n");
    } else {
        printf("User ID not found.\n");
    }
}
