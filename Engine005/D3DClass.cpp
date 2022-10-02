//������ ���
#include <iostream> //��������¿�
#include <fstream>
#include <string>   //string ����
#include <io.h>		//access �������翩�� Ȯ�ο�

#pragma warning(disable: 4996)
#define _CRT_SECURE_NO_WARNINGS 1

#include "D3DClass.h"

using namespace std;

D3DClass::D3DClass()
{
	//��� �����͵� NULL �ʱ�ȭ
	m_swapChain = 0;
	m_device = 0;
	m_deviceContext = 0;
	m_renderTargetView = 0;
	m_depthStencilBuffer = 0;
	m_depthStencilState = 0;
	m_depthStencilView = 0;
	m_rasterState = 0;
}

D3DClass::D3DClass(const D3DClass& other)
{
}

D3DClass::~D3DClass()
{
}

//screenDepth, Near �� 3D ȯ�濡���� ����. (Depth -> �ָ��ִ� z��, Near -> ������ �ִ� z�� �� ���� �����̴�)
//�� ���� ���� ����� ������� ����Ʈ�� ������ Near ~ Depth ������ �� ����
//vsync �� v��ũ ����� ���(ȭ��~�����~�� �ֻ����� ������ �ֻ����� ����) �Ұ�����
//�����Ұ�����(������� �ֻ����� ������� ������ �� ������ �ٽ� ������) ���� <�׳� ��������ȭ��� ���� �ȴ�.>
bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result;
	//�׷���ī�� �������̽��� �����ϱ� ���� Factory.
	IDXGIFactory* factory;
	//�׷���ī�忡 �޷��ִ� (HDMI�� DP�� �����) ����Ϳ� �����ϱ� ���� Adapter.
	//�翬�� �� ����ʹ� �׷���ī����. �׷���ī�忡 �����ϴϱ�.
	IDXGIAdapter* adapter;
	//������� Output (��´���) �� �ɷ��ִ� ���(�����) ������.
	IDXGIOutput* adapterOutput;
	//numModes -> ���÷��� ��� ��ü ������ (���� ����Ͱ� ���� �� �ִ� ��� ���)
	//i -> ?
	//numerator -> ����� ���ΰ�ħ ������ ����
	//denominator -> ����� ���ΰ�ħ ������ �и�
	//stringLength -> ?
	unsigned int numModes, i, numerator, denominator, stringLength;
	//��ü displayModeList. �迭�� ���� ���̹Ƿ� ������.
	DXGI_MODE_DESC* displayModeList;
	//�����(�׷���ī��)�� Description. (�̸��̶����)
	DXGI_ADAPTER_DESC adapterDesc;
	//�׳� ���� ã���..
	int error;
	//����ü�� ������ ���� swapChain Description.
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	//����� D3D ���� -> featureLevel
	D3D_FEATURE_LEVEL featureLevel;
	//����ۿ� ������
	ID3D11Texture2D* backBufferPtr;
	//���� ���ۿ� ���� ���ذ� �ʿ���.
	//ȭ�鿡 �ѷ����� ��, ���� �ƴ� 3D ��ȭ ���� �� �ٿ������� ����
	//�ᱹ 4D�̵� 3D�̵� 2D ���̽��� ǥ���� �� �ۿ� ����. (��ǻ�� ��ü�� ������� 3������ ����� �ϵ��� ������� ���Ⱑ �ƴϱ� ����)
	//���ذ� �ȵȴٸ� ������ ������ ����
	//�ᱹ 3D ó�� ȭ���� �ѷ��ֱ� ���ؼ��� ���̶�°��� ����� ȯ���� �������־�� �Ѵ�.
	//���� �׸��׸��� �̿��ϴ� 2�� ����, 3�� ���� ���� �� ���̴�.
	// 
	//������ �߿��� ����, �ָ��ִ°� -> ����� �� ������� �׸��� �������� �׸��鼭 �̸� �׷��� �ָ��ִ°͵��� ����� �Ϳ� �Ϻκ� Ȥ�� ��ü��
	//�������� �Ǹ鼭 ���̰��� �����Ǵµ�
	//������ 3������ ������Ʈ�� �� �����Ͱ� 2D�� ���� ������ ������
	//ó���� ���� �� �ָ��ִ°ͺ��� ������������ �����Ͱ� ���´ٰ� ������ �� ����.
	//������ ���� ���� ������� �׸����� �ᱹ �����͸� �ٽ� ���Ͽ� �����ؾ��ϴµ�
	//�翬�� ��ü�� ������ �ð��� ��û ���� �ɸ���, �¹��� ���� �׷��ϱ� ���� Depth(����) ���� ������ �����ִ°�쿡�� ������� �������
	//�Ǵ��� �����Ⱑ ����� ����� ó���� �� ����.
	// 
	//������ D3D ������ ���� ���۸� �Ǵ� z-���۸� �̶�� ����� ����ϸ�
	//�� ����� ����ϸ� ��ü���� �׸��� ������ �����ϰ� ����� ���� ���������� �׸� �� �ִ�.
	//���� ���۸� ����� �ĸ� ������ Ư�� �ȼ� ��ġ�� ��ϵ� �ȼ����� ���̸� ���Ͽ�
	//�����ڿ��� ���� ����� �ȼ��� ���ڰ� �Ǿ� �ĸ� ����(�� ����)�� ��ϵȴ�.
	//���̴� 0.0(�����) ~ 1.0(�ְ� �ָ� �ִ�) ���� ������
	//���� ���� ��ü�� ���� ������ ���������, �̹��� �ڷ�� ������� �ʴ� �ؽ�ó�̴�.

	//���⼭ �ؽ�ó��°���, �ᱹ ��ü�� �ܰ��� ���Ѵ�.
	//�簢���� �׸��ٰ� �����غ���, �ٺ������� �ʿ��� ���� �� 4���� ��ǥ�̴�.
	//�׷���, �� 4���� ��ǥ�� �����´ٰ� �ؼ� �װ��� �츮 ���� �簢������ �������� ���� ���̴�
	//�߿��� ���� �װ��� �ܰ��� ǥ���� �� �ִ� ���� �־�� �ϴ°��̴�.
	//�ؽ�ó�� �� �ܰ��� ��ü�� �ǹ��ϸ�, ȭ�鿡 ǥ���ϴ� ��ü�� ���ζ�� �ص� ������ �ƴϴ�.
	//������ D3D ������ �׷� �ǹ̷� �ؽ�ó�� �ٶ󺸸� �ȴ�. (ǥ���Ǵ� �̹��� �� ��ü)

	//�ᱹ ���������� ����, 3D ȭ���� ǥ���ϴ� ������ �뷫 �̷��ϴ�
	//
	// ---- ���� ȭ��(����Ʈ ����)�� �ѷ����� �ִ� Frame 1�� ȭ�� ----
	// ---- (���� ����) ���� ȭ���� ����� ���� �̹���(��ü)�� ���̸� ���� ���Ͽ� Back Buffer ���� ������ �־��ش� ----
	// ---- (�� ����) ������ ȭ���� ����ִ� << 2D Texture �� �����ֱ� ������ �� ���۴� 2D Texture �������� �� �ۿ� ���� ----
	// ---- (����ü��) Back Buffer �� �̹����� Front Buffer (ȭ��)�� �ѷ��ش� Frame 2 ----
	// ---- �����Ͱ� ���ų� ������ ����ɶ����� ���� ������ �ݺ��Ѵ� ----
	//
	//�ٵ� �׳� ���� �ϳ��� ����ϸ� ���� ���Ϸ� �ڿ��� ����ϰ� �ٲ��ִ��ĸ�
	//���۸� �ϳ��� ����ϸ� �� ���� ȭ���� �����ϰ� �ѷ��ٶ����� ����� ȭ���� ����.
	//������ ȭ���� ��� �����̰� �� ��

	//*****�ٽ� �� ���*****
	//���� ���۴� ������ ���� Z ���� ����� ����� �� �ʿ��� �� (�ش� �ȼ��� ���� ���� �����ϱ� ���� �ʿ��� �����̹Ƿ�)
	//Z ���� ����� ������� ���� �� ������ ���ߴ���ó�� �ȼ��� Sorting ���־�� �ϴµ� �̸� "Z Sort" ����̶�� �Ѵ�.
	//�̸� ���� �ø� �̶�� �Ѵ�. (������ �ʴ� �κ��� ����� ���)
	//�׷��� Z ���۴� �������� ��ü�� �ִ� ��� ����� �������� �ȵɰ��̴� (�������� ��ü�� �տ� ���� ��, ������ ��ü�� ������ �ϴµ�
	//���� ���۷θ� �Ǵ��ع����� �������� ��ü�� Depth ���� �� ���� ���(����� ���), ���İ� ������ ������ ��ü�� ������ �Կ��� �ұ��ϰ�
	//������ �ʰ� ó���ع����� �һ�簡 ���� ���� �����Ƿ�...)
	//�̶��� ���� ����(Alpha Sorting)���� ���� ó���Ͽ� ǥ���� �ش�.
	//���� D11 ���� ���� ���ۿ� ���ٽ� ���۴� 1���� ���ҽ��� �����ϹǷ� [����/���ٽ� ����] ��� ��� ����Ѵ�.

	//�ᱹ, ȭ�鿡 �׸��� �׸��� ����°��� ������ �����ε�
	//�� �׸��̶� ȭ���� 3D�� �ƴ� �̻� 2D�� �� �ۿ� ����
	//�ᱹ ȭ�鿡 2D�ؽ�ó�� �����°Ͱ� ������ ������ ���̴�!



	//������ ������ �� TEXTURE2D �� �����ϴ��� ������ �� ������

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;

	//������ ���Ͻ�Ʈ��
	char filepath[256] = "C:\\Users\\USER-PC\\Desktop\\test\\test.txt";

	//��������ȭ ���� ����
	m_vsync_enabled = vsync;

	//DirectX �׷��� �������̽� ���丮 ����
	//���������� �����ϰ� ���
	//**pointer ���� �� **pointer �� *�� �ι� ����.
	// int i = 1234, int* pnt = &i, int** pointer \ &pnt ��� �� ��,
	// pnt �� ������ ������ i�� �ּҸ�, pointer �� ������ ������ pnt�� �ּҸ� ���� �ִ�.
	// ���⼭ **pointer �� �ּҸ� �ι� (���� �ΰ��ϱ�) �ǳʰ��� i�� �ּ�, �� i�� ���� ���� �����ϸ�
	// *pointer �� �ּҸ� �ѹ� (���� �Ѱ��ϱ�) �ǳʰ��� i�� ����Ű���ִ� pnt �̳��� �ּҸ� ����Ų��.
	// 
	//���� �Ʒ��� ���� ������ ������, factory �� �����ͷ� �̹� ����Ǿ������ ��¿ �� ���� ���� �����͸� ���°��̶� ��������.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	//���� �˰�����, �̷� D3D �޼ҵ���� �⺻������ HRESULT�� �Ǵ��ϱ� ������ result �ڷ����� �׷��ٴ� ��.
	if (FAILED(result))
	{
		return false;
	}

	// ���丮 ��ü�� ����Ͽ� ù��° �׷��� ī�� �������̽��� ���� ����� ����
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// �̹��� �� ����Ϳ��� ���(�����)�� ���� ù��° ����� ����
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// �Ʒ� �ڵ���� ������
	// ������ �� ��� ����� ���÷��� ��带 ����Ͽ�,
	// ������ ��� ���÷��� ��� �߿���
	// ȭ�� �ʺ�/���̿� �´� ���÷��� ��带 ã�� �����ϱ� ������.
	// 
	// DXGI_FORMAT_R8G8B8A8_UNORM ����� ��� ���÷��� ���˿� �´� ����� ������ ����.
	// �⺻������ �����ͷ� �ްڴٴ°�, ���� �� ������ ���� �Ҵ��� �ְڴٴ� �ǹ�
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	//���� ������ numModes �� ���� �Լ����� �����ߴ���� ��� ���÷��� ����� ���� �����������̰�,
	//�� ����ŭ �迭�� �Ҵ��Ͽ� �����ϱ� ���� ����Ʈ�� ������
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	//���÷��� ���鿡 ���� ����Ʈ ������ ä������.
	//�Ʊ�� �Ȱ�����, ����Ʈ ������ �������� ���� ������ ������ �༭ �ű�ٰ� ���÷��� ��� ����Ʈ�� ä���ֵ��� �Ѵ�.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	//���� ������ ���÷��� ��带 ã�� (ȭ�� �ʺ�/���̿� �´� ���鸸)
	//�����ϸ� ������� ���ΰ�ħ ������ �и�, ���ڰ��� ������.
	for (i = 0; i < numModes; i++)
	{
		//���� ũ�Ⱑ ������
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			//���� ũ�⵵ ������
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	//���� �����(�׷���ī��)�� description ������
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	//Byte �����̹Ƿ� 1024 1024 -> KB MB ������ ������ (�׷���ī�� �޸�)
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	//�׷���ī�� �̸��� char�� ���ڿ� �迭�� �ٲ㼭 ������. ���� ũ�Ⱑ 128��.
	//��Ȯ�ϰ� wcstombs_s �� ��Ƽ����Ʈ ������ �������� ���̵� ������ �������� ��ȯ�� ����.
	//��ȯ ������ ���� �޸� �ּ� -> stringLength
	//�״��� ������� ���� �����Ұ��� �󸶸�ŭ �����Ұ��� (�翬�� 128).
	//�� �ٲܰ��� �󸶸�ŭ �ٲܰ��� (�ҽ�. ����͸� �����)
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	//���� �ʿ���� �κе��� �Ҵ� ��������
	//���÷��� ��� ����Ʈ�� �Ҵ��� ������
	delete[] displayModeList;
	displayModeList = 0;

	//��� ����͸� �Ҵ� ������
	adapterOutput->Release();
	adapterOutput = 0;

	//����͸� �Ҵ� ������
	adapter->Release();
	adapter = 0;

	//���丮�� �Ҵ� ������
	factory->Release();
	factory = 0;




	//����, �ֻ����� �˰� �����Ƿ� DirectX�� �ʱ�ȭ �� �� ����.
	//�ϴ� ���� ü���� description ����ü�� ä������.

	//�ϴ� swapChainDesc �ʱ�ȭ
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	//����۴� ���� �ϳ����� �����
	swapChainDesc.BufferCount = 1;

	//������ �ʺ�� ���̸� ������
	//�ʺ�� ���̶�°�, �ϴ� �� ����ü���� ����Ʈ/�� ���۸� �ϳ��� ĵ������� �����ϸ� �ȴ�.
	//�ڿ��� �׸��� �� �׸��� ���� �׸��� �ٲ��ִ°��̴�.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	//����۷� �Ϲ����� 32bit�� �����̽��� ��������.
	//���⼭ �������ڸ�, �ϴ� DXGI�� �翬�� Direct X Graphic Interface ��. Format�� �� �״�� ����
	//�׷� Direct X Graphic Interface �� � �������� ������ ���ΰ�? ���� ���ϴ� ��.. �̹����� ���� ���˰����Ŷ�� ��������.
	//���⼭ r8g8b8a8 �� �� 4ä�� (RGBA)�� ���� 8bit��, �� 32bit�� �����ϰڴٴ� �̾߱��̰�,
	//�ڿ� �ٴ� �� Float, Uint, Unorm, Snorm, Sint ���� �ڷ����� ��Ÿ��.
	//���� ��� ���⼭ ����� UNORM �� 0~1�� ���� ������
	//UINT�� 0~255 (���� ����)
	//SINT�� -127~128 (������ �ڱ� ����) ������ ����ϰڴٴ� ����.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


	//�� ��������, �ֻ����� ������ �־�� ��.
	//1�ʴ� ����̳� �� ���ۿ� ����Ʈ ���۸� �ٲ�ġ�� �� �������� �ɰŰ� (�ֻ����̶� 1�ʿ� ȭ���� ��� ���ΰ�ħ�Ұ��̳İ� �ǹǷ� ������ �ǹ�.)
	//���⼭ vsync == true ���, �ý��ۿ��� ������ ���ΰ�ħ ������ ������ ��.
	//vsync == false ���, ���� ������ �ƴ϶� �������� ���� ������ �����ð��� (Vsync ������ 60Hz���, Vsync �ƴ� ��쿡 �ִ� �� 120Hz ��´ٴ���.)
	//�׷���, �̴� �翬�� ������� �ֻ���(60Hz)�� �ٸ��� ������ ȭ�� �Ϻο� ������ �Ͼ �� ����.

	//vsync Ȱ��ȭ�� �Ǿ�������
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		// 0/1�� 0�̹Ƿ� ������ ���Ѵ�� ����.
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	//������� �뵵�� ������
	//���� ��¿����� ����
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	//�������� �̷���� �������� �ڵ��� ������.
	//�տ��� ���� �������� �ڵ�(hwnd) �ѱ�.
	swapChainDesc.OutputWindow = hwnd;

	//��Ƽ���ø��� ��
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	//������ ��� �Ǵ� Ǯ��ũ�� ��带 ������.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	//��ĵ������ ���İ� ��ĵ���̴��� �������� ��������(unspecified) ������
	//��ĵ������ ���ĸ�..
	//���� TV�� ����ʹ� ȭ�� ����� ���η� �Ǿ��־���.
	//�׷��� �ȼ��� �ػ󵵰� �������� �ʾҴ� TV�� ����Ϳ�
	//ũ�� ��ȭ�� �����̳� �ݴ�� �ػ󵵰� ���� �ʴ� ������ ����� ���
	//����� ���η� �Ǿ��־��� ������ (��ĵ�����̾��� ������)
	//���η� �ȼ��� �����ִ°�ó�� �Ǿ��־���.
	//�� �ڼ��Ѱ� �˻��غ���, ��ư �ʿ���� ��� OFF
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//��µ� ���� ����� ������ ������ �ɼ�.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//�߰� �ɼ� �÷��׸� ������� ����.
	//�߰� �ɼ��� ���� �ִ����� �𸣰�����.. �ϴ��� ���ϴ°�����.
	swapChainDesc.Flags = 0;


	//��������, ���� ü�� description ����ü ������ ��������, ���� ������ �����ؾ� ��.
	//�׳� �� ������ Direct3D �� ����ϰڳĴ� ����.
	//����� ���� ��ǻ��.. �����Ϸ��� DirectX 10 �Ǵ� 9�� �����ؾ� ��.
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	//���� ������ ����ü�� Desc�� �������� ��ġ�� ��ġ ���ؽ�Ʈ, ����ü�� ��ü�� ������
	//��... ���� ���� �ְ� ������ ������ �����༭ ����ٰ� ����ؼ� �־��ּ��� �ϴ°���.
	//����� D3D_DRIVER_TYPE_HARDWARE ��ſ� D3D_DRIVER_TYPE_REFERENCE �� ������,
	//GPU ��ſ� CPU�� ����Ͽ� ������ �����ϰ� �ǹǷ� (����ü�� ���� ����� CPU�� �ϵ��� �Ҵ��Ѵ�..)
	//���� �׷���ī�尡 ���ų� �׷� ȯ�濡����� �̰Ŷ� ���̷�Ʈ ���� ������ �غ��⸦ ������.
	//�Ʒ� ����ó���� else �־ cpu�� �ٲ㵵 �ǰ� ��..
	//+�׷��� ��ġ�� �������� �ʰ� NULL�� �ִ´ٸ�?
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc,
		&m_swapChain, &m_device, NULL, &m_deviceContext);
	if (FAILED(result))
	{
		return false;
	}

	// ����ۿ� �����͸� �Ҵ��ϱ�
	// �տ� NULL�� ���� �𸣰ڴ�..
	// ID3D11Texture2D�� �׸��� ���� ����۸�
	// backBufferPtr�� ����Ű���� �Ѵ�
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

	// ����� �����ͷ� ����Ÿ�� �並 �����ϵ��� �Ѵ�.
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	/********************************************/
	//����Ÿ���̶� �������δ� ǥ�õ� ���� �������� �׷����� �޸� �����̴�.
	//������ �翬�� �� ���۷� ������ ����ϰ���..?
	//�⺻������ ��ǻ�� �޸𸮶�� �ϳ�, PC �޸𸮸� ����ϸ� �ξ��� ������ ���̱⿡ (����ȭ�Ǿ������� ���� �Ӵ���)
	//������ ����, �׸��� ���� �����Ǵ� ����� �׷���ī��(GPU)�� ���� �޸� ����(VRAM)�� �־���� �����Ѵ�.
	/********************************************/

	// ���� �� ���۰� ����Ÿ�ٺ信 ����Ǿ������Ƿ� �ʿ���� ������ ����
	backBufferPtr->Release();
	backBufferPtr = 0;

	// �� ���۰� �� ������ �����ϵ��� �����Ǿ������Ƿ�,
	// ������ 3D ������ �ùٸ��� �׷� �� Depth Buffer (���� ����)�� �̿��Ͽ� 3D ���� ������ �����Ѵ�.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// ���� ���� Description �ۼ�
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	//Mip Map, �� ���̶�� �˻��ϸ� �� ���� ������ ���� ���̴�.
	//�� �����̶�, �� �� ���� �������� ������ �����ϴ� ���ε�,
	//�� ���̶� ������Ʈ�� ����ϴ� �ؽ�ó�� ���� �� ���ϰ� �����ϱ� ���� ���ȴ�.
	//���� �̹���(���ػ�)�� ���� 0����, �׺��� ���� ǰ���� 1, �׺��� �� ���� ǰ���� 2 ������,
	//ǰ���� �������� ������ ���� ������ �ȴ�.
	//������ 2048*2048 �̶�� level 0 �̰�
	//level 1 �϶� 1024*1024 �̷������� ���ư���.
	//�̰� ���ϸ� ȭ���̳� 3D �ִϸ��̼ǿ��� ��Ÿ���� "��Ʒ� ����" �� ���̴µ� ������ �ȴ�.
	//������ �ȼ����� ������ �ڿ� �־ �� �Ⱥ��̴� 
	depthBufferDesc.MipLevels = 1;
	//�� �״�� ���� �迭 ������
	depthBufferDesc.ArraySize = 1;
	//���� ���� ����, �Ʊ� ������ ���� ���� ���ۿ� ���ٽ� ����(������Ʈ �̿��� �������� ȿ���� �׸��� ����)
	//�� ���� ����Ѵٰ� �Ͽ���. ������ �տ��� ���� ���� 24bit, �ڿ��� ���ٽ� ���� 8bit �� �� 32bit �����ϴ� �����̴�.
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//��Ƽ ���ø� ��
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	//�̰� ���� �𸣰ڴ�.. ����Ʈ ����?
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	//���� ���۸� ����/���ٽ� ���۷ν� ����ϰڴٴ� ��
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//CPU�κ��ʹ� ������ ���� �ʰڴٴ� ��. UINT ������ �Ƹ� 0�Ǵ� �׿� �ϰŰ�.. 
	depthBufferDesc.CPUAccessFlags = 0;
	//�� ���� ���� ���ٴ� ��
	depthBufferDesc.MiscFlags = 0;


	//�̻�, Description ������ �̸� �̿��ؼ� ���� ������ �ؽ�ó�� ����
	//����� SubResources ���� �������� �� �𸣰���..
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

	//���� ���� ���� ������ �����µ�
	//�츰 ����/���ٽ� ���۸� ����ҰŴϱ�
	//���ٽ� ���� ���� ������ߵȴ�..........
	//DESC �ʱ�ȭ
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	//������ ����� ���̹Ƿ� true
	depthStencilDesc.DepthEnable = true;
	//�������ڸ�.. ���� ���۸� ����� �� Write Mask �� ����Ұų� ���ųĸ� �Ǵ��ϴ°Ͱ�����
	//Write Mask �� �÷�/����/���ٽ� ��ҵ��� ���� ������ ���ۿ� ������°� ������� ������ ���� ���� �����������̶�� ��
	//�� ���� �����ӿ� ������ų��ε�... MASK ��°� ���� �� �κ��� ������� �ʰڴٴ� ��(�����ٴ� ��) �̹Ƿ�....
	//�𸣰ڶ�
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	//������ ������ ���� �񱳸� ��� �� �������� ���� �����̴�.
	//Comparison Less �� ��� ������ Z ���� ������ Z�� �� ������
	//�� ������ Z�� �� �տ� �������� Z���� �����Ѵ�.
	//���� ���ϸ� ���� ���� 10�� �ִ� ���� �ﰢ���� �ʷ� �ﰢ���� �ִٰ� �� ��
	//������->�ʷϻ� ������ ���ļ� �׸���
	//�������� ���� 10�� �����ϰ�, �ʷϻ��� ���� 10�� �ٽ� �����Ϸ��� ���� ������ Z�� �����Ƿ� �������� �ʴ´�.
	//�̷��� �Ǹ� ������ �ﰢ���� �����ϰ� �������� �ʷϻ� �ﰢ���� �������� ������ ���� ���̴�.
	//��, ���� ���̿� ���� �� ���� ������� �׸��ڴٴ� ���̴�.
	//���߿� �׸��°� �� �տ� ������ ���ڴ� ���� ����
	//D3D11_COMPARISON_LESS_EQUAL �� ����Ѵ�. (���ų� ���� �� �����ϹǷ� �������� �����ϰ� �Ǿ� �ڿ� �� ���� ���ŵǰ� �׷����� �ȴ�)
	//�� ��쿡�� ������->�ʷϻ� ������ �׷��� �� �ʷϻ��� ���̰� �������� ������ ���̴�./
	//�� ���� ������ Windows App Development > Windows/Apps/Win32/API/Direct3D11Graphics/D3d11.h / D3D11_COMPARISON_FUNC enumeration �� ��������
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	//���ٽ� Read Mask ���� 0xFF �� ���� (�Ʒ��� ����)
	//���� �� Mask ���� ���ٽ� ���ذ�(Stencil Reference Value)�� �� AND �����Ͽ�
	//�ٽ� ������ �ȼ� ���� (���ٽ� ���� ���̶�� �Ѵٸ�) Mask ���� �� AND �����ϰ�
	//�� �ΰ��� ����� ���Ͽ� ���ۿ� ������� ������ ���Ѵ�
	//����� ���� ��� ���, ������ ��� ������� ����
	//��, �ڼ����� �𸣰����� ���ٽ� ���� �� ���ۿ� ������� ������ ���ϴ� �񱳰��̶�� �Ǵܵȴ�..
	//�� �� ���� ��ü�� ��� �Ǵ��� �˰������ �˻�����..

	//���⼭�� 0xff(11111111) �����Ƿ� � ���̶� AND �ص� �ڱ� �ڽ��� ���´�.
	//���� � ��Ʈ�� ���� �ʰڴٴ� ��.
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	//FrontFace -> �ȼ��� �ո� �����϶� ���ٽ� ������ ��� �� �������� ���� ������
	// ���� �ﰢ���� ���� ���ٽ� ���� ���� ���
	// 
	//�ȼ� ������ ���ٽ� ������ ���������� ���ٽ� ���۸� �����ϴ� ����� �����ϴ� �ʵ���
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//�ȼ� ������ ���ٽ� ������ ��������� ���� �������� �������� �� ���ٽ� ���۸� �����ϴ� ���
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	//�ȼ� ������ ���ٽ� ����, ���� ������ �� ���������� ��� �Ұ���
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//���ٽ� ���� �� �Լ��� ���� (D3D11_COMPARISION_FUNC ������ ����� ���� �ǰ�, Always �̹Ƿ� �׻� ���� �����ش�)
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//BackFace -> �ȼ��� �޸� �����϶� ���ٽ� ������ ��� �� �������� ���� ������
	// �ĸ� �ﰢ���� ���� ���ٽ� ���� ���� ���
	// 
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//**** �� �ո� �޸��� �����̳��ϸ� ****//
	//�⺻������ ��� ��������� �ﰢ������ ǥ���Ǵµ�
	//�� �������� ������ �� �ﰢ���� ����� ���� �������� �ո� �޸��� �����Ǹ� �̸� Normal (�ﰢ���� ���� ����) �̶�� �Ѵ�.
	//���� ���� �ϴٺ��� Back Face Culling �̶�� ���� �� �� �ִµ�
	//���⼭ ���ϴ� �ø��̶�°��� �ᱹ ������ �ʴ� ���� ���ܽ�Ű�� ������ ���Ѵ�
	//�ᱹ Backface Culling �̶�� ���� ������ �ո�,�޸��� �����Ѵٸ� �� �� �� �߿��� �� �þ߿� ����� �� ����� �������� ���ܽ�Ų�ٴ� �ǹ��̴�.

	//���� ������� ������ ���� ���ٽ� ���� ������ ����
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	//���� ���� ������ �����ϵ��� ������
	//���⼭�� ��ġ ���ؽ�Ʈ ����Ѵ�..
	//���⼭ ��� ����ϴ� OM�̶� Output Merger ����������� ����
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);


	//���� ���� ���ٽ� ���� �信 ���� Description �ۼ�
	//�䰡 �־�� Direct3D�� ���� ���۸� ����-���ٽ� �ؽ��ķ� �ν���
	//�翬�ϰԵ� Depth View �ۿ� ���µ� Depth-Stencil Buffer �� ���� �ٿ����� �ǹ̰� ��������
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	//�Ʊ�� ���� �������� ����/���ٽ� �並 ����� ��.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//����/���ٽ� �信 � �ҽ��� ����� ������ -> 2D �ؽ�ó�� ���ٺ��ϰ��̴�.
	//������ �������� D3D11_DSV_DIMENSION enumeration �� Ȯ���� ��
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//�� �����̽���.. �Ӹ��� ��� ���� �ӵ� ������ ���� �ؽ�ó ȭ���� �����ϴ� ������ �Ѵٰ� �ߴµ�,
	//�� �����̽��� ��� �Ӹ� Ȥ�� ���� ǰ�� ������ ���� ������� ���� ȸ�����ϰ� ������ ���
	//�� ��ġ�� �����ټ��� ǰ���� �ö󰣴ٴµ�.. ��Ȯ�� � ���������� �𸣰ڴ�.
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	//����/���ٽ� ���۸� ����ؼ� ����/���ٽ� �� DESC�� �������� ���� ���ٽ� �並 ����
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	//����Ÿ�� ��� ����/���ٽ� ���۸� ���� ��� ���������ο� ���ε���
	//�̷��� �������ν� �� ������������ �̿��� �������� ����� �� �� ���ۿ� ����� �׷����� ��
	//����Ÿ���� �츮�� ������� ����Ÿ�ٺ�� ���� ���ٽ� �並 ����ϰڴٴ� �ǹ�
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	//�츮�� �������� ������ �׸��� �ȼ�ȭ��Ű�°��� "������ȭ(Rasterize)" �Ѵٰ� �Ѵ�.
	//���� ��� �簢���� ���� ������ ����� �� ���� ������� ĥ�ߴٰ� ������ ����.
	//�̴� �ܼ��� �츮�� ������θ� ������ ��, ���Ƿ� �����Ϸ��� ���� ��ȭ���� ��� �׷��� �Ѵ�
	//�̸� ȭ���̶�� �����ϸ� �ȼ� ��ĭ ��ĭ�� �츮�� �׸� �簢���� ���¿� �°� ä���־�� �Ѵٴ� ���̴�.

	//�Ʒ��� �ٷ� �̰�, ������ȭ�� ���� Description �� �ۼ��ϴ� ���̴�.
	//��Ƽ �ٸ�����̶�°��� �ȼ����� ǰ���� ���� �� �����شٴ� ���̴�.
	//���� ��� ���� �缱�� �׸��ٰ� ������ ����. ��Ƽ �ٸ������ ���ٸ�
	//30�� ������ �׷��� �� �������� ���� �̻��� ���̴�. �翬�ϰԵ� �ȼ��� �׸� ������� ����µ�
	//�� �ȼ��� �׸� ��ĭ ��ü�� ĥ�ϴ� ������ ���� ��, ���ݸ� ĥ�Ѵٰų� ���� ������ŭ ĥ�ϴ� ����� ����.
	//�翬�� �׷��� �ϵ��� ���� ���� ����. (����� ���� �ϳ��� �ݹݾ� ��Ʈ���� �� �ִ°�?)
	//������ �� ��Ƽ �ٸ�����̶�� ������ ��ġ�� (�ܼ��� ����Ѱ� ��)
	//�츮�� �� �����δ� �� �� ���� ���� �ȼ��鿡 �� �밢���� �Ϻ��ϰ� ���̵��� ����� ����ϴ°��̴�.
	//�翬�� �ȼ� �ѳ� �ѳ� ����ϴ°ͺ��� (��Ƽ �ٸ���� 0�ܰ�)
	//�� �ȼ��� ���� �ֺ������� ���� �����ϴ°��� (��Ƽ �ٸ���� 1�ܰ�)
	//�ξ� �ڿ��� ���� �Ҹ��ϰ� �ð��� ���� �ɸ� ���̴�.
	rasterDesc.AntialiasedLineEnable = false;
	//BackFace Culling �̶� ������ �ɼ��̴�. -> D3D11_CULL_BACK
	//D3D11_CULL_MODE �������� ���� �Ǹ�
	//���ϴ� ȿ���� ��� ���� �ϳ��� ���ϱ����� CullMode �� �޸� �ξ� ������ �������ϴ� �˰��� ���� �ִ�.
	//
	//********* Clipping <-> Culling ���� ***********
	// ******** Clipping : �׸� �Ŀ�(���� �Ŀ�) ȭ�鿡 ������ �ʵ��� �߶�
	// ******** Culling : �׸��� ����(���� ����) �Ǵ��Ͽ� �����͸� �߶�
	//
	rasterDesc.CullMode = D3D11_CULL_BACK;
	//���� ������ ������������� ������ z-bias�� �߰������ν� �� �������� ���� ���� �������� ���� ��ó�� ���̰� �� �� �ִ�.
	//���� ������ �׸��ڰ� �����ϰ� ���÷��̵ǵ��� �����ϱ� ���� ���Ǵ� ����̶�� �Ѵ�.
	//���� ��� ������ ��ü�� ���� �������� ���� �׸��ڸ� ������ٰ� ������ ����.
	//���� �������� ���� �׸��ڴ� �Ϻ��� ������ ���̸� ���� ���� ���̴�.
	//�׷��� ������ �� �� ����, �츰 ���ٽ� �Ǵ��� �����ϸ鼭 ���� ���̸� ������ Comparison Less �� ���������ν�
	//�������³� ���ۿ� �ְ� �������� �����ٰ� �����Ͽ���.
	//�� ��� ���� ���� ����, �׸��ڰ� ���� ���� �� ���� ������ ��·�� �� �߿��� ���� ���� �� �����ǰ� �������� ������
	//�ᱹ ���̵� �׸��ڵ� �� �� �ϳ��� �츮 ���� ���̸���� ���� ������ �� �ִ�.
	//�׷� �̻����� ������? �׸��ڸ� �� ���ߴ� ���̶��.
	//������ ���� ���� z-bias (Depth bias) ���� �߰��Ͽ� ������ ������ ������� �Բ� ������ �Ǿ��� ��
	//�� ���� �� �ֵ��� Ȯ���� �����ش�.

	//��.. �Ͽ�ư �׷� ���꿡 ���Ǵ� z-bias ���� �󸶷� �� ������ ����.
	rasterDesc.DepthBias = 0;
	//�̰� ���� ���̾ Ŭ���ο� ���� ���̴�
	rasterDesc.DepthBiasClamp = 0.0f;
	//z-bias Ŭ������ ������� ������..
	//Ŭ���̶�°� �ᱹ �߶󳽴ٴ°ǵ�.. 
	//�Ƹ� ���� Depth Bias Clamp ���� ������ �߶�����ڴٴ� �ǹ̰���.
	//Ŭ������°� ���ĸ�..
	//�켱 Scene Depth �� Pixel Depth �� �˾ƾ� �Ѵ�.
	//�������� ���� �׷��� ������ ȭ�� ��ü�� ���̸� ��ġ�� ǥ���� ���̰� (�ȼ�����)
	//�ȼ������� ���� ���� ���ߴ� ī�޶���� ��� ���̸� ��ġ�� ǥ���� ���̴�.
	//���� �� ���� ������ ������ �ع����� �ش� ������Ʈ�� �� ��ü�� �󸶳� ������ �ִ��� �˷��ش�.
	//�̰� �Ϲ������� �������� ��ƼŬ(Opacity Particle)���� ������Ʈ�� �� �ڸ� �������ϰ� ���߾�� �� �� ���Ǵ� �����ε�
	//Fog ȿ���� �ִ´ٰ� �ϸ�, �� Fog�� �̷�� ���ڵ��� ������ �����ϰ� ���������� ���� ���̴�. ������ ����������
	//���� �߰��� ���� �� �Ȱ��� ���� �ٱ����� ������ �������� ������ �� ���ε�
	//�̶� �� �Ȱ��� ���� ������ �ٶ� �� ���� ���̴�.
	//�׷��� �� Ư���� �Ȱ� ���ڿ� �� �ڿ� �������� �� ������Ʈ�� �Ÿ���ŭ �� �� ���̰ų� �帴�ϰ� ���̰ų� �� ���̴�.
	//�� �Ÿ��� ����ϱ� ���ؼ� �� ��ü�� ������ ī�޶�κ����� ������ ���� ���� ������Ʈ�� ����� �� ����� �Ÿ��� ������� ���̴�.
	//�׷��� �ؼ� Opacity �� ����ϴµ� ���� �Ÿ��� 0~1�� ǥ��ȭ���Ѽ� ������ Opacity�� ����� �־�� �Ѵ�.
	//�׷��� �ȼ� ���� �����ִ°��� Ŭ���� �����شٰ� �Ѵ�.
	//�� �״�� ������ ��� ������ ����? ������ ������ ������� ���� �ȴ�. (��� �׷��Ƚ� ������ �� �ſ� �������̹Ƿ�)
	rasterDesc.DepthClipEnable = true;
	//������ �� �ﰢ��(������)�� ��� ǥ������ �����Ѵ�
	//D3D11_FILL_SOLID -> ��������� ��ĥ�Ѵ�..
	//D3D11_WIREFRAME -> ��������� ��ĥ���ϰ� ���븸 �׸���..
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	//�׷����� ������ ���� �ո�� �޸��� �����ִ� ��
	//CounterClockwise �� false �̹Ƿ� �ð������ �ո����� ����Ѵ�
	//�׷��ϱ�, �ﰢ���� 3 ���� �׸� �� �ð�������� �׸��� �װ��� �ո�
	//�翬�� �ݴ����� ���忡���� �ð�ݴ�������� �׷��� �״� �װ��� �޸��� �ȴٴ� �ǹ���.
	//Direct X �� ��������� �ð������ �ո����� ����Ѵٰ� ��
	rasterDesc.FrontCounterClockwise = false;
	//�ٵ� �ᱹ ��Ƽ�ٸ�����̳� ��Ƽ���ø��̳� �Ȱ��� (��Ƽ �ٸ������ ��Ƽ���ø� ��� ���� �ϳ��̴�)
	//��Ƽ���ø� ���Ѵٴ� �ǹ���
	rasterDesc.MultisampleEnable = false;
	//���� �׽�Ʈ ���� ����
	//���� �׽�Ʈ�� Scissor (����) �� �����Ͽ� Ư���� �κ��� �߶󳻴� ���� �ǹ��Ѵ�.
	//���� ��� ȭ�� �ٱ����� ���� �ȼ����� �߶�����ٰų� �ϴ� ������.
	//�翬�ϰԵ�, ����Ʈ ���� ������ �ڸ� �� ����
	rasterDesc.ScissorEnable = false;
	//������ ��(�׸���)�� ���� ����Ű�� ���� ���� ���̾ ��
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	//�ۼ��� Description ���κ��� ������ȭ ���¸� ������
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		return false;
	}

	//����Ÿ�� ���꿡�� Ŭ������ �����ϱ� ���� ����Ʈ�� �� �������� ��
	//����Ʈ ����
	//������� ȭ��ũ��, ȭ�� ����, ȭ�� �ֻ���� ��ǥ
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	//����Ʈ �Ѱ� ����
	m_deviceContext->RSSetViewports(1, &viewport);

	//Projection Matrix �� �����Ѵ�.
	//3D ȭ���� 2D ����Ʈ �������� ��ȯ�Ͽ� ���̴����� ����� �׸� �� ����ϵ��� ������

	//�̰Թ��ĸ� �ϴ� ó������ ���ߵ��� ����Ʈ�� ȭ�� ��ü�� 3D�� �ƴ� �̻��� 2D�� ǥ���� �Ǿ�� �Ѵٰ� ���ߴ�.
	//�ٸ� �ҽ��� ó������ 2D�� �ƴ϶� (�� ������� ���� ��� �ۼ��Ѵٸ�) 3D�� ���̴�
	//�� 3���� -> 2���� ġȯ�� �ؾ��ϴµ� n -> n-1 ������ ��� ������ ����(Projection) �̶�� �Ѵ�.
	//�� �߿��� �����������̶�� ����� �����ϴµ� �װ� �ٷ� �Ʒ��� D3DXMatrixPerspectiveFovLH

	//���� ��� ���� (Projection Matrix)
	fieldOfView = (float)D3DX_PI / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;



	//�Ʒ��� ��Ʈ�������� �� ForLH �ε� �̴� �޼� ��ǥ�� Perspective�� ����Ѵٴ� �ǹ��̴�.
	//Tekla�� �����غ��� ó�� �����ϸ� �� �޼� ��ǥ�谡 ǥ���̴�.
	//�𸣰����� �÷����� �޼չ�Ģ �ո�� �غ��� ������ ���� ���� �� ����->����->���� ������ Z,Y,X ���̴�. �̰� �޼� ��ǥ��.


	//3D �������� ���� ���� ��� ����
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);

	//������ �ٽ�.. ������Ʈ���� ��ǥ�� 3D�� ��ȯ�ϱ� ���� ���� ����� ������ ��.
	//�̴� 3D ��ǥ �Ӹ� �ƴ϶� 3���� ���������� ȸ��/�̵�/ũ�� ��ȯ���� ����.
	D3DXMatrixIdentity(&m_worldMatrix);

	//�� ������ ���� ���� ��Ʈ���� �������� ������� ���� �� ����� �����ϴ� ���̴�.
	//���� ��鿡�� �츮�� ��� ��ġ����, ��� ������ �����ִ����� ���� ������ ����
	//ī�޶�� �׳� �Ȱ��ٰ� ���� �ȴ�.
	//���߿� ���� ī�޶� Ŭ�������� �� �ڵ��Ұ��̹Ƿ� ���⼭�� ���� �ʴ´�.

	//���������� ���� ���� ����� ������ �Ѵ�.
	//�̰� 3D ��ü���� ���� ��Ʈ������ �ƴ϶�, 2D UI ���� ���� ��Ʈ�����̰�
	//���߿� 2D �׷���, ��Ʈ�� �ٷ�� �������� ���� �� ����
	D3DXMatrixOrthoLH(&m_orthoMatrix, (float)screenWidth, (float)screenHeight, screenNear, screenDepth);



	//������ �׷���ī�� �̸� �޸� ���
	//���Ͽ���
	FILE* fp = fopen(filepath, "w+");
	//���Ͽ� �������� fout ��Ʈ�� ����
	ofstream fout(fp);

	char vga_name[256] = "";
	int vga_mem;

	GetVideoCardInfo(vga_name, vga_mem);

	if (fout.is_open())
	{
		fout << "Graphics Card Name : " << vga_name << "\n";
		fout << "Graphics Card Memory : " << vga_mem << "\n";
		fout.close();
	}

	//���� �ݱ�
	fclose(fp);

	return true;
}

//������ �� �Ҵ�� ��� �����͵��� �����ϰ� �����ؾ� ��
//���� ü���� ��� Ǯ��ũ�� ��忡�� �����ع����� ������ ���� ����ٰ� �Ѵ�.. (�Ƹ� Ǯ��ũ���� ��� ��ü �ػ󵵿� ������ �ػ󵵰� �ٸ��� ������ ������ ����°� �ƴұ� ��)
//������ �ݵ�� ������ ���� �ٲ� �� ���� ü���� �����Ͽ��� ��
void D3DClass::Shutdown()
{
	if (m_swapChain)
	{
		//������ ���� ���ϰ� NULL
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if (m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if (m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}

	return;
}

//D3DClass�� �����ϴ� ����� �Լ���
//BeginScene : �� �������� ���۸��� 3D ȭ���� �׸��� ������ �� ȣ���.
//���۸� �� ������ �ʱ�ȭ�ϰ� �������� �� �̷�������� �غ���.
//EndScene : �� �������� ������ �����ӿ� ȣ��Ǿ�, ��<->����Ʈ ������ �����ϰ� ���� ü�ο��� 3Dȭ���� ǥ���ϰ� �Ѵ�.
//Device, Context Getter
//Matrixies Getter
//�׷���ī�� ���� ��ȯ
void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];

	//RGBA�� ����ؼ� ����
	//������ �޶����� �̰͵� �޶����� �Ұ���
	//���۸� � ������ �ʱ�ȭ �� �� RGBA ���� �Է�
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	//����� ������ ������ ���� Į��� �ʱ�ȭ (RGBA)
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	//���� ���۸� �ʱ�ȭ
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}

void D3DClass::EndScene()
{
	// �������� �Ϸ�Ǿ����Ƿ� ������� ������ ȭ�鿡 ǥ���մϴ�.
	//Vsync �� Ȱ��ȭ�Ǿ������� ����ü�ο��� ���ΰ�ħ ������ �����ϵ��� ����
	//Vsync �� ����� �ֻ����� ���ߴ� ���̹Ƿ� �̹� Initialize ���� ������ �Ǿ��ִ�. (����� �ֻ��� ��)
	if (m_vsync_enabled)
	{
		//present : �غ�� ȭ���� �����̱�
		m_swapChain->Present(1, 0);
	}
	//�װ� �ƴϸ� ������ �� ������ ǥ�� (0,0)
	else
	{
		m_swapChain->Present(0, 0);
	}

	return;
}

//����̽�, ���ؽ�Ʈ Getter
ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}

ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}

//�ٽ�, ����, ����, ���� ��� Getter (Reference)
void D3DClass::GetProjectionMatrix(D3DXMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}

void D3DClass::GetWorldMatrix(D3DXMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}

void D3DClass::GetOrthoMatric(D3DXMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}

//�׷���ī�� ���� ��ȯ
void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}