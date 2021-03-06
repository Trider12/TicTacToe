#pragma once

#include "Interfaces/IGame.hpp"

class UltimateTtt : public IGame
{
public:
	UltimateTtt(const sf::Vector2u& windowSize);

	void reset(bool isPlayerFirst) override;
	void input(const sf::Event& event) override;
	void update(float delta) override;
	void render(sf::RenderTarget& target) override;
	void updateAi() override;
};