//
// Created by EmsiaetKadosh on 25-3-22.
//

#include "Entity.h"
#include "..\world\World.h"

void Entity::processVelocity() noexcept {
	location.setPosition(location.getPosition() + lastVelocity.getRelativeLocation(2));
	if (!world) return;
	world->adaptEntityVelocity(*this);
}
