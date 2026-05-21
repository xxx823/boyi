# Bugfix需求文档：国际跳棋AI中局劣势修复

## 简介

本文档定义了修复国际跳棋AI程序在中局阶段（27-28步后）陷入劣势的bug。该bug导致AI在对局进行到中局时开始失去优势，最终导致失败。通过分析现有实现，问题可能出现在评估函数权重、搜索深度分配或中局战术理解上。

**Bug影响：**
- AI在开局阶段表现正常
- 在27-28步后开始陷入劣势
- 走法生成和执行功能正常
- 评估函数可能未充分考虑中局特征

**技术栈：** C++17, Alpha-Beta搜索, 位棋盘（Bitboard）

## Bug分析

### 1. 当前行为（缺陷）

**Bug条件C(X)定义：**

```pascal
FUNCTION isBugCondition(X)
  INPUT: X of type GamePosition
  OUTPUT: boolean
  
  // X包含：棋盘状态、走法数、双方棋子分布
  move_count ← X.move_count
  phase ← determine_game_phase(X)
  
  // Bug条件：中局阶段（27-28步后）
  RETURN (move_count >= 27 AND phase = MIDGAME)
END FUNCTION

FUNCTION determine_game_phase(X)
  INPUT: X of type GamePosition
  OUTPUT: GamePhase (OPENING, MIDGAME, ENDGAME)
  
  total_pieces ← count_all_pieces(X.board)
  move_count ← X.move_count
  
  IF move_count <= 15 THEN
    RETURN OPENING
  ELSE IF total_pieces <= 8 THEN
    RETURN ENDGAME
  ELSE
    RETURN MIDGAME
  END IF
END FUNCTION
```

**当前缺陷行为：**

1.1 WHEN 对局进入中局阶段（27-28步后）THEN AI的评估函数未能准确评估中局局面的战术价值

1.2 WHEN 中局阶段需要进行战术交换时 THEN AI的评估函数过于保守，错失主动权

1.3 WHEN 中局阶段需要控制关键格子时 THEN AI未能识别和占据战略要地

1.4 WHEN 中局阶段对手形成攻击态势时 THEN AI的防御评估不足，未能及时应对威胁

1.5 WHEN 中局阶段需要协调多个棋子时 THEN AI的位置评估未充分考虑棋子间的配合

1.6 WHEN 搜索深度在中局阶段不足时 THEN AI无法看到足够远的战术组合

### 2. 预期行为（正确）

**修复属性P(result)定义：**

```pascal
// Property: Fix Checking - 中局阶段AI决策质量
FOR ALL X WHERE isBugCondition(X) DO
  move ← AI_search'(X)  // 修复后的AI
  result ← evaluate_move_quality(X, move)
  
  ASSERT result.maintains_advantage OR result.improves_position
  ASSERT result.tactical_awareness >= THRESHOLD_MIDGAME
  ASSERT result.strategic_control >= THRESHOLD_MIDGAME
END FOR

FUNCTION evaluate_move_quality(position, move)
  INPUT: position (GamePosition), move (Move)
  OUTPUT: MoveQuality
  
  quality ← new MoveQuality()
  
  // 评估走法是否保持或改善局面
  score_before ← evaluate(position)
  position_after ← apply_move(position, move)
  score_after ← evaluate(position_after)
  
  quality.maintains_advantage ← (score_after >= score_before - TOLERANCE)
  quality.improves_position ← (score_after > score_before)
  
  // 评估战术意识（是否识别吃子机会、威胁等）
  quality.tactical_awareness ← assess_tactical_features(position_after)
  
  // 评估战略控制（中心控制、王的活跃度等）
  quality.strategic_control ← assess_strategic_features(position_after)
  
  RETURN quality
END FUNCTION
```

**预期正确行为：**

2.1 WHEN 对局进入中局阶段（27-28步后）THEN AI的评估函数SHALL准确评估中局局面，包括战术机会和战略位置

2.2 WHEN 中局阶段存在战术交换机会时 THEN AI SHALL正确评估交换后的局面优劣，选择有利的交换

2.3 WHEN 中局阶段需要控制关键格子时 THEN AI SHALL识别并占据战略要地（中心格子22,23,27,28）

2.4 WHEN 中局阶段对手形成攻击态势时 THEN AI SHALL及时识别威胁并采取防御措施

2.5 WHEN 中局阶段需要协调多个棋子时 THEN AI SHALL评估棋子间的配合和连续性，避免孤立棋子

2.6 WHEN 中局阶段进行搜索时 THEN AI SHALL使用足够的搜索深度（至少6-8层）以看到战术组合

2.7 WHEN 中局阶段评估王棋时 THEN AI SHALL充分考虑王棋的活跃度和控制力

2.8 WHEN 中局阶段面临升王机会时 THEN AI SHALL正确评估升王的战略价值

### 3. 不变行为（回归预防）

**保持属性（Preservation Checking）：**

```pascal
// Property: Preservation Checking
FOR ALL X WHERE NOT isBugCondition(X) DO
  move_original ← AI_search(X)   // 原始AI
  move_fixed ← AI_search'(X)     // 修复后的AI
  
  // 在非中局阶段，行为应保持一致或更好
  quality_original ← evaluate_move_quality(X, move_original)
  quality_fixed ← evaluate_move_quality(X, move_fixed)
  
  ASSERT quality_fixed >= quality_original
END FOR
```

**必须保持的行为：**

3.1 WHEN 对局处于开局阶段（前15步）THEN AI SHALL CONTINUE TO使用开局库或标准开局走法

3.2 WHEN 对局处于残局阶段（8子或更少）THEN AI SHALL CONTINUE TO使用残局库或精确计算

3.3 WHEN 存在强制吃子机会时 THEN AI SHALL CONTINUE TO正确识别并执行最大吃子规则

3.4 WHEN 走法生成和执行时 THEN AI SHALL CONTINUE TO遵守国际跳棋规则（升王、连续跳吃等）

3.5 WHEN 评估终局状态时 THEN AI SHALL CONTINUE TO正确判定胜负和和棋

3.6 WHEN 时间管理时 THEN AI SHALL CONTINUE TO合理分配思考时间，避免超时

3.7 WHEN 使用置换表时 THEN AI SHALL CONTINUE TO正确缓存和查询已搜索局面

3.8 WHEN 进行Alpha-Beta剪枝时 THEN AI SHALL CONTINUE TO保持搜索的正确性

## Bug根本原因分析

基于代码审查（boyi/boyi.cpp），识别出以下潜在根本原因：

### 原因1：评估函数权重不适合中局

**当前实现：**
```cpp
// Evaluator类中的权重常量
static constexpr int MAN_VALUE = 100;
static constexpr int KING_VALUE = 350;
static constexpr int CENTER_BONUS = 15;
static constexpr int MOBILITY_WEIGHT = 8;
static constexpr int SAFETY_BONUS = 12;
```

**问题：**
- 权重是静态的，未根据游戏阶段调整
- 中局阶段的战术特征（如王的活跃度、升王威胁）权重可能不足
- 位置评估可能过于简单，未充分考虑中局的复杂性

### 原因2：搜索深度分配不合理

**当前实现：**
```cpp
// TimeManager::get_time_factor()
if (move_number <= 15) {
    return 0.8;  // 开局：80%时间
} else if (move_number <= 40) {
    return 1.0;  // 中局：100%时间
} else {
    return 1.2;  // 残局：120%时间
}
```

**问题：**
- 中局阶段（27-28步）使用标准时间因子1.0
- 可能需要在关键的中局阶段（25-35步）增加搜索深度
- 迭代加深可能在中局复杂局面下深度不足

### 原因3：中局战术评估不足

**当前实现：**
```cpp
// Evaluator::evaluate_advanced()包含：
// - evaluate_material()
// - evaluate_position()
// - evaluate_mobility()
// - evaluate_safety()
// - evaluate_structure()
// - evaluate_key_squares()
// - evaluate_promotion_threat()
// - evaluate_king_activity()
```

**问题：**
- 缺少中局特定的战术评估（如双王配合、包围战术）
- `evaluate_key_squares()`可能权重不足
- `evaluate_king_activity()`可能未充分考虑中局王的进攻性

### 原因4：走法排序在中局效率低

**当前实现：**
```cpp
// SearchEngine::score_move()
// 1. 置换表走法：10000
// 2. 吃子走法：1000 + num_captures * 100
// 3. 杀手走法：500
// 4. 历史启发：history.get(from, to)
// 5. 位置启发：position_value差值
```

**问题：**
- 中局阶段的战术走法（如控制中心、形成攻击）可能排序靠后
- 历史启发可能未充分学习中局模式

## 修复策略

### 策略1：引入游戏阶段自适应评估

**实现方案：**
1. 在`Evaluator`类中添加`get_game_phase()`方法
2. 根据游戏阶段动态调整评估权重
3. 中局阶段增加以下权重：
   - `KING_VALUE`: 350 → 400（中局王更重要）
   - `CENTER_BONUS`: 15 → 25（中局中心控制更关键）
   - `MOBILITY_WEIGHT`: 8 → 12（中局机动性更重要）
   - `evaluate_king_activity()`权重：20 → 35

**预期效果：**
- AI在中局阶段更重视王的活跃度和中心控制
- 评估分数更准确反映中局局面优劣

### 策略2：增加中局搜索深度

**实现方案：**
1. 修改`TimeManager::get_time_factor()`
2. 在关键中局阶段（25-35步）增加时间因子：
   ```cpp
   if (move_number >= 25 && move_number <= 35) {
       return 1.3;  // 中局关键阶段：130%时间
   }
   ```
3. 在`SearchEngine::iterative_deepening()`中，中局阶段目标深度+1

**预期效果：**
- 中局阶段搜索更深，能看到更远的战术组合
- 减少中局阶段的战术失误

### 策略3：增强中局战术评估

**实现方案：**
1. 在`Evaluator`类中添加`evaluate_midgame_tactics()`方法
2. 评估以下中局特征：
   - 双王配合（两个王在相邻对角线）：+30分
   - 包围对手棋子（形成夹击）：+20分
   - 控制对手升王路线：+15分
   - 形成连续攻击态势：+25分
3. 在`evaluate_advanced()`中，中局阶段调用此方法

**预期效果：**
- AI能识别和执行中局战术
- 提高中局阶段的战术意识

### 策略4：改进中局走法排序

**实现方案：**
1. 在`SearchEngine::score_move()`中添加中局启发
2. 中局阶段，以下走法优先级提升：
   - 占据中心格子（22,23,27,28）：+300分
   - 王向中心移动：+200分
   - 形成双王配合：+250分
   - 威胁对手升王路线：+150分

**预期效果：**
- 中局阶段优先搜索战术走法
- 提高搜索效率和走法质量

## 验证方法

### 验证1：自我对弈测试

**方法：**
1. 修复前的AI vs 修复后的AI
2. 进行100局对弈
3. 统计中局阶段（27-40步）的胜率和局面评估

**成功标准：**
- 修复后的AI在中局阶段胜率 >= 60%
- 修复后的AI在27-28步后的平均评估分数 > 修复前

### 验证2：中局局面测试集

**方法：**
1. 准备10个标准中局局面（27-30步）
2. 让修复前后的AI分别搜索最佳走法
3. 对比走法质量和评估分数

**成功标准：**
- 修复后的AI在至少8/10个局面中选择更优走法
- 修复后的AI的评估分数更接近理论最优值

### 验证3：回归测试

**方法：**
1. 运行现有的所有单元测试和集成测试
2. 验证开局和残局阶段性能未下降
3. 验证规则遵守和时间管理未受影响

**成功标准：**
- 所有现有测试通过
- 开局和残局阶段胜率保持不变或提升
- 无新的规则违规或超时问题

## 实现优先级

1. **高优先级**：策略1（自适应评估）和策略2（增加搜索深度）
   - 这两个策略实现简单，影响大
   - 预计能解决70%的中局劣势问题

2. **中优先级**：策略3（中局战术评估）
   - 需要更多的领域知识和调试
   - 预计能进一步提升20%的中局表现

3. **低优先级**：策略4（改进走法排序）
   - 优化性质，非根本性修复
   - 预计能提升10%的搜索效率

## 风险和注意事项

1. **过度拟合风险**：针对27-28步的优化可能影响其他阶段
   - 缓解措施：使用渐进式权重调整，而非硬编码阈值

2. **搜索时间增加**：增加中局搜索深度可能导致超时
   - 缓解措施：动态调整，监控实际用时

3. **评估函数复杂度**：增加评估项可能降低评估速度
   - 缓解措施：优化计算，使用位运算加速

4. **回归风险**：修改评估函数可能影响开局和残局
   - 缓解措施：充分的回归测试，分阶段部署

## 参考资料

- 现有代码：`boyi/boyi.cpp`（Evaluator类，SearchEngine类）
- 需求文档：`.kiro/specs/international-checkers-ai/requirements.md`
- 任务列表：`.kiro/specs/international-checkers-ai/tasks.md`
- 国际跳棋战术理论：中局阶段的关键是中心控制和王的活跃度
