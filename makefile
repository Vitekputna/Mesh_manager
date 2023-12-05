CXX = g++
CXXFLAGS = -Wall -std=c++17 -O0 -ffast-math 
SRC_DIR = src
MAIN_DIR = src
LIB_FLAGS = -fopenmp -lmetis -L/home/vitek/local/lib -lGKlib
BUILD_DIR = bin
EXECUTABLE = mesh_manager

# Default 
MAIN = main

# List all the source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Generate a list of object files by replacing the .cpp extension with .o
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Define the main source file
MAIN_SRC = $(MAIN_DIR)/$(addsuffix .cpp,$(MAIN))

# Define the main object file by replacing .cpp extension with .o
MAIN_OBJ = $(BUILD_DIR)/$(addsuffix .o,$(MAIN))

# Define the include paths
INC_FLAGS = -I$(INC_DIR)

all: $(EXECUTABLE)

# Link all the object files into the executable
$(EXECUTABLE): $(OBJS) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIB_FLAGS)

# Compile all the source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LIB_FLAGS)

# Compile the main source file into an object file
$(MAIN_OBJ): $(MAIN_SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LIB_FLAGS)

clean:
	rm -rf $(BUILD_DIR)/*.o
	rm $(EXECUTABLE)

.PHONY: all clean
