#include <cmath>
#include <random>

namespace ourMath {
	struct Vec2f {
		float x, y;
		
		Vec2f() { x = 0;y = 0; }
		Vec2f(float x, float y) { this->x = x; this->y = y; };
		
		Vec2f operator+(const Vec2f& other) { return Vec2f(x + other.x, y + other.y); }
		Vec2f operator*(float scalar) const { return Vec2f(x * scalar, y * scalar); }
		Vec2f operator*(const Vec2f& other) { return Vec2f(x * other.x, y * other.y); }
		float magnitude() const { return sqrt(x * x + y * y); }
		float sqrMagnitude() const { return x * x + y * y; }
		
		operator glm::vec2() { return glm::vec2(x, y); }
	};

	inline std::mt19937& rng() {
		static std::mt19937 gen(std::random_device{}());
		return gen;
	}

	inline float randomFloat(float min, float max) {
		std::uniform_real_distribution<float> dist(min, max);
		return dist(rng());
	}

	inline int randomInt(int min, int max) {
		std::uniform_int_distribution<int> dist(min, max);
		return dist(rng());
	}
};

