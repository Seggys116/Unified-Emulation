#include <Engine/Core/Input/Keyboard.h>

using namespace UnifiedEngine;


namespace UnifiedEmulation {
    namespace NES {
        struct ControllerButtons{
            bool ControlerNotKeyboard = false;

            Keys Up = Key_UP;
            Keys Down = Key_DOWN;
            Keys Left = Key_LEFT;
            Keys Right = Key_RIGHT;

            Keys Select = Key_RIGHT_SHIFT;
            Keys Start = Key_ENTER;

            Keys A = Key_A;
            Keys B = Key_S;

            Keys X = Key_Z;
            Keys Y = Key_X;
        };
    }
}