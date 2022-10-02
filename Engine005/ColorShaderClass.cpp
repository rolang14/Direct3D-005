#include "ColorShaderClass.h"

//생성/소멸
ColorShaderClass::ColorShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
}

ColorShaderClass::ColorShaderClass(const ColorShaderClass& other)
{

}

ColorShaderClass::~ColorShaderClass()
{

}

bool ColorShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;

	//경로 넘기는 과정에서
	//튜토리얼대로 하니까 래핑 오류가 있어서 (L"" (wchar_t) -> WCHAR)
	//명시적 형 변환 해줌
	WCHAR vsWcs[256] = { L'\0', };
	WCHAR psWcs[256] = { L'\0', };

	CHAR vsPath[] = "../Engine005/color.vs";
	CHAR psPath[] = "../Engine005/color.ps";

	MultiByteToWideChar(CP_ACP, 0, vsPath, sizeof(vsPath), vsWcs, _countof(vsWcs));
	MultiByteToWideChar(CP_ACP, 0, psPath, sizeof(psPath), psWcs, _countof(psWcs));

	//정점 셰이더와 픽셀 셰이더 초기화
	//아래 보면 알겠지만 InitializeShader 로 넘기는건 디바이스, 핸들, 파일명 두개이다 (vs, ps) 여기서는 아까 만들었던 셰이더들을
	//넣을 것이므로 파일경로를 이렇게 잡아 줌
	result = InitializeShader(device, hwnd, vsWcs, psWcs);
	if (!result)
	{
		return false;
	}

	return true;
}

void ColorShaderClass::Shutdown()
{
	//정점 셰이더와 픽셸 셰이더 및 그와 관련된 것들을 반환
	ShutdownShader();

	return;
}

//여기의 Render 는 SetShaderParameters 에게 셰이더 인자를 넘겨주고
//RenderShader 으로 셋팅된 HLSL 셰이더를 이용하여 물체를 그린다..
bool ColorShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
{
	bool result;

	//렌더에 사용될 셰이더 인자 입력
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
	{
		return false;
	}

	//셰이더 그리기
	RenderShader(deviceContext, indexCount);

	return true;
}

//아래 함수는 매우 중요한 함수로,
//실제로 여기서 셰이더 파일을 불러오고, Direct X와 GPU에서 사용 가능하도록 한다.
//또한, 레이아웃을 세팅하고 어떻게 정점 버퍼의 데이터가 GPU에서 사용되는지 볼 수 있다.
//이 레이아웃은 modelclass.h 뿐만 아니라 color.vs에 선언된 VertexType 와 반드시! 동일해야 한다.
bool ColorShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFileName, WCHAR* psFileName)
{
	//결과 판단용
	HRESULT result;
	//ID3D10Blob 인터페이스는 임의의 길이의 데이터를 반환함..
	//주로 정점, 픽셀 등의 데이터 정보를 저장하는 데이터 버퍼로 사용하고,
	//다시 셰이더를 컴파일하는 API에서 객체 코드 및 오류 메세지를 반환하는데 사용된다.
	//한개의 인터페이스가 왜 다른 두개의 역할을 같이 하는지는 모르겠다. 뭔가 이유가 있긴 할듯
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;

	//포인터 초기화
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	//정점 셰이더 컴파일
	// 
	//보통 3D 모델링은 버텍스 정보와(정점) 셰이더 정보가 혼합되어있다.
	//셰이더 정보는 그냥 머테리얼, 버텍스는 모델링한것 이라고 생각하면 됨
	//앞에서부터 순서대로 셰이더 파일 명(경로), 셰이더 매크로를 정의하는 선택적 배열인 pDefines (정확히 뭔지는 모르겠다),
	//pInclude : 컴파일러가 Include 파일을 처리하는데 사용하는 ID3DInclude 인터페이스에 대한 선택적 포인터임. 만약에
	//HLSL 코드에 무언가 Include 선언이 되어있는데, 여기에 NULL 이 들어가면 오류가 발생함. 지금은 없기에 NULL.
	//셰이더의 이름, 셰이더를 컴파일 할 컴파일러의 버전(DX11은 5.0이다)
	//vs는 vertexshader 를 의미하고 5_0은 버전이다.
	//다음 두개는 Flag 임. 앞뒤 OR 연산을 사용하여 컴파일 옵션을 만들 수 있다고 한다... 뭔지는 모르겠다.
	//그 다음 pPump 는 뭔지 모르겠다.
	//데이터를 컴파일하여 받을 버퍼, 결과에 대한 에러메세지
	//에러메세지의 경우 아무것도 없다면 셰이더 파일 자체가 없다는 의미이므로 해당하는 다이얼로그를 따로 생성해 주고
	//나머지는 과정이 실패했을 때 에러 메세지를 담아 출력해야 한다.
	result = D3DX11CompileFromFile(vsFileName, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&vertexShaderBuffer, &errorMessage, NULL);

	//실패하는 경우에만
	if (FAILED(result))
	{
		//셰이더가 컴파일에 실패하면 에러 메세지를 기록함
		//에러메세지가 비어있지 않으면 컴파일 중 에러가 발생
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFileName);
		}
		else
		{
			MessageBox(hwnd, vsFileName, L"Missing Vertex Shader File", MB_OK);
		}

		return false;
	}

	//이번엔 픽셀 셰이더 컴파일
	result = D3DX11CompileFromFile(psFileName, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&pixelShaderBuffer, &errorMessage, NULL);

	//다시 예외처리
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFileName);
		}
		else
		{
			MessageBox(hwnd, vsFileName, L"Missing Pixel Shader File", MB_OK);
		}

		return false;
	}


	//컴파일된 버퍼로부터 정점 셰이더 생성
	//디바이스에 컴파일된 정점, 픽셀 셰이더의 데이터로 셰이더 객체를 만드는 과정임
	//여기에서 나온 포인터를 정점 셰이더와 픽셀 셰이더의 인터페이스로서 사용할 것임.
	//앞 두개는 컴파일한 바이너리 코드의 포인터와 길이를 전달, 세번째는 클래스 연계 인터페이스에 대한 포인터를 넣어준다는데 뭔지는 모르겠다
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}


	//다음은 셰이더에서 사용할 정점 데이터의 레이아웃을 생성해야 함.
	//어떤 식의 데이터를 사용할 것인지... 등
	//
	// 그니깐 예를 들면
	// 한개의 정점이 갖는 정보는 매우 많다
	// 위치가 어디인지(정점 데이터) 그리는 순서는 어떻게 되는지(인덱스 데이터) 법선은 어떻게 되는지(법선이란 그 Face 가 갖는 노멀 벡터라고 생각하면 된다)
	// 컬러는 어떻게 되는지 텍스처는 어떻게 되고 그 텍스처 내의 offset 이라던지.. 등 굉장히 많은 정보가 추가될 수 있는데
	// 이런 데이터들을 실제로는 내부적으로 어떻게 나열되어있고 그 크기는 얼마인지 등을 나타내는게
	// 바로 레이아웃이다.
	// 
	// 
	// 당연하게도 각각의 정보들은 각기 다른 데이터형을 갖고있을것이기 때문에 이를 잘 모듈화 해서
	// 최적의 구조체로 정의하는게 중요하다.
	// 다만, 각각의 데이터를 구조체로 정의하여도 그건 사람의 입장에서 정리되어있는 것일 뿐,
	// GPU의 입장에서는 그냥 메모리 한칸에 들어있는 데이터에 불과하다. 때문에
	// 이 데이터가 어떤 정보를 가지고 있는지에 대한 설명! 을 해주기 위해서는 입력 레이아웃이라는 것을 사용해야만 하는 것이다.
	// 가령.. 여기있는 이 버퍼가 가리키는 메모리에 있는것들은 위치 데이터다... 여기는 컬러 데이터다.. 라는걸 정의해주는 과정이
	// 레이아웃을 만들고 적용해주는 과정인 것이다.
	// 
	// 
	//여기에서는 위치 벡터와 색상 벡터를 사용하므로, 레이아웃에 각각 두개의 벡터를 포함하는 두 레이아웃을 만듬.
	//SemanticName -> 이 요소가 레이아웃에서 어떻게 사용될 것인지를 알려주는 요소이므로 레이아웃에서 가장 먼저 채워져야 함
	//일단은 POSITION 을 첫번째로, COLOR 를 두번째 레이아웃으로 처리함.
	//Format도 매우 중요함.
	//AlignedByteOffset -> 버퍼에 데이터를 어떻게 배열할 것인지 알려줌
	//예를 들면 처음 12Byte를 위치 벡터에, 다음 16Byte를 색상 벡터에 사용할 것임을 알려줘야 할 때 등에 사용하며
	//D3D11_APPEND_ALIGNED_ELEMENT 를 지정하여 자동으로 알아내도록 함. (여기서는 기본값)

	//Vertex Input Layout Description
	//반드시 ModelClass 와 셰이더에 있는 Vertex Type과 일치해야 함.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	//지금까지 설정한 것들로 (정점, 픽셀 셰이더와 레이아웃 DESC)
	//D3D 디바이스를 사용하여 입력 레이아웃을 생성함.
	//레이아웃이 생성되면 이제 정점/픽셀 셰이더들은 더 이상 사용되지 않으므로 할당을 제거

	//레이아웃의 요소 개수 (전체 레이아웃 크기 / 레이아웃 0번의 크기로 나누므로 몇개인지 확인 가능)
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = device->CreateInputLayout(polygonLayout, numElements, 
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	//사용되지 않는 셰이더 버퍼 해제
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;


	//셰이더를 사용하기 위한 마지막 단계
	//상수 버퍼 생성 (Constant Buffer)
	//지금까지 구현해왔듯이, Vertex Buffer (정점 버퍼) - 3D 물체의 Vertex 데이터
	//Index Buffer (인덱스 버퍼) - Vertex Buffer 의 데이터 순서 (정수 오프셋)을 포함한 버퍼이며 보다 효율적으로 렌더링하기 위한 버퍼임
	//위 두개는 앞서 적어놓았던 주석들을 참고하여 이해하고
	//그 다음 마지막으로 Constant Buffer 가 있다.
	//이는 셰이더 상수 데이터를 위한 버퍼이며 셰이더 상수란 그냥 말 그대로 HLSL 데이터를 의미한다.
	//셰이더 상수는 대기 시간이 낮고 CPU에서 더 자주 업데이트하는 상수들 사용에 최적화되어있어 일부는 CPU에서 사용하도록 지정해주어야 한다.
	//보통 CPU가 프레임당 한번 갱신하는것이 일반적이고 만약 카메라가 매 프레임 이동한다면 프레임마다 상수 버퍼를 새로운 시야 행렬로 갱신해야 함
	//반드시 최소 하드웨어 할당 크기 (256Byte) 의 배수여야만 한다.

	//정점 셰이더에 있는 행렬 상수 버퍼의 description 을 작성
	//지금까지 계속 Usage 에 대한 상수(열거형)들이 쓰여졌는데
	//자세히 보면
	//D3D11_USAGE_DEFAULT : 이 버퍼는 GPU에 의해 읽고 쓰기 접근이 모두 가능하다.
	//D3D11_USAGE_IMMUTABLE : GPU에서는 읽기만 가능하고 CPU는 접근이 불가능하다 (한번 생성되어진 뒤에 변화하지 않는 데이터라면 이렇게 하는게 효율적)
	//D3D11_USAGE_DYNAMIC : GPU에서는 읽기만 가능하고 CPU는 쓰기만 가능하다 (읽기 위해 Map 사용하는 등 방법 필요)
	//D3D11_USAGE_STAGING : GPU에서 계산하고 CPU로 값을 전달한다
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//상수 버퍼 바이트(블록) 크기
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	//상수 버퍼를 (D3D11_BIND_CONSTANT_BUFFER) matrixBuffer 로 사용하겠다는 의미 (Flag)
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//D3D11_USAGE_DYNAMIC 이므로 CPU에 쓰기 플래그를 주어야 한다 : D3D11_CPU_ACCESS_WRITE
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//기타 플래그 따로 지정하지 않음
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	
	//matrixBufferDescription 으로, Initial Data는 없이 (NULL) 버퍼를 만들어서 m_matrixBuffer 에다가 포인터좀 넣어줘라!
	//모든 버퍼는 이렇게 만든다
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

//InitializeShader 에서 생성했던 4개 인터페이스 해제
//보통 result = device-> 혹은 device_context-> 로 가는 부분 다 해제 (포인터를 할당받는 헤더의 멤버들)
void ColorShaderClass::ShutdownShader()
{
	// 상수 버퍼 해제
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	//레이아웃 해제
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	//픽셀 셰이더 해제
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	//정점 셰이더 해제
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}

void ColorShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize;
	unsigned long i;
	ofstream fout;

	//에러 메세지를 담고 있는 문자열 버퍼의 포인터를 가지고 옴
	compileErrors = (char*)(errorMessage->GetBufferPointer());
	//이번엔 메세지 길이를 가지고 옴
	bufferSize = errorMessage->GetBufferSize();
	//파일을 열고 메세지 기록 (로그)
	fout.open("shader-error.txt");
	//메세지 작성
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}
	//파일 닫기
	fout.close();

	//에러 메세지 반환..
	errorMessage->Release();
	errorMessage = 0;

	//에러 팝업
	MessageBox(hwnd, L"Error occurred while compiling shader. Check caption in this window and shader-error.txt for debug.", shaderFilename, MB_OK);

	return;
}

bool ColorShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	//행렬을 transpose 하여 셰이더에서 사용할 수 있게 함
	//행렬 Transpose -> 행렬을 전치시켜주는것
	//이유는 연산시에 모양을 맞추기 위함
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	//상수 버퍼의 내용을 쓸 수 있도록 잠금
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	//상수 버퍼 데이터에 대한 포인터 획득
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	//상수 버퍼에 행렬을 복사하여 사용함
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	//상수 버퍼의 잠금 해제
	deviceContext->Unmap(m_matrixBuffer, 0);

	//정점 셰이더에서의 상수 버퍼 위치를 설정
	//맨 앞으로
	bufferNumber = 0;

	//마지막으로 정점 셰이더의 상수 버퍼를 바뀐 값으로 변경
	//그러니까 상수 버퍼를 만들어준걸로 Set 하겠다는 의미 (이 메소드 이름처럼)
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	return true;
}

//호출 순서는
//Render 호출되면 Render 함수 내에서 SetShaderParameter 가 호출되어 셰이더 인자를 제대로 설정해주고 (세팅)
//그 다음 RenderShader 함수가 두번째로 호출됨.
//설정된 셰이더 버퍼 등으로 이제 실제로 셰이더를 가지고 오브젝트를 그려 줌
void ColorShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	//정점 입력 레이아웃을 설정함.
	deviceContext->IASetInputLayout(m_layout);

	//삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정
	//VS Shader
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	//PS Shader
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	//삼각형 그리기
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}


//여기까지가 일단 HLSL 프로그래밍과 여러 버퍼, 셰이더를 설정하고 호출했다.
// 
// PS, VS 정의부터 (Color .ps .vs)
// ModelClass -> GPU가 각 버퍼들을 가지고 그 내용으로 그릴 수 있도록 설정
// ColorShaderclass -> 버퍼들을 가지고 그리는데, HLSL 셰이더 (ps, vs) 에다가 셰이더 파일 사용을 위한
// 상수 버퍼를 추가하고 실제로 DeviceContext 에게 이를 이용해 그리도록 설정한다.
// 
//하지만 지금까지는 전부 World 기준에서 오브젝트의 데이터이고, 사실 화면을 통해서 보는건 Camera를 통해 보고있는 것이기 때문에
//어떻게 우리가 장면을 보는지에 대한 정보를 DirectX 11 에게 전달해줄 필요가 있다.
//이를 위해 Camera Class 를 설정하고 호출하도록 하자.