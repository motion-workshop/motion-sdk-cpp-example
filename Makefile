TARGET   = example
OBJ      = example.o Client.o Format.o
CPP      = clang++
CPPFLAGS = -std=c++11 -DMOTION_SDK_NOTHROW=1 -Isdk/cpp
LDFLAGS  =

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CPP) -o $@ $(LDFLAGS) $(OBJ)

# Example application, includes main.
example.o: example.cpp
	$(CPP) -c $< $(CPPFLAGS) -o $@

# Compile in the SDK Client and Format classes.
%.o: sdk/cpp/src/%.cpp
	$(CPP) -c $< $(CPPFLAGS) -o $@

clean:
	rm -f $(OBJ) $(TARGET)
