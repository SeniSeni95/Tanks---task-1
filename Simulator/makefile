CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -g -fsanitize=address \
           -I. -I../common -I../GameManager -I../Simulator -I../algorithm -I../algorithm/tanks
LDFLAGS = -Wl,--no-as-needed -ldl -pthread -rdynamic -fsanitize=address

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
    ../GameManager/SatelliteViewImpl.cpp

OBJ = $(SRC:.cpp=.o)

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
