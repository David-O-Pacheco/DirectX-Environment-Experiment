#include "Header.hlsli"

// Pixel colour data and sampler
Texture2D imageData : register(t1);
SamplerState colourSampler : register(s0);

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(VS_OUTPUT input) : SV_Target
{
	return imageData.Sample(colourSampler, input.texCoord);
//return input.Color; // Return the colour associated with the vertex
}