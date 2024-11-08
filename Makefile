CONTIKI_PROJECT = main
all: $(CONTIKI_PROJECT)

PROJECT_SOURCEFILES += sensor.c

MAKE_MAC = MAKE_MAC_TSCH

CONTIKI = /home/user/contiki-ng
MODULES += os/services/shell
#MODULES += os/net/ipv6/multicast

include $(CONTIKI)/Makefile.include