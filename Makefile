CXX = g++
CXXFLAGS = -I./include
LDFLAGS = -L./lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
TARGET = breakout

$(TARGET): main.cpp
	$(CXX) main.cpp -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET)
	