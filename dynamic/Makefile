CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
RAPIDJSON_INC = ../rapidjson/include

LDFLAGS = -lcurl

TARGET = dynamic
SRC = client.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -I$(RAPIDJSON_INC) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
