#Makefile for the example project using MuLan String library

CFLAGS = -g 

#compile the program
example.bin: main.o mls_module.o
	g++ ${CFLAGS} -o $@ $^

%.o: %.cpp
	g++ --std=c++17 ${CFLAGS} -I../ -c -o $@ $<

exe: example.bin

clean_exe:
	rm *.o example.bin

#make translations

po/example.pot: main.cpp
	xgettext --keyword=_ --language=C++ --add-comments --join-existing --package-name=example --package-version=1.0 --from-code=UTF-8 -o $@ $^

po/pl/example.po: po/example.pot
#msginit --input=$< --locale=pl --output=$@
	msgmerge --update $@ $^ 

po/pl/example.mo: po/pl/example.po
	msgfmt --output-file=$@ $<

locale/pl/LC_MESSAGES/example.mo: po/pl/example.mo
	cp $^ $@

trans: locale/pl/LC_MESSAGES/example.mo

#----------
all: trans exe

.PHONY: trans exe all clean_exe