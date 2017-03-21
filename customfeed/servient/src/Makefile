# build helloworld executable when user executes "make"
servient: servient.o ucitest.o curler.o metrics.o
	$(CC) $(LDFLAGS) servient.o ucitest.o curler.o metrics.o -o servient -luci -lucimap -lcurl -lpthread -ljson-c
servient.o: servient.c
	$(CC) $(CFLAGS) -c servient.c
ucitest.o: ucitest.c
	$(CC) $(CFLAGS) -c ucitest.c
curler.o: curler.c
	$(CC) $(CFLAGS) -c curler.c
metrics.o: metrics.c
	$(CC) $(CFLAGS) -c metrics.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o servient

