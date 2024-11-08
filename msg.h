#pragma once

typedef enum
{
  VEHICLE_DETECTED,
  MAX_VEL_CHANGE,
  DISTANCE_CHANGE,
  VEL_ALERT
} msg_type_t;

typedef struct
{
  msg_type_t type;
  union
  {
    uint64_t t_init;
    uint32_t new_max_vel;
    uint32_t new_distance;
    union {
      uint16_t node_id;
      uint32_t vel;
    } vel_alert_data;
  } value;
} msg_t;
