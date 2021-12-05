#pragma once

#include "Interfaces/IAiPlayer.hpp"

class AiPlayerUltimateTtt : public IAiPlayer
{
public:
	int getMove(std::vector<uint8_t> boardCells) override;
	bool isGameOver(const std::vector<uint8_t>& boardCells) override;
};