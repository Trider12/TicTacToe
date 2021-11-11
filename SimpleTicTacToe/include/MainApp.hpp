#pragma once

#include <thread>
#include <mutex>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>

class MainApp
{
public:
	MainApp();
	~MainApp();

	bool isRunning() const { return _isRunning; };
	void update();

private:
	void draw();
	void updateAi();

	bool _isRunning = false;

	sf::RenderWindow _window;
	sf::Clock _renderDeltaClock;
	std::thread _renderThread;
	std::thread _aiThread;
	mutable std::mutex _dataMutex;
};