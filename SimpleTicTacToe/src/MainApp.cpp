#include "MainApp.hpp"
#include "AiPlayer.hpp"

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
		std::vector<sf::Vertex> outerLoopVertices = std::vector<sf::Vertex>(5);
		std::vector<sf::Vertex> innerLinesVertices = std::vector<sf::Vertex>(8);
		sf::Rect<float> rect;
		sf::RectangleShape crossLines[2];
		sf::CircleShape circle;

		void init(const sf::Vector2u& windowSize);
	} boardRenderData;
}

MainApp::MainApp(const sf::Vector2u& windowSize /*= { 1280u, 720u }*/, const std::string& windowTitle /*= "STTT"*/) : IMainApp(windowSize, windowTitle)
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

	_aiPlayer = std::make_unique<AiPlayer>();

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
#ifndef _DEBUG
		if (!_isGameOver && _isPlayerTurn)
#endif // !_DEBUG
		{
			switch (event.type)
			{
			default:
				break;
			case sf::Event::KeyPressed:
#ifdef _DEBUG
				if (event.key.code == sf::Keyboard::Space)
				{
					_isPlayerTurn = !_isPlayerTurn;
				}
				if (event.key.code == sf::Keyboard::R)
				{
					std::scoped_lock lock(boardState.mutex);

					boardState.cells.assign(boardState.cells.size(), 0);

					_isGameOver = false;
				}
#endif // _DEBUG
				break;
			case sf::Event::KeyReleased:
				break;
			case sf::Event::MouseButtonPressed:
			{
				bool left = event.mouseButton.button == sf::Mouse::Left;
				bool right = false;
#ifdef _DEBUG
				right = event.mouseButton.button == sf::Mouse::Right;
#endif // _DEBUG

				if ((left || right) && boardRenderData.rect.contains((float)event.mouseButton.x, (float)event.mouseButton.y))
				{
					const sf::Vector2i pos(event.mouseButton.x - (int)boardRenderData.rect.left, event.mouseButton.y - (int)boardRenderData.rect.top);
					const int i = pos.y / std::lroundf(boardRenderData.rect.height / 3);
					const int j = pos.x / std::lroundf(boardRenderData.rect.width / 3);
					const int index = i * 3 + j;

					assert(index >= 0);

					{
						std::scoped_lock lock(boardState.mutex);

						if (boardState.cells[index] == 0)
						{
							boardState.cells[index] = playerMark;
#ifdef _DEBUG
							if (right)
							{
								boardState.cells[index] = aiMark;
							}
#endif // _DEBUG
						}
					}
#ifndef _DEBUG
					_isPlayerTurn = false;
#endif // !_DEBUG
				}

				break;
			}
			case sf::Event::MouseButtonReleased:
				break;
			case sf::Event::MouseMoved:
				break;
			}
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

	_window.draw(boardRenderData.outerLoopVertices.data(), boardRenderData.outerLoopVertices.size(), sf::LineStrip);
	_window.draw(boardRenderData.innerLinesVertices.data(), boardRenderData.innerLinesVertices.size(), sf::Lines);

	{
		std::scoped_lock lock(boardState.mutex);

		for (uint8_t i = 0; i < boardState.cells.size(); i++)
		{
			const auto& val = boardState.cells[i];

			if (val != 1 && val != 2)
			{
				continue;
			}

			const auto offset = boardRenderData.rect.width / 6.f;
			const auto pos = sf::Vector2f(boardRenderData.rect.left + (1 + 2 * (i % 3)) * offset, boardRenderData.rect.top + (1 + 2 * (i / 3)) * offset);

			if (val == 1)
			{
				auto& shape1 = boardRenderData.crossLines[0];
				shape1.setPosition(pos);
				_window.draw(shape1);
				auto& shape2 = boardRenderData.crossLines[1];
				shape2.setPosition(pos);
				_window.draw(shape2);
			}

			if (val == 2)
			{
				auto& shape = boardRenderData.circle;
				shape.setPosition(pos);
				_window.draw(shape);
			}
		}
	}

	ImGui::SFML::Render(_window);
	_window.display();
}

void MainApp::initUi()
{
	auto winSize = sf::Vector2f(_window.getSize());
	auto buttonSize = sf::Vector2f(winSize.x * 0.1f, winSize.y * 0.1f);

	ImGui::SetNextWindowPos(winSize * 0.5f, ImGuiCond_Always, { 0.5f, 0.5f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 40.f, 40.f });
	ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_NoTitleBar |
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
		const auto outerColor = sf::Color::Black;
		const auto innerColor = sf::Color{ 55, 55, 55 };

		outerLoopVertices[0] = sf::Vertex(center + sf::Vector2f(-rectDimHalf, -rectDimHalf), outerColor);
		outerLoopVertices[1] = sf::Vertex(center + sf::Vector2f(rectDimHalf, -rectDimHalf), outerColor);
		outerLoopVertices[2] = sf::Vertex(center + sf::Vector2f(rectDimHalf, rectDimHalf), outerColor);
		outerLoopVertices[3] = sf::Vertex(center + sf::Vector2f(-rectDimHalf, rectDimHalf), outerColor);
		outerLoopVertices[4] = outerLoopVertices[0];

		const auto& leftTopCorner = outerLoopVertices[0].position;

		rect = { leftTopCorner, {rectDim, rectDim} };

		innerLinesVertices[0] = sf::Vertex(leftTopCorner + sf::Vector2f(rectDimThird, 0.f), innerColor);
		innerLinesVertices[1] = sf::Vertex(innerLinesVertices[0].position + sf::Vector2f(0.f, rectDim), innerColor);
		innerLinesVertices[2] = sf::Vertex(leftTopCorner + sf::Vector2f(2 * rectDimThird, 0.f), innerColor);
		innerLinesVertices[3] = sf::Vertex(innerLinesVertices[2].position + sf::Vector2f(0.f, rectDim), innerColor);
		innerLinesVertices[4] = sf::Vertex(leftTopCorner + sf::Vector2f(0.f, rectDimThird), innerColor);
		innerLinesVertices[5] = sf::Vertex(innerLinesVertices[4].position + sf::Vector2f(rectDim, 0.f), innerColor);
		innerLinesVertices[6] = sf::Vertex(leftTopCorner + sf::Vector2f(0.f, 2 * rectDimThird), innerColor);
		innerLinesVertices[7] = sf::Vertex(innerLinesVertices[6].position + sf::Vector2f(rectDim, 0.f), innerColor);

		const float markSize = rectDim * 0.2f;
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