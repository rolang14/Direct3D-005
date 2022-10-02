#pragma once

#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include <d3d11.h>
#include <d3dx10math.h>

//5��. TextureShader �߰�
#include "TextureClass.h"

//ModelClass �� 3D �𵨵��� ������ �����е��� ĸ��ȭ�ϴ� Ŭ�����̴�.
//������ ��� �ﰢ���� ����� ���� �����͸� �������� ����� �� ���̴�.
//�ﰢ���� �׷����� ���ؼ��� VS,PS �� ���� �̵��� ����� �ε��� ����, ���� ���� ���� �ʿ��ϹǷ�
class ModelClass
{
private:
	//���� ���ۿ� ���� ������ ����ü (VertexType)
	//ColorShaderClass���� ����� �Ͱ� �� ������ �翬�� ���ƾ� �Ѵ�.
	struct VertexType
	{
		D3DXVECTOR3 position;
		//5��. Color �� Texture �� ��ü�Ѵ�.
		//D3DXVECTOR4 color;
		D3DXVECTOR2 texture;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();
	//������� ����, �Ҹ�

	//������ʹ� ���� �Լ���
	//5��. �ؽ�ó ��ο��� �ؽ�ó�� ��������� �ϱ� ���ؼ� Initialize �Լ��� Texture ��α��� �־��ش�.
	bool Initialize(ID3D11Device*, WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	int GetIndexCount();
	//5��. ���������� ���̴����� �ڽ��� �ؽ�ó �ڿ��� �����ϰ� �׸��� ���� GetTexture �Լ��� ���� �־�� �Ѵ�.
	//����� �׳� Texture Class ���� ������� �Լ� �ε��� ���̴�
	ID3D11ShaderResourceView* GetTexture();

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	//5��. ���� �׸��µ� ����� �ؽ�ó�� �ҷ����� ��ȯ�ϴµ� ����� �Լ���.
	bool LoadTexture(ID3D11Device*, WCHAR*);
	void ReleaseTexture();


private:
	//���� ���� ���ۿ� �ε��� ���۸� ������ش�. (�����ͷ� �Ǿ��ִ°� ���� ��...���� �޼ҵ尡 �Ҵ����ٰ��̴�)
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	int m_vertexCount, m_indexCount;
	//5��. �ؽ�ó ����
	TextureClass* m_Texture;
};

#endif