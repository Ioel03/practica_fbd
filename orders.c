#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "orders.h"

static int OrdersOpen(void);
static int OrdersRange(void);
static int OrdersDetail(void);

static int ShowOrdersSubMenu(void) {
    int nSelected = 0;
    char buf[16];

    do {
        printf(" (1) Open\n"
               " (2) Range\n"
               " (3) Detail\n"
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

void ShowOrdersMenu(void) {
    int nChoice = 0;
    do {
        nChoice = ShowOrdersSubMenu();
        switch (nChoice) {
            case 1:
                (void) OrdersOpen();
                break;
            case 2:
                (void) OrdersRange();
                break;
            case 3:
                (void) OrdersDetail();
                break;
            case 4:
                break;
        }
    } while (nChoice != 4);
}


static int OrdersOpen(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    SQLRETURN ret2; /* ODBC API return status */
    #define BufferLength 512
    char ordernum[BufferLength] = "\0";

    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    ret = SQLPrepare(stmt, (SQLCHAR*) "SELECT ordernumber FROM orders WHERE shippeddate IS NULL ORDER BY ordernumber", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    (void) SQLExecute(stmt);
    
    (void) SQLBindCol(stmt, 1, SQL_C_CHAR,(SQLCHAR *) ordernum, BufferLength, NULL);

    
    while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
        printf("%s\n", ordernum);
    }

    (void) SQLCloseCursor(stmt);
    printf("\n");

    
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


static int OrdersRange(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    SQLRETURN ret2; /* ODBC API return status */
    #define BufferLength 512
    char x[BufferLength] = "\0";
    char date1[32] = "\0";
    char date2[32] = "\0";
    char ordernum[BufferLength] = "\0";
    char orderdate[BufferLength] = "\0";
    char shippeddate[BufferLength] = "\0";
    SQLLEN ind_shipped = 0;

    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    ret = SQLPrepare(stmt, (SQLCHAR*) "SELECT ordernumber, orderdate, shippeddate FROM orders WHERE orderdate >= ? AND orderdate <= ? ORDER BY ordernumber", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    printf("Enter dates (YYYY-MM-DD - YYYY-MM-DD) > ");
    (void) fflush(stdout);
    if (fgets(x, BufferLength, stdin) != NULL) {
        x[strcspn(x, "\r\n")] = '\0';
        (void) sscanf(x, "%10s - %10s", date1, date2);

        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR,
                                0, 0, date1, 0, NULL);
        (void) SQLBindParameter(stmt, 2, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR,
                                0, 0, date2, 0, NULL);

        (void) SQLExecute(stmt);
        
        (void) SQLBindCol(stmt, 1, SQL_C_CHAR,(SQLCHAR *) ordernum, BufferLength, NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR,(SQLCHAR *) orderdate, BufferLength, NULL);
        (void) SQLBindCol(stmt, 3, SQL_C_CHAR,(SQLCHAR *) shippeddate, BufferLength, &ind_shipped);

        
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            if (ind_shipped == SQL_NULL_DATA) {
                strcpy(shippeddate, "NULL");
            }
            printf("%s %s %s\n", ordernum, orderdate, shippeddate);
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


static int OrdersDetail(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    SQLRETURN ret2; /* ODBC API return status */
    #define BufferLength 512
    char x[BufferLength] = "\0";
    char orderdate[BufferLength] = "\0";
    char status[BufferLength] = "\0";
    char totalcost[BufferLength] = "\0";
    char prodcode[BufferLength] = "\0";
    char qty[BufferLength] = "\0";
    char price[BufferLength] = "\0";

    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    printf("Enter ordernumber > ");
    (void) fflush(stdout);
    if (fgets(x, BufferLength, stdin) != NULL) {
        x[strcspn(x, "\r\n")] = '\0';

        ret = SQLPrepare(stmt, (SQLCHAR*) "SELECT o.orderdate, o.status, SUM(od.quantityordered * od.priceeach) FROM orders o JOIN orderdetails od ON o.ordernumber = od.ordernumber WHERE o.ordernumber = ? GROUP BY o.orderdate, o.status", SQL_NTS);
        
        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR,
                                0, 0, x, 0, NULL);

        (void) SQLExecute(stmt);
        
        (void) SQLBindCol(stmt, 1, SQL_C_CHAR,(SQLCHAR *) orderdate, BufferLength, NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR,(SQLCHAR *) status, BufferLength, NULL);
        (void) SQLBindCol(stmt, 3, SQL_C_CHAR,(SQLCHAR *) totalcost, BufferLength, NULL);

        
        if (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("Order Date=%s - status=%s\n", orderdate, status);
            printf("Total sum = %s\n", totalcost);
        }

        (void) SQLCloseCursor(stmt);

        ret = SQLPrepare(stmt, (SQLCHAR*) "SELECT productcode, quantityordered, priceeach FROM orderdetails WHERE ordernumber = ? ORDER BY orderlinenumber", SQL_NTS);
        
        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR,
                                0, 0, x, 0, NULL);

        (void) SQLExecute(stmt);
        
        (void) SQLBindCol(stmt, 1, SQL_C_CHAR,(SQLCHAR *) prodcode, BufferLength, NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR,(SQLCHAR *) qty, BufferLength, NULL);
        (void) SQLBindCol(stmt, 3, SQL_C_CHAR,(SQLCHAR *) price, BufferLength, NULL);

        
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s %s\n", prodcode, qty, price);
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