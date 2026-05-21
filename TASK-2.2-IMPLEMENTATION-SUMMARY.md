# 任务2.2实现总结：中局着法排序增强

## 任务概述

**任务**: 实现中局着法排序增强  
**Spec路径**: `.kiro/specs/ai-comprehensive-optimization`  
**需求引用**: 2.7  
**实现文件**: `boyi/boyi.cpp`

## 实现内容

### 1. 修改的方法和类

#### 1.1 SearchEngine类增强

在`SearchEngine`类中添加了`current_move_count`成员变量，用于跟踪当前游戏的走法计数：

```cpp
// 当前游戏的走法计数（用于游戏阶段检测）
int current_move_count;
```

#### 1.2 构造函数修改

更新构造函数以初始化`current_move_count`：

```cpp
SearchEngine(size_t tt_size_mb = 256) : tt(tt_size_mb), max_depth(64), current_move_count(0) {
    clear_tables();
}
```

#### 1.3 新增search重载方法

添加了带`move_count`参数的`search`方法重载：

```cpp
// 主搜索接口（带走法计数）
Move search(Board& board, int time_limit_ms, int move_count) {
    this->current_move_count = move_count;
    return search(board, time_limit_ms);
}
```

#### 1.4 sort_moves方法修改

修改`sort_moves`方法以传递`current_move_count`给`score_move`：

```cpp
void sort_moves(MoveList& moves, const Board& board, int depth) {
    // 为每个走法计算分数
    for (Move& move : moves) {
        move.score = score_move(move, board, depth, current_move_count);
    }
    
    // 按分数降序排序
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.score > b.score;
    });
}
```

#### 1.5 score_move方法增强

在`score_move`方法中添加了中局着法排序增强逻辑（该方法已经有`move_count`参数）。

#### 1.6 主程序调用修改

修改主程序中的`search`调用，传递`game_state.get_move_count()`：

```cpp
best_move = search_engine.search(game_state.get_mutable_board(), allocated_time, game_state.get_move_count());
```

### 2. 实现的功能

#### 2.1 游戏阶段检测
```cpp
Evaluator::GamePhase phase = Evaluator::get_game_phase(board, move_count);
if (phase == Evaluator::MIDGAME) {
    // 中局增强逻辑
}
```

使用`Evaluator::get_game_phase`方法检测当前游戏阶段：
- **开局** (OPENING): 前15步
- **中局** (MIDGAME): 16-40步且棋子数>8
- **残局** (ENDGAME): 棋子数<=8

#### 2.2 中心控制走法奖励
```cpp
// 中心控制走法奖励（格子21,22,26,27）
if (move.to == 21 || move.to == 22 || move.to == 26 || move.to == 27) {
    score += 50;  // 中心控制奖励
}
```

在中局阶段，移动到中心关键格子（21, 22, 26, 27）的走法获得**+50分**额外奖励。这些格子是棋盘的战略中心，控制这些位置可以：
- 增强棋子的机动性
- 限制对手的移动空间
- 为后续战术创造机会

#### 2.3 王的活跃度奖励
```cpp
// 王的活跃度奖励
if (is_king) {
    // 计算from和to到中心的距离
    int from_row = move.from / 5;
    int from_col = (move.from % 5) * 2 + (from_row % 2);
    int to_row = move.to / 5;
    int to_col = (move.to % 5) * 2 + (to_row % 2);
    
    // 计算到中心(4,4)的曼哈顿距离
    int from_center_dist = abs(from_row - 4) + abs(from_col - 4);
    int to_center_dist = abs(to_row - 4) + abs(to_col - 4);
    
    // 如果王向中心移动（距离减少），给予奖励
    if (to_center_dist < from_center_dist) {
        score += 30;  // 王活跃度奖励
    }
}
```

在中局阶段，王棋向中心移动的走法获得**+30分**额外奖励。这鼓励AI：
- 将王棋移动到更有影响力的中心位置
- 提高王棋的活跃度和控制范围
- 在中局阶段发挥王棋的最大价值

### 3. 实现要点

1. **不破坏现有优先级**: 中局增强在基础优先级系统之上添加，不影响：
   - 置换表最佳着法 (10000分)
   - 强制吃子 (1000+MVV-LVA分数)
   - 产生王棋 (900分)
   - 杀手走法 (500分)
   - 历史启发 (0-100分)

2. **仅在中局生效**: 增强逻辑仅在`phase == Evaluator::MIDGAME`时执行，不影响开局和残局的着法排序。

3. **合理的奖励分数**:
   - 中心控制奖励: +50分（高于历史启发，低于杀手走法）
   - 王活跃度奖励: +30分（鼓励但不过度优先）

## 代码位置

**文件**: `boyi/boyi.cpp`

**修改的类和方法**:
1. `SearchEngine`类 - 添加`current_move_count`成员变量
2. `SearchEngine::SearchEngine` - 构造函数初始化
3. `SearchEngine::search` - 新增重载方法
4. `SearchEngine::sort_moves` - 传递`move_count`参数
5. `SearchEngine::score_move` - 中局增强逻辑（约1817-1950行）
6. 主程序 - 调用`search`时传递`move_count`（约2905行）

## 关键改进

### 为什么需要传递move_count？

原始实现中，`score_move`方法有`move_count`参数但有默认值0。这导致：
- 如果不传递`move_count`，它会被当作开局阶段（0 <= 15）
- 中局增强逻辑永远不会生效

### 解决方案

1. **在SearchEngine中跟踪move_count**: 添加`current_move_count`成员变量
2. **提供带move_count的search接口**: 允许调用者传递当前走法数
3. **在sort_moves中使用move_count**: 确保`score_move`接收正确的走法计数
4. **在主程序中传递move_count**: 使用`game_state.get_move_count()`获取实际走法数

这样，中局增强逻辑才能在正确的游戏阶段生效。

## 测试验证

创建了独立的测试文件来验证实现：

### 测试文件
- `test-midgame-move-ordering.cpp`: 独立测试程序
- `test-task-2.2.bat`: 编译和运行测试的批处理脚本

### 测试用例

1. **游戏阶段检测测试**
   - 验证开局、中局、残局的正确识别

2. **中心控制奖励测试**
   - 验证移动到格子21, 22, 26, 27获得额外分数
   - 验证中心格子分数高于非中心格子

3. **王活跃度奖励测试**
   - 验证王向中心移动获得额外分数
   - 验证王向边缘移动不获得奖励

4. **中局专属测试**
   - 验证增强仅在中局阶段生效
   - 验证开局和残局不受影响

### 运行测试

```bash
# 编译并运行测试
test-task-2.2.bat
```

## 需求验证

### 需求2.7验证

**需求**: WHEN Game_Phase为中局，THE Move_Ordering SHALL额外奖励Center_Control走法和King_Activity走法

**验证**:
- ✅ 使用`Evaluator::get_game_phase`检测游戏阶段
- ✅ 在中局阶段为中心控制走法（格子21,22,26,27）增加+50分
- ✅ 在中局阶段为王向中心移动的走法增加+30分
- ✅ 增强逻辑不破坏现有优先级系统
- ✅ 增强逻辑仅在中局阶段生效

## 预期效果

实现中局着法排序增强后，AI在中局阶段将：

1. **更积极地控制中心**: 优先考虑移动到关键中心格子的走法
2. **提高王的活跃度**: 鼓励王棋向中心移动，增强控制力
3. **改善战术决策**: 通过更好的着法排序，更早触发Alpha-Beta剪枝
4. **提升搜索效率**: 减少搜索节点数，提高搜索深度

## 后续任务

根据任务列表，下一个任务是：

**任务2.3**: 更新历史启发表和杀手走法
- 在alpha_beta方法中，当发生beta剪枝且走法为安静走法时更新
- 更新KillerMoves（存储当前深度的剪枝走法）
- 更新HistoryTable（增加depth * depth分数）

## 注意事项

1. **编译器要求**: 代码使用C++17标准，需要支持的编译器（MSVC、GCC、Clang）
2. **代码诊断**: 使用`getDiagnostics`工具验证，无语法错误
3. **测试独立性**: 测试文件独立于主程序，可单独编译运行
4. **性能影响**: 中局增强逻辑简单高效，对性能影响极小

## 总结

任务2.2已成功实现，为中局着法排序添加了战术增强。实现遵循了需求2.7的所有验收标准，并在现有优先级系统之上添加了合理的奖励机制。通过独立测试验证了功能的正确性。
