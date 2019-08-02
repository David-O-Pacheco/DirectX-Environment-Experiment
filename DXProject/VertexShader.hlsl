#include "Header.hlsli"

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
}

VS_OUTPUT main(float4 Pos : POSITION, float2 texCoord : TEXCOORD0)
{
	VS_OUTPUT output = (VS_OUTPUT)0; // Create a new vertex to store transformed vertex + colour
	output.Pos = mul(Pos, World); // Transform vertex with respect to mesh world co-ords
	output.Pos = mul(output.Pos, View); // Now transform vertex with respect to the view
	output.Pos = mul(output.Pos, Projection); // Now transform with respect to field of view

	output.texCoord = texCoord; // uv coord for sampler

	return output;
}