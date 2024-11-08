#pragma once

typedef enum
{
  VEHICLE_DETECTED,
  MAX_VEL_CHANGE,
  DISTANCE_CHANGE
} msg_type_t;

typedef struct
{
  msg_type_t type;
  union
  {
    uint64_t t_init;
    uint32_t new_max_vel;
    uint32_t new_distance;
  } value;
} msg_t;
