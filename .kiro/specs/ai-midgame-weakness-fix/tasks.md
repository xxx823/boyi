# 实现计划

- [ ] 1. 编写bug条件探索测试（在未修复代码上验证bug）
  - **Property 1: Bug Condition** - 中局阶段AI决策质量下降
  - **关键说明**：此测试必须在未修复代码上失败 - 失败确认bug存在
  - **不要在测试失败时尝试修复测试或代码**
  - **注意**：此测试编码了预期行为 - 它将在实现修复后通过时验证修复
  - **目标**：暴露反例以演示bug存在
  - **作用域PBT方法**：对于确定性bug，将属性作用域限定为具体失败案例以确保可重现性
  - 测试实现细节来自设计文档中的Bug Condition
  - 测试断言应匹配设计文档中的Expected Behavior Properties
  - 在未修复代码上运行测试
  - **预期结果**：测试失败（这是正确的 - 它证明bug存在）
  - 记录发现的反例以理解根本原因
  - 当测试编写完成、运行并记录失败时，标记任务完成
  - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8_

- [ ] 2. 编写保持属性测试（在实现修复之前）
  - **Property 2: Preservation** - 非中局阶段行为保持
  - **重要**：遵循观察优先方法
  - 在未修复代码上观察非bug输入（isBugCondition返回false的情况）的行为
  - 编写基于属性的测试，捕获来自Preservation Requirements的观察行为模式
  - 基于属性的测试生成许多测试用例以提供更强保证
  - 在未修复代码上运行测试
  - **预期结果**：测试通过（这确认了要保持的基线行为）
  - 当测试编写完成、运行并在未修复代码上通过时，标记任务完成
  - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8_

- [ ] 3. 修复中局劣势问题

  - [x] 3.1 在Evaluator类中添加游戏阶段检测
    - 在Evaluator类中添加`enum GamePhase { OPENING, MIDGAME, ENDGAME }`枚举
    - 添加`static GamePhase get_game_phase(const Board& board, int move_count)`方法
    - 根据走法数和棋子数判断游戏阶段：
      - OPENING: move_count <= 15
      - ENDGAME: total_pieces <= 8
      - MIDGAME: 其他情况
    - _Bug_Condition: isBugCondition(input) where input.move_count >= 27 AND input.game_phase = MIDGAME_
    - _Expected_Behavior: AI在中局阶段保持战术意识和战略控制（来自设计文档）_
    - _Preservation: 开局和残局阶段行为保持不变（来自设计文档）_
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 3.1, 3.2_

  - [x] 3.2 修改Evaluator类的评估权重以适应中局
    - 在`evaluate_advanced()`中调用`get_game_phase()`
    - 根据游戏阶段动态调整权重：
      - 中局阶段：KING_VALUE 350→400
      - 中局阶段：CENTER_BONUS 15→25
      - 中局阶段：MOBILITY_WEIGHT 8→12
      - 中局阶段：`evaluate_king_activity()`的返回值乘以1.75倍
      - 中局阶段：`evaluate_key_squares()`的返回值乘以1.67倍
    - _Bug_Condition: isBugCondition(input) where input.move_count >= 27 AND input.game_phase = MIDGAME_
    - _Expected_Behavior: 评估函数准确评估中局局面的战术和战略价值_
    - _Preservation: 开局和残局阶段的评估权重保持不变_
    - _Requirements: 2.1, 2.2, 2.7, 3.1, 3.2_

  - [x] 3.3 修改TimeManager类的时间分配以增加中局搜索深度
    - 在`get_time_factor()`中添加中局关键阶段判断
    - 走法数在25-35之间时，返回1.3倍时间因子（而非1.0倍）
    - 确保不超过剩余时间的50%（安全机制）
    - _Bug_Condition: isBugCondition(input) where input.move_count >= 27 AND input.game_phase = MIDGAME_
    - _Expected_Behavior: 中局阶段搜索更深，能看到更远的战术组合_
    - _Preservation: 开局和残局阶段的时间分配保持不变_
    - _Requirements: 2.6, 3.6_

  - [x] 3.4 在Evaluator类中添加中局战术评估
    - 添加`static int evaluate_midgame_tactics(const Board& board)`方法
    - 评估双王配合（两个王在相邻对角线）：+30分
    - 评估包围对手棋子（形成夹击）：+20分
    - 评估控制对手升王路线：+15分
    - 评估形成连续攻击态势：+25分
    - 在`evaluate_advanced()`中，中局阶段调用此方法
    - _Bug_Condition: isBugCondition(input) where input.move_count >= 27 AND input.game_phase = MIDGAME_
    - _Expected_Behavior: AI识别和执行中局战术（双王配合、包围、控制升王路线）_
    - _Preservation: 开局和残局阶段不调用此方法_
    - _Requirements: 2.2, 2.3, 2.4, 2.5, 3.1, 3.2_

  - [x] 3.5 修改SearchEngine类的走法排序以改进中局走法优先级
    - 在`score_move()`中添加move_count参数
    - 中局阶段，以下走法优先级提升：
      - 占据中心格子（21,22,26,27）：+300分
      - 王向中心移动：+200分
      - 形成双王配合：+250分
      - 威胁对手升王路线：+150分
    - _Bug_Condition: isBugCondition(input) where input.move_count >= 27 AND input.game_phase = MIDGAME_
    - _Expected_Behavior: 中局阶段优先搜索战术走法，提高搜索效率和走法质量_
    - _Preservation: 开局和残局阶段的走法排序保持不变_
    - _Requirements: 2.3, 2.4, 2.5, 3.1, 3.2_

  - [ ] 3.6 验证bug条件探索测试现在通过
    - **Property 1: Expected Behavior** - 中局阶段AI决策质量
    - **重要**：重新运行任务1中的相同测试 - 不要编写新测试
    - 任务1中的测试编码了预期行为
    - 当此测试通过时，它确认预期行为得到满足
    - 运行任务1中的bug条件探索测试
    - **预期结果**：测试通过（确认bug已修复）
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8_

  - [ ] 3.7 验证保持测试仍然通过
    - **Property 2: Preservation** - 非中局阶段行为保持
    - **重要**：重新运行任务2中的相同测试 - 不要编写新测试
    - 运行任务2中的保持属性测试
    - **预期结果**：测试通过（确认无回归）
    - 确认修复后所有测试仍然通过（无回归）
    - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8_

- [ ] 4. 检查点 - 确保所有测试通过
  - 确保所有测试通过，如有问题请询问用户。
