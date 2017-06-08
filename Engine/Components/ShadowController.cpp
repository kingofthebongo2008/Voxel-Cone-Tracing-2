#include "ShadowController.h"

#include "Core/Math/glm/vec3.hpp"
#include "Core/Math/glm/detail/func_geometric.hpp"

#include "Systems/Camera/Camera.h"
#include "Systems/Input/Input.h"
#include "Systems/Time/Time.h"

#include "Lighting/LightsManager.h"

#include "Utils/Extensions/MathExtend.h"

#include "Systems/Window/Window.h"

void ShadowController::Start ()
{
	_directionalLight = LightsManager::Instance ()->GetDirectionalLight (0);
}

void ShadowController::Update ()
{
	if (!Input::GetMouseButton (MOUSE_BUTTON_RIGHT)) {
		return;
	}

	glm::ivec2 mousePosition = Input::GetMousePosition ();

	Transform* lightTransform = _directionalLight->GetTransform ();

	std::size_t windowWidth = Window::GetWidth ();
	std::size_t windowHeight = Window::GetHeight ();

	glm::vec2 normalizedMousePos = glm::vec2 (
		((float) mousePosition.x / windowWidth) * 2.0f - 1.0f,
		((float) mousePosition.y / windowHeight) * 2.0f - 1.0f
	);

	glm::vec3 lightDirection = glm::vec3 (1.0f);

	lightDirection.x = normalizedMousePos.x;
	lightDirection.z = normalizedMousePos.y;

	lightTransform->SetPosition (lightDirection);
}