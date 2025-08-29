CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -pedantic -fPIC -g \
            -I. -I../common -I../GameManager
LDFLAGS  := -shared

SRC := MyPlayerFactory.cpp \
       players/AbstractPlayer.cpp \
       players/AggressivePlayer.cpp \
       tanks/AbstractTankAlgorithm.cpp \
       tanks/AggressiveTank.cpp \
       algorithms.cpp

OBJ := $(SRC:.cpp=.o)

TARGET := Algorithm_329506620_206055055.so

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^ -ldl -pthread

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I. -I../common -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
