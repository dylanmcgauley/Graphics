Texture2D skinTexture : register(t0);
SamplerState sampler0 : register(s0);

Texture2D shadowTexture : register(t1);
SamplerState sampler1 : register(s1);

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
    float3 worldPos : TEXCOORD1;
    float4 lightViewPos : TEXCOORD2;
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
    float4 resultColour;
    float3 lightVector[4];

    float depthValue;
    float lightDepthValue;
    float shadowMapBias = 0.005f;

    // attenuation values
    float constantF = 1.0f;
    float linearF = 0.125f;
    float quadraticF = 0.0f;
    float distance[4];
    float attenuation[4];

    // Calculate projected coordinates.
    float2 texCoord = input.lightViewPos.xy / input.lightViewPos.w;
    texCoord *= float2(0.5, -0.5);
    texCoord += float2(0.5f, 0.5f);

    // sample the texture
    skinColour = skinTexture.Sample(sampler0, input.tex);
    // sample shadow texture
    depthValue = shadowTexture.Sample(sampler1, texCoord).r;

// Determine if the projected coordinates are in the 0 to 1 range.  If not don't do lighting.
    //if (texCoord.x < 0.f || texCoord.x > 1.f || texCoord.y < 0.f || texCoord.y > 1.f)
    //{
    //    return skinColour;
    //}

    // calculate depth
    lightDepthValue = input.lightViewPos.z / input.lightViewPos.w;
    lightDepthValue -= shadowMapBias;

// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    //if (lightDepthValue < depthValue)
    //{
        for (int x = 0; x < 4; x++)
        {
            distance[x] = length(lightPos[x].xyz - input.worldPos.xyz);
            attenuation[x] = 1 / (constantF + (linearF * distance[x]) + (quadraticF * pow(distance[x], 2)));
            lightVector[x] = normalize(lightPos[x].xyz - input.worldPos.xyz);
        // calculate all light colours
            lightColours[x] = ambientColour[x] + calculateLighting(lightVector[x], input.normal, (diffuseColour[x] * attenuation[x]));
        // combine lights
            combinedColours += lightColours[x];
        //}
    }

    //resultColour = saturate(combinedColours) * skinColour;
	
    // return true colour
    return combinedColours * skinColour;
}
