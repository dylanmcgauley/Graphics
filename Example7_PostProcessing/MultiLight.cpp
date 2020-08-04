#include "MultiLight.h"

MultiLight::MultiLight(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"multilight_vs.cso", L"multilight_ps.cso");
}


MultiLight::~MultiLight()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void MultiLight::initShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_SAMPLER_DESC shadowDesc;
	D3D11_BUFFER_DESC lightBufferDesc;

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

	// Sampler for shadow map sampling.
	shadowDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	shadowDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	shadowDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	shadowDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	shadowDesc.MipLODBias = 0.0f;
	shadowDesc.MaxAnisotropy = 1;
	shadowDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	shadowDesc.MinLOD = 0;
	shadowDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&shadowDesc, &shadowState);

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
}


void MultiLight::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView*depthTexture, Light* light1, Light* light2, Light* light3, Light* light4)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	
	XMMATRIX tworld, tview, tproj, tLightView, tLightProj;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	tLightView = XMMatrixTranspose(light2->getViewMatrix());
	tLightProj = XMMatrixTranspose(light2->getOrthoMatrix());
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	dataPtr->lightView = tLightView;
	dataPtr->lightProjection = tLightProj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	//Additional
	// Send light data to pixel shader
	LightBufferType* lightPtr;
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
	lightPtr->lightPos[0] =XMFLOAT4( light1->getPosition().x, light1->getPosition().y, light1->getPosition().z, 1.0f);
	lightPtr->lightPos[1] =XMFLOAT4( light2->getPosition().x, light2->getPosition().y, light2->getPosition().z, 1.0f);
	lightPtr->lightPos[2] =XMFLOAT4( light3->getPosition().x, light3->getPosition().y, light3->getPosition().z, 1.0f);
	lightPtr->lightPos[3] =XMFLOAT4( light4->getPosition().x, light4->getPosition().y, light4->getPosition().z, 1.0f);
	//lightPtr->padding[0] = 1.0f;
	//lightPtr->padding[1] = 1.0f;
	//lightPtr->padding[2] = 1.0f;
	//lightPtr->padding[3] = 1.0f;
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &depthTexture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &shadowState);
}
