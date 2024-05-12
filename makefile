PROG = GitInsights
LIB = GitInsights.a
XLIB = /usr/local/lib64/shslib.a  -lcurl
DEF = /usr/local/include
PRG = /usr/local/bin/$(PROG)

FILES = \
	$(LIB)(GitInsights.o)

.SILENT:

$(PRG): $(LIB) $(XLIB)
	echo "using gcc to load $(PRG)"
	gcc -o $(PRG) $(LIB) $(XLIB)

$(LIB): $(FILES)

$(FILES): $(DEF)/shslib.h

png:
	cp -pv GitInsights05122024.png /var/www/vhosts/silverhammersoftware/html/GitInsights05122024.png

clean:
	rm -f $(LIB)

all:
	make clean
	make
	make png

.PRECIOUS: $(LIB)

