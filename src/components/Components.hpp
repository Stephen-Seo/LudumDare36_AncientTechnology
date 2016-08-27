
#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include <EC/Meta/Meta.hpp>

#include "Position.hpp"
#include "Velocity.hpp"
#include "Acceleration.hpp"

using GameComponentsList = EC::Meta::TypeList<
    Position,
    Velocity,
    Acceleration
>;

using GameTagsList = EC::Meta::TypeList<
>;

#endif

