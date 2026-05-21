# AI中局劣势修复 - 设计文档

## 概述

本设计文档详细说明了修复国际跳棋AI在中局阶段（27-28步后）陷入劣势的bug。通过分析现有代码（boyi/boyi.cpp），我们识别出评估函数权重静态、搜索深度分配不合理、中局战术评估不足等根本原因。修复方案采用游戏阶段自适应评估、增加中局搜索深度、增强中局战术评估和改进走法排序四大策略。

**修复目标：**
- 在中局阶段（27-40步）保持或改善局面优势
- 提高中局战术意识（识别吃子机会、威胁、控制中心）
- 增强中局战略控制（王的活跃度、升王威胁）
- 确保开局和残局阶段性能不受影响

## 术语表

- **Bug_Condition (C)**: 触发bug的条件 - 对局进入中局阶段（走法数 >= 27 且游戏阶段为MIDGAME）
- **Property (P)**: 修复后的期望行为 - AI在中局阶段保持战术意识和战略控制
- **Preservation**: 必须保持不变的行为 - 开局和残局阶段的性能、规则遵守、时间管理
- **GamePhase**: 游戏阶段枚举 - OPENING（开局，前15步）、MIDGAME（中局，16-40步且棋子数>8）、ENDGAME（残局，棋子数<=8）
- **Evaluator**: 评估器类 - 负责评估棋盘局面的静态分数
- **SearchEngine**: 搜索引擎类 - 负责Alpha-Beta搜索和迭代加深
- **TimeManager**: 时间管理器类 - 负责分配每步的思考时间
- **MoveGenerator**: 走法生成器类 - 负责生成所有合法走法

## Bug详情

### Bug条件

Bug在对局进入中局阶段时触发。中局阶段的特征是：走法数达到27步以上，且棋盘上仍有足够多的棋子（总棋子数>8）。在这个阶段，AI的评估函数未能准确评估局面的战术和战略价值，导致选择次优走法，逐渐失去优势。

**形式化规范：**
```
FUNCTION isBugCondition(input)
  INPUT: input of type GamePosition
  OUTPUT: boolean
  
  move_count ← input.move_count
  total_pieces ← count_all_pieces(input.board)
  
  // 中局阶段定义
  is_midgame ← (move_count >= 27) AND (total_pieces > 8)
  
  RETURN is_midgame
END FUNCTION
```

### 示例

- **示例1**：走法数=28，黑方10子，白方9子 → Bug条件成立，AI可能选择保守走法而非主动进攻
- **示例2**：走法数=30，黑方8子，白方7子 → Bug条件成立，AI可能错失控制中心格子的机会
- **示例3**：走法数=32，黑方有王在边缘，白方有王在中心 → Bug条件成立，AI未能评估王的活跃度差异
- **边缘情况**：走法数=27，黑方5子，白方3子 → Bug条件不成立（总棋子数<=8，进入残局阶段）

## 预期行为

### 保持要求

**不变行为：**
- 开局阶段（前15步）的开局库使用和标准走法选择必须继续正常工作
- 残局阶段（8子或更少）的残局库查询和精确计算必须继续正常工作
- 强制吃子规则和最大吃子规则的执行必须继续正确

**范围：**
所有不涉及中局阶段（走法数<27或总棋子数<=8）的输入应完全不受此修复影响。这包括：
- 开局阶段的走法选择和评估
- 残局阶段的走法选择和评估
- 规则遵守（升王、连续跳吃、50步规则）
- 时间管理和超时控制
- 置换表和历史启发的使用

## 假设的根本原因

基于对boyi/boyi.cpp的代码分析，识别出以下最可能的问题：

1. **评估函数权重静态**：当前评估函数使用固定权重，未根据游戏阶段调整
   - `Evaluator`类中的权重常量（MAN_VALUE=100, KING_VALUE=350等）在所有阶段都相同
   - 中局阶段王的价值应该更高（因为王的机动性在中局更重要）
   - 中局阶段中心控制的价值应该更高

2. **搜索深度在中局不足**：`TimeManager::get_time_factor()`在中局阶段使用标准时间因子1.0
   - 中局是最复杂的阶段，可能需要更深的搜索
   - 当前时间分配：开局0.8倍，中局1.0倍，残局1.2倍
   - 中局关键阶段（27-35步）可能需要增加到1.3倍

3. **中局战术评估不足**：`Evaluator::evaluate_advanced()`包含多个评估项，但可能权重不足
   - `evaluate_key_squares()`评估中心控制，但权重可能太低（每个中心格子+15分）
   - `evaluate_king_activity()`评估王的活跃度，但可能未充分考虑中局王的进攻性
   - 缺少双王配合、包围战术等中局特定的战术评估

4. **走法排序在中局效率低**：`SearchEngine::score_move()`的走法排序可能未优先考虑中局战术走法
   - 当前排序：置换表走法(10000) > 吃子走法(1000+) > 杀手走法(500) > 历史启发 > 位置启发
   - 中局阶段的战术走法（如占据中心、王向中心移动）可能排序靠后

## 正确性属性

Property 1: Bug Condition - 中局阶段AI决策质量

_For any_ 游戏局面，当走法数>=27且总棋子数>8时（中局阶段），修复后的AI SHALL选择保持或改善局面优势的走法，具有足够的战术意识（识别吃子机会、威胁、控制中心）和战略控制（王的活跃度、升王威胁），评估分数的变化应反映真实的局面优劣。

**验证：需求 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8**

Property 2: Preservation - 非中局阶段行为保持

_For any_ 游戏局面，当走法数<27或总棋子数<=8时（非中局阶段），修复后的AI SHALL产生与原始AI相同或更好的走法质量，保持开局库使用、残局库查询、规则遵守、时间管理等所有现有功能。

**验证：需求 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8**

## 修复实现

### 需要的修改

假设我们的根本原因分析是正确的：

**文件**: `boyi/boyi.cpp`

**具体修改**:

1. **在Evaluator类中添加游戏阶段检测**：
   - 添加`enum GamePhase { OPENING, MIDGAME, ENDGAME }`枚举
   - 添加`static GamePhase get_game_phase(const Board& board, int move_count)`方法
   - 根据走法数和棋子数判断游戏阶段

2. **修改Evaluator类的评估权重**：
   - 在`evaluate_advanced()`中调用`get_game_phase()`
   - 根据游戏阶段动态调整权重：
     - 中局阶段：KING_VALUE 350→400，CENTER_BONUS 15→25，MOBILITY_WEIGHT 8→12
     - 中局阶段：`evaluate_king_activity()`的返回值乘以1.75倍
     - 中局阶段：`evaluate_key_squares()`的返回值乘以1.67倍

3. **修改TimeManager类的时间分配**：
   - 在`get_time_factor()`中添加中局关键阶段判断
   - 走法数在25-35之间时，返回1.3倍时间因子（而非1.0倍）
   - 确保不超过剩余时间的50%（安全机制）

4. **在Evaluator类中添加中局战术评估**：
   - 添加`static int evaluate_midgame_tactics(const Board& board)`方法
   - 评估双王配合（两个王在相邻对角线）：+30分
   - 评估包围对手棋子（形成夹击）：+20分
   - 评估控制对手升王路线：+15分
   - 评估形成连续攻击态势：+25分
   - 在`evaluate_advanced()`中，中局阶段调用此方法

5. **修改SearchEngine类的走法排序**：
   - 在`score_move()`中添加游戏阶段参数
   - 中局阶段，以下走法优先级提升：
     - 占据中心格子（21,22,26,27）：+300分
     - 王向中心移动：+200分
     - 形成双王配合：+250分
     - 威胁对手升王路线：+150分

### 代码修改示例

**修改1：添加游戏阶段检测（Evaluator类）**

```cpp
// 在Evaluator类中添加
enum GamePhase {
    OPENING,   // 开局：前15步
    MIDGAME,   // 中局：16-40步且棋子数>8
    ENDGAME    // 残局：棋子数<=8
};

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
```

**修改2：动态调整评估权重（Evaluator类）**

```cpp
// 修改evaluate_advanced()方法
int Evaluator::evaluate_advanced(const Board& board, int move_count) {
    // 检查终局
    int terminal_result;
    if (is_terminal(board, terminal_result)) {
        return terminal_result;
    }
    
    // 获取游戏阶段
    GamePhase phase = get_game_phase(board, move_count);
    
    // 综合评估
    int score = 0;
    score += evaluate_material(board);
    score += evaluate_position(board);
    score += evaluate_mobility(board);
    score += evaluate_safety(board);
    score += evaluate_structure(board);
    
    // 根据游戏阶段调整高级评估的权重
    if (phase == MIDGAME) {
        // 中局阶段：增加王活跃度和中心控制的权重
        score += (int)(evaluate_key_squares(board) * 1.67);
        score += evaluate_promotion_threat(board);
        score += (int)(evaluate_king_activity(board) * 1.75);
        score += evaluate_midgame_tactics(board);
    } else {
        // 开局和残局：使用标准权重
        score += evaluate_key_squares(board);
        score += evaluate_promotion_threat(board);
        score += evaluate_king_activity(board);
    }
    
    return score;
}
```

**修改3：增加中局搜索深度（TimeManager类）**

```cpp
// 修改get_time_factor()方法
double get_time_factor(int move_number) const {
    if (move_number <= 15) {
        // 开局阶段：使用较少时间（0.8倍）
        return 0.8;
    } else if (move_number >= 25 && move_number <= 35) {
        // 中局关键阶段：使用更多时间（1.3倍）
        return 1.3;
    } else if (move_number <= 40) {
        // 中局其他阶段：使用正常时间（1.0倍）
        return 1.0;
    } else {
        // 残局阶段：使用较多时间（1.2倍）
        return 1.2;
    }
}
```

**修改4：添加中局战术评估（Evaluator类）**

```cpp
// 在Evaluator类中添加新方法
static int evaluate_midgame_tactics(const Board& board) {
    bool is_black = (board.current_player == 1);
    uint64_t my_kings = is_black ? board.black_kings : board.white_kings;
    uint64_t opp_men = is_black ? board.white_men : board.black_men;
    uint64_t opp_pieces = is_black ? board.get_all_white() : board.get_all_black();
    
    int score = 0;
    
    // 1. 双王配合（两个王在相邻对角线）
    if (__builtin_popcountll(my_kings) >= 2) {
        uint64_t kings_copy = my_kings;
        while (kings_copy) {
            int sq1 = __builtin_ctzll(kings_copy);
            kings_copy &= kings_copy - 1;
            
            uint64_t remaining = kings_copy;
            while (remaining) {
                int sq2 = __builtin_ctzll(remaining);
                remaining &= remaining - 1;
                
                // 检查是否在相邻对角线（距离为4或6）
                int distance = abs(sq1 - sq2);
                if (distance == 4 || distance == 6) {
                    score += 30;  // 双王配合奖励
                }
            }
        }
    }
    
    // 2. 包围对手棋子（形成夹击）
    uint64_t opp_copy = opp_pieces;
    while (opp_copy) {
        int opp_sq = __builtin_ctzll(opp_copy);
        opp_copy &= opp_copy - 1;
        
        int adjacent_count = 0;
        for (int dir = 0; dir < 4; ++dir) {
            int offset = MoveGenerator::DIRECTIONS[dir];
            int adj = opp_sq + offset;
            
            if (MoveGenerator::is_valid_square(adj)) {
                uint64_t adj_mask = 1ULL << adj;
                if ((is_black ? board.get_all_black() : board.get_all_white()) & adj_mask) {
                    adjacent_count++;
                }
            }
        }
        
        // 如果对手棋子被3个或4个己方棋子包围
        if (adjacent_count >= 3) {
            score += 20;  // 包围奖励
        }
    }
    
    // 3. 控制对手升王路线
    uint64_t opp_men_copy = opp_men;
    while (opp_men_copy) {
        int opp_sq = __builtin_ctzll(opp_men_copy);
        opp_men_copy &= opp_men_copy - 1;
        
        int row = opp_sq / 5;
        int distance_to_promotion;
        
        if (!is_black) {
            distance_to_promotion = 9 - row;  // 对手是黑方
        } else {
            distance_to_promotion = row;  // 对手是白方
        }
        
        // 如果对手接近升王（2步内）
        if (distance_to_promotion <= 2) {
            // 检查我方是否控制升王路线
            bool blocking = false;
            for (int dir = 0; dir < 2; ++dir) {  // 只检查前进方向
                int offset = !is_black ? MoveGenerator::DIRECTIONS[dir] : MoveGenerator::DIRECTIONS[dir + 2];
                int target = opp_sq + offset;
                
                if (MoveGenerator::is_valid_square(target)) {
                    uint64_t target_mask = 1ULL << target;
                    if ((is_black ? board.get_all_black() : board.get_all_white()) & target_mask) {
                        blocking = true;
                        break;
                    }
                }
            }
            
            if (blocking) {
                score += 15;  // 控制升王路线奖励
            }
        }
    }
    
    // 4. 形成连续攻击态势（多个棋子向前推进）
    uint64_t my_men = is_black ? board.black_men : board.white_men;
    uint64_t men_copy = my_men;
    int advanced_count = 0;
    
    while (men_copy) {
        int sq = __builtin_ctzll(men_copy);
        men_copy &= men_copy - 1;
        
        int row = sq / 5;
        
        // 检查是否在前进位置（黑方：行>=5，白方：行<=4）
        if ((is_black && row >= 5) || (!is_black && row <= 4)) {
            advanced_count++;
        }
    }
    
    // 如果有3个或更多棋子在前进位置
    if (advanced_count >= 3) {
        score += 25;  // 连续攻击态势奖励
    }
    
    return score;
}
```

**修改5：改进中局走法排序（SearchEngine类）**

```cpp
// 修改score_move()方法，添加move_count参数
int score_move(const Move& move, const Board& board, int depth, int move_count) {
    int score = 0;
    
    // 1. 置换表走法（最高优先级）
    Move tt_move = tt.get_best_move(board.hash);
    if (tt_move.is_valid() && tt_move.from == move.from && tt_move.to == move.to) {
        return 10000;
    }
    
    // 2. 吃子走法（MVV-LVA）
    if (move.num_captures > 0) {
        score = 1000 + move.num_captures * 100;
        
        bool is_black = (board.current_player == 1);
        uint64_t opponent_kings = is_black ? board.white_kings : board.black_kings;
        
        for (int i = 0; i < move.num_captures; ++i) {
            if (move.captures[i] >= 0 && move.captures[i] < 50) {
                uint64_t capture_mask = 1ULL << move.captures[i];
                if (opponent_kings & capture_mask) {
                    score += 200;
                }
            }
        }
        
        return score;
    }
    
    // 3. 中局战术走法（仅在中局阶段）
    Evaluator::GamePhase phase = Evaluator::get_game_phase(board, move_count);
    if (phase == Evaluator::MIDGAME) {
        // 占据中心格子（21,22,26,27）
        const int center_squares[] = {21, 22, 26, 27};
        for (int center_sq : center_squares) {
            if (move.to == center_sq) {
                score += 300;
                break;
            }
        }
        
        // 王向中心移动
        bool is_black = (board.current_player == 1);
        uint64_t from_mask = 1ULL << move.from;
        bool is_king = (is_black && (board.black_kings & from_mask)) ||
                      (!is_black && (board.white_kings & from_mask));
        
        if (is_king) {
            int from_row = move.from / 5;
            int from_col = (move.from % 5) * 2 + (from_row % 2);
            int to_row = move.to / 5;
            int to_col = (move.to % 5) * 2 + (to_row % 2);
            
            int from_center_dist = abs(from_row - 4) + abs(from_col - 4);
            int to_center_dist = abs(to_row - 4) + abs(to_col - 4);
            
            if (to_center_dist < from_center_dist) {
                score += 200;  // 王向中心移动
            }
        }
        
        // 形成双王配合（简化检测）
        if (is_king) {
            uint64_t my_kings = is_black ? board.black_kings : board.white_kings;
            uint64_t other_kings = my_kings & ~from_mask;
            
            while (other_kings) {
                int other_sq = __builtin_ctzll(other_kings);
                other_kings &= other_kings - 1;
                
                int distance = abs(move.to - other_sq);
                if (distance == 4 || distance == 6) {
                    score += 250;  // 形成双王配合
                    break;
                }
            }
        }
        
        // 威胁对手升王路线
        uint64_t opp_men = is_black ? board.white_men : board.black_men;
        uint64_t opp_men_copy = opp_men;
        
        while (opp_men_copy) {
            int opp_sq = __builtin_ctzll(opp_men_copy);
            opp_men_copy &= opp_men_copy - 1;
            
            int opp_row = opp_sq / 5;
            int distance_to_promotion = !is_black ? (9 - opp_row) : opp_row;
            
            if (distance_to_promotion <= 2) {
                // 检查走法后是否阻挡升王路线
                for (int dir = 0; dir < 2; ++dir) {
                    int offset = !is_black ? MoveGenerator::DIRECTIONS[dir] : MoveGenerator::DIRECTIONS[dir + 2];
                    int target = opp_sq + offset;
                    
                    if (move.to == target) {
                        score += 150;  // 威胁对手升王路线
                        break;
                    }
                }
            }
        }
    }
    
    // 4. 杀手走法
    if (killers.is_killer(move, depth)) {
        return 500 + score;
    }
    
    // 5. 历史启发
    score += history.get(move.from, move.to);
    
    // 6. 位置启发
    if (move.from >= 0 && move.from < 50 && move.to >= 0 && move.to < 50) {
        score += Evaluator::position_value[move.to] - Evaluator::position_value[move.from];
    }
    
    return score;
}
```

## 测试策略

### 验证方法

测试策略遵循两阶段方法：首先，在未修复的代码上运行探索性测试以暴露bug的反例；然后，验证修复后的代码正确工作并保持现有行为。

### 探索性Bug条件检查

**目标**：在实施修复之前，在未修复的代码上暴露反例以演示bug。确认或反驳根本原因分析。如果反驳，我们需要重新假设。

**测试计划**：创建中局局面（27-30步），让未修复的AI搜索最佳走法，观察评估分数和走法质量。预期会看到AI选择保守走法、错失战术机会、评估分数不准确等问题。

**测试用例**：
1. **中局标准局面测试**：设置走法数=28，黑方10子（含2王），白方9子（含1王），黑方回合（未修复代码将失败）
2. **中心控制测试**：设置走法数=30，中心格子（22,23,27,28）为空，黑方有机会占据（未修复代码将失败）
3. **王活跃度测试**：设置走法数=32，黑方王在边缘，白方王在中心，黑方回合（未修复代码将失败）
4. **升王威胁测试**：设置走法数=29，白方有兵距离升王2步，黑方有机会阻挡（未修复代码可能失败）

**预期反例**：
- AI选择保守走法而非主动进攻
- 可能原因：评估函数权重不足，搜索深度不够，战术评估缺失

### 修复检查

**目标**：验证对于所有满足bug条件的输入，修复后的函数产生预期行为。

**伪代码：**
```
FOR ALL input WHERE isBugCondition(input) DO
  move := AI_search_fixed(input)
  result := evaluate_move_quality(input, move)
  ASSERT result.maintains_advantage OR result.improves_position
  ASSERT result.tactical_awareness >= THRESHOLD_MIDGAME
  ASSERT result.strategic_control >= THRESHOLD_MIDGAME
END FOR
```

### 保持检查

**目标**：验证对于所有不满足bug条件的输入，修复后的函数产生与原始函数相同或更好的结果。

**伪代码：**
```
FOR ALL input WHERE NOT isBugCondition(input) DO
  move_original := AI_search_original(input)
  move_fixed := AI_search_fixed(input)
  quality_original := evaluate_move_quality(input, move_original)
  quality_fixed := evaluate_move_quality(input, move_fixed)
  ASSERT quality_fixed >= quality_original
END FOR
```

**测试方法**：在未修复代码上观察开局和残局阶段的行为，然后编写基于属性的测试捕获该行为。

**测试用例**：
1. **开局保持测试**：观察未修复代码在开局阶段（走法数<=15）的走法选择，验证修复后继续使用开局库
2. **残局保持测试**：观察未修复代码在残局阶段（棋子数<=8）的走法选择，验证修复后继续使用残局库
3. **规则遵守保持测试**：观察未修复代码的强制吃子、最大吃子、升王规则执行，验证修复后继续正确
4. **时间管理保持测试**：观察未修复代码的时间分配和超时控制，验证修复后继续正常

### 单元测试

- 测试`Evaluator::get_game_phase()`在各种棋子数和走法数下返回正确的游戏阶段
- 测试`Evaluator::evaluate_midgame_tactics()`在各种中局局面下返回合理的分数
- 测试`TimeManager::get_time_factor()`在中局关键阶段返回1.3倍时间因子
- 测试`SearchEngine::score_move()`在中局阶段为战术走法分配更高分数

### 基于属性的测试

- 生成随机中局局面（走法数27-40，棋子数9-20），验证修复后的AI评估分数更准确
- 生成随机开局和残局局面，验证修复后的AI行为与原始AI一致或更好
- 测试所有游戏阶段的规则遵守（强制吃子、升王、50步规则）在许多场景下继续正确

### 集成测试

- 测试完整对局流程，修复后的AI在中局阶段保持优势
- 测试自我对弈（修复前vs修复后），统计中局阶段胜率
- 测试时间管理在整个对局中正常工作，无超时问题
