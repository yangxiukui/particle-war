//粒子系统(雪花、烟花、粒子枪)

#include "../base/d3dUtility.h"
#include "../camera/camera.h"
#include "../model/terrain.h"
#include "../model/vertex.h"
#include "../function/fps.h"
#include "../particle/particleSystem.h"
#include "../model/cube.h"
#include <fstream>

#include <iostream>
#include <mmsystem.h>

IDirect3DDevice9* Device = 0;

Terrain* TheTerrain = 0;

par::Snow* Sno = 0;
par::Firework* Fir = 0;
par::ParticleGun* Gun = 0;
par::Water* Water = 0;
par::Aoi* Aoi = 0;

Camera TheCamera(Camera::LANDOBJECT);
FPSCounter* FPS = 0;

IDirect3DVertexBuffer9 * m_VB = 0;		//镜子的顶点缓存

ID3DXMesh* ParentMesh = 0;
//IDirect3DTexture9* MirrorTex = 0;
//D3DMATERIAL9 MirrorMtrl = d3d::YELLOW_MTRL;

//Cube* Box = 0;

//LPD3DXLINE m_line = NULL;		//3D画线函数
//D3DXVECTOR3 *v_lines = NULL;


bool GameInit()
{
	srand(GetTickCount());

	/*D3DXCreateLine(Device,&m_line);

	v_lines = new D3DXVECTOR3[3];
	v_lines[0] = D3DXVECTOR3(-20.0f, 100.0f, -20.0f);
	v_lines[1] = D3DXVECTOR3(-20.0f, 100.0f,  20.0f);
	v_lines[2] = D3DXVECTOR3( 20.0f, 100.0f,  20.0f);*/

	//Box = new Cube(Device);

	d3d::BoundingBox boundbox;
	boundbox._min = D3DXVECTOR3(-200.0f,0.0f,-200.0f);
	boundbox._max = D3DXVECTOR3(200.0f,200.0f,200.0f);

	Sno = new par::Snow(&boundbox, 4096);
	Sno->init(Device, "..\\Picture\\snowflake.dds");

	d3d::BoundingSphere boundsphere;
	boundsphere._center = D3DXVECTOR3(0.0f,150.0f,0.0f);
	boundsphere._radius = 30.0f;

	Fir = new par::Firework(&boundsphere,4096);
	Fir->init(Device, "..\\Picture\\snowflake.dds");

	D3DXVECTOR3 pos = D3DXVECTOR3(0.0,300.0,0.0);

	Water = new par::Water(&pos,1);
	Water->init(Device, "..\\Picture\\snowflake.dds");

	Gun = new par::ParticleGun(&TheCamera);
	Gun->init(Device, "..\\Picture\\pargun.dds");

	d3d::BoundingBox aoi_boundbox;
	aoi_boundbox._min = D3DXVECTOR3(-10.0f,80.0f,-10.0f);
	aoi_boundbox._max = D3DXVECTOR3(10.0f,100.0f,10.0f);

	Aoi = new par::Aoi(&aoi_boundbox);
	Aoi->init(Device, "..\\Picture\\snowflake.dds");

	Device->CreateVertexBuffer(
			6 * sizeof(Vertex),
			0, 
			FVF_VERTEX,
			D3DPOOL_MANAGED,
			&m_VB,
			0);

	Vertex* v = 0;
	m_VB->Lock(0, 0, (void**)&v, 0);

	v[0] = Vertex(-20.0f, 100.0f, -20.0f, 0.0f, 1.0f, 0.0f, 0.0f, 5.0f);
	v[1] = Vertex(-20.0f, 100.0f,  20.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex(-20.0f, 100.0f, -20.0f, 0.0f, 1.0f, 0.0f, 0.0f, 5.0f);

	v[3] = Vertex(-20.0f, 100.0f, -20.0f, 0.0f, 1.0f, 0.0f, 0.0f, 5.0f);
	v[4] = Vertex( 20.0f, 100.0f,  20.0f, 0.0f, 1.0f, 0.0f, 5.0f, 0.0f);
	v[5] = Vertex( 20.0f, 100.0f, -20.0f, 0.0f, 1.0f, 0.0f, 5.0f, 5.0f);

	m_VB->Unlock();

	/*D3DXCreateTextureFromFile(
			Device,
			"..\\Picture\\grass.bmp",
			&MirrorTex);*/

	//画球体
	D3DXCreateSphere(
		Device,
		36.0f,
		100,			//竖向的切面个数
		100,			//横向的切面个数
		&ParentMesh,
		0);

	D3DXVECTOR3 lightDirection(0.0f, 1.0f, 0.0f);
	TheTerrain = new Terrain(Device,128, 128, 10, 0.5f);		//"..\\Picture\\coastMountain64.raw",
	TheTerrain->createTexture(&lightDirection);

	FPS = new FPSCounter(Device);
	
	Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	D3DXCOLOR col(1.0f, 1.0f, 1.0f, 1.0f);
	D3DLIGHT9 light = d3d::InitDirectionalLight(&lightDirection, &col);

	Device->SetLight(0, &light);
	Device->LightEnable(0, true);
	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);	//重新规格化所有法线
	Device->SetRenderState(D3DRS_SPECULARENABLE, true);		//镜面高光设置可用

	RECT rc = {SCREEN_XPOS,SCREEN_YPOS,SCREEN_WIDTH,SCREEN_HEIGHT};
	AdjustWindowRect(&rc,WS_POPUP,NULL);
	rc.right = rc.left + GetSystemMetrics(SM_CXSCREEN);		//获取屏幕的宽度
	rc.bottom = rc.top + GetSystemMetrics(SM_CYSCREEN);		//获取屏幕的高度

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
		&proj,
		D3DX_PI * 0.25f,
		(float)(rc.right-rc.left) / (float)(rc.bottom-rc.top),
		1.0f,
		1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	mciSendString(TEXT("open ..\\Music\\bgm.mp3 alias bgm"),NULL,0,NULL);
	mciSendString(TEXT("play bgm repeat"),NULL,0,NULL);

	return true;
}

//最后声明的最先删除
void GameEnd()
{
	/*if(m_line != NULL)
	{
		m_line->Release();
	}*/
	//d3d::Release<IDirect3DTexture9*>(MirrorTex);
	//d3d::Delete<Cube*>(Box);
	d3d::Release<ID3DXMesh*>(ParentMesh);
	d3d::Release<IDirect3DVertexBuffer9*>(m_VB);

	d3d::Delete<par::PSystem*>( Sno );
	d3d::Delete<par::PSystem*>( Fir );
	d3d::Delete<par::PSystem*>( Gun );
	d3d::Delete<par::PSystem*>( Water );
	d3d::Delete<par::PSystem*>( Aoi );
	d3d::Delete<Terrain*>(TheTerrain);

	
}

bool GameStart(float timeDelta)
{
	if( Device )
	{
		D3DXVECTOR3 pos;
		TheCamera.getPosition(&pos);
		float height = TheTerrain->getHeight( pos.x, pos.z );
		pos.y = height + peopleHeight;
		TheCamera.setPosition(&pos);

		/*using namespace std;
		ofstream rolePos;

		rolePos.open("..\\Logs\\rolePos.txt");
		rolePos << "x = " << pos.x << " y = " << pos.y << " z = " << pos.z << endl;*/

		D3DXMATRIX V;
		TheCamera.getViewMatrix(&V);
		Device->SetTransform(D3DTS_VIEW, &V);


		if(KEYDOWN('W'))
			TheCamera.walk(16.0f * timeDelta);

		if(KEYDOWN('S'))
			TheCamera.walk(-16.0f * timeDelta);

		if(KEYDOWN('A'))
			TheCamera.strafe(-16.0f * timeDelta);

		if(KEYDOWN('D'))
			TheCamera.strafe(16.0f * timeDelta);

		if(KEYDOWN('R'))
			TheCamera.fly(16.0f * timeDelta);

		if(KEYDOWN('F'))
			TheCamera.fly(-16.0f * timeDelta);

		if(KEYDOWN('Q'))
			TheCamera.yaw(-1.0f * timeDelta);

		if(KEYDOWN('E'))
			TheCamera.yaw(1.0f * timeDelta);

		if(KEYDOWN(VK_UP))
			TheCamera.pitch(-1.0f * timeDelta);

		if(KEYDOWN(VK_DOWN))
			TheCamera.pitch(1.0f * timeDelta);

		if(KEYDOWN(VK_LEFT))
			TheCamera.roll(1.0f * timeDelta);

		if(KEYDOWN(VK_RIGHT))
			TheCamera.roll(-1.0f * timeDelta);

		//Sno->updateCover(timeDelta, TheTerrain);
		//Sno->update(timeDelta);
		//Fir->update(timeDelta);
		//Gun->update(timeDelta);
		//Water->update(timeDelta);
		//Aoi->update(timeDelta);

		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);  //132522
		Device->BeginScene();

		D3DXMATRIX I;
		D3DXMatrixIdentity(&I);

		if( TheTerrain )
			TheTerrain->draw(&I, false);
		
		//Fir->render();
		//Sno->render();
		//Gun->render();
		//Water->render();
		//Aoi->render();
		//Aoi->drawLine(Device);

		//Device->SetStreamSource(0, m_VB, 0, sizeof(Vertex));	//设置资源流
		//Device->SetFVF(FVF_VERTEX);								//设置顶点格式

		//Device->SetRenderState(D3DRS_LIGHTING, false);
		//Device->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);			//禁用背面消隐
		//Device->DrawPrimitive(D3DPT_LINELIST, 0, 1);

		//Device->SetMaterial(&MirrorMtrl);
		//Device->SetTexture(0, MirrorTex);
		//Box->draw(&I,0,0);		//,MirrorTex
		
		//m_line->DrawTransform(v_lines,1,&I,d3d::GREEN);
		D3DXMATRIX P,W;
		D3DXMatrixTranslation(&P,  0.0f, 200.0f, 0.0f);		//平移函数

		W = I * P;
		Device->SetTransform(D3DTS_WORLD, &W);

		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);		//D3DBLEND_INVSRCALPHA

		D3DMATERIAL9 magenta = d3d::BLUE_MTRL;
		magenta.Diffuse.a = 0.10f;
		Device->SetMaterial(&magenta);
		ParentMesh->DrawSubset(0);

		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

		Device->SetTransform(D3DTS_WORLD, &I);		//进行世界变换

		/*if( FPS )
			FPS->render(0xffffffff, timeDelta);*/

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
	}
	return true;
}


LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE )
			::DestroyWindow(hwnd);
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance, 
	PSTR cmdLine,
	int showCmd)
{
	RECT rc = {SCREEN_XPOS,SCREEN_YPOS,SCREEN_WIDTH,SCREEN_HEIGHT};
	AdjustWindowRect(&rc,WS_POPUP,NULL);
	rc.right = rc.left + GetSystemMetrics(SM_CXSCREEN);		//获取屏幕的宽度
	rc.bottom = rc.top + GetSystemMetrics(SM_CYSCREEN);		//获取屏幕的高度

	if(!d3d::InitD3D(hinstance,rc.left,rc.top,(rc.right-rc.left),(rc.bottom-rc.top),SCREEN_WINDOWED, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if(!GameInit())
	{
		::MessageBox(0, "GameInit() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(GameStart);

	GameEnd();

	Device->Release();

	return 0;
}

