#pragma once
#pragma once

#ifndef _INPUTCLASS_H_
#define _INPUTLCASS_H_

//�����ϰ� �����ϱ� ���� �����쿡�� �����ϴ� InputClass �� ����ϴµ�..
//�����δ� DirecInput �� �ξ� ������ ���� ��. ����.
class InputClass
{
public:
	//������
	InputClass();
	//������2
	InputClass(const InputClass&);
	//�Ҹ���
	~InputClass();

	void Initialize();

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int);

private:
	//Ű �Է�
	//bool -> on/off ����.
	bool m_keys[256];
};

#endif