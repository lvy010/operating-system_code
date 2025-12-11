CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g -pthread

TARGET = timer_applications_test
SOURCE = main.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean 