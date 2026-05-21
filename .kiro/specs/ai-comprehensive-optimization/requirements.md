# 需求文档：国际跳棋AI深度优化

## 简介

本文档定义了国际跳棋AI引擎的深度优化需求。当前AI已实现基础功能（Alpha-Beta搜索、置换表、迭代加深、静态搜索、历史启发、杀手启发、开局库、残局库），但在竞赛中需要更强的战术能力和搜索效率。本次优化聚焦于：完善静态搜索以解决水平线效应、优化着法排序提升剪枝效率、改进位置评估函数、实现空步裁剪、增强底线保护和边缘安全评估、添加阵型评估、扩充开局库和残局库数据。

## 术语表

- **AI_Engine**: 国际跳棋AI搜索引擎（SearchEngine类）
- **Evaluator**: 局面评估器（Evaluator类）
- **Move_Generator**: 着法生成器（MoveGenerator类）
- **Quiescence_Search**: 静态搜索模块（quiescence_search方法）
- **Move_Ordering**: 着法排序模块（sort_moves和score_move方法）
- **History_Table**: 历史启发表（HistoryTable类）
- **Killer_Moves**: 杀手启发表（KillerMoves类）
- **Transposition_Table**: 置换表（TranspositionTable类）
- **Opening_Book**: 开局库（OpeningBook类）
- **Endgame_Database**: 残局库（EndgameDatabase类）
- **Piece_Square_Table**: 位置价值表（position_value数组）
- **Null_Move_Pruning**: 空步裁剪算法
- **Horizon_Effect**: 水平线效应（搜索深度限制导致的战术盲点）
- **Alpha_Beta_Search**: Alpha-Beta剪枝搜索（alpha_beta方法）
- **Capture_Move**: 吃子走法（num_captures > 0的Move）
- **Quiet_Move**: 非吃子走法（num_captures == 0的Move）
- **Game_Phase**: 游戏阶段（开局、中局、残局）
- **Center_Control**: 中心控制（控制关键中心格子21,22,26,27）
- **Back_Row**: 底线（黑方第1行0-4，白方第10行45-49）
- **Edge_Square**: 边缘格子（列0或列9的格子）
- **Formation**: 阵型（棋子的连结性和兵链结构）
- **Promotion_Threat**: 升王威胁（距离升王线2步内的兵）
- **King_Activity**: 王的活跃度（王在中心区域的控制力）

## 需求

### 需求1：完善静态搜索以解决水平线效应

**用户故事：** 作为AI引擎，我需要完善静态搜索，以便在搜索深度截止时避免水平线效应，从而做出更准确的战术决策。

#### 验收标准

1. WHEN Alpha_Beta_Search达到深度0，THE Quiescence_Search SHALL继续搜索所有Capture_Move直到局面静止
2. WHEN Quiescence_Search评估局面，THE Quiescence_Search SHALL使用stand_pat分数作为下界
3. IF stand_pat分数 >= beta，THEN THE Quiescence_Search SHALL立即返回beta（beta剪枝）
4. WHEN Quiescence_Search生成Capture_Move，THE Quiescence_Search SHALL按MVV-LVA顺序排序（Most Valuable Victim - Least Valuable Attacker）
5. WHEN Quiescence_Search递归深度超过10层，THE Quiescence_Search SHALL终止搜索并返回当前评估
6. WHEN Quiescence_Search检测到重复局面，THE Quiescence_Search SHALL返回和棋分数0
7. FOR ALL 包含连续吃子序列的测试局面，解析后再打印后再解析 SHALL 产生等价的局面（round-trip property）

### 需求2：优化着法排序以提升剪枝效率

**用户故事：** 作为AI引擎，我需要优化着法排序，以便更早触发Alpha-Beta剪枝，从而减少搜索节点数并提升搜索深度。

#### 验收标准

1. WHEN Move_Ordering排序着法，THE Move_Ordering SHALL按以下优先级排序：Transposition_Table最佳着法 > 强制Capture_Move > 产生王棋的走法 > Killer_Moves > History_Table高分走法 > Quiet_Move
2. WHEN Move_Ordering评估Capture_Move，THE Move_Ordering SHALL使用MVV-LVA启发（吃王 > 吃兵）
3. WHEN Move_Ordering评估Killer_Moves，THE Move_Ordering SHALL检查当前深度的两个杀手走法
4. WHEN Move_Ordering评估History_Table走法，THE Move_Ordering SHALL使用History_Table.get(from, to)分数
5. WHEN Alpha_Beta_Search发生beta剪枝且走法为Quiet_Move，THE Alpha_Beta_Search SHALL更新Killer_Moves和History_Table
6. WHEN History_Table更新走法分数，THE History_Table SHALL增加depth * depth分数（深度越大奖励越高）
7. WHEN Game_Phase为中局，THE Move_Ordering SHALL额外奖励Center_Control走法和King_Activity走法

### 需求3：改进位置评估函数

**用户故事：** 作为Evaluator，我需要改进位置评估，以便更准确地评估棋子位置价值，从而引导AI占据有利位置。

#### 验收标准

1. THE Evaluator SHALL维护一个50元素的Piece_Square_Table，存储每个格子的基础位置价值
2. WHEN Evaluator初始化Piece_Square_Table，THE Evaluator SHALL根据格子到中心的距离计算价值（中心价值高，边缘价值低）
3. WHEN Evaluator评估普通兵位置，THE Evaluator SHALL使用Piece_Square_Table基础值加上前进程度奖励
4. WHEN Evaluator评估王棋位置，THE Evaluator SHALL使用Piece_Square_Table基础值的2倍（王在中心更有价值）
5. WHEN 普通兵距离升王线 <= 2步，THE Evaluator SHALL增加20分升王威胁奖励
6. WHEN 普通兵距离升王线 <= 4步，THE Evaluator SHALL增加10分接近升王奖励
7. WHEN Edge_Square包含己方棋子，THE Evaluator SHALL减少5分边缘惩罚

### 需求4：实现空步裁剪算法

**用户故事：** 作为AI引擎，我需要实现空步裁剪，以便在优势局面下快速验证beta剪枝，从而提升搜索效率。

#### 验收标准

1. WHEN Alpha_Beta_Search深度 >= 3且不在Quiescence_Search中，THE Alpha_Beta_Search SHALL尝试Null_Move_Pruning
2. WHEN Alpha_Beta_Search尝试Null_Move_Pruning，THE Alpha_Beta_Search SHALL跳过当前回合并以reduced_depth = depth - 3搜索
3. IF Null_Move_Pruning搜索结果 >= beta，THEN THE Alpha_Beta_Search SHALL返回beta（空步剪枝成功）
4. WHEN 局面处于Zugzwang状态（只有王棋且王棋数 <= 3），THE Alpha_Beta_Search SHALL禁用Null_Move_Pruning
5. WHEN 上一步走法为空步，THE Alpha_Beta_Search SHALL禁用Null_Move_Pruning（防止连续空步）
6. WHEN Game_Phase为残局且总棋子数 <= 6，THE Alpha_Beta_Search SHALL禁用Null_Move_Pruning

### 需求5：增强底线保护和边缘安全评估

**用户故事：** 作为Evaluator，我需要评估底线保护和边缘安全，以便防止对手突破底线升王，从而提升防守能力。

#### 验收标准

1. WHEN Evaluator评估黑方局面，THE Evaluator SHALL检查Back_Row（格子0-4）是否有黑方棋子保护
2. WHEN Evaluator评估白方局面，THE Evaluator SHALL检查Back_Row（格子45-49）是否有白方棋子保护
3. WHEN Back_Row格子包含己方棋子，THE Evaluator SHALL增加8分底线保护奖励
4. WHEN Back_Row格子为空且对手有Promotion_Threat，THE Evaluator SHALL减少15分底线暴露惩罚
5. WHEN Edge_Square包含己方棋子且相邻格子有己方棋子，THE Evaluator SHALL增加6分边缘安全奖励
6. WHEN Edge_Square包含己方棋子且相邻格子有对手棋子，THE Evaluator SHALL减少10分边缘威胁惩罚

### 需求6：添加阵型评估

**用户故事：** 作为Evaluator，我需要评估阵型质量，以便奖励连结性强的阵型和惩罚孤立棋子，从而提升整体阵型强度。

#### 验收标准

1. WHEN Evaluator评估Formation，THE Evaluator SHALL检查每个己方棋子的4个对角相邻格子
2. WHEN 棋子有 >= 2个对角相邻己方棋子，THE Evaluator SHALL增加10分连结奖励
3. WHEN 棋子有0个对角相邻己方棋子，THE Evaluator SHALL减少15分孤立惩罚
4. WHEN 3个或更多己方兵形成对角线兵链，THE Evaluator SHALL增加25分兵链奖励
5. WHEN 两个己方王在相邻对角线（距离4或6），THE Evaluator SHALL增加30分双王配合奖励
6. WHEN 己方棋子包围对手棋子（对手棋子4个方向中有 >= 3个己方棋子），THE Evaluator SHALL增加20分包围奖励

### 需求7：扩充开局库数据

**用户故事：** 作为Opening_Book，我需要扩充开局库数据，以便在开局阶段快速选择经过验证的走法，从而节省搜索时间并避免开局陷阱。

#### 验收标准

1. THE Opening_Book SHALL支持从文件加载开局走法序列（格式：每行一个走法序列，如"6-11 31-26 11-16 26-21"）
2. WHEN Opening_Book加载文件，THE Opening_Book SHALL解析每行走法序列并添加到开局库
3. WHEN Opening_Book添加走法序列，THE Opening_Book SHALL为序列中每个局面记录下一步走法
4. WHEN Opening_Book查询局面，THE Opening_Book SHALL使用Zobrist哈希值匹配局面
5. WHEN Opening_Book查询命中且有多个候选走法，THE Opening_Book SHALL使用加权随机选择走法
6. THE Opening_Book SHALL包含至少20条不同的开局走法序列（每条序列至少6步）
7. WHEN Opening_Book初始化，THE Opening_Book SHALL包含标准开局走法：6-11（权重100）、11-16（权重80）、7-12（权重60）

### 需求8：扩充残局库数据

**用户故事：** 作为Endgame_Database，我需要扩充残局库数据，以便在残局阶段直接查询最佳走法，从而避免搜索错误并加速残局处理。

#### 验收标准

1. THE Endgame_Database SHALL支持从二进制文件加载残局数据（格式：哈希值8字节 + from 4字节 + to 4字节 + distance 4字节）
2. WHEN Endgame_Database查询局面，THE Endgame_Database SHALL使用Zobrist哈希值匹配局面
3. WHEN Endgame_Database查询命中，THE Endgame_Database SHALL返回最佳走法和距离胜利的步数
4. WHEN 总棋子数 <= 6，THE Endgame_Database SHALL允许查询残局库
5. WHEN 总棋子数 > 6，THE Endgame_Database SHALL跳过残局库查询
6. THE Endgame_Database SHALL包含至少100个基础残局局面（王对兵、双王对单王等）
7. WHEN Endgame_Database生成残局库，THE Endgame_Database SHALL使用回溯搜索计算每个局面的最佳走法和距离

### 需求9：集成所有优化到搜索引擎

**用户故事：** 作为AI引擎，我需要集成所有优化模块，以便在实际对局中综合运用所有优化技术，从而达到最佳性能。

#### 验收标准

1. WHEN AI_Engine搜索最佳走法，THE AI_Engine SHALL按顺序尝试：Endgame_Database查询 > Opening_Book查询 > Alpha_Beta_Search
2. WHEN Alpha_Beta_Search执行，THE Alpha_Beta_Search SHALL使用优化后的Move_Ordering
3. WHEN Alpha_Beta_Search执行，THE Alpha_Beta_Search SHALL在适当条件下使用Null_Move_Pruning
4. WHEN Alpha_Beta_Search达到深度0，THE Alpha_Beta_Search SHALL调用完善后的Quiescence_Search
5. WHEN Alpha_Beta_Search评估局面，THE Alpha_Beta_Search SHALL使用改进后的Evaluator（包含Piece_Square_Table、底线保护、边缘安全、Formation评估）
6. WHEN Game_Phase为中局，THE AI_Engine SHALL在Move_Ordering中增加中局战术走法的权重
7. WHEN AI_Engine完成搜索，THE AI_Engine SHALL输出搜索深度、节点数、置换表命中率、开局库命中率统计信息

### 需求10：性能验证和基准测试

**用户故事：** 作为开发者，我需要验证优化效果，以便确认优化确实提升了AI性能，从而保证优化的有效性。

#### 验收标准

1. THE AI_Engine SHALL在标准测试局面集上搜索深度比优化前提升至少20%
2. THE AI_Engine SHALL在标准测试局面集上搜索节点数比优化前减少至少30%
3. THE Transposition_Table SHALL在中局阶段命中率 >= 40%
4. THE Opening_Book SHALL在前15步命中率 >= 60%
5. WHEN AI_Engine与优化前版本对弈100局，THE AI_Engine SHALL胜率 >= 65%
6. WHEN AI_Engine在中局劣势局面（评估分数 < -50）搜索，THE AI_Engine SHALL找到防守走法使评估分数提升至少30分
7. THE AI_Engine SHALL在5分钟时限内完成至少50步走法且不超时

## 优先级说明

根据用户提供的优先级建议，需求的实现顺序为：

**高优先级（必须实现）：**
- 需求1：完善静态搜索（解决水平线效应）
- 需求2：优化着法排序（历史启发、杀手启发）
- 需求3：改进位置评估（Piece-Square Tables）

**中优先级（重要但可延后）：**
- 需求4：空步裁剪
- 需求5：底线保护、边缘安全评估
- 需求6：阵型评估

**低优先级（可选）：**
- 需求7：开局库数据扩充
- 需求8：残局库数据扩充

**集成和验证（最后阶段）：**
- 需求9：集成所有优化
- 需求10：性能验证和基准测试

## 技术约束

1. 实现语言：C++17
2. 现有架构：基于位棋盘（Bitboard）的Alpha-Beta搜索引擎
3. 现有组件：置换表、迭代加深、历史启发表、杀手启发表、开局库、残局库
4. 性能要求：5分钟时限内完成至少50步走法
5. 内存限制：置换表128MB，开局库和残局库合计不超过100MB

## 验证方法

1. **单元测试**：为每个新增或修改的模块编写单元测试
2. **集成测试**：测试各模块协同工作的正确性
3. **性能基准测试**：对比优化前后的搜索深度、节点数、时间消耗
4. **对弈测试**：与优化前版本对弈100局，统计胜率
5. **战术测试**：在特定战术局面（如中局劣势、残局精确计算）测试AI表现

## 附录：EARS模式和INCOSE质量规则说明

本需求文档遵循EARS（Easy Approach to Requirements Syntax）模式和INCOSE质量规则：

**EARS模式：**
- **Ubiquitous（普遍）**：THE <system> SHALL <response>
- **Event-driven（事件驱动）**：WHEN <trigger>, THE <system> SHALL <response>
- **Unwanted event（非期望事件）**：IF <condition>, THEN THE <system> SHALL <response>

**INCOSE质量规则：**
- 使用主动语态和明确的系统名称
- 避免模糊术语（如"快速"、"合理"）
- 使用可测试的标准（具体数值、可验证条件）
- 每个需求测试一件事
- 避免逃避条款（如"如果可能"、"在适当情况下"）
- 使用肯定陈述（说明系统应该做什么，而非不应该做什么）
