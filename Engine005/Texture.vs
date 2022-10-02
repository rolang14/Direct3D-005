/////////////////////////
// FileName : Texture.vs
////////////////////////

//기본적으로 오브젝트들은
//그냥 정점에 컬러만 입힐 것인지 (Default Diffuse)
//아니면 텍스처를 입힐 것인지 (Textured Shader)
//둘중 하나이다..
//그러므로 컬러 셰이더와 마찬가지로
//텍스처 정점 셰이터, 픽셀 셰이더가 모두 존재한다.
//또한 작정법도 텍스처의 UV를 사용하는것, 텍스처 씌우기를 사용하는것만 제외하면
//대부분이 컬러 셰이더와 비슷하다.

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

//텍스처 정점 셰이더를 위한 인풋타입 정의
//텍스처는 Z축이 없고, (이미지이므로) U, V 좌표만 있으므로 float2 형을 사용한다.
//텍스처의 좌표를 표현하기 위해서 TEXCOORD0 라는것을 사용함.
//텍스처의 좌표라는건, 텍스처를 사용할 좌표 즉 물체가 아닌 텍스처 자체를 이동시키는것을 의미한다

struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

//픽셀 셰이더를 위한 인풋타입 정의
struct PixelInputType
{
	//정점 셰이더의 포지션과 구분을 주기 위하여 SV_POSITION 사용하였다.
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};


////////////////////////
// Vertex Shader
////////////////////////

PixelInputType TextureVertexShader(VertexInputType input)
{
	PixelInputType output;

	//올바른 행렬 연산을 하기 위해 position 벡터를 w까지 있는 4성분으로 사용한다..
	input.position.w = 1.0f;

	//정점의 위치를 각각 월드, 뷰, 직교의 순으로 계산한다
	//여기서 mul은 행렬에 대한 곱연산
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	//컬러 정점 셰이더는 여기에 color 값을 넘겨주었는데,
	//텍스처 정점 셰이더에서는 컬러가 아니라 텍스처를 전달하도록 한다.
	//정점 셰이더라는건 말 그대로 정점 (Vertex) 를 칠해주는... 셰이더다..
	output.tex = input.tex;

	return output;
}