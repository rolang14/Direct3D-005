#pragma once

#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

//객체 모듈을 사용하기 위해 링크하는 라이브러리들을 명시.
//Direct3D에 들어가있는 모든 기능들이 여기에 있음.
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

	//초기화 함수
	bool Initialize(int, int, bool, HWND, bool, float, float);
	//종료 함수
	void Shutdown();

	//씬 초기화 함수
	void BeginScene(float, float, float, float);
	//씬 배출 함수
	void EndScene();

	//Device 정보
	//**********Direct3D 에서 Device란 그래픽카드를 추상화한 객체이다. 쉽게 말하면,
	//**********그래픽 카드를 다룰 수 있도록 해주는 기본적으로 핵심적인 객체이며,
	//**********오브젝트의 할당 및 해제, 렌더링, 그래픽 드라이버 및 하드웨어와의 통신을 담당한다.

	//Device 는 두가지로 분류되는데,
	//이 Device Object 는 주로 초기화 및 릴리즈 단계에서 자원과 같이 무언가를 만들고 지우는 역할을 한다.
	ID3D11Device* GetDevice();
	//Device Context 는 렌더링과 같은 실질적인 어떤 행위들을 수행한다.
	ID3D11DeviceContext* GetDeviceContext();
	//멀티 스레딩 활용에 용이하도록 이렇게 분리가 되어있음.

	//일단 알고넘어가야될것이,
	//기본적으로 화면에 무언가를 그린다(렌더)라는건 버퍼를 통해 이루어진다.
	//버퍼는 스왑 체인(Swap Chain)에 의해서 캡슐화되어있고,
	//Front Buffer, Back Buffer 2개의 버퍼로 구성되어 (화면에 그려지는 Front Buffer, 보이지 않는 곳에서 미리 다음 장면을 그려 넣는 Back Buffer)
	//위처럼 작동하며, Back Buffer에 다음 장면이 그려지면(렌더 연산이 끝나면) 두 버퍼를 스왑(present)하여 다음 장면을 화면에 표시함.

	//**참고로, 백버퍼를 프론트버퍼에 전송하는 방법은 Flip과 Blit (플립과 블릿)이 있다.
	//**플립은 두 버퍼의 포인터를 교환하는 것이고 매우 빠르다.
	//**블릿은 교환이 아닌 memcpy 이다.
	//**그러나 생각해보자, 무언가 프로그램을 윈도우 모드(창모드)로 실행한다고 하면,
	//**화면에는 백그라운드에 깔려있는 내 PC의 윈도우 화면(여기서는 그냥 바탕화면이라고 생각하자)
	//**그리고 프로그램 윈도우 두개가 돌아가야 한다. 
	//**때문에 두 버퍼의 포인터를 모두 사용하는 플립 방식을 사용할 수는 없다.
	//**따라서 내부적으로는 모두 다 블릿으로 돌아가게 된다고 하는데... (기존에는 FullScreen 에서는 보여지는 윈도우가 하나이므로 플립이었음)
	//**일단 D3D에서는 플립을 사용한다고 하니 참고하자

	//매트릭스
	void GetProjectionMatrix(D3DXMATRIX&);
	void GetWorldMatrix(D3DXMATRIX&);
	void GetOrthoMatric(D3DXMATRIX&);

	//비디오카드 정보
	void GetVideoCardInfo(char*, int&);

private:
	//일단은 그냥 여러 정보들이라고 보자.
	//vsync 설정
	bool m_vsync_enabled;
	//그래픽카드 메모리
	int m_videoCardMemory;
	//그래픽카드 이름
	char m_videoCardDescription[128];
	//위에서 설명한것이 스왑체인.
	IDXGISwapChain* m_swapChain;

	//위에서 설명한 것이 디바이스 및 컨텍스트.
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