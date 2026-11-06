namespace global {
	inline constexpr float PI = 3.141592;
	inline float ballSize = 15.0f;

	inline float win_w = 1000.0f, win_h = 750.0f, winpos_x = 100.0f, winpos_y = 100.0f;

	inline float tableMinX = -400.0f, tableMaxX = 400.0f;
    inline float tableMinY = -200.0f, tableMaxY = 200.0f;
		
	inline glm::vec2 range_x{-500.0f, 500.0f}, range_y{-375.0f, 375.0f};
	inline glm::vec2 delta{range_x.y - range_x.x, range_y.y - range_y.x};


};