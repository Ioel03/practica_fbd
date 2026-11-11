#include <stdio.h>
#include <stdlib.h>
#include "odbc.h"
#include "products.h"
#include "orders.h"
#include "customers.h"

/* Prototipo de la función que muestra el menú principal */
static int ShowMainMenu(void);

int main(void) {
    int nChoice = 0;

    do {
        nChoice = ShowMainMenu();
        switch (nChoice) {
            case 1:
                ShowProductsMenu();
                break;
            case 2:
                ShowOrdersMenu();
                break;
            case 3:
                ShowCustomersMenu();
                break;
            case 4:
                printf("Bye Bye\n\n");
                break;
        }
    } while (nChoice != 4);

    return EXIT_SUCCESS;
}

static int ShowMainMenu(void) {
    int nSelected = 0;
    char buf[16];

    do {
        printf(" (1) Products\n"
               " (2) Orders\n"
               " (3) Customers\n"
               " (4) Exit\n\n"
               "Enter a number that corresponds to your choice > ");
        
        if (!fgets(buf, 16, stdin)) {
            nSelected = 0;
        } else {
            nSelected = atoi(buf);
        }
        printf("\n");

        if ((nSelected < 1) || (nSelected > 4)) {
            printf("You have entered an invalid choice. Please try again\n\n\n");
        }
    } while ((nSelected < 1) || (nSelected > 4));

    return nSelected;
}
     
     


