# Customize to fit your system

PREFIX = /usr/local

INCDIR = ${PREFIX}/include
LIBDIR = ${PREFIX}/lib

INCLUDES = -I. -I${INCDIR} -I/usr/include
LIBS = -L${LIBDIR} -L/usr/lib -lc

CFLAGS = -std=c99 -O2 ${INCLUDES}
LDFLAGS = ${LIBS} 

CC = gcc
CP = cp -f
RM = rm -f
MKDIR = mkdir
