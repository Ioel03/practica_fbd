CC = gcc -g
CFLAGS = -Wall -Wextra -pedantic -ansi
LDLIBS = -lodbc

# Actualizamos las cabeceras y los objetos para incluir todos los módulos
HEADERS = odbc.h products.h orders.h customers.h
EXE = menu
OBJ = menu.o odbc.o products.o orders.o customers.o

# 1. Compilar el código (make compile)
compile: $(EXE)

# 2. Borrar, crear y poblar la base de datos (make all)
# Dependiendo de tu entorno, ajusta el comando psql. 
# Asumimos el usuario 'alumnodb' y la base de datos 'test01' que tienes en odbc.h
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


