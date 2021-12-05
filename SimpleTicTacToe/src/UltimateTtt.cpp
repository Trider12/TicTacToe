#include "UltimateTtt.hpp"

#include <mutex>
#include <cassert>

#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include "AiPlayerUltimateTtt.hpp"
#include "MainApp.hpp"

namespace
{
	const uint8_t playerMark = 1, aiMark = 2;

	class UltimateBoard : public IBoard
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
		std::vector<uint8_t> _cells = std::vector<uint8_t>(81);
	};

	struct BoardRenderData
	{
		std::vector<sf::Vertex> linesVertices;
		sf::Rect<float> rect;
		sf::RectangleShape crossLines[2];
		sf::CircleShape circle;

		void init(const sf::Vector2u& windowSize);
	};

	static BoardRenderData boardRenderData;
}

UltimateTtt::UltimateTtt(const sf::Vector2u& windowSize) : IGame(windowSize)
{
	boardRenderData.init(windowSize);

	_board = std::make_unique<UltimateBoard>();
	_aiPlayer = std::make_unique<AiPlayerUltimateTtt>();
}

void UltimateTtt::start(bool isPlayerFirst)
{
	_board->clear();
}

void UltimateTtt::handleInput(const sf::Event& event)
{
	//TODO
}

void UltimateTtt::update(float delta)
{
	//TODO
}

void UltimateTtt::render(sf::RenderTarget& target)
{
	target.draw(boardRenderData.linesVertices.data(), boardRenderData.linesVertices.size(), sf::Lines);
}

void UltimateTtt::updateAi()
{
	//TODO
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