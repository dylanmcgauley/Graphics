#pragma once
#include "DXF.h"

using namespace std;
using namespace DirectX;

class ToonShader : public BaseShader
{
	struct ToonBufferType
	{
		float cutoff1;
		float cutoff2;
		float cutoff3;
		float cutoff4;
	};

	struct LightBufferType
	{
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;
		XMFLOAT4 ambientColour;
	};

public:

	ToonShader(ID3D11Device* device, HWND hwnd);
	~ToonShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture, float cutoff1, float cutoff2, float cutoff3, float cutoff4);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* toonBuffer;
};