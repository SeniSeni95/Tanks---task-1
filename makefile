CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -I./src -I./common

SRC = src/main.cpp \
      src/Board.cpp \
      src/GameManager.cpp \
      src/GameObject.cpp \
      src/algorithms.cpp \
      src/utils.cpp \
      src/Vector2D.cpp \
      src/MyTankAlgorithm.cpp \
      src/MyTankAlgorithmFactory.cpp \
      src/SatelliteViewImpl.cpp \
      src/players/AbstractPlayer.cpp \
      src/players/CalmPlayer.cpp \
      src/players/AggressivePlayer.cpp \
      src/tanks/AbstractTankAlgorithm.cpp \
      src/tanks/AggressiveTank.cpp \
      src/tanks/CalmTank.cpp \

TARGET = tanks_main

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)

clean:
	rm -f $(TARGET) *.o
