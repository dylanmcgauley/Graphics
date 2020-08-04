Texture2D skinTexture : register(t0);
SamplerState sampler0 : register(s0);

cbuffer ToonBuffer : register(b0)
{
    float cutoff1;
    float cutoff2;
    float cutoff3;
    float cutoff4;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

float4 main(InputType input) : SV_TARGET
{
    float4 skinColour;

    // sample current pixel
    skinColour = skinTexture.Sample(sampler0, input.tex);

    // find new R value
    if (skinColour.x > cutoff1)
        skinColour.x = 1;
    else if (skinColour.x > cutoff2)
        skinColour.x = 0.75;
    else if (skinColour.x > cutoff3)
        skinColour.x = 0.5;
    else if (skinColour.x > cutoff4)
        skinColour.x = 0.35;
    else
        skinColour.x = 0.1;

    // find new G value
    if (skinColour.y > cutoff1)
        skinColour.y = 1;
    else if (skinColour.y > cutoff2)
        skinColour.y = 0.75;
    else if (skinColour.y > cutoff3)
        skinColour.y = 0.5;
    else if (skinColour.y > cutoff4)
        skinColour.y = 0.35;
    else
        skinColour.y = 0.1;

    // find new B value
    if (skinColour.z > cutoff1)
        skinColour.z = 1;
    else if (skinColour.z > cutoff2)
        skinColour.z = 0.75;
    else if (skinColour.z > cutoff3)
        skinColour.z = 0.5;
    else if (skinColour.z > cutoff4)
        skinColour.z = 0.35;
    else
        skinColour.z = 0.1;

    // return the final colour
    return skinColour;
}
