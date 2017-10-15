IDIR=-Iinc -I/usr/include -I/usr/include/eigen3/
CC=g++
CFLAGS=-std=c++0x $(IDIR) -Wno-write-strings # --verbose

ODIR=obj
LDIR =-L../lib -L/usr/lib -L/usr/lib/nvidia-375

LIBS=-lm -lglut -lGLEW -lGL -lGLU -lX11

_DEPS = Camera.hpp Tree.hpp TreeTestBootstrapper.hpp
DEPS = $(patsubst %, $(IDIR)/%, $(_DEPS))

_MOTION_VIEWER_OBJ = motionViewer.o
MOTION_VIEWER_OBJ = $(patsubst %, $(ODIR)/%, $(_MOTION_VIEWER_OBJ))

$(ODIR)/%.o: src/%.cpp #$(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

default: all

motionViewer: $(MOTION_VIEWER_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

all: motionViewer

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ motionViewer
