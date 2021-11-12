#pragma once

#include <vector>

class AiPlayer
{
public:
	int getMove(std::vector<uint8_t> boardCells);
	bool isGameOver(const std::vector<uint8_t>& boardCells);
};