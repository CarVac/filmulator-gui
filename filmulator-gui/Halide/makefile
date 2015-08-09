CC = g++
CFLAGS = -g -I include/
LIBS= -L bin/ -lHalide -lpthread -ldl -lpng
DEPS= halideFilmulate.h
OBJS= setupFilmulator.o	generateFilmulatedImage.o develop.o diffuse.o calcLayerMix.o \
			calcReservoirConcentration.o performLayerMix.o

all: runFilmulator

%.o: %.cpp $(DEPS)
	$(CC) $^ $(CFLAGS) $(LIBS) -o $(@:.o=)
	LD_LIBRARY_PATH=bin ./$(@:.o=)

runFilmulator: $(OBJS) $(DEPS) runFilmulator.cpp
	$(CC) runFilmulator.cpp $(OBJS) $(FILMULATEOBJS) $(CFLAGS) $(LIBS) -o runFilmulator 

.PHONY: clean printOBJS

clean:
	rm -f *.o $(OBJS:.o=.h) $(FILMULATEOBJS:.o=.h) $(OBJS:.o=) $(FILMULATEOBJS:.o=) filmulate

printOBJS:
	echo $(OBJS:.o=.h)
