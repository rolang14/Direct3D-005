#pragma once

//ColorShaderClass -> GPU 상에 존재하는 3D 모델을 그리기 위해 사용하는 HLSL 셰이더를 호출하기 위한 클래스

#ifndef _COLORSHADERCLASS_H_
#define _COLORSHADERCLASS_H_

#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>

using namespace std;

class ColorShaderClass
{
private:
	//여기서 다시 선언해주는 이유는, 정점 셰이더에 사용될 cBuffer 타입을 정의해야하므로
	//반드시 Color.vs 에서 선언한 cBuffer 와 동일한 구조로 구조체를 선언해 준다.
	struct MatrixBufferType
	{
		//3개 행렬 구조체 선언
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

public:
	ColorShaderClass();
	ColorShaderClass(const ColorShaderClass&);
	~ColorShaderClass();
	//셰이더의 초기화와 마무리를 제어함.
	//Render 는 셰이더에 사용되는 변수들을 설정하고 셰이더를 이용해 준비된 모델의 정점들을 그려 냄
	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
};

#endif // !_COLORSHADERCLASS_H_
