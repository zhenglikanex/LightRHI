cbuffer cbConstans : register(b0)
{
	float4 gColor; 
};

cbuffer cbConstansView : register(b1)
{
	float4 gColor2;
};

struct VertexIn
{
	float3 PosL  : POSITION;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vout.PosH = float4(vin.PosL, 1.0f);
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return gColor2;
}
