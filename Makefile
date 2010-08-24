SRC        := src
OBJ        := build/obj
BIN        := bin

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


MODULES    := VGL
VGL				 := build/VGL

OBJS       := \
							$(OBJ)/camera.o \
							$(OBJ)/model.o \
							$(OBJ)/objviewer.o \
							$(OBJ)/renderer.o \
							$(OBJ)/resources.o \
							$(OBJ)/vector.o

TARGET     := $(BIN)/objviewer


ifeq ($(OSTYPE), linux-gnu)
CXX        := g++
CXXFLAGS   := -Wall -fmessage-length=0 -m64
CC         := gcc
CCFLAGS    := $(CXXFLAGS)
LD         := g++
LDFLAGS    := -m64 -fopenmp -Wl,--rpath,\$$ORIGIN
CMAKE      := cmake
INCLUDE	   := -I$(VGL)/include
LIBS       := -L$(VGL)/lib -lm -lglut -lpthread -lvgl
VGL_LIB		 := $(VGL)/lib/libvgl.so
else
CXX        := g++
CXXFLAGS   := -Wall -isysroot /Developer/SDKs/MacOSX10.6.sdk -arch x86_64 -msse4.2 -mfpmath=sse
CC         := gcc
CCFLAGS    := $(CXXFLAGS)
LD         := g++
LDFLAGS    := -framework OpenGL -framework GLUT -Wl,-syslibroot,/Developer/SDKs/MacOSX10.6.sdk -arch x86_64 -fopenmp -Wl,-rpath,@loader_path/
CMAKE      := cmake
INCLUDE	   := -I$(VGL)/include
LIBS       := -L$(VGL)/lib -lm -lvgl
VGL_LIB    := $(VGL)/lib/libvgl.dylib
endif



.PHONY: debug
debug:
	$(MAKE) CXXFLAGS="$(DBGFLAGS) $(CXXFLAGS)" CCFLAGS="$(DBGFLAGS) $(CCFLAGS)" all


.PHONY: release
release:
	$(MAKE) CXXFLAGS="$(OPTFLAGS) $(CXXFLAGS)" CCFLAGS="$(OPTFLAGS) $(CXXFLAGS)" all


.PHONY: all
all: dirs $(MODULES) $(TARGET)


.PHONY: clean
clean:
	rm -rf $(BIN) $(OBJ) *.linkinfo


.PHONY: allclean
allclean: clean
	rm -rf $(VGL)


.PHONY: dirs
dirs:
	@mkdir -p $(VGL)
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)


.PHONY: $(MODULES)
VGL: dirs
	@cd $(VGL) && cmake -D CMAKE_INSTALL_PREFIX=. ../../VGL && $(MAKE) install
	@cp $(VGL_LIB) $(BIN)


$(TARGET): $(MODULES) $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)


$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

