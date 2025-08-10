#ifndef GRID_LOOKUP
#define GRID_LOOKUP

#include "particle.h"
#include <vector>
class GridLookup {
public:

	std::vector<std::vector<int>> cells;
	int width{ 0.0 };
	int height{ 0.0 };
	float cellSize{ 0.0f };

	GridLookup(int screen_width, int screen_height, int maxRadius) {

		cellSize = 2 * maxRadius;
		width = ceil(screen_width / cellSize);
		height = ceil(screen_height / cellSize);

		cells.reserve(width * height);
	}

	inline int index(int x, int y) const {
		return y * width + x;
	}

	void buildGrid(const std::vector<Particle>& particles) {

		//clear grid
		for (std::vector<int>& cell : cells) {
			cell.clear();
		}

		glm::vec2 windowSize = particles[0].getWindowSize();
		int offset_x = windowSize.x / 2;
		int offset_y = windowSize.y / 2;

		for (int i = 0, n = particles.size(); i < n; i++) {

			const glm::vec2& pos = particles[i].position;

			int px = floor(pos.x + offset_x/ cellSize);
			int py = floor(pos.y + offset_y/ cellSize);

			px = glm::clamp(px, 0, width - 1);
			py = glm::clamp(py, 0, height - 1);

			cells[index(px, py)].push_back(i);
		}
	}

	void resolveCollisions(std::vector<Particle>& particles) {

		int offsets[9][2] = {
			{-1,-1}, {0,-1}, {1,-1},
			{-1, 0}, {0, 0}, {1, 0},
			{-1, 1}, {0, 1}, {1, 1}
		};

		for (int cy = 0; cy < height; cy++) {
			for (int cx = 0; cx < width; cx++) {

				const std::vector<int>& cell = cells[index(cx, cy)];
				for (int i : cell) {

					auto& p1 = particles[i];
					for (auto& offset : offsets) {

						int nx = cx + offset[0];
						int ny = cy + offset[1];

						if (nx < 0 || ny < 0 || nx >= width || ny >= height)
							continue;

						const std::vector<int>& neighborCell = cells[index(nx, ny)];
						for (int j : neighborCell) {

							if (j <= i) continue;
							auto& p2 = particles[j];
							handleParticleCollisions(p1, p2);
						}
					}
				}
			}
		}
	}

private:


};


#endif // !GRID_LOOKUP
