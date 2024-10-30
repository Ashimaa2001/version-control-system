CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
LIBS = -lz -lcrypto
SRC = mygit.cpp
TARGET = mygit

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
