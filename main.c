#include "contiki.h"
#include "dev/gpio-hal.h"
#include "gpio-interrupt.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "sys/int-master.h"

#include <stdio.h>

#define LOG_MODULE "Nodo"
#define LOG_LEVEL LOG_LEVEL_INFO

#define TRIG_PIN BOARD_IOID_DIO21
#define ECHO_PIN BOARD_IOID_DIO22

static struct etimer et;

static gpio_hal_port_t trig = TRIG_PIN;
static gpio_hal_port_t echo = ECHO_PIN;

static void echo_handler(unsigned char pin_mask) {
  LOG_INFO("pin echo detectado\n");
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

PROCESS(trigger, "Sensor Control");
AUTOSTART_PROCESSES(&trigger);

PROCESS_THREAD(trigger, ev, data)
{
  PROCESS_BEGIN();

  config_pins();

  etimer_set(&et, CLOCK_SECOND);

  while(1) {
    PROCESS_YIELD();

    // Subir trig durante 10 us
    // Prender timer
    // Esperar a echo en alto
    // Apagar timer
    // Calcular distancia (x = v*t)

    gpio_hal_arch_toggle_pin(GPIO_HAL_NULL_PORT, trig);

    if(ev == PROCESS_EVENT_TIMER && data == &et) {
      LOG_INFO("Pins are trig=%u, echo=%u\n",
            gpio_hal_arch_read_pin(GPIO_HAL_NULL_PORT, trig),
            gpio_hal_arch_read_pin(GPIO_HAL_NULL_PORT, echo)
      );

      etimer_set(&et, CLOCK_SECOND);
    }
  }

  PROCESS_END();
}