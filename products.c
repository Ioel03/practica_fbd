#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "products.h"

#define BufferLength 512

static int ShowProductsSubMenu(void) {
    int nSelected = 0;
    char buf[16];

    do {
        printf(" (1) Stock\n"
               " (2) Find\n"
               " (3) Back\n\n"
               "Enter a number that corresponds to your choice > ");
        if (!fgets(buf, 16, stdin))
            nSelected = 0;
        else
            nSelected = atoi(buf);
        printf("\n");

        if ((nSelected < 1) || (nSelected > 3)) {
            printf("You have entered an invalid choice. Please try again\n\n\n");
        }
    } while ((nSelected < 1) || (nSelected > 3));

    return nSelected;
}

void ShowProductsMenu(void) {
    int nChoice = 0;
    do {
        nChoice = ShowProductsSubMenu();
        switch (nChoice) {
            case 1:
                (void) ProductsStock();
                break;
            case 2:
                (void) ProductsFind();
                break;
            case 3:
                break;
        }
    } while (nChoice != 3);
}

/* 3.1.1 Stock: Unidades en stock dado un productcode */
int ProductsStock(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret;
    SQLRETURN ret2;
    char x[BufferLength] = "\0";
    char y[BufferLength] = "\0";
    char query[BufferLength + 256];

    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    printf("Enter productcode > ");
    (void) fflush(stdout);
    if (fgets(x, (int) sizeof(x), stdin) != NULL) {
        x[strcspn(x, "\r\n")] = '\0';

        (void) sprintf(query, "SELECT quantityinstock FROM products WHERE productcode = '%s';", x);

        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);
        (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) y, BufferLength, NULL);

        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s\n", y);
        }

        ret2 = SQLCloseCursor(stmt);
        if (!SQL_SUCCEEDED(ret2)) {
            odbc_extract_error("", stmt, SQL_HANDLE_STMT);
            return ret;
        }
    }
    printf("\n");

    ret2 = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (!SQL_SUCCEEDED(ret2)) {
        odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
    }

    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* 3.1.2 Find: Buscar productos por nombre ordenados por productcode */
int ProductsFind(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret;
    SQLRETURN ret2;
    char x[BufferLength] = "\0";
    char code[BufferLength] = "\0";
    char name[BufferLength] = "\0";
    char query[BufferLength + 256];

    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    printf("Enter productname > ");
    (void) fflush(stdout);
    if (fgets(x, (int) sizeof(x), stdin) != NULL) {
        x[strcspn(x, "\r\n")] = '\0';

        (void) sprintf(query, "SELECT p.productcode, p.productname FROM products p WHERE p.productname LIKE '%%%s%%' ORDER BY p.productcode;", x);

        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

        (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) code, BufferLength, NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*) name, BufferLength, NULL);

        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s\n", code, name);
        }

        ret2 = SQLCloseCursor(stmt);
        if (!SQL_SUCCEEDED(ret2)) {
            odbc_extract_error("", stmt, SQL_HANDLE_STMT);
            return ret;
        }
    }
    printf("\n");

    ret2 = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (!SQL_SUCCEEDED(ret2)) {
        odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
    }

    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}