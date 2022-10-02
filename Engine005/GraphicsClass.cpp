#include "GraphicsClass.h"

//���߿� ������� ä������ ��.
GraphicsClass::GraphicsClass()
{
	//������ ȣ�� �� ������ ���� �ʱ�ȭ
	m_D3D = 0;

	//Engine004 �߰�-------------------------
	//�߰��Ȱ͵鵵 �ʱ�ȭ
	m_Camera = 0;
	m_Model = 0;
	//5��. ���̴� ����
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

//���⼭ D3D ��ü�� ������.
bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	//���ο� Direct 3D ��ü ����
	m_D3D = new D3DClass;
	if (!m_D3D)
	{
		return false;
	}

	//Direct3D ��ü �ʱ�ȭ
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	//Engine004 �߰�-------------------------
	//�ٽ�, �߰��Ȱ��� Initialize
	//�켱 ī�޶���� ����
	m_Camera = new CameraClass;
	if (!m_Camera)
	{
		return false;
	}

	//ī�޶� ������ ����
	//���⼭ Z�� 0������� �Ƹ� �׸��� ��ü�� �Ⱥ��ϰ��� (��ü�� (0.0f, 0.0f, 1.0f)�� �ֱ� ����)
	//����4 ī�޶� ������
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->SetRotation(0.0f, 30.0f, 0.0f);

	//���� ������ Model Object �� ����
	m_Model = new ModelClass;
	if (!m_Model)
	{
		return false;
	}

	//5��. �ؽ�ó ���� ��ġ�� ���� �ѱ��
	WCHAR textureFileName[] = TEXT("../Engine005/data/mytexture.dds");
	result = m_Model->Initialize(m_D3D->GetDevice(), textureFileName);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Model Error", MB_OK);
		return false;
	}

	//���� ��������� ���� Color Shader ����
	//5��. �ؽ�ó ���̴��� ����
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

//���� �� D3D ��ü�� ����
void GraphicsClass::Shutdown()
{
	//Engine004 �߰�-------------------------
	// Shutdown �� ���������� �߰� ȣ���Ѱ͵鵵 Shutdown ���ش�.
	// �̹����� �۵� ������ �������� �������ش�. (�̰͵� �翬��)

	//5��.
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

	//Direct3D ��ü�� ������ �����ϰ� NULL
	if (m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}

	return;
}

//�� �����Ӹ��� Render �Լ� ȣ��
bool GraphicsClass::Frame()
{
	bool result;

	//�׷��� �������� ����
	result = Render();
	if (!result)
	{
		return false;
	}

	return true;
}

//���� ������ �����Ұ���.
// 
//Engine004 �߰�-------------------------
// ��.. ���ݱ��� HLSL ���̴��� ����� �ϸ鼭 ���� ���� Render �Լ� ó���� �߰��Ǿ���.
// ���ݱ����� �ܼ��� BeginScene �� �ʱ�ȭ�� RGB ���� ���� ��� ȭ�鸸 ����ߴٸ�
// ������ ���̴��� Matrix�� ���� ����� ��ü�� �߰��ؾ� �ϹǷ�
// ���� �κ��� �߰��ȴ�.
// 
// �ϴ��� ������ ȭ�� �ʱ�ȭ���� �����ϰ�,
// Initialize �Լ����� ������ ī�޶��� ��ġ�� ���� �� ����� ����� ���� ī�޶��� Render�� ���� ȣ����
// �� ����� ���������, �� ���纻�� ��������
// �ٽ� D3D Class ��ü�κ��� ������ ����, ���� ����� ������ ��
// �� ��ĵ��� �̿��� ModelClass->Render �� ȣ���Ͽ� �׷��� ���������ο� ���� �׸����� �ϴ� ����
// ModelClass ������ �̹� �غ�� ������� ���̴��� ȣ���ϰ�
// �ٽ� ���̴��� �� ����� ����Ͽ� �������� �׷����� �Ǵ� ��
// ���ݱ��� �׷��� ��ü�� �� ���ۿ� ����������
// �̸� EndScene ȣ��� ���ÿ� ȭ�鿡 ǥ���ϴ°�(����)
//---------------------------------------

bool GraphicsClass::Render()
{
	//Engine004 �߰�-------------------------
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;
	//---------------------------------------

	//���� �׸��� ���� ������ ������ ����
	//���� ������ �ڵ庸�� �˰����� openGL 0~1 �÷��� (RGBA)
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	//Engine004 �߰�-------------------------
	//Camera�� Render �� ���� Camera ���� Position, Rotation ��������
	//viewMatrix �� ������ �ش�.
	m_Camera->Render();

	//3�� �� world, view, projection Matrix�� �����´�
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	//������ �ε��� ���۸� �׷��� ���������ο� �׸����� �غ��Ѵ�
	m_Model->Render(m_D3D->GetDeviceContext());

	//�÷� ���̴��� �̿��Ͽ� ���� �׷�����
	//5��. ���� �ؽ�ó ���̴��� �׸���.
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

	//���ۿ� �׷��� ���� ȭ�鿡 ǥ����
	m_D3D->EndScene();

	return true;
}