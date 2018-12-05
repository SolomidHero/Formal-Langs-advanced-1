CPPFLAGS = -std=c++17
CPP_COMP = arm-linux-gnueabi-g++
C_COMP = arm-linux-gnueabi-gcc
ARM_LIB = ~/arm-sysroot

TARGET = tester_program.o jit_compile_function.o

program: $(TARGET)
	$(CPP_COMP) $(CPPFLAGS) -o program $(TARGET)
	make clean

jit_compile_function.o: jit_compile_function.cpp
	$(CPP_COMP) $(CPPFLAGS) -c jit_compile_function.cpp

tester_program.o: tester_program.c
	$(C_COMP) -c tester_program.c


clean:
	rm $(TARGET)

launch:
	qemu-arm -L $(ARM_LIB) ./program
