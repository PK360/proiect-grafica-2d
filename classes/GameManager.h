#pragma once
#include <vector>

class GameManager {
public:
    int cur_player = 1, p1_type = -1, p2_type = -1;
    bool cont_turn = false, foul = false;
    std::vector<int> p1_balls, p2_balls;
    

    void onBallPocketed(int ball) {
        if (ball == 0) { foul = true; return; }
        if (ball == 8) { return; }

        // Assign types on the very first pocketed ball
        int type = (ball >= 1 && ball <= 7) ? 1 : 2;
        if (p1_type == -1 && type != 0) {
            p1_type = (cur_player == 1) ? type : (type == 1 ? 2 : 1);
            p2_type = (p1_type == 1) ? 2 : 1;
        }

        if (type == p1_type) {
            p1_balls.push_back(ball);
            if (cur_player == 1) { cont_turn = true; }
        } else {
            p2_balls.push_back(ball);
            if (cur_player == 2) { cont_turn = true; }
        }
    }
    
    void endTurn() {
        if (foul || !cont_turn) { cur_player = (cur_player == 1) ? 2 : 1; }
        cont_turn = false; foul = false;
    }
    
    [[nodiscard]] int getP1Score() const { return p1_balls.size(); }
    [[nodiscard]] int getP2Score() const { return p2_balls.size(); }
};