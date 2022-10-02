//예제용 헤더
#include <iostream> //파일입출력용
#include <fstream>
#include <string>   //string 사용용
#include <io.h>		//access 파일존재여부 확인용

#pragma warning(disable: 4996)
#define _CRT_SECURE_NO_WARNINGS 1

#include "D3DClass.h"

using namespace std;

D3DClass::D3DClass()
{
	//모든 포인터들 NULL 초기화
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

//screenDepth, Near 은 3D 환경에서의 깊이. (Depth -> 멀리있는 z값, Near -> 가까이 있는 z값 에 대한 설정이다)
//즉 위와 같이 만들면 만들어질 뷰포트의 뎁스는 Near ~ Depth 까지가 될 것임
//vsync 는 v싱크 기능을 사용(화면~모니터~의 주사율에 렌더링 주사율을 맞춤) 할것인지
//사용안할것인지(모니터의 주사율에 관계없이 가능한 한 빠르게 다시 렌더함) 지정 <그냥 수직동기화라고 보면 된다.>
bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result;
	//그래픽카드 인터페이스에 접근하기 위한 Factory.
	IDXGIFactory* factory;
	//그래픽카드에 달려있는 (HDMI나 DP로 연결된) 모니터에 접근하기 위한 Adapter.
	//당연히 그 어댑터는 그래픽카드임. 그래픽카드에 연결하니깐.
	IDXGIAdapter* adapter;
	//어댑터의 Output (출력단자) 에 걸려있는 출력(모니터) 정보임.
	IDXGIOutput* adapterOutput;
	//numModes -> 디스플레이 모드 전체 가짓수 (현재 모니터가 가질 수 있는 모든 모드)
	//i -> ?
	//numerator -> 모니터 새로고침 비율의 분자
	//denominator -> 모니터 새로고침 비율의 분모
	//stringLength -> ?
	unsigned int numModes, i, numerator, denominator, stringLength;
	//전체 displayModeList. 배열을 받을 것이므로 포인터.
	DXGI_MODE_DESC* displayModeList;
	//어댑터(그래픽카드)의 Description. (이름이라던지)
	DXGI_ADAPTER_DESC adapterDesc;
	//그냥 에러 찾기용..
	int error;
	//스왑체인 설정을 위한 swapChain Description.
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	//사용할 D3D 버전 -> featureLevel
	D3D_FEATURE_LEVEL featureLevel;
	//백버퍼용 포인터
	ID3D11Texture2D* backBufferPtr;
	//뎁스 버퍼에 대해 이해가 필요함.
	//화면에 뿌려지는 것, 흔히 아는 3D 영화 등은 그 근원적으로 들어가면
	//결국 4D이든 3D이든 2D 베이스로 표현될 수 밖에 없다. (컴퓨터 자체가 어셈블리로 3차원의 계산을 하도록 만들어진 계산기가 아니기 때문)
	//이해가 안된다면 스스로 생각해 보고
	//결국 3D 처럼 화면을 뿌려주기 위해서는 깊이라는것이 고려된 환경을 구성해주어야 한다.
	//흔히 그림그릴때 이용하는 2점 투시, 3점 투시 등이 그 예이다.
	// 
	//하지만 중요한 것은, 멀리있는것 -> 가까운 것 순서대로 그리면 가까운것을 그리면서 미리 그려진 멀리있는것들은 가까운 것에 일부분 혹은 전체가
	//가려지게 되면서 깊이감이 형성되는데
	//문제는 3차원의 오브젝트는 그 데이터가 2D에 비해 굉장히 많으며
	//처음에 들어올 때 멀리있는것부터 내림차순으로 데이터가 들어온다고 단정할 수 없다.
	//때문에 위와 같은 기법으로 그리려면 결국 데이터를 다시 비교하여 정리해야하는데
	//당연히 물체가 많으면 시간이 엄청 오래 걸리고, 맞물린 형태 그러니까 가령 Depth(깊이) 축이 같은데 겹쳐있는경우에는 어느것이 가까운지
	//판단을 내리기가 어려워 제대로 처리할 수 없다.
	// 
	//떄문에 D3D 에서는 깊이 버퍼링 또는 z-버퍼링 이라는 기법을 사용하며
	//이 기법을 사용하면 물체들을 그리는 순서와 무관하게 제대로 서로 가려지도록 그릴 수 있다.
	//깊이 버퍼링 방식은 후면 버퍼의 특정 픽셀 위치에 기록될 픽셀들의 깊이를 비교하여
	//관찰자에게 가장 가까운 픽셀이 승자가 되어 후면 버퍼(백 버퍼)에 기록된다.
	//깊이는 0.0(가까운) ~ 1.0(최고 멀리 있는) 값을 가지며
	//깊이 버퍼 자체는 깊이 정보만 담고있을뿐, 이미지 자료는 담고있지 않는 텍스처이다.

	//여기서 텍스처라는것은, 결국 물체의 외관을 뜻한다.
	//사각형을 그린다고 생각해보면, 근본적으로 필요한 것은 점 4개의 좌표이다.
	//그러나, 점 4개의 좌표만 찍어놓는다고 해서 그것이 우리 눈에 사각형으로 보이지는 않을 것이다
	//중요한 것은 그것의 외관을 표현할 수 있는 면이 있어야 하는것이다.
	//텍스처는 그 외관면 자체를 의미하며, 화면에 표현하는 물체의 전부라고 해도 과언이 아니다.
	//때문에 D3D 에서는 그런 의미로 텍스처를 바라보면 된다. (표현되는 이미지 그 자체)

	//결국 종합적으로 보면, 3D 화면을 표현하는 순서는 대략 이러하다
	//
	// ---- 현재 화면(프론트 버퍼)에 뿌려지고 있는 Frame 1의 화면 ----
	// ---- (깊이 버퍼) 다음 화면을 만들기 위해 이미지(물체)의 깊이를 서로 비교하여 Back Buffer 으로 정리해 넣어준다 ----
	// ---- (백 버퍼) 정리된 화면을 담고있다 << 2D Texture 를 갖고있기 때문에 백 버퍼는 2D Texture 포인터일 수 밖에 없다 ----
	// ---- (스왑체인) Back Buffer 의 이미지를 Front Buffer (화면)에 뿌려준다 Frame 2 ----
	// ---- 데이터가 없거나 렌더가 종료될때까지 위의 사항을 반복한다 ----
	//
	//근데 그냥 버퍼 하나만 사용하면 되지 뭐하러 뒤에서 계산하고 바꿔주느냐면
	//버퍼를 하나만 사용하면 그 다음 화면을 구성하고 뿌려줄때까지 사용할 화면이 없다.
	//때문에 화면이 계속 깜빡이게 될 것

	//*****다시 안 사실*****
	//깊이 버퍼는 위에서 말한 Z 버퍼 기법을 사용할 때 필요한 것 (해당 픽셀의 깊이 값을 저장하기 위해 필요한 버퍼이므로)
	//Z 버퍼 기법을 사용하지 않을 때 위에서 말했던것처럼 픽셀을 Sorting 해주어야 하는데 이를 "Z Sort" 기법이라고 한다.
	//이를 보통 컬링 이라고 한다. (보이지 않는 부분을 숨기는 기능)
	//그런데 Z 버퍼는 불투명한 물체가 있는 경우 제대로 렌더링이 안될것이다 (불투명한 물체가 앞에 있을 때, 뒤쪽의 물체가 보여야 하는데
	//깊이 버퍼로만 판단해버리면 불투명한 물체의 Depth 값이 더 작을 경우(가까울 경우), 알파값 떄문에 뒤쪽의 물체가 보여야 함에도 불구하고
	//보이지 않게 처리해버리는 불상사가 있을 수도 있으므로...)
	//이때는 알파 소팅(Alpha Sorting)까지 같이 처리하여 표현해 준다.
	//보통 D11 에서 깊이 버퍼와 스텐실 버퍼는 1개의 리소스를 공유하므로 [깊이/스텐실 버퍼] 라고 묶어서 취급한다.

	//결국, 화면에 그리는 그림을 만드는것이 버퍼의 역할인데
	//그 그림이란 화면이 3D가 아닌 이상 2D일 수 밖에 없고
	//결국 화면에 2D텍스처를 입히는것과 동일한 동작인 것이다!



	//이정도 했으면 왜 TEXTURE2D 로 선언하는지 이해할 수 있을듯

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;

	//예제용 파일스트링
	char filepath[256] = "C:\\Users\\USER-PC\\Desktop\\test\\test.txt";

	//수직동기화 설정 저장
	m_vsync_enabled = vsync;

	//DirectX 그래픽 인터페이스 팩토리 생성
	//이중포인터 간단하게 상기
	//**pointer 있을 때 **pointer 는 *가 두번 있음.
	// int i = 1234, int* pnt = &i, int** pointer \ &pnt 라고 할 때,
	// pnt 는 본인의 값으로 i의 주소를, pointer 는 본인의 값으로 pnt의 주소를 갖고 있다.
	// 여기서 **pointer 는 주소를 두번 (별이 두개니깐) 건너가서 i의 주소, 즉 i의 값에 접근 가능하며
	// *pointer 는 주소를 한번 (별이 한개니깐) 건너가서 i를 가리키고있는 pnt 이놈의 주소를 가리킨다.
	// 
	//따라서 아래의 더블 포인터 변수는, factory 가 포인터로 이미 선언되어버려서 어쩔 수 없이 더블 포인터를 쓰는것이라 생각하자.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	//보면 알겠지만, 이런 D3D 메소드들은 기본적으로 HRESULT로 판단하기 때문에 result 자료형이 그렇다는 점.
	if (FAILED(result))
	{
		return false;
	}

	// 팩토리 객체를 사용하여 첫번째 그래픽 카드 인터페이스에 대한 어댑터 생성
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// 이번엔 그 어댑터에서 출력(모니터)에 대한 첫번째 어댑터 나열
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// 아래 코드들의 목적은
	// 가능한 한 모든 경우의 디스플레이 모드를 계산하여,
	// 가능한 모든 디스플레이 모드 중에서
	// 화면 너비/높이에 맞는 디스플레이 모드를 찾아 세팅하기 위함임.
	// 
	// DXGI_FORMAT_R8G8B8A8_UNORM 모니터 출력 디스플레이 포맷에 맞는 모드의 개수를 구함.
	// 기본적으로 포인터로 받겠다는건, 뭔가 그 변수에 값을 할당해 주겠다는 의미
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	//따라서 여기의 numModes 는 위의 함수에서 전달했던대로 모든 디스플레이 모드의 수가 적혀있을것이고,
	//그 수만큼 배열을 할당하여 저장하기 위한 리스트를 생성함
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	//디스플레이 모드들에 대한 리스트 구조를 채워넣음.
	//아까랑 똑같은데, 리스트 개수를 구했으니 이제 포인터 변수를 줘서 거기다가 디스플레이 모드 리스트를 채워넣도록 한다.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	//이제 가능한 디스플레이 모드를 찾고 (화면 너비/높이에 맞는 모드들만)
	//적절하면 모니터의 새로고침 비율의 분모, 분자값을 저장함.
	for (i = 0; i < numModes; i++)
	{
		//가로 크기가 같으면
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			//세로 크기도 같으면
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	//이젠 어댑터(그래픽카드)의 description 가져옴
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	//Byte 단위이므로 1024 1024 -> KB MB 단위로 가져옴 (그래픽카드 메모리)
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	//그래픽카드 이름을 char형 문자열 배열로 바꿔서 저장함. 각각 크기가 128임.
	//정확하게 wcstombs_s 는 멀티바이트 문자의 시퀀스를 와이드 문자의 시퀀스로 변환을 해줌.
	//변환 개수를 담을 메모리 주소 -> stringLength
	//그다음 순서대로 어디다 저장할건지 얼마만큼 저장할건지 (당연히 128).
	//뭘 바꿀건지 얼마만큼 바꿀건지 (소스. 어댑터를 사용함)
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	//이제 필요없는 부분들을 할당 해제해줌
	//디스플레이 모드 리스트의 할당을 해제함
	delete[] displayModeList;
	displayModeList = 0;

	//출력 어댑터를 할당 해제함
	adapterOutput->Release();
	adapterOutput = 0;

	//어댑터를 할당 해제함
	adapter->Release();
	adapter = 0;

	//팩토리를 할당 해제함
	factory->Release();
	factory = 0;




	//이제, 주사율을 알고 있으므로 DirectX를 초기화 할 수 있음.
	//일단 스왑 체인의 description 구조체를 채워야함.

	//일단 swapChainDesc 초기화
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	//백버퍼는 보통 하나만을 사용함
	swapChainDesc.BufferCount = 1;

	//버퍼의 너비와 높이를 설정함
	//너비와 높이라는건, 일단 이 스왑체인의 프론트/백 버퍼를 하나의 캔버스라고 생각하면 된다.
	//뒤에서 그림을 다 그리면 앞의 그림과 바꿔주는것이다.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	//백버퍼로 일반적인 32bit의 서페이스를 지정해줌.
	//여기서 설명하자면, 일단 DXGI는 당연히 Direct X Graphic Interface 임. Format은 말 그대로 포맷
	//그럼 Direct X Graphic Interface 를 어떤 형식으로 지정할 것인가? 흔히 말하는 뭐.. 이미지의 저장 포맷같은거라고 생각하자.
	//여기서 r8g8b8a8 은 총 4채널 (RGBA)를 각각 8bit씩, 총 32bit로 구성하겠다는 이야기이고,
	//뒤에 붙는 뭐 Float, Uint, Unorm, Snorm, Sint 등은 자료형을 나타냄.
	//예를 들어 여기서 사용한 UNORM 은 0~1의 값을 가지며
	//UINT는 0~255 (오직 정수)
	//SINT는 -127~128 (원래의 자기 범위) 포맷을 사용하겠다는 것임.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


	//그 다음으로, 주사율을 설정해 주어야 함.
	//1초당 몇번이나 백 버퍼와 프론트 버퍼를 바꿔치기 할 것인지가 될거고 (주사율이란 1초에 화면을 몇번 새로고침할것이냐가 되므로 동일한 의미.)
	//여기서 vsync == true 라면, 시스템에서 설정한 새로고침 비율로 고정이 됨.
	//vsync == false 라면, 고정 비율이 아니라 가능한한 빠른 비율을 가져올것임 (Vsync 비율이 60Hz라면, Vsync 아닐 경우에 최대 뭐 120Hz 찍는다던가.)
	//그러나, 이는 당연히 모니터의 주사율(60Hz)과 다르기 때문에 화면 일부에 결점이 일어날 수 있음.

	//vsync 활성화가 되어있으면
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		// 0/1은 0이므로 가능한 무한대로 간다.
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	//백버퍼의 용도를 설정함
	//렌더 출력용으로 설정
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	//렌더링이 이루어질 윈도우의 핸들을 설정함.
	//앞에서 만든 윈도우의 핸들(hwnd) 넘김.
	swapChainDesc.OutputWindow = hwnd;

	//멀티샘플링을 끔
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	//윈도우 모드 또는 풀스크린 모드를 설정함.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	//스캔라인의 정렬과 스캔라이닝을 지정되지 않음으로(unspecified) 설정함
	//스캔라인이 뭐냐면..
	//옛날 TV나 모니터는 화면 출력이 가로로 되어있었음.
	//그렇게 픽셀과 해상도가 정교하지 않았던 TV와 모니터에
	//크게 녹화한 영상이나 반대로 해상도가 맞지 않는 영상이 송출될 경우
	//출력이 가로로 되어있었기 때문에 (스캔라인이었기 때문에)
	//가로로 픽셀이 층져있는것처럼 되어있었음.
	//더 자세한건 검색해보고, 여튼 필요없는 기능 OFF
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//출력된 이후 백버퍼 내용을 버리는 옵션.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//추가 옵션 플래그를 사용하지 않음.
	//추가 옵션이 뭐뭐 있는지는 모르겠지만.. 일단은 안하는것으로.
	swapChainDesc.Flags = 0;


	//다음으로, 스왑 체인 description 구조체 설정이 끝났으면, 피쳐 레벨을 지정해야 함.
	//그냥 몇 버전의 Direct3D 를 사용하겠냐는 것임.
	//사양이 낮은 컴퓨터.. 지원하려면 DirectX 10 또는 9로 설정해야 함.
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	//이제 만들어온 스왑체인 Desc를 기준으로 장치와 장치 컨텍스트, 스왑체인 객체를 생성함
	//뭐... 대충 정보 넣고 포인터 변수들 참조줘서 여기다가 계산해서 넣어주세요 하는것임.
	//참고로 D3D_DRIVER_TYPE_HARDWARE 대신에 D3D_DRIVER_TYPE_REFERENCE 를 넣으면,
	//GPU 대신에 CPU를 사용하여 렌더를 진행하게 되므로 (스왑체인 버퍼 계산을 CPU가 하도록 할당한다..)
	//만약 그래픽카드가 없거나 그런 환경에서라면 이거랑 다이렉트 버전 조정을 해보기를 권장함.
	//아래 에러처리에 else 넣어서 cpu로 바꿔도 되고 뭐..
	//+그래픽 장치를 지정하지 않고 NULL을 넣는다면?
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc,
		&m_swapChain, &m_device, NULL, &m_deviceContext);
	if (FAILED(result))
	{
		return false;
	}

	// 백버퍼에 포인터를 할당하기
	// 앞에 NULL은 뭔지 모르겠다..
	// ID3D11Texture2D를 그리기 위한 백버퍼를
	// backBufferPtr가 가리키도록 한다
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

	// 백버퍼 포인터로 렌더타겟 뷰를 생성하도록 한다.
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	/********************************************/
	//렌더타겟이란 정론으로는 표시될 다음 프레임이 그려지는 메모리 영역이다.
	//때문에 당연히 백 버퍼로 가리켜 사용하겠지..?
	//기본적으로 컴퓨터 메모리라고 하나, PC 메모리만 사용하면 턱없이 부족할 것이기에 (전문화되어있지도 않을 뿐더러)
	//굉장히 많이, 그리고 자주 렌더되는 대상은 그래픽카드(GPU)의 전용 메모리 영역(VRAM)에 넣어놓고 연산한다.
	/********************************************/

	// 이제 백 버퍼가 렌더타겟뷰에 연결되어있으므로 필요없는 포인터 해제
	backBufferPtr->Release();
	backBufferPtr = 0;

	// 백 버퍼가 뷰 렌더를 실행하도록 설정되어있으므로,
	// 이제는 3D 공간을 올바르게 그려 줄 Depth Buffer (깊이 버퍼)를 이용하여 3D 공간 렌더를 설정한다.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// 깊이 버퍼 Description 작성
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	//Mip Map, 밉 맵이라고 검색하면 더 많은 정보가 나올 것이다.
	//밉 레벨이란, 이 밉 맵의 디테일한 레벨을 설정하는 것인데,
	//밉 맵이란 오브젝트가 사용하는 텍스처를 조금 덜 상세하게 렌더하기 위해 사용된다.
	//원본 이미지(고해상도)를 레벨 0으로, 그보다 낮은 품질을 1, 그보다 더 낮은 품질은 2 등으로,
	//품질이 낮을수록 레벨은 높게 잡으면 된다.
	//원본이 2048*2048 이라면 level 0 이고
	//level 1 일때 1024*1024 이런식으로 돌아간다.
	//이걸 안하면 화면이나 3D 애니메이션에서 나타나는 "모아레 패턴" 을 줄이는데 도움이 된다.
	//앞쪽의 픽셀보다 오히려 뒤에 있어서 잘 안보이는 
	depthBufferDesc.MipLevels = 1;
	//말 그대로 버퍼 배열 사이즈
	depthBufferDesc.ArraySize = 1;
	//깊이 버퍼 포맷, 아까 위에서 보통 깊이 버퍼와 스텐실 버퍼(오브젝트 이외의 여러가지 효과를 그리는 버퍼)
	//를 같이 취급한다고 하였다. 떄문에 앞에는 깊이 버퍼 24bit, 뒤에는 스텐실 버퍼 8bit 로 총 32bit 구성하는 포맷이다.
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//멀티 샘플링 수
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	//이건 뭔지 모르겠다.. 디폴트 사용법?
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	//깊이 버퍼를 깊이/스텐실 버퍼로써 사용하겠다는 뜻
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//CPU로부터는 엑세스 하지 않겠다는 뜻. UINT 이지만 아마 0또는 그외 일거고.. 
	depthBufferDesc.CPUAccessFlags = 0;
	//그 외의 설정 없다는 뜻
	depthBufferDesc.MiscFlags = 0;


	//이상, Description 끝났고 이를 이용해서 깊이 버퍼의 텍스처를 생성
	//가운데는 SubResources 던데 뭐인지는 잘 모르겠음..
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

	//이제 깊이 버퍼 설정은 끝났는데
	//우린 깊이/스텐실 버퍼를 사용할거니까
	//스텐실 버퍼 설정 또해줘야된다..........
	//DESC 초기화
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	//뎁스를 사용할 것이므로 true
	depthStencilDesc.DepthEnable = true;
	//직역하자면.. 뎁스 버퍼를 사용할 때 Write Mask 를 사용할거냐 말거냐를 판단하는것같은데
	//Write Mask 란 컬러/뎁스/스텐실 요소들을 현재 프레임 버퍼에 써버리는걸 허용할지 말지에 대한 렌더 파이프라인이라고 함
	//뭐 현재 프레임에 덧씌울거냐인데... MASK 라는건 보통 그 부분을 사용하지 않겠다는 뜻(가린다는 뜻) 이므로....
	//모르겠땅
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	//위에서 설명한 깊이 비교를 어떻게 할 것인지에 대한 설정이다.
	//Comparison Less 의 경우 기존의 Z 보다 현재의 Z가 더 작을때
	//즉 현재의 Z가 더 앞에 있을때만 Z값을 갱신한다.
	//쉽게 말하면 같은 깊이 10에 있는 빨간 삼각형과 초록 삼각형이 있다고 할 때
	//빨간색->초록색 순서로 겹쳐서 그리면
	//빨간색의 깊이 10을 갱신하고, 초록색의 깊이 10을 다시 갱신하려고 보니 기존의 Z랑 같으므로 갱신하지 않는다.
	//이렇게 되면 빨간색 삼각형은 온전하게 보이지만 초록색 삼각형은 빨간색에 가려져 보일 것이다.
	//즉, 동일 깊이에 있을 때 오는 순서대로 그리겠다는 뜻이다.
	//나중에 그리는게 더 앞에 왔으면 좋겠다 싶을 때는
	//D3D11_COMPARISON_LESS_EQUAL 을 사용한다. (같거나 적을 떄 갱신하므로 같을때도 갱신하게 되어 뒤에 온 놈이 갱신되고 그려지게 된다)
	//이 경우에는 빨간색->초록색 순서로 그렸을 때 초록색이 보이고 빨간색이 가려질 것이다./
	//더 많은 정보는 Windows App Development > Windows/Apps/Win32/API/Direct3D11Graphics/D3d11.h / D3D11_COMPARISON_FUNC enumeration 를 참고하자
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	//스텐실 Read Mask 값을 0xFF 로 설정 (아래도 같음)
	//보통 이 Mask 값은 스텐실 기준값(Stencil Reference Value)과 논리 AND 연산하여
	//다시 들어오는 픽셀 값과 (스텐실 버퍼 값이라고 한다면) Mask 값을 논리 AND 연산하고
	//그 두개의 결과를 비교하여 버퍼에 기록할지 말지를 정한다
	//결과가 참일 경우 기록, 거짓일 경우 기록하지 않음
	//즉, 자세히는 모르겠지만 스텐실 값을 백 버퍼에 기록할지 말지를 정하는 비교값이라고 판단된다..
	//그 비교 연산 자체가 어떻게 되는지 알고싶으면 검색하자..

	//여기서는 0xff(11111111) 줬으므로 어떤 값이랑 AND 해도 자기 자신이 나온다.
	//따라서 어떤 비트도 막지 않겠다는 뜻.
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	//FrontFace -> 픽셀의 앞면 방향일때 스텐실 판정을 어떻게 할 것인지에 대한 설정들
	// 전면 삼각형에 대한 스텐실 버퍼 적용 방식
	// 
	//픽셀 단편이 스텐실 판정에 실패했을때 스텐실 버퍼를 갱신하는 방식을 결정하는 필드임
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//픽셀 단편이 스텐실 판정을 통과했지만 깊이 판정에는 실패했을 때 스텐실 버퍼를 갱신하는 방식
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	//픽셀 단편이 스텐실 판정, 깊이 판정을 다 성공했을때 어떻게 할건지
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//스텐실 판정 비교 함수를 결정 (D3D11_COMPARISION_FUNC 열거형 멤버를 보면 되고, Always 이므로 항상 참을 돌려준다)
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//BackFace -> 픽셀의 뒷면 방향일때 스텐실 판정을 어떻게 할 것인지에 대한 설정들
	// 후면 삼각형에 대한 스텐실 버퍼 적용 방식
	// 
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//**** 이 앞면 뒷면은 무엇이냐하면 ****//
	//기본적으로 모든 폴리곤들은 삼각형으로 표현되는데
	//이 폴리곤이 형성될 때 삼각형이 감기는 벡터 방향으로 앞면 뒷면이 결정되며 이를 Normal (삼각형이 갖는 방향) 이라고 한다.
	//게임 등을 하다보면 Back Face Culling 이라는 것을 볼 수 있는데
	//여기서 말하는 컬링이라는것은 결국 보이지 않는 면을 제외시키는 행위를 말한다
	//결국 Backface Culling 이라는 것은 도형에 앞면,뒷면이 존재한다면 그 두 면 중에서 더 시야에 가까운 놈만 남기고 나머지는 제외시킨다는 의미이다.

	//이제 만들어진 정보로 뎁스 스텐실 상태 변수를 만듬
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	//만든 상태 변수를 적용하도록 설정함
	//여기서는 장치 컨텍스트 사용한다..
	//여기서 계속 사용하는 OM이란 Output Merger 스테이지라는 것임
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);


	//이젠 깊이 스텐실 버퍼 뷰에 대한 Description 작성
	//뷰가 있어야 Direct3D가 깊이 버퍼를 깊이-스텐실 텍스쳐로 인식함
	//당연하게도 Depth View 밖에 없는데 Depth-Stencil Buffer 를 갖다 붙여봤자 의미가 없을것임
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	//아까랑 같은 포맷으로 뎁스/스텐실 뷰를 만들어 줌.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//뎁스/스텐실 뷰에 어떤 소스를 사용할 것인지 -> 2D 텍스처를 갖다붙일것이다.
	//나머지 열거형은 D3D11_DSV_DIMENSION enumeration 을 확인할 것
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//밉 슬라이스란.. 밉맵의 경우 렌더 속도 개선을 위해 텍스처 화질을 조절하는 역할을 한다고 했는데,
	//밉 슬라이스의 경우 밉맵 혹은 렌더 품질 등으로 인해 계단현상 등의 회질저하가 생겼을 경우
	//이 수치를 높여줄수록 품질이 올라간다는데.. 정확히 어떤 원리인지는 모르겠다.
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	//뎁스/스텐실 버퍼를 사용해서 뎁스/스텐실 뷰 DESC의 설정으로 뎁스 스텐실 뷰를 만듦
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	//렌더타겟 뷰와 뎁스/스텐실 버퍼를 각각 출력 파이프라인에 바인딩함
	//이렇게 해줌으로써 위 파이프라인을 이용한 렌더링이 수행될 때 백 버퍼에 장면이 그려지게 됨
	//렌더타겟을 우리가 만들었던 렌더타겟뷰와 뎁스 스텐실 뷰를 사용하겠다는 의미
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	//우리가 논리적으로 제시한 그림을 픽셀화시키는것을 "래스터화(Rasterize)" 한다고 한다.
	//예를 들어 사각형을 점과 선으로 만들고 그 안을 흰색으로 칠했다고 생각해 보자.
	//이는 단순히 우리의 상상으로만 존재할 뿐, 현실로 구현하려면 직접 도화지에 대고 그려야 한다
	//이를 화면이라고 생각하면 픽셀 한칸 한칸을 우리가 그린 사각형의 형태에 맞게 채워넣어야 한다는 것이다.

	//아래는 바로 이것, 래스터화에 대한 Description 을 작성하는 것이다.
	//안티 앨리어싱이라는것은 픽셀들의 품질을 조금 더 높여준다는 것이다.
	//예를 들면 검은 사선을 그린다고 생각해 보자. 안티 앨리어싱이 없다면
	//30도 각도로 그렸을 때 생각보다 선이 이상할 것이다. 당연하게도 픽셀은 네모 모양으로 생겼는데
	//이 픽셀은 네모 한칸 전체를 칠하는 행위만 있을 뿐, 절반만 칠한다거나 일정 각도만큼 칠하는 기능은 없다.
	//당연히 그렇게 하도록 만들 수도 없다. (당신은 세포 하나를 반반씩 컨트롤할 수 있는가?)
	//때문에 이 안티 앨리어싱이라는 연산을 거치면 (단순히 명명한것 뿐)
	//우리가 맨 눈으로는 볼 수 없는 인접 픽셀들에 이 대각선을 완벽하게 보이도록 만들게 명령하는것이다.
	//당연히 픽셀 한놈 한놈만 계산하는것보다 (안티 앨리어싱 0단계)
	//그 픽셀의 넓은 주변까지도 같이 연산하는것이 (안티 앨리어싱 1단계)
	//훨씬 자원도 많이 소모하고 시간도 오래 걸릴 것이다.
	rasterDesc.AntialiasedLineEnable = false;
	//BackFace Culling 이랑 동일한 옵션이다. -> D3D11_CULL_BACK
	//D3D11_CULL_MODE 열거형을 보면 되며
	//원하는 효과를 얻기 위해 하나의 기하구조를 CullMode 를 달리 두어 여러번 렌더링하는 알고리즘도 많이 있다.
	//
	//********* Clipping <-> Culling 차이 ***********
	// ******** Clipping : 그린 후에(연산 후에) 화면에 보이지 않도록 잘라냄
	// ******** Culling : 그리기 전에(연산 전에) 판단하여 데이터를 잘라냄
	//
	rasterDesc.CullMode = D3D11_CULL_BACK;
	//동일 평면상의 폴리곤들이지만 각각에 z-bias를 추가함으로써 두 폴리곤이 동일 평면상에 존재하지 않은 것처럼 보이게 할 수 있다.
	//보통 씬에서 그림자가 적절하게 디스플레이되도록 보장하기 위해 사용되는 기법이라고 한다.
	//예를 들면 무언가의 물체가 빛에 비춰져서 벽에 그림자를 만들었다고 생각해 보자.
	//지금 시점에서 벽과 그림자는 완벽히 동일한 깊이를 갖고 있을 것이다.
	//그런데 위에서 한 걸 보면, 우린 스텐실 판단을 결정하면서 동일 깊이를 가지면 Comparison Less 를 설정함으로써
	//먼저오는놈만 버퍼에 넣고 나머지는 버린다고 결정하였다.
	//이 경우 벽이 먼저 올지, 그림자가 먼저 올지 알 수는 없지만 어쨌든 둘 중에서 먼저 오는 놈만 렌더되고 나머지는 버려져
	//결국 벽이든 그림자든 둘 중 하나만 우리 눈에 보이리라는 것을 예상할 수 있다.
	//그럼 이상하지 않은가? 그림자를 못 비추는 벽이라니.
	//때문에 뎁스 말고도 z-bias (Depth bias) 값을 추가하여 동일한 평면상의 폴리곤과 함께 렌더링 되었을 때
	//잘 보일 수 있도록 확률을 높여준다.

	//뭐.. 하여튼 그런 연산에 사용되는 z-bias 값을 얼마로 할 것인지 설정.
	rasterDesc.DepthBias = 0;
	//이건 깊이 바이어스 클램핑에 사용될 값이다
	rasterDesc.DepthBiasClamp = 0.0f;
	//z-bias 클리핑을 사용할지 안할지..
	//클립이라는건 결국 잘라낸다는건데.. 
	//아마 위의 Depth Bias Clamp 값이 음수면 잘라버리겠다는 의미같음.
	//클램프라는건 뭐냐면..
	//우선 Scene Depth 와 Pixel Depth 를 알아야 한다.
	//씬뎁스는 현재 그려진 씬에서 화면 전체의 깊이를 수치로 표현한 것이고 (픽셀마다)
	//픽셀뎁스는 현재 씬을 비추는 카메라와의 상대 깊이를 수치로 표현한 것이다.
	//만약 이 둘을 가지고 뺄셈을 해버리면 해당 오브젝트와 씬 전체가 얼마나 떨어져 있는지 알려준다.
	//이건 일반적으로 반투명한 파티클(Opacity Particle)등의 오브젝트가 그 뒤를 반투명하게 비추어야 할 때 사용되는 연산인데
	//Fog 효과를 넣는다고 하면, 이 Fog를 이루는 입자들이 전역에 균일하게 퍼져있지는 않을 것이다. 구름도 마찬가지로
	//보통 중간이 조금 더 안개가 많고 바깥으로 갈수록 연해지는 형상을 띌 것인데
	//이때 그 안개를 통해 뒤쪽을 바라볼 수 있을 것이다.
	//그러면 그 특정한 안개 입자와 그 뒤에 비춰져야 할 오브젝트의 거리만큼 더 잘 보이거나 흐릿하게 보이거나 할 것이다.
	//그 거리를 계산하기 위해서 씬 자체의 뎁스와 카메라로부터의 뎁스를 빼서 남은 오브젝트와 비춰야 할 대상의 거리를 갖고오는 것이다.
	//그렇게 해서 Opacity 를 계산하는데 위의 거리를 0~1로 표준화시켜서 각각의 Opacity를 계산해 주어야 한다.
	//그렇게 픽셀 값을 정해주는것을 클램프 시켜준다고 한다.
	//말 그대로 물감에 흰색 물감을 섞는? 정도의 수학적 행위라고 보면 된다. (모든 그래픽스 연산은 다 매우 수학적이므로)
	rasterDesc.DepthClipEnable = true;
	//렌더할 때 삼각형(폴리곤)을 어떻게 표시할지 결정한다
	//D3D11_FILL_SOLID -> 폴리곤면을 색칠한다..
	//D3D11_WIREFRAME -> 폴리곤면을 색칠안하고 뼈대만 그린다..
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	//그려지는 순서에 따라 앞면과 뒷면을 정해주는 값
	//CounterClockwise 가 false 이므로 시계방향을 앞면으로 사용한다
	//그러니까, 삼각형의 3 점을 그릴 때 시계방향으로 그리면 그곳이 앞면
	//당연히 반대쪽의 입장에서는 시계반대방향으로 그려질 테니 그곳이 뒷면이 된다는 의미임.
	//Direct X 는 통상적으로 시계방향을 앞면으로 사용한다고 함
	rasterDesc.FrontCounterClockwise = false;
	//근데 결국 안티앨리어싱이나 멀티샘플링이나 똑같음 (안티 앨리어싱은 멀티샘플링 기법 중의 하나이다)
	//멀티샘플링 안한다는 의미임
	rasterDesc.MultisampleEnable = false;
	//시저 테스트 하지 않음
	//시저 테스트란 Scissor (가위) 를 설정하여 특정한 부분을 잘라내는 것을 의미한다.
	//예를 들면 화면 바깥으로 나간 픽셀들을 잘라버린다거나 하는 행위임.
	//당연하게도, 뷰포트 밖의 영역은 자를 수 없다
	rasterDesc.ScissorEnable = false;
	//섀도우 맵(그림자)을 더욱 향상시키기 위한 뎁스 바이어스 값
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	//작성한 Description 으로부터 레스터화 상태를 생성함
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		return false;
	}

	//렌더타겟 공산에서 클리핑을 수행하기 위해 뷰포트를 또 만들어줘야 함
	//뷰포트 설정
	//순서대로 화면크기, 화면 깊이, 화면 최상단의 좌표
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	//뷰포트 한개 생성
	m_deviceContext->RSSetViewports(1, &viewport);

	//Projection Matrix 를 생성한다.
	//3D 화면을 2D 뷰포트 공간으로 변환하여 셰이더에서 장면을 그릴 때 사용하도록 설정함

	//이게뭐냐면 일단 처음부터 말했듯이 뷰포트는 화면 자체가 3D가 아닌 이상은 2D로 표현이 되어야 한다고 말했다.
	//다만 소스는 처음부터 2D가 아니라 (뭐 예를들어 블렌더 등에서 작성한다면) 3D일 것이다
	//즉 3차원 -> 2차원 치환을 해야하는데 n -> n-1 차원을 얻는 과정을 투영(Projection) 이라고 한다.
	//그 중에서 원근투영법이라는 방법이 존재하는데 그게 바로 아래의 D3DXMatrixPerspectiveFovLH

	//투영 행렬 설정 (Projection Matrix)
	fieldOfView = (float)D3DX_PI / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;



	//아래의 매트릭스들은 다 ForLH 인데 이는 왼손 좌표계 Perspective를 사용한다는 의미이다.
	//Tekla를 생각해보면 처음 시작하면 다 왼손 좌표계가 표준이다.
	//모르겠으면 플레밍의 왼손법칙 손모양 해보자 엄지가 위를 향할 때 엄지->검지->중지 순으로 Z,Y,X 축이다. 이게 왼손 좌표계.


	//3D 렌더링을 위한 투영 행렬 생성
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);

	//이제는 다시.. 오브젝트들의 좌표를 3D로 변환하기 위한 월드 행렬을 만들어야 함.
	//이는 3D 좌표 뿐만 아니라 3차원 공간에서의 회전/이동/크기 변환에도 사용됨.
	D3DXMatrixIdentity(&m_worldMatrix);

	//그 다음엔 이제 위의 매트릭스 정보들을 기반으로 만들 뷰 행렬을 생성하는 곳이다.
	//현재 장면에서 우리가 어느 위치에서, 어느 방향을 보고있는지에 대한 정보가 담기며
	//카메라랑 그냥 똑같다고 보면 된다.
	//나중에 이제 카메라 클래스에서 더 코딩할것이므로 여기서는 하지 않는다.

	//마지막으로 직교 투영 행렬을 만들어야 한다.
	//이건 3D 객체들을 위한 매트릭스가 아니라, 2D UI 등을 위한 매트릭스이고
	//나중에 2D 그래픽, 폰트를 다루는 예제에서 보게 될 것임
	D3DXMatrixOrthoLH(&m_orthoMatrix, (float)screenWidth, (float)screenHeight, screenNear, screenDepth);



	//예제용 그래픽카드 이름 메모리 출력
	//파일열고
	FILE* fp = fopen(filepath, "w+");
	//파일에 쓰기위한 fout 스트림 버퍼
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

	//파일 닫기
	fclose(fp);

	return true;
}

//종료할 때 할당된 모든 포인터들을 해제하고 정리해야 함
//스왑 체인의 경우 풀스크린 모드에서 해제해버리면 오류가 많이 생긴다고 한다.. (아마 풀스크린의 경우 자체 해상도와 윈도우 해상도가 다르기 때문에 오류가 생기는게 아닐까 함)
//때문에 반드시 윈도우 모드로 바꾼 후 스왑 체인을 해제하여야 함
void D3DClass::Shutdown()
{
	if (m_swapChain)
	{
		//윈도우 모드로 셋하고 NULL
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

//D3DClass에 존재하는 도우미 함수들
//BeginScene : 매 프레임의 시작마다 3D 화면을 그리기 시작할 때 호출됨.
//버퍼를 빈 값으로 초기화하고 렌더링이 잘 이루어지도록 준비함.
//EndScene : 매 프레임의 마지막 프레임에 호출되어, 백<->프론트 스왑을 진행하고 스왑 체인에게 3D화면을 표시하게 한다.
//Device, Context Getter
//Matrixies Getter
//그래픽카드 정보 반환
void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];

	//RGBA를 사용해서 지움
	//포맷이 달라지면 이것도 달라져야 할것임
	//버퍼를 어떤 색으로 초기화 할 지 RGBA 값을 입력
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	//백버퍼 내용을 위에서 정한 칼라로 초기화 (RGBA)
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	//깊이 버퍼를 초기화
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}

void D3DClass::EndScene()
{
	// 렌더링이 완료되었으므로 백버퍼의 내용을 화면에 표시합니다.
	//Vsync 가 활성화되어있으면 스왑체인에게 새로고침 비율을 고정하도록 지시
	//Vsync 란 모니터 주사율과 맞추는 것이므로 이미 Initialize 에서 설정이 되어있다. (모니터 주사율 등)
	if (m_vsync_enabled)
	{
		//present : 준비된 화면을 선보이기
		m_swapChain->Present(1, 0);
	}
	//그게 아니면 가능한 한 빠르게 표시 (0,0)
	else
	{
		m_swapChain->Present(0, 0);
	}

	return;
}

//디바이스, 컨텍스트 Getter
ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}

ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}

//다시, 투영, 월드, 직교 행렬 Getter (Reference)
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

//그래픽카드 정보 반환
void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}