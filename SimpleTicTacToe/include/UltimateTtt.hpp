#pragma once

#include "Interfaces/IGame.hpp"

class UltimateTtt : public IGame
{
public:
	UltimateTtt(const sf::Vector2u& windowSize);

	void start(bool isPlayerFirst) override;
	void handleInput(const sf::Event& event) override;
	void render(sf::RenderTarget& target) override;
	void updateAi() override;
};