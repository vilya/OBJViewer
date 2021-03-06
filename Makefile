SRC        := src
OBJ        := build/obj
BIN        := bin

THIRDPARTY_SRC := thirdparty
THIRDPARTY_OBJ := build/thirdparty_obj

TESTSRC    := test
TESTBIN    := build/test

OPTFLAGS   := -O3 -fopenmp
DBGFLAGS   := -g

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
IMAGELIB	 := ImageLib/dist

OBJS       := $(OBJ)/objviewer.o \
							$(OBJ)/model.o \
							$(OBJ)/renderer.o \
							$(OBJ)/vector.o \
							$(OBJ)/parser.o \
							$(OBJ)/plyparser.o \
							$(OBJ)/objparser.o \
							$(OBJ)/camera.o \
							$(OBJ)/resources.o

#							$(OBJ)/curve.o \
#							$(OBJ)/math3d.o \

THIRDPARTY_OBJS := $(THIRDPARTY_OBJ)/ply.o


TARGET     := $(BIN)/objviewer


ifeq ($(OSTYPE), linux-gnu)
CXX        := g++
CXXFLAGS   := -Wall -fmessage-length=0 -m64
CC         := gcc
CCFLAGS    := $(CXXFLAGS)
LD         := g++
LDFLAGS    := -m64 -fopenmp -Wl,--rpath,\$$ORIGIN
INCLUDE	   := -I$(IMAGELIB)/include -I$(THIRDPARTY_SRC)
LIBS       := -L$(IMAGELIB)/lib -lm -lglut -lpthread -limagelib
DYLIB_EXT	 := .so
IMAGELIB_LIB := $(IMAGELIB)/lib/libimagelib.so
else
IMAGELIB	 := ImageLib/dist
CXX        := g++
CXXFLAGS   := -Wall -isysroot /Developer/SDKs/MacOSX10.6.sdk -arch x86_64 -msse4.2 -mfpmath=sse
CC         := gcc
CCFLAGS    := $(CXXFLAGS)
LD         := g++
LDFLAGS    := -framework OpenGL -framework GLUT -Wl,-syslibroot,/Developer/SDKs/MacOSX10.6.sdk -arch x86_64 -fopenmp -Wl,-rpath,@loader_path/
INCLUDE	   := -I$(IMAGELIB)/include -I$(THIRDPARTY_SRC)
LIBS       := -L$(IMAGELIB)/lib -lm -limagelib
IMAGELIB_LIB := $(IMAGELIB)/lib/libimagelib.dylib
endif



.PHONY: debug
debug:
	$(MAKE) CXXFLAGS="$(DBGFLAGS) $(CXXFLAGS)" CCFLAGS="$(DBGFLAGS) $(CCFLAGS)" all


.PHONY: release
release:
	$(MAKE) CXXFLAGS="$(OPTFLAGS) $(CXXFLAGS)" CCFLAGS="$(OPTFLAGS) $(CXXFLAGS)" all


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
	cp $(IMAGELIB_LIB) $(BIN)


$(TARGET): $(MODULES) $(OBJS) $(THIRDPARTY_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(THIRDPARTY_OBJS) $(LIBS)


$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@


$(THIRDPARTY_OBJ)/ply.o: $(THIRDPARTY_SRC)/ply.c
	$(CC) $(CCFLAGS) $(INCLUDE) -c $< -o $@


$(TESTBIN)/math3dtest: $(TESTSRC)/math3dtest.cpp $(OBJ)/math3d.o
	$(CXX) $(CXXFLAGS) $(INCLUDE) -I$(SRC) $(LDFLAGS) -o $@ $^ $(LIBS)

