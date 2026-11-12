CC = gcc -g
CFLAGS = -Wall -Wextra -pedantic -ansi
LDLIBS = -lodbc

HEADERS = odbc.h products.h orders.h customers.h
EXE = menu
OBJ = menu.o odbc.o products.o orders.o customers.o

compile: $(EXE)


all: compile
	@echo "Borrando y creando la base de datos..."
	dropdb -U alumnodb test01 --if-exists
	createdb -U alumnodb test01
	psql -U alumnodb -d test01 -f classicmodels.sql

%.o: %.c $(HEADERS)
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXE): $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LDLIBS)

clean:
	rm -f *.o core $(EXE)


