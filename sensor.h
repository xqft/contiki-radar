#pragma once

#include "contiki.h"

PROCESS_NAME(handle_sensor);

// Evento al detectar un vehiculo.
extern process_event_t vehicle_event;
// Evento al recibir una senal del sensor que no se trata de un vehiculo.
extern process_event_t no_vehicle_event;