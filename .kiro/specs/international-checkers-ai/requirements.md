# 需求文档：国际跳棋AI程序

## 简介

本文档定义了参加2026年辽宁省大学生计算机博弈大赛的国际跳棋（100格）AI程序的功能需求和性能需求。该程序需要实现完整的国际跳棋规则、高性能的AI搜索算法、局面评估系统，并满足比赛平台的接口要求。

**比赛信息：**
- 时间：2026年6月7日
- 地点：沈阳航空航天大学
- 比赛方式：双循环赛，程序自动对弈，定时制
- 提交物：程序设计说明书（PDF）、源代码、PPT答辩材料

**技术栈：**
- 编程语言：C++
- 数据结构：位棋盘（Bitboard）
- 已实现基础：棋盘表示、开局初始化、棋盘打印

## 术语表

- **Checkers_Engine**: 国际跳棋引擎，负责规则执行和AI决策的核心系统
- **Bitboard**: 位棋盘，使用64位整数表示棋盘状态的数据结构
- **Move_Generator**: 走法生成器，生成所有合法走法的模块
- **Search_Algorithm**: 搜索算法，使用Alpha-Beta剪枝的极大极小搜索
- **Evaluation_Function**: 评估函数，评估局面优劣的计算模块
- **Time_Manager**: 时间管理器，控制AI思考时间的模块
- **Competition_Interface**: 比赛接口，与比赛平台通信的输入输出模块
- **Opening_Book**: 开局库，存储常见开局走法的数据库
- **Endgame_Database**: 残局库，存储残局最优解的数据库
- **Man**: 普通棋子（兵），只能向前斜移的棋子
- **King**: 王棋，可沿对角线移动任意格数的棋子
- **Capture**: 跳吃，跳过对方棋子并吃掉的走法
- **Promotion**: 升王，普通棋子到达对方底线升级为王
- **Mandatory_Capture**: 强制吃子规则，有吃子机会时必须吃子
- **Maximum_Capture**: 最大吃子规则，必须选择吃子数量最多的走法
- **Valid_Square**: 有效格子，10×10棋盘中的50个黑色格子（编号1-50）
- **Game_State**: 游戏状态，包括棋盘、当前玩家、历史记录的完整状态
- **Self_Play**: 自我对弈，程序与自己对弈用于测试和训练

## 需求

### 需求 1：走法生成

**用户故事：** 作为AI引擎，我需要生成所有合法走法，以便搜索算法能够探索所有可能的下一步。

#### 验收标准

1. WHEN 当前玩家有普通棋子且无吃子机会时，THE Move_Generator SHALL 生成所有向前斜移一格的合法走法
2. WHEN 当前玩家有王棋且无吃子机会时，THE Move_Generator SHALL 生成所有沿对角线移动任意格数的合法走法
3. WHEN 当前玩家有吃子机会时，THE Move_Generator SHALL 仅生成吃子走法（强制吃子规则）
4. WHEN 一个棋子可以连续跳吃时，THE Move_Generator SHALL 生成完整的连续跳吃序列
5. WHEN 存在多个吃子走法时，THE Move_Generator SHALL 仅生成吃子数量最多的走法（最大吃子规则）
6. THE Move_Generator SHALL 确保生成的走法不会移动到已占据的格子
7. THE Move_Generator SHALL 确保生成的走法仅在50个有效黑格内移动
8. FOR ALL 生成的走法，执行后的棋盘状态 SHALL 符合国际跳棋规则

### 需求 2：升王机制

**用户故事：** 作为AI引擎，我需要正确处理棋子升王，以便遵守国际跳棋规则。

#### 验收标准

1. WHEN 黑方普通棋子移动到第10行（46-50号格子）时，THE Checkers_Engine SHALL 将该棋子从black_men转移到black_kings
2. WHEN 白方普通棋子移动到第1行（1-5号格子）时，THE Checkers_Engine SHALL 将该棋子从white_men转移到white_kings
3. WHEN 棋子在跳吃过程中经过底线但未停留时，THE Checkers_Engine SHALL NOT 升王
4. WHEN 棋子在跳吃过程中停留在底线时，THE Checkers_Engine SHALL 升王并结束该回合
5. WHEN 棋子升王后，THE Checkers_Engine SHALL 在下一回合允许该棋子按王棋规则移动

### 需求 3：Alpha-Beta搜索算法

**用户故事：** 作为AI引擎，我需要高效的搜索算法，以便在有限时间内找到最优走法。

#### 验收标准

1. THE Search_Algorithm SHALL 实现极大极小搜索（Minimax）框架
2. THE Search_Algorithm SHALL 实现Alpha-Beta剪枝优化
3. WHEN 搜索深度达到预设限制时，THE Search_Algorithm SHALL 调用评估函数返回局面分数
4. WHEN 发现必胜或必败局面时，THE Search_Algorithm SHALL 立即返回结果
5. THE Search_Algorithm SHALL 支持迭代加深搜索（Iterative Deepening）
6. WHEN 时间即将耗尽时，THE Search_Algorithm SHALL 返回当前最佳走法
7. THE Search_Algorithm SHALL 实现走法排序以提高剪枝效率
8. FOR ALL 搜索深度d，搜索结果的准确性 SHALL 不低于深度d-1的结果

### 需求 4：局面评估函数

**用户故事：** 作为AI引擎，我需要准确评估局面优劣，以便选择最有利的走法。

#### 验收标准

1. THE Evaluation_Function SHALL 计算双方棋子数量差异（材料优势）
2. THE Evaluation_Function SHALL 对王棋赋予比普通棋子更高的价值
3. THE Evaluation_Function SHALL 评估棋子的位置优势（中心控制、前进程度）
4. THE Evaluation_Function SHALL 评估棋子的机动性（可移动格子数量）
5. THE Evaluation_Function SHALL 评估棋子的安全性（是否容易被吃）
6. WHEN 一方无合法走法时，THE Evaluation_Function SHALL 返回该方失败的最大负分
7. WHEN 一方所有棋子被吃光时，THE Evaluation_Function SHALL 返回该方失败的最大负分
8. THE Evaluation_Function SHALL 在200微秒内完成单次评估
9. FOR ALL 对称局面，评估分数的绝对值 SHALL 相等

### 需求 5：时间管理系统

**用户故事：** 作为AI引擎，我需要合理分配思考时间，以便在比赛中不超时且充分利用时间。

#### 验收标准

1. THE Time_Manager SHALL 跟踪每一步的剩余思考时间
2. WHEN 剩余时间充足时，THE Time_Manager SHALL 允许更深的搜索深度
3. WHEN 剩余时间紧张时，THE Time_Manager SHALL 限制搜索深度以避免超时
4. THE Time_Manager SHALL 为开局、中局、残局分配不同的时间比例
5. WHEN 搜索时间超过分配时间的90%时，THE Time_Manager SHALL 通知搜索算法停止
6. THE Time_Manager SHALL 预留至少100毫秒的安全缓冲时间
7. THE Time_Manager SHALL 记录每步实际使用的时间用于调整策略

### 需求 6：比赛接口协议

**用户故事：** 作为比赛参赛程序，我需要与比赛平台正确通信，以便参加自动对弈比赛。

#### 验收标准

1. THE Competition_Interface SHALL 从标准输入读取比赛平台的指令
2. THE Competition_Interface SHALL 将AI决策的走法输出到标准输出
3. WHEN 收到初始化指令时，THE Competition_Interface SHALL 初始化棋盘为开局状态
4. WHEN 收到对手走法时，THE Competition_Interface SHALL 更新棋盘状态
5. WHEN 轮到己方行棋时，THE Competition_Interface SHALL 调用搜索算法并输出走法
6. THE Competition_Interface SHALL 支持标准的走法表示格式（如"15-19"或"15x24x33"）
7. WHEN 收到无效指令时，THE Competition_Interface SHALL 输出错误信息并继续运行
8. THE Competition_Interface SHALL 在程序启动后1秒内完成初始化

### 需求 7：开局库系统

**用户故事：** 作为AI引擎，我需要使用开局库，以便在开局阶段快速走出高质量的走法。

#### 验收标准

1. THE Opening_Book SHALL 存储至少50个常见开局变化
2. WHEN 当前局面在开局库中时，THE Checkers_Engine SHALL 从开局库中选择走法
3. WHEN 开局库中有多个候选走法时，THE Checkers_Engine SHALL 随机选择以增加变化
4. THE Opening_Book SHALL 支持从文本文件加载开局数据
5. WHEN 当前局面不在开局库中时，THE Checkers_Engine SHALL 切换到搜索算法
6. THE Opening_Book SHALL 在程序启动时加载完成，加载时间不超过500毫秒

### 需求 8：残局数据库

**用户故事：** 作为AI引擎，我需要使用残局库，以便在残局阶段走出理论最优解。

#### 验收标准

1. WHERE 残局数据库功能启用时，THE Endgame_Database SHALL 存储棋子总数不超过6个的所有局面
2. WHERE 残局数据库功能启用时，WHEN 当前局面在残局库中时，THE Checkers_Engine SHALL 直接返回最优走法
3. WHERE 残局数据库功能启用时，THE Endgame_Database SHALL 支持从二进制文件加载残局数据
4. WHERE 残局数据库功能启用时，THE Endgame_Database SHALL 在程序启动时加载完成，加载时间不超过2秒
5. WHERE 残局数据库功能启用时，THE Endgame_Database SHALL 占用内存不超过500MB

### 需求 9：游戏状态管理

**用户故事：** 作为AI引擎，我需要正确管理游戏状态，以便支持悔棋、重复局面检测等功能。

#### 验收标准

1. THE Game_State SHALL 维护完整的棋盘状态（4个位棋盘和当前玩家）
2. THE Game_State SHALL 维护历史走法列表
3. THE Game_State SHALL 支持执行走法（make_move）操作
4. THE Game_State SHALL 支持撤销走法（unmake_move）操作
5. WHEN 执行走法后撤销时，THE Game_State SHALL 恢复到执行前的完全相同状态
6. THE Game_State SHALL 检测三次重复局面（和棋规则）
7. THE Game_State SHALL 检测50步无吃子规则（和棋规则）
8. FOR ALL 走法执行和撤销操作，状态一致性 SHALL 得到保证

### 需求 10：胜负判定

**用户故事：** 作为AI引擎，我需要正确判定胜负，以便在游戏结束时返回正确结果。

#### 验收标准

1. WHEN 一方所有棋子被吃光时，THE Checkers_Engine SHALL 判定该方失败
2. WHEN 一方无合法走法时，THE Checkers_Engine SHALL 判定该方失败
3. WHEN 出现三次重复局面时，THE Checkers_Engine SHALL 判定为和棋
4. WHEN 连续50步无吃子且无普通棋子移动时，THE Checkers_Engine SHALL 判定为和棋
5. THE Checkers_Engine SHALL 在每次走法后检查游戏是否结束
6. WHEN 游戏结束时，THE Checkers_Engine SHALL 返回结果代码（胜/负/和）

### 需求 11：自我对弈测试

**用户故事：** 作为开发者，我需要自我对弈功能，以便测试AI的正确性和强度。

#### 验收标准

1. THE Self_Play SHALL 支持AI与自己对弈完成完整对局
2. THE Self_Play SHALL 记录每一步走法和思考时间
3. THE Self_Play SHALL 在对局结束后输出完整棋谱
4. THE Self_Play SHALL 支持设置不同的搜索深度进行对弈
5. THE Self_Play SHALL 检测并报告规则违规（如非法走法）
6. THE Self_Play SHALL 统计胜率、平均步数、平均思考时间等指标
7. THE Self_Play SHALL 支持批量对弈（如1000局）用于性能测试

### 需求 12：性能优化

**用户故事：** 作为AI引擎，我需要高性能执行，以便在有限时间内搜索更深的层数。

#### 验收标准

1. THE Checkers_Engine SHALL 使用位运算优化棋盘操作
2. THE Move_Generator SHALL 在10微秒内生成所有合法走法
3. THE Checkers_Engine SHALL 每秒搜索至少100,000个节点
4. THE Checkers_Engine SHALL 使用置换表（Transposition Table）缓存已搜索局面
5. THE Checkers_Engine SHALL 实现走法排序以优先搜索可能的最佳走法
6. THE Checkers_Engine SHALL 使用编译器优化选项（-O3或/O2）
7. THE Checkers_Engine SHALL 避免不必要的内存分配和复制

### 需求 13：日志和调试

**用户故事：** 作为开发者，我需要详细的日志，以便调试和分析AI行为。

#### 验收标准

1. WHERE 调试模式启用时，THE Checkers_Engine SHALL 输出每步的搜索深度和评估分数
2. WHERE 调试模式启用时，THE Checkers_Engine SHALL 输出搜索的节点数和剪枝率
3. WHERE 调试模式启用时，THE Checkers_Engine SHALL 输出置换表的命中率
4. WHERE 调试模式启用时，THE Checkers_Engine SHALL 输出每步的思考时间
5. WHERE 调试模式启用时，THE Checkers_Engine SHALL 支持将日志输出到文件
6. THE Checkers_Engine SHALL 支持通过命令行参数或配置文件启用调试模式
7. WHEN 发生错误时，THE Checkers_Engine SHALL 输出详细的错误信息和堆栈跟踪

### 需求 14：配置管理

**用户故事：** 作为开发者和用户，我需要灵活的配置选项，以便调整AI参数和行为。

#### 验收标准

1. THE Checkers_Engine SHALL 支持从配置文件读取参数
2. THE Checkers_Engine SHALL 支持配置搜索深度限制
3. THE Checkers_Engine SHALL 支持配置评估函数的权重参数
4. THE Checkers_Engine SHALL 支持配置时间管理策略
5. THE Checkers_Engine SHALL 支持启用/禁用开局库和残局库
6. THE Checkers_Engine SHALL 支持配置置换表大小
7. WHEN 配置文件不存在或格式错误时，THE Checkers_Engine SHALL 使用默认参数并继续运行

### 需求 15：比赛文档生成

**用户故事：** 作为参赛者，我需要生成比赛要求的文档，以便满足比赛提交要求。

#### 验收标准

1. THE Checkers_Engine SHALL 提供程序设计说明书模板（PDF格式）
2. THE Checkers_Engine SHALL 提供答辩PPT模板
3. THE Checkers_Engine SHALL 在文档中说明算法原理（Alpha-Beta搜索、评估函数）
4. THE Checkers_Engine SHALL 在文档中说明数据结构设计（位棋盘）
5. THE Checkers_Engine SHALL 在文档中说明性能优化措施
6. THE Checkers_Engine SHALL 在文档中提供测试结果和对弈棋谱
7. THE Checkers_Engine SHALL 提供源代码注释和README文档

### 需求 16：代码质量和可维护性

**用户故事：** 作为开发者，我需要高质量的代码，以便后续维护和答辩。

#### 验收标准

1. THE Checkers_Engine SHALL 使用清晰的函数和变量命名
2. THE Checkers_Engine SHALL 为关键函数提供注释说明
3. THE Checkers_Engine SHALL 将代码模块化（走法生成、搜索、评估分离）
4. THE Checkers_Engine SHALL 避免使用魔法数字，使用常量定义
5. THE Checkers_Engine SHALL 使用一致的代码风格
6. THE Checkers_Engine SHALL 提供单元测试覆盖核心功能
7. THE Checkers_Engine SHALL 在Visual Studio 2022中成功编译且无警告

### 需求 17：跨平台兼容性

**用户故事：** 作为参赛者，我需要程序能在比赛环境运行，以便顺利参赛。

#### 验收标准

1. THE Checkers_Engine SHALL 在Windows 10/11系统上正常运行
2. THE Checkers_Engine SHALL 在Linux系统上正常运行（比赛可能使用Linux）
3. THE Checkers_Engine SHALL 使用标准C++17或更高版本
4. THE Checkers_Engine SHALL 避免使用平台特定的API
5. THE Checkers_Engine SHALL 提供CMakeLists.txt或Makefile用于跨平台编译
6. THE Checkers_Engine SHALL 在编译时不依赖第三方库（除标准库外）

### 需求 18：错误处理和鲁棒性

**用户故事：** 作为AI引擎，我需要健壮的错误处理，以便在异常情况下不崩溃。

#### 验收标准

1. WHEN 收到无效的走法输入时，THE Competition_Interface SHALL 拒绝该走法并请求重新输入
2. WHEN 内存分配失败时，THE Checkers_Engine SHALL 降级到更小的置换表并继续运行
3. WHEN 搜索超时时，THE Search_Algorithm SHALL 安全返回当前最佳走法
4. WHEN 文件加载失败时，THE Checkers_Engine SHALL 输出警告并使用默认配置
5. THE Checkers_Engine SHALL 捕获所有异常并避免程序崩溃
6. THE Checkers_Engine SHALL 在异常情况下输出有用的错误信息
7. THE Checkers_Engine SHALL 在比赛模式下禁用所有可能导致崩溃的调试断言

### 需求 19：性能基准测试

**用户故事：** 作为开发者，我需要性能基准测试，以便评估和优化AI性能。

#### 验收标准

1. THE Checkers_Engine SHALL 提供标准测试局面集（如初始局面、中局、残局）
2. THE Checkers_Engine SHALL 测量并报告每秒搜索节点数（NPS）
3. THE Checkers_Engine SHALL 测量并报告不同深度的搜索时间
4. THE Checkers_Engine SHALL 测量并报告置换表命中率
5. THE Checkers_Engine SHALL 测量并报告走法生成速度
6. THE Checkers_Engine SHALL 提供性能对比工具（优化前后对比）
7. THE Checkers_Engine SHALL 在标准硬件上达到至少100,000 NPS

### 需求 20：比赛合规性

**用户故事：** 作为参赛程序，我需要符合比赛规则，以便顺利参赛并避免违规。

#### 验收标准

1. THE Checkers_Engine SHALL 严格遵守国际跳棋（100格）规则
2. THE Checkers_Engine SHALL 在规定时间内完成每步决策
3. THE Checkers_Engine SHALL 不使用外部资源（网络、外部程序）
4. THE Checkers_Engine SHALL 不使用预先计算的完整对局库
5. THE Checkers_Engine SHALL 支持比赛平台的通信协议
6. THE Checkers_Engine SHALL 在比赛环境中稳定运行不崩溃
7. THE Checkers_Engine SHALL 提供完整的源代码供审查
