#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "dev/button-hal.h"
#include "dev/gpio-hal.h"
#include "dev/leds.h"

#include "sys/log.h"
#include "sys/node-id.h"
#include "sys/timer.h"

#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "net/netstack.h"

#include "contiki.h"
#include "msg.h"
#ifndef COOJA
#include "sensor.h"
#endif

#define LOG_MODULE "Nodo"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY 1
#define UDP_PORT 5678

#define DISTANCE 10 // distancia entre sensores, en metros

static float max_vel = 3; // m/s
static bool waiting_for_sensor = false;
static struct simple_udp_connection udp_conn;
static uint64_t t_init;

static uip_ipaddr_t ip_server;
static uip_ipaddr_t ip_next;
static uip_ipaddr_t ip_prev;
static uip_ipaddr_t ip_multicast;

static void
udp_rx_callback(struct simple_udp_connection *c, const uip_ipaddr_t *sender_addr, uint16_t sender_port, const uip_ipaddr_t *receiver_addr, uint16_t receiver_port, const uint8_t *data, uint16_t datalen)
{
  LOG_INFO("Mensaje recibido de ");
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");

  // asumimos que todos los mensajes recibidos son del tipo msg_t
  msg_t *msg = (msg_t *)data;

  bool is_sender_prev_node = uip_ip6addr_cmp(sender_addr, &ip_prev);
  bool is_sender_multicast = uip_ip6addr_cmp(sender_addr, &ip_multicast);

  // si el mensaje me lo manda el nodo anterior y es el mensaje de activacion de sensor
  if (is_sender_prev_node && msg->type == VEHICLE_DETECTED)
  {
    waiting_for_sensor = true;
    t_init = msg->value.t_init;
    LOG_INFO("Esperando por sensor. Tiempo inicial: %zu\n", t_init);
  }
  // aca la idea es que cuando por shell en el server se modifica la velocidad se recibe el msj y se cambia
  if (is_sender_multicast && msg->type == MAX_VEL_CHANGE)
  {
    max_vel = msg->value.new_max_vel;
    LOG_INFO("Nueva velocidad maxima establecida: %.2f\n", max_vel);
  }
}

static void set_addresses()
{
  // DEFINIR BIEN LAS IP
  //  server node id = 1
  uip_ip6addr(&ip_server, 0xfd00, 0x0000, 0x0000, 0x0000, 0x0201, 1, 1, 1);
  // se asume que estan ordenados crecientes en el espacio fisico y configurados correctamente
  uip_ip6addr(&ip_next, 0xfd00, 0x0000, 0x0000, 0x0000, 0x0201 + node_id, node_id + 1, node_id + 1, node_id + 1);
  uip_ip6addr(&ip_prev, 0xfd00, 0x0000, 0x0000, 0x0000, 0x0200 + node_id - 1, node_id - 1, node_id - 1, node_id - 1);
  uip_ip6addr(&ip_multicast, 0xfe80, 0x0000, 0x0000, 0x0000, 0x0201, 1, 1, 1);
}

PROCESS(loop, "Main loop process");
AUTOSTART_PROCESSES(&loop);

PROCESS_THREAD(loop, ev, data)
{
  static char str[64];

  PROCESS_BEGIN();

  set_addresses();
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, udp_rx_callback);
  if (node_id == 1) NETSTACK_ROUTING.root_start();

#ifndef COOJA
  process_start(&handle_sensor, NULL);
#endif

  while (1)
  {
#ifdef COOJA
    PROCESS_WAIT_EVENT_UNTIL(ev == button_hal_press_event);
#else
    PROCESS_WAIT_EVENT_UNTIL(ev == vehicle_event);
#endif

    uint64_t t_now = tsch_get_network_uptime_ticks();

    // mandar mensaje a siguiente nodo
    msg_t msg = 
    {
      .type = VEHICLE_DETECTED, 
      .value = {.t_init = t_now}
    };
    simple_udp_sendto(&udp_conn, &msg, sizeof(msg_t), &ip_next);

    if (waiting_for_sensor)
    {
      // parar timer, calcular velocidad(en ticks)
      float delta_ticks = t_now - t_init;
      float delta_t = delta_ticks / CLOCK_SECOND;
      float vel = DISTANCE / delta_t;

      if (vel <= max_vel)
      {
        LOG_INFO("TODO EN ORDEN\n");
        LOG_INFO("Velocidad detectada (m/s): %.2f\n", vel);
        LOG_INFO("Diferencia de tiempo (s): %.2f\n", delta_t);
      }
      else
      { // PODRIA VERSE DE MANDAR AL SERVER SOLO CUANDO SE SUPERA VELOCIDAD Y QUE EL MISMO PRINTEE POR UART O LOG
        LOG_INFO("ALERTA: VELOCIDAD MAXIMA SUPERADA\n");
        LOG_INFO("Velocidad detectada (m/s): %.2f\n", vel);
        LOG_INFO("Diferencia de tiempo (s): %.2f\n", delta_t);
      }
      LOG_INFO("Velocidad maxima permitida: %.2f\n", max_vel);

      if (node_id != 1) {
        snprintf(str, sizeof(str), "VEL:%f", vel);
        simple_udp_sendto(&udp_conn, str, strlen(str), &ip_server);
      }

      waiting_for_sensor = false;
    }
  }

  PROCESS_END();
}