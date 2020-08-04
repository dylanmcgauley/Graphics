#include "ToonShader.h"

ToonShader::ToonShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"toon_vs.cso", L"toon_ps.cso");
}

ToonShader::~ToonShader()
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

	// Release Toon Buffer
	if (toonBuffer)
	{
		toonBuffer->Release();
		toonBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void ToonShader::setShaderParameters(ID3D11DeviceContext * deviceContext, const XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, ID3D11ShaderResourceView * texture, float cutoff1, float cutoff2, float cutoff3, float cutoff4)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	ToonBufferType* toonPtr;
	LightBufferType* lightPtr;

	XMMATRIX tworld, tview, tproj;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(world);
	tview = XMMatrixTranspose(view);
	tproj = XMMatrixTranspose(projection);
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->Map(toonBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	toonPtr = (ToonBufferType*)mappedResource.pData;
	toonPtr->cutoff1 = cutoff1;
	toonPtr->cutoff2 = cutoff2;
	toonPtr->cutoff3 = cutoff3;
	toonPtr->cutoff4 = cutoff4;
	deviceContext->Unmap(toonBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &toonBuffer);
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}

void ToonShader::initShader(WCHAR * vsFilename, WCHAR * psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC toonDesc;
	D3D11_SAMPLER_DESC samplerDesc;

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

	// setup the description of the edge buffer
	toonDesc.Usage = D3D11_USAGE_DYNAMIC;
	toonDesc.ByteWidth = sizeof(ToonBufferType);
	toonDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	toonDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	toonDesc.MiscFlags = 0;
	toonDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&toonDesc, NULL, &toonBuffer);

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
}

