CONTIKI_PROJECT = nodo udp-server
all: $(CONTIKI_PROJECT)

MAKE_MAC = MAKE_MAC_TSCH

CONTIKI = /home/user/contiki-ng
MODULES += os/services/shell
#MODULES += os/services/orchestra
include $(CONTIKI)/Makefile.include