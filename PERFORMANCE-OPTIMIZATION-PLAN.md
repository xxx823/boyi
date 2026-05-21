# 性能优化与多线程实现计划

## 📊 当前性能状态

根据测试输出：
- **NPS（节点/秒）**: 55,296 - 59,392
- **目标**: > 100,000 NPS
- **差距**: 需要提升 **70-80%**

---

## 🎯 优化目标

1. **性能优化**: 提升搜索速度到 100,000+ NPS
2. **多线程搜索**: 利用多核CPU，提升 2-4倍速度
3. **自我对弈测试**: 验证整体效果和稳定性

---

## 📋 实施计划

### 阶段1：基础性能优化（立即可做）

#### 1.1 编译器优化
```bash
# 当前编译选项
cl /EHsc /std:c++17 /O2 boyi.cpp

# 优化后编译选项
cl /EHsc /std:c++17 /O2 /Oi /Ot /GL /arch:AVX2 boyi.cpp /link /LTCG
```

**优化选项说明**：
- `/O2`: 最大速度优化
- `/Oi`: 启用内联函数
- `/Ot`: 优先速度而非大小
- `/GL`: 全程序优化
- `/arch:AVX2`: 使用AVX2指令集
- `/LTCG`: 链接时代码生成

**预期提升**: +15-20%

#### 1.2 代码级优化

**优化点1: 预分配内存**
```cpp
// 在MoveGenerator中预分配走法列表
void generate_moves(const Board& board, MoveList& moves) {
    moves.reserve(100);  // 预分配空间
    // ... 生成走法
}
```

**优化点2: 减少函数调用**
```cpp
// 将小函数标记为inline
inline bool is_valid_square(int sq) {
    return sq >= 0 && sq < 50;
}
```

**优化点3: 优化位运算**
```cpp
// 使用更快的位运算技巧
// 原来：遍历所有位
while (bitboard) {
    int sq = __builtin_ctzll(bitboard);
    bitboard &= bitboard - 1;
    // 处理...
}

// 优化：使用查表法（对于频繁操作）
```

**预期提升**: +10-15%

---

### 阶段2：多线程搜索实现

#### 2.1 Lazy SMP算法

**原理**：
- 多个线程并行搜索同一棵搜索树
- 共享置换表
- 每个线程独立搜索，互不干扰
- 主线程收集最佳结果

**实现步骤**：

**步骤1: 创建线程池**
```cpp
class ThreadPool {
private:
    std::vector<std::thread> threads;
    std::vector<SearchEngine*> engines;
    int num_threads;
    
public:
    ThreadPool(int threads = 4) : num_threads(threads) {
        for (int i = 0; i < num_threads; ++i) {
            engines.push_back(new SearchEngine());
        }
    }
    
    ~ThreadPool() {
        for (auto* engine : engines) {
            delete engine;
        }
    }
};
```

**步骤2: 并行搜索**
```cpp
Move parallel_search(const Board& board, int depth, int time_ms) {
    std::vector<Move> results(num_threads);
    std::vector<int> scores(num_threads);
    
    // 启动所有线程
    for (int i = 0; i < num_threads; ++i) {
        threads[i] = std::thread([&, i]() {
            results[i] = engines[i]->search(board, depth, time_ms / num_threads);
            scores[i] = engines[i]->get_last_score();
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    // 选择最佳结果
    int best_idx = 0;
    for (int i = 1; i < num_threads; ++i) {
        if (scores[i] > scores[best_idx]) {
            best_idx = i;
        }
    }
    
    return results[best_idx];
}
```

**步骤3: 共享置换表**
```cpp
class SharedTranspositionTable {
private:
    std::vector<TTEntry> table;
    std::vector<std::mutex> locks;  // 分段锁
    
public:
    void store(uint64_t hash, int depth, int score, Move move, Flag flag) {
        size_t index = hash % table.size();
        size_t lock_index = index % locks.size();
        
        std::lock_guard<std::mutex> lock(locks[lock_index]);
        // 存储...
    }
};
```

**预期提升**: +200-300% (4核CPU)

---

### 阶段3：自我对弈测试系统

#### 3.1 自我对弈程序

**文件**: `selfplay.cpp`

```cpp
#include "boyi/boyi.cpp"
#include <fstream>
#include <chrono>

struct GameResult {
    int winner;  // 1=黑胜, -1=白胜, 0=和棋
    int moves;
    int time_ms;
    std::string pgn;
};

class SelfPlayEngine {
private:
    CompetitionInterface black_player;
    CompetitionInterface white_player;
    
public:
    GameResult play_game(int time_per_move_ms = 1000) {
        GameState game;
        GameResult result;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        while (!game.is_game_over()) {
            Move move;
            
            if (game.get_board().current_player == 1) {
                // 黑方走棋
                move = black_player.think_and_move(game, time_per_move_ms);
            } else {
                // 白方走棋
                move = white_player.think_and_move(game, time_per_move_ms);
            }
            
            game.make_move(move);
            result.pgn += move.to_string() + " ";
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        result.moves = game.get_move_count();
        
        // 判断胜负
        if (game.is_draw()) {
            result.winner = 0;
        } else {
            result.winner = game.get_board().current_player == 1 ? -1 : 1;
        }
        
        return result;
    }
    
    void run_tournament(int num_games = 100) {
        int black_wins = 0, white_wins = 0, draws = 0;
        int total_moves = 0, total_time = 0;
        
        std::ofstream log("selfplay_results.txt");
        
        for (int i = 0; i < num_games; ++i) {
            std::cout << "对局 " << (i + 1) << "/" << num_games << "... ";
            
            GameResult result = play_game();
            
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
                      << (result.time_ms / 1000.0) << "秒)\n";
            
            // 记录到文件
            log << "对局" << (i + 1) << ": ";
            if (result.winner == 1) log << "黑胜";
            else if (result.winner == -1) log << "白胜";
            else log << "和棋";
            log << " " << result.moves << "步 " << result.time_ms << "ms\n";
            log << "棋谱: " << result.pgn << "\n\n";
        }
        
        // 输出统计
        std::cout << "\n========== 自我对弈统计 ==========\n";
        std::cout << "总对局数: " << num_games << "\n";
        std::cout << "黑方胜: " << black_wins << " (" << (100.0 * black_wins / num_games) << "%)\n";
        std::cout << "白方胜: " << white_wins << " (" << (100.0 * white_wins / num_games) << "%)\n";
        std::cout << "和棋: " << draws << " (" << (100.0 * draws / num_games) << "%)\n";
        std::cout << "平均步数: " << (total_moves / num_games) << "\n";
        std::cout << "平均用时: " << (total_time / num_games / 1000.0) << "秒\n";
        std::cout << "==================================\n";
        
        log.close();
    }
};

int main() {
    SelfPlayEngine engine;
    engine.run_tournament(100);
    return 0;
}
```

#### 3.2 性能基准测试

**文件**: `benchmark.cpp`

```cpp
#include "boyi/boyi.cpp"
#include <chrono>

class Benchmark {
public:
    void test_move_generation() {
        Board board;
        MoveList moves;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 100000; ++i) {
            moves.clear();
            MoveGenerator::generate_moves(board, moves);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "走法生成速度: " << (100000.0 / duration.count() * 1000000) << " 次/秒\n";
        std::cout << "平均耗时: " << (duration.count() / 100000.0) << " 微秒\n";
    }
    
    void test_search_speed() {
        Board board;
        SearchEngine engine;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        Move move = engine.search(board, 6, 5000);  // 深度6，5秒
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        int nodes = engine.get_nodes_searched();
        double nps = nodes / (duration.count() / 1000.0);
        
        std::cout << "搜索深度: 6\n";
        std::cout << "搜索节点: " << nodes << "\n";
        std::cout << "搜索时间: " << duration.count() << " ms\n";
        std::cout << "NPS: " << nps << "\n";
        
        if (nps >= 100000) {
            std::cout << "✓ 达到目标 (>100,000 NPS)\n";
        } else {
            std::cout << "✗ 未达到目标 (目标: >100,000 NPS)\n";
        }
    }
    
    void test_tt_hit_rate() {
        Board board;
        SearchEngine engine;
        
        engine.search(board, 8, 10000);  // 深度8，10秒
        
        double hit_rate = engine.get_tt_hit_rate();
        
        std::cout << "置换表命中率: " << (hit_rate * 100) << "%\n";
        
        if (hit_rate >= 0.5) {
            std::cout << "✓ 命中率良好 (>50%)\n";
        } else {
            std::cout << "✗ 命中率偏低 (<50%)\n";
        }
    }
    
    void run_all() {
        std::cout << "========== 性能基准测试 ==========\n\n";
        
        std::cout << "测试1: 走法生成速度\n";
        test_move_generation();
        std::cout << "\n";
        
        std::cout << "测试2: 搜索速度 (NPS)\n";
        test_search_speed();
        std::cout << "\n";
        
        std::cout << "测试3: 置换表命中率\n";
        test_tt_hit_rate();
        std::cout << "\n";
        
        std::cout << "==================================\n";
    }
};

int main() {
    Benchmark bench;
    bench.run_all();
    return 0;
}
```

---

## 📝 实施步骤

### 第1步：编译器优化（5分钟）

1. 修改编译脚本
2. 重新编译
3. 运行测试验证

### 第2步：代码优化（30分钟）

1. 添加inline关键字
2. 预分配内存
3. 优化热点代码

### 第3步：多线程实现（2小时）

1. 实现线程池
2. 实现并行搜索
3. 实现共享置换表
4. 测试验证

### 第4步：自我对弈测试（1小时）

1. 创建selfplay.cpp
2. 编译运行
3. 分析结果

### 第5步：性能基准测试（30分钟）

1. 创建benchmark.cpp
2. 运行所有测试
3. 记录结果

---

## 🎯 预期效果

| 优化项 | 预期提升 | 累计提升 |
|--------|----------|----------|
| 编译器优化 | +15-20% | 1.15-1.20x |
| 代码优化 | +10-15% | 1.27-1.38x |
| 多线程(4核) | +200-300% | 3.81-5.52x |
| **总计** | **+280-450%** | **3.8-5.5x** |

**最终NPS**：
- 当前：55,000 - 60,000
- 优化后：**210,000 - 330,000**
- 目标：100,000+ ✓

---

## 📊 测试验证

### 性能测试
```bash
# 编译benchmark
cl /EHsc /std:c++17 /O2 /Oi /Ot /GL /arch:AVX2 benchmark.cpp -o benchmark.exe /link /LTCG

# 运行测试
benchmark.exe
```

### 自我对弈测试
```bash
# 编译selfplay
cl /EHsc /std:c++17 /O2 /Oi /Ot /GL /arch:AVX2 selfplay.cpp -o selfplay.exe /link /LTCG

# 运行100局测试
selfplay.exe
```

---

## 🚀 快速开始

1. **立即优化编译选项**
2. **添加代码优化**
3. **实现多线程搜索**
4. **运行性能测试**
5. **运行自我对弈验证**

---

**创建日期**: 2026年5月20日
**预计完成时间**: 4-5小时
**优先级**: 高
