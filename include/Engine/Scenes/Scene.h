#pragma once

//Includes
#include "../UI/Canvas.h"

namespace UnifiedEngine {
	//Transitioning
	enum Transition_Type {
		None = 0,
		Fade,
		Block,
		Swipe,
		Custom,
	};

	//To Do in a future date
	class Transition {
	private:
		Transition_Type Type;
	public:
		Transition(Transition_Type type) {
			this->Type = type;
		}
	};

	//Main scene
	class Scene {
	public:
		Canvas UI;
	};
}