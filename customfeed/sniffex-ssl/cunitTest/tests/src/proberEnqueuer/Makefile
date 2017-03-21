# build helloworld executable when user executes "make"
programTestProbeEnqueuer: testProbeEnqueuer.o commonFunctions.o probe_enqueuer.o queue.o radiotap-parser.o
	$(CC) $(LDFLAGS) testProbeEnqueuer.o queue.o radiotap-parser.o commonFunctions.o probe_enqueuer.o -o programTestProbeEnqueuer -lm -lpcap -lcurl -ljson-c -lpthread -lcunit
testProbeEnqueuer.o: testProbeEnqueuer.c
	$(CC) $(CFLAGS) -c testProbeEnqueuer.c
commonFunctions.o: ../../../src/commonFunctions.c
	$(CC) $(CFLAGS) -c ../../../src/commonFunctions.c
probe_enqueuer.o: ../../../src/probe_enqueuer.c
	$(CC) $(CFLAGS) -c ../../../src/probe_enqueuer.c
queue.o: ../../../src/queue.c
	$(CC) $(CFLAGS) -c ../../../src/queue.c
radiotap-parser.o: ../../../src/radiotap-parser.c
	$(CC) $(CFLAGS) -c ../../../src/radiotap-parser.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o programTestProbeEnqueuer
