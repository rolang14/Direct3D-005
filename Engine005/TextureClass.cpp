#include "TextureClass.h"

TextureClass::TextureClass()
{
	//������ �ʱ�ȭ
	m_texture = 0;
}

TextureClass::TextureClass(const TextureClass& other)
{

}

TextureClass::~TextureClass()
{

}

bool TextureClass::Initialize(ID3D11Device* device, WCHAR* filename)
{
	HRESULT result;

	//�ؽ�ó�� �ҷ��ͼ� ShaderResourceView �� �����
	result = D3DX11CreateShaderResourceViewFromFile(device, filename, NULL, NULL, &m_texture, NULL);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void TextureClass::Shutdown()
{
	if (m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}

	return;
}

ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_texture;
}

//������ ModelClass �� �ؽ�ó ���̴��� �߰����� �ϸ� ���Ҵ�.