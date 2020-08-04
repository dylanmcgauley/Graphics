#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// call all inits
	initMeshes(screenWidth, screenHeight);

	loadTextures();

	initShaders(hwnd);

	initRenderTextures(screenWidth, screenHeight);

	initLights();

	camera->setPosition(80.0f, 6.0f, 130.0f);
	camera->setRotation(0.0f, 180.0f, 0.0f);

	shadowLight->generateOrthoMatrix((float)100, (float)100, 0.1f, 100.f);
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	// Depth Pass
	depth();
	// Render scene
	firstPass();
	// Down Sample
	downSample();

	// Post Processing
	if (edgeActive) detectEdges();
	if (toonActive) applyToon();
	if (spinActive) doSpinBlur();

	// Up Sample
	upSample();
	// Render final pass to frame buffer
	finalPass();

	return true;
}

void App1::depth()
{
	// Set the render target to be the render to texture and clear it
	shadowTexture->setRenderTarget(renderer->getDeviceContext());
	shadowTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);

	shadowLight->generateViewMatrix();
	XMMATRIX lightViewMatrix = shadowLight->getViewMatrix();
	XMMATRIX lightProjectionMatrix = shadowLight->getOrthoMatrix();


	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX translate;
	XMMATRIX rotate;
	XMMATRIX scale;

	// render heightmap with spotlights
	heightmapMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), heightmapMesh->getIndexCount());

	// render heightmap with spotlights
	translate = XMMatrixTranslation(99, 0, 0);
	worldMatrix = translate;
	heightmapMesh2->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), heightmapMesh2->getIndexCount());

	// render plane with spotlights
	worldMatrix = renderer->getWorldMatrix();
	translate = XMMatrixTranslation(0, 0, 99);
	worldMatrix = translate;
	lightMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), lightMesh->getIndexCount());

	// render plane with spotlights
	worldMatrix = renderer->getWorldMatrix();
	translate = XMMatrixTranslation(99, 0, 99);
	worldMatrix = translate;
	lightMesh2->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), lightMesh2->getIndexCount());

	// render shadow plane
	worldMatrix = renderer->getWorldMatrix();
	translate = XMMatrixTranslation(-99, 0, 99);
	worldMatrix = translate;
	shadowMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), shadowMesh->getIndexCount());

	// render cubes
	for (int x = 0; x < 7; x++)
	{
		worldMatrix = renderer->getWorldMatrix();
		translate = XMMatrixTranslation(-99 + (x * 10), 5, 110 + (x * 10));
		worldMatrix = translate;
		cubeMesh[x]->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), cubeMesh[x]->getIndexCount());
	}

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::firstPass()
{
	// Set the render target to be the render to texture and clear it
	renderTexture->setRenderTarget(renderer->getDeviceContext());
	renderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);

	// Get matrices
	camera->update();

	for (int x = 0; x < 4; x++)
	{
		spotLight[x]->setDiffuseColour(lightColour[x].x, lightColour[x].y, lightColour[x].z, 1.0f);
		spotLight[x]->setPosition(lightPos[x].x, lightPos[x].y, lightPos[x].z);
	}

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	XMMATRIX translate;
	XMMATRIX rotate;
	XMMATRIX scale;

	// render heightmap with spotlights
	heightmapMesh->sendData(renderer->getDeviceContext());
	heightMap->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("heightmap"), textureMgr->getTexture("terrain"), manipVal, spotLight[0], spotLight[1], spotLight[2], spotLight[3]);
	heightMap->render(renderer->getDeviceContext(), heightmapMesh->getIndexCount());

	// render heightmap with spotlights
	translate = XMMatrixTranslation(99, 0, 0);
	worldMatrix = translate;
	heightmapMesh2->sendData(renderer->getDeviceContext());
	heightMap2->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("heightmap"), textureMgr->getTexture("terrain"), manipVal, spotLight[0], spotLight[1], spotLight[2], spotLight[3]);
	heightMap2->render(renderer->getDeviceContext(), heightmapMesh2->getIndexCount());

	// render plane with spotlights
	worldMatrix = renderer->getWorldMatrix();
	translate = XMMatrixTranslation(0, 0, 99);
	worldMatrix = translate;
	lightMesh->sendData(renderer->getDeviceContext());
	multiLight->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("terrain"), shadowTexture->getShaderResourceView(), spotLight[0], spotLight[1], spotLight[2], spotLight[3]);
	multiLight->render(renderer->getDeviceContext(), lightMesh->getIndexCount());

	// render plane with spotlights
	worldMatrix = renderer->getWorldMatrix();
	translate = XMMatrixTranslation(99, 0, 99);
	worldMatrix = translate;
	lightMesh2->sendData(renderer->getDeviceContext());
	multiLight2->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("terrain"), shadowTexture->getShaderResourceView(), spotLight[0], spotLight[1], spotLight[2], spotLight[3]);
	multiLight2->render(renderer->getDeviceContext(), lightMesh2->getIndexCount());

	// render shadow plane
	worldMatrix = renderer->getWorldMatrix();
	translate = XMMatrixTranslation(-99, 0, 99);
	worldMatrix = translate;
	shadowMesh->sendData(renderer->getDeviceContext());
	shadows->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("terrain"), shadowTexture->getShaderResourceView(), shadowLight);
	shadows->render(renderer->getDeviceContext(), shadowMesh->getIndexCount());

	// render cubes
	for (int x = 0; x < 7; x++)
	{
		worldMatrix = renderer->getWorldMatrix();
		translate = XMMatrixTranslation(-99 + (x * 10), 5, 110 + (x * 10));
		worldMatrix = translate;
		cubeMesh[x]->sendData(renderer->getDeviceContext());
		cubes[x]->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("cube"), shadowTexture->getShaderResourceView(), shadowLight);
		cubes[x]->render(renderer->getDeviceContext(), cubeMesh[x]->getIndexCount());
	}

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::downSample()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	dsampleTexture->setRenderTarget(renderer->getDeviceContext());
	dsampleTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = dsampleTexture->getOrthoMatrix();

	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, renderTexture->getShaderResourceView());
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::doSpinBlur()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	float screenSizeX = (float)spinTexture->getTextureWidth();
	float screenSizeY = (float)spinTexture->getTextureHeight();
	spinTexture->setRenderTarget(renderer->getDeviceContext());
	spinTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	// Get the ortho matrix from the render to texture since texture has different dimensions being that it is smaller.
	orthoMatrix = spinTexture->getOrthoMatrix();

	// do the edge detection
	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	// set edge paramerters
	spinBlur->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, dsampleTexture->getShaderResourceView());
	spinBlur->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::detectEdges()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	float screenSizeX = (float)edgeTexture->getTextureWidth();
	float screenSizeY = (float)edgeTexture->getTextureHeight();
	edgeTexture->setRenderTarget(renderer->getDeviceContext());
	edgeTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	// Get the ortho matrix from the render to texture since texture has different dimensions being that it is smaller.
	orthoMatrix = edgeTexture->getOrthoMatrix();

	// do the edge detection
	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	// set edge paramerters
	edgeShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, dsampleTexture->getShaderResourceView(), screenSizeX, screenSizeY, edgeThickness, lowerTol, upperTol);
	edgeShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::applyToon()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	float screenSizeX = (float)toonTexture->getTextureWidth();
	float screenSizeY = (float)toonTexture->getTextureHeight();
	toonTexture->setRenderTarget(renderer->getDeviceContext());
	toonTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	// Get the ortho matrix from the render to texture since texture has different dimensions being that it is smaller.
	orthoMatrix = toonTexture->getOrthoMatrix();

	// do the edge detection
	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	// set edge paramerters
	toonShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, edgeTexture->getShaderResourceView(), cutoff1, cutoff2, cutoff3, cutoff4);
	toonShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::horizontalBlur()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	float screenSizeX = (float)horizontalBlurTexture->getTextureWidth();
	horizontalBlurTexture->setRenderTarget(renderer->getDeviceContext());
	horizontalBlurTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = horizontalBlurTexture->getOrthoMatrix();

	// Render for Horizontal Blur
	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	horizontalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, renderTexture->getShaderResourceView(), screenSizeX);
	horizontalBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::verticalBlur()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	float screenSizeY = (float)verticalBlurTexture->getTextureHeight();
	verticalBlurTexture->setRenderTarget(renderer->getDeviceContext());
	verticalBlurTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 1.0f, 1.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	// Get the ortho matrix from the render to texture since texture has different dimensions being that it is smaller.
	orthoMatrix = verticalBlurTexture->getOrthoMatrix();

	// Render for Vertical Blur
	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	verticalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, horizontalBlurTexture->getShaderResourceView(), screenSizeY);
	verticalBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::upSample()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	usampleTexture->setRenderTarget(renderer->getDeviceContext());
	usampleTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	// Get the ortho matrix from the render to texture since texture has different dimensions being that it is smaller.
	orthoMatrix = usampleTexture->getOrthoMatrix();

	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	if (edgeActive)
	{
		if (toonActive)
		{
			textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, toonTexture->getShaderResourceView());
		}
		else
		{
			textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, edgeTexture->getShaderResourceView());
		}
	}
	else if (spinActive)
	{
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, spinTexture->getShaderResourceView());
	}
	else
	{
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, dsampleTexture->getShaderResourceView());
	}
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// RENDER THE RENDER TEXTURE SCENE
	// Requires 2D rendering and an ortho mesh.
	renderer->setZBuffer(false);
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, usampleTexture->getShaderResourceView());
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();
}

// initialize render textures
void App1::initRenderTextures(int screenWidth, int screenHeight)
{
	renderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	dsampleTexture = new RenderTexture(renderer->getDevice(), screenWidth / 2, screenHeight / 2, SCREEN_NEAR, SCREEN_DEPTH);
	usampleTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	horizontalBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	verticalBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	edgeTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	toonTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	spinTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	shadowTexture = new RenderTexture(renderer->getDevice(), 2048, 2048, 0.1, 100);
}

// initialize meshes
void App1::initMeshes(int screenWidth, int screenHeight)
{
	for (int x = 0; x < 7; x++)
	{
		cubeMesh[x] = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	}
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);	// Full screen size
	heightmapMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	lightMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	heightmapMesh2 = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	lightMesh2 = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	shadowMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
}

// initialize shaders
void App1::initShaders(HWND hwnd)
{
	lightShader = new LightShader(renderer->getDevice(), hwnd);
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	horizontalBlurShader = new HorizontalBlurShader(renderer->getDevice(), hwnd);
	verticalBlurShader = new VerticalBlurShader(renderer->getDevice(), hwnd);
	edgeShader = new EdgeDetector(renderer->getDevice(), hwnd);
	toonShader = new ToonShader(renderer->getDevice(), hwnd);
	spinBlur = new SpinBlur(renderer->getDevice(), hwnd);
	heightMap = new HeightMap(renderer->getDevice(), hwnd);
	multiLight = new MultiLight(renderer->getDevice(), hwnd);
	heightMap2 = new HeightMap(renderer->getDevice(), hwnd);
	multiLight2 = new MultiLight(renderer->getDevice(), hwnd);
	shadows = new ShadowShader(renderer->getDevice(), hwnd);
	for (int x = 0; x < 7; x++)
	{
		cubes[x] = new ShadowShader(renderer->getDevice(), hwnd);
	}
	depthShader = new Depth(renderer->getDevice(), hwnd);
}

// load in all required textures
void App1::loadTextures()
{
	textureMgr->loadTexture("cube", L"..res/test.png");
	textureMgr->loadTexture("terrain", L"../res/terrain.png");
	textureMgr->loadTexture("heightmap", L"../res/heightmap.png");
}

// initialize lights
void App1::initLights()
{
	light = new Light;
	light->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
	light->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light->setDirection(0.7f, 0.0f, 0.7f);

	// setup for the spotlights
	spotLight[0] = new Light;
	spotLight[0]->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
	lightColour[0] = XMFLOAT3(4.0f, 0.0f, 0.0f);
	lightPos[0] = XMFLOAT3(50.0f, 20.0f, 50.0f);

	spotLight[1] = new Light;
	spotLight[1]->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
	lightColour[1] = XMFLOAT3(0.0f, 0.0f, 4.0f);
	lightPos[1] = XMFLOAT3(70.0f, 20.0f, 140.0f);

	spotLight[2] = new Light;
	spotLight[2]->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
	lightColour[2] = XMFLOAT3(0.0f, 4.0f, 0.0f);
	lightPos[2] = XMFLOAT3(150.0f, 20.0f, 150.0f);

	spotLight[3] = new Light;
	spotLight[3]->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
	lightColour[3] = XMFLOAT3(3.0f, 3.0f, 0.0f);
	lightPos[3] = XMFLOAT3(140.0f, 40.0f, 50.0f);

	shadowLight = new Light;
	shadowLight->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	shadowLight->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	shadowLight->setDirection(1.0f, -1.0f, 0.0f);
	shadowLight->setPosition(-70.0f, 6.f, 140.f);

	for (int x = 0; x < 4; x++)
	{
		spotLight[x]->setDiffuseColour(lightColour[x].x, lightColour[x].y, lightColour[x].z, 1.0f);
		spotLight[x]->setPosition(lightPos[x].x, lightPos[x].y, lightPos[x].z);
	}
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	// Edge Detection Values
	ImGui::Checkbox("Edge Detection", &edgeActive);
	if (edgeActive)
	{
		ImGui::SliderFloat("LowerTollerence", &lowerTol, -2, 2);
		ImGui::SliderFloat("HigherTollerence", &upperTol, -2, 2);
		ImGui::SliderFloat("Edge Thickness", &edgeThickness, 0.1, 7);

		// Toon Shader Values
		ImGui::Checkbox("Toon Shader", &toonActive);
		if (toonActive)
		{
			ImGui::SliderFloat("Cutoff 1", &cutoff1, 0, 1);
			ImGui::SliderFloat("Cutoff 2", &cutoff2, 0, 1);
			ImGui::SliderFloat("Cutoff 3", &cutoff3, 0, 1);
			ImGui::SliderFloat("Cutoff 4", &cutoff4, 0, 1);
		}
	}

	// Light Values
	ImGui::Checkbox("Edit Multi Lights", &editLights);
	if (editLights)
	{
		ImGui::DragFloat("Light1 X Position", &lightPos[0].x);
		ImGui::DragFloat("Light1 Y Position", &lightPos[0].y);
		ImGui::DragFloat("Light1 Z Position", &lightPos[0].z);
		ImGui::DragFloat("Light1 Red Slider", &lightColour[0].x);
		ImGui::DragFloat("Light1 Green Slider", &lightColour[0].y);
		ImGui::DragFloat("Light1 Blue Slider", &lightColour[0].z);
		ImGui::DragFloat("Light2 X Position", &lightPos[1].x);
		ImGui::DragFloat("Light2 Y Position", &lightPos[1].y);
		ImGui::DragFloat("Light2 Z Position", &lightPos[1].z);
		ImGui::DragFloat("Light2 Red Slider", &lightColour[1].x);
		ImGui::DragFloat("Light2 Green Slider", &lightColour[1].y);
		ImGui::DragFloat("Light2 Blue Slider", &lightColour[1].z);
		ImGui::DragFloat("Light3 X Position", &lightPos[2].x);
		ImGui::DragFloat("Light3 Y Position", &lightPos[2].y);
		ImGui::DragFloat("Light3 Z Position", &lightPos[2].z);
		ImGui::DragFloat("Light3 Red Slider", &lightColour[2].x);
		ImGui::DragFloat("Light3 Green Slider", &lightColour[2].y);
		ImGui::DragFloat("Light3 Blue Slider", &lightColour[2].z);
		ImGui::DragFloat("Light4 X Position", &lightPos[3].x);
		ImGui::DragFloat("Light4 Y Position", &lightPos[3].y);
		ImGui::DragFloat("Light4 Z Position", &lightPos[3].z);
		ImGui::DragFloat("Light4 Red Slider", &lightColour[3].x);
		ImGui::DragFloat("Light4 Green Slider", &lightColour[3].y);
		ImGui::DragFloat("Light4 Blue Slider", &lightColour[3].z);
	}

	// Heightmap Values
	ImGui::Checkbox("Edit Heightmap", &editHeight);
	if (editHeight)
	{
		ImGui::SliderFloat("Manipulation Value", &manipVal, 0, 50);
	}

	// Spin Values
	ImGui::Checkbox("Enable Spin Blur", &spinActive);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

