#pragma once

#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_

#include <D3DX10math.h>

//CameraClass 는 지금까지 만들어 온 버퍼들의 내용을 그리긴 그리는데
//우리가 이것을 어떻게 볼 것인지에 대한 설정을 한다 (기본적으로 우리는 모든 3D 화면을 (실제로는 2D 이겠지만) 카메라를 통해 볼 수 밖에 없다.)
//(우리 눈 자체가 사실 일종의 카메라 이므로)
//이를 위해서는 카메라의 위치와 현재 회전 상태를 계속 가지고 있어야 하며, 이를 계속해서 전달하여 렌더링시에 HLSL 셰이더가 어떻게 뷰 행렬을
//정의해야 하는지에 대한 정보를 주어야 하는 것이다

//지금은 함수 4개로 굉장히 간단하지만,
//나중에 뭐 FOV 등을 넣으려면 더욱 많아져야 할 것이다

class CameraClass
{
public:
	//생성/소멸
	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	//카메라의 Position, Rotation 핸들링
	void SetPosition(float, float, float);
	void SetRotation(float, float, float);
	
	//카메라의 Position, Rotation 반환
	D3DXVECTOR3 GetPosition();
	D3DXVECTOR3 GetRotation();

	//기본 인터페이스
	void Render();
	void GetViewMatrix(D3DXMATRIX&);

private:
	//멤버들
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	//카메라에서 가져야 할 정보는 viewMatrix 이므로
	D3DXMATRIX m_viewMatrix;
};

#endif // !_CAMERACLASS_H_
