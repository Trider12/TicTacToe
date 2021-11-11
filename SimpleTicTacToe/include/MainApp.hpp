#pragma once

#include <thread>
#include <mutex>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>

class MainApp
{
public:
	MainApp(const sf::Vector2u& windowSize = { 1280u, 720u }, const std::string& windowTitle = "STTT");
	~MainApp();

	bool isRunning() const { return _isRunning; };
	void update();

private:
	void render();
	void initUi();
	void updateAi();

	bool _isRunning = false;

	sf::RenderWindow _window;
	sf::Clock _renderDeltaClock;
	std::thread _renderThread;
	std::thread _aiThread;
};