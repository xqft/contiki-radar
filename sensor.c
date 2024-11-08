#include "contiki.h"
#include "dev/gpio-hal.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "sys/log.h"
#include "sys/int-master.h"

#include "gpio-interrupt.h"
#include "sensor.h"

#define LOG_MODULE "Sensor"
#define LOG_LEVEL LOG_LEVEL_INFO

#define TRIG_PIN BOARD_IOID_DIO26
#define ECHO_PIN BOARD_IOID_DIO27

#define SOUND_VEL 343000 // mm/s
#define MIN_DISTANCE 200 // mm

static struct etimer et;

static gpio_hal_port_t trig = TRIG_PIN;
static gpio_hal_port_t echo = ECHO_PIN;

// Evento al detectar un vehiculo.
process_event_t vehicle_event;
// Evento al recibir una senal del sensor que no se trata de un vehiculo.
process_event_t no_vehicle_event;

// Dispara el sensor
void trigger_sensor()
{
  gpio_hal_arch_set_pin(GPIO_HAL_NULL_PORT, trig);
  clock_wait(1);
  gpio_hal_arch_clear_pin(GPIO_HAL_NULL_PORT, trig);
}

// Administra interrupcion del pin ECHO
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
    // flanco de bajada, calculo distancia y envio eventos
    flight_time = RTIMER_NOW() - initial_time;

    uint32_t distance = (SOUND_VEL * flight_time) / RTIMER_SECOND / 2; // in mm
    if (distance <= MIN_DISTANCE)
    {
      process_post(PROCESS_BROADCAST, vehicle_event, NULL);
    }
    else
    {
      process_post(&handle_sensor, no_vehicle_event, NULL);
    }

    is_rise_edge = true;
  }
}

// Configuracion inicial de pins
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

// Proceso para administrar el sensor
PROCESS(handle_sensor, "Handle sensor");
PROCESS_THREAD(handle_sensor, ev, data)
{
  PROCESS_BEGIN();
  LOG_INFO("Inicio proceso handle_sensor\n");

  vehicle_event = process_alloc_event();
  no_vehicle_event = process_alloc_event();

  config_pins();

  while (1)
  {
    trigger_sensor();
    PROCESS_YIELD();

    if (ev == no_vehicle_event)
    {
      // si no se detecto un vehiculo, vuelve a disparar el sensor inmediatamente.
      continue;
    }
    else if (ev == vehicle_event)
    {
      LOG_INFO("Vehiculo detectado\n");

      // espera 0.5 segundos antes de detectar el siguiente vehiculo.
      etimer_set(&et, CLOCK_SECOND / 2);
      PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    }
  }

  PROCESS_END();
}