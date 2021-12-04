#pragma once

#include "Interfaces/IMainApp.hpp"

class MainApp : public IMainApp
{
public:
	MainApp(const sf::Vector2u& windowSize = { 1280u, 720u }, const std::string& windowTitle = "Tic-Tac-Toe");
	~MainApp();

	void update() override;

	static std::atomic_bool debugPlayEnabled; // for debug purposes only
protected:
	void render() override;
	void drawUi() override;
	void updateAi() override;
};