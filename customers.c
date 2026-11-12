#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "customers.h"

static int CustomersFind(void);
static int CustomersListProducts(void);
static int CustomersBalance(void);

static int ShowCustomersSubMenu(void) {
    int nSelected = 0;
    char buf[16];

    do {
        printf(" (1) Find\n"
               " (2) List Products\n"
               " (3) Balance\n"
               " (4) Back\n\n"
               "Enter a number that corresponds to your choice > ");
        if (!fgets(buf, 16, stdin))
            nSelected = 0;
        else
            nSelected = atoi(buf);
        printf("\n");

        if ((nSelected < 1) || (nSelected > 4)) {
            printf("You have entered an invalid choice. Please try again\n\n\n");
        }
    } while ((nSelected < 1) || (nSelected > 4));

    return nSelected;
}

void ShowCustomersMenu(void) {
    int nChoice = 0;
    do {
        nChoice = ShowCustomersSubMenu();
        switch (nChoice) {
            case 1:
                (void) CustomersFind();
                break;
            case 2:
                (void) CustomersListProducts();
                break;
            case 3:
                (void) CustomersBalance();
                break;
            case 4:
                break;
        }
    } while (nChoice != 4);
}


static int CustomersFind(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    SQLRETURN ret2; /* ODBC API return status */
    #define BufferLength 512
    char x[BufferLength] = "\0";
    char param[1024];
    char custnum[BufferLength] = "\0";
    char custname[BufferLength] = "\0";
    char firstname[BufferLength] = "\0";
    char lastname[BufferLength] = "\0";

    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    ret = SQLPrepare(stmt, (SQLCHAR*) "SELECT customernumber, customername, contactfirstname, contactlastname FROM customers WHERE contactfirstname LIKE ? OR contactlastname LIKE ? ORDER BY customernumber", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    printf("Enter customer name > ");
    (void) fflush(stdout);
    if (fgets(x, BufferLength, stdin) != NULL) {
        x[strcspn(x, "\r\n")] = '\0';
        /*@ -bufferoverflowhigh @*/
        sprintf(param, "%%%s%%", x);
        /*@ =bufferoverflowhigh @*/

        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR,
                                0, 0, param, 0, NULL);
        (void) SQLBindParameter(stmt, 2, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR,
                                0, 0, param, 0, NULL);

        (void) SQLExecute(stmt);
        
        (void) SQLBindCol(stmt, 1, SQL_C_CHAR,(SQLCHAR *) custnum, BufferLength, NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR,(SQLCHAR *) custname, BufferLength, NULL);
        (void) SQLBindCol(stmt, 3, SQL_C_CHAR,(SQLCHAR *) firstname, BufferLength, NULL);
        (void) SQLBindCol(stmt, 4, SQL_C_CHAR,(SQLCHAR *) lastname, BufferLength, NULL);

        /* Loop through the rows in the result-set */
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s %s %s\n", custnum, custname, firstname, lastname);
        }

        (void) SQLCloseCursor(stmt);
    }
    printf("\n");

    /* free up statement handle */
    ret2 = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (!SQL_SUCCEEDED(ret2)) {
        odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
    }

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


static int CustomersListProducts(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    SQLRETURN ret2; /* ODBC API return status */
    #define BufferLength 512
    char x[BufferLength] = "\0";
    char prodname[BufferLength] = "\0";
    char totalqty[BufferLength] = "\0";

    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    ret = SQLPrepare(stmt, (SQLCHAR*) "SELECT p.productname, SUM(od.quantityordered) FROM products p JOIN orderdetails od ON p.productcode = od.productcode JOIN orders o ON od.ordernumber = o.ordernumber WHERE o.customernumber = ? GROUP BY p.productcode, p.productname ORDER BY p.productcode", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    printf("Enter customer number > ");
    (void) fflush(stdout);
    if (fgets(x, BufferLength, stdin) != NULL) {
        x[strcspn(x, "\r\n")] = '\0';

        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR,
                                0, 0, x, 0, NULL);

        (void) SQLExecute(stmt);
        
        (void) SQLBindCol(stmt, 1, SQL_C_CHAR,(SQLCHAR *) prodname, BufferLength, NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR,(SQLCHAR *) totalqty, BufferLength, NULL);

        /* Loop through the rows in the result-set */
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s\n", prodname, totalqty);
        }

        (void) SQLCloseCursor(stmt);
    }
    printf("\n");

    /* free up statement handle */
    ret2 = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (!SQL_SUCCEEDED(ret2)) {
        odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
    }

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


static int CustomersBalance(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    SQLRETURN ret2; /* ODBC API return status */
    #define BufferLength 512
    char x[BufferLength] = "\0";
    char balance[BufferLength] = "\0";

    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    ret = SQLPrepare(stmt, (SQLCHAR*) "SELECT (SELECT COALESCE(SUM(amount), 0) FROM payments WHERE customernumber = ?) - (SELECT COALESCE(SUM(od.quantityordered * od.priceeach), 0) FROM orders o JOIN orderdetails od ON o.ordernumber = od.ordernumber WHERE o.customernumber = ?) AS balance", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    printf("Enter customer number > ");
    (void) fflush(stdout);
    if (fgets(x, BufferLength, stdin) != NULL) {
        x[strcspn(x, "\r\n")] = '\0';

        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR,
                                0, 0, x, 0, NULL);
        (void) SQLBindParameter(stmt, 2, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR,
                                0, 0, x, 0, NULL);

        (void) SQLExecute(stmt);
        
        (void) SQLBindCol(stmt, 1, SQL_C_CHAR,(SQLCHAR *) balance, BufferLength, NULL);

        
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("Balance = %s\n", balance);
        }

        (void) SQLCloseCursor(stmt);
    }
    printf("\n");

    /* free up statement handle */
    ret2 = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (!SQL_SUCCEEDED(ret2)) {
        odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
    }

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}