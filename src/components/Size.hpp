
#ifndef COMPONENT_SIZE_HPP
#define COMPONENT_SIZE_HPP

struct Size
{
    Size(float width = 1.0f, float height = 1.0f) :
    width(width),
    height(height)
    {}

    float width;
    float height;
};

#endif

