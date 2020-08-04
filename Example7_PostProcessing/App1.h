// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include "TextureShader.h"
#include "VerticalBlurShader.h"
#include "HorizontalBlurShader.h"
#include "EdgeDetector.h"
#include "ToonShader.h"
#include "SpinBlur.h"
#include "MultiLight.h"
#include "HeightMap.h"
#include "Depth.h"
#include "ShadowShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void depth();
	void firstPass();
	void downSample();
	void upSample();
	void verticalBlur();
	void horizontalBlur();
	void doSpinBlur();
	void detectEdges();
	void applyToon();
	void finalPass();
	void initRenderTextures(int screenWidth, int screenHeight);
	void initMeshes(int screenWidth, int screenHeight);
	void loadTextures();
	void initLights();
	void initShaders(HWND hwnd);
	void gui();

private:
	// meshes
	CubeMesh* cubeMesh[7];
	PlaneMesh* heightmapMesh;
	PlaneMesh* lightMesh;
	PlaneMesh* heightmapMesh2;
	PlaneMesh* lightMesh2;
	PlaneMesh* shadowMesh;
	OrthoMesh* orthoMesh;

	// render textures
	RenderTexture* renderTexture;
	RenderTexture* dsampleTexture;
	RenderTexture* usampleTexture;
	RenderTexture* horizontalBlurTexture;
	RenderTexture* verticalBlurTexture;
	RenderTexture* spinTexture;
	RenderTexture* edgeTexture;
	RenderTexture* toonTexture;
	RenderTexture* shadowTexture;

	// shaders
	LightShader* lightShader;
	TextureShader* textureShader;
	VerticalBlurShader* verticalBlurShader;
	HorizontalBlurShader* horizontalBlurShader;
	EdgeDetector* edgeShader;
	ToonShader* toonShader;
	SpinBlur* spinBlur;
	HeightMap* heightMap;
	MultiLight* multiLight;
	HeightMap* heightMap2;
	MultiLight* multiLight2;
	ShadowShader* cubes[7];
	Depth* depthShader;
	ShadowShader* shadows;
	
	// lights
	Light* light;
	Light* spotLight[4];
	Light* shadowLight;
	XMFLOAT3 lightColour[4];
	XMFLOAT3 lightPos[4];
	bool editLights = false;

	// edge detector values
	float lowerTol = 0;
	float upperTol = 0;
	float edgeThickness = 1;
	bool edgeActive = false;

	// toon shader values
	float cutoff1 = 0.95;
	float cutoff2 = 0.5;
	float cutoff3 = 0.05;
	float cutoff4 = 0.01;
	bool toonActive = false;

	// heightmap values
	float manipVal = 10;
	bool editHeight = false;

	// spin values
	bool spinActive = false;
};

#endif