#pragma once

#include <mutex>
#include <vector>

class IBoard
{
public:
	mutable std::mutex mutex;

	virtual std::vector<uint8_t>& getCellsUnsafe() = 0;

	virtual std::vector<uint8_t> getCellsCopySafe() const = 0;

	virtual void clear() = 0;
};