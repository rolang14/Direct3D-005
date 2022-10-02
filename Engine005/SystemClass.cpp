#include "SystemClass.h"

//::�� ���� ������, SystemClass �ȿ� �ִ� SystemClass() <���⼭�� ������ ��� ��> �� ����ϰڴ�.
//�ٸ� Class ���� ���� �̸��� �޼ҵ尡 ������ �� �����Ƿ�.
SystemClass::SystemClass()
{
	//SystemClass (���)
	//��� �����Ϳ� �������� �ݵ�� null�� �ʱ�ȭ �� ��
	//null�� �ƴ϶� �ٸ������� �ʱ�ȭ�� �Ǿ������, ������ �޸� ���� ����ִ°����� �Ǵ��ϰ�
	//�ùٸ��� �Ҵ��ϱ� ���� �޸� ������ �����ϱ� ������ ������ �� ���� ������ ���� �� �ִ�.
	m_input = 0;
	m_Graphics = 0;
}

//Ŭ���� ���� ������, �ı���(�Ҹ���)
//�Ϻη� �ƹ��͵� ���� ������, ���⼭�� �Ҹ��ڿ� ������ �ִٰ� �ۼ��ڰ� �Ǵ��Ͽ����Ƿ�,
//�Ҹ��ڿ��� ������ �������� �ʰ� Shutdown �Լ����� ������ ������ ����.
SystemClass::SystemClass(const SystemClass& other)
{
}

SystemClass::~SystemClass()
{
}

//Initialize ����
//���ø����̼��� �ʱ�ȭ
bool SystemClass::Initialize()
{
	//ȭ�� �ػ�, �����
	int screenWidth, screenHeight;
	bool result;

	//ȭ�� �ػ� �ϴ� �ʱ�ȭ
	screenWidth = 0;
	screenHeight = 0;

	//Windows API ����Ͽ� �� ������ �ʱ�ȭ
	InitializeWindows(screenWidth, screenHeight);

	//Input ��ü ����. ������ Ű���� �Է¿� ���� ó��
	m_input = new InputClass;
	//����ó��.
	if (!m_input)
	{
		return false;
	}

	//m_input ������ Initialize �޼ҵ带 �����Ѵٴ� ��. (InputClass�� ���ǰ� �Ǿ����� ����.)
	//input ��ü �ʱ�ȭ
	m_input->Initialize();

	//����������, graphics ��ü ���� �� �ʱ�ȭ.
	//�� ���ø����̼��� ��� �׷��� ��Ҹ� ó���ϴ� ���� �� ����
	m_Graphics = new GraphicsClass;
	if (!m_Graphics)
	{
		return false;
	}

	//result �� ��� �޾ƿ���
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);

	if (!result)
	{
		return false;
	}

	return true;
}

//�޸� ���� �� ����.
void SystemClass::Shutdown()
{
	//Graphics ��ü�� �����ϸ�
	if (m_Graphics)
	{
		//Graphics �� Shutdown �� ������ (�̷� �Լ��� Graphics ���� ���ǵǾ����� ����.)
		m_Graphics->Shutdown();
		//�����Ǿ����� ������
		//C#���� �����Ҵ��� ������ ���� ���� �����й�������
		//C/CPP������ �����Ҵ��(malloc Ȥ�� new) �����͸� ������ ������� �޸� ������ �߻����� ����.
		delete m_Graphics;
		m_Graphics = 0;
	}

	//Input �� �ܼ��� Ŭ���� �ϳ��̹Ƿ� �������ָ� �ȴ�.
	if (m_input)
	{
		delete m_input;
		m_input = 0;
	}

	ShutdownWindows();

	return;
}

//���α׷��� ����ɶ����� ��� ������ ���鼭 ���ø����̼��� ��� �۾��� ó����.
//while ������� ���� ����
//		�������� �ý��� �޼����� Ȯ��
//		�޼��� ó��
//		���ø����̼��� �۾�
//		������ �۾��� ���α׷��� ���Ḧ ���ϴ��� Ȯ��
void SystemClass::Run()
{
	//MessageHandler ���� �ڵ鸵�ϴ� �޼��� (�������� �����Ͽ� �ݿ��ǰ� �ϴ� �޼���.)
	//MSG ����ü(winuser�� ���ǵǾ��ֵ��� �Ұ���) �� ����.
	MSG msg;
	bool done, result;

	//�޼��� ����ü �ʱ�ȭ
	//����ü�� �ʱ�ȭ �ϴ� ��� 3����
	//1. memset()
	//2. ZeroMemory()
	//3. Struct something = {0} (�� �ʱ�ȭ)
	//�ϴ�, 1�� 2���� ������� ������ ������ �Ȱ��� ��ƾ�̹Ƿ� �����ϴ�.
	//3���� ���, �ش� ����ü ���� ũ�⸸ŭ 16bit �޸𸮶�� �� ������ŭ �̵��ؼ� ���� NULL�� ä���ִ� ��ƾ.

	//ū ����ü�� ��� �ణ �ٸ���
	//memset -> �޸𸮸� ����(char ����)�� ä������
	//ZeroMemory -> �޸𸮸� 0���� ä������
	//�� �״��, memset �� �ܼ��� null �ִ°� �̿ܿ��� ���ϴ� ������ "�ʱ�ȭ"�� �����ϴٴ� ��.

	//msg��, msg ũ�⸸ŭ �޸� ������ ���η� ����� �ش�.
	//�޼����� �ޱ� ��, �ʱ�ȭ �����ִ� �ǹ�
	ZeroMemory(&msg, sizeof(MSG));

	//������ ���� �޼����� �ָ� true�� �ٲٰ� ���� ����
	done = false;
	while (!done)
	{
		//������ �޼����� ó���մϴ�.
		//������ ���� �޼��� �ڵ鷯�̰�, �޼��� ť���� �޼����� �޾ƿ�
		//�޼����� ó���ϰ� ť���� �����ϴ� ��ƾ.
		//�ٸ�, ���� ó������ ������ �ѹ� ������ �޼����� ������ �׾�����⿡ (�׷��ٰ� ������ ms���� �޼����� �������� �������� �����״�)
		//false �� ��� Render�� �ѹ� ���شٰų� �ϴ�, ���ø����̼��� ���ӵǰ� ������ ��
		//Non-Zero ���� : ����, Zero ���� : ����
		//&msg : �޼���
		//NULL : ���⿣ ���� �ڵ��� ���� �ϳ�, NULL�� ������ ������ thread�� ȣ���� ������� ���õ� �޼����� ã�ƺ�.
		//0, 0 : �����ε�, ���� ���� ������ (��ΰ� 0�̸�) ��� �޼����� Ȯ����
		//���� ���� wMsgFilterMin ~ ���� wMsgFilterMax ������ �޼����� ��. (winuser.h ����)

		//�޼����� ���������� ��������,
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			//Ű �Է��� ���ڷ� ��ȯ
			TranslateMessage(&msg);
			//â ���ν����� �̷� �޼����� �Դٰ� �˷� ��
			DispatchMessage(&msg);

			//�⺻������ ��ƾ�� �̷���
			//���콺 Ŭ���� �ߴٰ� ������ ��
			//�ü���� WM_LBUTTONDOWN �޼����� �޼��� ť�� ���� (�ٸ��͵� �� ��� ���ð���)
			//���α׷��� PeekMessage Ȥ�� GetMessage �� ȣ����
			//�޼��� ť���� ���� �տ��ִ°� WM_LBUTTONDOWN �޼����� ����� MSG ������ ä�� ��
			//TranslateMessge �� MSG ���� ���� �޼����� �ؼ��ϰ� (���ڷ� �ٲٰ�)
			//DispatchMessge ������ � ü���� â ���ν����� ȣ���Ͽ� �޼��� ó��
			//���� ������ ����� ��ȯ�Ǹ� ���� �ڵ� ����
		}

		//�����쿡�� ���Ḧ ��û�ϴ� ���
		//Message -> WN_QUIT
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		//���� �̿��� ��� Ŀ�ǵ�(�޼���)�� Frame �Լ��� ó��
		//���߿� ���� Ű�� �ְ�, ���� Ŀ�ǵ尡 �����
		//�翬�� ������ �ڵ嵵 Ȯ��Ǿ�� ��.
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

//Frame �Լ� ����
bool SystemClass::Frame()
{
	bool result;

	//������ Esc Ű�� ������ ���ø����̼� ����
	//Esc Ű�� �������� Ȯ����
	if (m_input->IsKeyDown(VK_ESCAPE))
	{
		//false ��ȯ �� Run ������ ����ǰ� ���̻� Ű �Է��� ���� ����.
		return false;
	}

	//Graphics ��ü�� �۾��� ó����
	result = m_Graphics->Frame();
	if (!result)
	{
		return false;
	}

	return true;
}

//�޼��� �ڵ鷯 ����
//�տ������� �ڵ�, �޼���, Word Parameter : �Ϲ������� �ڵ� or ������ �޾Ƶ��̴� ���, Long Parameter : �Ϲ������� ������ ���� �޾Ƶ��̴� ���
//�⺻������, hwnd �� �޼����� �߻��� �������� �ڵ��� �������ְ�, umsg �� �ش� �޼����� Ȯ���ϸ� wParam, lParam �� ����ִ� ��(�޼����� ���� �ٸ�����)
//���� ��Ʈ�����ش�.
LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		//Ű���� Ű�� ������
	case WM_KEYDOWN:
	{
		//���� Ű�� ���ȴ���(wparam) input ��ü���� ����
		m_input->KeyDown((unsigned int)wparam);
		return 0;
	}
	//���ȴ� Ű�� ��������
	case WM_KEYUP:
	{
		m_input->KeyUp((unsigned int)wparam);
		return 0;
	}
	//�� ���� �ٸ� �޼����� ���� �⺻ ����
	//���⼭�� ������� �����Ƿ� �⺻ �޼��� ó���⿡ ������
	//�⺻ �޼��� ó����� ���� �� �״�� �ƹ��͵� �ڵ����� �ʾ��� ��
	//�����쿡�� �⺻������ ���� �ִ� Ű �Է¿� ���� ���ν��� ó���� �ǹ��Ѵ�.
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}

//������ �ʱ�ȭ �Լ� ����
//���⼭ ������ �ϰ� �� �����츦 ����.
void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	//������ ����ſ� ����.. (�޴�, Ŀ��, ������ ��) Ŭ���� WNDCLASSEX.
	WNDCLASSEX wc;
	//Devmode ����ü�̸�, ���� ��ġ ������ ����ϱ� ���� �����Ѵٰ� ��...
	//������ ���⼭�� ȭ�鿡 ����... ������ ȭ���� �ػ� � ���� ������ ����ϱ� ����.
	DEVMODE dmScreenSettings;
	int posX, posY;

	//�ܺ� �����͸� �� ��ü�� ����
	ApplicationHandle = this;

	//�� ���ø����̼��� �ν��Ͻ��� ������
	m_hinstance = GetModuleHandle(NULL);

	//���ø����̼��� �̸��� �����մϴ�
	//string �տ� L�� �ٴ� ���� Wide Character ���� �����ϱ� ���ؼ���.
	//���� �ѱ۰� ���� ��Ƽ����Ʈ�� ����Ͽ� ǥ���Ǵ� ���ڸ� ����Ѵٴ� �ǹ�.

	//��Ƽ����Ʈ��, �ϴ��� ASCII �� ���� ���ذ� �ʿ��ѵ�
	//���ڸ� ǥ���ϱ� ���� 1byte�� ������ �������� �ϳ��� ���ڸ� �ǹ��ϴ�
	//������ ����<->������ ��ġ���ε�,
	//�̰ɷδ�(1Byte�δ�) �ѱ��̳� �Ͼ� ���� ���ڸ� ǥ���� �� ���� (�翬��, ���ĺ� �� �ۿ� �����Ƿ�)
	//������ 2Byte �� ����Ͽ� �ѱ��̳� �Ͼ� ���� ���ڵ� ǥ���ϴµ� (�Ƹ� ��򰡿��� 3Byte�� �ʿ��� ���� ��������)
	//�翬��, RAW Data �� ������ �ѱ� ��ġǥ����, �Ͼ� ��ġǥ������ ���� ǥ�õǴ� ���ڴ� �޶�����
	//�̴� ���� ������ ������ �ȴ�.
	//������ �� ������� ���°��� Unicode.
	//char -> TCHAR, "" -> TEXT("") ������ ���� �Ŀ� �����ڵ�� ������ �ϴ� ������ ����.
	//�̿� ���� �ڼ��� ������ OneNote ����
	m_applicationName = L"Engine";

	//������ Ŭ������ �⺻ �������� ����ϴ�.
	//��Ÿ�Ͽ� ���� CS_HREDRAW�� CS_VREDRAW��
	//Ŭ���̾�Ʈ ����(�۾�����, ����� ����)�� ���� ���̰� ����Ǿ��� ��� �ش� ũ�⸦ ����Ͽ� ���� �ٽ� �׸����� ���ش�.
	//CS_DWNDC �� DC ĳ�ÿ� �� DC�� �������̰�, �������� ����� �ǹ̸� ���´�.
	//�̷��� �ǹ̸� �ο����ִ� ������ ������� DC�� ����� �� �ְ� �ϱ� ���ؼ�.
	//�ٸ� �̷��� DC �Ҵ��� ���� �ϸ� ���� �ʴ�.. (���� ������ ���� ����� �Ǿ����� �ʱ� ����)
	//�����찡 �ı��� �� �ڵ����� DC�� �ı��ǹǷ� ������ �ʿ䵵 ���� �����Ͽ��� �ȵȴ�.
	//�ڼ��� ������ Private DC ��� �˻��غ���
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	//�������� �޼��� ó���� ���� �ݹ� �Լ��� �����.
	//�޼����� ó���� ������ WndProc�� ��ϵ� �Լ��� ����Ѵٴ� �ǹ���.
	wc.lpfnWndProc = WndProc;

	//�Ʒ� �ΰ���, �� ������, �� WNDCLASSEX �� ����� �� ������ �޸� ������ �� �Ҵ��� �ֱ� ���� ����.
	//���� �� 0�� �ش�.
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	wc.hInstance = m_hinstance;
	//������ ����
	//�ڵ鿡 NULL�� �������ν� �����쿡�� �⺻���� �����ϴ� ICON�� ����� �� �ְ�,
	//�� ICON �߿��� WINLOGO ��� �������� ����ϰڴٴ� ��
	//�̸� ������ �������� ����ϰ�ʹٸ� �ν��Ͻ��� �ѱ�� (NULL�� �ƴ�), ���⼭�� m_hinstance �� �ɰ�.
	//resource.h�� ���ǵ� ID�� �ѱ�� �ȴ�.
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	//���� �𸣰ڴµ�.. �׳� �Ȱ��� ������ ��������
	//�Ƹ� �� �ּ�ȭ�Ǿ��ų� Ʈ���̾������� �ǹ��ϴ°� �ƴұ� ������..
	//16x16�� ���� ������ �ڵ��̶�� ��.
	wc.hIconSm = wc.hIcon;
	//���� ���������ϰ���. Ŀ�� ����� �ǹ���
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//�⺻ ��� ����
	//GetStockObject �� ����ϸ� �����쿡�� �����ϴ� �⺻ ������ ������ ����� �� ����.
	//WHITE_BRUSH : ��� ȭ��, �ݴ�� ������ ȭ��
	//�ٸ�, �޼ҵ� ����Ͽ� ��ȯ�� ��� �ǵ������� �� ��ȯ(HBRUSH) ���־�� �Ѵ�.
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	//�޴� ���ҽ��ε�...
	//���⼭�� �ϴ� ������� ������ ���߿� �˻��غ���.
	wc.lpszMenuName = NULL;
	//�� �������� Ŭ������ id���� ���� (���������� L""���� ��������� ��)
	//���⼭�� applicationName ����.
	wc.lpszClassName = m_applicationName;
	//������ Ŭ���� ����ü�� ũ�⸦ ������.
	//WNDCLASSEX ũ�⸦ �׳� ����������.
	wc.cbSize = sizeof(WNDCLASSEX);

	//������ Ŭ������ �������Ϳ� �����.
	//�翬�ϰԵ� ����ؾ� �� �� �ְ���..
	RegisterClassEx(&wc);

	//���� ����� ȭ���� �ػ󵵸� �˾ƿ�
	//C"X"SCREEN -> Width
	//C"Y"SCREEN -> Height
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	//Ǯ��ũ�� ��忡 ���� ȭ�� ���� ����
	//FULL_SCREEN �������� �׷������� ����̹Ƿ�, graphicsclass.h �� ����Ǿ�����.
	if (FULL_SCREEN)
	{
		//Ǯ��ũ�� ����� ��� ȭ�� ũ�⸦ ����ũ�� ũ�⿡ ���߰� ������ 32bit�� ��.

		//�ϴ��� DEVMODE ����ü �ʱ�ȭ
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);

		//ȭ��ũ��
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;

		//���� ��Ʈ ��
		//�Ʒ��� ���� �𸣰ڴ�.. �ƹ����� dmField �� ���ڵ�� �����ϴ� �����̴�.
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		//���� �������� Ǯ��ũ���� �´� ���÷��� ������ ����
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		//�׸��� â�� �׻� ���� ���� ����
		posX = posY = 0;
	}
	//Ǯ��ũ���� �ƴ� ������ ����� ���
	else
	{
		//â��� �� 800x600 ���� �ػ� ����
		screenWidth = 800;
		screenHeight = 600;

		//�̶��� â�� ������� �߾ӿ� �;� ��.
		//����Ͱ� 1920 ¥�����, 1920 - 800 = 1120 (������ ���� ���� ����) / 2 = 560 (�� ����) �� �;� ��.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	//������ �������� �����츦 �����, �� �ڵ��� ������
	//��.. �������
	//CreateWindow �� CreateWindowsEX �� â�� ���� �� �ִµ�, ���� �̷��ֵ��� ���� ���ڿ� �ѵΰ� ���̳��µ� �����ε� ���� �Լ��̴�.
	//���⼭�� �� �տ� ���ڰ� �߰��Ǿ���,
	// WS_EX_APPWINDOW : �����찡 ���� �� ������ �½�ũ �� ���� �ֵ��� ��
	// �� �̷� �ɼǵ���. �ڼ��Ѱ� ã�ƺ��� �ȴ�.
	// �״��� className, applicationName �� ���⼱ ������ ���� ���ְ�,
	// WS_CLIPSIBLINGS : �ڽĵ鳢�� ��ģ ������ �������� �ʰڴ� (��������� �ʰڴ�.)
	// WS_CLIPSIBLINGS : �ڽ��� �ִ� ������ �������� �ʰڴ�. (��������� �ʰڴ�.)
	// WS_POPUP : �����츦 �˾� ������� ����ڴ�.
	// �� ������ �����찡 �ڸ��� ��ġ posX, posY
	// �� ���� �ػ� screenWidth, screenHeight
	// �� ������ �θ��� �ڵ�, �޴��� �ڵ� (������ ���⼭�� �����Ƿ� NULL)
	// �� ������ �� �������� �ڵ� �ν��Ͻ� (�������� ��� ���� ������ m_hinstance �� ����ϵ��� ��)
	// ������ lpParam. LPVOID : void �� ������.. MDI Ŭ���̾�Ʈ �������� ��쿡 CLIENTCREATESTRUCT ����ü�� ��������� �ϴµ�
	// ����ٰ� �ش� �����͸� ������ �ȴٰ� ��.
	// MDI Ŭ���̾�Ʈ��... ������ �����찡 �ְ� �� �ȿ� ����ؼ� �ڽ� ��������� �����ǰ� �����Ǿ�� �ϴ� ��������.
	// �ڼ��Ѱ� �˻�����.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// �����츦 ȭ�鿡 ǥ����
	ShowWindow(m_hwnd, SW_SHOW);
	// �����츦 �� ������ ������
	SetForegroundWindow(m_hwnd);
	// �����쿡 ��Ŀ���� ��
	SetFocus(m_hwnd);

	// ���콺 Ŀ���� ǥ������ ����
	ShowCursor(false);

	return;
}

// ȭ�� ������ ������ �ǵ����� ������� �� �ڵ���� ��ȯ��.
void SystemClass::ShutdownWindows()
{
	// ���콺 Ŀ�� �ٽ� ǥ����
	ShowCursor(true);

	// Ǯ��ũ�� ��忴�ٸ� �ٽ� ���÷��� ������ �ٲ� ��
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// �����츦 �����ϰ� �ڵ� �ݳ�
	DestroyWindow(m_hwnd);
	// �ڵ� ����
	// �̷��� �ݳ��� �ڵ��̶� NULL ���� ������ Ȥ�ó� ���� �����־� �ٸ� ���α׷��� ������ �� �� �ִ�.
	// �ٸ��͵鵵 ��������.
	m_hwnd = NULL;

	// ���ø����̼� �ν��Ͻ��� ������
	// �� ������ �̸��� �ְ� ��������
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// �� Ŭ������ ���� �ܺ� ������ ������ ������
	ApplicationHandle = NULL;

	return;
}


//���⼭ �޼��� ó���� ���� �ݹ� �Լ� ����
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// �����찡 ���ŵǸ�
	case WM_DESTROY:
	{
		// ���ŵǾ������� ���� �޼����� ������ �ݹ鵵 ����
		PostQuitMessage(0);
		return 0;
	}

	// �����찡 ������
	case WM_CLOSE:
	{
		// ���������� ����.
		PostQuitMessage(0);
		return 0;
	}

	// �̿��� �޼������� �޼��� �ڵ鷯���� �����Ͽ� ó��
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}