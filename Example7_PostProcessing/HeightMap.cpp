#include "HeightMap.h"

HeightMap::HeightMap(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"heightmap_vs.cso", L"heightmap_ps.cso");
}


HeightMap::~HeightMap()
{
	// Release sampler
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release matrix buffer
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release layout
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release light buffer
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}
	
	// Release value buffer
	if (valueBuffer)
	{
		valueBuffer->Release();
		valueBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void HeightMap::initShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC valueBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	valueBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	valueBufferDesc.ByteWidth = sizeof(ValueBufferType);
	valueBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	valueBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	valueBufferDesc.MiscFlags = 0;
	valueBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&valueBufferDesc, NULL, &valueBuffer);
}


void HeightMap::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* heightTexture, ID3D11ShaderResourceView* skinTexture, float manipValue, Light* light1, Light* light2, Light* light3, Light* light4)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* lightPtr;
	ValueBufferType* valuePtr;

	XMMATRIX tworld, tview, tproj;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);


	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	

	//Additional
	// Send light data to pixel shader
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambientColour[0] = light1->getAmbientColour();
	lightPtr->ambientColour[1] = light2->getAmbientColour();
	lightPtr->ambientColour[2] = light3->getAmbientColour();
	lightPtr->ambientColour[3] = light4->getAmbientColour();
	lightPtr->diffuseColour[0] = light1->getDiffuseColour();
	lightPtr->diffuseColour[1] = light2->getDiffuseColour();
	lightPtr->diffuseColour[2] = light3->getDiffuseColour();
	lightPtr->diffuseColour[3] = light4->getDiffuseColour();
	lightPtr->lightPos[0] = XMFLOAT4(light1->getPosition().x, light1->getPosition().y, light1->getPosition().z, 1.0f);
	lightPtr->lightPos[1] = XMFLOAT4(light2->getPosition().x, light2->getPosition().y, light2->getPosition().z, 1.0f);
	lightPtr->lightPos[2] = XMFLOAT4(light3->getPosition().x, light3->getPosition().y, light3->getPosition().z, 1.0f);
	lightPtr->lightPos[3] = XMFLOAT4(light4->getPosition().x, light4->getPosition().y, light4->getPosition().z, 1.0f);
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &skinTexture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	
	// send manipulation value to vertex shader
	deviceContext->Map(valueBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	valuePtr = (ValueBufferType*)mappedResource.pData;
	valuePtr->heightValue = manipValue;
	valuePtr->padding = XMFLOAT3(1, 1, 1);
	deviceContext->Unmap(valueBuffer, 0);

	// set buffers
	deviceContext->VSSetConstantBuffers(1, 1, &valueBuffer);
	deviceContext->VSSetShaderResources(0, 1, &heightTexture);
	deviceContext->VSSetSamplers(0, 1, &sampleState);
}
