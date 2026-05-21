#include "boyi/boyi.cpp"
#include <chrono>
#include <iomanip>

class Benchmark {
public:
    void test_move_generation() {
        std::cout << "========== 测试1: 走法生成速度 ==========\n";
        
        Board board;
        MoveList moves;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        const int iterations = 100000;
        for (int i = 0; i < iterations; ++i) {
            moves.clear();
            MoveGenerator::generate_moves(board, moves);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        double ops_per_sec = iterations / (duration.count() / 1000000.0);
        double avg_time_us = duration.count() / (double)iterations;
        
        std::cout << "迭代次数: " << iterations << "\n";
        std::cout << "总耗时: " << (duration.count() / 1000.0) << " ms\n";
        std::cout << "走法生成速度: " << std::fixed << std::setprecision(0) << ops_per_sec << " 次/秒\n";
        std::cout << "平均耗时: " << std::fixed << std::setprecision(2) << avg_time_us << " 微秒\n";
        
        if (avg_time_us < 10.0) {
            std::cout << "✓ 性能优秀 (<10微秒)\n";
        } else if (avg_time_us < 20.0) {
            std::cout << "✓ 性能良好 (<20微秒)\n";
        } else {
            std::cout << "✗ 性能需要优化 (>20微秒)\n";
        }
        std::cout << "\n";
    }
    
    void test_search_speed() {
        std::cout << "========== 测试2: 搜索速度 (NPS) ==========\n";
        
        Board board;
        SearchEngine engine;
        
        std::cout << "开始搜索（深度6，时间限制5秒）...\n";
        
        auto start = std::chrono::high_resolution_clock::now();
        
        Move move = engine.search(board, 6, 5000);  // 深度6，5秒
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        int nodes = engine.get_nodes_searched();
        double nps = nodes / (duration.count() / 1000.0);
        
        std::cout << "搜索完成！\n";
        std::cout << "搜索深度: 6\n";
        std::cout << "搜索节点: " << nodes << "\n";
        std::cout << "搜索时间: " << duration.count() << " ms\n";
        std::cout << "NPS: " << std::fixed << std::setprecision(0) << nps << "\n";
        std::cout << "最佳走法: " << move.to_string() << "\n";
        
        if (nps >= 100000) {
            std::cout << "✓ 达到目标 (>100,000 NPS)\n";
        } else if (nps >= 50000) {
            std::cout << "○ 接近目标 (>50,000 NPS)\n";
            std::cout << "  还需提升: " << std::fixed << std::setprecision(1) 
                      << ((100000.0 / nps - 1) * 100) << "%\n";
        } else {
            std::cout << "✗ 未达到目标 (目标: >100,000 NPS)\n";
            std::cout << "  还需提升: " << std::fixed << std::setprecision(1) 
                      << ((100000.0 / nps - 1) * 100) << "%\n";
        }
        std::cout << "\n";
    }
    
    void test_tt_hit_rate() {
        std::cout << "========== 测试3: 置换表命中率 ==========\n";
        
        Board board;
        SearchEngine engine;
        
        std::cout << "开始搜索（深度8，时间限制10秒）...\n";
        
        engine.search(board, 8, 10000);  // 深度8，10秒
        
        double hit_rate = engine.get_tt_hit_rate();
        
        std::cout << "搜索完成！\n";
        std::cout << "置换表命中率: " << std::fixed << std::setprecision(1) << (hit_rate * 100) << "%\n";
        
        if (hit_rate >= 0.6) {
            std::cout << "✓ 命中率优秀 (>60%)\n";
        } else if (hit_rate >= 0.5) {
            std::cout << "✓ 命中率良好 (>50%)\n";
        } else if (hit_rate >= 0.4) {
            std::cout << "○ 命中率一般 (>40%)\n";
        } else {
            std::cout << "✗ 命中率偏低 (<40%)\n";
        }
        std::cout << "\n";
    }
    
    void test_evaluation_speed() {
        std::cout << "========== 测试4: 评估函数速度 ==========\n";
        
        Board board;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        const int iterations = 1000000;
        int total_score = 0;
        for (int i = 0; i < iterations; ++i) {
            total_score += Evaluator::evaluate(board);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        double ops_per_sec = iterations / (duration.count() / 1000000.0);
        double avg_time_us = duration.count() / (double)iterations;
        
        std::cout << "迭代次数: " << iterations << "\n";
        std::cout << "总耗时: " << (duration.count() / 1000.0) << " ms\n";
        std::cout << "评估速度: " << std::fixed << std::setprecision(0) << ops_per_sec << " 次/秒\n";
        std::cout << "平均耗时: " << std::fixed << std::setprecision(3) << avg_time_us << " 微秒\n";
        
        if (avg_time_us < 1.0) {
            std::cout << "✓ 性能优秀 (<1微秒)\n";
        } else if (avg_time_us < 2.0) {
            std::cout << "✓ 性能良好 (<2微秒)\n";
        } else {
            std::cout << "✗ 性能需要优化 (>2微秒)\n";
        }
        std::cout << "\n";
    }
    
    void run_all() {
        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════╗\n";
        std::cout << "║   国际跳棋AI - 性能基准测试           ║\n";
        std::cout << "╚════════════════════════════════════════╝\n";
        std::cout << "\n";
        
        test_move_generation();
        test_evaluation_speed();
        test_search_speed();
        test_tt_hit_rate();
        
        std::cout << "========================================\n";
        std::cout << "所有测试完成！\n";
        std::cout << "========================================\n";
    }
};

int main() {
    Benchmark bench;
    bench.run_all();
    return 0;
}
