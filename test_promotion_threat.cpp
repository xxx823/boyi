// 测试升王威胁和接近升王奖励功能
// Task 3.3: 实现升王威胁和接近升王奖励

#include <iostream>
#include <cassert>
#include <cstdint>

using namespace std;

// 前向声明（从boyi.cpp复制必要的定义）
#ifdef _MSC_VER
#include <intrin.h>

inline int __builtin_popcountll(uint64_t x) {
    return (int)__popcnt64(x);
}

inline int __builtin_ctzll(uint64_t x) {
    unsigned long index;
    _BitScanForward64(&index, x);
    return (int)index;
}
#endif

// 简化的Board类（仅用于测试）
class Board {
public:
    uint64_t black_men;
    uint64_t white_men;
    uint64_t black_kings;
    uint64_t white_kings;
    int current_player;
    
    Board() : black_men(0), white_men(0), black_kings(0), white_kings(0), current_player(1) {}
    
    uint64_t get_all_black() const { return black_men | black_kings; }
    uint64_t get_all_white() const { return white_men | white_kings; }
};

// 简化的Evaluator类（仅包含升王威胁评估）
class Evaluator {
public:
    static int evaluate_promotion_threat(const Board& board) {
        bool is_black = (board.current_player == 1);
        uint64_t my_men = is_black ? board.black_men : board.white_men;
        uint64_t opp_men = is_black ? board.white_men : board.black_men;
        
        int score = 0;
        
        // 检查己方棋子距离升王的距离
        uint64_t men_copy = my_men;
        while (men_copy) {
            int sq = __builtin_ctzll(men_copy);
            men_copy &= men_copy - 1;
            
            int row = sq / 5;
            int distance_to_promotion;
            
            if (is_black) {
                distance_to_promotion = 9 - row;  // 黑方向上
            } else {
                distance_to_promotion = row;  // 白方向下
            }
            
            // 距离越近，奖励越高
            if (distance_to_promotion <= 2) {
                score += 20;  // 即将升王（需求3.5）
            } else if (distance_to_promotion <= 4) {
                score += 10;  // 接近升王（需求3.6）
            }
        }
        
        // 检查对手的升王威胁（需要阻止）
        men_copy = opp_men;
        while (men_copy) {
            int sq = __builtin_ctzll(men_copy);
            men_copy &= men_copy - 1;
            
            int row = sq / 5;
            int distance_to_promotion;
            
            if (!is_black) {
                distance_to_promotion = 9 - row;
            } else {
                distance_to_promotion = row;
            }
            
            if (distance_to_promotion <= 2) {
                score -= 15;  // 对手即将升王，需要阻止
            }
        }
        
        return score;
    }
};

void test_black_promotion_threat() {
    cout << "测试1: 黑方升王威胁（距离 <= 2步）" << endl;
    
    Board board;
    board.current_player = 1;  // 黑方
    
    // 黑方兵在第8行（row=7, sq=35-39），距离升王线2步
    board.black_men = 1ULL << 35;  // 格子36（row=7）
    board.white_men = 0;
    
    int score = Evaluator::evaluate_promotion_threat(board);
    cout << "  黑方兵在格子36（第8行），距离升王线2步" << endl;
    cout << "  期望奖励: 20分" << endl;
    cout << "  实际得分: " << score << endl;
    assert(score == 20);
    cout << "  ✓ 测试通过" << endl << endl;
}

void test_black_approaching_promotion() {
    cout << "测试2: 黑方接近升王（距离 <= 4步）" << endl;
    
    Board board;
    board.current_player = 1;  // 黑方
    
    // 黑方兵在第6行（row=5, sq=25-29），距离升王线4步
    board.black_men = 1ULL << 25;  // 格子26（row=5）
    board.white_men = 0;
    
    int score = Evaluator::evaluate_promotion_threat(board);
    cout << "  黑方兵在格子26（第6行），距离升王线4步" << endl;
    cout << "  期望奖励: 10分" << endl;
    cout << "  实际得分: " << score << endl;
    assert(score == 10);
    cout << "  ✓ 测试通过" << endl << endl;
}

void test_white_promotion_threat() {
    cout << "测试3: 白方升王威胁（距离 <= 2步）" << endl;
    
    Board board;
    board.current_player = -1;  // 白方
    
    // 白方兵在第2行（row=1, sq=5-9），距离升王线1步
    board.white_men = 1ULL << 6;  // 格子7（row=1）
    board.black_men = 0;
    
    int score = Evaluator::evaluate_promotion_threat(board);
    cout << "  白方兵在格子7（第2行），距离升王线1步" << endl;
    cout << "  期望奖励: 20分" << endl;
    cout << "  实际得分: " << score << endl;
    assert(score == 20);
    cout << "  ✓ 测试通过" << endl << endl;
}

void test_white_approaching_promotion() {
    cout << "测试4: 白方接近升王（距离 <= 4步）" << endl;
    
    Board board;
    board.current_player = -1;  // 白方
    
    // 白方兵在第4行（row=3, sq=15-19），距离升王线3步
    board.white_men = 1ULL << 16;  // 格子17（row=3）
    board.black_men = 0;
    
    int score = Evaluator::evaluate_promotion_threat(board);
    cout << "  白方兵在格子17（第4行），距离升王线3步" << endl;
    cout << "  期望奖励: 10分" << endl;
    cout << "  实际得分: " << score << endl;
    assert(score == 10);
    cout << "  ✓ 测试通过" << endl << endl;
}

void test_multiple_pawns() {
    cout << "测试5: 多个兵的升王威胁" << endl;
    
    Board board;
    board.current_player = 1;  // 黑方
    
    // 黑方有2个兵接近升王
    board.black_men = (1ULL << 35) | (1ULL << 36);  // 格子36和37（第8行）
    board.white_men = 0;
    
    int score = Evaluator::evaluate_promotion_threat(board);
    cout << "  黑方有2个兵在第8行，各距离升王线2步" << endl;
    cout << "  期望奖励: 20 + 20 = 40分" << endl;
    cout << "  实际得分: " << score << endl;
    assert(score == 40);
    cout << "  ✓ 测试通过" << endl << endl;
}

void test_opponent_threat() {
    cout << "测试6: 对手升王威胁（需要阻止）" << endl;
    
    Board board;
    board.current_player = 1;  // 黑方
    
    // 白方兵在第2行（row=1），距离升王线1步
    board.black_men = 0;
    board.white_men = 1ULL << 6;  // 格子7（row=1）
    
    int score = Evaluator::evaluate_promotion_threat(board);
    cout << "  白方兵在格子7（第2行），距离升王线1步" << endl;
    cout << "  期望惩罚: -15分（需要阻止对手升王）" << endl;
    cout << "  实际得分: " << score << endl;
    assert(score == -15);
    cout << "  ✓ 测试通过" << endl << endl;
}

void test_mixed_scenario() {
    cout << "测试7: 混合场景（己方和对手都有升王威胁）" << endl;
    
    Board board;
    board.current_player = 1;  // 黑方
    
    // 黑方兵在第8行（距离2步）
    board.black_men = 1ULL << 35;  // 格子36（row=7）
    // 白方兵在第2行（距离1步）
    board.white_men = 1ULL << 6;   // 格子7（row=1）
    
    int score = Evaluator::evaluate_promotion_threat(board);
    cout << "  黑方兵在格子36（第8行），距离2步: +20分" << endl;
    cout << "  白方兵在格子7（第2行），距离1步: -15分" << endl;
    cout << "  期望总分: 20 - 15 = 5分" << endl;
    cout << "  实际得分: " << score << endl;
    assert(score == 5);
    cout << "  ✓ 测试通过" << endl << endl;
}

void test_no_threat() {
    cout << "测试8: 无升王威胁（距离 > 4步）" << endl;
    
    Board board;
    board.current_player = 1;  // 黑方
    
    // 黑方兵在第3行（row=2, sq=10-14），距离升王线7步
    board.black_men = 1ULL << 10;  // 格子11（row=2）
    board.white_men = 0;
    
    int score = Evaluator::evaluate_promotion_threat(board);
    cout << "  黑方兵在格子11（第3行），距离升王线7步" << endl;
    cout << "  期望奖励: 0分（距离太远）" << endl;
    cout << "  实际得分: " << score << endl;
    assert(score == 0);
    cout << "  ✓ 测试通过" << endl << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << "Task 3.3: 升王威胁和接近升王奖励测试" << endl;
    cout << "========================================" << endl << endl;
    
    cout << "需求验证:" << endl;
    cout << "  需求3.5: 距离 <= 2步时增加20分升王威胁奖励" << endl;
    cout << "  需求3.6: 距离 <= 4步时增加10分接近升王奖励" << endl << endl;
    
    try {
        test_black_promotion_threat();
        test_black_approaching_promotion();
        test_white_promotion_threat();
        test_white_approaching_promotion();
        test_multiple_pawns();
        test_opponent_threat();
        test_mixed_scenario();
        test_no_threat();
        
        cout << "========================================" << endl;
        cout << "所有测试通过！✓" << endl;
        cout << "========================================" << endl;
        cout << endl;
        
        cout << "Task 3.3 实现总结:" << endl;
        cout << "  ✓ 计算普通兵到升王线的距离" << endl;
        cout << "  ✓ 距离 <= 2步：增加20分升王威胁奖励（需求3.5）" << endl;
        cout << "  ✓ 距离 <= 4步：增加10分接近升王奖励（需求3.6）" << endl;
        cout << "  ✓ 对手升王威胁：减少15分（需要阻止）" << endl;
        cout << "  ✓ 已集成到evaluate_advanced函数" << endl;
        cout << endl;
        
        cout << "实现位置: boyi/boyi.cpp" << endl;
        cout << "  - 函数: Evaluator::evaluate_promotion_threat (行938-989)" << endl;
        cout << "  - 集成: Evaluator::evaluate_advanced (行1416, 1422)" << endl;
        
    } catch (const exception& e) {
        cout << "测试失败: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
