CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -I./src -I./common

SRC = src/main.cpp \
      src/Board.cpp \
      src/GameManager.cpp \
      src/GameObject.cpp \
      src/utils.cpp \
      src/MyTankAlgorithm.cpp \
      src/MyTankAlgorithmFactory.cpp \
      src/SatelliteViewImpl.cpp

TARGET = tanks_main

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)

clean:
	rm -f $(TARGET) *.o
