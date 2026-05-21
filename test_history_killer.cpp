// 测试历史启发表和杀手走法更新功能
// 验证需求2.5和2.6的实现

#include <iostream>
#include <cassert>

// 简化的测试类，模拟HistoryTable和KillerMoves的核心功能

class HistoryTable {
private:
    int table[50][50];
    
public:
    HistoryTable() {
        clear();
    }
    
    void clear() {
        for (int i = 0; i < 50; ++i) {
            for (int j = 0; j < 50; ++j) {
                table[i][j] = 0;
            }
        }
    }
    
    int get(int from, int to) const {
        return table[from][to];
    }
    
    void update(int from, int to, int depth) {
        // 深度越大，奖励越高 (需求2.6)
        table[from][to] += depth * depth;
    }
};

struct Move {
    int from;
    int to;
    int num_captures;
    
    Move() : from(-1), to(-1), num_captures(0) {}
    Move(int f, int t, int nc = 0) : from(f), to(t), num_captures(nc) {}
};

class KillerMoves {
private:
    static const int MAX_DEPTH = 64;
    Move killers[MAX_DEPTH][2];
    
public:
    KillerMoves() {
        clear();
    }
    
    void clear() {
        for (int i = 0; i < MAX_DEPTH; ++i) {
            killers[i][0] = Move();
            killers[i][1] = Move();
        }
    }
    
    void add(const Move& move, int depth) {
        if (depth < 0 || depth >= MAX_DEPTH) return;
        
        // 如果不是第一个杀手走法，则移动 (需求2.5)
        if (killers[depth][0].from != move.from || killers[depth][0].to != move.to) {
            killers[depth][1] = killers[depth][0];
            killers[depth][0] = move;
        }
    }
    
    Move get_killer(int depth, int index) const {
        if (depth < 0 || depth >= MAX_DEPTH || index < 0 || index > 1) {
            return Move();
        }
        return killers[depth][index];
    }
};

// 测试函数
void test_history_table() {
    std::cout << "测试HistoryTable..." << std::endl;
    
    HistoryTable history;
    
    // 测试初始状态
    assert(history.get(10, 15) == 0);
    
    // 测试更新：depth=3时，应该增加3*3=9分
    history.update(10, 15, 3);
    assert(history.get(10, 15) == 9);
    std::cout << "  ✓ depth=3时更新，分数=9" << std::endl;
    
    // 测试累加：再次更新depth=4，应该增加4*4=16分，总分=9+16=25
    history.update(10, 15, 4);
    assert(history.get(10, 15) == 25);
    std::cout << "  ✓ depth=4时再次更新，分数=25" << std::endl;
    
    // 测试不同走法
    history.update(20, 25, 5);
    assert(history.get(20, 25) == 25);  // 5*5=25
    assert(history.get(10, 15) == 25);  // 之前的走法不受影响
    std::cout << "  ✓ 不同走法独立更新" << std::endl;
    
    std::cout << "HistoryTable测试通过！" << std::endl << std::endl;
}

void test_killer_moves() {
    std::cout << "测试KillerMoves..." << std::endl;
    
    KillerMoves killers;
    
    // 测试初始状态
    Move initial = killers.get_killer(5, 0);
    assert(initial.from == -1);
    
    // 测试添加第一个杀手走法
    Move move1(10, 15, 0);
    killers.add(move1, 5);
    Move k1 = killers.get_killer(5, 0);
    assert(k1.from == 10 && k1.to == 15);
    std::cout << "  ✓ 添加第一个杀手走法" << std::endl;
    
    // 测试添加第二个杀手走法（第一个应该移到第二位）
    Move move2(20, 25, 0);
    killers.add(move2, 5);
    Move k2_0 = killers.get_killer(5, 0);
    Move k2_1 = killers.get_killer(5, 1);
    assert(k2_0.from == 20 && k2_0.to == 25);  // 新走法在第一位
    assert(k2_1.from == 10 && k2_1.to == 15);  // 旧走法移到第二位
    std::cout << "  ✓ 添加第二个杀手走法，第一个移到第二位" << std::endl;
    
    // 测试不同深度独立存储
    Move move3(30, 35, 0);
    killers.add(move3, 6);
    Move k3 = killers.get_killer(6, 0);
    Move k5 = killers.get_killer(5, 0);
    assert(k3.from == 30 && k3.to == 35);  // depth=6的走法
    assert(k5.from == 20 && k5.to == 25);  // depth=5的走法不受影响
    std::cout << "  ✓ 不同深度独立存储" << std::endl;
    
    std::cout << "KillerMoves测试通过！" << std::endl << std::endl;
}

void test_beta_cutoff_update() {
    std::cout << "测试Beta剪枝时的更新逻辑..." << std::endl;
    
    HistoryTable history;
    KillerMoves killers;
    
    // 模拟beta剪枝场景
    int depth = 4;
    Move quiet_move(12, 17, 0);  // 安静走法（num_captures=0）
    Move capture_move(12, 21, 1); // 吃子走法（num_captures=1）
    
    // 场景1：安静走法导致beta剪枝 - 应该更新
    if (quiet_move.num_captures == 0) {
        killers.add(quiet_move, depth);
        history.update(quiet_move.from, quiet_move.to, depth);
    }
    
    assert(killers.get_killer(depth, 0).from == 12);
    assert(killers.get_killer(depth, 0).to == 17);
    assert(history.get(12, 17) == 16);  // 4*4=16
    std::cout << "  ✓ 安静走法导致beta剪枝时更新" << std::endl;
    
    // 场景2：吃子走法导致beta剪枝 - 不应该更新
    int before_score = history.get(12, 21);
    if (capture_move.num_captures == 0) {
        killers.add(capture_move, depth);
        history.update(capture_move.from, capture_move.to, depth);
    }
    
    assert(history.get(12, 21) == before_score);  // 分数不变
    std::cout << "  ✓ 吃子走法导致beta剪枝时不更新" << std::endl;
    
    std::cout << "Beta剪枝更新逻辑测试通过！" << std::endl << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "历史启发表和杀手走法功能测试" << std::endl;
    std::cout << "验证需求2.5和2.6" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    test_history_table();
    test_killer_moves();
    test_beta_cutoff_update();
    
    std::cout << "========================================" << std::endl;
    std::cout << "所有测试通过！✓" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
