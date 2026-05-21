// 测试中局着法排序增强
// 验证任务2.2的实现：检测游戏阶段并在中局额外奖励中心控制和王活跃度走法

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>

// 简化的测试框架 - 只包含必要的类定义
// 注意：这是一个独立的测试文件，不依赖完整的boyi.cpp

using namespace std;

// 简化的Move结构
struct Move {
    int from;
    int to;
    int num_captures;
    int captures[12];
    
    Move() : from(-1), to(-1), num_captures(0) {
        for (int i = 0; i < 12; ++i) captures[i] = -1;
    }
    
    Move(int f, int t) : from(f), to(t), num_captures(0) {
        for (int i = 0; i < 12; ++i) captures[i] = -1;
    }
    
    bool is_valid() const { return from >= 0 && to >= 0; }
};

// 简化的Board结构
struct Board {
    uint64_t black_men;
    uint64_t black_kings;
    uint64_t white_men;
    uint64_t white_kings;
    int current_player;  // 1=黑方, 2=白方
    uint64_t hash;
    
    Board() : black_men(0), black_kings(0), white_men(0), white_kings(0), 
              current_player(1), hash(0) {}
    
    uint64_t get_all_black() const { return black_men | black_kings; }
    uint64_t get_all_white() const { return white_men | white_kings; }
};

// 简化的Evaluator类
class Evaluator {
public:
    enum GamePhase {
        OPENING,   // 开局：前15步
        MIDGAME,   // 中局：16-40步且棋子数>8
        ENDGAME    // 残局：棋子数<=8
    };
    
    static int position_value[50];
    static bool initialized;
    
    static void init() {
        if (initialized) return;
        
        for (int sq = 0; sq < 50; ++sq) {
            int row = sq / 5;
            int col = (sq % 5) * 2 + (row % 2);
            int center_distance = abs(row - 4) + abs(col - 4);
            position_value[sq] = 20 - center_distance * 2;
            if (col == 0 || col == 9) {
                position_value[sq] -= 5;
            }
        }
        
        initialized = true;
    }
    
    static GamePhase get_game_phase(const Board& board, int move_count) {
        int total_pieces = __builtin_popcountll(board.get_all_black()) +
                          __builtin_popcountll(board.get_all_white());
        
        if (total_pieces <= 8) {
            return ENDGAME;
        } else if (move_count <= 15) {
            return OPENING;
        } else {
            return MIDGAME;
        }
    }
};

int Evaluator::position_value[50] = {0};
bool Evaluator::initialized = false;

// 简化的SearchEngine类（只包含score_move方法）
class SearchEngine {
public:
    SearchEngine() {
        Evaluator::init();
    }
    
    // 这是我们要测试的方法
    int score_move(const Move& move, const Board& board, int depth, int move_count = 0) {
        int score = 0;
        
        // 简化版本：只测试中局增强部分
        // 跳过置换表、吃子、升王、杀手走法、历史启发等
        
        // 基础位置启发分数
        if (move.from >= 0 && move.from < 50 && move.to >= 0 && move.to < 50) {
            score += Evaluator::position_value[move.to] - Evaluator::position_value[move.from];
        }
        
        // 中局着法排序增强（需求2.7）
        Evaluator::GamePhase phase = Evaluator::get_game_phase(board, move_count);
        if (phase == Evaluator::MIDGAME) {
            bool is_black = (board.current_player == 1);
            uint64_t from_mask = 1ULL << move.from;
            
            // 7a. 中心控制走法奖励（格子21,22,26,27）
            if (move.to == 21 || move.to == 22 || move.to == 26 || move.to == 27) {
                score += 50;  // 中心控制奖励
            }
            
            // 7b. 王的活跃度奖励
            bool is_king = false;
            if (is_black) {
                is_king = (board.black_kings & from_mask) != 0;
            } else {
                is_king = (board.white_kings & from_mask) != 0;
            }
            
            if (is_king) {
                int from_row = move.from / 5;
                int from_col = (move.from % 5) * 2 + (from_row % 2);
                int to_row = move.to / 5;
                int to_col = (move.to % 5) * 2 + (to_row % 2);
                
                int from_center_dist = abs(from_row - 4) + abs(from_col - 4);
                int to_center_dist = abs(to_row - 4) + abs(to_col - 4);
                
                if (to_center_dist < from_center_dist) {
                    score += 30;  // 王活跃度奖励
                }
            }
        }
        
        return score;
    }
};

// 测试用例
void test_game_phase_detection() {
    cout << "\n测试1: 游戏阶段检测" << endl;
    cout << "==================" << endl;
    
    Board board;
    
    // 测试开局阶段（前15步）
    board.black_men = 0xFFFFF;  // 20个黑兵
    board.white_men = 0xFFFFF;  // 20个白兵
    
    Evaluator::GamePhase phase = Evaluator::get_game_phase(board, 10);
    cout << "步数=10, 棋子数=40: " << (phase == Evaluator::OPENING ? "开局 ✓" : "错误 ✗") << endl;
    
    // 测试中局阶段（16-40步且棋子数>8）
    phase = Evaluator::get_game_phase(board, 20);
    cout << "步数=20, 棋子数=40: " << (phase == Evaluator::MIDGAME ? "中局 ✓" : "错误 ✗") << endl;
    
    // 测试残局阶段（棋子数<=8）
    board.black_men = 0xF;  // 4个黑兵
    board.white_men = 0xF;  // 4个白兵
    phase = Evaluator::get_game_phase(board, 50);
    cout << "步数=50, 棋子数=8: " << (phase == Evaluator::ENDGAME ? "残局 ✓" : "错误 ✗") << endl;
}

void test_center_control_bonus() {
    cout << "\n测试2: 中心控制走法奖励" << endl;
    cout << "========================" << endl;
    
    SearchEngine engine;
    Board board;
    board.current_player = 1;  // 黑方
    board.black_men = 0xFFFFF;  // 20个黑兵
    board.white_men = 0xFFFFF;  // 20个白兵
    
    int move_count = 20;  // 中局阶段
    
    // 测试移动到中心格子21
    Move move_to_center(16, 21);
    int score_center = engine.score_move(move_to_center, board, 0, move_count);
    
    // 测试移动到非中心格子
    Move move_to_edge(16, 20);
    int score_edge = engine.score_move(move_to_edge, board, 0, move_count);
    
    cout << "移动到中心格子21的分数: " << score_center << endl;
    cout << "移动到边缘格子20的分数: " << score_edge << endl;
    cout << "中心格子奖励差值: " << (score_center - score_edge) << endl;
    cout << (score_center > score_edge ? "中心控制奖励生效 ✓" : "错误 ✗") << endl;
    
    // 测试其他中心格子
    Move move_to_22(17, 22);
    Move move_to_26(21, 26);
    Move move_to_27(22, 27);
    
    int score_22 = engine.score_move(move_to_22, board, 0, move_count);
    int score_26 = engine.score_move(move_to_26, board, 0, move_count);
    int score_27 = engine.score_move(move_to_27, board, 0, move_count);
    
    cout << "格子22分数: " << score_22 << " " << (score_22 > score_edge ? "✓" : "✗") << endl;
    cout << "格子26分数: " << score_26 << " " << (score_26 > score_edge ? "✓" : "✗") << endl;
    cout << "格子27分数: " << score_27 << " " << (score_27 > score_edge ? "✓" : "✗") << endl;
}

void test_king_activity_bonus() {
    cout << "\n测试3: 王的活跃度奖励" << endl;
    cout << "======================" << endl;
    
    SearchEngine engine;
    Board board;
    board.current_player = 1;  // 黑方
    board.black_men = 0xFFFFF;  // 20个黑兵
    board.white_men = 0xFFFFF;  // 20个白兵
    
    // 在格子10放置一个黑王
    board.black_kings = 1ULL << 10;
    
    int move_count = 20;  // 中局阶段
    
    // 测试王向中心移动（从格子10到格子15，更接近中心）
    Move king_to_center(10, 15);
    int score_king_center = engine.score_move(king_to_center, board, 0, move_count);
    
    // 测试王向边缘移动（从格子10到格子5，远离中心）
    Move king_to_edge(10, 5);
    int score_king_edge = engine.score_move(king_to_edge, board, 0, move_count);
    
    cout << "王向中心移动的分数: " << score_king_center << endl;
    cout << "王向边缘移动的分数: " << score_king_edge << endl;
    cout << "活跃度奖励差值: " << (score_king_center - score_king_edge) << endl;
    cout << (score_king_center > score_king_edge ? "王活跃度奖励生效 ✓" : "错误 ✗") << endl;
}

void test_midgame_only() {
    cout << "\n测试4: 中局增强仅在中局生效" << endl;
    cout << "==============================" << endl;
    
    SearchEngine engine;
    Board board;
    board.current_player = 1;  // 黑方
    
    // 开局阶段测试
    board.black_men = 0xFFFFF;  // 20个黑兵
    board.white_men = 0xFFFFF;  // 20个白兵
    int opening_move_count = 10;
    
    Move move_to_center(16, 21);
    int score_opening = engine.score_move(move_to_center, board, 0, opening_move_count);
    
    // 中局阶段测试
    int midgame_move_count = 20;
    int score_midgame = engine.score_move(move_to_center, board, 0, midgame_move_count);
    
    // 残局阶段测试
    board.black_men = 0xF;  // 4个黑兵
    board.white_men = 0xF;  // 4个白兵
    int endgame_move_count = 50;
    int score_endgame = engine.score_move(move_to_center, board, 0, endgame_move_count);
    
    cout << "开局阶段分数: " << score_opening << endl;
    cout << "中局阶段分数: " << score_midgame << endl;
    cout << "残局阶段分数: " << score_endgame << endl;
    cout << "中局分数 > 开局分数: " << (score_midgame > score_opening ? "✓" : "✗") << endl;
    cout << "中局分数 > 残局分数: " << (score_midgame > score_endgame ? "✓" : "✗") << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << "中局着法排序增强测试" << endl;
    cout << "任务2.2验证" << endl;
    cout << "========================================" << endl;
    
    Evaluator::init();
    
    test_game_phase_detection();
    test_center_control_bonus();
    test_king_activity_bonus();
    test_midgame_only();
    
    cout << "\n========================================" << endl;
    cout << "所有测试完成！" << endl;
    cout << "========================================" << endl;
    
    return 0;
}
