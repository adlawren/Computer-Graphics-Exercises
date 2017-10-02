IDIR=-Iinc -I/usr/include -I/usr/include/eigen3/
CC=g++
CFLAGS=-std=c++0x $(IDIR) -Wno-write-strings # --verbose

ODIR=obj
LDIR =-L../lib -L/usr/lib -L/usr/lib/nvidia-375

LIBS=-lm -lglut -lGLEW -lGL -lGLU -lX11

_DEPS = Model.hpp ModelFactory.hpp Camera.hpp
DEPS = $(patsubst %, $(IDIR)/%, $(_DEPS))

_MODEL_VIEWER_OBJ = modelViewer.o
MODEL_VIEWER_OBJ = $(patsubst %, $(ODIR)/%, $(_MODEL_VIEWER_OBJ))

_MODEL_VIEWER_VBO_OBJ = modelViewerVBO.o
MODEL_VIEWER_VBO_OBJ = $(patsubst %, $(ODIR)/%, $(_MODEL_VIEWER_VBO_OBJ))

$(ODIR)/%.o: src/%.cpp #$(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

default: all

modelViewer: $(MODEL_VIEWER_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

modelViewerVBO: $(MODEL_VIEWER_VBO_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

all: modelViewer modelViewerVBO

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ modelViewer modelViewerVBO
