#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME "users.txt"

typedef struct {
    int id;
    char name[50];
    int age;
} User;

void create();
void read();
void update();
void Userdelete();

int main() {
    int choice;

    while(1) {
        printf("1. Add User\n");
        printf("2. Show Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1: create(); break;
            case 2: read(); break;
            case 3: update(); break;
            case 4: Userdelete(); break;
            case 5: exit(0);
            default: printf("Invalid choice!\n");
        }
    }
}

// To Create
void create() {
    FILE *f = fopen(FILENAME, "a");
    User u;

    printf("Enter ID: ");
    scanf("%d",&u.id);
    
    printf("Enter Name:");
    scanf("%s",u.name);
    
    printf("Enter Age:");
    scanf("%d", &u.age);

    fprintf(f, "%d %s %d\n", u.id, u.name, u.age);
    fclose(f);
    printf("User added!\n");
}

// To Read
void read() {
    FILE *f = fopen(FILENAME, "r");
    User u;

    if (!f) { printf("No users found.\n"); 
	return; 
	}

    printf("\nThis is the data of users stored: \n");
    while (fscanf(f, "%d %s %d", &u.id, u.name, &u.age) != EOF)
        printf("ID: %d | Name: %s | Age: %d\n", u.id, u.name, u.age);

    fclose(f);
}

// To Update
void update() {
    FILE *f = fopen(FILENAME, "r");
    FILE *t = fopen("temp.txt", "w");
    User u;
    int id;
	int found = 0;

    if (!f) { printf("No users found.\n"); return; }

    printf("Enter ID to update: ");
    scanf("%d", &id);

    while(fscanf(f, "%d %s %d", &u.id, u.name, &u.age) != EOF) {
        if (u.id == id) {
            printf("Enter new Name: ");
            scanf("%s %d", u.name);
            printf("Enter new Age:");
            scanf("%d",&u.age);
            found = 1;
        }
        fprintf(t, "%d %s %d\n", u.id, u.name, u.age);
    }

    fclose(f); fclose(t);
    remove(FILENAME);
    rename("temp.txt", FILENAME);

    if (found) 
	printf("User updated!\n");
    else 
	printf("User not found!\n");
}

// To Delete
void Userdelete() {
    FILE *f = fopen(FILENAME, "r");
    FILE *t = fopen("temp.txt", "w");
    User u;
    int id, found = 0;

    if (!f) { printf("No users found.\n"); 
	return
	}

    printf("Enter ID to delete: ");
    scanf("%d", &id);

    while (fscanf(f, "%d %s %d", &u.id, u.name, &u.age) != EOF) {
        if (u.id != id)
            fprintf(t, "%d %s %d\n", u.id, u.name, u.age);
        else
            found = 1;
    }

    fclose(f); 
	fclose(t);
    remove(FILENAME);
    rename("temp.txt", FILENAME);

    if (found)
	printf("User deleted!\n");
    else
	printf("User not found!\n");
}
