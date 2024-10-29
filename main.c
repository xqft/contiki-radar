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
static uint8_t counter;

static gpio_hal_port_t trig = TRIG_PIN;
static gpio_hal_port_t echo = ECHO_PIN;

void echo_handler(gpio_hal_pin_mask_t pin_mask) {
  LOG_INFO("pin echo detectado");
}
void echo_handler2(unsigned char pin_mask) {
  LOG_INFO("pin echo detectado");
}

// static gpio_hal_event_handler_t echo_event_handler = {
// 	.next = NULL,
// 	.handler = echo_handler,
// 	.pin_mask = gpio_hal_pin_to_mask(ECHO_PIN),
// };

PROCESS(trigger, "Sensor Control");
AUTOSTART_PROCESSES(&trigger);

PROCESS_THREAD(trigger, ev, data)
{
  PROCESS_BEGIN();

  gpio_hal_init();
  // int_master_enable();

  // gpio_hal_register_handler(&echo_event_handler);

  // gpio_hal_pin_cfg_t int_cfg = GPIO_HAL_PIN_CFG_INT_ENABLE | GPIO_HAL_PIN_CFG_EDGE_BOTH | GPIO_HAL_PIN_CFG_PULL_DOWN;
  // gpio_hal_arch_no_port_pin_cfg_set(echo, int_cfg);
  // gpio_hal_arch_interrupt_enable(GPIO_HAL_NULL_PORT, echo);

  // LOG_INFO("config: %lu", gpio_hal_arch_no_port_pin_cfg_get(echo));

  gpio_hal_arch_pin_set_output(GPIO_HAL_NULL_PORT, trig);
  gpio_hal_arch_pin_set_input(GPIO_HAL_NULL_PORT, echo);

  gpio_interrupt_init();
  gpio_interrupt_register_handler(ECHO_PIN, echo_handler2);

  LOG_INFO("is master int enabled: %u\n", int_master_is_enabled());

  counter = 0;

  etimer_set(&et, CLOCK_SECOND);

  while(1) {

    uint32_t pin_mask = (HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0) & GPIO_DIO_ALL_MASK);
    LOG_INFO("pin_mask: %lu\n", pin_mask);

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

      counter++;
      etimer_set(&et, CLOCK_SECOND);
    }
  }

  PROCESS_END();
}