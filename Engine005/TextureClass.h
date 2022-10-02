#pragma once

#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_

#include <d3d11.h>
#include <D3DX11tex.h>

//TextureClass ������ �ؽ�ó �ڿ��� �ҷ�����, �����ϰ�, �����ϴ� �۾��� ĸ��ȭ �Ѵ�.
//��� �ؽ�ó�� ���ؼ� ���� �� Ŭ������ �� ��������־�� ��.
class TextureClass
{
public:
	TextureClass();
	TextureClass(const TextureClass&);
	~TextureClass();

	//�־��� ���� �̸����� �ؽ�ó�� �ҷ���
	bool Initialize(ID3D11Device*, WCHAR*);
	//�ٸ� Ŭ������ ó�� �Ⱦ��°� ����/�����ϴ� �Լ�
	void Shutdown();

	//���̴��� �ؽ�ó�� �׸��� �۾��� ����� �� �ֵ��� �ؽ�ó �ڿ��� �����͸� ��ȯ
	ID3D11ShaderResourceView* GetTexture();

private:
	//���� �ؽ�ó �ڿ�
	ID3D11ShaderResourceView* m_texture;
};

#endif