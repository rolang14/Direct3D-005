#pragma once
#pragma once

#ifndef _INPUTCLASS_H_
#define _INPUTLCASS_H_

//간단하게 시작하기 위해 윈도우에서 제공하는 InputClass 를 사용하는데..
//실제로는 DirecInput 이 훨씬 빠르고 많이 씀. 참고.
class InputClass
{
public:
	//생성자
	InputClass();
	//생성자2
	InputClass(const InputClass&);
	//소멸자
	~InputClass();

	void Initialize();

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int);

private:
	//키 입력
	//bool -> on/off 인지.
	bool m_keys[256];
};

#endif