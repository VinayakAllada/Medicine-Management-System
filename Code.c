#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEDICATIONS 100
#define SUPPLIERS 20

typedef struct Node_Tag {
    int data;
    struct Node_Tag* next;
    struct Node_Tag* prev;
} Node;

typedef struct Date_Tag {
    int date;
    int month;
    int year;
} expiryDate;

typedef struct Supplier_Tag {
    long int supplier_id;
    char supplier_name[30];
    int quantity_of_medication_supply;
    long long contact_information;

    struct Supplier_Tag* nextSupplier;
    struct Supplier_Tag* prevSupplier;
} supplier;

typedef struct Medicine_Tag {
    long int medication_id;
    char medication_name[30];
    long int batch_number;
    expiryDate expiration_date;
    int quantity_of_stock;
    int price_per_unit;
    int total_sales;
    supplier* suppliers;
    int reorder_level;

    struct Medicine_Tag* nextMedicine;
    struct Medicine_Tag* prevMedicine;
} medicine;

typedef struct uniqueSupplier_Tag { // for unique suppliers
    long int supplier_id;
    int noOfUniqueMedicine;
    float totalTurnOver;

    struct uniqueSupplier_Tag* nextUniqueSupplier;
    struct uniqueSupplier_Tag* prevUniqueSupplier;
} uniqueSupplier;

uniqueSupplier* uniqSuppliers = NULL;
int uniqueSupplierCount = 0;
int flag = 1;
int medication_count = 0;

void totalNumberOfSuppliers(int supplierId, int quantity, int price_per_unit) {
    uniqueSupplier* temp = uniqSuppliers;
    for (int i = 0; i < uniqueSupplierCount; i++) {
        if (temp->supplier_id == supplierId) { // for already existing supplier
            temp->noOfUniqueMedicine++;
            temp->totalTurnOver += quantity * price_per_unit;
            return;
        }
        temp = temp->nextUniqueSupplier;
    }

    uniqueSupplierCount++;

    uniqueSupplier* newSupplier = (uniqueSupplier*)malloc(sizeof(uniqueSupplier)); // allocate memory for new supplier
    if (!newSupplier) {
        printf("Memory allocation failed\n");
        return;
    }
    newSupplier->supplier_id = supplierId;
    newSupplier->noOfUniqueMedicine = 1;
    newSupplier->totalTurnOver = quantity * price_per_unit;
    newSupplier->nextUniqueSupplier = NULL;
    newSupplier->prevUniqueSupplier = NULL;

    if (!uniqSuppliers) {
        uniqSuppliers = newSupplier;
        return;
    }

    temp = uniqSuppliers;
    while (temp->nextUniqueSupplier) {
        temp = temp->nextUniqueSupplier;
    }
    temp->nextUniqueSupplier = newSupplier;
    newSupplier->prevUniqueSupplier = temp;
}

void saveData(medicine* medicationHead) {
    FILE* file = fopen("medication_data.txt", "w");
    if (!file) {
        printf("Error opening file for writing\n");
        return;
    }

    medicine* temp = medicationHead;
    while (temp) {
        fprintf(file, "%ld %s %ld %d %d %d %d %d %d %d %ld\n",
                temp->medication_id, temp->medication_name, temp->batch_number,
                temp->expiration_date.date, temp->expiration_date.month, temp->expiration_date.year,
                temp->quantity_of_stock, temp->price_per_unit, temp->total_sales, temp->reorder_level,
                temp->suppliers ? temp->suppliers->supplier_id : -1);

        supplier* tempSupplier = temp->suppliers;
        while (tempSupplier) {
            fprintf(file, "%ld %s %d %lld\n",
                    tempSupplier->supplier_id, tempSupplier->supplier_name,
                    tempSupplier->quantity_of_medication_supply, tempSupplier->contact_information);
            tempSupplier = tempSupplier->nextSupplier;
        }
        fprintf(file, "-1\n"); // End of suppliers for this medication

        temp = temp->nextMedicine;
    }

    // Save unique suppliers
    fprintf(file, "UNIQUE_SUPPLIERS\n");
    uniqueSupplier* tempUnique = uniqSuppliers;
    while (tempUnique) {
        fprintf(file, "%ld %d %.2f\n",
                tempUnique->supplier_id, tempUnique->noOfUniqueMedicine, tempUnique->totalTurnOver);
        tempUnique = tempUnique->nextUniqueSupplier;
    }
    fprintf(file, "END_UNIQUE_SUPPLIERS\n");

    fclose(file);
    printf("Data saved successfully\n");
}

void loadData(medicine** medicationHead, medicine** medicationTail) {
    FILE* file = fopen("medication_data.txt", "r");
    if (!file) {
        printf("Error opening file for reading\n");
        return;
    }

    while (!feof(file)) {
        medicine* newMedicine = (medicine*)malloc(sizeof(medicine));
        if (!newMedicine) {
            printf("Memory allocation failed\n");
            fclose(file);
            return;
        }

        if (fscanf(file, "%ld %s %ld %d %d %d %d %d %d %d %d",
                   &newMedicine->medication_id, newMedicine->medication_name, &newMedicine->batch_number,
                   &newMedicine->expiration_date.date, &newMedicine->expiration_date.month, &newMedicine->expiration_date.year,
                   &newMedicine->quantity_of_stock, &newMedicine->price_per_unit, &newMedicine->total_sales, &newMedicine->reorder_level,
                   &newMedicine->suppliers) != 11) {
            free(newMedicine);
            break;
        }

        newMedicine->suppliers = NULL;
        supplier* lastSupplier = NULL;
        while (1) {
            long int supplier_id;
            if (fscanf(file, "%ld", &supplier_id) != 1 || supplier_id == -1) {
                break;
            }

            supplier* newSupplier = (supplier*)malloc(sizeof(supplier));
            if (!newSupplier) {
                printf("Memory allocation failed\n");
                fclose(file);
                return;
            }

            newSupplier->supplier_id = supplier_id;
            fscanf(file, "%s %d %lld", newSupplier->supplier_name, &newSupplier->quantity_of_medication_supply, &newSupplier->contact_information);
            newSupplier->nextSupplier = NULL;

            if (lastSupplier) {
                lastSupplier->nextSupplier = newSupplier;
                newSupplier->prevSupplier = lastSupplier;
            } else {
                newMedicine->suppliers = newSupplier;
                newSupplier->prevSupplier = NULL;
            }
            lastSupplier = newSupplier;
            totalNumberOfSuppliers(newSupplier->supplier_id, newSupplier->quantity_of_medication_supply, newMedicine->price_per_unit);  
        }

        newMedicine->nextMedicine = NULL;
        newMedicine->prevMedicine = *medicationTail;
        if (*medicationTail) {
            (*medicationTail)->nextMedicine = newMedicine;
        } else {
            *medicationHead = newMedicine;
        }
        *medicationTail = newMedicine;
    }

    fclose(file);
    printf("Data loaded successfully\n");
}

supplier* initSupplier(medicine** medicine, supplier** suppliers) {
    // Allocate memory for the new supplier
    *suppliers = (supplier*)malloc(sizeof(supplier));
    if (!*suppliers) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    printf("enter the supplier id\n");
    scanf("%ld", &(*suppliers)->supplier_id);
    getchar(); // Consume the newline character left by scanf

    printf("enter the supplier name\n");
    fgets((*suppliers)->supplier_name, sizeof((*suppliers)->supplier_name), stdin);
    (*suppliers)->supplier_name[strcspn((*suppliers)->supplier_name, "\n")] = '\0'; // Remove the newline character

    printf("enter the quantity of medication supply by suppliers\n");
    scanf("%d", &(*suppliers)->quantity_of_medication_supply);
    (*medicine)->quantity_of_stock += (*suppliers)->quantity_of_medication_supply;
    getchar(); // Consume the newline character left by scanf

    printf("enter the contact information of supplier \n");
    scanf("%lld", &(*suppliers)->contact_information);
    while (getchar() != '\n'); // Consume the newline character left by scanf

    totalNumberOfSuppliers((*suppliers)->supplier_id, (*suppliers)->quantity_of_medication_supply, (*medicine)->price_per_unit);

    return *suppliers;
}

supplier* initAllSupplier(medicine** medicine, int nS) {
    for (int i = nS; i > 0; i--) {
        supplier* newSupplier;
        initSupplier(&(*medicine), &newSupplier);

        if ((*medicine)->suppliers == NULL) {
            (*medicine)->suppliers = newSupplier;
        } else {
            supplier* temp = (*medicine)->suppliers;
            while (temp->nextSupplier != NULL) {
                temp = temp->nextSupplier;
            }
            temp->nextSupplier = newSupplier;
            temp->nextSupplier->prevSupplier = temp;
        }
    }
    return (*medicine)->suppliers;
}

void initMedicine(medicine** med) {
    *med = (medicine*)malloc(sizeof(medicine)); // allocate memory for medicine
    if (!*med) {
        printf("Memory allocation failed\n");
        return;
    }

    printf("enter the medication id\n");
    scanf("%ld", &(*med)->medication_id);

    printf("enter the medication name\n");
    scanf("%s", (*med)->medication_name);

    printf("enter the batch number\n");
    scanf("%ld", &(*med)->batch_number);

    printf("enter the price per unit stock\n");
    scanf("%d", &(*med)->price_per_unit);

    printf("enter the expiration date\n");
    scanf("%d %d %d", &(*med)->expiration_date.date, &(*med)->expiration_date.month, &(*med)->expiration_date.year);

    printf("enter reorder level (>30)\n");
    scanf("%d", &(*med)->reorder_level);

    int nS;
    printf("enter the number of suppliers you want to add for this medication\n");
    scanf("%d", &nS);

    (*med)->suppliers = initAllSupplier(med, nS);

    (*med)->nextMedicine = NULL;
    (*med)->prevMedicine = NULL;
}

void displayMedication(medicine* medicine) {
    printf("\nmedication id: %ld\n", medicine->medication_id);
    printf("medication name: %s\n", medicine->medication_name);
    printf("quantity in stock: %d\n", medicine->quantity_of_stock);
    printf("price per unit: %d\n", medicine->price_per_unit);
    printf("reorder level: %d\n", medicine->reorder_level);
    printf("batch: %ld\n", medicine->batch_number);
    printf("expiration date: %d-%d-%d\n", medicine->expiration_date.date, medicine->expiration_date.month, medicine->expiration_date.year);
    printf("quantity in stock: %d\n", medicine->quantity_of_stock);

    int j = 0;
    while (medicine->suppliers) {
        printf("Supplier-%d id : %ld\n", j + 1, medicine->suppliers->supplier_id);
        printf("Supplier-%d name : %s\n", j + 1, medicine->suppliers->supplier_name);
        printf("Supplier-%d quantity supplied : %d\n", j + 1, medicine->suppliers->quantity_of_medication_supply);
        printf("Supplier-%d contact number : %lld\n", j + 1, medicine->suppliers->contact_information);

        medicine->suppliers = medicine->suppliers->nextSupplier;
        j++;
    }
    printf("total sales: %d\n", medicine->total_sales);
    printf("\n");
}

void find_suppliers_with_largest_turn_over() {
    if (!uniqSuppliers) return;

    int flag;
    uniqueSupplier *temp, *prev, *nextNode;

    do {
        flag = 0;
        temp = uniqSuppliers;
        prev = NULL;

        while (temp && temp->nextUniqueSupplier) {
            nextNode = temp->nextUniqueSupplier;

            if (temp->totalTurnOver < nextNode->totalTurnOver) {
                // Swap nodes
                temp->nextUniqueSupplier = nextNode->nextUniqueSupplier;
                nextNode->nextUniqueSupplier = temp;

                if (prev) {
                    prev->nextUniqueSupplier = nextNode;
                } else {
                    uniqSuppliers = nextNode;  // Update head if first node is swapped
                }

                prev = nextNode;
                flag = 1;
            } else {
                prev = temp;
                temp = temp->nextUniqueSupplier;
            }
        }
    } while (flag);

    // Print top 10 suppliers
    temp = uniqSuppliers;
    int rank = 1;
    while (temp && rank <= 10) {
        printf("Supplier with ID %ld has turnover %.2f and is ranked %d\n",
               temp->supplier_id, temp->totalTurnOver, rank);
        temp = temp->nextUniqueSupplier;
        rank++;
    }
}

void find_all_rounder_suppliers() {
    if (!uniqSuppliers) return;

    int flag;
    uniqueSupplier *temp, *prev, *nextNode;

    do {
        flag = 0;
        temp = uniqSuppliers;
        prev = NULL;

        while (temp && temp->nextUniqueSupplier) {
            nextNode = temp->nextUniqueSupplier;

            if (temp->noOfUniqueMedicine < nextNode->noOfUniqueMedicine) {
                // Swap nodes
                temp->nextUniqueSupplier = nextNode->nextUniqueSupplier;
                nextNode->nextUniqueSupplier = temp;

                if (prev) {
                    prev->nextUniqueSupplier = nextNode;
                } else {
                    uniqSuppliers = nextNode;  // Update head if first node is swapped
                }

                prev = nextNode;
                flag = 1;
            } else {
                prev = temp;
                temp = temp->nextUniqueSupplier;
            }
        }
    } while (flag);

    // Print top 10 suppliers
    temp = uniqSuppliers;
    int rank = 1;
    while (temp && rank <= 10) {
        printf("Supplier with ID %ld supplies %d unique medications and is ranked %d\n",
               temp->supplier_id, temp->noOfUniqueMedicine, rank);
        temp = temp->nextUniqueSupplier;
        rank++;
    }
}

void salesTracking(medicine* medication) {
    printf("enter the id of medication that you want to track the sales\n");
    long int id;
    scanf("%ld", &id);

    flag = 1;
    medicine* temp = medication;
    while (temp && flag) {
        if (temp->medication_id == id) {
            flag = 0;
            int sales;
            printf("enter the sales of medication id - %ld and name - %s\n", temp->medication_id, temp->medication_name);
            scanf("%d", &sales);

            if (temp->quantity_of_stock >= sales) {
                temp->quantity_of_stock -= sales;
                temp->total_sales += sales;
                printf("sales recorded successfully\n");
            } else {
                printf("insufficient stock for %s with id %ld\n", temp->medication_name, temp->medication_id);
            }
        } else {
            temp = temp->nextMedicine;
        }
    }

    if (flag == 1) {
        printf("entered invalid id\n");
    }
}

void supplier_Management(medicine** medication) {
    long int id;
    printf("Enter the ID of the medicine for which you want to modify suppliers:\n");
    scanf("%ld", &id);

    medicine* temp = *medication;
    int found = 0;

    while (temp) {
        if (temp->medication_id == id) {
            found = 1;
            break;
        }
        temp = temp->nextMedicine;
    }

    if (!found) {
        printf("Invalid medication ID. Please try again.\n");
        return;
    }

    int ch;
    printf("What operation do you want to perform?\n");
    printf("1. Add Supplier\n2. Update Supplier\n3. Search for Supplier\n");
    scanf("%d", &ch);

    switch (ch) {
        case 1: {  // **ADD SUPPLIER**
            int nS;
            printf("Enter the number of suppliers you want to add for this medication:\n");
            scanf("%d", &nS);
            supplier* newSupplier;

            for (int i = 0; i < nS; i++) {
                initSupplier(&temp, &newSupplier);

                if (!temp->suppliers) {
                    temp->suppliers = newSupplier;
                } else {
                    supplier* last = temp->suppliers;
                    while (last->nextSupplier) {
                        last = last->nextSupplier;
                    }
                    last->nextSupplier = newSupplier;
                    newSupplier->prevSupplier = last;
                }
            }
            printf("Suppliers added successfully!\n");
            totalNumberOfSuppliers(newSupplier->supplier_id, newSupplier->quantity_of_medication_supply, temp->price_per_unit);
            break;
        }

        case 2: {  // **UPDATE SUPPLIER**
            long int supplierId;
            printf("Enter the ID of the supplier to update details:\n");
            scanf("%ld", &supplierId);

            supplier* tempSupplier = temp->suppliers;
            found = 0;

            while (tempSupplier) {
                if (tempSupplier->supplier_id == supplierId) {
                    found = 1;
                    
                    printf("Enter the new name of the supplier:\n");
                    getchar();
                    fgets(tempSupplier->supplier_name, sizeof(tempSupplier->supplier_name), stdin);
                    tempSupplier->supplier_name[strcspn(tempSupplier->supplier_name, "\n")] = '\0';

                    printf("Enter the new quantity supplied:\n");
                    scanf("%d", &tempSupplier->quantity_of_medication_supply);

                    printf("Enter the new contact information:\n");
                    scanf("%lld", &tempSupplier->contact_information);

                    printf("Supplier details updated successfully!\n");
                    break;
                }
                tempSupplier = tempSupplier->nextSupplier;
            }

            if (!found) {
                printf("Supplier ID not found.\n");
            }
            break;
        }

        case 3: {  // **SEARCH SUPPLIER**
            long int supplierId;
            printf("Enter the ID of the supplier to search:\n");
            scanf("%ld", &supplierId);

            supplier* tempSupplier = temp->suppliers;
            found = 0;

            while (tempSupplier) {
                if (tempSupplier->supplier_id == supplierId) {
                    printf("\nSupplier Found:\n");
                    printf("Supplier ID: %ld\n", tempSupplier->supplier_id);
                    printf("Supplier Name: %s\n", tempSupplier->supplier_name);
                    printf("Quantity Supplied: %d\n", tempSupplier->quantity_of_medication_supply);
                    printf("Contact Info: %lld\n", tempSupplier->contact_information);
                    found = 1;
                    break;
                }
                tempSupplier = tempSupplier->nextSupplier;
            }

            if (!found) {
                printf("Supplier ID not found.\n");
            }
            break;
        }

        default:
            printf("Invalid option! Please choose 1, 2, or 3.\n");
    }
    saveData(*medication);
}

void swap(medicine** a, medicine** b) {
    // Swap the contents of two medicine nodes
    long int temp_medication_id = (*a)->medication_id;
    char temp_medication_name[30];
    strcpy(temp_medication_name, (*a)->medication_name);
    long int temp_batch_number = (*a)->batch_number;
    expiryDate temp_expiration_date = (*a)->expiration_date;
    int temp_quantity_of_stock = (*a)->quantity_of_stock;
    int temp_price_per_unit = (*a)->price_per_unit;
    int temp_total_sales = (*a)->total_sales;
    int temp_reorder_level = (*a)->reorder_level;
    supplier* temp_suppliers = (*a)->suppliers;

    (*a)->medication_id = (*b)->medication_id;
    strcpy((*a)->medication_name, (*b)->medication_name);
    (*a)->batch_number = (*b)->batch_number;
    (*a)->expiration_date = (*b)->expiration_date;
    (*a)->quantity_of_stock = (*b)->quantity_of_stock;
    (*a)->price_per_unit = (*b)->price_per_unit;
    (*a)->total_sales = (*b)->total_sales;
    (*a)->reorder_level = (*b)->reorder_level;
    (*a)->suppliers = (*b)->suppliers;

    (*b)->medication_id = temp_medication_id;
    strcpy((*b)->medication_name, temp_medication_name);
    (*b)->batch_number = temp_batch_number;
    (*b)->expiration_date = temp_expiration_date;
    (*b)->quantity_of_stock = temp_quantity_of_stock;
    (*b)->price_per_unit = temp_price_per_unit;
    (*b)->total_sales = temp_total_sales;
    (*b)->reorder_level = temp_reorder_level;
    (*b)->suppliers = temp_suppliers;
}

int compareDates(expiryDate a, expiryDate b) {
    if (a.year != b.year) return a.year - b.year;
    if (a.month != b.month) return a.month - b.month;
    return a.date - b.date;
}

medicine* partition(medicine* low, medicine* high) {
    expiryDate pivot = high->expiration_date;
    medicine* i = low->prevMedicine;

    for (medicine* j = low; j != high; j = j->nextMedicine) {
        if (compareDates(j->expiration_date, pivot) < 0) {
            i = (i == NULL) ? low : i->nextMedicine;
            swap(&i, &j);
        }
    }
    i = (i == NULL) ? low : i->nextMedicine;
    swap(&i, &high);
    return i;
}

void quickSort(medicine* low, medicine* high) {
    if (high != NULL && low != high && low != high->nextMedicine) {
        medicine* p = partition(low, high);
        quickSort(low, p->prevMedicine);
        quickSort(p->nextMedicine, high);
    }
}

void sort_Medication_by_Expiration_Dates(medicine** medication) {
    if (*medication == NULL) return;

    medicine* head = *medication;
    medicine* tail = *medication;

    while (tail->nextMedicine != NULL) {
        tail = tail->nextMedicine;
    }
    quickSort(head, tail);

    printf("Medicines sorted by expiry date:\n");

    medicine* temp = *medication;
    while(temp){
        displayMedication(temp);
        temp = temp->nextMedicine;
    }
}

int isLeapYear( int i){ // for checking if the year is leap year or not

    if ( i % 400 == 0 || ( i % 100 != 0 && i % 4 == 0))
        return 1;
    return 0;
}

int noOfDaysTillDay(expiryDate Date){ // finding no of days form 0th year 

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int noOfDays = Date.date;
    
    for (int i = 0; i < Date.year; i++) {
        noOfDays += (isLeapYear(i) ? 366 : 365);
    }

    for (int i = 0; i < Date.month - 1; i++) {
        noOfDays += daysInMonth[i];
        if (i == 1 && isLeapYear(Date.year))  // Adjust for February in leap year
            noOfDays += 1;
    }

    return noOfDays;
}

int difference(expiryDate today, expiryDate expiryDay){

    return noOfDaysTillDay(expiryDay)-noOfDaysTillDay(today);
}

void check_Expiration_Dates (medicine* medication){
    expiryDate today;

    printf("enter today date:\n");
    scanf("%d",&today.date);
    while( today.date > 31 || today.date < 1){
        printf("enter valid date\n");
        scanf("%d",&today.date);
    }

    printf("enter this month:\n");
    scanf("%d",&today.month);
    while( today.month > 12 || today.month < 1){
        printf("enter valid month\n");
        scanf("%d",&today.month);
    }

    printf("enter this year:\n");
    scanf("%d",&today.year);
    while(today.year < 0){
        printf("enter valid year\n");
        scanf("%d",&today.year);
    }

    medicine* temp = medication;
    while(temp){
        int differenceOfDays = difference(today,temp->expiration_date);
        if( differenceOfDays < 30 && differenceOfDays > 0 ){
            printf("Alert: medication named %s with id %ld will expire in %d days\n",temp->medication_name,temp->medication_id,differenceOfDays);
        }
        else if(differenceOfDays < 0){
            printf("Alert: your medication named %s with id %ld is expired\n",temp->medication_name,temp->medication_id);
        }
        else{
            printf("your medication named %s with id %ld is valid and %d days are left before expiration\n",temp->medication_name,temp->medication_id,differenceOfDays);
        }
        temp = temp->nextMedicine;
    }
}

void stockAlert(medicine* medication){
    
    flag = 1;
    medicine* temp = medication;
    while(temp){
        if(temp->quantity_of_stock < temp->reorder_level){
            flag = 0;
            printf("Alert : Quantity of your medication named %s with id number %ld falls below reorder-level\n",medication->medication_name,medication->medication_id);
        }
        temp = temp->nextMedicine;
    }
    if(flag) printf("All the medicine stocks are sufficient(>more than re-order level)\n");
}

void search_Medication(medicine* medication){

    int ch;
    printf("enter the type of information you are giving\n");
    printf("1. id\n 2. name\n 3. supplier\n ");
    scanf("%d",&ch);
    flag = 1;

    switch(ch){

        case 1:
            {
                long int searchById;
                printf("enter the id\n");
                scanf("%ld",&searchById);

                medicine* temp = medication;
                while(temp && flag){
                    if(temp->medication_id == searchById){
                        displayMedication(temp);
                        flag = 0;
                    }
                    temp = temp->nextMedicine;
                }

                if(flag == 1){
                    printf("entered invalid id\n");
                    printf("\n");
                    return ;
                }

                break;
            }

        case 2:
            {
                char searchByName[30];
                printf("enter the name\n");
                scanf("%s",searchByName);

                medicine* temp = medication;
                while(temp && flag){
                    if(!(strcmp(temp->medication_name, searchByName))){
                        displayMedication(temp);
                        flag = 0;
                    }
                    temp = temp->nextMedicine;
                }

                if(flag == 1){
                    printf("entered invalid name\n");
                    printf("\n");
                    return ;
                }

                break;
            }

        case 3:
            {
                long int searchBySupplierId;
                printf("enter the supplier id\n");
                scanf("%ld",&searchBySupplierId);

                medicine* temp = medication;
                while(temp && flag){
                    supplier* tempSupplier = temp->suppliers;
                    while(tempSupplier && flag){
                        if(tempSupplier->supplier_id == searchBySupplierId){
                            displayMedication(temp);
                        }
                        tempSupplier = tempSupplier->nextSupplier;
                    }
                    temp = temp->nextMedicine;
                }

                if(flag == 1){
                    printf("entered invalid supplier id\n");
                    printf("\n");
                    return ;
                }
                break;
            }

        default:
            printf("Invalid option!\nEnter correct option again\n");
            search_Medication(medication);
            break;
    }
}

void delete_Medication(medicine** medication){
   int ch;
   printf("which information are you giving\n");
   printf("1. medication id \n 2. batch number\n");
   scanf("%d",&ch);

   flag = 1;

   switch(ch){
     case 1:
          {
             long int id;
             printf("enter the id of the medication that you want to delete\n");
             scanf("%ld",&id);

             medicine* temp = *medication;

             while(temp && flag){
                    if(temp->medication_id == id){
                        flag = 0;
                        if(temp->prevMedicine == NULL && temp->nextMedicine == NULL){
                            free(temp);
                            *medication = NULL;
                        }
                        else if(temp->prevMedicine == NULL){
                            *medication = temp->nextMedicine;
                            temp->nextMedicine->prevMedicine = NULL;
                            free(temp);
                        }
                        else if(temp->nextMedicine == NULL){
                            temp->prevMedicine->nextMedicine = NULL;
                            free(temp);
                        }
                        else{
                            temp->prevMedicine->nextMedicine = temp->nextMedicine;
                            temp->nextMedicine->prevMedicine = temp->prevMedicine;
                            free(temp);
                        }
                    }
                    temp = temp->nextMedicine;
             }

             if(flag == 1){
                 printf("enter valid id\n");
                 delete_Medication(&(*medication));
                 return;
             }

             break;
          }

     case 2:
          {
            long int batch;
            printf("enter the batch number\n");
            scanf("%ld",&batch);

            medicine* temp = *medication;

            while(temp){
                if(temp->batch_number == batch){
                    if(temp->prevMedicine == NULL && temp->nextMedicine == NULL){
                        free(temp);
                        *medication = NULL;
                    }
                    else if(temp->prevMedicine == NULL){
                        *medication = temp->nextMedicine;
                        temp->nextMedicine->prevMedicine = NULL;
                        free(temp);
                    }
                    else if(temp->nextMedicine == NULL){
                        temp->prevMedicine->nextMedicine = NULL;
                        free(temp);
                    }
                    else{
                        temp->prevMedicine->nextMedicine = temp->nextMedicine;
                        temp->nextMedicine->prevMedicine = temp->prevMedicine;
                        free(temp);
                    }
                }
                temp = temp->nextMedicine;
            }

            if(flag == 1){
                printf("enter valid batch number\n");
                delete_Medication(&(*medication));
                return;
            }

            break;
          }

          default:
            printf("Invalid option!\nEnter correct option again\n");
            delete_Medication(&(*medication));
            break;
   }
   medication_count--;
}

void update_Medication_Details(medicine** medication){

    long int id;
    printf("enter the id of the medication\n");
    scanf("%ld",&id);
    int reqId;

    medicine* temp = *medication;
    while(temp && flag == 1){

        if(temp->medication_id == id){

            flag = 0;

            int ch;
            printf("enter which details do you want to modify (1-3)\n");
            printf("1. price\n2. stock\n3. supplier information\n");

            scanf("%d",&ch);
            if ( ch == 1 ){
                printf("enter the new price\n");
                scanf("%d",&temp->price_per_unit);
            }
            else if ( ch == 2){
                printf("enter the quantity of the stock\n");
                scanf("%d",&temp->quantity_of_stock); 
            }
            else{
                int sId;
                int required_supplier;
                printf("enter the supplier id for which you want to modify details\n");
                scanf("%d",&sId);
                int flag = 1;

                while(temp->suppliers && flag == 1){

                    if(temp->suppliers->supplier_id == sId){
                        flag = 0;

                        printf("enter the new supplier name \n");
                        scanf("%s",temp->suppliers->supplier_name);
                        printf("enter the new quantity of medication supply by suppliers\n");
                        scanf("%d",&temp->suppliers->quantity_of_medication_supply);
                        printf("enter the new contact information of supplier \n");
                        scanf("%lld",&temp->suppliers->contact_information);
                    }
                    temp->suppliers = temp->suppliers->nextSupplier;
                }
            }
        }
        else temp = temp->nextMedicine;
    }

    if(flag == 1){
        printf("enter valid medication id\n\n");
        update_Medication_Details(&(*medication));
    }
}

void add_New_Medication(medicine** medicineHead, medicine** medicineTail){
    if(medication_count == MEDICATIONS){
        printf("Medication limit reached!\n");
        return;
    }
    medication_count++;

    if((*medicineHead) == NULL){
        *medicineHead = (medicine*)malloc(sizeof(medicine));
        initMedicine(&(*medicineHead));
        *medicineTail = *medicineHead;
    }
    else{
        (*medicineTail)->nextMedicine = (medicine*)malloc(sizeof(medicine));
        (*medicineTail)->nextMedicine->prevMedicine = *medicineTail;
        initMedicine(&(*medicineTail)->nextMedicine);
        *medicineTail = (*medicineTail)->nextMedicine;
    }
}

void printAllUniqueSuppliers() {
    uniqueSupplier* suppliers = uniqSuppliers;
    while (suppliers) {
        printf("Supplier ID: %ld\n", suppliers->supplier_id);
        printf("No of unique medicines supplied: %d\n", suppliers->noOfUniqueMedicine);
        printf("Total Turnover: %.2f\n", suppliers->totalTurnOver);
        suppliers = suppliers->nextUniqueSupplier;
    }
}

void displayAllMedications(medicine* medication){
    medicine* temp = medication;
    while(temp){
        displayMedication(temp);
        temp = temp->nextMedicine;
    }
}

int main() {
    medicine* medicationHead = NULL;
    medicine* medicationTail = NULL;

    loadData(&medicationHead, &medicationTail); // Load data at the start

    while (1) {
        int ch;

        printf("1. Add New Medication\n2. Update Medication Details\n3. Delete Medication\n4. Search Medication\n5. Stock Alerts\n");
        printf("6. Check Expiration Dates\n7. Sort Medication by Expiration Date\n8. Sales Tracking\n");
        printf("9. Supplier Management\n10. Find all-rounder suppliers\n11. Find suppliers with largest turn-over\n");
        printf("12.Display\n13. print all unique suppliers\n14. Exit\n");
        printf("\n");
        printf("Enter the value of choice to access the function you desire\n");
        printf("\n");
        scanf("%d", &ch);

        switch (ch) {
            case 1:
                add_New_Medication(&medicationHead, &medicationTail);
                saveData(medicationHead);
                printf("\n");
                break;
            case 2:
                update_Medication_Details(&medicationHead);
                saveData(medicationHead);
                printf("\n");
                break;
            case 3:
                delete_Medication(&medicationHead);
                saveData(medicationHead);
                printf("\n");
                break;
            case 4:
                search_Medication(medicationHead);
                printf("\n");
                break;
            case 5:
                stockAlert(medicationHead);
                printf("\n");
                break;
            case 6:
                check_Expiration_Dates(medicationHead);
                printf("\n");
                break;
            case 7:
                sort_Medication_by_Expiration_Dates(&medicationHead);
                saveData(medicationHead);
                printf("\n");
                break;
            case 8:
                salesTracking(medicationHead);
                printf("\n");
                break;
            case 9:
                supplier_Management(&medicationHead);
                saveData(medicationHead);
                printf("\n");
                break;
            case 10:
                find_all_rounder_suppliers();
                printf("\n");
                break;
            case 11:
                find_suppliers_with_largest_turn_over(medicationHead);
                printf("\n");
                break;
            case 12:
                displayAllMedications(medicationHead);
                printf("\n");
                break;
            case 13:
                printAllUniqueSuppliers();
                printf("\n");
                break;
            case 14:
                printf("Exiting...\n");
                saveData(medicationHead); // Save data before exiting
                printf("\n");
                return 0;
            default:
                printf("Invalid option!\n");
                printf("\n");
                break;
        }
    }

    return 0;
}