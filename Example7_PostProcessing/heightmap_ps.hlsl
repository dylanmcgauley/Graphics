Texture2D skinTexture : register(t0);
SamplerState sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
    float4 ambientColour[4];
    float4 diffuseColour[4];
    float4 lightPos[4];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 main(InputType input) : SV_TARGET
{
    float lightIntensity[4];
    float4 skinColour;
    float4 lightColours[4];
    float4 combinedColours = 0;
    float3 lightVector[4];

    // attenuation values
    float constantF = 1.0f;
    float linearF = 0.125f;
    float quadraticF = 0.0f;
    float distance[4];
    float attenuation[4];

    for (int x = 0; x < 4; x++)
    {
        distance[x] = length(lightPos[x].xyz - input.worldPosition.xyz);
        attenuation[x] = 1 / (constantF + (linearF * distance[x]) + (quadraticF * pow(distance[x], 2)));
        lightVector[x] = normalize(lightPos[x].xyz - input.worldPosition.xyz);
        // calculate all light colours
        lightColours[x] = ambientColour[x] + calculateLighting(lightVector[x], input.normal, (diffuseColour[x] * attenuation[x]));
        // combine lights
        combinedColours += lightColours[x];
    }

	// sample the texture
    skinColour = skinTexture.Sample(sampler0, input.tex);

    //resultColour = saturate(combinedColours) * skinColour;
	
    // return true colour
    return combinedColours * skinColour;
}
