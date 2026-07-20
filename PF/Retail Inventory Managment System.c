#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_LINE 1024
#define MAX_FILENAME 256
#define MAX_NAME_LEN 200


void inputError(void) {
    printf("\nNo more input detected. Exiting program.\n");
    exit(0);
}


FILE *openFile(const char *name, const char *mode) {
    FILE *fp = fopen(name, mode);

    if (fp == NULL) {
        printf("Error opening %s\n", name);
    }

    return fp;
}


int readInt(const char *prompt) {
    char line[256];
    int value;
    char extra;

    while (1) {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            inputError();
        }
        if (sscanf(line, " %d %c", &value, &extra) == 1) {
            return value;
        }
        printf("Invalid input. Please enter a whole number.\n");
    }
}


int readIntRange(const char *prompt, int min, int max) {
    int value;
    while (1) {
        value = readInt(prompt);
        if (value < min || value > max) {
            printf("Please enter a number between %d and %d.\n", min, max);
            continue;
        }
        return value;
    }
}


float readFloatRange(const char *prompt, float min, float max) {
    char line[256];
    float value;
    char extra;

    while (1) {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            inputError();
        }
        if (sscanf(line, " %f %c", &value, &extra) != 1) {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        if (value < min || value > max) {
            printf("Please enter a number between %.2f and %.2f.\n", min, max);
            continue;
        }
        return value;
    }
}


void readWord(const char *prompt, char *buffer, const int size) {
    char line[MAX_LINE];
    char token[MAX_LINE];

    while (1) {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            inputError();
        }
        if (sscanf(line, "%1023s", token) != 1) {
            printf("Input cannot be empty. Please try again.\n");
            continue;
        }
        if ((int)strlen(token) >= size) {
            printf("That's too long (max %d characters). Please try again.\n", size - 1);
            continue;
        }
        strcpy(buffer, token);
        return;
    }
}

void feedback() {
    char ans[5][20], suggestion[200];
    int yes = 0, no = 0;

    readWord("Did you find everything you were looking for? (yes/no): ", ans[0], sizeof(ans[0]));
    if (strcasecmp(ans[0], "no") == 0)
        printf("We are sorry to hear that. We will try to improve our inventory.\n");

    readWord("Was everything okay with your shopping experience? (yes/no): ", ans[1], sizeof(ans[1]));
    readWord("Would you shop with us again in the future? (yes/no): ", ans[2], sizeof(ans[2]));
    readWord("Would you recommend our store to others? (yes/no): ", ans[3], sizeof(ans[3]));
    readWord("Did you find our store easy to navigate? (yes/no): ", ans[4], sizeof(ans[4]));

    for (int i = 0; i < 5; i++) {
        if (strcasecmp(ans[i], "yes") == 0)
            yes++;
        else if (strcasecmp(ans[i], "no") == 0)
            no++;
    }

    if (yes > no) {
        printf("\nI am so glad to hear that! Thank you for shopping with us.\n");
    } else {
        printf("\nWhat would you like us to improve? (Type your suggestion): ");
        fgets(suggestion, sizeof(suggestion), stdin);
        printf("\nThank you for letting us know. I will share your feedback with our team so we can improve.\n");
    }
    printf("Have a great day and come back anytime.\n");
}

void Checkout() {
    char category[MAX_FILENAME], item[MAX_NAME_LEN], tempFile[MAX_FILENAME], buffer[MAX_LINE];
    int quantity, found = 0;
    float pricePerUnit, totalCost = 0.0;
    FILE *categoryFile, *temp;

    char rawCategory[MAX_NAME_LEN];
    readWord("Enter the category of the item: ", rawCategory, MAX_NAME_LEN);
    snprintf(category, sizeof(category), "%s.txt", rawCategory);

    categoryFile = openFile(category, "r");
    if (categoryFile == NULL) {
        printf("The category '%s' does not exist. Please try again.\n", category);
        return;
    }

    readWord("Enter the name of the item: ", item, MAX_NAME_LEN);

    quantity = readIntRange("Enter the quantity you want to buy: ", 1, 1000000);

    snprintf(tempFile, sizeof(tempFile), "temp_%s", category);
    temp = openFile(tempFile, "w");
    if (temp == NULL) {
        printf("Error: could not create a temporary file. Please try again.\n");
        fclose(categoryFile);
        return;
    }

    while (fgets(buffer, sizeof(buffer), categoryFile)) {
        char currentItem[MAX_LINE] = "";
        int currentQuantity = 0;
        float currentPrice = 0.0;

        if (sscanf(buffer, "%[^=] = %d @ %f", currentItem, &currentQuantity, &currentPrice) != 3) {
            continue;
        }

        currentItem[strcspn(currentItem, "\n")] = '\0';

        if (strcasecmp(currentItem, item) == 0) {
            found = 1;
            if (currentQuantity >= quantity) {
                pricePerUnit = currentPrice;
                totalCost = pricePerUnit * quantity;
                fprintf(temp, "%s = %d @ %.2f\n", currentItem, currentQuantity - quantity, currentPrice);
            } else {
                printf("Insufficient stock! Available quantity: %d\n", currentQuantity);
                fprintf(temp, "%s = %d @ %.2f\n", currentItem, currentQuantity, currentPrice);
            }
        } else {
            fprintf(temp, "%s", buffer);
        }
    }

    fclose(categoryFile);
    fclose(temp);

    remove(category);
    rename(tempFile, category);

    if (!found) {
        printf("The item '%s' was not found in category '%s'.\n", item, category);
    } else if (totalCost > 0) {
        printf("You have purchased %d units of '%s'. Total cost: $%.2f\n", quantity, item, totalCost);
    }
}


void InitialSetup() {
    FILE *fptr;
    FILE *categoryList;
    int i, categoryCount;
    char itemName[MAX_LINE], fileName[MAX_FILENAME];
    char prompt[MAX_NAME_LEN + 50];
    int quantity;
    float price;

    categoryList = openFile("FileName.txt", "w");
    if (categoryList == NULL) {
        printf("Error: could not create FileName.txt. Please check folder permissions.\n");
        return;
    }

    categoryCount = readIntRange("How many categories do you want in your shop? ", 1, 1000);

    for (i = 0; i < categoryCount; i++) {
        char rawCategory[MAX_NAME_LEN];
        snprintf(prompt, sizeof(prompt), "Enter the category number %d: ", i + 1);
        readWord(prompt, rawCategory, MAX_NAME_LEN);
        snprintf(fileName, sizeof(fileName), "%s.txt", rawCategory);
        fputs(fileName, categoryList);
        fputs("\n", categoryList);

        fptr = openFile(fileName, "w");
        if (fptr == NULL) {
            printf("Error: could not create file for category '%s'. Skipping this category.\n", fileName);
            continue;
        }

        int continueAdding;
        do {
            snprintf(prompt, sizeof(prompt), "Enter the item name in category %d: ", i + 1);
            readWord(prompt, itemName, MAX_NAME_LEN);

            quantity = readIntRange("Enter the quantity of the item: ", 0, 1000000);

            snprintf(prompt, sizeof(prompt), "Enter the price for %s: ", itemName);
            price = readFloatRange(prompt, 0.0f, 1000000000.0f);

            fprintf(fptr, "%s = %d @ %.2f\n", itemName, quantity, price);

            do {
                continueAdding = readInt("Enter -1 to end adding items to this category or 1 to continue: ");
            } while (continueAdding != -1 && continueAdding != 1);

        } while (continueAdding == -1);

        fclose(fptr);
    }
    fclose(categoryList);
}

void DeleteOldFiles() {
    FILE *fptr = openFile("FileName.txt", "r");
    if (fptr == NULL) {
        printf("No existing inventory files to delete.\n");
        return;
    }

    char fileName[MAX_FILENAME];
    while (fgets(fileName, sizeof(fileName), fptr)) {
        fileName[strcspn(fileName, "\n")] = '\0';
        if (strlen(fileName) == 0) continue;  // skip blank lines
        if (remove(fileName) == 0) {
            printf("Deleted file: %s\n", fileName);
        } else {
            printf("Failed to delete file: %s\n", fileName);
        }
    }
    fclose(fptr);

    remove("FileName.txt");
}

void NewSetup() {
    DeleteOldFiles();
    InitialSetup();
}

int main() {
    printf("-----------WELCOME TO YOUR BUSINESS MANAGEMENT PAGE-----------\n");
    printf("\nLETS FIRST START BY SETTING UP THE INITIAL INVENTORY\n");
    int choice;

    choice = readInt("Enter 1 to continue: ");
    if (choice != 1)
        return 0;

    InitialSetup();

    while (1) {
        printf("\nWhich function do you want to choose:\n");
        printf("1) Set up inventory\n2) Checkout\n3) Feedback\n4) Exit Program\n");
        choice = readIntRange("Enter your choice: ", 1, 4);

        switch (choice) {
            case 1:
                NewSetup();
                break;
            case 2:
                Checkout();
                break;
            case 3:
                feedback();
                break;
            case 4:
                printf("Exiting program...\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}