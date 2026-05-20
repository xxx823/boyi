# 设计文档：国际跳棋AI程序

## Overview

### 系统目标

本系统是一个高性能的国际跳棋（100格）AI引擎，旨在参加2026年辽宁省大学生计算机博弈大赛。系统采用经典的博弈树搜索算法（Alpha-Beta剪枝）结合位棋盘（Bitboard）数据结构，实现快速的走法生成、局面评估和最优决策。

**核心设计目标：**
- **正确性**：严格遵守国际跳棋规则，包括强制吃子、最大吃子、升王等复杂规则
- **性能**：达到100,000+ NPS（每秒搜索节点数），支持深度8-12层的实时搜索
- **鲁棒性**：在比赛环境中稳定运行，处理各种边界情况和异常输入
- **可维护性**：模块化设计，清晰的代码结构，便于调试和优化

### 技术栈

- **编程语言**：C++17
- **核心数据结构**：Bitboard（64位整数位运算）
- **搜索算法**：Alpha-Beta剪枝 + 迭代加深
- **优化技术**：置换表、走法排序、历史启发、杀手走法
- **编译器**：MSVC 2022 (Windows) / GCC 9+ (Linux)
- **构建系统**：CMake 3.15+

### 系统约束

- 不依赖第三方库（仅使用C++标准库）
- 跨平台兼容（Windows/Linux）
- 内存占用 < 1GB（主要是置换表）
- 单线程执行（比赛规则限制）
- 响应时间 < 分配时间的95%（预留安全缓冲）

## Architecture

### 高层架构

系统采用分层架构，从底层到高层依次为：

```
┌─────────────────────────────────────────────────────────┐
│           Competition Interface Layer                    │
│  (标准输入/输出，协议解析，比赛平台通信)                  │
└─────────────────────────────────────────────────────────┘
                          ↓↑
┌─────────────────────────────────────────────────────────┐
│              Game Control Layer                          │
│  (游戏状态管理，时间管理，开局库/残局库查询)              │
└─────────────────────────────────────────────────────────┘
                          ↓↑
┌─────────────────────────────────────────────────────────┐
│              Search Engine Layer                         │
│  (Alpha-Beta搜索，迭代加深，置换表，走法排序)            │
└─────────────────────────────────────────────────────────┘
                          ↓↑
┌─────────────────────────────────────────────────────────┐
│         Move Generation & Evaluation Layer               │
│  (走法生成器，局面评估函数)                              │
└─────────────────────────────────────────────────────────┘
                          ↓↑
┌─────────────────────────────────────────────────────────┐
│              Core Data Layer                             │
│  (Bitboard表示，走法结构，游戏状态)                      │
└─────────────────────────────────────────────────────────┘
```

### 模块划分

#### 1. Core Data Module（核心数据模块）

**职责**：
- 定义Bitboard数据结构（4个uint64_t表示棋盘）
- 定义Move结构（起点、终点、吃子序列、升王标志）
- 提供基本的位运算操作（设置位、清除位、计数、移位）

**关键类型**：
- `Board`：棋盘状态（black_men, white_men, black_kings, white_kings, current_player）
- `Move`：走法表示（from, to, captures[], is_promotion）
- `GameState`：完整游戏状态（Board + 历史记录 + 重复局面计数）

#### 2. Move Generation Module（走法生成模块）

**职责**：
- 生成所有合法走法（普通移动、跳吃、连续跳吃）
- 实现强制吃子规则（有吃子必须吃）
- 实现最大吃子规则（选择吃子最多的走法）
- 区分普通棋子和王棋的移动规则

**关键函数**：
- `generate_moves(Board, MoveList&)`：生成所有合法走法
- `generate_captures(Board, MoveList&)`：生成所有吃子走法
- `generate_quiet_moves(Board, MoveList&)`：生成所有非吃子走法
- `generate_man_captures(Board, int square, MoveList&)`：生成普通棋子的吃子走法
- `generate_king_captures(Board, int square, MoveList&)`：生成王棋的吃子走法

**算法要点**：
- 使用位运算快速检测可移动方向
- 递归生成连续跳吃序列
- 使用预计算表加速方向移动

#### 3. Evaluation Module（评估模块）

**职责**：
- 评估局面的静态分数（从当前玩家视角）
- 综合考虑材料、位置、机动性、安全性等因素
- 识别终局状态（胜/负/和）

**评估因素**：
```
总分 = 材料分 + 位置分 + 机动性分 + 安全性分 + 结构分

材料分：
  - 普通棋子：100分/个
  - 王棋：300分/个
  
位置分：
  - 中心控制：+10分（中央格子）
  - 前进程度：+5分/行（普通棋子）
  - 底线王：+20分（王棋在己方底线）
  
机动性分：
  - 可移动格子数 × 5分
  
安全性分：
  - 被保护的棋子：+10分
  - 暴露的棋子：-15分
  
结构分：
  - 连续棋子：+8分（形成防线）
  - 孤立棋子：-12分
```

**关键函数**：
- `evaluate(Board)`：返回局面评估分数
- `is_terminal(Board)`：检查是否为终局
- `count_mobility(Board, player)`：计算机动性

#### 4. Search Engine Module（搜索引擎模块）

**职责**：
- 实现Alpha-Beta剪枝搜索
- 实现迭代加深（Iterative Deepening）
- 管理置换表（Transposition Table）
- 实现走法排序和启发式优化

**核心算法**：Alpha-Beta搜索伪代码
```
function alpha_beta(board, depth, alpha, beta, maximizing_player):
    # 查询置换表
    tt_entry = transposition_table.lookup(board.hash)
    if tt_entry.depth >= depth:
        return tt_entry.score
    
    # 终止条件
    if depth == 0 or is_terminal(board):
        return evaluate(board)
    
    # 生成并排序走法
    moves = generate_moves(board)
    sort_moves(moves, board)  # 使用历史表、杀手走法、MVV-LVA
    
    if maximizing_player:
        max_eval = -INFINITY
        for move in moves:
            board.make_move(move)
            eval = alpha_beta(board, depth-1, alpha, beta, false)
            board.unmake_move(move)
            max_eval = max(max_eval, eval)
            alpha = max(alpha, eval)
            if beta <= alpha:
                break  # Beta剪枝
        transposition_table.store(board.hash, depth, max_eval)
        return max_eval
    else:
        min_eval = +INFINITY
        for move in moves:
            board.make_move(move)
            eval = alpha_beta(board, depth-1, alpha, beta, true)
            board.unmake_move(move)
            min_eval = min(min_eval, eval)
            beta = min(beta, eval)
            if beta <= alpha:
                break  # Alpha剪枝
        transposition_table.store(board.hash, depth, min_eval)
        return min_eval
```

**迭代加深策略**：
```
function iterative_deepening_search(board, time_limit):
    best_move = null
    for depth = 1 to MAX_DEPTH:
        if time_elapsed() > time_limit * 0.9:
            break
        score = alpha_beta(board, depth, -INF, +INF, true)
        best_move = get_pv_move()  # 从置换表获取主变例
        if is_mate_score(score):
            break  # 找到必胜/必败，提前结束
    return best_move
```

**关键数据结构**：
- `TranspositionTable`：哈希表，存储已搜索局面的分数和最佳走法
- `HistoryTable`：历史启发表，记录走法的成功剪枝次数
- `KillerMoves`：杀手走法表，每层深度记录2个最佳剪枝走法

#### 5. Game Control Module（游戏控制模块）

**职责**：
- 管理完整的游戏状态（棋盘 + 历史）
- 执行和撤销走法（make/unmake）
- 检测重复局面和50步规则
- 管理时间分配策略

**时间管理策略**：
```
总时间分配：
  - 开局（前15步）：5%
  - 中局（16-40步）：60%
  - 残局（41步后）：35%

单步时间计算：
  time_for_move = remaining_time / expected_remaining_moves * safety_factor
  其中 safety_factor = 0.95（预留5%缓冲）
  
  expected_remaining_moves:
    - 开局：50步
    - 中局：30步
    - 残局：20步
```

**关键函数**：
- `make_move(Move)`：执行走法，更新棋盘和历史
- `unmake_move()`：撤销上一步走法
- `is_draw()`：检查是否和棋（重复局面或50步规则）
- `allocate_time()`：计算本步允许的思考时间

#### 6. Competition Interface Module（比赛接口模块）

**职责**：
- 解析比赛平台的输入指令
- 格式化输出AI的走法
- 处理协议错误和异常情况

**协议格式**（基于常见博弈平台）：
```
输入指令：
  - "START BLACK" / "START WHITE"：开始游戏，指定己方颜色
  - "MOVE 15-19"：对手的普通移动
  - "MOVE 15x24x33"：对手的跳吃走法
  - "QUIT"：结束游戏

输出格式：
  - "MOVE 15-19"：普通移动
  - "MOVE 15x24x33"：跳吃走法
  - "ERROR: Invalid move"：错误信息
```

### 数据流

**典型对局流程**：
```
1. Competition Interface 接收 "START BLACK"
   ↓
2. Game Control 初始化棋盘为开局状态
   ↓
3. 轮到己方行棋：
   a. 查询开局库（如果在开局阶段）
   b. 如果不在开局库，调用 Search Engine
   c. Search Engine 调用 Move Generator 和 Evaluation
   d. 返回最佳走法
   ↓
4. Game Control 执行走法，更新状态
   ↓
5. Competition Interface 输出走法
   ↓
6. Competition Interface 接收对手走法
   ↓
7. Game Control 执行对手走法
   ↓
8. 重复步骤3-7，直到游戏结束
```

## Components and Interfaces

### 1. Board类（核心棋盘）

```cpp
class Board {
public:
    // 位棋盘表示（4个64位整数）
    uint64_t black_men;    // 黑方普通棋子
    uint64_t white_men;    // 白方普通棋子
    uint64_t black_kings;  // 黑方王棋
    uint64_t white_kings;  // 白方王棋
    
    int current_player;    // 1=黑方，-1=白方
    uint64_t hash;         // Zobrist哈希值（用于置换表）
    
    // 构造函数
    Board();  // 初始化为开局状态
    
    // 基本查询
    uint64_t get_all_black() const;
    uint64_t get_all_white() const;
    uint64_t get_empty_squares() const;
    uint64_t get_occupied_squares() const;
    
    // 走法执行
    void make_move(const Move& move);
    void unmake_move(const Move& move);
    
    // 辅助功能
    void print_board() const;
    bool is_valid_square(int sq) const;
    int square_to_index(int row, int col) const;
    void index_to_square(int index, int& row, int& col) const;
};
```

### 2. Move结构（走法表示）

```cpp
struct Move {
    int from;              // 起始格子（0-49）
    int to;                // 目标格子（0-49）
    int captures[12];      // 被吃掉的棋子位置（最多12个连续跳吃）
    int num_captures;      // 吃子数量
    bool is_promotion;     // 是否升王
    int score;             // 走法排序分数（用于搜索优化）
    
    Move() : from(-1), to(-1), num_captures(0), is_promotion(false), score(0) {}
    
    // 走法表示转换
    std::string to_string() const;  // 转换为"15-19"或"15x24x33"格式
    static Move from_string(const std::string& str);  // 从字符串解析
};

using MoveList = std::vector<Move>;
```

### 3. MoveGenerator类（走法生成器）

```cpp
class MoveGenerator {
public:
    // 生成所有合法走法
    static void generate_moves(const Board& board, MoveList& moves);
    
private:
    // 生成吃子走法（优先级高）
    static void generate_captures(const Board& board, MoveList& moves);
    static void generate_man_captures(const Board& board, int square, 
                                      uint64_t captured, MoveList& moves);
    static void generate_king_captures(const Board& board, int square, 
                                       uint64_t captured, MoveList& moves);
    
    // 生成非吃子走法
    static void generate_quiet_moves(const Board& board, MoveList& moves);
    static void generate_man_moves(const Board& board, int square, MoveList& moves);
    static void generate_king_moves(const Board& board, int square, MoveList& moves);
    
    // 辅助函数
    static bool has_captures(const Board& board);
    static void filter_max_captures(MoveList& moves);  // 最大吃子规则
    
    // 预计算表（静态初始化）
    static uint64_t king_moves[50][4];     // 王棋的4个方向移动掩码
    static int man_capture_offsets[4];     // 普通棋子跳吃偏移量
    static int king_directions[4];         // 王棋移动方向
};
```

### 4. Evaluator类（评估器）

```cpp
class Evaluator {
public:
    // 评估局面（从当前玩家视角）
    static int evaluate(const Board& board);
    
    // 检查终局
    static bool is_terminal(const Board& board, int& result);
    // result: 1=当前玩家胜, -1=当前玩家负, 0=和棋
    
private:
    // 评估子项
    static int evaluate_material(const Board& board);
    static int evaluate_position(const Board& board);
    static int evaluate_mobility(const Board& board);
    static int evaluate_safety(const Board& board);
    static int evaluate_structure(const Board& board);
    
    // 权重参数（可配置）
    static constexpr int MAN_VALUE = 100;
    static constexpr int KING_VALUE = 300;
    static constexpr int CENTER_BONUS = 10;
    static constexpr int MOBILITY_WEIGHT = 5;
    static constexpr int SAFETY_BONUS = 10;
    static constexpr int EXPOSED_PENALTY = -15;
    
    // 位置价值表（预计算）
    static int position_value[50];
};
```

### 5. SearchEngine类（搜索引擎）

```cpp
class SearchEngine {
public:
    SearchEngine(size_t tt_size_mb = 256);
    
    // 主搜索接口
    Move search(const Board& board, int time_limit_ms);
    
    // 配置
    void set_max_depth(int depth) { max_depth = depth; }
    void clear_tables();
    
    // 统计信息
    uint64_t get_nodes_searched() const { return nodes_searched; }
    int get_search_depth() const { return completed_depth; }
    
private:
    // Alpha-Beta搜索
    int alpha_beta(Board& board, int depth, int alpha, int beta, bool maximizing);
    
    // 迭代加深
    Move iterative_deepening(Board& board, int time_limit_ms);
    
    // 走法排序
    void sort_moves(MoveList& moves, const Board& board, int depth);
    int score_move(const Move& move, const Board& board, int depth);
    
    // 静止搜索（Quiescence Search）
    int quiescence_search(Board& board, int alpha, int beta);
    
    // 数据成员
    TranspositionTable tt;
    HistoryTable history;
    KillerMoves killers;
    
    int max_depth;
    uint64_t nodes_searched;
    int completed_depth;
    std::chrono::time_point<std::chrono::steady_clock> search_start_time;
    int time_limit_ms;
};
```

### 6. TranspositionTable类（置换表）

```cpp
struct TTEntry {
    uint64_t hash;         // Zobrist哈希值
    int depth;             // 搜索深度
    int score;             // 评估分数
    Move best_move;        // 最佳走法
    enum Flag { EXACT, LOWER_BOUND, UPPER_BOUND } flag;
};

class TranspositionTable {
public:
    TranspositionTable(size_t size_mb);
    
    bool probe(uint64_t hash, int depth, int alpha, int beta, int& score);
    void store(uint64_t hash, int depth, int score, const Move& best_move, 
               TTEntry::Flag flag);
    void clear();
    
    // 统计
    double get_hit_rate() const;
    
private:
    std::vector<TTEntry> table;
    size_t size;
    uint64_t hits;
    uint64_t probes;
};
```

### 7. GameState类（游戏状态管理）

```cpp
class GameState {
public:
    GameState();
    
    // 走法执行
    void make_move(const Move& move);
    void unmake_move();
    
    // 状态查询
    const Board& get_board() const { return board; }
    bool is_draw() const;
    bool is_game_over(int& result) const;
    
    // 历史管理
    int get_move_count() const { return history.size(); }
    const Move& get_last_move() const;
    
private:
    Board board;
    std::vector<Move> history;
    std::vector<Board> board_history;  // 用于检测重复局面
    int halfmove_clock;  // 50步规则计数器
    
    // 重复局面检测
    int count_repetitions() const;
};
```

### 8. TimeManager类（时间管理）

```cpp
class TimeManager {
public:
    TimeManager(int total_time_ms);
    
    // 计算本步允许的思考时间
    int allocate_time(int move_number, int remaining_time_ms);
    
    // 检查是否超时
    bool should_stop(int elapsed_ms, int allocated_ms) const;
    
    // 更新统计
    void record_move_time(int move_number, int time_used_ms);
    
private:
    int total_time;
    std::vector<int> time_used_per_move;
    
    // 时间分配策略
    double get_time_factor(int move_number) const;
};
```

### 9. CompetitionInterface类（比赛接口）

```cpp
class CompetitionInterface {
public:
    CompetitionInterface();
    
    // 主循环
    void run();
    
private:
    // 协议处理
    void handle_start_command(const std::string& color);
    void handle_move_command(const std::string& move_str);
    void handle_quit_command();
    
    // 输出
    void send_move(const Move& move);
    void send_error(const std::string& message);
    
    // 数据成员
    GameState game_state;
    SearchEngine search_engine;
    TimeManager time_manager;
    bool is_my_turn;
    int my_color;  // 1=黑方，-1=白方
};
```

## Data Models

### Bitboard表示详解

**格子编号映射**：
```
10×10棋盘，只有50个黑色格子有效（编号1-50）
在位棋盘中，使用索引0-49表示这50个格子

棋盘布局（黑格编号）：
    0   1   2   3   4   5   6   7   8   9  (列)
0      1       2       3       4       5
1  6       7       8       9      10
2     11      12      13      14      15
3 16      17      18      19      20
4     21      22      23      24      25
5 26      27      28      29      30
6     31      32      33      34      35
7 36      37      38      39      40
8     41      42      43      44      45
9 46      47      48      49      50

索引计算公式：
  index = row * 5 + col / 2
  其中 (row + col) % 2 == 1 表示黑格
```

**位运算操作**：
```cpp
// 设置位（放置棋子）
board.black_men |= (1ULL << index);

// 清除位（移除棋子）
board.white_men &= ~(1ULL << index);

// 检查位（查询是否有棋子）
bool has_piece = (board.black_kings & (1ULL << index)) != 0;

// 计数（统计棋子数量）
int count = __builtin_popcountll(board.black_men);

// 获取最低位（找到第一个棋子）
int first_piece = __builtin_ctzll(board.white_kings);

// 移位（模拟移动）
uint64_t moved = (board.black_men >> 5);  // 向上移动一行
```

**方向移动掩码**：
```cpp
// 普通棋子的4个跳吃方向（黑方向下，白方向上）
// 黑方跳吃偏移：左下(-4), 右下(-6), 左上(+6), 右上(+4)
// 白方跳吃偏移：左上(+6), 右上(+4), 左下(-4), 右下(-6)

// 王棋的4个对角线方向
enum Direction {
    UP_LEFT = 6,      // 向左上
    UP_RIGHT = 4,     // 向右上
    DOWN_LEFT = -4,   // 向左下
    DOWN_RIGHT = -6   // 向右下
};

// 预计算每个格子在每个方向上可以移动的所有目标格子
// king_moves[square][direction] = 该方向上所有可达格子的位掩码
uint64_t king_moves[50][4];
```

### Zobrist哈希

用于置换表的快速局面识别：

```cpp
class ZobristHash {
public:
    static void init();  // 初始化随机数表
    
    static uint64_t compute_hash(const Board& board);
    
    // 增量更新（移动棋子时）
    static uint64_t update_hash(uint64_t old_hash, int piece_type, 
                                int from_square, int to_square);
    
private:
    // 随机数表：[棋子类型][格子位置]
    static uint64_t zobrist_table[4][50];  // 4种棋子类型，50个格子
    static uint64_t zobrist_side;          // 当前玩家
};

// 使用示例：
board.hash = ZobristHash::compute_hash(board);

// 执行走法时增量更新：
board.hash ^= zobrist_table[BLACK_MAN][from];  // 移除起点棋子
board.hash ^= zobrist_table[BLACK_MAN][to];    // 添加终点棋子
board.hash ^= zobrist_side;                    // 切换玩家
```

### 走法排序优先级

为了提高Alpha-Beta剪枝效率，走法按以下优先级排序：

```
1. 置换表走法（TT Move）：上次搜索的最佳走法
   分数：+10000

2. 吃子走法（Captures）：按MVV-LVA排序
   MVV-LVA = Most Valuable Victim - Least Valuable Attacker
   分数：+1000 + (被吃棋子价值 - 吃子棋子价值)
   
3. 杀手走法（Killer Moves）：该深度的历史最佳剪枝走法
   分数：+500

4. 历史启发（History Heuristic）：该走法的历史成功次数
   分数：history_table[from][to]

5. 其他走法：位置启发
   分数：position_value[to] - position_value[from]
```

### 内存布局优化

为了提高缓存命中率，关键数据结构采用紧凑布局：

```cpp
// Board类：64字节（正好一个缓存行）
struct Board {
    uint64_t black_men;      // 8字节
    uint64_t white_men;      // 8字节
    uint64_t black_kings;    // 8字节
    uint64_t white_kings;    // 8字节
    uint64_t hash;           // 8字节
    int current_player;      // 4字节
    int padding[7];          // 28字节填充，对齐到64字节
};

// Move结构：64字节
struct Move {
    int from;                // 4字节
    int to;                  // 4字节
    int captures[12];        // 48字节
    int num_captures;        // 4字节
    bool is_promotion;       // 1字节
    char padding1[3];        // 3字节填充
    int score;               // 4字节
};

// TTEntry：32字节（半个缓存行）
struct TTEntry {
    uint64_t hash;           // 8字节
    int depth;               // 4字节
    int score;               // 4字节
    Move best_move;          // 8字节（压缩表示：from+to+flags）
    uint8_t flag;            // 1字节
    char padding[7];         // 7字节填充
};
```


## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property Reflection

在分析了所有验收标准后，我识别出以下适合property-based testing的核心properties。经过反思，我发现一些properties可以合并以避免冗余：

**合并决策：**
- 需求1.1和1.2（普通棋子和王棋的移动规则）可以合并为一个综合的"合法移动生成"property
- 需求1.6和1.7（目标格子检查和边界检查）可以合并到需求1.8的综合正确性检查中
- 需求2.1和2.2（黑方和白方升王）可以合并为一个通用的升王property
- 需求9.5和9.8（状态一致性）是重复的，合并为一个round-trip property

### Property 1: 合法走法生成的完整性和正确性

*For any* 有效的棋盘状态，生成的所有走法在执行后应产生符合国际跳棋规则的新棋盘状态，包括：
- 所有走法的起点和终点都在有效范围内（0-49）
- 非吃子走法的目标格子必须为空
- 吃子走法必须跳过对手棋子
- 棋子不会重叠
- 棋子总数的变化符合吃子数量

**Validates: Requirements 1.1, 1.2, 1.6, 1.7, 1.8**

### Property 2: 强制吃子规则

*For any* 有吃子机会的棋盘状态，生成的所有走法都必须是吃子走法（num_captures > 0），不应包含任何非吃子走法。

**Validates: Requirements 1.3**

### Property 3: 最大吃子规则

*For any* 有多个吃子选项的棋盘状态，生成的所有走法的吃子数量都应等于所有可能吃子走法中的最大吃子数量。

**Validates: Requirements 1.5**

### Property 4: 升王规则的正确性

*For any* 普通棋子移动到对方底线的走法（黑方到46-50号格子，白方到1-5号格子），该走法的is_promotion标志应为true，且执行后该棋子应出现在对应的kings位棋盘中而不是men位棋盘中。

**Validates: Requirements 2.1, 2.2**

### Property 5: 升王后的移动能力

*For any* 刚升王的棋子，在下一回合应能够生成符合王棋规则的走法（沿对角线移动任意格数），而不是普通棋子的走法（仅向前移动一格）。

**Validates: Requirements 2.5**

### Property 6: 搜索深度的单调性

*For any* 给定的棋盘状态和两个搜索深度d1 < d2，深度d2的搜索结果的评估分数的准确性应不低于深度d1（即更深的搜索应找到至少同样好或更好的走法）。

**Validates: Requirements 3.8**

### Property 7: 评估函数的材料一致性

*For any* 两个棋盘状态，如果一个棋盘的材料优势（棋子数量和类型）明显优于另一个，则评估分数应反映这种优势（材料多的一方评估分数更高）。

**Validates: Requirements 4.1**

### Property 8: 王棋价值高于普通棋子

*For any* 两个仅在棋子类型上不同的棋盘状态（一个有n个普通棋子，另一个有n个王棋，其他条件相同），王棋棋盘的评估分数的绝对值应大于普通棋子棋盘。

**Validates: Requirements 4.2**

### Property 9: 评估函数的对称性

*For any* 棋盘状态及其镜像状态（黑白棋子互换，当前玩家互换），两个状态的评估分数的绝对值应相等（即eval(board) = -eval(mirror(board))）。

**Validates: Requirements 4.9**

### Property 10: 走法格式转换的往返一致性

*For any* 有效的走法对象，将其转换为字符串格式（如"15-19"或"15x24x33"）后再解析回走法对象，应得到与原始走法完全相同的走法（起点、终点、吃子序列都相同）。

**Validates: Requirements 6.6**

### Property 11: 走法执行和撤销的往返一致性

*For any* 有效的棋盘状态和合法走法，执行make_move后立即执行unmake_move，应恢复到与原始状态完全相同的棋盘状态（所有4个位棋盘、当前玩家、哈希值都相同）。

**Validates: Requirements 9.5, 9.8**

## Error Handling

### 错误分类和处理策略

系统采用分层的错误处理策略，根据错误的严重程度采取不同的应对措施：

#### 1. 致命错误（Fatal Errors）

这些错误表示系统无法继续运行，必须终止程序：

**错误类型：**
- 内存分配失败（无法分配置换表）
- 核心数据结构损坏（棋盘状态不一致）
- 无法恢复的内部逻辑错误

**处理策略：**
```cpp
try {
    transposition_table = new TTEntry[table_size];
} catch (std::bad_alloc& e) {
    std::cerr << "FATAL: Cannot allocate transposition table: " 
              << e.what() << std::endl;
    std::cerr << "Attempting to reduce table size..." << std::endl;
    
    // 尝试降级到更小的表
    try {
        table_size /= 4;
        transposition_table = new TTEntry[table_size];
        std::cerr << "Successfully allocated reduced table" << std::endl;
    } catch (std::bad_alloc& e2) {
        std::cerr << "FATAL: Cannot allocate even reduced table. Exiting." 
                  << std::endl;
        exit(1);
    }
}
```

#### 2. 可恢复错误（Recoverable Errors）

这些错误可以通过降级或使用默认值来恢复：

**错误类型：**
- 配置文件加载失败
- 开局库/残局库加载失败
- 无效的输入指令

**处理策略：**
```cpp
bool load_config(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "WARNING: Cannot open config file: " << filename 
                  << std::endl;
        std::cerr << "Using default configuration" << std::endl;
        use_default_config();
        return false;
    }
    
    try {
        // 解析配置文件
        parse_config(file);
        return true;
    } catch (std::exception& e) {
        std::cerr << "WARNING: Error parsing config: " << e.what() 
                  << std::endl;
        std::cerr << "Using default configuration" << std::endl;
        use_default_config();
        return false;
    }
}
```

#### 3. 输入验证错误（Input Validation Errors）

这些错误来自外部输入，需要拒绝并请求重新输入：

**错误类型：**
- 无效的走法格式
- 非法的走法（不符合规则）
- 无法识别的协议指令

**处理策略：**
```cpp
Move parse_move(const std::string& move_str) {
    // 验证格式
    if (!is_valid_format(move_str)) {
        throw std::invalid_argument("Invalid move format: " + move_str);
    }
    
    Move move = parse_move_string(move_str);
    
    // 验证走法合法性
    if (!is_legal_move(current_board, move)) {
        throw std::invalid_argument("Illegal move: " + move_str);
    }
    
    return move;
}

// 在接口层捕获并处理
void handle_move_command(const std::string& move_str) {
    try {
        Move move = parse_move(move_str);
        game_state.make_move(move);
    } catch (std::invalid_argument& e) {
        std::cout << "ERROR: " << e.what() << std::endl;
        std::cout << "Please enter a valid move" << std::endl;
        // 不更新状态，等待下一个输入
    }
}
```

#### 4. 超时错误（Timeout Errors）

搜索超时不是错误，而是正常的时间管理：

**处理策略：**
```cpp
Move search_with_timeout(const Board& board, int time_limit_ms) {
    search_start_time = std::chrono::steady_clock::now();
    this->time_limit_ms = time_limit_ms;
    
    Move best_move;
    try {
        best_move = iterative_deepening(board, time_limit_ms);
    } catch (TimeoutException& e) {
        // 超时是正常情况，返回当前最佳走法
        std::cerr << "Search timeout at depth " << completed_depth 
                  << std::endl;
    }
    
    if (!best_move.is_valid()) {
        // 如果没有找到任何走法（极端情况），生成一个合法走法
        MoveList moves;
        MoveGenerator::generate_moves(board, moves);
        if (!moves.empty()) {
            best_move = moves[0];
        } else {
            throw std::runtime_error("No legal moves available");
        }
    }
    
    return best_move;
}
```

### 断言和调试检查

在开发和测试阶段使用断言检查内部不变量，在发布版本中禁用：

```cpp
#ifdef DEBUG
    #define ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                std::cerr << "Assertion failed: " << message << std::endl; \
                std::cerr << "File: " << __FILE__ << ", Line: " << __LINE__ << std::endl; \
                std::abort(); \
            } \
        } while (0)
#else
    #define ASSERT(condition, message) do { } while (0)
#endif

// 使用示例
void make_move(const Move& move) {
    ASSERT(move.from >= 0 && move.from < 50, "Invalid from square");
    ASSERT(move.to >= 0 && move.to < 50, "Invalid to square");
    ASSERT(is_legal_move(board, move), "Illegal move");
    
    // 执行走法...
}
```

### 错误日志

所有错误都应记录到日志文件（在调试模式下）：

```cpp
class Logger {
public:
    static void log_error(const std::string& message) {
        if (log_file.is_open()) {
            log_file << "[ERROR] " << get_timestamp() << ": " 
                     << message << std::endl;
        }
        std::cerr << message << std::endl;
    }
    
    static void log_warning(const std::string& message) {
        if (log_file.is_open()) {
            log_file << "[WARNING] " << get_timestamp() << ": " 
                     << message << std::endl;
        }
    }
    
private:
    static std::ofstream log_file;
    static std::string get_timestamp();
};
```

## Testing Strategy

### 测试方法概述

本系统采用多层次的测试策略，结合单元测试、property-based testing、集成测试和性能测试，确保系统的正确性、鲁棒性和性能。

### 1. Property-Based Testing（属性测试）

**测试框架选择：** RapidCheck（C++的property-based testing库）

**配置要求：**
- 每个property测试运行最少100次迭代
- 使用随机种子确保可重现性
- 每个测试标注对应的设计property编号

**测试实现示例：**

```cpp
#include <rapidcheck.h>

// Property 1: 合法走法生成的完整性和正确性
TEST_CASE("Property 1: Generated moves produce valid board states") {
    rc::check("All generated moves result in valid boards",
        [](const Board& board) {
            // 生成所有走法
            MoveList moves;
            MoveGenerator::generate_moves(board, moves);
            
            // 对每个走法进行验证
            for (const Move& move : moves) {
                // 检查范围
                RC_ASSERT(move.from >= 0 && move.from < 50);
                RC_ASSERT(move.to >= 0 && move.to < 50);
                
                // 执行走法
                Board new_board = board;
                new_board.make_move(move);
                
                // 验证新棋盘的合法性
                RC_ASSERT(is_valid_board(new_board));
                
                // 验证棋子数量变化
                int old_count = count_pieces(board);
                int new_count = count_pieces(new_board);
                RC_ASSERT(new_count == old_count - move.num_captures);
            }
        });
}

// Property 2: 强制吃子规则
TEST_CASE("Property 2: Mandatory capture rule") {
    rc::check("When captures available, only capture moves generated",
        []() {
            // 生成有吃子机会的随机棋盘
            Board board = *rc::gen::suchThat(
                rc::gen::arbitrary<Board>(),
                [](const Board& b) { return has_captures(b); }
            );
            
            MoveList moves;
            MoveGenerator::generate_moves(board, moves);
            
            // 所有走法都应该是吃子走法
            for (const Move& move : moves) {
                RC_ASSERT(move.num_captures > 0);
            }
        });
}

// Property 11: 走法执行和撤销的往返一致性
TEST_CASE("Property 11: Make/unmake move round-trip") {
    rc::check("Make then unmake restores original state",
        [](const Board& original_board) {
            MoveList moves;
            MoveGenerator::generate_moves(original_board, moves);
            
            if (moves.empty()) return;  // 跳过无走法的局面
            
            // 选择一个随机走法
            const Move& move = *rc::gen::elementOf(moves);
            
            // 执行走法
            Board board = original_board;
            board.make_move(move);
            
            // 撤销走法
            board.unmake_move(move);
            
            // 验证完全恢复
            RC_ASSERT(board.black_men == original_board.black_men);
            RC_ASSERT(board.white_men == original_board.white_men);
            RC_ASSERT(board.black_kings == original_board.black_kings);
            RC_ASSERT(board.white_kings == original_board.white_kings);
            RC_ASSERT(board.current_player == original_board.current_player);
            RC_ASSERT(board.hash == original_board.hash);
        });
}
```

**随机数据生成器：**

```cpp
namespace rc {
    template<>
    struct Arbitrary<Board> {
        static Gen<Board> arbitrary() {
            return gen::build<Board>(
                gen::set(&Board::black_men, gen::arbitrary<uint64_t>()),
                gen::set(&Board::white_men, gen::arbitrary<uint64_t>()),
                gen::set(&Board::black_kings, gen::arbitrary<uint64_t>()),
                gen::set(&Board::white_kings, gen::arbitrary<uint64_t>()),
                gen::set(&Board::current_player, gen::element(1, -1)),
                gen::exec([](Board& b) {
                    // 确保棋子不重叠
                    uint64_t all_pieces = b.black_men | b.white_men | 
                                         b.black_kings | b.white_kings;
                    // 修正重叠...
                    
                    // 限制在50个有效格子内
                    uint64_t mask = (1ULL << 50) - 1;
                    b.black_men &= mask;
                    b.white_men &= mask;
                    b.black_kings &= mask;
                    b.white_kings &= mask;
                    
                    // 计算哈希
                    b.hash = ZobristHash::compute_hash(b);
                })
            );
        }
    };
}
```

### 2. Unit Testing（单元测试）

**测试框架：** Catch2

**测试范围：**
- 走法生成的特定场景（连续跳吃、升王等）
- 评估函数的各个子项
- 时间管理的边界情况
- 协议解析和格式转换

**测试示例：**

```cpp
#include <catch2/catch.hpp>

TEST_CASE("Move generation: Multiple captures", "[movegen]") {
    // 构造一个有连续跳吃机会的棋盘
    Board board;
    board.black_men = 0;
    board.white_men = 0;
    board.black_kings = 0;
    board.white_kings = 0;
    
    // 设置特定布局：黑子在15，白子在19和24
    board.black_men |= (1ULL << 15);
    board.white_men |= (1ULL << 19);
    board.white_men |= (1ULL << 24);
    board.current_player = 1;  // 黑方
    
    MoveList moves;
    MoveGenerator::generate_moves(board, moves);
    
    // 应该生成一个连续跳吃走法：15x24x33
    REQUIRE(moves.size() == 1);
    REQUIRE(moves[0].from == 15);
    REQUIRE(moves[0].to == 33);
    REQUIRE(moves[0].num_captures == 2);
    REQUIRE(moves[0].captures[0] == 19);
    REQUIRE(moves[0].captures[1] == 24);
}

TEST_CASE("Evaluation: Material advantage", "[eval]") {
    Board board1, board2;
    
    // board1: 黑方有5个普通棋子，白方有3个
    board1.black_men = 0x1F;  // 前5位
    board1.white_men = 0x07;  // 前3位
    board1.current_player = 1;
    
    // board2: 黑方有3个普通棋子，白方有5个
    board2.black_men = 0x07;
    board2.white_men = 0x1F;
    board2.current_player = 1;
    
    int eval1 = Evaluator::evaluate(board1);
    int eval2 = Evaluator::evaluate(board2);
    
    // board1对黑方更有利，评估分数应该更高
    REQUIRE(eval1 > eval2);
}

TEST_CASE("Protocol: Move string parsing", "[protocol]") {
    // 测试普通移动
    Move move1 = Move::from_string("15-19");
    REQUIRE(move1.from == 15);
    REQUIRE(move1.to == 19);
    REQUIRE(move1.num_captures == 0);
    
    // 测试跳吃
    Move move2 = Move::from_string("15x24x33");
    REQUIRE(move2.from == 15);
    REQUIRE(move2.to == 33);
    REQUIRE(move2.num_captures == 2);
    
    // 测试无效格式
    REQUIRE_THROWS(Move::from_string("invalid"));
    REQUIRE_THROWS(Move::from_string("15-60"));  // 超出范围
}
```

### 3. Integration Testing（集成测试）

**测试范围：**
- 完整的对局流程（从开局到终局）
- 比赛接口协议的完整交互
- 开局库和残局库的集成
- 跨平台编译和运行

**测试方法：**

```cpp
TEST_CASE("Integration: Complete game flow", "[integration]") {
    CompetitionInterface interface;
    
    // 模拟比赛平台的输入
    std::istringstream input(
        "START BLACK\n"
        "MOVE 31-27\n"  // 白方走法
        "MOVE 32-28\n"
        // ... 更多走法
        "QUIT\n"
    );
    
    std::ostringstream output;
    
    // 重定向标准输入输出
    std::cin.rdbuf(input.rdbuf());
    std::cout.rdbuf(output.rdbuf());
    
    // 运行接口
    interface.run();
    
    // 验证输出包含合法的走法
    std::string output_str = output.str();
    REQUIRE(output_str.find("MOVE") != std::string::npos);
}

TEST_CASE("Integration: Self-play", "[integration]") {
    GameState game;
    SearchEngine engine1(128);  // 128MB置换表
    SearchEngine engine2(128);
    
    int move_count = 0;
    const int MAX_MOVES = 200;
    
    while (!game.is_game_over() && move_count < MAX_MOVES) {
        SearchEngine& current_engine = 
            (game.get_board().current_player == 1) ? engine1 : engine2;
        
        Move move = current_engine.search(game.get_board(), 1000);  // 1秒
        REQUIRE(move.is_valid());
        
        game.make_move(move);
        move_count++;
    }
    
    // 验证游戏正常结束
    REQUIRE(move_count < MAX_MOVES);  // 不应该达到最大步数限制
}
```

### 4. Performance Testing（性能测试）

**测试指标：**
- 每秒搜索节点数（NPS）：目标 > 100,000
- 走法生成速度：目标 < 10微秒
- 评估函数速度：目标 < 200微秒
- 置换表命中率：目标 > 80%

**性能测试实现：**

```cpp
TEST_CASE("Performance: Move generation speed", "[performance]") {
    Board board;  // 初始局面
    
    const int ITERATIONS = 100000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < ITERATIONS; i++) {
        MoveList moves;
        MoveGenerator::generate_moves(board, moves);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end - start).count();
    
    double avg_time = static_cast<double>(duration) / ITERATIONS;
    
    std::cout << "Average move generation time: " << avg_time 
              << " microseconds" << std::endl;
    
    REQUIRE(avg_time < 10.0);  // 应该小于10微秒
}

TEST_CASE("Performance: Search nodes per second", "[performance]") {
    Board board;
    SearchEngine engine(256);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    Move move = engine.search(board, 5000);  // 5秒搜索
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();
    
    uint64_t nodes = engine.get_nodes_searched();
    double nps = (nodes * 1000.0) / duration;
    
    std::cout << "Nodes searched: " << nodes << std::endl;
    std::cout << "Time: " << duration << " ms" << std::endl;
    std::cout << "NPS: " << nps << std::endl;
    
    REQUIRE(nps > 100000);  // 应该大于100,000 NPS
}

TEST_CASE("Performance: Transposition table hit rate", "[performance]") {
    Board board;
    SearchEngine engine(256);
    
    // 进行多次搜索
    for (int depth = 1; depth <= 8; depth++) {
        engine.search(board, 1000);
    }
    
    double hit_rate = engine.get_tt_hit_rate();
    
    std::cout << "TT hit rate: " << (hit_rate * 100) << "%" << std::endl;
    
    REQUIRE(hit_rate > 0.8);  // 应该大于80%
}
```

### 5. Regression Testing（回归测试）

**测试套件：**
- 标准测试局面集（Checkers Test Suite）
- 已知的战术问题（Tactical Puzzles）
- 历史bug的测试用例

```cpp
TEST_CASE("Regression: Known tactical positions", "[regression]") {
    struct TacticalTest {
        std::string fen;  // 局面描述
        std::string expected_move;  // 期望的最佳走法
        std::string description;
    };
    
    std::vector<TacticalTest> tests = {
        {"...", "15x24x33", "Double jump wins piece"},
        {"...", "27-31", "Promotion threat"},
        // ... 更多测试用例
    };
    
    SearchEngine engine(256);
    
    for (const auto& test : tests) {
        Board board = Board::from_fen(test.fen);
        Move move = engine.search(board, 5000);
        
        INFO("Test: " << test.description);
        REQUIRE(move.to_string() == test.expected_move);
    }
}
```

### 测试覆盖率目标

- **代码覆盖率**：> 85%（核心模块 > 95%）
- **分支覆盖率**：> 80%
- **Property测试迭代**：每个property最少100次

### 持续集成

使用GitHub Actions或类似CI系统自动运行所有测试：

```yaml
# .github/workflows/ci.yml
name: CI

on: [push, pull_request]

jobs:
  test:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest]
        
    steps:
    - uses: actions/checkout@v2
    
    - name: Build
      run: |
        cmake -B build -DCMAKE_BUILD_TYPE=Release
        cmake --build build
        
    - name: Run unit tests
      run: ./build/tests/unit_tests
      
    - name: Run property tests
      run: ./build/tests/property_tests
      
    - name: Run integration tests
      run: ./build/tests/integration_tests
      
    - name: Run performance tests
      run: ./build/tests/performance_tests
```

---

## 总结

本设计文档详细描述了国际跳棋AI程序的技术架构、核心算法、数据结构和测试策略。系统采用模块化设计，使用高效的位运算和经典的Alpha-Beta搜索算法，结合property-based testing确保正确性，能够满足比赛要求并达到高性能目标。

**下一步工作：**
1. 实现核心数据结构（Board、Move）
2. 实现走法生成器（MoveGenerator）
3. 实现评估函数（Evaluator）
4. 实现搜索引擎（SearchEngine）
5. 编写property-based tests验证正确性
6. 性能优化和调优
7. 准备比赛文档和答辩材料
