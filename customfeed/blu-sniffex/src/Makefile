# build helloworld executable when user executes "make"
sniffex: sniffex.o commonFunctions.o probe_enqueuer.o queue.o radiotap-parser.o curler.o metrics.o ucitest.o totp.o
	$(CC) $(LDFLAGS) sniffex.o queue.o radiotap-parser.o commonFunctions.o probe_enqueuer.o curler.o metrics.o ucitest.o totp.o -o sniffex -lm -lpcap -lcurl -ljson-c -lpthread -luci -lucimap -lpolarssl
sniffex.o: sniffex.c
	$(CC) $(CFLAGS) -c sniffex.c -g
commonFunctions.o: commonFunctions.c
	$(CC) $(CFLAGS) -c commonFunctions.c -g
probe_enqueuer.o: probe_enqueuer.c
	$(CC) $(CFLAGS) -c probe_enqueuer.c -g
queue.o: queue.c
	$(CC) $(CFLAGS) -c queue.c -g
radiotap-parser.o: radiotap-parser.c
	$(CC) $(CFLAGS) -c radiotap-parser.c -g
curler.o: curler.c
	$(CC) $(CFLAGS) -c curler.c -g
totp.o: totp.c
	$(CC) $(CFLAGS) -c totp.c -g
metrics.o: metrics.c
	$(CC) $(CFLAGS) -c metrics.c -g
ucitest.o: ucitest.c
	$(CC) $(CFLAGS) -c ucitest.c -g

# remove object files and executable when user executes "make clean"
clean:
	rm *.o sniffex
