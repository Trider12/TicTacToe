#pragma once

#include <atomic>
#include <thread>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>

#include "IAiPlayer.hpp"

class IMainApp
{
public:
	IMainApp(const sf::Vector2u& windowSize, const std::string& windowTitle) : _isAppRunning{ true } {}
	virtual ~IMainApp() = default;

	virtual bool isAppRunning() const { return _isAppRunning; }
	virtual void update() = 0;

protected:
	virtual void render() = 0;
	virtual void initUi() = 0;
	virtual void updateAi() = 0;

	std::atomic_bool _isAppRunning = false;
	std::atomic_bool _isGameOver = true;
	std::atomic_bool _isPlayerTurn = true;
	std::thread _renderThread;
	std::thread _aiThread;

	sf::RenderWindow _window;
	sf::Clock _renderDeltaClock;

	std::unique_ptr<IAiPlayer> _aiPlayer;

private:
	IMainApp();
};