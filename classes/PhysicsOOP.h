#include <vector>
#include <algorithm>
#include <cmath>
#include "global.h"
#include "Ball.h"

class PhysicsOOP
{
	int cols, rows;
	float cellSize, sqr_radius;
	float frict = 0.8f, stop_move = 9.0f;
	float circ = 2.0f * global::PI * global::ballSize;

	std::vector<Ball*> shapes;
	std::vector<std::vector<int>> grid;

public:
	PhysicsOOP() {}
    void init() {
        cellSize = 2 * global::ballSize;
        sqr_radius = cellSize * cellSize;

        float tableWidth = global::tableMaxX - global::tableMinX;
        float tableHeight = global::tableMaxY - global::tableMinY;

        cols = (int)(tableWidth / cellSize) + 1;
        rows = (int)(tableHeight / cellSize) + 1;
        grid.assign(rows * cols, {});
    }

	~PhysicsOOP() { for (Ball* shape : shapes) { delete shape; } }



	// getters & setters
	[[nodiscard]] const std::vector<std::vector<int>>& getGrid() const { return grid; }
	[[nodiscard]] const std::vector<Ball*>& getShapes() const { return shapes; }
	[[nodiscard]] float getCellSize() const { return cellSize; }
	[[nodiscard]] int getCols() const { return cols; }
	[[nodiscard]] int getRows() const { return rows; }
	[[nodiscard]] float getSquaredRadius() const { return sqr_radius; }

	void setGrid(const std::vector<std::vector<int>>& grd) { this->grid = grd; }
	void setShapes(const std::vector<Ball*>& sh) { this->shapes = sh; }
	void setCellSize(float cell_size) { cellSize = cell_size; }
	void setCols(int cls) { this->cols = cls; }
	void setRows(int rws) { this->rows = rws; }
	void setSquaredRadius(float sqr_rad) { sqr_radius = sqr_rad; }



	// functions
	void update(float deltatime) {
        updateGrid();
        updateCollidedCircles();

        float radius = global::ballSize, pocketSize = global::pocketSize;
        
		float frict_mult = frict * deltatime;
        float min_x = global::tableMinX, max_x = global::tableMaxX;
        float min_y = global::tableMinY, max_y = global::tableMaxY;
        float mid_x = (min_x + max_x) / 2.0f;

        struct Segment { Vec2f a, b; };
        Segment walls[6] = {
            {{min_x, min_y + pocketSize}, {min_x, max_y - pocketSize}}, // stanga
            {{max_x, min_y + pocketSize}, {max_x, max_y - pocketSize}}, // dreapta
            {{min_x + pocketSize, min_y}, {mid_x - pocketSize, min_y}}, // st. jos
            {{mid_x + pocketSize, min_y}, {max_x - pocketSize, min_y}}, // dr. jos
            {{min_x + pocketSize, max_y}, {mid_x - pocketSize, max_y}}, // st. sus
            {{mid_x + pocketSize, max_y}, {max_x - pocketSize, max_y}}  // dr. sus
        };

        for (Ball* shape : shapes) {
			if (shape->isActive()) {
				Vec2f pos = shape->getPosition(), speed = shape->getSpeed();

				// friction
				speed.x -= speed.x * frict_mult;
				speed.y -= speed.y * frict_mult;

				// null movement
				if (speed.sqrMagnitude() < stop_move) { speed.x = 0.0f; speed.y = 0.0f; }

				float moveX = speed.x * deltatime, moveY = speed.y * deltatime;
				pos.x += moveX; pos.y += moveY;
				shape->add_uv(Vec2f(-moveX / circ, moveY / circ));

				// pocket check
				{
					bool inPocket = false;
					for (const auto& p : global::pockets) {
						float dist = glm::distance(glm::vec2(pos), p);
						if (dist < global::pocketSize - global::ballSize) {
							inPocket = true;
							break;
						}
					}

					if (inPocket) {
						if (shape->getNumber() == 0) { // bila alba
							shape->setPosition({global::tableMinX + global::table_w * 0.25f, global::table_center_y});
							shape->setSpeed({0.0f, 0.0f});

							global::aiming = false; global::cueStriking = false;
							break;
						} else { // alte bile
							shape->setSpeed({0.0f, 0.0f});
							shape->setActive(false);

							break;
						}
					}
				}

				// collision
				for (int i = 0; i < 6; i++) {
					Vec2f a = walls[i].a, b = walls[i].b;
					float ab_x = b.x - a.x, ab_y = b.y - a.y;

					// segment projection point
					float t = ((pos.x - a.x) * ab_x + (pos.y - a.y) * ab_y) / (ab_x * ab_x + ab_y * ab_y);
					t = std::clamp(t, 0.0f, 1.0f);

					// determine distance from projection to center
					float close_x = a.x + t * ab_x, close_y = a.y + t * ab_y;
					float dx = pos.x - close_x, dy = pos.y - close_y;
					float dist_sqr = dx * dx + dy * dy;

					// check if distance is < radius
					if (dist_sqr < radius * radius && dist_sqr > 0.0001f) {
						float dist = std::sqrt(dist_sqr);
						
						// normala distantei
						float nx = dx / dist, ny = dy / dist;

						float overlap = radius - dist;
						pos.x += nx * overlap; pos.y += ny * overlap;

						// push ball back
						float prod = speed.x * nx + speed.y * ny;
						if (prod < 0) { speed.x -= 2.0f * prod * nx; speed.y -= 2.0f * prod * ny; }
					}
				}

				shape->setPosition(pos);
				shape->setSpeed(speed);
			}
		}
    }

	void updateGrid() {
		for (auto& gridCell : grid) { gridCell.clear(); }

		int shape_nr = shapes.size();
		for (int i = 0; i < shape_nr; i++) { grid[cellIndex(shapes[i]->getPosition())].push_back(i); }
	}

	int cellIndex(Vec2f position) const {
		int x = std::clamp(int((position.x - global::tableMinX) / cellSize), 0, cols - 1);
		int y = std::clamp(int((position.y - global::tableMinY) / cellSize), 0, rows - 1);
		return x + y * cols;
	}

	void checkBetweenCells(std::vector<int>& cell, int x, int y) {
	    if (x < 0 || x >= cols || y >= rows)
	        return;
	    std::vector<int>& neighbor = grid[x + cols * y];
	    for (int a : cell)
	        for (int b : neighbor)
	            resolveColision(*shapes[a], *shapes[b]);
	}

	void updateCollidedCircles() {
	    for (int y = 0; y < rows; y++) {
	        for (int x = 0; x < cols; x++) {
	            std::vector<int>& cell = grid[x + cols * y];
	            if (cell.empty()) continue;

	            for (int k = 0; k < (int)cell.size(); k++)
	                for (int w = k + 1; w < (int)cell.size(); w++)
	                    resolveColision(*shapes[cell[k]], *shapes[cell[w]]);

	            checkBetweenCells(cell, x + 1, y);
	            checkBetweenCells(cell, x - 1, y + 1);
	            checkBetweenCells(cell, x, y + 1);
	            checkBetweenCells(cell, x + 1, y + 1);
	        }
	    }
	}

	void resolveColision(Ball& s1, Ball& s2) {
	    Vec2f p1 = s1.getPosition(), p2 = s2.getPosition();
	    float dirX = p2.x - p1.x, dirY = p2.y - p1.y;
	    float dist_sqr = dirX * dirX + dirY * dirY;

		// verifica daca cele doua cercuri sunt in coliziune
	    if (dist_sqr >= sqr_radius) { return; }

	    // distanta necesara a.i. sa nu mai colizioneze
	    float dist = std::sqrt(dist_sqr);
	    float nx = 1.0f, ny = 0.0f;

	    // normala directiei daca se poate calcula
	    if (dist > 0.0001f) { nx = dirX / dist; ny = dirY / dist; }

	    //le pozitionez pe media overlapului ca sa nu mai fie una in alta.
	    float overlap = cellSize - dist;
	    s1.setPosition(Vec2f(p1.x - nx * overlap / 2.0f, p1.y - ny * overlap / 2.0f));
	    s2.setPosition(Vec2f(p2.x + nx * overlap / 2.0f, p2.y + ny * overlap / 2.0f));

	    // puterea de apropiere una spre alta
	    Vec2f v1 = s1.getSpeed(), v2 = s2.getSpeed();
	    float relVel = (v2.x - v1.x) * nx + (v2.y - v1.y) * ny;
	    if (relVel > 0) { return; }

		// aplic impulsul in sensul opus
	    float power = -1.99f * relVel / 2.0f;
	    s1.setSpeed(Vec2f(v1.x - power * nx, v1.y - power * ny));
	    s2.setSpeed(Vec2f(v2.x + power * nx, v2.y + power * ny));
	}


	void addCircles(const std::vector<Ball*>& addedShapes) {
		shapes.insert(shapes.end(), addedShapes.begin(), addedShapes.end());
	}

	void addCircle(Ball* cerc) {
		shapes.push_back(cerc);
	}

	void changeNumberOfCircles(int number) {
		if (number > shapes.size()) {
			for (int i = 0; i < number - shapes.size();i++) {
				addCircle(new Ball(
					i + 1, true,
					{ randomFloat(200,1700), randomFloat(200,800) },
					{ randomFloat(-20, 20), randomFloat(-20, 20) }
				));
			}
		}
		else {
			for (int i = shapes.size() - 1; i >= number;i--) {
				delete shapes[i];
				shapes.pop_back();
			}
		}
	}
};