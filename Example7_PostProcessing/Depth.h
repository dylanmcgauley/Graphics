#pragma once
#include "DXF.h"

using namespace std;
using namespace DirectX;

class Depth : public BaseShader
{

public:

	Depth(ID3D11Device* device, HWND hwnd);
	~Depth();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
};
