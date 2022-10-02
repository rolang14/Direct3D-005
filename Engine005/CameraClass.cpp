#include "CameraClass.h"

//일단 카메라 클래스 생성 시 카메라의 위치와 회전을 초기화시키자
CameraClass::CameraClass()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}

//이건 복사생성자이다
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

//현재 카메라 Position 반환
D3DXVECTOR3 CameraClass::GetPosition()
{
	return D3DXVECTOR3(m_positionX, m_positionY, m_positionZ);
}

//현재 카메라 Rotation 반환
D3DXVECTOR3 CameraClass::GetRotation()
{
	return D3DXVECTOR3(m_rotationX, m_rotationY, m_rotationZ);
}

//이제 카메라의 위치 정보를 이용해서 viewMatrix를 갱신해야 한다.
//UpVector, 위치, 회전 등의 변수를 초기화하고
//화면의 원점에서 회전을 먼저 수행하고
//그 다음 올바른 위치로 이동한다.
//Position, LookAt, Up 값이 잘 설정되었다면 D3DXMatrixLookAtLH 함수를 통해
//현재 카메라의 위치와 회전 정도를 표현하는 뷰 행렬을 만들어 냄. (이런 복잡한 연산은 D3D가 해준다)
void CameraClass::Render()
{
	D3DXVECTOR3 up, position, lookAt;
	float yaw, pitch, roll;
	D3DXMATRIX rotationMatrix;

	//Up Vector 설정
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	//World 기준으로 카메라의 위치를 설정
	//모든 멤버들은 전부 World 기준이다. (당연히, 카메라도 World 내에서 존재하는 것이므로)
	position.x = m_positionX;
	position.y = m_positionY;
	position.z = m_positionZ;

	//기본적으로 카메라가 바라보고 있는 방향을 정함 (Default)
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	//yaw(Y axis), pitch(X axis), roll(Z axis) <- 이건 카메라 rotation을 의미한다.
	// 
	//이것을 Radian 값으로 세팅한다.
	//설명을 좀 하자면
	//	예를들어 사람이라고 치자
	//	Yaw 는 오브젝트(사람) 자체가 회전하는 것이다
	//	Pitch 는 그 중에서 어떤 한 기관이 회전하는 것이다. 여기선 상체가 회전한다고 치자
	//	Roll 은 그 기관 안에서 어떤 행동을 하는 것이다. 여기선 고개를 돌린다고 치자.
	//	그러면 머리의 관점에서는 Yaw 가 돌아간만큼 돌아가고 다시 Pitch 가 돌아간만큼 돌아가서 Roll 값만큼 움직여야 되는 것이다.

	//인간의 기준으로 다시 설명하면
	//우리가 무언가를 바라볼때 영향을 받는것은 결국 3가지 축에 의해서다
	//가장 큰 변화는 우선 우리가 몸 자체를 기울이는 것이다.
	//무언가를 바라본 상태에서 눈동자와 고개를 움직이지 않고 상체만 움직여 보자
	//당연히 맺히는 상이 다르다. 이것이 Yaw 이다. (3D 벡터에서는 Y축으로 (0,1,0)을 보통 의미한다. Yaw의 Y는 Y축이지 않는가)
	//Y로 1인 이유는 당연하게도 우리가 기본적으로 거꾸로 서있거나 상체를 기울인 상태가 Default라고 생각하지 않기 때문이다. (다른것도 마찬가지.)
	//상체 다음으로는 고개를 저어보자. 고개가 움직이는 방향 (회전하는 방향)에 따라 다시 상이 Yaw 보다는 작은 폭으로 바뀐다. 이것이 Pitch 이다.
	//그 다음으로는 Roll. 지금까지의 상태로 눈동자만 굴려 보자. 다시 Pitch 보다는 더 작은 폭으로 상이 바뀐다. 이것이 Roll 이다.
	//뭐 어쨌든 개념적으로 이렇다.

	//이걸 3D 카메라 차원으로 생각해보면
	//우선을 3D 카메라의 위치가 있어야 할 것이고.. (Position)
	//그 3D 카메라가 바라보는 방향이 있어야 할 것이고.. (lookAt)
	//3D 카메라가 현재 뭐 뒤집어져있는지 아닌지가 있어야 함.. (upVector)

	//0.0174532925f 값은 Degree -> Radian 변환하기 위한 상수이다.
	pitch = m_rotationX * 0.0174532925f;
	yaw = m_rotationY * 0.0174532925f;
	roll = m_rotationZ * 0.0174532925f;

	//pitch, yaw, roll 을 가지고 rotation Matrix를 만든다.
	D3DXMatrixRotationYawPitchRoll(&rotationMatrix, yaw, pitch, roll);

	//lookAt, up Vector 가 현재 rotationMatrix 기준으로 (카메라의 회전값 기준으로, 이런 환경을 Coordination 이라고 함) 다시 설정되도록 계산함
	//이렇게 함으로써 카메라가 바라보고 있는 방향, 업벡터가 제대로 설정되어 온전한 View 가 나온다.
	//앞에서부터 결과 저장할 벡터 자료, 기존 벡터 자료, 로테이션 정보 갖고있는 배트릭스
	D3DXVec3TransformCoord(&lookAt, &lookAt, &rotationMatrix);
	D3DXVec3TransformCoord(&up, &up, &rotationMatrix);

	//이제 회전한 카메라의 위치값을 갱신함
	//벡터합을 해주는데 왜 position + lookAt인지는.. 좀 봐야 할듯
	lookAt = position + lookAt;

	//모든 벡터가 설정 완료되었으니 행렬 연산을 통해 올마른 view Matrix 를 만들어 준다.
	D3DXMatrixLookAtLH(&m_viewMatrix, &position, &lookAt, &up);

	return;
}

//뷰 행렬 생성을 마친 뒤에는(Render 실행 후) GetViewMatrix 함수를 사용하여 뷰 행렬을 얻어올 수 있게 된다.
void CameraClass::GetViewMatrix(D3DXMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}