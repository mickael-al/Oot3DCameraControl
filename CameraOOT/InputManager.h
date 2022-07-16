#pragma once
#include <Windows.h>
#include <iostream>
class InputManager
{
public:
	void SetupInput(INPUT* ip, int key);
	void InputGetKey(INPUT* ip, bool state);
	void InputGetKeyStay(INPUT* ip, bool state, double dt);
};

