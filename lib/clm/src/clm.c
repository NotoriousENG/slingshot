#include <clm.h>

#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

vec2 vec2_new(float x, float y) { return (vec2){x, y}; }

float vec2_length(vec2 v) { return sqrtf(v.x * v.x + v.y * v.y); }

vec2 vec2_normalize(vec2 v) {
  float len = vec2_length(v);
  if (len == 0) {
    return v;
  }
  return (vec2){v.x / len, v.y / len};
}

vec2 vec2_add(vec2 a, vec2 b) { return (vec2){a.x + b.x, a.y + b.y}; }

vec2 vec2_subtract(vec2 a, vec2 b) { return (vec2){a.x - b.x, a.y - b.y}; }

vec2 vec2_scale(vec2 v, float s) { return (vec2){v.x * s, v.y * s}; }

float vec2_distance(vec2 a, vec2 b) {
  float dx = a.x - b.x;
  float dy = a.y - b.y;
  return sqrtf(dx * dx + dy * dy);
}

float vec2_angle_degrees(vec2 a, vec2 b) {
  float angle = atan2f(b.x, b.y) - atan2f(a.x, a.y);
  return angle * 180 / M_PI;
}

float lerpf(float a, float b, float f) { return a * (1.0 - f) + (b * f); }

float clampf(float value, float min, float max) {
  if (value < min) {
    return min;
  } else if (value > max) {
    return max;
  } else {
    return value;
  }
}

float smooth_rotation(float from_rot, float to_rot, float delta) {
  // fix lerping the rotation for 0 and 360 degrees
  if (from_rot > 270 && to_rot < 90) {
    from_rot -= 360;
  }
  if (from_rot < 90 && to_rot > 270) {
    from_rot += 360;
  }

  // lerping the rotation makes it look smoother
  return lerpf(from_rot, to_rot, delta);
}