# ----------------------------------------------------------------
#                                                               //
#   common.mk                                                   //
#                                                               //
#   This file is part of the Amber project                      //
#   http://www.opencores.org/project,amber                      //
#                                                               //
#   Description                                                 //
#   Contains common makefile code.                              //
#                                                               //
#   Author(s):                                                  //
#       - Conor Santifort, csantifort.amber@gmail.com           //
#                                                               //
#/ ///////////////////////////////////////////////////////////////
#                                                               //
#  Copyright (C) 2010 Authors and OPENCORES.ORG                 //
#                                                               //
#  This source file may be used and distributed without         //
#  restriction provided that this copyright statement is not    //
#  removed from the file and that any derivative work contains  //
#  the original copyright notice and the associated disclaimer. //
#                                                               //
#  This source file is free software; you can redistribute it   //
#  and/or modify it under the terms of the GNU Lesser General   //
#  Public License as published by the Free Software Foundation; //
#  either version 2.1 of the License, or (at your option) any   //
#  later version.                                               //
#                                                               //
#  This source is distributed in the hope that it will be       //
#  useful, but WITHOUT ANY WARRANTY; without even the implied   //
#  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      //
#  PURPOSE.  See the GNU Lesser General Public License for more //
#  details.                                                     //
#                                                               //
#  You should have received a copy of the GNU Lesser General    //
#  Public License along with this source; if not, download it   //
#  from http://www.opencores.org/lgpl.shtml                     //
#                                                               //
# ----------------------------------------------------------------

TOOLSPATH        = ../tools
LDS              = ../include/sections.lds
LIBPATH          = ../lib
AMBER_CROSSTOOL ?= arm-linux-gnueabi

AS       = $(AMBER_CROSSTOOL)-as
CC       = $(AMBER_CROSSTOOL)-gcc
CXX      = $(AMBER_CROSSTOOL)-g++
AR       = $(AMBER_CROSSTOOL)-ar
LD       = $(AMBER_CROSSTOOL)-ld
DS       = $(AMBER_CROSSTOOL)-objdump
OC       = $(AMBER_CROSSTOOL)-objcopy
ELF      = $(TOOLSPATH)/amber-elfsplitter
TCODE32  = $(TOOLSPATH)/amber-code32.sh

MEM       = $(addsuffix .mem, $(basename $(TGT)))
DIS       = $(addsuffix .dis, $(basename $(TGT)))
OBJ       = $(addsuffix .o,   $(basename $(SRC)))
CODE32HEX = p.txt
LIBOBJ    = $(LIBPATH)/start.o


ifndef TGT
    TGT = aout.elf
endif

MAP = $(addsuffix .map, $(basename $(TGT))) 

ifdef LDS
    TLDS = -T $(LDS)
else
    TLDS = 
endif

ifdef MIN_SIZE
    # optimize for size
    OPTIMIZE = -Os
else
    # optimize for speed
    OPTIMIZE = -O3
endif

 CFLAGS  = -c $(OPTIMIZE) -std=c99 -march=armv2a -mno-thumb-interwork -ffreestanding -fif-conversion2
 DSFLAGS = -C -S -EL
 LDFLAGS = -Bstatic -Map $(MAP) --strip-debug --fix-v4bx

all:  $(ELF) $(CODE32HEX) $(DIS) 

$(MEM): $(TGT)
	$(ELF) $(TGT) > $(MEM)

$(TGT): $(OBJ) $(LIBOBJ)
	$(LD) $(LDFLAGS) -o $(TGT) $(TLDS) $(LIBOBJ) $(OBJ) 
	$(OC) -R .comment -R .note $(TGT)

$(ELF):
	$(MAKE) -s -C $(TOOLSPATH)

$(LIBOBJ):
	$(MAKE) -s -C $(LIBPATH)

$(DIS): $(TGT)
	$(DS) $(DSFLAGS) $^ > $@

$(CODE32HEX): $(MEM)
	$(TCODE32) $(MEM) $(CODE32HEX)


clean:
	@rm -rfv *.o *.elf *.flt *.gdb *.dis *.map *.mem *.v $(CODE32HEX)

