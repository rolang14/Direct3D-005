#include "InputClass.h"

InputClass::InputClass()
{
}

//마찬가지로 외부 참조..
InputClass::InputClass(const InputClass& other)
{
}

InputClass::~InputClass()
{
}

void InputClass::Initialize()
{
	int i;

	for (i = 0; i < 256; i++)
	{
		//모든 키를 눌리지 않은 것으로 초기화
		m_keys[i] = false;
	}

	return;
}

//키가 눌리면
void InputClass::KeyDown(unsigned int input)
{
	//키를 눌린것으로 배열에 저장함
	m_keys[input] = true;
	return;
}

//키가 떼어지면
void InputClass::KeyUp(unsigned int input)
{
	//키가 안눌린것으로 배열에 저장함
	m_keys[input] = false;
	return;
}

//키가 눌려있는지 떼어져있는지 확인
bool InputClass::IsKeyDown(unsigned int key)
{
	//해당 키의 현재 상태를 반환
	return m_keys[key];
}