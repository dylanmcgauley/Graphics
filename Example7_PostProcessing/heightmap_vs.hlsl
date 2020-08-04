Texture2D heightTexture : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer ValueBuffer : register(b1)
{
    float manipulationValue;
    float3 padding;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
};

OutputType main(InputType input)
{
    OutputType output;
    float current;
    // height values for surrounding pixels
    float n, s, e, w;
    float3 vertex1, vertex2, vertex3, vertex4, vertex5, vector1, vector2, vector3, vector4;

    // sample for current vertice
    current = heightTexture.SampleLevel(sampler0, input.tex, 0);

    // calculate height factor using sampled level
    float height = current * (5 * manipulationValue);
    
    // move vertices based on heighmap sample levels
    input.position.y += height;

    // sample for surrounding vertices
    n = heightTexture.SampleLevel(sampler0, input.tex + float2(0, 1), 0);
    s = heightTexture.SampleLevel(sampler0, input.tex + float2(0, -1), 0);
    e = heightTexture.SampleLevel(sampler0, input.tex + float2(1, 0), 0);
    w = heightTexture.SampleLevel(sampler0, input.tex + float2(-1, 0), 0);

    // Normal Calculation
    // current vertex
    vertex1.x = input.position.x;
    vertex1.y = current * (5 * manipulationValue);
    vertex1.z = input.position.z;

    // east vertex
    vertex2.x = input.position.x - 1;
    vertex2.y = e * (5 * manipulationValue);
    vertex2.z = input.position.z;

    // north vertex
    vertex3.x = input.position.x;
    vertex3.y = n * (5 * manipulationValue);
    vertex3.z = input.position.z + 1;

    // south vertex
    vertex4.x = input.position.x;
    vertex4.y = s * (5 * manipulationValue);
    vertex4.z = input.position.z - 1;

    // west vertex
    vertex5.x = input.position.x + 1;
    vertex5.y = w * (5 * manipulationValue);
    vertex5.z = input.position.z;
    
    // tangents of normal
    //float3 horizontalT = (2, vertex2.y - vertex5.y, 0);
    //float3 verticalT = (0, vertex4.y - vertex3.y, 2);

    //output.normal = normalize(cross(verticalT, horizontalT));

    // west - > current
    vector1 = vertex1 - vertex5;

    // current - > east
    vector2 = vertex2 - vertex1;

    // north - > current
    vector3 = vertex1 - vertex3;

    // current - > south
    vector4 = vertex4 - vertex1;

    //add the
    //cross products
    //then normalize
    //for final normals
    output.normal = normalize(cross(vector3, vector1) + cross(vector3, vector2) + cross(vector4, vector1) + cross(vector4, vector2));
 
	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;
	
	// Calculate the normal vector against the world matrix only and normalise.
    //output.normal = mul(output.normal, (float3x3) worldMatrix);
    //output.normal = normalize(output.normal);
    output.worldPos = mul(input.position, worldMatrix).xyz;

    return output;
}