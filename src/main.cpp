#define OPENAL_AUDIO
#include <Engine/Core/Game/Game.h>
#include <Emulators/NES/Emulator.h>
#include <Engine/Core/Window/Console.h>

using namespace UnifiedEngine;
using namespace UnifiedEmulation;
using namespace NES;

void UpdateKeys(Game* game) {
	if (game->Input.Keyboard.KeyPressed(Key_ESCAPE)) {
		game->quitApplication();
	}
}

int main(int argc, char*argv[]){
    Game game("Game", 780, 480, true, 30, 4, 6, true);
    NESEmulator emu(&game, "./rsc/Fonts/Font.ttf");

	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	Console::HideConsole();
	#endif
	game.ActiveScene.UI.ScaleWithWindowSize = true;

	*emu.PlayAudio = true;
	emu.RomHotSwap = true;

	if(argc > 1){
		string Arg (argv[1]);
		size_t found = Arg.find("debug");
		if(found != std::string::npos){
			emu.DebugMode = true;

			emu.Debug.Section1 = Debug_Status;
			emu.Debug.Section2 = Debug_Audio;
			emu.Debug.Section3 = Debug_Paletts;
		}
		else{
			emu.DebugMode = false;
		}
	}

	while (!game.getApplicationShouldClose()) {
		game.Update();

		UpdateKeys(&game);
		//updateMouse(&game);

		emu.Update();

		string fps = to_string(1/Time.deltaTime);
		game.window.Title = (fps.substr(0, fps.find(".")) + " FPS").c_str();

		emu.Render();
	}

	glfwTerminate();
	return 0;
}
