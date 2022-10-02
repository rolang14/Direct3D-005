#pragma once

#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include <d3d11.h>
#include <d3dx10math.h>

//5강. TextureShader 추가
#include "TextureClass.h"

//ModelClass 는 3D 모델들의 복합한 기하학들을 캡슐화하는 클래스이다.
//지금은 녹색 삼각형을 만들기 위한 데이터를 차근차근 만들어 볼 것이다.
//삼각형이 그려지기 위해서는 VS,PS 에 더해 이들이 사용할 인덱스 버퍼, 정점 버퍼 역시 필요하므로
class ModelClass
{
private:
	//정점 버퍼에 사용될 정점의 구조체 (VertexType)
	//ColorShaderClass에서 사용할 것과 그 구조가 당연히 같아야 한다.
	struct VertexType
	{
		D3DXVECTOR3 position;
		//5강. Color 를 Texture 로 대체한다.
		//D3DXVECTOR4 color;
		D3DXVECTOR2 texture;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();
	//여기까지 생성, 소멸

	//여기부터는 구현 함수들
	//5강. 텍스처 경로에서 텍스처를 갖고오도록 하기 위해서 Initialize 함수에 Texture 경로까지 넣어준다.
	bool Initialize(ID3D11Device*, WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	int GetIndexCount();
	//5강. 마찬가지로 셰이더에게 자신의 텍스처 자원을 전달하고 그리기 위한 GetTexture 함수를 갖고 있어야 한다.
	//참고로 그냥 Texture Class 에서 만들었던 함수 로딩할 뿐이다
	ID3D11ShaderResourceView* GetTexture();

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	//5강. 모델을 그리는데 사용할 텍스처를 불러오고 반환하는데 사용할 함수들.
	bool LoadTexture(ID3D11Device*, WCHAR*);
	void ReleaseTexture();


private:
	//각각 정점 버퍼와 인덱스 버퍼를 만들어준다. (포인터로 되어있는걸 보아 또...뭔가 메소드가 할당해줄것이다)
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	int m_vertexCount, m_indexCount;
	//5강. 텍스처 변수
	TextureClass* m_Texture;
};

#endif