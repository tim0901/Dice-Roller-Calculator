
ifeq ($(OS),Windows_NT)
	TARGET = dice_roller_calculator1.0.dll
else
	TARGET = dice_roller_calculator1.0.so
endif


all: $(TARGET)

$(TARGET): calculator.o dice_roller.o lexer.o roller.o
	g++ -shared -Wl,-soname,$(TARGET) -o $(TARGET) dice_roller.o calculator.o lexer.o roller.o

calculator.o: calculator.cpp calculator.h
	g++ -c -fPIC calculator.cpp

dice_roller.o: dice_roller.cpp
	g++ -c -fPIC dice_roller.cpp

lexer.o: lexer.cpp lexer.h
	g++ -c -fPIC lexer.cpp

roller.o: roller.cpp roller.h
	g++ -c -fPIC roller.cpp

clean:
	rm -f *.o *.so *.dll
