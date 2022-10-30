EXECS=matrixmulti
MPICC?=mpicc

all: ${EXECS}

matrixmulti: matrixmulti.c
	${MPICC} -o matrixmulti matrixmulti.c

clean:
	rm -f ${EXECS}