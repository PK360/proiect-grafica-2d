#include "ourMath.h"
using namespace ourMath;

class Ball {
	unsigned int number;
	Vec2f position, speed, uv_offset{0.0f, 0.0f};
public:
	Ball(unsigned int nr, Vec2f pos, Vec2f spd = Vec2f()) : number(nr), position(pos), speed(spd) {}
	~Ball() = default;
	
	void setPosition(const Vec2f& pos) { this->position = pos; }
	void setSpeed(const Vec2f& spd) { this->speed = spd; }
	
    const Vec2f& getUVOffset() const { return uv_offset; }
	const Vec2f& getPosition() { return position; }
	const Vec2f& getSpeed() { return speed; }
	
	// functions
	void move(const Vec2f& offset) { position = position + offset; }
	bool isFull() { return number < 8; }
	void add_uv(const Vec2f& offset) {
		uv_offset = uv_offset + offset;
		uv_offset.x = uv_offset.x - std::floor(uv_offset.x);
	    uv_offset.y = uv_offset.y - std::floor(uv_offset.y);
	}
};

