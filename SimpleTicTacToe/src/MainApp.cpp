#include "MainApp.hpp"

#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Window/Event.hpp>

MainApp::MainApp() : _isRunning{ true }
{
	_window.create(sf::VideoMode(1280, 720), "TTT", sf::Style::Titlebar | sf::Style::Close);
	_window.setFramerateLimit(60);
	_window.setVerticalSyncEnabled(true);
	_window.setActive(false);

	ImGui::SFML::Init(_window);

	_aiThread = std::thread([this]() { for (; _isRunning; updateAi()); });
	_renderThread = std::thread([this]() { for (_window.setActive(true); _isRunning || !_window.setActive(false); draw()); });
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
	sf::Event event;

	while (_window.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(event);

		_isRunning = event.type != sf::Event::Closed;
	}
}

void MainApp::draw()
{
	ImGui::SFML::Update(_window, _renderDeltaClock.restart());

	ImGui::Begin("Hello, world!");
	ImGui::Button("Look at this pretty button");
	ImGui::End();

	_window.clear();



	ImGui::SFML::Render(_window);
	_window.display();
}

void MainApp::updateAi()
{
}