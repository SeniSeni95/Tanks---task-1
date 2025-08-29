CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -pedantic -fPIC -g
LDFLAGS  := -shared

# Sources for your GameManager .so
SRC := \
    Board.cpp \
    GameManager.cpp \
    GameObject.cpp \
    SatelliteViewImpl.cpp \
    Vector2D.cpp \
    utils.cpp \
    ../Simulator/GameManagerRegistration.cpp

# Convert .cpp to .o
OBJ := $(SRC:.cpp=.o)

# Target name (with your IDs)
TARGET := GameManager_329506620_206055055.so

# Default rule
all: $(TARGET)

# Link rule
$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

# Explicit rule for the simulator cpp (since it's outside this dir)
../Simulator/GameManagerRegistration.o: ../Simulator/GameManagerRegistration.cpp
	$(CXX) $(CXXFLAGS) -I. -I../common -I../Simulator -c $< -o $@

# Generic compile rule for sources in this dir
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I. -I../common -I../Simulator -c $< -o $@

# Cleanup
clean:
	rm -f *.o $(TARGET) ../Simulator/GameManagerRegistration.o
