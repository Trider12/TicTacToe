#pragma once

#include <atomic>
#include <thread>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>

#include "AiPlayer.hpp"

class MainApp
{
public:
	MainApp(const sf::Vector2u& windowSize = { 1280u, 720u }, const std::string& windowTitle = "STTT");
	~MainApp();

	bool isAppRunning() const { return _isAppRunning; };
	void update();

private:
	void render();
	void initUi();
	void updateAi();

	std::atomic_bool _isAppRunning = false;
	std::atomic_bool _isGameOver = false;
	std::atomic_bool _isPlayerTurn = true;
	std::thread _renderThread;
	std::thread _aiThread;

	sf::RenderWindow _window;
	sf::Clock _renderDeltaClock;

	AiPlayer _aiPlayer;
};