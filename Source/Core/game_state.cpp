#include "game_state.h"

SceneObject* GameState::scene = 0;

FrameGraph GameState::frameGraph;

ImGuiDbgConsole GameState::dbgConsole;

uint64_t GameState::frameCount = 0;
float GameState::deltaTime = 0.0f;
Au::Timer GameState::timer;

GLFWwindow* GameState::window;
//Au::Window* GameState::window;
AudioMixer3D GameState::audioMixer;

InputKeyboardMouseWin32* GameState::keyboardWin32;
Input GameState::input;

