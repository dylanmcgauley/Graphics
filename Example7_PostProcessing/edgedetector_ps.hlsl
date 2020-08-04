Texture2D skinTexture : register(t0);
SamplerState sampler0 : register(s0);
SamplerState sampler1 : register(s1);

cbuffer EdgeBuffer : register(b0)
{
    float screenHeight;
    float screenWidth;
    float thickness;
    float lowerTol;
    float upperTol;
    float3 padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

float CheckEdge(float lowerVal, float upperVal, float3 average)
{
    // checks to see if the average is lower than the the set lower tollerance
    if (average.x < lowerVal || average.y < lowerVal || average.z < lowerVal)
    {
        return 0;
    }
    // checks to see if the average is greater than the the set upper tollerance
    if (average.x > upperVal || average.y > upperVal || average.z > upperVal)
    {
        return 1;
    }
    return average;
}

float4 main(InputType input) : SV_TARGET
{
    float4 skinColour;
    float4 pixels[4];
    float4 finalColour = (0, 0, 0, 1);
    float l = 0;

    // set the offset for where the edge will be
    float2 pixelOffset = thickness / float2(screenWidth, screenHeight);

   // sample current pixel
    skinColour = skinTexture.Sample(sampler0, input.tex);

    // find average value of current pixel
    float skinAverage = (skinColour.x + skinColour.y + skinColour.z) / 3;

    // sample surrounding pixels
    for (int x = -1; x < 2; x += 2)
    {
        for (int y = -1; y < 2; y += 2)
        {
            pixels[l] = skinTexture.Sample(sampler1, input.tex + float2(x, y) * pixelOffset);
            l++;
        }
    }
    
    // find average of surrounding pixels R G B values
    float3 average;
    average.x = (abs(pixels[0].x - pixels[3].x) + abs(pixels[1].x - pixels[2]).x);
    average.y = (abs(pixels[0].y - pixels[3].y) + abs(pixels[1].y - pixels[2]).y);
    average.z = (abs(pixels[0].z - pixels[3].z) + abs(pixels[1].z - pixels[2]).z);

    // check for an edge and set colour values acordingly 
    skinColour *= 1 - CheckEdge(skinAverage - lowerTol, skinAverage + upperTol, average);


    // return the final colour
    return skinColour;

}
