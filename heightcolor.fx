uniform extern float4x4 gWVP;
uniform extern float gTime;

struct OutputVS
{
	float4 posH : POSITION0;
	float4 color : COLOR;
};

static float a[2] = { 0.8f, 0.2f };
static float k[2] = { 1.0f, 8.0f };
static float w[2] = { 1.0f, 8.0f };
static float p[2] = { 0.0f, 1.0f };

float SumOfRadialSineWaves(float x, float z)
{
	float d = sqrt(x * x + z * z);
	float sum = 0.0f;
	for (int i = 0; i < 2; ++i)
		sum += a[i] * sin(k[i] * d - gTime * w[i] + p[i]);
	return sum;
}

float SumOfDirectionalSineWaves(float x, float z)
{
	float d = x + z;
	float sum = 0.0f;
	for (int i = 0; i < 2; ++i)
		sum += a[i] * sin(k[i] * d - gTime * w[i] + p[i]);
	return sum;
}

float4 GetColorFromHeight(float y)
{
	if (abs(y) <= 0.2f)
		return float4(0.0f, 0.0f, 0.0f, 1.0f);
	else if (abs(y) <= 0.5f)
		return float4(0.0f, 0.0f, 1.0f, 1.0f);
	else if (abs(y) <= 0.8f)
		return float4(0.0f, 1.0f, 0.0f, 1.0f);
	else if (abs(y) <= 1.0f)
		return float4(1.0f, 0.0f, 0.0f, 1.0f);
	else
		return float4(1.0f, 1.0f, 0.0f, 1.0f);
}

OutputVS ColorVS(float3 posL : POSITION0)
{
	OutputVS outVS = (OutputVS)0;
	posL.y = SumOfDirectionalSineWaves(posL.x, posL.z);
	outVS.color = GetColorFromHeight(posL.y);
	outVS.posH = mul(float4(posL, 1.0f), gWVP);
	
	return outVS;
}

float4 ColorPS(float4 c : COLOR0) : COLOR
{
	return c;
}

technique HeightColorTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 ColorVS();
		pixelShader = compile ps_2_0 ColorPS();

		FillMode = WireFrame;
	}
}