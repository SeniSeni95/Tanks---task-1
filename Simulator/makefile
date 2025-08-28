CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic \
           -I. -I../common -I../GameManager -I../Simulator -I../Algorithm -I../Algorithm/tanks
LDFLAGS = -ldl -pthread

SRC = \
    main.cpp \
    ../Simulator/AlgorithmRegistrar.cpp \
    ../Simulator/GameManagerRegistrar.cpp \
    ../Simulator/PlayerRegistration.cpp \
    ../Simulator/TankAlgorithmRegistration.cpp \
    ../GameManager/Board.cpp \
    ../GameManager/GameObject.cpp \
    ../GameManager/utils.cpp \
    ../GameManager/Vector2D.cpp \
    ../GameManager/SatelliteViewImpl.cpp \
    ../Algorithm/MyPlayerFactory.cpp \
    ../Algorithm/algorithms.cpp \
    ../Algorithm/players/AbstractPlayer.cpp \
    ../Algorithm/players/AggressivePlayer.cpp \
    ../Algorithm/players/CalmPlayer.cpp \
    ../Algorithm/tanks/AbstractTankAlgorithm.cpp \
    ../Algorithm/tanks/AggressiveTank.cpp \
    ../Algorithm/tanks/CalmTank.cpp

TARGET = simulator

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
