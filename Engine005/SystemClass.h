#pragma once

#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

//Win32 ����� �������� �ʱ� ���ؼ� (�� �Ⱦ��µ� �ð��� �����ɸ�)
//define ����
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

	//�ݹ� ��ȯ�� ���� ������� ��ȯ���ִ� LRESULT ��
	//�ý��� �޼��� ���޿� �ڵ鷯 �޼ҵ�.
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
	//Instance Handle, �������� �������α׷� ������ Handle (�� �����̳� ID�� ��Ʈ��)
	HINSTANCE m_hinstance;
	//Windows â ��� ���� Windows Handle
	HWND m_hwnd;

	//Input �ޱ�
	InputClass* m_input;
	//Graphic ó��
	GraphicsClass* m_Graphics;
};

//
// Function Prototypes
//
//�޼����� ó���ϱ� ���� �ݹ��Լ� �̸� ����.
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//
// Globals
//
//�ܺ� ������ �ڵ� ������.
static SystemClass* ApplicationHandle = 0;

#endif

