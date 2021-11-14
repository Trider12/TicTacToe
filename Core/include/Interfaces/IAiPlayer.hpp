#pragma once

#include <vector>

class IAiPlayer
{
public:
	virtual int getMove(std::vector<uint8_t> boardCells) = 0;
	virtual bool isGameOver(const std::vector<uint8_t>& boardCells) = 0;
};