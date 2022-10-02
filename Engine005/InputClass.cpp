#include "InputClass.h"

InputClass::InputClass()
{
}

//���������� �ܺ� ����..
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
		//��� Ű�� ������ ���� ������ �ʱ�ȭ
		m_keys[i] = false;
	}

	return;
}

//Ű�� ������
void InputClass::KeyDown(unsigned int input)
{
	//Ű�� ���������� �迭�� ������
	m_keys[input] = true;
	return;
}

//Ű�� ��������
void InputClass::KeyUp(unsigned int input)
{
	//Ű�� �ȴ��������� �迭�� ������
	m_keys[input] = false;
	return;
}

//Ű�� �����ִ��� �������ִ��� Ȯ��
bool InputClass::IsKeyDown(unsigned int key)
{
	//�ش� Ű�� ���� ���¸� ��ȯ
	return m_keys[key];
}