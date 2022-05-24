CC=g++
SRC=src
BUILD=build
VERSION=4.0-0

bin:
	mkdir $(BUILD)
	$(CC) $(SRC)/TCPforward.cpp -pthread -o $(BUILD)/TCPforward
	mv -v $(BUILD)/TCPforward ./

clean:
	rm -rvf $(BUILD)
	rm -vf TCPforward
	rm -vf TCPforward-$(VERSION).deb


deb:
	mkdir $(BUILD)
	mkdir $(BUILD)/TCPforward-$(VERSION)
	mkdir $(BUILD)/TCPforward-$(VERSION)/DEBIAN
	cp -v $(SRC)/PackageData/control $(BUILD)/TCPforward-$(VERSION)/DEBIAN/control
	mkdir $(BUILD)/TCPforward-$(VERSION)/usr
	mkdir $(BUILD)/TCPforward-$(VERSION)/usr/bin
	$(CC) $(SRC)/TCPforward.cpp -pthread -o $(BUILD)/TCPforward
	mv -v $(BUILD)/TCPforward $(BUILD)/TCPforward-$(VERSION)/usr/bin/
	dpkg-deb --build $(BUILD)/TCPforward-$(VERSION)
	mv -v $(BUILD)/TCPforward-$(VERSION).deb ./
	

install:
	echo "Installing TCP Forward"
	mv -v $(BUILD)/TCPforward /usr/bin/


all:
	deb
	bin
