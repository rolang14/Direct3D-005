#pragma once

#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_

#include <d3d11.h>
#include <D3DX11tex.h>

//TextureClass 에서는 텍스처 자원을 불러오고, 해제하고, 접근하는 작업을 캡슐화 한다.
//모든 텍스처에 대해서 각각 이 클래스가 다 만들어져있어야 함.
class TextureClass
{
public:
	TextureClass();
	TextureClass(const TextureClass&);
	~TextureClass();

	//주어진 파일 이름으로 텍스처를 불러옴
	bool Initialize(ID3D11Device*, WCHAR*);
	//다른 클래스들 처럼 안쓰는거 삭제/해제하는 함수
	void Shutdown();

	//셰이더가 텍스처를 그리는 작업에 사용할 수 있도록 텍스처 자원의 포인터를 반환
	ID3D11ShaderResourceView* GetTexture();

private:
	//실제 텍스처 자원
	ID3D11ShaderResourceView* m_texture;
};

#endif