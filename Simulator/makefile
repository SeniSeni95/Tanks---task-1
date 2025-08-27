CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic \
           -I. -I../common -I../GameManager -I../Simulator -I../algorithm -I../algorithm/tanks
LDFLAGS = -Wl,--no-as-needed -ldl -pthread

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
    ../algorithm/MyPlayerFactory.cpp \
    ../algorithm/algorithms.cpp \
    ../algorithm/players/AbstractPlayer.cpp \
    ../algorithm/players/AggressivePlayer.cpp \
    ../algorithm/players/CalmPlayer.cpp \
    ../algorithm/tanks/AbstractTankAlgorithm.cpp \
    ../algorithm/tanks/AggressiveTank.cpp \
    ../algorithm/tanks/CalmTank.cpp

TARGET = simulator

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
