#include "TextureShaderClass.h"

TextureShaderClass::TextureShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	//추가된 변수도 초기화
	m_sampleState = 0;
}

TextureShaderClass::TextureShaderClass(const TextureShaderClass& other)
{

}

TextureShaderClass::~TextureShaderClass()
{

}

bool TextureShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;

	WCHAR vsWcs[256] = { L'\0', };
	WCHAR psWcs[256] = { L'\0', };

	CHAR vsPath[] = "../Engine005/Texture.vs";
	CHAR psPath[] = "../Engine005/Texture.ps";

	MultiByteToWideChar(CP_ACP, 0, vsPath, sizeof(vsPath), vsWcs, _countof(vsWcs));
	MultiByteToWideChar(CP_ACP, 0, psPath, sizeof(psPath), psWcs, _countof(psWcs));

	//이번엔 Color 말고 Texture vs/ps 로 셰이더 로드.
	result = InitializeShader(device, hwnd, vsWcs, psWcs);
	if (!result)
	{
		return false;
	}

	return true;
}

void TextureShaderClass::Shutdown()
{
	ShutdownShader();

	return;
}

//이제 Texture 추가해서 렌더
bool TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
								ID3D11ShaderResourceView* texture)
{
	bool result;

	//렌더에 사용될 셰이더 인자 입력
	//텍스처까지 넘겨준다
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
	if (!result)
	{
		return false;
	}

	//셰이더 그리기
	RenderShader(deviceContext, indexCount);

	return true;
}

//이번엔 텍스처 셰이더를 초기화
bool TextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFileName, WCHAR* psFileName)
{
	//결과 판단용
	HRESULT result;
	//ID3D10Blob 인터페이스는 임의의 길이의 데이터를 반환함..
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	//5강. 이번엔 텍스처 샘플러의 Desc 도 추가한다.
	D3D11_SAMPLER_DESC samplerDesc;

	//포인터 초기화
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	//정점, 픽셀 셰이더 컴파일하고 텍스처를 불러온다.
	result = D3DX11CompileFromFile(vsFileName, NULL, NULL, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&vertexShaderBuffer, &errorMessage, NULL);

	//실패하는 경우에만
	if (FAILED(result))
	{
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

	result = D3DX11CompileFromFile(psFileName, NULL, NULL, "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
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

	//레이아웃 0번 포지션에 대한건 동일하다. 바뀔 일도 없고..
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	//5강. 레이아웃 1번은 원래 Color 였지만, TextureShader 를 사용하므로 이번엔 Texture 가 된다.
	//항상 ModelClass 에서 정의된 멤버들과 동일해야 한다..
	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	//포맷을 FLOAT2로 변경
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

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


	//매트릭스 버퍼 Descriptuon 작성
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

	//이번엔 텍스처를 사용할 것이므로, 추가로 Sampler State Description 도 정의해준다.
	//나중에 픽셀 셰이더로 전달되어 매핑될 것이고, 여기서 가장 중요한 것은 Filter 이라고 한다.
	//최종 도형 표면에서 텍스처의 어느 픽셀이 사용되거나 어떻게 혼합할 것인지 (Sampling 방식) 방법을 정한다.
	//여기서는 D3D11_FILTER_MIN_MAG_MIP_LINEAR 옵션을 사용하는데
	//축소, 확대, 밉레벨 등의 샘플링 시에 선형 보간법(Linear Interpolation) 을 사용한다는 의미이다.
	//Linear Interpolation 은 처리량은 무겁지만 결과는 가장 좋다고 함.
	//Address U, V 는 언제나 0.0f~1.0f 사이에 있어야 한다는것을 잊지 말자.
	//그 범위를 나가면 0.0f~1.0f 사이로 정규화 해주어야 한다.. -> D3D11 에서는 D3D11_TEXTURE_ADDRESS_WRAP 으로 알아서 해준다
	//그러니까 이걸로 선언하는것
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//Address W 는 뭐지..?
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//밉맵의 세부 레벨 바이어스 값이라고 함.
	//계산된 LOD에 이 Bias 값을 더하면 최종적인 밉맵 레벨이 됨.
	samplerDesc.MipLODBias = 0.0f;
	//Max Anisotropy 라는건 비등방성(이방성) 필터링이다.
	//Ainsotropy : 비등방성(이방성) = 특정 방향으로의 방향성이 있다는 것을 의미함.
	//예를 들면 어떤 건물에 대한 사진을 찍었다고 할 때, 건물과 그 뒤 배경이 있을 것이다.
	//배경이 존재하는 픽셀들은 그 건물이 솟아있는 방향으로 (세로 혹은 가로 혹은 대각으로) 같은(비슷한) 색상의 픽셀들이 존재할 것이고
	//배경과 구분되는 Edge 점에서 그 테두리가 보일 것이다.. 그런 부분이 많을수록 이등방적인 이미지... 여튼 그런 거다
	//어쨌든 이 비등방성의 필터 수준이 높으면 그 구분이 더욱 명확해지게 된다. (결국 이미지가 보다 선명해지는 효과)
	samplerDesc.MaxAnisotropy = 1;
	//그러한 샘플링에 대한 판정 방식이 항상 참을 반환함
	//이게 무엇을 의미하는지는 아직 잘 모르겠음..
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;

	//자, 기본적으로 LOD라는건
	//Level of Detail 이고, 이건 그래픽 이외에도 많은 분야에서 사용하는 단어이다.
	//여기서 LOD라는건 렌더링 속도와 품질에 관련이 깊은데
	//모든건 Frame 수 만큼 초당 n번씩 렌더를 해줘야 하는 환경에서 비롯된다.
	//때문에 렌더링 속도와 품질 사이에서 타협을 잘 해야 좋은 환경이 만들어질텐데,
	//그래도 지향점은 항상 가능한 한 품질은 좋게 하면서 속도도 빠른 렌더링 기술을 개발하는 것이다.
	//여기에서 일단 LOD는 두가지로 나뉜다.
	// 
	//1. 정적 LOD : 애초에 특정 물체에 대한 Mesh 를 여러개 (품질 좋은거, 품질 보통인거, 품질 낮은거) 준비해 놓고
	//카메라와의 거리를 계산해서 가까워지면 품질이 좋은 Mesh 를,
	//거리가 중간쯤이면 품질이 보통인 것을, 거리가 매우 멀어져서 굳이 정밀하게 렌더해줄 필요가 없으면 품질 낮은거로 바꿔치기하는(?)
	//방법이다.
	//연산 자체는 아주 간단하기 때문에(거리만 비교해서 바꿔치기하면 끝이니깐) 속도는 정말 빠르다
	//그러나, 애초에 여러개의 Mesh가 내포해있는 것이므로 메모리의 낭비가 굉장히 심하고,
	//품질 변환의 기준이 되는 거리에서 급격하게 Mesh 가 변하기 때문에 튀는 현상(poppiong)이 발생하고 그로 인한 간접적 품질 저하가 있다.
	// 
	//2. 동적 LOD : Mesh 는 하나지만, 카메라와의 거리에 따라서 실시간으로 Mesh의 정밀도를 내부적으로 변화시킨다.
	//일반적으로는 Mesh 분할과 Mesh 간략화 기법으로 나뉜다고 함.
	//일단 거리에 따라 Smooth 하게 Mesh가 변화하므로 튀는 현상(poppiong)이 적고 불필요하게 메모리를 점유하는 부분도 없다.
	//대신 Mesh에 대한 연산이 필요하기 때문에 속도가 매우 느려진다.
	//정점 계산을 동적으로 계속 수행하기 때문에 Level 이 바뀌거나, 정점 데이터를 갱신해야 할 때 Lock-Unlock 를 자주 수행해야 한다.
	// 
	//DX11 부터는 이걸 GPU에서 알아서 하도록 해놓았기 때문에 따로 구현해줄 필요는 없다..
	//그리고 Poppiong 
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//이제 샘플러 상태를 만들어준다
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

//InitializeShader 에서 생성했던 4개 인터페이스 해제
//보통 result = device-> 혹은 device_context-> 로 가는 부분 다 해제 (포인터를 할당받는 헤더의 멤버들)
void TextureShaderClass::ShutdownShader()
{
	//5강. Sampler State 해제
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

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

void TextureShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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

//5강. 이제 셰이더에 Texture 같이 전달해준다.
//당연하게도, 텍스처는 버퍼에 Render 하기 전에 이미 설정되어있어야 한다.
bool TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
											ID3D11ShaderResourceView* texture)
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
	//5강. 이전에는 PSSet 은 없었다 (Vertex Color Shader 에 색만 들어가므로 픽셀 셰이더에는 뭔가를 넣어줄 일이 없었다.)
	//그러나 이번에는 텍스처를 픽셀 셰이더에 넣어주어야 한다.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}

//호출 순서는
//Render 호출되면 Render 함수 내에서 SetShaderParameter 가 호출되어 셰이더 인자를 제대로 설정해주고 (세팅)
//그 다음 RenderShader 함수가 두번째로 호출됨.
//설정된 셰이더 버퍼 등으로 이제 실제로 셰이더를 가지고 오브젝트를 그려 줌
void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	//정점 입력 레이아웃을 설정함.
	deviceContext->IASetInputLayout(m_layout);

	//삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정
	//VS Shader
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	//PS Shader
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	//5강. 이제 샘플러 상태를 PS에 넣어줘서 주어진 자원으로(텍스처) 이렇게 샘플링 하라는걸 알려주자.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	//삼각형 그리기
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}


//여기까지가 일단 HLSL 프로그래밍과 여러 버퍼, 셰이더를 설정하고 호출했다.
// 
// PS, VS 정의부터 (Color .ps .vs)
// ModelClass -> GPU가 각 버퍼들을 가지고 그 내용으로 그릴 수 있도록 설정
// TextureShaderclass -> 버퍼들을 가지고 그리는데, HLSL 셰이더 (ps, vs) 에다가 셰이더 파일 사용을 위한
// 상수 버퍼를 추가하고 실제로 DeviceContext 에게 이를 이용해 그리도록 설정한다.
// 
//하지만 지금까지는 전부 World 기준에서 오브젝트의 데이터이고, 사실 화면을 통해서 보는건 Camera를 통해 보고있는 것이기 때문에
//어떻게 우리가 장면을 보는지에 대한 정보를 DirectX 11 에게 전달해줄 필요가 있다.
//이를 위해 Camera Class 를 설정하고 호출하도록 하자.