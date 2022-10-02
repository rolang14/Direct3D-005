#include "SystemClass.h"

//::는 범위 연산자, SystemClass 안에 있는 SystemClass() <여기서는 생성자 라고 봄> 을 사용하겠다.
//다른 Class 내에 같은 이름의 메소드가 존재할 수 있으므로.
SystemClass::SystemClass()
{
	//SystemClass (헤더)
	//모든 포인터와 변수들을 반드시 null로 초기화 할 것
	//null이 아니라 다른것으로 초기화가 되어버리면, 뭔가가 메모리 내에 들어있는것으로 판단하고
	//올바르게 할당하기 위해 메모리 정리를 수행하기 때문에 릴리즈 시 빌드 오류가 생길 수 있다.
	m_input = 0;
	m_Graphics = 0;
}

//클래스 복사 생성자, 파괴자(소멸자)
//일부러 아무것도 하지 않으며, 여기서는 소멸자에 에러가 있다고 작성자가 판단하였으므로,
//소멸자에서 삭제를 진행하지 않고 Shutdown 함수에서 별도로 진행할 것임.
SystemClass::SystemClass(const SystemClass& other)
{
}

SystemClass::~SystemClass()
{
}

//Initialize 구현
//어플리케이션의 초기화
bool SystemClass::Initialize()
{
	//화면 해상도, 결과값
	int screenWidth, screenHeight;
	bool result;

	//화면 해상도 일단 초기화
	screenWidth = 0;
	screenHeight = 0;

	//Windows API 사용하여 이 윈도우 초기화
	InitializeWindows(screenWidth, screenHeight);

	//Input 객체 생성. 유저의 키보드 입력에 대한 처리
	m_input = new InputClass;
	//오류처리.
	if (!m_input)
	{
		return false;
	}

	//m_input 내부의 Initialize 메소드를 실행한다는 것. (InputClass에 정의가 되어있을 것임.)
	//input 객체 초기화
	m_input->Initialize();

	//마찬가지로, graphics 객체 생성 및 초기화.
	//이 어플리케이션의 모든 그래픽 요소를 처리하는 일을 할 것임
	m_Graphics = new GraphicsClass;
	if (!m_Graphics)
	{
		return false;
	}

	//result 로 결과 받아오기
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);

	if (!result)
	{
		return false;
	}

	return true;
}

//메모리 정리 및 종료.
void SystemClass::Shutdown()
{
	//Graphics 객체가 존재하면
	if (m_Graphics)
	{
		//Graphics 의 Shutdown 을 실행함 (이런 함수가 Graphics 내에 정의되어있을 것임.)
		m_Graphics->Shutdown();
		//정리되었으니 삭제함
		//C#에는 동적할당의 개념이 거의 없고 자유분방하지만
		//C/CPP에서는 동적할당된(malloc 혹은 new) 데이터를 꾸준히 지워줘야 메모리 누수가 발생하지 않음.
		delete m_Graphics;
		m_Graphics = 0;
	}

	//Input 은 단순히 클래스 하나이므로 삭제해주면 된다.
	if (m_input)
	{
		delete m_input;
		m_input = 0;
	}

	ShutdownWindows();

	return;
}

//프로그램이 종료될때까지 계속 루프를 돌면서 어플리케이션의 모든 작업을 처리함.
//while 종료되지 않은 동안
//		윈도우의 시스템 메세지를 확인
//		메세지 처리
//		어플리케이션의 작업
//		유저가 작업중 프로그램의 종료를 원하는지 확인
void SystemClass::Run()
{
	//MessageHandler 에서 핸들링하는 메세지 (전역으로 전달하여 반영되게 하는 메세지.)
	//MSG 구조체(winuser에 정의되어있든지 할거임) 로 생성.
	MSG msg;
	bool done, result;

	//메세지 구조체 초기화
	//구조체를 초기화 하는 방법 3가지
	//1. memset()
	//2. ZeroMemory()
	//3. Struct something = {0} (널 초기화)
	//일단, 1번 2번은 어셈블리어 상으로 완전히 똑같은 루틴이므로 동일하다.
	//3번의 경우, 해당 구조체 인자 크기만큼 16bit 메모리라면 그 단위만큼 이동해서 각각 NULL을 채워주는 루틴.

	//큰 구조체의 경우 약간 다른데
	//memset -> 메모리를 인자(char 변수)로 채워버림
	//ZeroMemory -> 메모리를 0으로 채워버림
	//말 그대로, memset 은 단순히 null 넣는것 이외에도 원하는 값으로 "초기화"가 가능하다는 것.

	//msg를, msg 크기만큼 메모리 영역을 제로로 만들어 준다.
	//메세지를 받기 전, 초기화 시켜주는 의미
	ZeroMemory(&msg, sizeof(MSG));

	//유저가 종료 메세지를 주면 true로 바꾸고 루프 종료
	done = false;
	while (!done)
	{
		//윈도우 메세지를 처리합니다.
		//게임을 위한 메세지 핸들러이고, 메세지 큐에서 메세지를 받아와
		//메세지를 처리하고 큐에서 제거하는 루틴.
		//다만, 별도 처리하지 않으면 한번 꺼내고 메세지가 없으면 죽어버리기에 (그렇다고 유저가 ms동안 메세지를 마구마구 보내지는 않을테니)
		//false 일 경우 Render를 한번 해준다거나 하는, 어플리케이션이 지속되게 만들어야 함
		//Non-Zero 리턴 : 성공, Zero 리턴 : 실패
		//&msg : 메세지
		//NULL : 여기엔 원래 핸들이 들어가야 하나, NULL을 넣으면 현재의 thread를 호출한 윈도우와 관련된 메세지를 찾아봄.
		//0, 0 : 필터인데, 따로 넣지 않으면 (모두가 0이면) 모든 메세지를 확인함
		//보통 앞의 wMsgFilterMin ~ 뒤의 wMsgFilterMax 사이의 메세지를 봄. (winuser.h 참조)

		//메세지를 성공적으로 가져오면,
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			//키 입력을 문자로 변환
			TranslateMessage(&msg);
			//창 프로시저에 이런 메세지가 왔다고 알려 줌
			DispatchMessage(&msg);

			//기본적으로 루틴은 이렇다
			//마우스 클릭을 했다고 가정할 때
			//운영체제가 WM_LBUTTONDOWN 메세지를 메세지 큐에 넣음 (다른것도 뭐 계속 들어올거임)
			//프로그램이 PeekMessage 혹은 GetMessage 를 호출함
			//메세지 큐에서 가장 앞에있는것 WM_LBUTTONDOWN 메세지를 갖고와 MSG 구조를 채워 줌
			//TranslateMessge 로 MSG 구조 내의 메세지를 해석하고 (문자로 바꾸고)
			//DispatchMessge 내에서 운영 체제가 창 프로시저를 호출하여 메세지 처리
			//모든게 끝나고 결과가 반환되면 다음 코드 진행
		}

		//윈도우에서 종료를 요청하는 경우
		//Message -> WN_QUIT
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		//종료 이외의 모든 커맨드(메세지)는 Frame 함수로 처리
		//나중에 여러 키를 넣고, 많은 커맨드가 생기면
		//당연히 여기의 코드도 확장되어야 함.
		else
		{
			result = Frame();
			if (!result)
			{
				done = true;
			}
		}
	}
}

//Frame 함수 구현
bool SystemClass::Frame()
{
	bool result;

	//유저가 Esc 키를 누르면 어플리케이션 종료
	//Esc 키를 누르는지 확인함
	if (m_input->IsKeyDown(VK_ESCAPE))
	{
		//false 반환 시 Run 루프가 종료되고 더이상 키 입력을 받지 않음.
		return false;
	}

	//Graphics 객체의 작업을 처리함
	result = m_Graphics->Frame();
	if (!result)
	{
		return false;
	}

	return true;
}

//메세지 핸들러 구현
//앞에서부터 핸들, 메세지, Word Parameter : 일반적으로 핸들 or 정수를 받아들이는 경우, Long Parameter : 일반적으로 포인터 값을 받아들이는 경우
//기본적으로, hwnd 로 메세지가 발생한 윈도우의 핸들을 전달해주고, umsg 로 해당 메세지를 확인하며 wParam, lParam 에 들어있는 값(메세지에 따라 다를것임)
//으로 컨트롤해준다.
LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		//키보드 키가 눌리면
	case WM_KEYDOWN:
	{
		//무슨 키가 눌렸는지(wparam) input 객체에게 전달
		m_input->KeyDown((unsigned int)wparam);
		return 0;
	}
	//눌렸던 키가 떼어지면
	case WM_KEYUP:
	{
		m_input->KeyUp((unsigned int)wparam);
		return 0;
	}
	//그 외의 다른 메세지에 대한 기본 동작
	//여기서는 사용하지 않으므로 기본 메세지 처리기에 전달함
	//기본 메세지 처리기는 정말 말 그대로 아무것도 코딩하지 않았을 때
	//윈도우에서 기본적으로 갖고 있는 키 입력에 대한 프로시저 처리를 의미한다.
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}

//윈도우 초기화 함수 정의
//여기서 렌더링 하게 될 윈도우를 만듬.
void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	//윈도우 생긴거에 대한.. (메뉴, 커서, 아이콘 등) 클래스 WNDCLASSEX.
	WNDCLASSEX wc;
	//Devmode 구조체이며, 여러 장치 정보를 기록하기 위해 존재한다고 함...
	//때문에 여기서는 화면에 대한... 윈도우 화면의 해상도 등에 대한 설정을 사용하기 위함.
	DEVMODE dmScreenSettings;
	int posX, posY;

	//외부 포인터를 이 객체로 설정
	ApplicationHandle = this;

	//이 어플리케이션의 인스턴스를 가져옴
	m_hinstance = GetModuleHandle(NULL);

	//어플리케이션의 이름을 설정합니다
	//string 앞에 L이 붙는 것은 Wide Character 임을 선언하기 위해서임.
	//보통 한글과 같이 멀티바이트를 사용하여 표현되는 문자를 사용한다는 의미.

	//멀티바이트란, 일단은 ASCII 에 대한 이해가 필요한데
	//문자를 표시하기 위해 1byte로 구성된 묶음으로 하나의 문자를 의미하는
	//일종의 문자<->데이터 배치집인데,
	//이걸로는(1Byte로는) 한글이나 일어 등의 문자를 표현할 수 없다 (당연히, 알파벳 등 밖에 없으므로)
	//때문에 2Byte 를 사용하여 한글이나 일어 등의 문자도 표현하는데 (아마 어딘가에는 3Byte가 필요한 곳도 있을것임)
	//당연히, RAW Data 가 같더라도 한글 배치표인지, 일어 배치표인지에 따라 표시되는 문자는 달라지고
	//이는 문자 깨짐의 원인이 된다.
	//때문에 그 대안으로 나온것이 Unicode.
	//char -> TCHAR, "" -> TEXT("") 등으로 지정 후에 유니코드로 컴파일 하는 습관을 갖자.
	//이에 대한 자세한 내용은 OneNote 참고
	m_applicationName = L"Engine";

	//윈도우 클래스를 기본 설정으로 맞춥니다.
	//스타일에 들어가는 CS_HREDRAW나 CS_VREDRAW는
	//클라이언트 영역(작업영역, 사용자 영역)의 폭과 높이가 변경되었을 경우 해당 크기를 고려하여 전부 다시 그리도록 해준다.
	//CS_DWNDC 는 DC 캐시에 이 DC를 넣을것이고, 제거하지 말라는 의미를 갖는다.
	//이러한 의미를 부여해주는 이유는 마음대로 DC를 사용할 수 있게 하기 위해서.
	//다만 이러한 DC 할당을 많이 하면 좋지 않다.. (많은 개수에 대한 고려가 되어있지 않기 때문)
	//윈도우가 파괴될 때 자동으로 DC도 파괴되므로 해제할 필요도 없고 해제하여도 안된다.
	//자세한 내용은 Private DC 라고 검색해보자
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	//윈도우의 메세지 처리를 위해 콜백 함수를 등록함.
	//메세지를 처리할 때마다 WndProc에 등록된 함수를 사용한다는 의미임.
	wc.lpfnWndProc = WndProc;

	//아래 두개는, 이 윈도우, 즉 WNDCLASSEX 를 등록할 때 여분의 메모리 공간을 더 할당해 주기 위한 값임.
	//보통 다 0을 준다.
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	wc.hInstance = m_hinstance;
	//아이콘 설정
	//핸들에 NULL을 넣음으로써 윈도우에서 기본으로 제공하는 ICON을 사용할 수 있고,
	//그 ICON 중에서 WINLOGO 라는 아이콘을 사용하겠다는 것
	//미리 지정한 아이콘을 사용하고싶다면 인스턴스를 넘기고 (NULL이 아닌), 여기서는 m_hinstance 가 될것.
	//resource.h에 정의된 ID를 넘기면 된다.
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	//뭔지 모르겠는데.. 그냥 똑같은 아이콘 물려주자
	//아마 뭐 최소화되었거나 트레이아이콘을 의미하는게 아닐까 싶은데..
	//16x16의 작은 아이콘 핸들이라고 함.
	wc.hIconSm = wc.hIcon;
	//위와 마찬가지일거임. 커서 모양을 의미함
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//기본 배경 색상
	//GetStockObject 를 사용하면 윈도우에서 제공하는 기본 윈도우 색상을 사용할 수 있음.
	//WHITE_BRUSH : 흰색 화면, 반대는 검은색 화면
	//다만, 메소드 사용하여 반환할 경우 의도적으로 형 변환(HBRUSH) 해주어야 한다.
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	//메뉴 리소스인데...
	//여기서는 일단 사용하지 않으니 나중에 검색해볼것.
	wc.lpszMenuName = NULL;
	//이 윈도우의 클래스의 id값을 지정 (마찬가지로 L""으로 지정해줘야 함)
	//여기서는 applicationName 따라감.
	wc.lpszClassName = m_applicationName;
	//윈도우 클래스 구조체의 크기를 저장함.
	//WNDCLASSEX 크기를 그냥 지정해주자.
	wc.cbSize = sizeof(WNDCLASSEX);

	//윈도우 클래스를 레지스터에 등록함.
	//당연하게도 등록해야 쓸 수 있겠지..
	RegisterClassEx(&wc);

	//현재 모니터 화면의 해상도를 알아옴
	//C"X"SCREEN -> Width
	//C"Y"SCREEN -> Height
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	//풀스크린 모드에 따라 화면 설정 진행
	//FULL_SCREEN 설정값은 그래픽적인 요소이므로, graphicsclass.h 에 선언되어있음.
	if (FULL_SCREEN)
	{
		//풀스크린 모드의 경우 화면 크기를 데스크톱 크기에 맞추고 색상을 32bit로 함.

		//일단은 DEVMODE 구조체 초기화
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);

		//화면크기
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;

		//색상 비트 수
		//아래건 뭔지 모르겠다.. 아무래도 dmField 를 인자들로 설정하는 느낌이다.
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		//위의 설정으로 풀스크린에 맞는 디스플레이 설정을 진행
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		//그리고 창이 항상 왼쪽 위로 가게
		posX = posY = 0;
	}
	//풀스크린이 아닌 윈도우 모드의 경우
	else
	{
		//창모드 시 800x600 으로 해상도 설정
		screenWidth = 800;
		screenHeight = 600;

		//이때는 창이 모니터의 중앙에 와야 함.
		//모니터가 1920 짜리라면, 1920 - 800 = 1120 (윈도우 빼고 남은 여백) / 2 = 560 (의 절반) 에 와야 함.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	//설정이 끝났으니 윈도우를 만들고, 그 핸들을 가져옴
	//자.. 순서대로
	//CreateWindow 와 CreateWindowsEX 로 창을 만들 수 있는데, 보통 이런애들은 뭔가 인자에 한두개 차이나는데 오버로딩 안한 함수이다.
	//여기서는 맨 앞에 인자가 추가되었고,
	// WS_EX_APPWINDOW : 윈도우가 보일 때 강제로 태스크 바 위에 있도록 함
	// 뭐 이런 옵션들임. 자세한건 찾아보면 된다.
	// 그다음 className, applicationName 는 여기선 같으니 같게 써주고,
	// WS_CLIPSIBLINGS : 자식들끼리 겹친 영역은 렌더하지 않겠다 (드로잉하지 않겠다.)
	// WS_CLIPSIBLINGS : 자식이 있는 영역은 렌더하지 않겠다. (드로잉하지 않겠다.)
	// WS_POPUP : 윈도우를 팝업 윈도우로 만들겠다.
	// 그 다음은 윈도우가 자리할 위치 posX, posY
	// 그 다음 해상도 screenWidth, screenHeight
	// 그 다음은 부모의 핸들, 메뉴의 핸들 (이지만 여기서는 없으므로 NULL)
	// 그 다음은 이 윈도우의 핸들 인스턴스 (앞으로의 제어를 위해 지정된 m_hinstance 를 사용하도록 함)
	// 마지막 lpParam. LPVOID : void 형 포인터.. MDI 클라이언트 윈도우일 경우에 CLIENTCREATESTRUCT 구조체를 지정해줘야 하는데
	// 여기다가 해당 포인터를 넣으면 된다고 함.
	// MDI 클라이언트란... 프레임 윈도우가 있고 그 안에 계속해서 자식 윈도우들이 생성되고 삭제되어야 하는 윈도우임.
	// 자세한건 검색하자.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// 윈도우를 화면에 표시함
	ShowWindow(m_hwnd, SW_SHOW);
	// 윈도우를 맨 앞으로 가져옴
	SetForegroundWindow(m_hwnd);
	// 윈도우에 포커스를 줌
	SetFocus(m_hwnd);

	// 마우스 커서를 표시하지 않음
	ShowCursor(false);

	return;
}

// 화면 설정을 원래로 되돌리고 윈도우와 그 핸들들을 반환함.
void SystemClass::ShutdownWindows()
{
	// 마우스 커서 다시 표시함
	ShowCursor(true);

	// 풀스크린 모드였다면 다시 디스플레이 설정을 바꿔 줌
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// 윈도우를 제거하고 핸들 반납
	DestroyWindow(m_hwnd);
	// 핸들 제거
	// 이렇게 반납된 핸들이라도 NULL 주지 않으면 혹시나 값이 남아있어 다른 프로그램에 지장을 줄 수 있다.
	// 다른것들도 마찬가지.
	m_hwnd = NULL;

	// 어플리케이션 인스턴스를 제거함
	// 이 윈도우 이름을 넣고 제거하자
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// 이 클래스에 대한 외부 포인터 참조를 제거함
	ApplicationHandle = NULL;

	return;
}


//여기서 메세지 처리를 위한 콜백 함수 정의
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// 윈도우가 제거되면
	case WM_DESTROY:
	{
		// 제거되어있으면 종료 메세지를 보내고 콜백도 종료
		PostQuitMessage(0);
		return 0;
	}

	// 윈도우가 닫히면
	case WM_CLOSE:
	{
		// 마찬가지로 종료.
		PostQuitMessage(0);
		return 0;
	}

	// 이외의 메세지들은 메세지 핸들러에게 전달하여 처리
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}