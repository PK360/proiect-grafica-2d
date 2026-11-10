namespace global {
	inline constexpr float PI = 3.141592;
	inline float ballSize = 15.0f;
	inline float pocketSize = ballSize * 1.65f;

	inline bool aiming = false, cueStriking = false;


	inline float win_w = 1000.0f, win_h = 750.0f, winpos_x = 100.0f, winpos_y = 100.0f;

	inline float tableMinX = -400.0f, tableMaxX = 400.0f;
    inline float tableMinY = -200.0f, tableMaxY = 200.0f;
		
	inline glm::vec2 range_x{-500.0f, 500.0f}, range_y{-375.0f, 375.0f};
	inline glm::vec2 delta{range_x.y - range_x.x, range_y.y - range_y.x};

	inline float table_w = global::tableMaxX - global::tableMinX, table_h = global::tableMaxY - global::tableMinY;
	inline float table_center_x = (global::tableMinX + global::tableMaxX) / 2.0f;
	inline float table_center_y = (global::tableMinY + global::tableMaxY) / 2.0f;

	inline std::vector<glm::vec2> pockets = {
		{global::tableMinX, global::tableMaxY}, // stanga sus
		{global::tableMinX, global::tableMinY}, // stanga jos
		{table_center_x, global::tableMaxY},    // centru sus
		{table_center_x, global::tableMinY},    // centru jos
		{global::tableMaxX, global::tableMaxY}, // dreapta sus
		{global::tableMaxX, global::tableMinY}  // dreapta jos
	};
};