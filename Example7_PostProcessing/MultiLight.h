#pragma once
#include "DXF.h"

using namespace std;
using namespace DirectX;

class MultiLight : public BaseShader
{
private:
	struct LightBufferType
	{
		XMFLOAT4 ambientColour[4];
		XMFLOAT4 diffuseColour[4];
		XMFLOAT4 lightPos[4];
	};

public:
	MultiLight(ID3D11Device* device, HWND hwnd);
	~MultiLight();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView*depthTexture, Light* light1, Light* light2, Light* light3, Light* light4);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer * matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* shadowState;
	ID3D11Buffer* lightBuffer;
};

