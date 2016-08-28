
#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include <EC/Meta/Meta.hpp>

#include "Position.hpp"
#include "Velocity.hpp"
#include "Acceleration.hpp"
#include "Rotation.hpp"
#include "AngularVelocity.hpp"
#include "Offset.hpp"
#include "Size.hpp"
#include "Timer.hpp"

using GameComponentsList = EC::Meta::TypeList<
    Position,
    Velocity,
    Acceleration,
    Rotation,
    AngularVelocity,
    Offset,
    Size,
    Timer
>;

struct TPlayer {};
struct TEnemy {};
struct TAsteroid {};
struct TParticle {};
struct TProjectile {};

using GameTagsList = EC::Meta::TypeList<
    TPlayer,
    TEnemy,
    TAsteroid,
    TParticle,
    TProjectile
>;

#endif

