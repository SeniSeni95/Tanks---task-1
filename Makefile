# Name of the executable
TARGET = tanks_game

# All the source files
SRCS = tanks.cpp board.cpp game_objects.cpp algorithms.cpp utils.cpp

# Compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic

# How to build the program
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Clean up build files
clean:
	rm -f $(TARGET)
