#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

#include "IBoard.hpp"
#include "IAiPlayer.hpp"

class IGame
{
public:
	IGame(const sf::Vector2u& windowSize) {}

	virtual void reset(bool isPlayerFirst) = 0;
	virtual void input(const sf::Event& event) = 0;
	virtual void update(float delta) = 0;
	virtual void render(sf::RenderTarget& target) = 0;
	virtual void updateAi() = 0;
	virtual void exit() {}

	virtual bool isOver() const { return _isGameOver; }

protected:
	std::atomic_bool _isGameOver = true;
	std::atomic_bool _isPlayerTurn = true;
	std::atomic_bool _isExiting = false;
	std::condition_variable _aiExecuteCondition;
	std::unique_ptr<IBoard> _board;
	std::unique_ptr<IAiPlayer> _aiPlayer;
};