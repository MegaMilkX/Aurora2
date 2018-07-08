#include "game_state.h"

FrameGraph GameState::frameGraph;

ImGuiDbgConsole GameState::dbgConsole;

uint64_t GameState::frameCount = 0;
float GameState::deltaTime = 0.0f;
Au::Timer GameState::timer;

std::stack<GameState*> GameState::stateStack;

GLFWwindow* GameState::window;
//Au::Window* GameState::window;
AudioMixer3D GameState::audioMixer;
GameState::MouseHandler GameState::mouseHandler;
GameState::KeyboardHandler GameState::keyboardHandler;

