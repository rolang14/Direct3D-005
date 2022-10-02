#include "GraphicsClass.h"

//나중에 내용들이 채워져야 함.
GraphicsClass::GraphicsClass()
{
	//생성자 호출 시 포인터 변수 초기화
	m_D3D = 0;

	//Engine004 추가-------------------------
	//추가된것들도 초기화
	m_Camera = 0;
	m_Model = 0;
	//5강. 셰이더 변경
	//m_ColorShader = 0;
	m_TextureShader = 0;
	//--------------------------------------
}

GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}

GraphicsClass::~GraphicsClass()
{
}

//여기서 D3D 객체를 생성함.
bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	//새로운 Direct 3D 객체 생성
	m_D3D = new D3DClass;
	if (!m_D3D)
	{
		return false;
	}

	//Direct3D 객체 초기화
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	//Engine004 추가-------------------------
	//다시, 추가된것을 Initialize
	//우선 카메라부터 생성
	m_Camera = new CameraClass;
	if (!m_Camera)
	{
		return false;
	}

	//카메라 포지션 설정
	//여기서 Z를 0줘버리면 아마 그리는 물체가 안보일거임 (물체도 (0.0f, 0.0f, 1.0f)에 있기 때문)
	//문제4 카메라 포지션
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->SetRotation(0.0f, 30.0f, 0.0f);

	//이제 보여줄 Model Object 를 생성
	m_Model = new ModelClass;
	if (!m_Model)
	{
		return false;
	}

	//5강. 텍스처 파일 위치도 같이 넘기기
	WCHAR textureFileName[] = TEXT("../Engine005/data/mytexture.dds");
	result = m_Model->Initialize(m_D3D->GetDevice(), textureFileName);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Model Error", MB_OK);
		return false;
	}

	//모델을 만들었으니 이제 Color Shader 생성
	//5강. 텍스처 셰이더로 변경
	//m_ColorShader = new ColorShaderClass;
	//if (!m_ColorShader)
	//{
	//	return false;
	//}

	//result = m_ColorShader->Initialize(m_D3D->GetDevice(), hwnd);
	//if (!result)
	//{
	//	MessageBox(hwnd, L"Could not initialize the color shader object.", L"Color Shader Error", MB_OK);
	//	return false;
	//}
	// 
	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader)
	{
		return false;
	}

	result = m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the Texture Shader object.", L"Texture Shader Error", MB_OK);
		return false;
	}
	//-----------------------

	return true;
}

//종료 시 D3D 객체도 정리
void GraphicsClass::Shutdown()
{
	//Engine004 추가-------------------------
	// Shutdown 도 마찬가지로 추가 호출한것들도 Shutdown 해준다.
	// 이번에는 작동 순서와 역순으로 삭제해준다. (이것도 당연함)

	//5강.
	/*if (m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = 0;
	}*/
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	//---------------------------------------

	//Direct3D 객체가 있으면 삭제하고 NULL
	if (m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}

	return;
}

//매 프레임마다 Render 함수 호출
bool GraphicsClass::Frame()
{
	bool result;

	//그래픽 렌더링을 수행
	result = Render();
	if (!result)
	{
		return false;
	}

	return true;
}

//이제 렌더를 진행할것임.
// 
//Engine004 추가-------------------------
// 자.. 지금까지 HLSL 셰이더를 만들고 하면서 정말 많은 Render 함수 처리가 추가되었다.
// 지금까지는 단순히 BeginScene 로 초기화된 RGB 색을 가진 배경 화면만 출력했다면
// 이제는 셰이더와 Matrix가 전부 고려된 물체를 추가해야 하므로
// 많은 부분이 추가된다.
// 
// 일단은 여전히 화면 초기화부터 시작하고,
// Initialize 함수에서 지정한 카메라의 위치를 토대로 뷰 행렬을 만들기 위해 카메라의 Render를 먼저 호출함
// 뷰 행렬이 만들어지면, 그 복사본을 가져오고
// 다시 D3D Class 객체로부터 나머지 월드, 투영 행렬을 복사해 옴
// 그 행렬들을 이용해 ModelClass->Render 를 호출하여 그래픽 파이프라인에 모델을 그리도록 하는 것임
// ModelClass 내에는 이미 준비된 정점들로 셰이더를 호출하고
// 다시 셰이더는 세 행렬을 사용하여 정점들을 그려내게 되는 것
// 지금까지 그려낸 물체는 백 버퍼에 남아있으며
// 이를 EndScene 호출과 동시에 화면에 표시하는것(스왑)
//---------------------------------------

bool GraphicsClass::Render()
{
	//Engine004 추가-------------------------
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;
	//---------------------------------------

	//씬을 그리기 위해 버퍼의 내용을 지움
	//들어가는 내용은 코드보면 알겠지만 openGL 0~1 컬러임 (RGBA)
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	//Engine004 추가-------------------------
	//Camera의 Render 는 현재 Camera 내의 Position, Rotation 기준으로
	//viewMatrix 를 생성해 준다.
	m_Camera->Render();

	//3개 뷰 world, view, projection Matrix를 가져온다
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	//정점과 인덱스 버퍼를 그래픽 파이프라인에 그리도록 준비한다
	m_Model->Render(m_D3D->GetDeviceContext());

	//컬러 셰이더를 이용하여 모델을 그려낸다
	//5강. 이제 텍스처 셰이더로 그린다.
	/*result = m_ColorShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
	{
		return false;
	}*/

	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model->GetTexture());
	if (!result)
	{
		return false;
	}

	//---------------------------------------

	//버퍼에 그려진 씬을 화면에 표시함
	m_D3D->EndScene();

	return true;
}