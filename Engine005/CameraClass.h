#pragma once

#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_

#include <D3DX10math.h>

//CameraClass �� ���ݱ��� ����� �� ���۵��� ������ �׸��� �׸��µ�
//�츮�� �̰��� ��� �� �������� ���� ������ �Ѵ� (�⺻������ �츮�� ��� 3D ȭ���� (�����δ� 2D �̰�����) ī�޶� ���� �� �� �ۿ� ����.)
//(�츮 �� ��ü�� ��� ������ ī�޶� �̹Ƿ�)
//�̸� ���ؼ��� ī�޶��� ��ġ�� ���� ȸ�� ���¸� ��� ������ �־�� �ϸ�, �̸� ����ؼ� �����Ͽ� �������ÿ� HLSL ���̴��� ��� �� �����
//�����ؾ� �ϴ����� ���� ������ �־�� �ϴ� ���̴�

//������ �Լ� 4���� ������ ����������,
//���߿� �� FOV ���� �������� ���� �������� �� ���̴�

class CameraClass
{
public:
	//����/�Ҹ�
	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	//ī�޶��� Position, Rotation �ڵ鸵
	void SetPosition(float, float, float);
	void SetRotation(float, float, float);
	
	//ī�޶��� Position, Rotation ��ȯ
	D3DXVECTOR3 GetPosition();
	D3DXVECTOR3 GetRotation();

	//�⺻ �������̽�
	void Render();
	void GetViewMatrix(D3DXMATRIX&);

private:
	//�����
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	//ī�޶󿡼� ������ �� ������ viewMatrix �̹Ƿ�
	D3DXMATRIX m_viewMatrix;
};

#endif // !_CAMERACLASS_H_
