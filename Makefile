CC=g++

CFLAGS=
RELFLAGS=-O2
DEVFLAGS=-g -Wall

SRC=main.cpp
EXEC=main

.PHONY: rel
rel: $(SRC)
	@echo "Release Build"
	$(CC) $(CFLAGS) $(RELFLAGS) $^ -o $(EXEC)

.PHONY: devel
devel: $(SRC)
	@echo "Development (DEBUG) Build"
	$(CC) $(CLFAGS) $(DEVFLAGS) $^ -o $(EXEC)

.PHONY: run
run: devel
	@echo "Executing..."
	@./$(EXEC)

.PHONY: clean
clean:
	@echo "CLEANING"
	@rm -rf $(EXEC)
	@echo "Done"
