#include "MainApp.hpp"

#include <mutex>

#if defined(_WIN32) && !defined(_DEBUG)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <imconfig-SFML.h>
#include <imgui.h>
#include <imgui-SFML.h>

#include "SimpleTTT.hpp"
#include "UltimateTtt.hpp"

namespace
{
	enum class GameMode : int32_t
	{
		SimpleMode = 0, UltimateMode = 1
	};

	const sf::Color clearColorDefault = sf::Color(100, 100, 100, 255);

	static bool isPlayerFirst = true;
	static GameMode gameMode = GameMode::SimpleMode;

	static void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
}

MainApp::MainApp(const sf::Vector2u& windowSize /*= { 1280u, 720u }*/, const std::string& windowTitle /*= "STTT"*/) : IMainApp(windowSize, windowTitle)
{
#if defined(_WIN32) && !defined(_DEBUG)
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
#endif
	_window.create(sf::VideoMode(windowSize.x, windowSize.y), windowTitle, sf::Style::Titlebar | sf::Style::Close);
	_window.setVerticalSyncEnabled(true);
	_window.setActive(false);

	ImGui::SFML::Init(_window);
	ImGui::GetIO().IniFilename = nullptr;

	_gameInstance = std::make_unique<SimpleTtt>(windowSize);

	_aiThread = std::thread([this]() { for (; _isAppRunning; updateAi()); });
	_renderThread = std::thread([this]() { for (_window.setActive(true); _isAppRunning || !_window.setActive(false); render()); });
}

MainApp::~MainApp()
{
	_aiThread.join();
	_renderThread.join();

	_window.close();

	ImGui::SFML::Shutdown();
}

void MainApp::update()
{
	for (sf::Event event; _window.pollEvent(event);)
	{
		ImGui::SFML::ProcessEvent(event);

		if (event.type == sf::Event::Closed)
		{
			_isAppRunning = false;
		}

		_gameInstance->handleInput(event);
	}
}

void MainApp::render()
{
	ImGui::SFML::Update(_window, _renderDeltaClock.restart());

	if (_gameInstance->isOver())
	{
		drawUi();
	}

	_window.clear(clearColorDefault);

	_gameInstance->render(_window);

	ImGui::SFML::Render(_window);
	_window.display();
}

void MainApp::drawUi()
{
	auto winSize = sf::Vector2f(_window.getSize());
	auto buttonSize = sf::Vector2f(winSize.x * 0.1f, winSize.y * 0.1f);

	ImGui::SetNextWindowPos(winSize * 0.5f, ImGuiCond_Always, { 0.5f, 0.5f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 40.f, 40.f });
	ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoNav);
	ImGui::PopStyleVar();

	if (ImGui::Button("Play", buttonSize))
	{
		_gameInstance->start(isPlayerFirst);
	}

	ImGui::Spacing();
	ImGui::Checkbox("Player First", &isPlayerFirst);
	ImGui::Spacing();
	ImGui::TextUnformatted("Game Mode:");

	if (ImGui::RadioButton("Simple", reinterpret_cast<int32_t*>(&gameMode), 0))
	{
		_gameInstance.reset(new SimpleTtt(_window.getSize()));
	}

	if (ImGui::RadioButton("Ultimate", reinterpret_cast<int32_t*>(&gameMode), 1))
	{
		_gameInstance.reset(new UltimateTtt(_window.getSize()));
	}

	ImGui::SameLine();
	HelpMarker("Work in Progress");
	ImGui::Spacing();

	if (ImGui::Button("Exit", buttonSize))
	{
		_isAppRunning = false;
	}

	//ImGui::ShowDemoWindow();

	ImGui::End();
}

void MainApp::updateAi()
{
	_gameInstance->updateAi();
}