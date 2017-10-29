#pragma once

// used for angle conversion
static const float PI = 3.14159265;
float degreesToRadians(float degrees) { return degrees * (PI / 180); }
float radiansToDegrees(float radians) { return radians * (180 / PI); }
