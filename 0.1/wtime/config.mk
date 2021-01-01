# Customize to fit your system

PREFIX	= /usr/local

INCDIR  = ${PREFIX}/include
LIBDIR  = ${PREFIX}/lib

INCLUDES = -I. -I${INCDIR} -I/usr/include
LIBS     = -L${LIBDIR} -L/usr/lib -lc

CFLAGS  = -O0 -g -Wall -pedantic -std=gnu99 -W -D_DEBUG -g3 ${INCLUDES}
LDFLAGS = -g -Wl  ${LIBS}
#CFLAGS   = -Os ${INCLUDES}
#LDFLAGS = ${LIBS} 

CC = gcc
CP = cp -f
RM = rm -f
MKDIR = mkdir
