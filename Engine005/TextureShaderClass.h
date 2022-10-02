#pragma once

//TextureShaderClass -> ColorShaderClass 의 개정판 같은 느낌으로 구현 (Color->Texture 타입 바뀐것 말고는 거의 동일하다고 보면 된다)
//이전과 다르게

#ifndef _TEXTURESHADERCLASS_H_
#define _TEXTURESHADERCLASS_H_

#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>

using namespace std;

class TextureShaderClass
{
private:
	//텍스처 셰이더 사용을 위해 똑같은 타입 선언
	struct MatrixBufferType
	{
		//3개 행렬 구조체 선언
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

public:
	TextureShaderClass();
	TextureShaderClass(const TextureShaderClass&);
	~TextureShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	//이번엔 렌더할때 텍스처 자원을 추가한다.
	bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;

	//샘플러 상태의 포인터에 해당하는 새로운 변수 추가됨
	//텍스쳐 셰이더와의 인터페이스로서 사용할 것임
	ID3D11SamplerState* m_sampleState;
};

#endif // !_TEXTURESHADERCLASS_H_
