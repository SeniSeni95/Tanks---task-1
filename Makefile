# Name of the executable
TARGET = Tanks

# All the source files
SRCS = tanks.cpp board.cpp game_objects.cpp algorithms.cpp utils.cpp

# Compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++17 -Wno-unused-variable

# How to build the program
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Clean up build files
clean:
	rm -f $(TARGET)
