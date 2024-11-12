CONTIKI_PROJECT = main
all: $(CONTIKI_PROJECT)

ifndef $(COOJA)
#PROJECT_SOURCEFILES += sensor.c
endif

MAKE_MAC = MAKE_MAC_TSCH

CONTIKI = /home/user/contiki-ng
MODULES += os/services/shell
MODULES += $(CONTIKI_NG_NET_DIR)/ipv6/multicast
#MODULES += os/net/ipv6/multicast

include $(CONTIKI)/Makefile.include