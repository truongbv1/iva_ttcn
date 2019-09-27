

IDIR = ./inc 
SDIR = src
ODIR = obj
main = main.c


SRC = $(wildcard ${SDIR}/*.c)
src = $(filter-out $(main), $(SRC))
_OBJ = $(src:.c=.o)

BUILD = $(shell pwd)/.formosa/build
PRJ_DIR = $(shell pwd)

LDFLAGS += -lrt -lsysconf -lrtscam -lrtsnm -lpemsg -lneuralyzer -lm -lc

CFLAGS += -I$(IDIR) \
			-I$(DIR_TMPFS)/include

#DFLAGS += -L$(DIR_TMPFS)/lib -Wl,-Bdynamic -lrtsacodec -lpthread -ljson-c -lrtsv4l2 -lrtsamixer -lrtsosd \
			-lm -lavcodec -lrtsisp -lrtsjpeg -lrtscamkit -lrtsaec -lh1encoder \
			-lrtstream \
			-Wl,-rpath=$(DIR_TMPFS)/lib

DFLAGS += -L$(DIR_TMPFS)/lib -Wl,-Bdynamic -lavformat -lavdevice -lswscale -lavutil -ljson-c -lavfilter -lswresample -lavcodec -lm -lz \
		-lpthread -lrtsjpeg -lrtsisp -lh1encoder -lopencore-amrnb -lrtscamkit -lrtsmp3 -lrtstream -lrtsisp -lrt -lasound -lrtscamkit \
		-Wl,-rpath=$(DIR_TMPFS)/lib

TARGET = iva

all : clean $(TARGET) romfs

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c
	mkdir -p $(ODIR)/${SDIR}
	$(CC) -Wall -c $<  -o $@ $(CFLAGS) $(LDFLAGS) $(DFLAGS) # -Werror 

$(TARGET):main.c $(OBJ)
	mkdir -p $(BUILD)
	$(CC) -Wall $(CFLAGS) $^ -o $(BUILD)/iva $(DFLAGS) $(LDFLAGS) # -Werror
	rm -r $(ODIR)

romfs:
	$(ROMFSINST) $(BUILD)/iva /bin/iva

clean:
	$(RM) $(TARGET) *.o
	rm -rf $(BUILD)
