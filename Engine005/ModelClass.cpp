#include "ModelClass.h"

//ModelClass �ʱ�ȭ

ModelClass::ModelClass()
{
	//Ŭ���� �����ϸ鼭 ���� ������ �ʱ�ȭ
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	//5��. ���ο� �ؽ�ó ������ ���� �ʱ�ȭ
	m_Texture = 0;
}

ModelClass::ModelClass(const ModelClass& other)
{

}

ModelClass::~ModelClass()
{

}

//5��. �ؽ�ó ���ϱ��� �߰��ؼ� �ʱ�ȭ
bool ModelClass::Initialize(ID3D11Device* device, WCHAR* textureFileName)
{
	bool result;

	//���� �ʱ�ȭ
	//���� Private ���� �Լ��� ���ư���.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	//5��. �ؽ�ó �ε�
	result = LoadTexture(device, textureFileName);
	if (!result)
	{
		return false;
	}

	return true;
}

void ModelClass::Shutdown()
{
	//5��. �ؽ�ó�� shutdown ���ش�.
	ReleaseTexture();

	//���� ����(����)�ϴ� �����Լ�
	ShutdownBuffers();

	return;
}

//GraphicsClass::Render ���� ȣ���� ModelClass Render �Լ�..
//RenderBuffers �Լ��� ȣ���Ͽ� ���� ���ۿ� �ε��� ���۸� �׷��� ���������ο� �־� �÷� ���̴��� �׸� �� �ֵ��� ����
void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// ���� ���ۿ� �ε��� ���۸� �׷��Ƚ� ���������ο� �־� ȭ�鿡 �׸� �غ� ��
	RenderBuffers(deviceContext);

	return;
}

//�ش� ���� �ε��� ������ �˷��ִµ�, �÷� ���̴����� ���� �׸��� ���ؼ� �� ������ �ʿ��ϴ�..
int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

//5��. Texture �ҷ����� (TextureClass ���� ������� �Լ��� �ε��ϴ°ͻ��̴�)
ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_Texture->GetTexture();
}

//InitializeBuffers �Լ��� ���� ���ۿ� �ε��� ���۸� �����ϴ� �۾��� ������.
//������ ���Ϸκ��� ���� ������ �о� �ͼ� ���۸� ����� ���� �Ѵ�..
//������ �ﰢ�� �ϳ��� ������̱� ������ ���� ���ۿ� �ε��� ���ۿ� �� ���� �����ϴ� �ϸ� ��
bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	//�ϴ� 4�� ������ �ñ��ϸ� Engine004-1 ������ ��
	//5��. �簢�� ���������� �ﰢ������ �ٽ� �����Ұ���
	// ���� �迭�� ���� ���� (�ﰢ���̹Ƿ� 3��)
	m_vertexCount = 4;
	// �ε��� �迭�� ���� ���� (�ﰢ���̹Ƿ� 3��)
	m_indexCount = 6;

	// ���� �迭�� ���� (vertexCount ��ŭ)
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// �ε��� �迭 ����
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// ����/�ε��� �迭�� �ﰢ���� �� ���� �� ������ ä������...
	// �߿��Ѱ��� D3DClass ���� �����ߵ��� �ð� �������� �������� �׷��� �ո����� �νĵǾ� �׷�����
	// �ݴ�� �׷������� �޸��� �ǹǷ� Backface Culling �� ���ؼ� ������ �ʰ� �ȴ�.

	//���� �ñ��ϸ� Engine 004-1 ã�ƺ���
	// 
	//5��. �ϴ� �ؽ�ó�� ��ǥ�迡 ���� ���ذ� �ʿ��ϴ�
	//�ؽ�ó�� 2D �̹����� �ϴ� ��ǥ�谡 2�� (D3DXVECTOR2, float2)
	//U,V ��ǥ�� �װ��̰�
	//�̹����� ���� ������ U�� ���� ���� (���ʺ��� 0 ������ ���� 1�̶� float �� 0.0f ~ 1.0f ���̶�� ǥ���ȴ�.)
	//V�� ���� ���� (���������� �Ʒ��� ������ 0.0f ~ 1.0f ���̶�� ǥ���ȴ�.)
	// 
	//�׷��ϱ�, �Ʒ����� D3DXVECTOR2 ǥ���ϴ°� 0.0f, 1.0f �� ���� �Ʒ�
	//0.5f, 0.0f �� ��� ��, 1.0f, 1.0f �� ������ �Ʒ��̴�.
	//�ؽ�ó ��ǥ�� �ﰢ���� �����شٰ� �� �� �ִ�..

	vertices[0].position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f); //���� �Ʒ�
	vertices[0].texture = D3DXVECTOR2(0.0f, 1.0f); //Texture UV Local Coord. �տ������� U, V �̰� 

	vertices[1].position = D3DXVECTOR3(-1.0f, 1.0f, 0.0f); //���� ��
	vertices[1].texture = D3DXVECTOR2(0.0f, 0.0f); //Texture UV Local Coord.

	vertices[2].position = D3DXVECTOR3(1.0f, 1.0f, 0.0f); //������ ��
	vertices[2].texture = D3DXVECTOR2(1.0f, 0.0f); //Texture UV Local Coord.

	vertices[3].position = D3DXVECTOR3(1.0f, -1.0f, 0.0f); //������ �Ʒ�
	vertices[3].texture = D3DXVECTOR2(1.0f, 1.0f); //Texture UV Local Coord.

	//�ε��� �迭�� �������� ����
	indices[0] = 0;	//���� �Ʒ�
	indices[1] = 1;	//���� ��
	indices[2] = 2; //������ ��

	indices[3] = 2;	//������ ��
	indices[4] = 3;	//������ �Ʒ�
	indices[5] = 0;	//���� �Ʒ�

	//���� �ݴ�� �׷ȴٸ� 2, 1, 0 ������ �Ҵ�����������

	//���� ����, �ε��� �迭 �� ä�����Ƿ� �� ���۸� ����� ��� ��
	//D3DClass ���� �����ߴ� ���� ������ �����ϴ�.
	//Description �ۼ��ϰ� ������ �����ϰ� Create �Ͽ� ���� ���ۿ� �����ϴ� ����

	//���� ���� description �ۼ�
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	//������ ũ��� �翬 VertexType ũ�⿡ vertex ������ŭ ������ ���� �� ���̴�.
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	//���� Flags�� �� ������ �ǹ��Ѵ�. ��������. ���� ������. �� ������ Ÿ���� vertexBuffer ���� ����� ���̹Ƿ� �����ش�.
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//CPU ������� ���� (GPU����)
	vertexBufferDesc.CPUAccessFlags = 0;
	//��Ÿ���� ����
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	//���� �����͸� ����Ű�� ���� ���ҽ� ����ü�� �ۼ�
	//�ڼ��Ѱ� ���� �𸣰ڴ�.
	//�̰� �Ƹ� vertex ���ҽ��� ���� �޸𸮸� �����ϴ°��ϰŰ�..
	vertexData.pSysMem = vertices;
	//�Ʒ� �ΰ��� ���� �𸣰ڴ�..
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	//���� ���۸� ����
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	//�ε��� ���۵� �Ȱ��� ���ش�
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	//���� ����(stride)�� ������ ����Ʈ�� ������ ���ΰ�? -> 0�� �ϴ� �ƴ϶�� �ǹ̴ϱ�..
	//�ڼ����� �𸣰����� �̷� �ǹ��� ���̴�
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = indices;

	//5��. ???
	/*indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;*/

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	//�پ��� ����
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

//������� ���� ����
void ModelClass::ShutdownBuffers()
{
	//�� ���� �Ҵ� ����
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


//�켱, ������ �������۴� ���� ���ۿ� �ε��� ���۷� ȭ�� ������ �׸��� ���� �ʿ��ϴ�.
//��Ȯ�ϰԴ� ���� ���ۿ� �ε��� ���۸� GPU�� ����� ���۷ν� Ȱ��ȭ��Ű��
//GPU�� �� Ȱ��ȭ�� ���۵��� �������ν� ���̴��� �̿��Ͽ� �� ���ۿ� ������ �׸� �� �ְ� �Ǵ°�.
//���⼭�� ���� ���� �����͸� �ﰢ���̳� ����, ��ä�� �� � ������� �׸��� ������ �� �ִ�.
void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// ���� ������ ������ �������� ����
	//Vertex ũ�Ⱑ �����̹Ƿ� �����͸� ���� �� �̸�ŭ�� ��� �о�� �°���?
	stride = sizeof(VertexType);
	//�ϴ� ������(������ �и�)�� ����.
	offset = 0;

	//input assembler �� ���� ���۸� Ȱ��ȭ�Ͽ� �׷��� �� �ֵ��� ��
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	//�̹��� �ε��� ����
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//���� ���۷� �׸� �⺻ ���� -> ���⼭�� �ϴ� �ﰢ�� (���� ��� ������Ʈ�� �ﰢ������ �̷����)
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

//5��. �Ʊ� ������ �ҷ��Դ� ���� �̸��� ������ �ؽ�ó ��ü�� ������ִ� �۾�
bool ModelClass::LoadTexture(ID3D11Device* device, WCHAR* filename)
{
	bool result;

	//Texture Object ����
	m_Texture = new TextureClass;
	if (!m_Texture)
	{
		return false;
	}

	//Texture �ҷ����� (���ϸ�����)
	result = m_Texture->Initialize(device, filename);
	if (!result)
	{
		return false;
	}

	return true;
}

void ModelClass::ReleaseTexture()
{
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}

	return;
}