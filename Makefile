SRC        := src
OBJ        := build/obj
BIN        := bin

THIRDPARTY_SRC := thirdparty
THIRDPARTY_OBJ := build/thirdparty_obj

TESTSRC    := test
TESTBIN    := build/test


ifeq ($(OSTYPE), linux-gnu)
IMAGELIB	 := /home/vilya/ImageLib/dist
CC         := gcc
CCFLAGS    := -g -Wall -fmessage-length=0 -m64
CXX        := g++
CXXFLAGS   := -g -Wall -fmessage-length=0 -m64
LD         := g++
LDFLAGS    := -m64
INCLUDE	   := -I$(IMAGELIB)/include -I$(THIRDPARTY_SRC)
LIBS       := -L$(IMAGELIB)/lib -lm -lglut -lpthread -limagelib
else
IMAGELIB	 := /users/vilya/Code/ImageLib/dist
CC         := gcc
CCFLAGS    := -g -Wall -isysroot /Developer/SDKs/MacOSX10.6.sdk -arch x86_64
CXX        := g++
CXXFLAGS   := -g -Wall -isysroot /Developer/SDKs/MacOSX10.6.sdk -arch x86_64
LD         := g++
LDFLAGS    := -framework OpenGL -framework GLUT -Wl,-syslibroot,/Developer/SDKs/MacOSX10.6.sdk -arch x86_64
INCLUDE	   := -I$(IMAGELIB)/include -I$(THIRDPARTY_SRC)
LIBS       := -L$(IMAGELIB)/lib -lm -limagelib
endif


OBJS       := $(OBJ)/objviewer.o \
							$(OBJ)/model.o \
							$(OBJ)/renderer.o \
							$(OBJ)/math3d.o \
							$(OBJ)/parser.o \
							$(OBJ)/plyparser.o \
							$(OBJ)/objparser.o

THIRDPARTY_OBJS := $(THIRDPARTY_OBJ)/ply.o


TARGET     := $(BIN)/objviewer


.PHONY: all
all: dirs $(TARGET)


.PHONY: test
test: $(TESTBIN)/math3dtest
	$(TESTBIN)/math3dtest


.PHONY: clean
clean:
	rm -rf $(BIN)/* $(OBJ)/* $(THIRDPARTY_OBJ)/* $(TESTBIN)/* *.linkinfo


.PHONY: dirs
dirs:
	@mkdir -p $(OBJ)
	@mkdir -p $(THIRDPARTY_OBJ)
	@mkdir -p $(BIN)
	@mkdir -p $(TESTBIN)


$(TARGET): $(OBJS) $(THIRDPARTY_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)


$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@


$(THIRDPARTY_OBJ)/ply.o: $(THIRDPARTY_SRC)/ply.c
	$(CC) $(CCFLAGS) $(INCLUDE) -c $< -o $@


$(TESTBIN)/math3dtest: $(TESTSRC)/math3dtest.cpp $(OBJ)/math3d.o
	$(CXX) $(CXXFLAGS) $(INCLUDE) -I$(SRC) $(LDFLAGS) -o $@ $^ $(LIBS)

