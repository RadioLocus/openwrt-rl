# build helloworld executable when user executes "make"
sniffex: sniffex.o commonFunctions.o probe_enqueuer.o queue.o radiotap-parser.o curler.o metrics.o
	$(CC) $(LDFLAGS) sniffex.o queue.o radiotap-parser.o commonFunctions.o probe_enqueuer.o curler.o metrics.o -o sniffex -lm -lpcap -lcurl -ljson-c -lpthread -lcunit
sniffex.o: src/sniffex.c
	$(CC) $(CFLAGS) -c src/sniffex.c -g
commonFunctions.o: src/commonFunctions.c
	$(CC) $(CFLAGS) -c src/commonFunctions.c -g
probe_enqueuer.o: src/probe_enqueuer.c
	$(CC) $(CFLAGS) -c src/probe_enqueuer.c -g
queue.o: src/queue.c
	$(CC) $(CFLAGS) -c src/queue.c -g
radiotap-parser.o: src/radiotap-parser.c
	$(CC) $(CFLAGS) -c src/radiotap-parser.c -g
curler.o: src/curler.c
	$(CC) $(CFLAGS) -c src/curler.c -g
metrics.o: src/metrics.c
	$(CC) $(CFLAGS) -c src/metrics.c -g

# remove object files and executable when user executes "make clean"
clean:
	rm *.o sniffex
