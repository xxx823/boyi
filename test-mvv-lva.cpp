#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>

// 简化的Move结构用于测试
struct Move {
    int from;
    int to;
    int captures[20];
    int num_captures;
    int score;
    std::string description;
    
    Move(int f, int t, const std::string& desc) 
        : from(f), to(t), num_captures(0), score(0), description(desc) {
        for (int i = 0; i < 20; ++i) {
            captures[i] = -1;
        }
    }
    
    void add_capture(int square) {
        if (num_captures < 20) {
            captures[num_captures++] = square;
        }
    }
};

// 模拟棋盘状态
struct TestBoard {
    uint64_t black_kings;
    uint64_t white_kings;
    int current_player;  // 1=黑方, -1=白方
    
    TestBoard() : black_kings(0), white_kings(0), current_player(1) {}
};

// MVV-LVA评分函数（与boyi.cpp中的实现一致）
int calculate_mvv_lva_score(const Move& move, const TestBoard& board) {
    int mvv_lva_score = 0;
    
    // 确定攻击者类型（from位置的棋子）
    uint64_t from_mask = 1ULL << move.from;
    bool attacker_is_king = false;
    
    if (board.current_player == 1) {
        // 黑方攻击
        attacker_is_king = (board.black_kings & from_mask) != 0;
    } else {
        // 白方攻击
        attacker_is_king = (board.white_kings & from_mask) != 0;
    }
    
    // 计算被吃棋子的总价值（Victim价值）
    int victim_value = 0;
    uint64_t opponent_kings = (board.current_player == 1) ? board.white_kings : board.black_kings;
    
    for (int i = 0; i < move.num_captures; ++i) {
        if (move.captures[i] >= 0 && move.captures[i] < 50) {
            uint64_t capture_mask = 1ULL << move.captures[i];
            if (opponent_kings & capture_mask) {
                victim_value += 300;  // 吃王价值高
            } else {
                victim_value += 100;  // 吃兵价值低
            }
        }
    }
    
    // 计算攻击者价值（Attacker价值，用于减分）
    int attacker_value = attacker_is_king ? 300 : 100;
    
    // MVV-LVA分数 = 被吃棋子价值 * 10 - 攻击者价值
    // 乘以10是为了让victim价值占主导地位
    mvv_lva_score = victim_value * 10 - attacker_value;
    
    return mvv_lva_score;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "MVV-LVA吃子排序测试" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // 创建测试棋盘
    TestBoard board;
    board.current_player = 1;  // 黑方
    
    // 设置棋子位置
    // 黑方：格子15是兵，格子20是王
    // 白方：格子25是兵，格子30是王
    board.black_kings = (1ULL << 20);
    board.white_kings = (1ULL << 30);
    
    // 创建测试走法
    std::vector<Move> moves;
    
    // 走法A: 黑兵(15)吃白王(30)
    Move moveA(15, 35, "黑兵吃白王");
    moveA.add_capture(30);
    moves.push_back(moveA);
    
    // 走法B: 黑王(20)吃白王(30)
    Move moveB(20, 35, "黑王吃白王");
    moveB.add_capture(30);
    moves.push_back(moveB);
    
    // 走法C: 黑兵(15)吃白兵(25)
    Move moveC(15, 30, "黑兵吃白兵");
    moveC.add_capture(25);
    moves.push_back(moveC);
    
    // 走法D: 黑王(20)吃白兵(25)
    Move moveD(20, 30, "黑王吃白兵");
    moveD.add_capture(25);
    moves.push_back(moveD);
    
    // 计算每个走法的MVV-LVA分数
    std::cout << "计算MVV-LVA分数：" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    for (Move& move : moves) {
        move.score = calculate_mvv_lva_score(move, board);
        
        // 确定攻击者和被吃者类型
        uint64_t from_mask = 1ULL << move.from;
        bool attacker_is_king = (board.black_kings & from_mask) != 0;
        
        uint64_t capture_mask = 1ULL << move.captures[0];
        bool victim_is_king = (board.white_kings & capture_mask) != 0;
        
        int victim_value = victim_is_king ? 300 : 100;
        int attacker_value = attacker_is_king ? 300 : 100;
        
        std::cout << move.description << std::endl;
        std::cout << "  攻击者价值: " << attacker_value << std::endl;
        std::cout << "  被吃者价值: " << victim_value << std::endl;
        std::cout << "  MVV-LVA分数: " << victim_value << "*10 - " << attacker_value 
                  << " = " << move.score << std::endl;
        std::cout << std::endl;
    }
    
    // 排序前
    std::cout << "排序前的走法顺序：" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    for (size_t i = 0; i < moves.size(); ++i) {
        std::cout << (i+1) << ". " << moves[i].description 
                  << " (分数: " << moves[i].score << ")" << std::endl;
    }
    std::cout << std::endl;
    
    // 按MVV-LVA分数降序排序
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.score > b.score;
    });
    
    // 排序后
    std::cout << "排序后的走法顺序（MVV-LVA）：" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    for (size_t i = 0; i < moves.size(); ++i) {
        std::cout << (i+1) << ". " << moves[i].description 
                  << " (分数: " << moves[i].score << ")" << std::endl;
    }
    std::cout << std::endl;
    
    // 验证排序结果
    std::cout << "验证结果：" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    bool correct = true;
    
    // 预期顺序：黑兵吃白王 > 黑王吃白王 > 黑兵吃白兵 > 黑王吃白兵
    if (moves[0].description == "黑兵吃白王" && moves[0].score == 2900) {
        std::cout << "✓ 第1位正确: 黑兵吃白王 (2900)" << std::endl;
    } else {
        std::cout << "✗ 第1位错误: 预期黑兵吃白王(2900), 实际" 
                  << moves[0].description << "(" << moves[0].score << ")" << std::endl;
        correct = false;
    }
    
    if (moves[1].description == "黑王吃白王" && moves[1].score == 2700) {
        std::cout << "✓ 第2位正确: 黑王吃白王 (2700)" << std::endl;
    } else {
        std::cout << "✗ 第2位错误: 预期黑王吃白王(2700), 实际" 
                  << moves[1].description << "(" << moves[1].score << ")" << std::endl;
        correct = false;
    }
    
    if (moves[2].description == "黑兵吃白兵" && moves[2].score == 900) {
        std::cout << "✓ 第3位正确: 黑兵吃白兵 (900)" << std::endl;
    } else {
        std::cout << "✗ 第3位错误: 预期黑兵吃白兵(900), 实际" 
                  << moves[2].description << "(" << moves[2].score << ")" << std::endl;
        correct = false;
    }
    
    if (moves[3].description == "黑王吃白兵" && moves[3].score == 700) {
        std::cout << "✓ 第4位正确: 黑王吃白兵 (700)" << std::endl;
    } else {
        std::cout << "✗ 第4位错误: 预期黑王吃白兵(700), 实际" 
                  << moves[3].description << "(" << moves[3].score << ")" << std::endl;
        correct = false;
    }
    
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    if (correct) {
        std::cout << "✓ 所有测试通过！MVV-LVA排序正确实现。" << std::endl;
        std::cout << std::endl;
        std::cout << "验证要点：" << std::endl;
        std::cout << "1. 吃王优先于吃兵（前2个走法都是吃王）" << std::endl;
        std::cout << "2. 同样吃王时，兵吃优先于王吃（2900 > 2700）" << std::endl;
        std::cout << "3. 同样吃兵时，兵吃优先于王吃（900 > 700）" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } else {
        std::cout << "✗ 测试失败！MVV-LVA排序实现有误。" << std::endl;
        std::cout << "========================================" << std::endl;
        return 1;
    }
}
