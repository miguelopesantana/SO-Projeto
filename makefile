CC = gcc
FLAGS = -Wall -g -pthread
OBJS = SystemManager.o functions.o MobileNode.o
PROG = server

all: 	${PROG}

clean:
	rm ${OBJS} *- ${PROG}

${PROG}:	${OBJS}
		${CC} ${FLAGS} ${OBJS} -o $@

.c.o:
		${CC} ${FLAGS} $< -c -o $@

######################
main.o: udp_adminServer.c server_header.h
functions.o: functions.c server_header.h
main: udp_adminServer.o functions.o
