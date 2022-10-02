#include "CameraClass.h"

//�ϴ� ī�޶� Ŭ���� ���� �� ī�޶��� ��ġ�� ȸ���� �ʱ�ȭ��Ű��
CameraClass::CameraClass()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}

//�̰� ����������̴�
CameraClass::CameraClass(const CameraClass& other)
{

}

CameraClass::~CameraClass()
{

}

void CameraClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;

	return;
}

void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;

	return;
}

//���� ī�޶� Position ��ȯ
D3DXVECTOR3 CameraClass::GetPosition()
{
	return D3DXVECTOR3(m_positionX, m_positionY, m_positionZ);
}

//���� ī�޶� Rotation ��ȯ
D3DXVECTOR3 CameraClass::GetRotation()
{
	return D3DXVECTOR3(m_rotationX, m_rotationY, m_rotationZ);
}

//���� ī�޶��� ��ġ ������ �̿��ؼ� viewMatrix�� �����ؾ� �Ѵ�.
//UpVector, ��ġ, ȸ�� ���� ������ �ʱ�ȭ�ϰ�
//ȭ���� �������� ȸ���� ���� �����ϰ�
//�� ���� �ùٸ� ��ġ�� �̵��Ѵ�.
//Position, LookAt, Up ���� �� �����Ǿ��ٸ� D3DXMatrixLookAtLH �Լ��� ����
//���� ī�޶��� ��ġ�� ȸ�� ������ ǥ���ϴ� �� ����� ����� ��. (�̷� ������ ������ D3D�� ���ش�)
void CameraClass::Render()
{
	D3DXVECTOR3 up, position, lookAt;
	float yaw, pitch, roll;
	D3DXMATRIX rotationMatrix;

	//Up Vector ����
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	//World �������� ī�޶��� ��ġ�� ����
	//��� ������� ���� World �����̴�. (�翬��, ī�޶� World ������ �����ϴ� ���̹Ƿ�)
	position.x = m_positionX;
	position.y = m_positionY;
	position.z = m_positionZ;

	//�⺻������ ī�޶� �ٶ󺸰� �ִ� ������ ���� (Default)
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	//yaw(Y axis), pitch(X axis), roll(Z axis) <- �̰� ī�޶� rotation�� �ǹ��Ѵ�.
	// 
	//�̰��� Radian ������ �����Ѵ�.
	//������ �� ���ڸ�
	//	������� ����̶�� ġ��
	//	Yaw �� ������Ʈ(���) ��ü�� ȸ���ϴ� ���̴�
	//	Pitch �� �� �߿��� � �� ����� ȸ���ϴ� ���̴�. ���⼱ ��ü�� ȸ���Ѵٰ� ġ��
	//	Roll �� �� ��� �ȿ��� � �ൿ�� �ϴ� ���̴�. ���⼱ ���� �����ٰ� ġ��.
	//	�׷��� �Ӹ��� ���������� Yaw �� ���ư���ŭ ���ư��� �ٽ� Pitch �� ���ư���ŭ ���ư��� Roll ����ŭ �������� �Ǵ� ���̴�.

	//�ΰ��� �������� �ٽ� �����ϸ�
	//�츮�� ���𰡸� �ٶ󺼶� ������ �޴°��� �ᱹ 3���� �࿡ ���ؼ���
	//���� ū ��ȭ�� �켱 �츮�� �� ��ü�� ����̴� ���̴�.
	//���𰡸� �ٶ� ���¿��� �����ڿ� ���� �������� �ʰ� ��ü�� ������ ����
	//�翬�� ������ ���� �ٸ���. �̰��� Yaw �̴�. (3D ���Ϳ����� Y������ (0,1,0)�� ���� �ǹ��Ѵ�. Yaw�� Y�� Y������ �ʴ°�)
	//Y�� 1�� ������ �翬�ϰԵ� �츮�� �⺻������ �Ųٷ� ���ְų� ��ü�� ����� ���°� Default��� �������� �ʱ� �����̴�. (�ٸ��͵� ��������.)
	//��ü �������δ� ���� �����. ���� �����̴� ���� (ȸ���ϴ� ����)�� ���� �ٽ� ���� Yaw ���ٴ� ���� ������ �ٲ��. �̰��� Pitch �̴�.
	//�� �������δ� Roll. ���ݱ����� ���·� �����ڸ� ���� ����. �ٽ� Pitch ���ٴ� �� ���� ������ ���� �ٲ��. �̰��� Roll �̴�.
	//�� ��·�� ���������� �̷���.

	//�̰� 3D ī�޶� �������� �����غ���
	//�켱�� 3D ī�޶��� ��ġ�� �־�� �� ���̰�.. (Position)
	//�� 3D ī�޶� �ٶ󺸴� ������ �־�� �� ���̰�.. (lookAt)
	//3D ī�޶� ���� �� ���������ִ��� �ƴ����� �־�� ��.. (upVector)

	//0.0174532925f ���� Degree -> Radian ��ȯ�ϱ� ���� ����̴�.
	pitch = m_rotationX * 0.0174532925f;
	yaw = m_rotationY * 0.0174532925f;
	roll = m_rotationZ * 0.0174532925f;

	//pitch, yaw, roll �� ������ rotation Matrix�� �����.
	D3DXMatrixRotationYawPitchRoll(&rotationMatrix, yaw, pitch, roll);

	//lookAt, up Vector �� ���� rotationMatrix �������� (ī�޶��� ȸ���� ��������, �̷� ȯ���� Coordination �̶�� ��) �ٽ� �����ǵ��� �����
	//�̷��� �����ν� ī�޶� �ٶ󺸰� �ִ� ����, �����Ͱ� ����� �����Ǿ� ������ View �� ���´�.
	//�տ������� ��� ������ ���� �ڷ�, ���� ���� �ڷ�, �����̼� ���� �����ִ� ��Ʈ����
	D3DXVec3TransformCoord(&lookAt, &lookAt, &rotationMatrix);
	D3DXVec3TransformCoord(&up, &up, &rotationMatrix);

	//���� ȸ���� ī�޶��� ��ġ���� ������
	//�������� ���ִµ� �� position + lookAt������.. �� ���� �ҵ�
	lookAt = position + lookAt;

	//��� ���Ͱ� ���� �Ϸ�Ǿ����� ��� ������ ���� �ø��� view Matrix �� ����� �ش�.
	D3DXMatrixLookAtLH(&m_viewMatrix, &position, &lookAt, &up);

	return;
}

//�� ��� ������ ��ģ �ڿ���(Render ���� ��) GetViewMatrix �Լ��� ����Ͽ� �� ����� ���� �� �ְ� �ȴ�.
void CameraClass::GetViewMatrix(D3DXMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}