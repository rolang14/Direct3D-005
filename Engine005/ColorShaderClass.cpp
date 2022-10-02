#include "ColorShaderClass.h"

//����/�Ҹ�
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

	//��� �ѱ�� ��������
	//Ʃ�丮���� �ϴϱ� ���� ������ �־ (L"" (wchar_t) -> WCHAR)
	//����� �� ��ȯ ����
	WCHAR vsWcs[256] = { L'\0', };
	WCHAR psWcs[256] = { L'\0', };

	CHAR vsPath[] = "../Engine005/color.vs";
	CHAR psPath[] = "../Engine005/color.ps";

	MultiByteToWideChar(CP_ACP, 0, vsPath, sizeof(vsPath), vsWcs, _countof(vsWcs));
	MultiByteToWideChar(CP_ACP, 0, psPath, sizeof(psPath), psWcs, _countof(psWcs));

	//���� ���̴��� �ȼ� ���̴� �ʱ�ȭ
	//�Ʒ� ���� �˰����� InitializeShader �� �ѱ�°� ����̽�, �ڵ�, ���ϸ� �ΰ��̴� (vs, ps) ���⼭�� �Ʊ� ������� ���̴�����
	//���� ���̹Ƿ� ���ϰ�θ� �̷��� ��� ��
	result = InitializeShader(device, hwnd, vsWcs, psWcs);
	if (!result)
	{
		return false;
	}

	return true;
}

void ColorShaderClass::Shutdown()
{
	//���� ���̴��� �ȼ� ���̴� �� �׿� ���õ� �͵��� ��ȯ
	ShutdownShader();

	return;
}

//������ Render �� SetShaderParameters ���� ���̴� ���ڸ� �Ѱ��ְ�
//RenderShader ���� ���õ� HLSL ���̴��� �̿��Ͽ� ��ü�� �׸���..
bool ColorShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
{
	bool result;

	//������ ���� ���̴� ���� �Է�
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
	{
		return false;
	}

	//���̴� �׸���
	RenderShader(deviceContext, indexCount);

	return true;
}

//�Ʒ� �Լ��� �ſ� �߿��� �Լ���,
//������ ���⼭ ���̴� ������ �ҷ�����, Direct X�� GPU���� ��� �����ϵ��� �Ѵ�.
//����, ���̾ƿ��� �����ϰ� ��� ���� ������ �����Ͱ� GPU���� ���Ǵ��� �� �� �ִ�.
//�� ���̾ƿ��� modelclass.h �Ӹ� �ƴ϶� color.vs�� ����� VertexType �� �ݵ��! �����ؾ� �Ѵ�.
bool ColorShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFileName, WCHAR* psFileName)
{
	//��� �Ǵܿ�
	HRESULT result;
	//ID3D10Blob �������̽��� ������ ������ �����͸� ��ȯ��..
	//�ַ� ����, �ȼ� ���� ������ ������ �����ϴ� ������ ���۷� ����ϰ�,
	//�ٽ� ���̴��� �������ϴ� API���� ��ü �ڵ� �� ���� �޼����� ��ȯ�ϴµ� ���ȴ�.
	//�Ѱ��� �������̽��� �� �ٸ� �ΰ��� ������ ���� �ϴ����� �𸣰ڴ�. ���� ������ �ֱ� �ҵ�
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;

	//������ �ʱ�ȭ
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	//���� ���̴� ������
	// 
	//���� 3D �𵨸��� ���ؽ� ������(����) ���̴� ������ ȥ�յǾ��ִ�.
	//���̴� ������ �׳� ���׸���, ���ؽ��� �𵨸��Ѱ� �̶�� �����ϸ� ��
	//�տ������� ������� ���̴� ���� ��(���), ���̴� ��ũ�θ� �����ϴ� ������ �迭�� pDefines (��Ȯ�� ������ �𸣰ڴ�),
	//pInclude : �����Ϸ��� Include ������ ó���ϴµ� ����ϴ� ID3DInclude �������̽��� ���� ������ ��������. ���࿡
	//HLSL �ڵ忡 ���� Include ������ �Ǿ��ִµ�, ���⿡ NULL �� ���� ������ �߻���. ������ ���⿡ NULL.
	//���̴��� �̸�, ���̴��� ������ �� �����Ϸ��� ����(DX11�� 5.0�̴�)
	//vs�� vertexshader �� �ǹ��ϰ� 5_0�� �����̴�.
	//���� �ΰ��� Flag ��. �յ� OR ������ ����Ͽ� ������ �ɼ��� ���� �� �ִٰ� �Ѵ�... ������ �𸣰ڴ�.
	//�� ���� pPump �� ���� �𸣰ڴ�.
	//�����͸� �������Ͽ� ���� ����, ����� ���� �����޼���
	//�����޼����� ��� �ƹ��͵� ���ٸ� ���̴� ���� ��ü�� ���ٴ� �ǹ��̹Ƿ� �ش��ϴ� ���̾�α׸� ���� ������ �ְ�
	//�������� ������ �������� �� ���� �޼����� ��� ����ؾ� �Ѵ�.
	result = D3DX11CompileFromFile(vsFileName, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&vertexShaderBuffer, &errorMessage, NULL);

	//�����ϴ� ��쿡��
	if (FAILED(result))
	{
		//���̴��� �����Ͽ� �����ϸ� ���� �޼����� �����
		//�����޼����� ������� ������ ������ �� ������ �߻�
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

	//�̹��� �ȼ� ���̴� ������
	result = D3DX11CompileFromFile(psFileName, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&pixelShaderBuffer, &errorMessage, NULL);

	//�ٽ� ����ó��
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


	//�����ϵ� ���۷κ��� ���� ���̴� ����
	//����̽��� �����ϵ� ����, �ȼ� ���̴��� �����ͷ� ���̴� ��ü�� ����� ������
	//���⿡�� ���� �����͸� ���� ���̴��� �ȼ� ���̴��� �������̽��μ� ����� ����.
	//�� �ΰ��� �������� ���̳ʸ� �ڵ��� �����Ϳ� ���̸� ����, ����°�� Ŭ���� ���� �������̽��� ���� �����͸� �־��شٴµ� ������ �𸣰ڴ�
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


	//������ ���̴����� ����� ���� �������� ���̾ƿ��� �����ؾ� ��.
	//� ���� �����͸� ����� ������... ��
	//
	// �״ϱ� ���� ���
	// �Ѱ��� ������ ���� ������ �ſ� ����
	// ��ġ�� �������(���� ������) �׸��� ������ ��� �Ǵ���(�ε��� ������) ������ ��� �Ǵ���(�����̶� �� Face �� ���� ��� ���Ͷ�� �����ϸ� �ȴ�)
	// �÷��� ��� �Ǵ��� �ؽ�ó�� ��� �ǰ� �� �ؽ�ó ���� offset �̶����.. �� ������ ���� ������ �߰��� �� �ִµ�
	// �̷� �����͵��� �����δ� ���������� ��� �����Ǿ��ְ� �� ũ��� ������ ���� ��Ÿ���°�
	// �ٷ� ���̾ƿ��̴�.
	// 
	// 
	// �翬�ϰԵ� ������ �������� ���� �ٸ� ���������� �����������̱� ������ �̸� �� ���ȭ �ؼ�
	// ������ ����ü�� �����ϴ°� �߿��ϴ�.
	// �ٸ�, ������ �����͸� ����ü�� �����Ͽ��� �װ� ����� ���忡�� �����Ǿ��ִ� ���� ��,
	// GPU�� ���忡���� �׳� �޸� ��ĭ�� ����ִ� �����Ϳ� �Ұ��ϴ�. ������
	// �� �����Ͱ� � ������ ������ �ִ����� ���� ����! �� ���ֱ� ���ؼ��� �Է� ���̾ƿ��̶�� ���� ����ؾ߸� �ϴ� ���̴�.
	// ����.. �����ִ� �� ���۰� ����Ű�� �޸𸮿� �ִ°͵��� ��ġ �����ʹ�... ����� �÷� �����ʹ�.. ��°� �������ִ� ������
	// ���̾ƿ��� ����� �������ִ� ������ ���̴�.
	// 
	// 
	//���⿡���� ��ġ ���Ϳ� ���� ���͸� ����ϹǷ�, ���̾ƿ��� ���� �ΰ��� ���͸� �����ϴ� �� ���̾ƿ��� ����.
	//SemanticName -> �� ��Ұ� ���̾ƿ����� ��� ���� �������� �˷��ִ� ����̹Ƿ� ���̾ƿ����� ���� ���� ä������ ��
	//�ϴ��� POSITION �� ù��°��, COLOR �� �ι�° ���̾ƿ����� ó����.
	//Format�� �ſ� �߿���.
	//AlignedByteOffset -> ���ۿ� �����͸� ��� �迭�� ������ �˷���
	//���� ��� ó�� 12Byte�� ��ġ ���Ϳ�, ���� 16Byte�� ���� ���Ϳ� ����� ������ �˷���� �� �� � ����ϸ�
	//D3D11_APPEND_ALIGNED_ELEMENT �� �����Ͽ� �ڵ����� �˾Ƴ����� ��. (���⼭�� �⺻��)

	//Vertex Input Layout Description
	//�ݵ�� ModelClass �� ���̴��� �ִ� Vertex Type�� ��ġ�ؾ� ��.
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

	//���ݱ��� ������ �͵�� (����, �ȼ� ���̴��� ���̾ƿ� DESC)
	//D3D ����̽��� ����Ͽ� �Է� ���̾ƿ��� ������.
	//���̾ƿ��� �����Ǹ� ���� ����/�ȼ� ���̴����� �� �̻� ������ �����Ƿ� �Ҵ��� ����

	//���̾ƿ��� ��� ���� (��ü ���̾ƿ� ũ�� / ���̾ƿ� 0���� ũ��� �����Ƿ� ����� Ȯ�� ����)
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = device->CreateInputLayout(polygonLayout, numElements, 
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	//������ �ʴ� ���̴� ���� ����
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;


	//���̴��� ����ϱ� ���� ������ �ܰ�
	//��� ���� ���� (Constant Buffer)
	//���ݱ��� �����ؿԵ���, Vertex Buffer (���� ����) - 3D ��ü�� Vertex ������
	//Index Buffer (�ε��� ����) - Vertex Buffer �� ������ ���� (���� ������)�� ������ �����̸� ���� ȿ�������� �������ϱ� ���� ������
	//�� �ΰ��� �ռ� ������Ҵ� �ּ����� �����Ͽ� �����ϰ�
	//�� ���� ���������� Constant Buffer �� �ִ�.
	//�̴� ���̴� ��� �����͸� ���� �����̸� ���̴� ����� �׳� �� �״�� HLSL �����͸� �ǹ��Ѵ�.
	//���̴� ����� ��� �ð��� ���� CPU���� �� ���� ������Ʈ�ϴ� ����� ��뿡 ����ȭ�Ǿ��־� �Ϻδ� CPU���� ����ϵ��� �������־�� �Ѵ�.
	//���� CPU�� �����Ӵ� �ѹ� �����ϴ°��� �Ϲ����̰� ���� ī�޶� �� ������ �̵��Ѵٸ� �����Ӹ��� ��� ���۸� ���ο� �þ� ��ķ� �����ؾ� ��
	//�ݵ�� �ּ� �ϵ���� �Ҵ� ũ�� (256Byte) �� ������߸� �Ѵ�.

	//���� ���̴��� �ִ� ��� ��� ������ description �� �ۼ�
	//���ݱ��� ��� Usage �� ���� ���(������)���� �������µ�
	//�ڼ��� ����
	//D3D11_USAGE_DEFAULT : �� ���۴� GPU�� ���� �а� ���� ������ ��� �����ϴ�.
	//D3D11_USAGE_IMMUTABLE : GPU������ �б⸸ �����ϰ� CPU�� ������ �Ұ����ϴ� (�ѹ� �����Ǿ��� �ڿ� ��ȭ���� �ʴ� �����Ͷ�� �̷��� �ϴ°� ȿ����)
	//D3D11_USAGE_DYNAMIC : GPU������ �б⸸ �����ϰ� CPU�� ���⸸ �����ϴ� (�б� ���� Map ����ϴ� �� ��� �ʿ�)
	//D3D11_USAGE_STAGING : GPU���� ����ϰ� CPU�� ���� �����Ѵ�
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//��� ���� ����Ʈ(���) ũ��
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	//��� ���۸� (D3D11_BIND_CONSTANT_BUFFER) matrixBuffer �� ����ϰڴٴ� �ǹ� (Flag)
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//D3D11_USAGE_DYNAMIC �̹Ƿ� CPU�� ���� �÷��׸� �־�� �Ѵ� : D3D11_CPU_ACCESS_WRITE
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//��Ÿ �÷��� ���� �������� ����
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	
	//matrixBufferDescription ����, Initial Data�� ���� (NULL) ���۸� ���� m_matrixBuffer ���ٰ� �������� �־����!
	//��� ���۴� �̷��� �����
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

//InitializeShader ���� �����ߴ� 4�� �������̽� ����
//���� result = device-> Ȥ�� device_context-> �� ���� �κ� �� ���� (�����͸� �Ҵ�޴� ����� �����)
void ColorShaderClass::ShutdownShader()
{
	// ��� ���� ����
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	//���̾ƿ� ����
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	//�ȼ� ���̴� ����
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	//���� ���̴� ����
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

	//���� �޼����� ��� �ִ� ���ڿ� ������ �����͸� ������ ��
	compileErrors = (char*)(errorMessage->GetBufferPointer());
	//�̹��� �޼��� ���̸� ������ ��
	bufferSize = errorMessage->GetBufferSize();
	//������ ���� �޼��� ��� (�α�)
	fout.open("shader-error.txt");
	//�޼��� �ۼ�
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}
	//���� �ݱ�
	fout.close();

	//���� �޼��� ��ȯ..
	errorMessage->Release();
	errorMessage = 0;

	//���� �˾�
	MessageBox(hwnd, L"Error occurred while compiling shader. Check caption in this window and shader-error.txt for debug.", shaderFilename, MB_OK);

	return;
}

bool ColorShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	//����� transpose �Ͽ� ���̴����� ����� �� �ְ� ��
	//��� Transpose -> ����� ��ġ�����ִ°�
	//������ ����ÿ� ����� ���߱� ����
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	//��� ������ ������ �� �� �ֵ��� ���
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	//��� ���� �����Ϳ� ���� ������ ȹ��
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	//��� ���ۿ� ����� �����Ͽ� �����
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	//��� ������ ��� ����
	deviceContext->Unmap(m_matrixBuffer, 0);

	//���� ���̴������� ��� ���� ��ġ�� ����
	//�� ������
	bufferNumber = 0;

	//���������� ���� ���̴��� ��� ���۸� �ٲ� ������ ����
	//�׷��ϱ� ��� ���۸� ������ذɷ� Set �ϰڴٴ� �ǹ� (�� �޼ҵ� �̸�ó��)
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	return true;
}

//ȣ�� ������
//Render ȣ��Ǹ� Render �Լ� ������ SetShaderParameter �� ȣ��Ǿ� ���̴� ���ڸ� ����� �������ְ� (����)
//�� ���� RenderShader �Լ��� �ι�°�� ȣ���.
//������ ���̴� ���� ������ ���� ������ ���̴��� ������ ������Ʈ�� �׷� ��
void ColorShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	//���� �Է� ���̾ƿ��� ������.
	deviceContext->IASetInputLayout(m_layout);

	//�ﰢ���� �׸� ���� ���̴��� �ȼ� ���̴��� ����
	//VS Shader
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	//PS Shader
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	//�ﰢ�� �׸���
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}


//��������� �ϴ� HLSL ���α׷��ְ� ���� ����, ���̴��� �����ϰ� ȣ���ߴ�.
// 
// PS, VS ���Ǻ��� (Color .ps .vs)
// ModelClass -> GPU�� �� ���۵��� ������ �� �������� �׸� �� �ֵ��� ����
// ColorShaderclass -> ���۵��� ������ �׸��µ�, HLSL ���̴� (ps, vs) ���ٰ� ���̴� ���� ����� ����
// ��� ���۸� �߰��ϰ� ������ DeviceContext ���� �̸� �̿��� �׸����� �����Ѵ�.
// 
//������ ���ݱ����� ���� World ���ؿ��� ������Ʈ�� �������̰�, ��� ȭ���� ���ؼ� ���°� Camera�� ���� �����ִ� ���̱� ������
//��� �츮�� ����� �������� ���� ������ DirectX 11 ���� �������� �ʿ䰡 �ִ�.
//�̸� ���� Camera Class �� �����ϰ� ȣ���ϵ��� ����.