struct Mtrl
{
	float4 ambient;
	float4 diffuse;
	float4 spec;
	float specPower;
};

struct DirLight
{
	float4 ambient;
	float4 diffuse;
	float4 spec;
	float3 dirW;
};

uniform extern float4x4 gWorldInv;
uniform extern float4x4 gWVP;
uniform extern Mtrl gMtrl;
uniform extern DirLight gLight;
uniform extern float3 gEyePosW;
uniform extern texture gTex;
uniform extern texture gNormalMap;

sampler TexS = sampler_state
{
	Texture = <gTex>;
	MinFilter = ANISOTROPIC;
	MaxAnisotropy = 8;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

sampler NormalMapS = sampler_state
{
	Texture = <gNormalMap>;
	MinFilter = ANISOTROPIC;
	MaxAnisotropy = 8;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct OutputVS
{
	float4 posH : POSITION0;
	float3 toEyeT : TEXCOORD0;
	float3 lightDirT : TEXCOORD1;
	float2 tex0 : TEXCOORD2;
};

OutputVS NormalMapVS(float3 posL : POSITION0,
	float3 tangentL : TANGENT0,
	float3 binormalL : BINORMAL0,
	float3 normalL : NORMAL0,
	float2 tex0 : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;
	float3x3 TBN;
	TBN[0] = tangentL;
	TBN[1] = binormalL;
	TBN[2] = normalL;

	float3x3 toTangentSpace = transpose(TBN);

	float3 eyePosL = mul(float4(gEyePosW, 1.0f), gWorldInv);

	float3 toEyeL = eyePosL - posL;
	outVS.toEyeT = mul(toEyeL, toTangentSpace);

	float3 lightDirL = mul(float4(gLight.dirW, 0.0f), gWorldInv).xyz;
	outVS.lightDirT = mul(lightDirL, toTangentSpace);
	outVS.posH = mul(float4(posL, 1.0f), gWVP);
	outVS.tex0 = tex0;

	return outVS;
}

float4 NormalMapPS(float3 toEyeT : TEXCOORD0,
	float3 lightDirT : TEXCOORD1,
	float2 tex0 : TEXCOORD2) : COLOR
{
	toEyeT = normalize(toEyeT);
	lightDirT = normalize(lightDirT);
	float3 lightVecT = -lightDirT;
	float3 normalT = tex2D(NormalMapS, tex0);
	normalT = 2.0f * normalT - 1.0f;
	normalT = normalize(normalT);

	float3 r = reflect(-lightVecT, normalT);
	float t = pow(max(dot(r, toEyeT), 0.0f), gMtrl.specPower);
	float s = max(dot(lightVecT, normalT), 0.0f);
	if (s <= 0.0f)
		t = 0.0f;
	float3 spec = t * (gMtrl.spec * gLight.spec).rgb;
	float3 diffuse = s * (gMtrl.diffuse * gLight.diffuse).rgb;
	float3 ambient = gMtrl.ambient * gLight.ambient;

	float4 texColor = tex2D(TexS, tex0);
	float3 color = (ambient + diffuse) * texColor.rgb + spec;

	return float4(color, gMtrl.diffuse.a * texColor.a);
}

technique NormalMapTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 NormalMapVS();
		pixelShader = compile ps_2_0 NormalMapPS();
	}
}
