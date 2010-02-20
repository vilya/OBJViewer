SRC        := src
OBJ        := build/obj
BIN        := bin

THIRDPARTY_SRC := thirdparty
THIRDPARTY_OBJ := build/thirdparty_obj

TESTSRC    := test
TESTBIN    := build/test

#OPTFLAGS   := -O3 -fopenmp
OPTFLAGS   := -g

# Linker options to check out:
# -dylib_file
# -dylinker_install_name
# -dynamic
# -dynamiclib
# -iframework
# -image_base
# -single_module
#
# Also:
# -msse, -msse2, ... -msse4.2


MODULES    := ImageLib

OBJS       := $(OBJ)/objviewer.o \
							$(OBJ)/model.o \
							$(OBJ)/renderer.o \
							$(OBJ)/math3d.o \
							$(OBJ)/parser.o \
							$(OBJ)/plyparser.o \
							$(OBJ)/objparser.o \
							$(OBJ)/camera.o \
							$(OBJ)/curve.o \
							$(OBJ)/resources.o

THIRDPARTY_OBJS := $(THIRDPARTY_OBJ)/ply.o


TARGET     := $(BIN)/objviewer


ifeq ($(OSTYPE), linux-gnu)
IMAGELIB	 := ImageLib/dist
CC         := gcc
CCFLAGS    := $(OPTFLAGS) -Wall -fmessage-length=0 -m64
CXX        := g++
CXXFLAGS   := $(OPTFLAGS) -Wall -fmessage-length=0 -m64
LD         := g++
LDFLAGS    := -m64 -fopenmp -rpath ../$(IMAGELIB)/lib
INCLUDE	   := -I$(IMAGELIB)/include -I$(THIRDPARTY_SRC)
LIBS       := -L$(IMAGELIB)/lib -lm -lglut -lpthread -limagelib
else
IMAGELIB	 := ImageLib/dist
CC         := gcc
CCFLAGS    := $(OPTFLAGS) -Wall -isysroot /Developer/SDKs/MacOSX10.6.sdk -arch x86_64
CXX        := g++
CXXFLAGS   := $(OPTFLAGS) -Wall -isysroot /Developer/SDKs/MacOSX10.6.sdk -arch x86_64
LD         := g++
LDFLAGS    := -framework OpenGL -framework GLUT -Wl,-syslibroot,/Developer/SDKs/MacOSX10.6.sdk -arch x86_64 -fopenmp -Wl,-rpath,@loader_path/../$(IMAGELIB)/lib
INCLUDE	   := -I$(IMAGELIB)/include -I$(THIRDPARTY_SRC)
LIBS       := -L$(IMAGELIB)/lib -lm -limagelib
endif


.PHONY: all
all: dirs $(TARGET)


.PHONY: test
test: $(TESTBIN)/math3dtest
	$(TESTBIN)/math3dtest


.PHONY: clean
clean:
	rm -rf $(BIN)/* $(OBJ)/* $(THIRDPARTY_OBJ)/* $(TESTBIN)/* *.linkinfo


.PHONY: allclean
allclean: clean
	$(MAKE) -C ImageLib clean


.PHONY: dirs
dirs:
	@mkdir -p $(OBJ)
	@mkdir -p $(THIRDPARTY_OBJ)
	@mkdir -p $(BIN)
	@mkdir -p $(TESTBIN)


.PHONY: $(MODULES)
ImageLib:
	$(MAKE) -C $@


$(TARGET): $(MODULES) $(OBJS) $(THIRDPARTY_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(THIRDPARTY_OBJS) $(LIBS)


$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@


$(THIRDPARTY_OBJ)/ply.o: $(THIRDPARTY_SRC)/ply.c
	$(CC) $(CCFLAGS) $(INCLUDE) -c $< -o $@


$(TESTBIN)/math3dtest: $(TESTSRC)/math3dtest.cpp $(OBJ)/math3d.o
	$(CXX) $(CXXFLAGS) $(INCLUDE) -I$(SRC) $(LDFLAGS) -o $@ $^ $(LIBS)

