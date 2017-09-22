IDIR=-Iinc -I/usr/include
CC=g++
CFLAGS=-std=c++0x $(IDIR) -Wno-write-strings

ODIR=obj
LDIR =-L../lib -L/usr/lib -L/usr/lib/nvidia-375

LIBS=-lm -lglut -lGLEW -lGL -lGLU -lX11

_DEPS = Model.hpp ModelFactory.hpp Camera.hpp
DEPS = $(patsubst %, $(IDIR)/%, $(_DEPS))

_OBJ = main.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ))

$(ODIR)/%.o: src/%.cpp #$(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

exec: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ exec
