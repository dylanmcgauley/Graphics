Texture2D skinTexture : register(t0);
SamplerState sampler0 : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

float4 main(InputType input) : SV_TARGET
{
    // variable delerations
    float4 finalColour = (0, 0, 0, 0);
    float2 blurCoords = (0, 0);
    int samples = 15;
    float blurLength = 2.5f;

    float2 pixelOffset = input.tex - float2(0.5, 0.5);
    float r = length(pixelOffset);
    float theta = atan2(pixelOffset.y, pixelOffset.x);

    for (float i = 0; i < samples; i++)
    {
        float tapTheta = theta + i * (blurLength / samples);
        float tapR = r * i; 

        blurCoords.x = tapR * cos(tapTheta) + 0.5;
        blurCoords.y = tapR * sin(tapTheta) + 0.5;

        finalColour += skinTexture.Sample(sampler0, blurCoords);
    }
    finalColour /= samples;
    return finalColour;
}