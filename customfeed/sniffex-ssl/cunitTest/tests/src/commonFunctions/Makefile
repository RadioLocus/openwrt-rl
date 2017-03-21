# build helloworld executable when user executes "make"
programTestCommonFunctions: testCommonFunctions.o commonFunctions.o
	$(CC) $(LDFLAGS) testCommonFunctions.o commonFunctions.o -o programTestCommonFunctions -lm -lpcap -lcurl -ljson-c -lpthread -lcunit
testCommonFunctions.o: testCommonFunctions.c
	$(CC) $(CFLAGS) -c testCommonFunctions.c -g
commonFunctions.o: ../../../src/commonFunctions.c
	$(CC) $(CFLAGS) -c ../../../src/commonFunctions.c -g

# remove object files and executable when user executes "make clean"
clean:
	rm *.o programTestCommonFunctions
