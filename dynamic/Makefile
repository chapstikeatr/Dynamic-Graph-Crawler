CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread -I/opt/homebrew/include
LDFLAGS = -lcurl

TARGET = dynamic
SRC = client.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
