
#ifndef COMPONENT_VELOCITY_HPP
#define COMPONENT_VELOCITY_HPP

struct Velocity
{
    Velocity(float x = 0.0f, float y = 0.0f) :
    x(x),
    y(y)
    {}

    float x;
    float y;
};

#endif

