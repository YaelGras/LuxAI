#include "Tiles.h"

#include <numeric>

#include "lux/World/Coordinates.h"

float CityTile::getFuelConsumption() const noexcept {
	const auto neighbour = Coordinates::getNeighborsTile(tilePos);
	return std::accumulate(neighbour.begin(), neighbour.end(), 23.0f,
		[this](float a, const iVec2& b) { return a - (m_city->contains(b) ? 5 : 0); });
}