#ifndef GRID_LOOKUP
#define GRID_LOOKUP

#include "particle.h"
#include <vector>

class GridLookup {
public:

	std::vector<std::vector<int>> cells;
	int width{ 0 };
	int height{ 0 };
	float cellSize{ 0.0f };

	GridLookup(int screen_width, int screen_height, int maxRadius) {

		cellSize = 2 * maxRadius;
		width = ceil(screen_width / cellSize);
		height = ceil(screen_height / cellSize);

		std::cout << width << ' ' << height << '\n';

		cells.resize(width * height);
	}

	inline int index(int x, int y) const {
		return y * width + x;
	}

	void updateGrid(int maxRadius, glm::vec2 windowSize) {
		cellSize = 2 * maxRadius;
		width = ceil(windowSize[0] / cellSize);
		height = ceil(windowSize[1] / cellSize);
		cells.resize(width * height);
	}

	void buildGrid(const std::vector<Particle>& particles, int maxRadius, glm::vec2 windowSize) {

		//clear grid
		for (std::vector<int>& cell : cells) {
			cell.clear();
		}

		//update grid
		//updateGrid(maxRadius, windowSize);

		//offset calc
		float offset_x = windowSize.x / 2.0f;
		float offset_y = windowSize.y / 2.0f;

		for (int i = 0, n = particles.size(); i < n; i++) {

			const glm::vec2& pos = particles[i].position;

			int px = floor((pos.x + windowSize[0] / 2.0f) / cellSize);
			int py = floor((pos.y + windowSize[1] / 2.0f) / cellSize);

			px = glm::clamp(px, 0, width - 1);
			py = glm::clamp(py, 0, height - 1);

			cells[index(px, py)].push_back(i);

			if (px >= width || px < 0)
				std::cout << pos.x << ' ' << px << ' ';
			if (py >= height || py < 0)
				std::cout << pos.y << ' ' << py << '\n';
			if (index(px, py) >= cells.size())
				std::cout << pos.x << ' ' << pos.y << ' ' << px << ' ' << py <<  ' ' << index(px, py) << ' ' << cells.size() << '\n';
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

	//Needs to be revamped
	void resolveGravity(std::vector<Particle>& particles) {

		std::vector<std::pair<int, int>> offsets;
		int size = 20;
		offsets.reserve(size * size);

		for (int dy = -size; dy <= size; dy++) {
			for (int dx = -size; dx <= size; dx++) {
				offsets.emplace_back(dx, dy);
			}
		}

		for (int cy = 0; cy < height; cy++) {
			for (int cx = 0; cx < width; cx++) {

				const std::vector<int>& cell = cells[index(cx, cy)];
				for (int i : cell) {

					auto& p1 = particles[i];
					for (auto& offset : offsets) {

						int nx = cx + offset.first;
						int ny = cy + offset.second;

						if (nx < 0 || ny < 0 || nx >= width || ny >= height)
							continue;

						const std::vector<int>& neighborCell = cells[index(nx, ny)];
						for (int j : neighborCell) {

							if (j <= i) continue;
							auto& p2 = particles[j];

							handleGravity(p1, p2);
						}
					}
				}
			}
		}
	}

private:
	

};


#endif // !GRID_LOOKUP
