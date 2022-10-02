/////////////////////////
// FileName : Texture.vs
////////////////////////

//�⺻������ ������Ʈ����
//�׳� ������ �÷��� ���� ������ (Default Diffuse)
//�ƴϸ� �ؽ�ó�� ���� ������ (Textured Shader)
//���� �ϳ��̴�..
//�׷��Ƿ� �÷� ���̴��� ����������
//�ؽ�ó ���� ������, �ȼ� ���̴��� ��� �����Ѵ�.
//���� �������� �ؽ�ó�� UV�� ����ϴ°�, �ؽ�ó ����⸦ ����ϴ°͸� �����ϸ�
//��κ��� �÷� ���̴��� ����ϴ�.

////////////////////////
// GLOBALS
////////////////////////

cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

////////////////////////
// TYPEDEFS
////////////////////////

//�ؽ�ó ���� ���̴��� ���� ��ǲŸ�� ����
//�ؽ�ó�� Z���� ����, (�̹����̹Ƿ�) U, V ��ǥ�� �����Ƿ� float2 ���� ����Ѵ�.
//�ؽ�ó�� ��ǥ�� ǥ���ϱ� ���ؼ� TEXCOORD0 ��°��� �����.
//�ؽ�ó�� ��ǥ��°�, �ؽ�ó�� ����� ��ǥ �� ��ü�� �ƴ� �ؽ�ó ��ü�� �̵���Ű�°��� �ǹ��Ѵ�

struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

//�ȼ� ���̴��� ���� ��ǲŸ�� ����
struct PixelInputType
{
	//���� ���̴��� �����ǰ� ������ �ֱ� ���Ͽ� SV_POSITION ����Ͽ���.
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};


////////////////////////
// Vertex Shader
////////////////////////

PixelInputType TextureVertexShader(VertexInputType input)
{
	PixelInputType output;

	//�ùٸ� ��� ������ �ϱ� ���� position ���͸� w���� �ִ� 4�������� ����Ѵ�..
	input.position.w = 1.0f;

	//������ ��ġ�� ���� ����, ��, ������ ������ ����Ѵ�
	//���⼭ mul�� ��Ŀ� ���� ������
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	//�÷� ���� ���̴��� ���⿡ color ���� �Ѱ��־��µ�,
	//�ؽ�ó ���� ���̴������� �÷��� �ƴ϶� �ؽ�ó�� �����ϵ��� �Ѵ�.
	//���� ���̴���°� �� �״�� ���� (Vertex) �� ĥ���ִ�... ���̴���..
	output.tex = input.tex;

	return output;
}