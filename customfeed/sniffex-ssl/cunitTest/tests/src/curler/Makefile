# build helloworld executable when user executes "make"
programTestCurler: testCurler.o commonFunctions.o curler.o
	$(CC) $(LDFLAGS) testCurler.o commonFunctions.o curler.o -o programTestCurler -lm -lpcap -lcurl -ljson-c -lpthread -lcunit
testCurler.o: testCurler.c
	$(CC) $(CFLAGS) -c testCurler.c
commonFunctions.o: ../../../src/commonFunctions.c
	$(CC) $(CFLAGS) -c ../../../src/commonFunctions.c
curler.o: ../../../src/curler.c
	$(CC) $(CFLAGS) -c ../../../src/curler.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o programTestCurler
