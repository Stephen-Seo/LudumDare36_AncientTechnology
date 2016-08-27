
#ifndef COMPONENT_POSITION_HPP
#define COMPONENT_POSITION_HPP

struct Position
{
    Position(float x = 0.0f, float y = 0.0f) :
    x(x),
    y(y)
    {}

    float x;
    float y;
};

#endif

