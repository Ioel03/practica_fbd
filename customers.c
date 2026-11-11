#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "customers.h"

#define BufferLength 512

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

/* 3.3.1 Find: Buscar clientes por contactfirstname o contactlastname */
int CustomersFind(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret;
    SQLRETURN ret2;
    char x[BufferLength] = "\0";
    char custnum[BufferLength] = "\0";
    char custname[BufferLength] = "\0";
    char firstname[BufferLength] = "\0";
    char lastname[BufferLength] = "\0";
    char query[2048];

    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    printf("Enter customer name > ");
    (void) fflush(stdout);
    if (fgets(x, (int) sizeof(x), stdin) != NULL) {
        x[strcspn(x, "\r\n")] = '\0';

        (void) sprintf(query,
            "SELECT customernumber, customername, contactfirstname, contactlastname "
            "FROM customers "
            "WHERE contactfirstname LIKE '%%%s%%' OR contactlastname LIKE '%%%s%%' "
            "ORDER BY customernumber;", x, x);

        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

        (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) custnum, BufferLength, NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*) custname, BufferLength, NULL);
        (void) SQLBindCol(stmt, 3, SQL_C_CHAR, (SQLCHAR*) firstname, BufferLength, NULL);
        (void) SQLBindCol(stmt, 4, SQL_C_CHAR, (SQLCHAR*) lastname, BufferLength, NULL);

        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s %s %s\n", custnum, custname, firstname, lastname);
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

/* 3.3.2 List Products: Productos solicitados por un cliente y suma de unidades */
int CustomersListProducts(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret;
    SQLRETURN ret2;
    char x[BufferLength] = "\0";
    char prodname[BufferLength] = "\0";
    char totalqty[BufferLength] = "\0";
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

    printf("Enter customer number > ");
    (void) fflush(stdout);
    if (fgets(x, (int) sizeof(x), stdin) != NULL) {
        x[strcspn(x, "\r\n")] = '\0';

        (void) sprintf(query,
            "SELECT p.productname, SUM(od.quantityordered) "
            "FROM products p "
            "JOIN orderdetails od ON p.productcode = od.productcode "
            "JOIN orders o ON od.ordernumber = o.ordernumber "
            "WHERE o.customernumber = %s "
            "GROUP BY p.productcode, p.productname "
            "ORDER BY p.productcode;", x);

        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

        (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) prodname, BufferLength, NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*) totalqty, BufferLength, NULL);

        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s\n", prodname, totalqty);
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

/* 3.3.3 Balance: Suma de pagos menos suma de productos comprados */
int CustomersBalance(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret;
    SQLRETURN ret2;
    char x[BufferLength] = "\0";
    char balance[BufferLength] = "\0";
    char query[2048];

    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    printf("Enter customer number > ");
    (void) fflush(stdout);
    if (fgets(x, (int) sizeof(x), stdin) != NULL) {
        x[strcspn(x, "\r\n")] = '\0';

        (void) sprintf(query,
            "SELECT "
            "(SELECT COALESCE(SUM(amount), 0) FROM payments WHERE customernumber = %s) - "
            "(SELECT COALESCE(SUM(od.quantityordered * od.priceeach), 0) "
            " FROM orders o JOIN orderdetails od ON o.ordernumber = od.ordernumber "
            " WHERE o.customernumber = %s) AS balance;", x, x);

        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

        (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) balance, BufferLength, NULL);

        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("Balance = %s\n", balance);
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