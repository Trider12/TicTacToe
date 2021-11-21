#include "MainApp.hpp"

#include <mutex>

#if defined(_WIN32) && !defined(_DEBUG)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <imconfig-SFML.h>
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>

namespace
{
	const int playerMark = 1, aiMark = 2;
	static bool isPlayerFirst = true;

	struct BoardState
	{
		mutable std::mutex mutex;
		std::vector<uint8_t> cells = std::vector<uint8_t>(9);
	} boardState = {};

	struct BoardRenderData
	{
		std::vector<sf::Vertex> linesVertices;
		sf::Rect<float> rect;
		sf::RectangleShape crossLines[2];
		sf::CircleShape circle;

		void init(const sf::Vector2u& windowSize);
	} boardRenderData;
}

MainApp::MainApp(const sf::Vector2u& windowSize /*= { 1280u, 720u }*/, const std::string& windowTitle /*= "UTTT"*/) : IMainApp(windowSize, windowTitle)
{
#if defined(_WIN32) && !defined(_DEBUG)
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
#endif
	_window.create(sf::VideoMode(windowSize.x, windowSize.y), windowTitle, sf::Style::Titlebar | sf::Style::Close);
	_window.setVerticalSyncEnabled(true);
	_window.setActive(false);

	ImGui::SFML::Init(_window);
	ImGui::GetIO().IniFilename = nullptr;

	boardRenderData.init(_window.getSize());

	//_aiPlayer = std::make_unique<AiPlayer>();

	_aiThread = std::thread([this]() { for (; _isAppRunning; updateAi()); });
	_renderThread = std::thread([this]() { for (_window.setActive(true); _isAppRunning || !_window.setActive(false); render()); });
}

MainApp::~MainApp()
{
	_aiThread.join();
	_renderThread.join();

	_window.close();

	ImGui::SFML::Shutdown();
}

void MainApp::update()
{
	for (sf::Event event; _window.pollEvent(event);)
	{
		ImGui::SFML::ProcessEvent(event);

		if (event.type == sf::Event::Closed)
		{
			_isAppRunning = false;
		}

		switch (event.type)
		{
		default:
			break;
		case sf::Event::KeyPressed:
			break;
		case sf::Event::KeyReleased:
			break;
		case sf::Event::MouseButtonPressed:
			break;
		case sf::Event::MouseButtonReleased:
			break;
		case sf::Event::MouseMoved:
			break;
		}
	}
}

void MainApp::render()
{
	ImGui::SFML::Update(_window, _renderDeltaClock.restart());

	if (_isGameOver)
	{
		initUi();
	}

	_window.clear(sf::Color::White);

	_window.draw(boardRenderData.linesVertices.data(), boardRenderData.linesVertices.size(), sf::Lines);

	ImGui::SFML::Render(_window);
	_window.display();
}

void MainApp::initUi()
{
	auto winSize = sf::Vector2f(_window.getSize());
	auto buttonSize = sf::Vector2f(winSize.x * 0.1f, winSize.y * 0.1f);

	ImGui::SetNextWindowPos(winSize * 0.5f, ImGuiCond_Always, { 0.5f, 0.5f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 40.f, 40.f });
	ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoNav);
	ImGui::PopStyleVar();

	if (ImGui::Button("Play", buttonSize))
	{
		std::scoped_lock lock(boardState.mutex);

		boardState.cells.assign(boardState.cells.size(), 0);

		_isGameOver = false;
		_isPlayerTurn = isPlayerFirst;
	}

	ImGui::Spacing();
	ImGui::Checkbox("Player First", &isPlayerFirst);
	ImGui::Spacing();

	if (ImGui::Button("Exit", buttonSize))
	{
		_isAppRunning = false;
	}

	//ImGui::ShowDemoWindow();
	ImGui::End();
}

void MainApp::updateAi()
{
	if (!_isGameOver && !_isPlayerTurn)
	{
		std::vector<uint8_t> cells;

		{
			std::scoped_lock lock(boardState.mutex);

			if (_aiPlayer->isGameOver(boardState.cells))
			{
				_isGameOver = true;
				return;
			}

			cells = boardState.cells;
		}

		int movePos = _aiPlayer->getMove(cells);

		{
			std::scoped_lock lock(boardState.mutex);
			boardState.cells[movePos] = aiMark;

			if (_aiPlayer->isGameOver(boardState.cells))
			{
				_isGameOver = true;
				return;
			}
		}

		_isPlayerTurn = true;
	}
}

namespace
{
	void BoardRenderData::init(const sf::Vector2u& windowSize)
	{
		const float rectDim = static_cast<float>(std::min(std::max(windowSize.x, windowSize.y) / 3u, std::min(windowSize.x, windowSize.y) * 3u / 4u));
		const float rectDimHalf = rectDim * 0.5f;
		const float rectDimThird = rectDim / 3.f;

		const auto center = static_cast<sf::Vector2f>(windowSize / 2u);
		const auto lineColor = sf::Color{ 55, 55, 55 };

		const auto leftTopCorner = center - sf::Vector2f(rectDimHalf, rectDimHalf);

		rect = { leftTopCorner, {rectDim, rectDim} };

		const unsigned linesPerDim = 10;
		const int gapsPerDim = linesPerDim - 1;
		const float lineGap = rectDim / gapsPerDim;

		linesVertices.reserve((linesPerDim + linesPerDim / 3 - 1) * 4);

		for (unsigned i = 0; i < linesPerDim; i++)
		{
			linesVertices.push_back({ {leftTopCorner.x, leftTopCorner.y + lineGap * i}, lineColor });
			linesVertices.push_back({ {leftTopCorner.x + rectDim, leftTopCorner.y + lineGap * i}, lineColor });
			linesVertices.push_back({ {leftTopCorner.x + lineGap * i, leftTopCorner.y}, lineColor });
			linesVertices.push_back({ {leftTopCorner.x + lineGap * i, leftTopCorner.y + rectDim}, lineColor });

			if (i > 0 && i < gapsPerDim && i % 3 == 0)
			{
				linesVertices.push_back({ {leftTopCorner.x, leftTopCorner.y + lineGap * i + 1}, lineColor });
				linesVertices.push_back({ {leftTopCorner.x + rectDim, leftTopCorner.y + lineGap * i + 1}, lineColor });
				linesVertices.push_back({ {leftTopCorner.x + lineGap * i + 1, leftTopCorner.y}, lineColor });
				linesVertices.push_back({ {leftTopCorner.x + lineGap * i + 1, leftTopCorner.y + rectDim}, lineColor });
				linesVertices.push_back({ {leftTopCorner.x, leftTopCorner.y + lineGap * i - 1}, lineColor });
				linesVertices.push_back({ {leftTopCorner.x + rectDim, leftTopCorner.y + lineGap * i - 1}, lineColor });
				linesVertices.push_back({ {leftTopCorner.x + lineGap * i - 1, leftTopCorner.y}, lineColor });
				linesVertices.push_back({ {leftTopCorner.x + lineGap * i - 1, leftTopCorner.y + rectDim}, lineColor });
			}
		}

		const float markSize = rectDim * 0.02f;
		const float markWidth = markSize * 0.1f;
		const float radius = markSize * 0.5f - markWidth;

		const auto xMarkColor = sf::Color::Blue;
		const auto oMarkColor = sf::Color::Red;

		crossLines[0].setSize({ markSize, markWidth });
		crossLines[0].setFillColor(xMarkColor);
		crossLines[0].setOrigin(crossLines[0].getSize() * 0.5f);
		crossLines[0].rotate(45);
		crossLines[1] = crossLines[0];
		crossLines[1].rotate(90);

		circle.setRadius(radius);
		circle.setFillColor(sf::Color::Transparent);
		circle.setOutlineColor(oMarkColor);
		circle.setOutlineThickness(markWidth);
		circle.setOrigin({ circle.getRadius(), circle.getRadius() });
	}
}