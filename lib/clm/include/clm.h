#pragma once

typedef struct vec2 {
  float x;
  float y;
} vec2;

// Create a new vector
vec2 vec2_new(float x, float y);

// Get the length of a vector
float vec2_length(vec2 v);

// Normalize a vector
vec2 vec2_normalize(vec2 v);

// Add two vectors
vec2 vec2_add(vec2 a, vec2 b);

// Subtract two vectors
vec2 vec2_subtract(vec2 a, vec2 b);

// Scale a vector
vec2 vec2_scale(vec2 a, float b);

// Get the distance between two vectors
float vec2_distance(vec2 a, vec2 b);

// Get the angle between two vectors in degrees
float vec2_angle_degrees(vec2 a, vec2 b);

// Linear interpolation between two floats
float lerpf(float a, float b, float f);

// Clamp a float between a min and max value
float clampf(float value, float min, float max);

float smooth_rotation(float from_rot, float to_rot, float delta);