CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

RAPIDJSON_INC = ../rapidjson/include
LDFLAGS = -lcurl

TARGET = seqLC
SRC = client.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -I$(RAPIDJSON_INC) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

