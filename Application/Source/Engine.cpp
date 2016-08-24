//	DX11Renderer - VDemo | DirectX11 Renderer
//	Copyright(C) 2016  - Volkan Ilbeyli
//
//	This program is free software : you can redistribute it and / or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.If not, see <http://www.gnu.org/licenses/>.
//
//	Contact: volkanilbeyli@gmail.com

#include "Engine.h"

#include "../../Renderer/Source/Renderer.h"
#include "Input.h"
#include "Camera.h"
#include "../../Renderer/Source/Mesh.h"

#include <sstream>

Engine* Engine::s_instance = nullptr;

Engine::Engine()
	:
	m_renderer(nullptr),
	m_input(nullptr),
	m_camera(nullptr),
	m_isPaused(false)
{}

void Engine::TogglePause()
{
	m_isPaused = !m_isPaused;
}

void Engine::CalcFrameStats()
{
	static long frameCount = 0;
	static float timeElaped = 0.0f;

	++frameCount;
	if (m_timer.TotalTime() - timeElaped >= 1.0f)
	{
		float fps = static_cast<float>(frameCount);	// #frames / 1.0f
		float frameTime = 1000.0f / fps;	// milliseconds

		std::ostringstream stats;
		stats.precision(6);
		stats << "VDemo | "
			<< "FPS: " << fps << " "
			<< "FrameTime: " << frameTime << "ms";
		SetWindowText(m_renderer->GetWindow(), stats.str().c_str());
		frameCount = 0;
		timeElaped += 1.0f;
	}
}

Engine::~Engine(){}

bool Engine::Initialize(HWND hWnd, int scr_width, int scr_height)
{
	m_renderer = new Renderer();
	if (!m_renderer) return false;
	m_input = new Input();
	if (!m_input)	return false;
	m_camera = new Camera(m_input);
	if (!m_camera)	return false;
	
	// initialize systems
	m_input->Init();
	if(!m_renderer->Init(scr_width, scr_height, hWnd)) 
		return false;

	m_camera->SetOthoMatrix(scr_width, scr_height, NEAR_PLANE, FAR_PLANE);
	m_camera->SetProjectionMatrix((float)XM_PIDIV4, SCREEN_RATIO, NEAR_PLANE, FAR_PLANE);
	m_camera->SetPosition(0, 1, -10);
	m_renderer->SetCamera(m_camera);

	return true;
}

bool Engine::Load()
{
	std::vector<InputLayout> layout = {
		{ "POSITION",	FLOAT32_3 },
		{ "NORMAL",		FLOAT32_3 },
		{ "TEXCOORD",	FLOAT32_2 }
	};
	ShaderID texShader = m_renderer->AddShader("tex", "Data/Shaders/", layout);

	m_timer.Reset();
	return true;
}

void Engine::Exit()
{
	if (m_input)
	{
		delete m_input;
		m_input = nullptr;
	}

	if (m_renderer)
	{
		m_renderer->Exit();
		delete m_renderer;
		m_renderer = nullptr;
	}

	if (s_instance)
	{
		delete s_instance;
		s_instance = nullptr;
	}
}

Engine * Engine::GetEngine()
{
	if (s_instance == nullptr)
	{
		s_instance = new Engine();
	}

	return s_instance;
}

bool Engine::Run()
{
	m_timer.Tick();
	if (m_input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	if (m_input->IsKeyTriggered(0x50)) // Key P
		TogglePause(); 

	if (!m_isPaused)
	{
		CalcFrameStats();
		Update(m_timer.DeltaTime());
		Render();
	}

	// since keyboard state is updated async, update previous state after frame;
	m_input->Update();	
	return true;
}

void Engine::Pause()
{
	m_isPaused = true;
}

void Engine::Unpause()
{
	m_isPaused = false;
}

void Engine::Update(float dt)
{
	m_camera->Update(dt);


}

void Engine::Render()
{
	const float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	XMMATRIX world	= XMMatrixIdentity();
	XMMATRIX view = m_camera->GetViewMatrix();
	XMMATRIX proj = m_camera->GetProjectionMatrix();
	//XMMATRIX view	= XMMatrixIdentity();
	//XMMATRIX proj	= XMMatrixIdentity();

	ShaderID shd = 0;	// first shader for now
	m_renderer->Begin(clearColor);
	m_renderer->SetShader(shd);
	m_renderer->SetViewport(SCREEN_WIDTH, SCREEN_HEIGHT);
	m_renderer->SetBufferObj(MESH_TYPE::CUBE);
	m_renderer->SetConstant4x4f("world", world);
	m_renderer->SetConstant4x4f("view", view);
	m_renderer->SetConstant4x4f("proj", proj);
	m_renderer->Apply();
	m_renderer->DrawIndexed();
	m_renderer->End();
}
