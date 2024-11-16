CONTIKI_PROJECT = main
all: $(CONTIKI_PROJECT)

# Comentar esta linea al compilar en Cooja
PROJECT_SOURCEFILES += sensor.c

MAKE_MAC = MAKE_MAC_TSCH

CONTIKI = /home/user/contiki-ng
MODULES += os/services/shell
MODULES += $(CONTIKI_NG_NET_DIR)/ipv6/multicast

include $(CONTIKI)/Makefile.include

CFLAGS += -ffunction-sections
LDFLAGS += -Wl,--gc-sections,--undefined=_reset_vector__,--undefined=InterruptVectors,--undefined=_copy_data_init__,--undefined=_clear_bss_init__,--undefined=_end_of_init__