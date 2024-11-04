#pragma once

typedef enum
{
  VEHICLE_DETECTED,
  MAX_VEL_CHANGE
} msg_type_t;

typedef struct
{
  msg_type_t type;
  union
  {
    uint64_t t_init;
    float new_max_vel;
  } value;
} msg_t;
