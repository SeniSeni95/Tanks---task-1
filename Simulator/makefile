CXX = g++
<<<<<<< HEAD
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -g -fsanitize=address \
           -I. -I../common -I../GameManager -I../Simulator -I../algorithm -I../algorithm/tanks
LDFLAGS = -Wl,--no-as-needed -ldl -pthread -rdynamic -fsanitize=address
=======
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic \
           -I. -I../common -I../GameManager -I../Simulator -I../Algorithm -I../Algorithm/tanks
LDFLAGS = -ldl -pthread
>>>>>>> ec6c4f4fe0fa3f31a1398e631f7d49c40b2a5038

SRC = \
    main.cpp \
    ../Simulator/AlgorithmRegistrar.cpp \
    ../Simulator/GameManagerRegistrar.cpp \
    ../Simulator/PlayerRegistration.cpp \
    ../Simulator/TankAlgorithmRegistration.cpp \
    ../Simulator/GameManagerRegistration.cpp \
    ../GameManager/Board.cpp \
    ../GameManager/GameObject.cpp \
    ../GameManager/utils.cpp \
    ../GameManager/Vector2D.cpp \
<<<<<<< HEAD
    ../GameManager/SatelliteViewImpl.cpp

OBJ = $(SRC:.cpp=.o)
=======
    ../GameManager/SatelliteViewImpl.cpp \
    ../Algorithm/MyPlayerFactory.cpp \
    ../Algorithm/algorithms.cpp \
    ../Algorithm/players/AbstractPlayer.cpp \
    ../Algorithm/players/AggressivePlayer.cpp \
    ../Algorithm/players/CalmPlayer.cpp \
    ../Algorithm/tanks/AbstractTankAlgorithm.cpp \
    ../Algorithm/tanks/AggressiveTank.cpp \
    ../Algorithm/tanks/CalmTank.cpp
>>>>>>> ec6c4f4fe0fa3f31a1398e631f7d49c40b2a5038

TARGET = simulator

# Default target: build everything with ASan
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $(TARGET)

# Compile each .cpp into .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: clean
