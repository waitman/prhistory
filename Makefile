all: prhistory.c
	${CC} -o prhistory -I${PREFIX}/include -L${PREFIX}/lib -lsqlite3 -lcurl prhistory.c

install:
	install -g wheel -m 555 -o root prhistory ${PREFIX}/bin
	install -g wheel -m 444 -o root prhistory.7 ${PREFIX}/man/man7/

deinstall:
	rm -f ${PREFIX}/bin/prhistory
	rm -f /var/db/ports/ports-pr.db

clean:
	rm -f prhistory

update:
	prhistory -u
