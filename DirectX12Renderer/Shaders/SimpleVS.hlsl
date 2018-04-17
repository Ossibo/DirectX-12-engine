struct VSIn
{
	float4 Position : POSITION;
	float3 normal : NORMAL;
	float3 binormal : BINORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
};

struct PSIn
{
	float4 Position : SV_Position;
	float3 normal : NORMAL;
	float3 binormal : BINORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD0;
	float4 worldPos : WPOS;
};

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};
PSIn main(VSIn input)
{

	PSIn output;

	// model-to-view
	matrix MV = mul(viewMatrix, worldMatrix);

	// model-to-projection
	matrix MVP = mul(projectionMatrix, MV);
	
	output.Position = mul(MVP, input.Position);
	output.texCoord = input.texCoord;
	output.worldPos = mul(worldMatrix, input.Position);

	output.normal = mul(worldMatrix, input.normal);
	output.normal = normalize(output.normal);
	output.binormal = mul(worldMatrix, input.binormal);
	output.binormal = normalize(output.binormal);
	output.tangent = mul(worldMatrix, input.tangent);
	output.tangent = normalize(output.tangent);

	return output;
}