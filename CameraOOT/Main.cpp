#include <vector>
#include <iostream>
#include "proc.h"
#include <Windows.h>
#include <Xinput.h>
#include "Time.h"

#pragma comment(user, "Compiled on " __DATE__ " at " __TIME__)
#pragma comment(lib, "XInput.lib")

#define PI 3.14159265359f
#define DeadZoneStick 0.40f //max 1.0f
#define XAngleSpeed 150.0f //Degree Seconde
#define YAngleSpeed 200.0f //~Degree Seconde

using namespace std;

void clearConsole() {
	std::cout << "\x1B[2J\x1B[H";
}

int main()
{
	SetConsoleTitle(L"Oot Camera");
	DWORD procId = GetProcId(L"Zelda Ocarina of Time 4K.exe");
	if (procId == 0)
	{
		std::cout << "Zelda Ocarina of Time 4K not found" << std::endl;
		procId = GetProcId(L"citra-qt.exe");
		if (procId == 0)
		{
			std::cout << "citra-qt.exe not found" << std::endl;
			system("pause");
			return 0;
		}
		else
		{
			std::cout << "citra-qt.exe Found" << std::endl;
		}
	}
	else
	{
		std::cout << "Zelda Ocarina of Time 4K Found" << std::endl;
	}
	HANDLE hProcess = 0;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
	if (hProcess == INVALID_HANDLE_VALUE)
	{
		std::cout << ":C ! Processus invalid" << std::endl;
		system("pause");
		return 1;
	}
	std::cout << "Sucess Open Oot" << std::endl;

	uint8_t pbPatternPP[17] = { 0x2A ,0x00 ,0x00 ,0x60 ,0x6A ,0x8F ,0x09 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x02 ,0xFF ,0x75 ,0x00 };
	uint8_t pbPatternPC[17] = { 0x80 ,0x3F ,0x00 ,0x00 ,0x80 ,0x3F ,0x00 ,0x00 ,0x80 ,0x3F ,0x00 ,0x00 ,0x00 ,0x00 ,0x07 ,0x00 ,0x01 };
	const char* mask = "xxxxxxxxxxxxxxxx";
	uintptr_t LocalPlayer = SearchInProcessMemory(hProcess, pbPatternPP, mask) + 0x33;
	uintptr_t LinkSneek = LocalPlayer - 0x33 + 0x250B;
	uintptr_t LinkOnEpona = LocalPlayer - 0x33 + 0x131;
	printf("[+] Found LocalPlayer @ 0x%X\n", LocalPlayer);
	uintptr_t LocalCamera = SearchInProcessMemory(hProcess, pbPatternPC, mask) + 0xB6;
	uintptr_t LookatCamera = LocalCamera - 0xB6 + 0x60;
	printf("[+] Found LocalCamera @ 0x%X\n", LocalCamera);
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	int controllerId = -1;
	XINPUT_STATE state;

	for (DWORD i = 0; i < XUSER_MAX_COUNT && controllerId == -1; i++)
	{
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		if (XInputGetState(i, &state) == ERROR_SUCCESS)
		{
			std::cout << "[+] Controller " << i << " is connected" << std::endl;
			controllerId = i;
		}
		else
		{
			std::cout << "[X] Controller " << i << " is not connected" << std::endl;
		}
	}
	if (controllerId == -1)
	{
		std::cout << "Controller Not Found" << std::endl;
		system("pause");
		return 0;
	}
	Time time;
	float joystickX = 0.0f;
	float joystickY = 0.0f;
	float baseAngle = 0.0f;
	float baseHauteur = 50.0f;
	double deltaTime = 0;
	time.startTime();
	float theta, dx, dz, dy;
	uint16_t lookLink = 60;
	uint16_t SneekLink = 11012;
	uint16_t EponaLink = 0;
	float lenghtBase = 250.0f;

	bool resetangle = true;
	bool pause = false;
	bool pausePressed = false;
	while (true)
	{
		time.fixedUpdateTime();
		XInputGetState(controllerId, &state);
		if (state.Gamepad.wButtons == XINPUT_GAMEPAD_DPAD_DOWN)
		{
			pausePressed = true;
		}
		else if (pausePressed)
		{
			pausePressed = false;
			pause = !pause;
			resetangle = true;
			clearConsole();
			if (pause)
			{
				system("Color 0C");
				std::cout << "[PAUSE]" << std::endl;
			}
			else
			{
				system("Color 0A");
				std::cout << "[ACTIVE]" << std::endl;
			}
			system("Color 00");
			std::cout << " " << std::endl;
		}
		ReadProcessMemory(hProcess, (void*)(LinkSneek), &SneekLink, sizeof(uint16_t), 0);
		ReadProcessMemory(hProcess, (void*)(LinkOnEpona), &EponaLink, sizeof(uint16_t), 0);
		if (!pause && SneekLink != 21012 && EponaLink != 2448)
		{
			WriteProcessMemory(hProcess, (void*)(LookatCamera), &lookLink, sizeof(uint16_t), 0);
			ReadProcessMemory(hProcess, (void*)(LocalPlayer), &x, sizeof(float), 0);
			ReadProcessMemory(hProcess, (void*)(LocalPlayer + 0x04), &y, sizeof(float), 0);
			ReadProcessMemory(hProcess, (void*)(LocalPlayer + 0x08), &z, sizeof(float), 0);
			if (state.Gamepad.wButtons == XINPUT_GAMEPAD_LEFT_SHOULDER)
			{
				resetangle = true;
			}
			else if (resetangle)
			{
				resetangle = false;
				ReadProcessMemory(hProcess, (void*)(LocalCamera), &dx, sizeof(float), 0);
				ReadProcessMemory(hProcess, (void*)(LocalCamera + 0x04), &dy, sizeof(float), 0);
				ReadProcessMemory(hProcess, (void*)(LocalCamera + 0x08), &dz, sizeof(float), 0);
				float xdx = x - dx;
				float zdz = z - dz;
				baseAngle = atan2(x - dx, z - dz) * -180.0f / PI;
				baseAngle -= 90.0f;
				if (baseAngle < -180.0f)
				{
					baseAngle += 360.0f;
				}
				if (baseAngle > 180.0f)
				{
					baseAngle -= 360.0f;
				}
			}
			joystickX = (float)state.Gamepad.sThumbRX / 32767.0f;
			if (joystickX < DeadZoneStick && joystickX > -DeadZoneStick)
			{
				joystickX = 0.0f;
			}
			joystickY = (float)state.Gamepad.sThumbRY / 32767.0f;
			if (joystickY < DeadZoneStick && joystickY > -DeadZoneStick)
			{
				joystickY = 0.0f;
			}
			baseAngle += time.getFixedDeltaTime() * joystickX * XAngleSpeed;
			if (baseAngle > 180.0f)
			{
				baseAngle = -179.999999f;
			}
			else if (baseAngle < -180.0f)
			{
				baseAngle = 179.9999999f;
			}
			baseHauteur -= time.getFixedDeltaTime() * joystickY * YAngleSpeed;
			if (baseHauteur > 175.0f)
			{
				baseHauteur = 175.0f;
			}
			if (baseHauteur < 0.0f)
			{
				baseHauteur = 0.0f;
			}
			theta = baseAngle * PI / 180.0f;
			dx = (cos(theta) * lenghtBase) + x;
			dy = baseHauteur + y;
			dz = (sin(theta) * lenghtBase) + z;
			if (!resetangle)
			{
				WriteProcessMemory(hProcess, (void*)(LocalCamera), &dx, sizeof(float), 0);
				WriteProcessMemory(hProcess, (void*)(LocalCamera + 0x04), &dy, sizeof(float), 0);
				WriteProcessMemory(hProcess, (void*)(LocalCamera + 0x08), &dz, sizeof(float), 0);
			}
		}
	}
}