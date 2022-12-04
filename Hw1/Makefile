CXX := g++
CXXFLAGS := -std=c++17

TARGET := server
OBJDIR := objects
SRC := $(wildcard *.cpp)
OBJ := $(patsubst %.cpp, %.o, ${SRC})

.PHONY: all clean

all: ${TARGET}

${TARGET}: ${OBJ}
	${CXX} $^ ${CXXFLAGS} -o $@

%.o: %.cpp
	${CXX} $^ ${CXXFLAGS} -c -o $@

clean:
	-rm ${TARGET}
	-rm ${OBJ}
