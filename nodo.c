#include "contiki.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "dev/leds.h"
#include "sys/node-id.h"
#include <stdint.h>
#include <inttypes.h>
#include "dev/button-hal.h"

#include "sys/log.h"

#define LOG_MODULE "Nodo"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY 1
#define UDP_PORT 5678

#define DISTANCE 1 // en metro

#define SENSOR_ACT_MSG "SENSOR_ACT"

static bool waiting_for_sensor = false;
static struct simple_udp_connection udp_conn; // udp_conn saves info of the comunication
static clock_time_t t_init;

static uip_ipaddr_t ip_server;
static uip_ipaddr_t ip_next;
static uip_ipaddr_t ip_prev;

/*---------------------------------------------------------------------------*/
PROCESS(udp_process, "UDP client");
AUTOSTART_PROCESSES(&udp_process);
/*-------------------------------------------------------(--------------------*/
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
  LOG_INFO_("\n");

  LOG_INFO("uip condition: %u\n", uip_ip6addr_cmp(sender_addr, &ip_prev));
  LOG_INFO("strmp condition: %u\n", strcmp((const char*)data, SENSOR_ACT_MSG));

  // si el mensaje me lo manda el nodo anterior y es el mensaje de activacion de sensor
  if (uip_ip6addr_cmp(sender_addr, &ip_prev) && strcmp((const char*)data, SENSOR_ACT_MSG) == 0)
  {
    waiting_for_sensor = true;
    clock_init();
    t_init = clock_time();
    LOG_INFO("Esperando por sensor. Tiempo inicial: %lu\n", t_init);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_process, ev, data)
{
  static button_hal_button_t *btn;
  static char str[36];

  PROCESS_BEGIN();

  /* Initialize NodeId identification */
  node_id_init();

  // server node id = 1
  uip_ip6addr(&ip_server, 0xfd00, 0x0000, 0x0000, 0x0000, 0x0201, 1, 1, 1);
  // fijar por ejemplo dir de los nodos como 0xfd00::0x200+nodeid+1,nodeid+1,nodeid+1,nodeid+1; para asumir que mandamos al siguiente
  // se asume que estan ordenados crecientes en el espacio fisico y configurados correctamente
  uip_ip6addr(&ip_next, 0xfd00, 0x0000, 0x0000, 0x0000, 0x0201 + node_id, node_id + 1, node_id + 1, node_id + 1);
  uip_ip6addr(&ip_prev, 0xfd00, 0x0000, 0x0000, 0x0000, 0x0200 + node_id - 1, node_id - 1, node_id - 1, node_id - 1);

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, udp_rx_callback);

  while (1)
  {
    PROCESS_WAIT_EVENT();

    // primer sensor (id=2)
    if (ev == button_hal_press_event)
    {
      btn = (button_hal_button_t *)data;

      // Check that the pressed button is BTN-1
      if (btn == button_hal_get_by_id(BUTTON_HAL_ID_BUTTON_ZERO))
      {
        // mandar mensaje a siguiente nodo
        snprintf(str, sizeof(str), SENSOR_ACT_MSG);
        simple_udp_sendto(&udp_conn, str, strlen(str), &ip_next);

        if (waiting_for_sensor)
        {
          // parar timer, calcular velocidad
          clock_time_t delta_t = clock_time() - t_init;
          float vel = (DISTANCE * 10000) / delta_t;
          LOG_INFO("Velocidad detectada: %f\n", vel);
          LOG_INFO("Diferencia de tiempo: %lu\n", delta_t);

          snprintf(str, sizeof(str), "VEL:%f", vel);
          simple_udp_sendto(&udp_conn, str, strlen(str), &ip_server);

          waiting_for_sensor = false;
        }
      }
    }
  }
  PROCESS_END();
}