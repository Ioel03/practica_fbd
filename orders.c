#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "orders.h"

#define BufferLength 512

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

/* 3.2.1 Open: Pedidos con shippeddate sin asignar (NULL) */
int OrdersOpen(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret;
    SQLRETURN ret2;
    char ordernum[BufferLength] = "\0";
    char query[BufferLength];

    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    (void) sprintf(query, "SELECT ordernumber FROM orders WHERE shippeddate IS NULL ORDER BY ordernumber;");

    (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);
    (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) ordernum, BufferLength, NULL);

    while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
        printf("%s\n", ordernum);
    }

    ret2 = SQLCloseCursor(stmt);
    if (!SQL_SUCCEEDED(ret2)) {
        odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
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

/* 3.2.2 Range: Pedidos solicitados entre dos fechas (YYYY-MM-DD - YYYY-MM-DD) */
int OrdersRange(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret;
    SQLRETURN ret2;
    char x[BufferLength] = "\0";
    char date1[32] = "\0";
    char date2[32] = "\0";
    char ordernum[BufferLength] = "\0";
    char orderdate[BufferLength] = "\0";
    char shippeddate[BufferLength] = "\0";
    SQLLEN ind_shipped;
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

    printf("Enter dates (YYYY-MM-DD - YYYY-MM-DD) > ");
    (void) fflush(stdout);
    if (fgets(x, (int) sizeof(x), stdin) != NULL) {
        x[strcspn(x, "\r\n")] = '\0';
        (void) sscanf(x, "%10s - %10s", date1, date2);

        (void) sprintf(query, "SELECT ordernumber, orderdate, shippeddate FROM orders WHERE orderdate >= '%s' AND orderdate <= '%s' ORDER BY ordernumber;", date1, date2);

        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

        (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) ordernum, BufferLength, NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*) orderdate, BufferLength, NULL);
        (void) SQLBindCol(stmt, 3, SQL_C_CHAR, (SQLCHAR*) shippeddate, BufferLength, &ind_shipped);

        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            if (ind_shipped == SQL_NULL_DATA) {
                strcpy(shippeddate, "NULL");
            }
            printf("%s %s %s\n", ordernum, orderdate, shippeddate);
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

/* 3.2.3 Detail: Detalles, coste total y desglose de un pedido */
int OrdersDetail(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret;
    SQLRETURN ret2;
    char x[BufferLength] = "\0";
    char orderdate[BufferLength] = "\0";
    char status[BufferLength] = "\0";
    char totalcost[BufferLength] = "\0";
    char prodcode[BufferLength] = "\0";
    char qty[BufferLength] = "\0";
    char price[BufferLength] = "\0";
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

    printf("Enter ordernumber > ");
    (void) fflush(stdout);
    if (fgets(x, (int) sizeof(x), stdin) != NULL) {
        x[strcspn(x, "\r\n")] = '\0';

        /* Cabecera y coste total */
        (void) sprintf(query,
            "SELECT o.orderdate, o.status, SUM(od.quantityordered * od.priceeach) "
            "FROM orders o JOIN orderdetails od ON o.ordernumber = od.ordernumber "
            "WHERE o.ordernumber = %s "
            "GROUP BY o.orderdate, o.status;", x);

        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);
        (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) orderdate, BufferLength, NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*) status, BufferLength, NULL);
        (void) SQLBindCol(stmt, 3, SQL_C_CHAR, (SQLCHAR*) totalcost, BufferLength, NULL);

        if (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("Order Date=%s - status=%s\n", orderdate, status);
            printf("Total sum = %s\n", totalcost);
        }

        ret2 = SQLCloseCursor(stmt);
        if (!SQL_SUCCEEDED(ret2)) {
            odbc_extract_error("", stmt, SQL_HANDLE_STMT);
            return ret;
        }

        /* Lineas de productos ordenadas por orderlinenumber */
        (void) sprintf(query,
            "SELECT productcode, quantityordered, priceeach "
            "FROM orderdetails "
            "WHERE ordernumber = %s "
            "ORDER BY orderlinenumber;", x);

        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);
        (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) prodcode, BufferLength, NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*) qty, BufferLength, NULL);
        (void) SQLBindCol(stmt, 3, SQL_C_CHAR, (SQLCHAR*) price, BufferLength, NULL);

        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s %s\n", prodcode, qty, price);
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