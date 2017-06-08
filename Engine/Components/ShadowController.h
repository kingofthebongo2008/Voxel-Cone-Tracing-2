#ifndef SHADOWCONTROLLER_H
#define SHADOWCONTROLLER_H

#include "Systems/Components/Component.h"	
#include "Systems/Components/ComponentsFactory.h"

#include "Lighting/DirectionalLight.h"

class ShadowController : public Component
{
public:
	void Start ();

	void Update ();
};

REGISTER_COMPONENT (ShadowController)

#endif