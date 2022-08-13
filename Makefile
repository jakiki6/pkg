TARGET=pkg
OBJ=main.o

all: pkg

$(TARGET): $(OBJ)
	gcc -o $@ $^

%.o: %.c
	gcc -c -o $@ $< -O2

dist: pkg
	install -D $< pkg_root/root/usr/bin/pkg
	cd pkg_root && tar cfz ../pkg.tar.gz . --owner=0 --group=0

clean:
	rm $(TARGET) $(OBJ) pkg_root/root/usr/bin/pkg pkg.tar.gz 2> /dev/null || true

.PHONY: all clean dist
