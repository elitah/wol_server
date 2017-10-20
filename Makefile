
#CROSS_COMPILER := arm-hisiv300-linux-

INCLUDE_PATH += -I.

LIBRARY_PATH += -L.

ifneq ($(CROSS_COMPILER),)
LIBRARY_S_LIST += -Wl,-Bstatic
LIBRARY_D_LIST += -Wl,-Bdynamic -lpthread -ldl
else
LIBRARY_S_LIST += -Wl,-Bstatic
LIBRARY_D_LIST += -Wl,-Bdynamic -lpthread -ldl
endif

all:: wol_server

wol_server: main.o ifconfig.o epoll.o cJSON.o json.o
	$(CROSS_COMPILER)g++ -Wall -Wl,-rpath=. $(LIBRARY_PATH) -o $@ $^ $(LIBRARY_S_LIST) $(LIBRARY_D_LIST)
	$(CROSS_COMPILER)strip $@

%.o: %.cpp
	$(CROSS_COMPILER)g++ -Wall -fPIC $(INCLUDE_PATH) -c -o $@ $<

%.o: %.c
	$(CROSS_COMPILER)gcc -Wall -fPIC $(INCLUDE_PATH) -c -o $@ $<

clean:
	rm -rf *.o *.cgi wol_server

distclean:
	make -C . clean
	rm -rf *.so *.a
