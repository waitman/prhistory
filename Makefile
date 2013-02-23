all: prhistory.c
	cc -o prhistory -I/usr/local/include -L/usr/local/lib -lsqlite3 -lcurl prhistory.c

install:
	cp prhistory /usr/local/bin
	cp prhistory.7 /usr/local/man/man7/

deinstall:
	rm -f /usr/local/bin/prhistory

clean:
	rm -f prhistory

update:
	prhistory -u
