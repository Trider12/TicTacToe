#pragma once

#include <atomic>
#include <thread>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>

#include "Interfaces/IGame.hpp"

class IMainApp
{
public:
	IMainApp(const sf::Vector2u& windowSize, const std::string& windowTitle) : _isAppRunning{ true } {}
	virtual ~IMainApp() = default;

	virtual bool isAppRunning() const { return _isAppRunning; }
	virtual void update() = 0;

protected:
	virtual void render() = 0;
	virtual void drawUi() = 0;
	virtual void updateAi() = 0;

	std::atomic_bool _isAppRunning = false;
	std::unique_ptr<IGame> _gameInstance;
	std::thread _renderThread;
	std::thread _aiThread;

	sf::RenderWindow _window;
	sf::Clock _updateDeltaClock;
	sf::Clock _renderDeltaClock;
private:
	IMainApp();
};