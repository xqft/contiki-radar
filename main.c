#include "contiki.h"
#include "dev/gpio-hal.h"
#include "sys/etimer.h"

#include <stdio.h>

static struct etimer et;
static uint8_t counter;

#if GPIO_HAL_PORT_PIN_NUMBERING
extern gpio_hal_port_t out_port1, out_port2_3;
extern gpio_hal_port_t btn_port;
#else
#define out_port1   GPIO_HAL_NULL_PORT
#define out_port2_3 GPIO_HAL_NULL_PORT
#define btn_port    GPIO_HAL_NULL_PORT
#endif

// Pins 
static gpio_hal_port_t out_pin1 = BOARD_IOID_LED_1;
static gpio_hal_port_t out_pin2 = BOARD_IOID_LED_1;

PROCESS(sensor, "Sensor Control");
AUTOSTART_PROCESSES(&sensor);

PROCESS_THREAD(sensor, ev, data)
{
  PROCESS_BEGIN();

  counter = 0;

  etimer_set(&et, CLOCK_SECOND);

  while(1) {

    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER && data == &et) {
      gpio_hal_arch_pin_set_output(out_port1, out_pin1);
      gpio_hal_arch_pin_set_output(out_port2_3, out_pin2);

      gpio_hal_arch_write_pin(out_port1, out_pin1, 1);

      printf("Pins are 1=%u, 2=%u\n",
            gpio_hal_arch_read_pin(out_port1, out_pin1),
            gpio_hal_arch_read_pin(out_port2_3, out_pin2)
      );

      counter++;
      etimer_set(&et, CLOCK_SECOND);
    }
  }

  PROCESS_END();
}