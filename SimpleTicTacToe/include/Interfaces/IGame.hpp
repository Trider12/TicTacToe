#pragma once

#include <atomic>
#include <memory>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

#include "IAiPlayer.hpp"

class IGame
{
public:
	IGame(const sf::Vector2u& windowSize) {}

	virtual void start(bool isPlayerFirst) = 0;
	virtual void handleInput(const sf::Event& event) = 0;
	virtual void update(float delta) = 0;
	virtual void render(sf::RenderTarget& target) = 0;
	virtual void updateAi() = 0;
	virtual bool isOver() const { return _isGameOver; }

protected:
	std::atomic_bool _isGameOver = true;
	std::atomic_bool _isPlayerTurn = true;
	std::unique_ptr<IAiPlayer> _aiPlayer;
};