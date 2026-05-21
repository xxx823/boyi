#include "boyi/boyi.cpp"
#include <fstream>
#include <chrono>
#include <iomanip>

struct GameResult {
    int winner;  // 1=黑胜, -1=白胜, 0=和棋
    int moves;
    int time_ms;
    std::string pgn;
    std::string termination_reason;
};

class SelfPlayEngine {
private:
    SearchEngine black_engine;
    SearchEngine white_engine;
    
public:
    GameResult play_game(int time_per_move_ms = 1000, int max_moves = 200) {
        GameState game;
        GameResult result;
        result.winner = 0;
        result.moves = 0;
        result.time_ms = 0;
        
        auto game_start = std::chrono::high_resolution_clock::now();
        
        while (!game.is_game_over() && result.moves < max_moves) {
            Board board = game.get_board();
            
            auto move_start = std::chrono::high_resolution_clock::now();
            
            Move move;
            if (board.current_player == 1) {
                // 黑方走棋
                move = black_engine.search(board, 10, time_per_move_ms);
            } else {
                // 白方走棋
                move = white_engine.search(board, 10, time_per_move_ms);
            }
            
            auto move_end = std::chrono::high_resolution_clock::now();
            int move_time = std::chrono::duration_cast<std::chrono::milliseconds>(move_end - move_start).count();
            
            if (!move.is_valid()) {
                result.termination_reason = "无效走法";
                result.winner = board.current_player == 1 ? -1 : 1;
                break;
            }
            
            game.make_move(move);
            result.pgn += move.to_string() + " ";
            result.moves++;
        }
        
        auto game_end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(game_end - game_start).count();
        
        // 判断胜负
        if (game.is_game_over()) {
            if (game.is_draw()) {
                result.winner = 0;
                result.termination_reason = "和棋";
            } else {
                // 当前玩家无法走棋，对手获胜
                result.winner = game.get_board().current_player == 1 ? -1 : 1;
                result.termination_reason = "无合法走法";
            }
        } else if (result.moves >= max_moves) {
            result.winner = 0;
            result.termination_reason = "达到最大步数";
        }
        
        return result;
    }
    
    void run_tournament(int num_games = 100, int time_per_move_ms = 1000) {
        int black_wins = 0, white_wins = 0, draws = 0;
        int total_moves = 0;
        long long total_time = 0;
        
        std::ofstream log("selfplay_results.txt");
        log << "========== 自我对弈测试结果 ==========\n";
        log << "对局数: " << num_games << "\n";
        log << "每步时间: " << time_per_move_ms << " ms\n";
        log << "开始时间: " << __DATE__ << " " << __TIME__ << "\n";
        log << "\n";
        
        auto tournament_start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_games; ++i) {
            std::cout << "对局 " << std::setw(3) << (i + 1) << "/" << num_games << " ... ";
            std::cout.flush();
            
            GameResult result = play_game(time_per_move_ms);
            
            if (result.winner == 1) black_wins++;
            else if (result.winner == -1) white_wins++;
            else draws++;
            
            total_moves += result.moves;
            total_time += result.time_ms;
            
            std::cout << "完成 (";
            if (result.winner == 1) std::cout << "黑胜";
            else if (result.winner == -1) std::cout << "白胜";
            else std::cout << "和棋";
            std::cout << ", " << result.moves << "步, " 
                      << std::fixed << std::setprecision(1) << (result.time_ms / 1000.0) << "秒";
            if (!result.termination_reason.empty()) {
                std::cout << ", " << result.termination_reason;
            }
            std::cout << ")\n";
            
            // 记录到文件
            log << "对局 " << (i + 1) << ": ";
            if (result.winner == 1) log << "黑胜";
            else if (result.winner == -1) log << "白胜";
            else log << "和棋";
            log << " (" << result.termination_reason << ")\n";
            log << "  步数: " << result.moves << "\n";
            log << "  用时: " << result.time_ms << " ms\n";
            log << "  棋谱: " << result.pgn << "\n\n";
            
            // 每10局输出一次进度
            if ((i + 1) % 10 == 0) {
                double progress = (i + 1) / (double)num_games * 100;
                std::cout << "  进度: " << std::fixed << std::setprecision(1) << progress << "% ";
                std::cout << "(黑:" << black_wins << " 白:" << white_wins << " 和:" << draws << ")\n";
            }
        }
        
        auto tournament_end = std::chrono::high_resolution_clock::now();
        long long tournament_time = std::chrono::duration_cast<std::chrono::milliseconds>(tournament_end - tournament_start).count();
        
        // 输出统计
        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════╗\n";
        std::cout << "║      自我对弈测试统计结果              ║\n";
        std::cout << "╚════════════════════════════════════════╝\n";
        std::cout << "\n";
        std::cout << "总对局数: " << num_games << "\n";
        std::cout << "黑方胜: " << black_wins << " (" << std::fixed << std::setprecision(1) << (100.0 * black_wins / num_games) << "%)\n";
        std::cout << "白方胜: " << white_wins << " (" << std::fixed << std::setprecision(1) << (100.0 * white_wins / num_games) << "%)\n";
        std::cout << "和棋: " << draws << " (" << std::fixed << std::setprecision(1) << (100.0 * draws / num_games) << "%)\n";
        std::cout << "\n";
        std::cout << "平均步数: " << (total_moves / num_games) << " 步\n";
        std::cout << "平均用时: " << std::fixed << std::setprecision(1) << (total_time / num_games / 1000.0) << " 秒/局\n";
        std::cout << "总用时: " << std::fixed << std::setprecision(1) << (tournament_time / 1000.0) << " 秒\n";
        std::cout << "\n";
        
        // 分析结果
        std::cout << "结果分析:\n";
        
        // 检查平衡性
        double black_rate = black_wins / (double)num_games;
        double white_rate = white_wins / (double)num_games;
        double draw_rate = draws / (double)num_games;
        
        if (std::abs(black_rate - white_rate) < 0.1) {
            std::cout << "✓ 黑白平衡良好 (胜率差异 < 10%)\n";
        } else {
            std::cout << "○ 黑白存在差异 (胜率差异 " << std::fixed << std::setprecision(1) 
                      << (std::abs(black_rate - white_rate) * 100) << "%)\n";
        }
        
        if (draw_rate < 0.3) {
            std::cout << "✓ 和棋率正常 (< 30%)\n";
        } else {
            std::cout << "○ 和棋率偏高 (" << std::fixed << std::setprecision(1) << (draw_rate * 100) << "%)\n";
        }
        
        int avg_moves = total_moves / num_games;
        if (avg_moves >= 30 && avg_moves <= 80) {
            std::cout << "✓ 平均步数正常 (30-80步)\n";
        } else if (avg_moves < 30) {
            std::cout << "○ 对局偏短 (< 30步)\n";
        } else {
            std::cout << "○ 对局偏长 (> 80步)\n";
        }
        
        std::cout << "\n";
        std::cout << "详细结果已保存到: selfplay_results.txt\n";
        std::cout << "========================================\n";
        
        // 写入统计到文件
        log << "========== 统计结果 ==========\n";
        log << "总对局数: " << num_games << "\n";
        log << "黑方胜: " << black_wins << " (" << (100.0 * black_wins / num_games) << "%)\n";
        log << "白方胜: " << white_wins << " (" << (100.0 * white_wins / num_games) << "%)\n";
        log << "和棋: " << draws << " (" << (100.0 * draws / num_games) << "%)\n";
        log << "平均步数: " << (total_moves / num_games) << "\n";
        log << "平均用时: " << (total_time / num_games / 1000.0) << " 秒\n";
        log << "总用时: " << (tournament_time / 1000.0) << " 秒\n";
        log << "==================================\n";
        
        log.close();
    }
};

int main(int argc, char* argv[]) {
    int num_games = 100;
    int time_per_move = 1000;
    
    // 解析命令行参数
    if (argc >= 2) {
        num_games = std::atoi(argv[1]);
    }
    if (argc >= 3) {
        time_per_move = std::atoi(argv[2]);
    }
    
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║   国际跳棋AI - 自我对弈测试系统       ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "配置:\n";
    std::cout << "  对局数: " << num_games << "\n";
    std::cout << "  每步时间: " << time_per_move << " ms\n";
    std::cout << "\n";
    std::cout << "开始测试...\n";
    std::cout << "\n";
    
    SelfPlayEngine engine;
    engine.run_tournament(num_games, time_per_move);
    
    return 0;
}
