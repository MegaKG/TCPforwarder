CC=g++
SRC=src
BUILD=build

all:
	mkdir $(BUILD)
	$(CC) $(SRC)/TCPforward.cpp -pthread -o $(BUILD)/TCPforward
	mv -v $(BUILD)/TCPforward ./

clean:
	rm -rvf $(BUILD)
	rm -vf TCPforward
