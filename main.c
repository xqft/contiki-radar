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

#define CLOCK_10US CLOCK_SECOND/100000

#define TRIG_PIN BOARD_IOID_DIO26
#define ECHO_PIN BOARD_IOID_DIO27

static struct etimer et;

static gpio_hal_port_t trig = TRIG_PIN;
static gpio_hal_port_t echo = ECHO_PIN;

static process_event_t echo_event;

void trigger_sensor() {
  gpio_hal_arch_set_pin(GPIO_HAL_NULL_PORT, trig);
  clock_wait(1);
  gpio_hal_arch_clear_pin(GPIO_HAL_NULL_PORT, trig);
}

PROCESS(trigger, "Sensor Control");
AUTOSTART_PROCESSES(&trigger);

static void echo_handler(unsigned char pin_mask) {
  LOG_INFO("pin echo detectado\n");
  process_post(&trigger, echo_event, NULL);
}

static void config_pins() {
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

PROCESS_THREAD(trigger, ev, data)
{
  static clock_time_t trig_time = 0;
  static clock_time_t flight_time = 0;

  PROCESS_BEGIN();

  echo_event = process_alloc_event();

  config_pins();

  while(1) {
    trigger_sensor();

    // espero flanco de subida
    PROCESS_WAIT_EVENT_UNTIL(ev == echo_event);
    trig_time = RTIMER_NOW();

    // espero flanco de bajada
    PROCESS_WAIT_EVENT_UNTIL(ev == echo_event);
    flight_time = RTIMER_NOW() - trig_time;

    uint32_t distance = 343 * flight_time;
    LOG_INFO("flight_time: %lu\n", flight_time);
    LOG_INFO("distance: %lu\n", distance);

    //gpio_hal_arch_toggle_pin(GPIO_HAL_NULL_PORT, trig);

    etimer_set(&et, CLOCK_SECOND);
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER && data == &et) {
      LOG_INFO("Pins are trig=%u, echo=%u\n",
            gpio_hal_arch_read_pin(GPIO_HAL_NULL_PORT, trig),
            gpio_hal_arch_read_pin(GPIO_HAL_NULL_PORT, echo)
      );
    }
  }

  PROCESS_END();
}