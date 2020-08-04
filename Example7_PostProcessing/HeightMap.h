#pragma once
#include "DXF.h"

using namespace std;
using namespace DirectX;

class HeightMap : public BaseShader
{
private:
	struct LightBufferType
	{
		XMFLOAT4 ambientColour[4];
		XMFLOAT4 diffuseColour[4];
		XMFLOAT4 lightPos[4];
	};

	struct ValueBufferType
	{
		float heightValue;
		XMFLOAT3 padding;
	};

public:
	HeightMap(ID3D11Device* device, HWND hwnd);
	~HeightMap();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* heightTexture, ID3D11ShaderResourceView* skinTexture, float manipValue, Light* light1, Light* light2, Light* light3, Light* light4);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* valueBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* lightBuffer;
};

