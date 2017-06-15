Texture2D texDiffuse : register(t0);
Texture2D texNormal : register(t1);
Texture2D texSpecular : register(t2);
SamplerState texSampler : register(s0);

cbuffer LightBuffer : register(b0)
{
	float4 lightColor;
	float3 lightPosition;
	float luminosity;
	float3 lightDirection;
	float padding;
	float3 cameraPos;
	float falloff;
};

cbuffer PhongBuffer : register(b1)
{
	float3 Ka;
	float padding1;
	float3 Kd;
	float padding2;
	float3 Ks;
	float specularPower;
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

float4 main(PSIn input) : SV_TARGET
{
	float4 colorKa, colorKd, colorKs, tex;
	float3 N, B, T, l, V, R, bumpMap, bumpNormal, lightDir;
	float dotLN, dotRV;

	N = input.normal;
	B = input.binormal;
	T = input.tangent;

	colorKa = float4(0, 0, 0, 1);
	colorKd = float4(0, 0, 0, 1);
	colorKs = float4(0, 0, 0, 1);
	//input.texCoord.x = 1 - input.texCoord.x;
	//input.texCoord.y = 1 - input.texCoord.y;
	//input.texCoord.x *= 0.93f;
	//input.texCoord.x -= 0.01f;
	tex = texDiffuse.Sample(texSampler, input.texCoord);

	bumpMap = texNormal.Sample(texSampler, input.texCoord);
	bumpMap = (bumpMap * 2.0f) - 1.0f;

	bumpNormal = (bumpMap.x * N) + (bumpMap.y * T) + (bumpMap.z * B);
	bumpNormal = normalize(bumpNormal);

	//return float4(bumpNormal, 1.0f);

	//Ambient
	if (tex.w > 0)
	{
		colorKa = tex * 0.1f;
	}
	else
	{
		colorKa = float4(Ka, 1.0f);
	}

	//return colorKa;

	//Diffuse

	lightDir = -lightDirection;
	lightDir = normalize(lightDir);
	
	dotLN = dot(bumpNormal, lightDir);
	if (dotLN > 0)
	{
		colorKd = texDiffuse.Sample(texSampler, input.texCoord);
		if (colorKd.w > 0)
		{
			colorKd *= float4(dotLN, dotLN, dotLN, 1.0f);
		}
		else
		{
			dotLN = dot(N, lightDir);
			colorKd = float4(dotLN, dotLN, dotLN, 1.0f) * float4(Kd, 1.0f);
		}
	}

	//Specular
	V = cameraPos - input.worldPos.xyz;
	
	V = normalize(V);
	
	l = input.worldPos.xyz - lightPosition;
	l = normalize(l);
	R = reflect(l, bumpNormal);
	
	dotRV = dot(R, V);

	if (dotRV > 0)
	{
		colorKs = texSpecular.Sample(texSampler, input.texCoord);
		if (colorKs.w > 0)
		{
			colorKs = saturate(colorKs * pow(dotRV, 100));
		}
		else
		{
			colorKs = float4(saturate(Ks * pow(dotRV, 100)), 1.0f);
		}
	}

	return colorKa +colorKd + colorKs;
}