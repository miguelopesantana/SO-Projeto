CC := gcc
FLAGS := -Wall -g -pthread

PROG := offloading
OBJS := SystemManager.o TaskManager.o EdgeServer.o Monitor.o functions.o MobileNode.o


all: ${PROG}

clean:
		rm ${OBJS} *~ ${PROG}

${PROG}:	${OBJS}
		${CC} ${FLAGS} ${OBJS} -lm -o $@		

.c.o:
		${CC} ${FLAGS} $< -c -o $@

######################

SystemManager.o: SystemManager.c functions.c Main_header.h
TaskManager.o: TaskManager.c functions.c Main_header.h
EdgeServer.o: EdgeServer.c functions.c Main_header.h
Monitor.o: Monitor.c functions.c Main_header.h
MaintenanceManager.o: MaintenanceManager.c functions.c Main_header.h
functions.o: functions.c Main_header.h
MobileNode.o: MobileNode.o MobileNode_header.h
