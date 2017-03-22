# build helloworld executable when user executes "make"
programTestMetrics: testMetrics.o commonFunctions.o metrics.o
	$(CC) $(LDFLAGS) testMetrics.o commonFunctions.o metrics.o -o programTestMetrics -lm -lpcap -lcurl -ljson-c -lpthread -lcunit
testMetrics.o: testMetrics.c
	$(CC) $(CFLAGS) -c testMetrics.c
commonFunctions.o: ../../../src/commonFunctions.c
	$(CC) $(CFLAGS) -c ../../../src/commonFunctions.c
metrics.o: ../../../src/metrics.c
	$(CC) $(CFLAGS) -c ../../../src/metrics.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o programTestMetrics
