#include "AiPlayerSimpleTtt.hpp"

#include <algorithm>
#include <cassert>

namespace
{
	const int playerMark = 1, aiMark = 2;

	static int getBoardScore(const std::vector<uint8_t>& boardCells, int depth)
	{
		const int maxValue = 20 - depth;
		const int minValue = depth - 20;

		for (uint8_t i = 0; i < 3; i++)
		{
			if (boardCells[i * 3] != 0 && boardCells[i * 3] == boardCells[i * 3 + 1] && boardCells[i * 3] == boardCells[i * 3 + 2])
			{
				return boardCells[i * 3] == aiMark ? maxValue : minValue;
			}

			if (boardCells[i] != 0 && boardCells[i] == boardCells[i + 3] && boardCells[i] == boardCells[i + 6])
			{
				return boardCells[i] == aiMark ? maxValue : minValue;
			}
		}

		if (boardCells[4] != 0 && (
			boardCells[0] == boardCells[4] && boardCells[4] == boardCells[8] ||
			boardCells[2] == boardCells[4] && boardCells[4] == boardCells[6]))
		{
			return boardCells[4] == aiMark ? maxValue : minValue;
		}

		return 0;
	}

	static int minimax(std::vector<uint8_t>& boardCells, int depth, bool max);

	static int makeMaxMove(std::vector<uint8_t>& boardCells, int depth)
	{
		int bestScore = std::numeric_limits<int>::min();

		for (auto& val : boardCells)
		{
			if (val == 0)
			{
				val = aiMark;
				bestScore = std::max(bestScore, minimax(boardCells, depth + 1, false));
				val = 0;
			}
		}

		return bestScore;
	}

	static int makeMinMove(std::vector<uint8_t>& boardCells, int depth)
	{
		int bestScore = std::numeric_limits<int>::max();

		for (auto& val : boardCells)
		{
			if (val == 0)
			{
				val = playerMark;
				bestScore = std::min(bestScore, minimax(boardCells, depth + 1, true));
				val = 0;
			}
		}

		return bestScore;
	}

	static int minimax(std::vector<uint8_t>& boardCells, int depth, bool max)
	{
		if (auto score = getBoardScore(boardCells, depth); score != 0 ||
			std::none_of(boardCells.begin(), boardCells.end(), [](const auto& val) { return val == 0; }))
		{
			return score;
		}

		return max ? makeMaxMove(boardCells, depth) : makeMinMove(boardCells, depth);
	}
}

int AiPlayerSimpleTtt::getMove(std::vector<uint8_t> boardCells)
{
	int move = -1;
	int bestScore = std::numeric_limits<int>::min();

	for (uint8_t i = 0; i < boardCells.size(); i++)
	{
		auto& val = boardCells[i];

		if (val == 0)
		{
			val = aiMark;
			auto score = minimax(boardCells, 0, false);
			val = 0;

			if (score > bestScore)
			{
				bestScore = score;
				move = i;
			}
		}
	}

	assert(move >= 0);

	return move;
}

bool AiPlayerSimpleTtt::isGameOver(const std::vector<uint8_t>& boardCells)
{
	return getBoardScore(boardCells, 0) != 0 || std::none_of(boardCells.begin(), boardCells.end(), [](const auto& val) { return val == 0; });
}