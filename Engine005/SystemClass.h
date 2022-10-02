#pragma once

#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

//Win32 헤더를 포함하지 않기 위해서 (잘 안쓰는데 시간이 오래걸림)
//define 해줌
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "InputClass.h"
#include "GraphicsClass.h"

class SystemClass
{
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	//콜백 반환시 여러 결과값을 반환해주는 LRESULT 형
	//시스템 메세지 전달용 핸들러 메소드.
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

	//methods
private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

	//variables
private:
	//Long Pointer Constant Wide String <Wide String -> Unicode>, <Long Pointer> 32bit(Under VC++ 6.0) or 64bit(.Net) Pointer
	LPCWSTR m_applicationName;
	//Instance Handle, 윈도우즈 응용프로그램 구별용 Handle (각 컨테이너 ID로 컨트롤)
	HINSTANCE m_hinstance;
	//Windows 창 제어를 위한 Windows Handle
	HWND m_hwnd;

	//Input 받기
	InputClass* m_input;
	//Graphic 처리
	GraphicsClass* m_Graphics;
};

//
// Function Prototypes
//
//메세지를 처리하기 위한 콜백함수 미리 선언.
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//
// Globals
//
//외부 참조용 핸들 포인터.
static SystemClass* ApplicationHandle = 0;

#endif

