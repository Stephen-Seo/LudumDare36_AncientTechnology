
#ifndef COMPONENT_ANGULAR_VELOCITY_HPP
#define COMPONENT_ANGULAR_VELOCITY_HPP

struct AngularVelocity
{
    AngularVelocity(float vrotation = 0.0f) :
    vrotation(vrotation)
    {}

    float vrotation;
};

#endif

