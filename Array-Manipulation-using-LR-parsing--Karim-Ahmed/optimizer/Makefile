CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

TARGET = optimizer
OBJS = optimizer_main.o ir_loader.o optimizer.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

optimizer_main.o: optimizer_main.cpp optimizer.h ir_loader.h optimizer_types.h
	$(CXX) $(CXXFLAGS) -c optimizer_main.cpp

ir_loader.o: ir_loader.cpp ir_loader.h optimizer_types.h
	$(CXX) $(CXXFLAGS) -c ir_loader.cpp

optimizer.o: optimizer.cpp optimizer.h optimizer_types.h
	$(CXX) $(CXXFLAGS) -c optimizer.cpp

clean:
	rm -f $(OBJS) $(TARGET) optimized_ir.txt

.PHONY: all clean
