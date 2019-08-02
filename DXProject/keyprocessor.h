#ifndef __KEYPROCESSOR_H__
#define __KEYPROCESSOR_H__

#include <windows.h>

void ProcessKeyUp(UINT keyCode);
void ProcessKeyDown(UINT keyCode);
bool IsKeyDown(UINT keyCode);
void InitialiseKeyboardHandler();

#endif





