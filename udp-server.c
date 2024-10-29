#include "contiki.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "dev/leds.h"
#include "sys/node-id.h"
#include <stdint.h>
#include <inttypes.h>
#include "net/routing/routing.h"

#include "sys/log.h"

#define LOG_MODULE "Server"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY 1
#define UDP_PORT 5678

static struct simple_udp_connection udp_conn; // udp_conn saves info of the comunication
const int GROUP_NUM = 1;

/*---------------------------------------------------------------------------*/
PROCESS(udp_process, "UDP server");
AUTOSTART_PROCESSES(&udp_process);
/*---------------------------------------------------------------------------*/
static void // When msg recived this function is excecuted
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{
  leds_toggle(LEDS_RED);
  LOG_INFO("Mensaje recibido '%.*s' de ", datalen, (char *)data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n\r");

  leds_toggle(LEDS_GREEN);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_process, ev, data)
{
  static char str[40];
  static radio_value_t ch_num;
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, udp_rx_callback);

  /* Initialize NodeId identification */
  node_id_init();

  NETSTACK_ROUTING.root_start();

  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 10 + GROUP_NUM);
  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &ch_num);
  LOG_INFO("RF_CHANNEL: %d\n", ch_num);
  /* Generates multicast address */
  uip_create_linklocal_allnodes_mcast(&dest_ipaddr);

  LOG_INFO("Enviando multicast a");
  LOG_INFO_6ADDR(&dest_ipaddr); // all motes connect to this IP, IP of multicast
  LOG_INFO_("\n");

  snprintf(str, sizeof(str), "Hola soy el servidor del grupo %d", GROUP_NUM);
  simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr); // send info to multicast IP
  leds_toggle(LEDS_GREEN);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
