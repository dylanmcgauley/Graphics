#include "EdgeDetector.h"



EdgeDetector::EdgeDetector(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"edgedetector_vs.cso", L"edgedetector_ps.cso");
}

EdgeDetector::~EdgeDetector()
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

	if (edgeBuffer)
	{
		edgeBuffer->Release();
		edgeBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the edge state.
	if (edgeState)
	{
		edgeState->Release();
		edgeState = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void EdgeDetector::setShaderParameters(ID3D11DeviceContext * deviceContext, const XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, ID3D11ShaderResourceView * texture, float width, float height, float thickness, float lowerTol, float upperTol)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	EdgeBufferType* edgePtr;

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
	deviceContext->Map(edgeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	edgePtr = (EdgeBufferType*)mappedResource.pData;
	edgePtr->screenHeight = height;
	edgePtr->screenWidth = width;
	edgePtr->thickness = thickness;
	edgePtr->lowerTol = lowerTol;
	edgePtr->upperTol = upperTol;
	edgePtr->padding = XMFLOAT3(0, 0, 0);
	deviceContext->Unmap(edgeBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &edgeBuffer);
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &edgeState);
}

void EdgeDetector::initShader(WCHAR * vsFilename, WCHAR * psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC edgeDesc;
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
	edgeDesc.Usage = D3D11_USAGE_DYNAMIC;
	edgeDesc.ByteWidth = sizeof(EdgeBufferType);
	edgeDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	edgeDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	edgeDesc.MiscFlags = 0;
	edgeDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&edgeDesc, NULL, &edgeBuffer);

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
	renderer->CreateSamplerState(&samplerDesc, &edgeState);
}
