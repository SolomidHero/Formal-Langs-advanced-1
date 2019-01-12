CPPFLAGS = -std=c++17
CPP_COMP = arm-linux-gnueabi-g++
C_COMP = arm-linux-gnueabi-gcc
ARM_LIB = ~/arm-sysroot
GTEST_LIB = /usr/local/arm-googletest/lib/

TARGET = tester_program.o jit_compile_function.o main.o
TEST_TARGET = jit_compile_function.o tester_program.o test_main.o
# $(CPP_COMP) $(CPPFLAGS) -o test_program $(TEST_TARGET) -I /usr/local/arm-googletest/include -L $(GTEST_LIB) -lgtest

program: $(TARGET)
	$(CPP_COMP) $(CPPFLAGS) -o program $(TARGET)
	make clean

jit_compile_function.o: jit_compile_function.cpp
	$(CPP_COMP) $(CPPFLAGS) -c jit_compile_function.cpp

tester_program.o: tester_program.c
	$(C_COMP) -c tester_program.c

main.o: main.cpp
	$(CPP_COMP) $(CPPFLAGS) -c main.cpp

clean:
	rm $(TARGET)

# test section
test: $(TEST_TARGET)
	$(CPP_COMP) $(CPPFLAGS) -o test_program $(TEST_TARGET)
	make test_clean

test_main.o: test_main.cpp
	$(CPP_COMP) $(CPPFLAGS) -c test_main.cpp 

test_clean:
	rm $(TEST_TARGET)

launch:
	qemu-arm -L $(ARM_LIB) ./program

launch_test:
	qemu-arm -L $(ARM_LIB) ./test_program

