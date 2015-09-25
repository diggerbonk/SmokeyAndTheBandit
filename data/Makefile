#
# src/tslib/util/Makefile.  Generated from Makefile.in by configure.
#
# File     : $Id: Makefile.in,v 1.6 2006-01-23 04:18:06 cvs Exp $
# Team     : Trent McNair
# Created  : May, 2005 
#
# This is the Makefile for the utils directory.

#
# BUILD TOOLS

CC = gconvert

SOURCES = $(wildcard *.xml) 

#
# OBJECTS

OBJECTS = $(patsubst %.xml, ,$(patsubst %.xml, %.inc, $(SOURCES)))

#
# BUILD TARGETS

.SUFFIXES:
.SUFFIXES: .xml .inc

.xml.inc:
	$(CC)  $^ 


.PHONY: all clean 

all: done.txt
        
done.txt: $(OBJECTS) $(SOURCES)
	touch done.txt

clean:
	rm done.txt
	rm -f *.inc

