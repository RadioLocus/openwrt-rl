# build helloworld executable when user executes "make"
alerts: alerts.o 
	$(CC) $(LDFLAGS) alerts.o -o alerts -luci -lucimap -lcurl -ljson-c
alerts.o: alerts.c
	$(CC) $(CFLAGS) -c alerts.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o alerts

