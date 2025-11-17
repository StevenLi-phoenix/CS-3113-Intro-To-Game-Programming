# Source and target
SRCS = main.cpp $(wildcard lib/*.cpp) $(wildcard lib/**/*.cpp) $(wildcard CS3113/*.cpp) $(wildcard leveldata/*.cpp)
TARGET = raylib_app

# ===== LibTorch config =====
TORCH_DIR ?= libtorch

TORCH_INCLUDE = -I$(TORCH_DIR)/include -I$(TORCH_DIR)/include/torch/csrc/api/include
TORCH_LIBS    = -L$(TORCH_DIR)/lib -ltorch -ltorch_cpu -lc10
TORCH_RPATH   = -Wl,-rpath,$(TORCH_DIR)/lib

# OS detection (macOS = Darwin, Windows via MinGW = MINGW*)
UNAME_S := $(shell uname -s)

# Default values
CXX = g++
CXXFLAGS = -std=c++17 $(TORCH_INCLUDE)
LDFLAGS = $(TORCH_RPATH)

# Raylib configuration using pkg-config
RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
RAYLIB_LIBS = $(shell pkg-config --libs raylib)

ifeq ($(DEBUG),1)
    CXXFLAGS += -g -DDEBUG_BUILD=1
else
    CXXFLAGS += -DDEBUG_BUILD=0
endif

ifeq ($(UNAME_S), Darwin)
    # macOS configuration
    CXXFLAGS += -arch arm64 $(RAYLIB_CFLAGS)
    LIBS = $(RAYLIB_LIBS) $(TORCH_LIBS) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    EXEC = ./$(TARGET)
else ifneq (,$(findstring MINGW,$(UNAME_S)))
    # Windows configuration (assumes raylib in C:/raylib)
    CXXFLAGS += -IC:/raylib/include
    LIBS = -LC:/raylib/lib -lraylib $(TORCH_LIBS) -lopengl32 -lgdi32 -lwinmm
    TARGET := $(TARGET).exe
    EXEC = ./$(TARGET)
else
    # Linux/WSL fallback
    CXXFLAGS +=
    LIBS = -lraylib $(TORCH_LIBS) -lGL -lm -lpthread -ldl -lrt -lX11
    EXEC = ./$(TARGET)
endif

# Build rule
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

# Clean rule
clean:
	@if [ -f "$(TARGET)" ]; then rm -f $(TARGET); fi
	@if [ -f "$(TARGET).exe" ]; then rm -f $(TARGET).exe; fi

# Run rule
run: $(TARGET)
	$(EXEC)

# Debug rule
debug: $(TARGET)
	$(EXEC) --debug

# test rule
test: $(TARGET)
	@if [ -f "$(TARGET)" ]; then rm -f $(TARGET); fi
	@if [ -f "$(TARGET).exe" ]; then rm -f $(TARGET).exe; fi
