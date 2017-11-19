# NOTICE: The majority of this code was written by Dale Schuurmans, and was copied and pasted with his permission

FLAGS = -std=c++11 -I/usr/include -I/usr/include/eigen3 -L/usr/lib -L/usr/lib/nvidia-375 -lglut -lGLEW -lGL -lGLU -lX11 -Wno-write-strings

default: all

all: modelSolidViewer personViewer

# viewers

modelSolidViewer: modelSolidViewer.cpp camera.h getbmp.h light.h material.h mesh.* path.h texture.*
	g++ getbmp.cpp modelSolidViewer.cpp mesh.cpp texture.cpp -o modelSolidViewer $(FLAGS)

personViewer: personViewer.cpp light.h camera.h getbmp.h timer.h mesh.* texture.* skeleton.* motion.* bvh_util.* attachment.*
	g++ getbmp.cpp personViewer.cpp mesh.cpp texture.cpp skeleton.cpp motion.cpp bvh_util.cpp attachment.cpp -o personViewer $(FLAGS)

%: %.cpp
	g++ $*.cpp -o $* $(FLAGS)

clean:
	rm *Viewer
