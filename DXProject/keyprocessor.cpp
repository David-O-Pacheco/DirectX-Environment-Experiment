#include "keyprocessor.h"

// Records key states, true meaning key pressed and false not
// Note this array is private to this module
bool KeyStates[256];

// Updates the keystate array to reflect key has been pressed
void ProcessKeyDown(UINT keyCode) {
	KeyStates[keyCode] = true;
}

// Updates the keystate array to reflect key is up
void ProcessKeyUp(UINT keyCode) {
	KeyStates[keyCode] = false;
}

// Returns true when the key code passed as the parameter is down
bool IsKeyDown(UINT keyCode) {
	return KeyStates[keyCode];
}

// Initialise keyboard array to false meaning no keypressed
void InitialiseKeyboardHandler() {
	ZeroMemory(KeyStates, sizeof(KeyStates));
}


