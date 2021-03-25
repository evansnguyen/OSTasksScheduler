#Phi Long Nguyen
#19247171

#DECLARE SECTION
CC = gcc
CFLAGS = -Wall -g
OBJ = scheduler.o queue.o generator.o
EXEC = Scheduler
################################################################################
#EXECUTABLE FILEs
Scheduler: $(OBJ)
	$(CC) $(OBJ) -pthread -o Scheduler
#################################################################################
#OBJECT FILEs
scheduler.o: scheduler.c queue.h scheduler.h generator.h
	$(CC) -c scheduler.c $(CFLAGS)
queue.o: queue.c queue.h
	$(CC) -c queue.c $(CFLAGS)
generator.o: generator.c generator.h
	$(CC) -c generator.c $(CFLAGS)
clean:
	rm -f $(EXEC) $(OBJ)