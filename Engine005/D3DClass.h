#pragma once

#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

//��ü ����� ����ϱ� ���� ��ũ�ϴ� ���̺귯������ ���.
//Direct3D�� ���ִ� ��� ��ɵ��� ���⿡ ����.
///
/// Linkings
///
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

///
/// Includes
/// 
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <D3DX10math.h>

class D3DClass
{
public:
	D3DClass();
	D3DClass(const D3DClass&);
	~D3DClass();

	//�ʱ�ȭ �Լ�
	bool Initialize(int, int, bool, HWND, bool, float, float);
	//���� �Լ�
	void Shutdown();

	//�� �ʱ�ȭ �Լ�
	void BeginScene(float, float, float, float);
	//�� ���� �Լ�
	void EndScene();

	//Device ����
	//**********Direct3D ���� Device�� �׷���ī�带 �߻�ȭ�� ��ü�̴�. ���� ���ϸ�,
	//**********�׷��� ī�带 �ٷ� �� �ֵ��� ���ִ� �⺻������ �ٽ����� ��ü�̸�,
	//**********������Ʈ�� �Ҵ� �� ����, ������, �׷��� ����̹� �� �ϵ������� ����� ����Ѵ�.

	//Device �� �ΰ����� �з��Ǵµ�,
	//�� Device Object �� �ַ� �ʱ�ȭ �� ������ �ܰ迡�� �ڿ��� ���� ���𰡸� ����� ����� ������ �Ѵ�.
	ID3D11Device* GetDevice();
	//Device Context �� �������� ���� �������� � �������� �����Ѵ�.
	ID3D11DeviceContext* GetDeviceContext();
	//��Ƽ ������ Ȱ�뿡 �����ϵ��� �̷��� �и��� �Ǿ�����.

	//�ϴ� �˰�Ѿ�ߵɰ���,
	//�⺻������ ȭ�鿡 ���𰡸� �׸���(����)��°� ���۸� ���� �̷������.
	//���۴� ���� ü��(Swap Chain)�� ���ؼ� ĸ��ȭ�Ǿ��ְ�,
	//Front Buffer, Back Buffer 2���� ���۷� �����Ǿ� (ȭ�鿡 �׷����� Front Buffer, ������ �ʴ� ������ �̸� ���� ����� �׷� �ִ� Back Buffer)
	//��ó�� �۵��ϸ�, Back Buffer�� ���� ����� �׷�����(���� ������ ������) �� ���۸� ����(present)�Ͽ� ���� ����� ȭ�鿡 ǥ����.

	//**�����, ����۸� ����Ʈ���ۿ� �����ϴ� ����� Flip�� Blit (�ø��� ��)�� �ִ�.
	//**�ø��� �� ������ �����͸� ��ȯ�ϴ� ���̰� �ſ� ������.
	//**���� ��ȯ�� �ƴ� memcpy �̴�.
	//**�׷��� �����غ���, ���� ���α׷��� ������ ���(â���)�� �����Ѵٰ� �ϸ�,
	//**ȭ�鿡�� ��׶��忡 ����ִ� �� PC�� ������ ȭ��(���⼭�� �׳� ����ȭ���̶�� ��������)
	//**�׸��� ���α׷� ������ �ΰ��� ���ư��� �Ѵ�. 
	//**������ �� ������ �����͸� ��� ����ϴ� �ø� ����� ����� ���� ����.
	//**���� ���������δ� ��� �� ������ ���ư��� �ȴٰ� �ϴµ�... (�������� FullScreen ������ �������� �����찡 �ϳ��̹Ƿ� �ø��̾���)
	//**�ϴ� D3D������ �ø��� ����Ѵٰ� �ϴ� ��������

	//��Ʈ����
	void GetProjectionMatrix(D3DXMATRIX&);
	void GetWorldMatrix(D3DXMATRIX&);
	void GetOrthoMatric(D3DXMATRIX&);

	//����ī�� ����
	void GetVideoCardInfo(char*, int&);

private:
	//�ϴ��� �׳� ���� �������̶�� ����.
	//vsync ����
	bool m_vsync_enabled;
	//�׷���ī�� �޸�
	int m_videoCardMemory;
	//�׷���ī�� �̸�
	char m_videoCardDescription[128];
	//������ �����Ѱ��� ����ü��.
	IDXGISwapChain* m_swapChain;

	//������ ������ ���� ����̽� �� ���ؽ�Ʈ.
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;


	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState;
	D3DXMATRIX m_projectionMatrix;
	D3DXMATRIX m_worldMatrix;
	D3DXMATRIX m_orthoMatrix;
};

#endif