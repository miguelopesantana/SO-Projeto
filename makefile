CC = gcc
FLAGS = -Wall -g -pthread

SERVER = offloading
SOBJS = SystemManager.o TaskManager.o EdgeServer.o Monitor.o

CLIENT = mobile_node
COBJS = MobileNode.o

all: ${SERVER}
all: ${CLIENT}

cleans:
		rm ${SOBJS} *~ ${SERVER}
		
cleanc:
		rm ${COBJS} *~ ${CLIENT}

${SERVER}:	${SOBJS}
		${CC} ${FLAGS} ${SOBJS} -lm -o $@

${CLIENT}:	${COBJS}
		${CC} ${FLAGS} ${COBJS} -lm -o $@		

.c.o:
		${CC} ${FLAGS} $< -c -o $@

######################
SystemManager.o: SystemManager.c Main_header.h
TaskManager.o: TaskManager.c Main_header.h
EdgeServer.o: EdgeServer.c Main_header.h
Monitor.o: Monitor.c Main_header.h
MaintenanceManager.o: MaintenanceManager.c Main_header.h
MobileNode.o: MobileNode.o MobileNode_header.h
