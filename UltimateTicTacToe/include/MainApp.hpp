#pragma once

#include "Interfaces/IMainApp.hpp"

class MainApp : public IMainApp
{
public:
	MainApp(const sf::Vector2u& windowSize = { 1280u, 720u }, const std::string& windowTitle = "UTTT");
	~MainApp();

	void update() override;
protected:
	void render() override;
	void initUi() override;
	void updateAi() override;
};