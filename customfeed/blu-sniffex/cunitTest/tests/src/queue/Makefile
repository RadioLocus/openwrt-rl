# build helloworld executable when user executes "make"
programTestQueue: testQueue.o commonFunctions.o queue.o
	$(CC) $(LDFLAGS) testQueue.o commonFunctions.o queue.o -o programTestQueue -lm -lpcap -lcurl -ljson-c -lpthread -lcunit
testQueue.o: testQueue.c
	$(CC) $(CFLAGS) -c testQueue.c
commonFunctions.o: ../../../src/commonFunctions.c
	$(CC) $(CFLAGS) -c ../../../src/commonFunctions.c
queue.o: ../../../src/queue.c
	$(CC) $(CFLAGS) -c ../../../src/queue.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o programTestQueue
