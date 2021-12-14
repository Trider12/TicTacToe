#include "SimpleTTT.hpp"

#include <mutex>
#include <cassert>

#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include "AiPlayerSimpleTtt.hpp"
#include "MainApp.hpp"

namespace
{
	const uint8_t playerMark = 1, aiMark = 2;

	class SimpleBoard : public IBoard
	{
		std::vector<uint8_t>& getCellsUnsafe() override
		{
			return _cells;
		}

		std::vector<uint8_t> getCellsCopySafe() const override
		{
			std::scoped_lock lock(mutex);
			return _cells;
		}

		void clear() override
		{
			std::scoped_lock lock(mutex);
			_cells.assign(_cells.size(), 0);
		}

	private:
		std::vector<uint8_t> _cells = std::vector<uint8_t>(9);
	};

	struct BoardRenderData
	{
		void init(const sf::Vector2u& windowSize);

		std::vector<sf::Vertex> outerLoopVertices = std::vector<sf::Vertex>(5);
		std::vector<sf::Vertex> innerLinesVertices = std::vector<sf::Vertex>(8);
		sf::Rect<float> rect;
		sf::RectangleShape crossLines[2];
		sf::CircleShape circle;
	};

	static BoardRenderData boardRenderData;
	static std::mutex aiThreadMutex;
}

SimpleTtt::SimpleTtt(const sf::Vector2u& windowSize) : IGame(windowSize)
{
	boardRenderData.init(windowSize);

	_board = std::make_unique<SimpleBoard>();
	_aiPlayer = std::make_unique<AiPlayerSimpleTtt>();
}

void SimpleTtt::reset(bool isPlayerFirst)
{
	_board->clear();

	std::scoped_lock(aiThreadMutex);
	_isGameOver = false;
	_isPlayerTurn = isPlayerFirst;
	_aiExecuteCondition.notify_all();
}

void SimpleTtt::input(const sf::Event& event)
{
	if (!_isGameOver && (_isPlayerTurn || MainApp::debugPlayEnabled))
	{
		switch (event.type)
		{
		default:
			break;
		case sf::Event::KeyPressed:
			if (MainApp::debugPlayEnabled)
			{
				if (event.key.code == sf::Keyboard::Space)
				{
					std::scoped_lock(aiThreadMutex);
					_isPlayerTurn = !_isPlayerTurn;
					_aiExecuteCondition.notify_all();
				}
				if (event.key.code == sf::Keyboard::R)
				{
					reset(_isPlayerTurn);
				}
			}

			break;
		case sf::Event::KeyReleased:
			break;
		case sf::Event::MouseButtonPressed:
		{
			bool left = event.mouseButton.button == sf::Mouse::Left;
			bool right = MainApp::debugPlayEnabled && event.mouseButton.button == sf::Mouse::Right;

			if ((left || right) && boardRenderData.rect.contains((float)event.mouseButton.x, (float)event.mouseButton.y))
			{
				const sf::Vector2i pos(event.mouseButton.x - (int)boardRenderData.rect.left, event.mouseButton.y - (int)boardRenderData.rect.top);
				const int i = pos.y / std::lroundf(boardRenderData.rect.height / 3);
				const int j = pos.x / std::lroundf(boardRenderData.rect.width / 3);
				const int index = i * 3 + j;

				assert(index >= 0);

				{
					std::scoped_lock lock(_board->mutex);

					auto& cells = _board->getCellsUnsafe();

					if (cells[index] == 0)
					{
						cells[index] = left ? playerMark : right ? aiMark : 0;
					}
				}

				if (!MainApp::debugPlayEnabled)
				{
					std::scoped_lock(aiThreadMutex);
					_isPlayerTurn = false;
					_aiExecuteCondition.notify_all();
				}
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

void SimpleTtt::update(float delta)
{
}

void SimpleTtt::render(sf::RenderTarget& target)
{
	target.draw(boardRenderData.outerLoopVertices.data(), boardRenderData.outerLoopVertices.size(), sf::LineStrip);
	target.draw(boardRenderData.innerLinesVertices.data(), boardRenderData.innerLinesVertices.size(), sf::Lines);

	auto cellsCopy = _board->getCellsCopySafe();

	for (uint8_t i = 0; i < cellsCopy.size(); i++)
	{
		const auto& val = cellsCopy[i];

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
			target.draw(shape1);
			auto& shape2 = boardRenderData.crossLines[1];
			shape2.setPosition(pos);
			target.draw(shape2);
		}

		if (val == 2)
		{
			auto& shape = boardRenderData.circle;
			shape.setPosition(pos);
			target.draw(shape);
		}
	}
}

void SimpleTtt::updateAi()
{
	std::unique_lock lock(aiThreadMutex);
	_aiExecuteCondition.wait(lock, [this]() { return !_isGameOver && !_isPlayerTurn || _isExiting; });

	{
		auto cellsCopy = _board->getCellsCopySafe();

		if (_isGameOver = _aiPlayer->isGameOver(cellsCopy))
		{
			return;
		}

		int movePos = _aiPlayer->getMove(cellsCopy);
		std::scoped_lock lock(_board->mutex);
		_board->getCellsUnsafe()[movePos] = aiMark;
	}

	if (_isGameOver = _aiPlayer->isGameOver(_board->getCellsCopySafe()))
	{
		return;
	}

	_isPlayerTurn = true;
}

void SimpleTtt::exit()
{
	std::scoped_lock(aiThreadMutex);
	_isExiting = true;
	_aiExecuteCondition.notify_all();
}

namespace
{
	void BoardRenderData::init(const sf::Vector2u& windowSize)
	{
		const float rectDim = static_cast<float>(std::min(std::max(windowSize.x, windowSize.y) / 3u, std::min(windowSize.x, windowSize.y) * 3u / 4u));
		const float rectDimHalf = rectDim * 0.5f;
		const float rectDimThird = rectDim / 3.f;

		const auto center = static_cast<sf::Vector2f>(windowSize / 2u);
		const auto lineColor = sf::Color::Black;

		outerLoopVertices[0] = sf::Vertex(center + sf::Vector2f(-rectDimHalf, -rectDimHalf), lineColor);
		outerLoopVertices[1] = sf::Vertex(center + sf::Vector2f(rectDimHalf, -rectDimHalf), lineColor);
		outerLoopVertices[2] = sf::Vertex(center + sf::Vector2f(rectDimHalf, rectDimHalf), lineColor);
		outerLoopVertices[3] = sf::Vertex(center + sf::Vector2f(-rectDimHalf, rectDimHalf), lineColor);
		outerLoopVertices[4] = outerLoopVertices[0];

		const auto& leftTopCorner = outerLoopVertices[0].position;

		rect = { leftTopCorner, {rectDim, rectDim} };

		innerLinesVertices[0] = sf::Vertex(leftTopCorner + sf::Vector2f(rectDimThird, 0.f), lineColor);
		innerLinesVertices[1] = sf::Vertex(innerLinesVertices[0].position + sf::Vector2f(0.f, rectDim), lineColor);
		innerLinesVertices[2] = sf::Vertex(leftTopCorner + sf::Vector2f(2 * rectDimThird, 0.f), lineColor);
		innerLinesVertices[3] = sf::Vertex(innerLinesVertices[2].position + sf::Vector2f(0.f, rectDim), lineColor);
		innerLinesVertices[4] = sf::Vertex(leftTopCorner + sf::Vector2f(0.f, rectDimThird), lineColor);
		innerLinesVertices[5] = sf::Vertex(innerLinesVertices[4].position + sf::Vector2f(rectDim, 0.f), lineColor);
		innerLinesVertices[6] = sf::Vertex(leftTopCorner + sf::Vector2f(0.f, 2 * rectDimThird), lineColor);
		innerLinesVertices[7] = sf::Vertex(innerLinesVertices[6].position + sf::Vector2f(rectDim, 0.f), lineColor);

		const float markSize = rectDim * 0.2f;
		const float markWidth = markSize * 0.1f;
		const float radius = markSize * 0.5f - markWidth;

		const auto xMarkColor = sf::Color::Blue;
		const auto oMarkColor = sf::Color::Red;

		crossLines[0].setFillColor(xMarkColor);
		crossLines[0].setSize({ markSize, markWidth });
		crossLines[0].setOrigin(crossLines[0].getSize() * 0.5f);
		crossLines[0].setRotation(0);
		crossLines[0].rotate(45);
		crossLines[1] = crossLines[0];
		crossLines[1].rotate(90);

		circle.setFillColor(sf::Color::Transparent);
		circle.setOutlineColor(oMarkColor);
		circle.setOutlineThickness(markWidth);
		circle.setRadius(radius);
		circle.setOrigin({ circle.getRadius(), circle.getRadius() });
	}
}