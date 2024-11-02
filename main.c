#include "contiki.h"
#include "dev/gpio-hal.h"
#include "gpio-interrupt.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "sys/log.h"
#include "sys/int-master.h"

#include <stdio.h>

#define LOG_MODULE "Nodo"
#define LOG_LEVEL LOG_LEVEL_INFO

#define TRIG_PIN BOARD_IOID_DIO26
#define ECHO_PIN BOARD_IOID_DIO27

#define SOUND_VEL 343000 // mm/s
#define MIN_DISTANCE 200 // mm

static struct etimer et;

static gpio_hal_port_t trig = TRIG_PIN;
static gpio_hal_port_t echo = ECHO_PIN;

static process_event_t sensor_event;

void trigger_sensor()
{
  gpio_hal_arch_set_pin(GPIO_HAL_NULL_PORT, trig);
  clock_wait(1);
  gpio_hal_arch_clear_pin(GPIO_HAL_NULL_PORT, trig);
}

PROCESS(sensor, "Sensor Control");
AUTOSTART_PROCESSES(&sensor);

static void echo_handler(unsigned char pin_mask)
{
  static rtimer_clock_t initial_time = 0;
  static rtimer_clock_t flight_time = 0;
  static bool is_rise_edge = true;

  if (is_rise_edge)
  {
    // inicialmente flanco de subida
    is_rise_edge = false;
    initial_time = RTIMER_NOW();
  }
  else
  {
    // flanco de bajada, calculo distancia y envio evento
    flight_time = RTIMER_NOW() - initial_time;

    uint32_t distance = (SOUND_VEL * flight_time) / RTIMER_SECOND / 2;
    LOG_INFO("%lu\n", distance);
    if (distance <= MIN_DISTANCE)
      process_post(&sensor, sensor_event, NULL);

    is_rise_edge = true;
  }
}

static void config_pins()
{
  // trigger pin
  gpio_hal_arch_pin_set_output(GPIO_HAL_NULL_PORT, trig);

  // echo pin
  gpio_hal_arch_pin_set_input(GPIO_HAL_NULL_PORT, echo);

  gpio_interrupt_init();
  gpio_hal_pin_cfg_t echo_cfg;

  echo_cfg = GPIO_HAL_PIN_CFG_EDGE_BOTH | GPIO_HAL_PIN_CFG_INT_ENABLE;
  gpio_hal_arch_pin_cfg_set(GPIO_HAL_NULL_PORT, echo, echo_cfg);
  gpio_interrupt_register_handler(ECHO_PIN, echo_handler);
}

PROCESS_THREAD(sensor, ev, data)
{
  PROCESS_BEGIN();

  sensor_event = process_alloc_event();

  config_pins();

  while (1)
  {
    trigger_sensor();
    PROCESS_YIELD();

    if (ev == sensor_event)
    {
      LOG_INFO("Vehiculo detectado\n");

      etimer_set(&et, CLOCK_SECOND / 2);
      PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    }
    trigger_sensor();
  }

  PROCESS_END();
}