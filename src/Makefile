CFLAGS=-I/home/fran/local/include/lttoolbox-3.3 -I/usr/local/include/lttoolbox-3.3
LDFLAGS=-L/home/fran/local/lib -L/usr/local/lib -llttoolbox3

transducer: transducer.cpp
	g++ -ggdb $(CFLAGS) -Wall transducer.cpp -o $@ $(LDFLAGS)

processor: processor.cpp
	g++ -ggdb $(CFLAGS) -Wall processor.cpp -o $@ $(LDFLAGS)

proc2: processor2.cpp
	g++ -ggdb $(CFLAGS) -Wall processor2.cpp -o $@ $(LDFLAGS)

proc3: processor3.cpp
	g++ -ggdb $(CFLAGS) -Wall processor3.cpp -o $@ $(LDFLAGS)

all: transducer processor proc2

test:
	./transducer ./processor ./proc2 ./proc3

clean:
	rm -f *.o transducer processor proc2 proc3
