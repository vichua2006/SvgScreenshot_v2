CXX ?= x86_64-w64-mingw32-g++
CXXFLAGS ?= -std=c++17 -Wall -O2 $(shell pkg-config --cflags opencv4 2>/dev/null)
LDFLAGS = -lgdi32 -lole32 -luuid -lcomdlg32 -lshell32 -lmsimg32 $(shell pkg-config --libs opencv4 2>/dev/null)
ifdef OPENCV
    CXXFLAGS += -DUSE_OPENCV
endif

SRC_DIR := src
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:.cpp=.o)
TARGET := screenshot.exe

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
