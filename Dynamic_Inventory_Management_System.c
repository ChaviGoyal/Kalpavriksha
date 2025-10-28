#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NAME_LEN 50

typedef struct {
    int id;
    char name[NAME_LEN];
    float price;
    int quantity;
} Product;

Product *inventory = NULL;
int productCount = 0;

void clearBuffer() {
    while (getchar() != '\n');
}

int inputInteger(char *msg) {
    int value;
    printf("%s", msg);

    while (scanf("%d", &value) != 1) {
        printf("Invalid input! Enter a valid integer.\n");
        clearBuffer();
        printf("%s", msg);
    }
    clearBuffer();
    return value;
}

float inputFloat(char *msg) {
    float value;
    printf("%s", msg);

    while (scanf("%f", &value) != 1) {
        printf("Invalid input! Enter a valid number.\n");
        clearBuffer();
        printf("%s", msg);
    }
    clearBuffer();
    return value;
}

void inputString(char *msg, char *str) {
    printf("%s", msg);
    fgets(str, NAME_LEN, stdin);
    str[strcspn(str, "\n")] = '\0';

    while (strlen(str) == 0 || strspn(str, "0123456789") == strlen(str)) {
        printf("Invalid name! Enter alphabets only.\n");
        printf("%s", msg);
        fgets(str, NAME_LEN, stdin);
        str[strcspn(str, "\n")] = '\0';
    }
}

char* findSubstringIgnoreCase(const char* text, const char* pattern) {
    if (!*pattern) return (char*)text;

    for (; *text; text++) {
        const char *txtPtr = text;
        const char *patPtr = pattern;
        while (*txtPtr && *patPtr && tolower(*txtPtr) == tolower(*patPtr)) {
            txtPtr++;
            patPtr++;
        }
        if (!*patPtr) return (char*)text;
    }
    return NULL;
}

void sortInventory() {
    for (int i = 0; i < productCount - 1; i++) {
        for (int j = i + 1; j < productCount; j++) {
            if (inventory[i].id > inventory[j].id) {
                Product temp = inventory[i];
                inventory[i] = inventory[j];
                inventory[j] = temp;
            }
        }
    }
}

int isDuplicateID(int id) {
    for (int i = 0; i < productCount; i++) {
        if (inventory[i].id == id) return 1;
    }
    return 0;
}

void addProduct() {
    int id;
    do {
        id = inputInteger("Enter Product ID: ");
        if (isDuplicateID(id)) {
            printf("ID already exists! Enter a different Product ID.\n");
        }
    } while (isDuplicateID(id));

    productCount++;
    inventory = realloc(inventory, productCount * sizeof(Product));
    if (!inventory) {
        printf("Memory Allocation Failed!\n");
        exit(1);
    }

    inventory[productCount - 1].id = id;
    inputString("Enter Product Name: ", inventory[productCount - 1].name);
    inventory[productCount - 1].price = inputFloat("Enter Product Price: ");
    inventory[productCount - 1].quantity = inputInteger("Enter Product Quantity: ");

    sortInventory();
    printf("Product Added Successfully!\n");
}

void viewProducts() {
    if (productCount == 0) {
        printf("No products available.\n");
        return;
    }

    printf("\n--- Inventory List (Sorted by ID) ---\n");
    for (int i = 0; i < productCount; i++) {
        printf("ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
               inventory[i].id, inventory[i].name,
               inventory[i].price, inventory[i].quantity);
    }
}

void updateQuantity() {
    int id = inputInteger("Enter Product ID to update quantity: ");
    for (int i = 0; i < productCount; i++) {
        if (inventory[i].id == id) {
            inventory[i].quantity = inputInteger("Enter New Quantity: ");
            printf("Quantity Updated Successfully!\n");
            return;
        }
    }
    printf("Product not found!\n");
}

void searchByID() {
    int id = inputInteger("Enter Product ID to search: ");
    for (int i = 0; i < productCount; i++) {
        if (inventory[i].id == id) {
            printf("Product Found: ID: %d | Name: %s | Price: %.2f | Qty: %d\n",
                   inventory[i].id, inventory[i].name,
                   inventory[i].price, inventory[i].quantity);
            return;
        }
    }
    printf("Product not found!\n");
}

void searchByName() {
    char keyword[NAME_LEN];
    inputString("Enter name (partial allowed): ", keyword);

    int found = 0;
    for (int i = 0; i < productCount; i++) {
        if (findSubstringIgnoreCase(inventory[i].name, keyword)) {
            printf("ID: %d | Name: %s | Price: %.2f | Qty: %d\n",
                   inventory[i].id, inventory[i].name,
                   inventory[i].price, inventory[i].quantity);
            found = 1;
        }
    }

    if (!found) printf("No matching products found!\n");
}

void searchByPriceRange() {
    float min = inputFloat("Enter Minimum Price: ");
    float max = inputFloat("Enter Maximum Price: ");

    int found = 0;
    for (int i = 0; i < productCount; i++) {
        if (inventory[i].price >= min && inventory[i].price <= max) {
            printf("ID: %d | Name: %s | Price: %.2f | Qty: %d\n",
                   inventory[i].id, inventory[i].name,
                   inventory[i].price, inventory[i].quantity);
            found = 1;
        }
    }
    if (!found) printf("No matching products found!\n");
}

void deleteProduct() {
    int id = inputInteger("Enter Product ID to delete: ");

    for (int i = 0; i < productCount; i++) {
        if (inventory[i].id == id) {
            for (int j = i; j < productCount - 1; j++) {
                inventory[j] = inventory[j + 1];
            }
            productCount--;
            inventory = realloc(inventory, productCount * sizeof(Product));
            printf("Product deleted successfully!\n");
            return;
        }
    }
    printf("Product not found!\n");
}

int main() {
    int initial;
    initial = inputInteger("Enter initial number of products: ");

    inventory = calloc(initial, sizeof(Product));
    productCount = 0;

    for (int i = 0; i < initial; i++) {
        printf("\nEnter details for product %d:\n", i + 1);
        addProduct();
    }

    while (1) {
        printf("\n========= INVENTORY MENU =========\n");
        printf("1. Add New Product\n");
        printf("2. View All Products\n");
        printf("3. Update Quantity\n");
        printf("4. Search Product by ID\n");
        printf("5. Search Product by Name\n");
        printf("6. Search Product by Price Range\n");
        printf("7. Delete Product\n");
        printf("8. Exit\n");

        int choice = inputInteger("Enter your choice: ");

        switch (choice) {
            case 1: addProduct(); break;
            case 2: viewProducts(); break;
            case 3: updateQuantity(); break;
            case 4: searchByID(); break;
            case 5: searchByName(); break;
            case 6: searchByPriceRange(); break;
            case 7: deleteProduct(); break;
            case 8:
                free(inventory);
                printf("Memory released! Program exiting...\n");
                exit(0);
            default:
                printf("Invalid choice! Try again.\n");
        }
    }
}
