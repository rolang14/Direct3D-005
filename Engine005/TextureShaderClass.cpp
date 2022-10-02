#include "TextureShaderClass.h"

TextureShaderClass::TextureShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	//�߰��� ������ �ʱ�ȭ
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

	//�̹��� Color ���� Texture vs/ps �� ���̴� �ε�.
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

//���� Texture �߰��ؼ� ����
bool TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
								ID3D11ShaderResourceView* texture)
{
	bool result;

	//������ ���� ���̴� ���� �Է�
	//�ؽ�ó���� �Ѱ��ش�
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
	if (!result)
	{
		return false;
	}

	//���̴� �׸���
	RenderShader(deviceContext, indexCount);

	return true;
}

//�̹��� �ؽ�ó ���̴��� �ʱ�ȭ
bool TextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFileName, WCHAR* psFileName)
{
	//��� �Ǵܿ�
	HRESULT result;
	//ID3D10Blob �������̽��� ������ ������ �����͸� ��ȯ��..
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	//5��. �̹��� �ؽ�ó ���÷��� Desc �� �߰��Ѵ�.
	D3D11_SAMPLER_DESC samplerDesc;

	//������ �ʱ�ȭ
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	//����, �ȼ� ���̴� �������ϰ� �ؽ�ó�� �ҷ��´�.
	result = D3DX11CompileFromFile(vsFileName, NULL, NULL, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&vertexShaderBuffer, &errorMessage, NULL);

	//�����ϴ� ��쿡��
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

	//���̾ƿ� 0�� �����ǿ� ���Ѱ� �����ϴ�. �ٲ� �ϵ� ����..
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	//5��. ���̾ƿ� 1���� ���� Color ������, TextureShader �� ����ϹǷ� �̹��� Texture �� �ȴ�.
	//�׻� ModelClass ���� ���ǵ� ������ �����ؾ� �Ѵ�..
	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	//������ FLOAT2�� ����
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

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


	//��Ʈ���� ���� Descriptuon �ۼ�
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

	//�̹��� �ؽ�ó�� ����� ���̹Ƿ�, �߰��� Sampler State Description �� �������ش�.
	//���߿� �ȼ� ���̴��� ���޵Ǿ� ���ε� ���̰�, ���⼭ ���� �߿��� ���� Filter �̶�� �Ѵ�.
	//���� ���� ǥ�鿡�� �ؽ�ó�� ��� �ȼ��� ���ǰų� ��� ȥ���� ������ (Sampling ���) ����� ���Ѵ�.
	//���⼭�� D3D11_FILTER_MIN_MAG_MIP_LINEAR �ɼ��� ����ϴµ�
	//���, Ȯ��, �ӷ��� ���� ���ø� �ÿ� ���� ������(Linear Interpolation) �� ����Ѵٴ� �ǹ��̴�.
	//Linear Interpolation �� ó������ �������� ����� ���� ���ٰ� ��.
	//Address U, V �� ������ 0.0f~1.0f ���̿� �־�� �Ѵٴ°��� ���� ����.
	//�� ������ ������ 0.0f~1.0f ���̷� ����ȭ ���־�� �Ѵ�.. -> D3D11 ������ D3D11_TEXTURE_ADDRESS_WRAP ���� �˾Ƽ� ���ش�
	//�׷��ϱ� �̰ɷ� �����ϴ°�
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//Address W �� ����..?
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//�Ӹ��� ���� ���� ���̾ ���̶�� ��.
	//���� LOD�� �� Bias ���� ���ϸ� �������� �Ӹ� ������ ��.
	samplerDesc.MipLODBias = 0.0f;
	//Max Anisotropy ��°� ���漺(�̹漺) ���͸��̴�.
	//Ainsotropy : ���漺(�̹漺) = Ư�� ���������� ���⼺�� �ִٴ� ���� �ǹ���.
	//���� ��� � �ǹ��� ���� ������ ����ٰ� �� ��, �ǹ��� �� �� ����� ���� ���̴�.
	//����� �����ϴ� �ȼ����� �� �ǹ��� �ھ��ִ� �������� (���� Ȥ�� ���� Ȥ�� �밢����) ����(�����) ������ �ȼ����� ������ ���̰�
	//���� ���еǴ� Edge ������ �� �׵θ��� ���� ���̴�.. �׷� �κ��� �������� �̵������ �̹���... ��ư �׷� �Ŵ�
	//��·�� �� ���漺�� ���� ������ ������ �� ������ ���� ��Ȯ������ �ȴ�. (�ᱹ �̹����� ���� ���������� ȿ��)
	samplerDesc.MaxAnisotropy = 1;
	//�׷��� ���ø��� ���� ���� ����� �׻� ���� ��ȯ��
	//�̰� ������ �ǹ��ϴ����� ���� �� �𸣰���..
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;

	//��, �⺻������ LOD��°�
	//Level of Detail �̰�, �̰� �׷��� �̿ܿ��� ���� �о߿��� ����ϴ� �ܾ��̴�.
	//���⼭ LOD��°� ������ �ӵ��� ǰ���� ������ ������
	//���� Frame �� ��ŭ �ʴ� n���� ������ ����� �ϴ� ȯ�濡�� ��Եȴ�.
	//������ ������ �ӵ��� ǰ�� ���̿��� Ÿ���� �� �ؾ� ���� ȯ���� ��������ٵ�,
	//�׷��� �������� �׻� ������ �� ǰ���� ���� �ϸ鼭 �ӵ��� ���� ������ ����� �����ϴ� ���̴�.
	//���⿡�� �ϴ� LOD�� �ΰ����� ������.
	// 
	//1. ���� LOD : ���ʿ� Ư�� ��ü�� ���� Mesh �� ������ (ǰ�� ������, ǰ�� �����ΰ�, ǰ�� ������) �غ��� ����
	//ī�޶���� �Ÿ��� ����ؼ� ��������� ǰ���� ���� Mesh ��,
	//�Ÿ��� �߰����̸� ǰ���� ������ ����, �Ÿ��� �ſ� �־����� ���� �����ϰ� �������� �ʿ䰡 ������ ǰ�� �����ŷ� �ٲ�ġ���ϴ�(?)
	//����̴�.
	//���� ��ü�� ���� �����ϱ� ������(�Ÿ��� ���ؼ� �ٲ�ġ���ϸ� ���̴ϱ�) �ӵ��� ���� ������
	//�׷���, ���ʿ� �������� Mesh�� �������ִ� ���̹Ƿ� �޸��� ���� ������ ���ϰ�,
	//ǰ�� ��ȯ�� ������ �Ǵ� �Ÿ����� �ް��ϰ� Mesh �� ���ϱ� ������ Ƣ�� ����(poppiong)�� �߻��ϰ� �׷� ���� ������ ǰ�� ���ϰ� �ִ�.
	// 
	//2. ���� LOD : Mesh �� �ϳ�����, ī�޶���� �Ÿ��� ���� �ǽð����� Mesh�� ���е��� ���������� ��ȭ��Ų��.
	//�Ϲ������δ� Mesh ���Ұ� Mesh ����ȭ ������� �����ٰ� ��.
	//�ϴ� �Ÿ��� ���� Smooth �ϰ� Mesh�� ��ȭ�ϹǷ� Ƣ�� ����(poppiong)�� ���� ���ʿ��ϰ� �޸𸮸� �����ϴ� �κе� ����.
	//��� Mesh�� ���� ������ �ʿ��ϱ� ������ �ӵ��� �ſ� ��������.
	//���� ����� �������� ��� �����ϱ� ������ Level �� �ٲ�ų�, ���� �����͸� �����ؾ� �� �� Lock-Unlock �� ���� �����ؾ� �Ѵ�.
	// 
	//DX11 ���ʹ� �̰� GPU���� �˾Ƽ� �ϵ��� �س��ұ� ������ ���� �������� �ʿ�� ����..
	//�׸��� Poppiong 
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//���� ���÷� ���¸� ������ش�
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

//InitializeShader ���� �����ߴ� 4�� �������̽� ����
//���� result = device-> Ȥ�� device_context-> �� ���� �κ� �� ���� (�����͸� �Ҵ�޴� ����� �����)
void TextureShaderClass::ShutdownShader()
{
	//5��. Sampler State ����
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

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

void TextureShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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

//5��. ���� ���̴��� Texture ���� �������ش�.
//�翬�ϰԵ�, �ؽ�ó�� ���ۿ� Render �ϱ� ���� �̹� �����Ǿ��־�� �Ѵ�.
bool TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
											ID3D11ShaderResourceView* texture)
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
	//5��. �������� PSSet �� ������ (Vertex Color Shader �� ���� ���Ƿ� �ȼ� ���̴����� ������ �־��� ���� ������.)
	//�׷��� �̹����� �ؽ�ó�� �ȼ� ���̴��� �־��־�� �Ѵ�.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}

//ȣ�� ������
//Render ȣ��Ǹ� Render �Լ� ������ SetShaderParameter �� ȣ��Ǿ� ���̴� ���ڸ� ����� �������ְ� (����)
//�� ���� RenderShader �Լ��� �ι�°�� ȣ���.
//������ ���̴� ���� ������ ���� ������ ���̴��� ������ ������Ʈ�� �׷� ��
void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	//���� �Է� ���̾ƿ��� ������.
	deviceContext->IASetInputLayout(m_layout);

	//�ﰢ���� �׸� ���� ���̴��� �ȼ� ���̴��� ����
	//VS Shader
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	//PS Shader
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	//5��. ���� ���÷� ���¸� PS�� �־��༭ �־��� �ڿ�����(�ؽ�ó) �̷��� ���ø� �϶�°� �˷�����.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	//�ﰢ�� �׸���
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}


//��������� �ϴ� HLSL ���α׷��ְ� ���� ����, ���̴��� �����ϰ� ȣ���ߴ�.
// 
// PS, VS ���Ǻ��� (Color .ps .vs)
// ModelClass -> GPU�� �� ���۵��� ������ �� �������� �׸� �� �ֵ��� ����
// TextureShaderclass -> ���۵��� ������ �׸��µ�, HLSL ���̴� (ps, vs) ���ٰ� ���̴� ���� ����� ����
// ��� ���۸� �߰��ϰ� ������ DeviceContext ���� �̸� �̿��� �׸����� �����Ѵ�.
// 
//������ ���ݱ����� ���� World ���ؿ��� ������Ʈ�� �������̰�, ��� ȭ���� ���ؼ� ���°� Camera�� ���� �����ִ� ���̱� ������
//��� �츮�� ����� �������� ���� ������ DirectX 11 ���� �������� �ʿ䰡 �ִ�.
//�̸� ���� Camera Class �� �����ϰ� ȣ���ϵ��� ����.