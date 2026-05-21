# 需求文档：国际跳棋AI开局库功能

## 简介

本文档定义了为国际跳棋AI程序添加开局库（Opening Book）功能的需求。开局库是一个存储常见开局走法的数据库，能够让AI在开局阶段快速走出高质量的走法，避免开局失误，节省思考时间，提升整体棋力。

**功能价值：**
- **性价比评级：** ⭐⭐⭐⭐⭐（最高）
- **预计开发时间：** 2-3天
- **预期效果：** +50 Elo，节省30秒思考时间
- **代码量：** 约200行

**现有基础：**
- 已有OpeningBook类的基础实现（boyi/boyi.cpp）
- 已实现基本的load()和probe()方法
- 已集成到CompetitionInterface类中

**实现方案：**
1. 完善OpeningBook类的三个核心函数
2. 准备开局数据（从网上下载PDN格式对局库或AI自我对弈生成）
3. 集成到主程序并测试效果

## 术语表

- **Opening_Book**: 开局库，存储常见开局走法的数据库系统
- **PDN**: Portable Draughts Notation，国际跳棋的标准棋谱格式
- **Opening_Line**: 开局线，一个完整的开局走法序列
- **Book_Move**: 开局库走法，从开局库中查询到的推荐走法
- **Position_Hash**: 局面哈希值，使用Zobrist哈希唯一标识一个棋盘局面
- **Self_Play**: 自我对弈，AI与自己对弈生成训练数据
- **Move_Variation**: 走法变化，同一局面下的不同走法选择
- **Opening_Phase**: 开局阶段，通常指对局的前10-15步
- **Book_Hit**: 开局库命中，当前局面在开局库中找到匹配
- **Book_Miss**: 开局库未命中，当前局面不在开局库中
- **Transposition**: 移位，不同走法顺序到达相同局面
- **Opening_Repertoire**: 开局库存，AI掌握的所有开局变化
- **Book_Depth**: 开局库深度，开局库覆盖的最大步数
- **Move_Weight**: 走法权重，用于加权随机选择开局走法

## 需求

### 需求 1：开局库数据加载

**用户故事：** 作为AI引擎，我需要从文件加载开局库数据，以便在程序启动时准备好开局知识。

#### 验收标准

1. THE Opening_Book SHALL 支持从文本文件加载开局数据
2. THE Opening_Book SHALL 支持PDN格式的棋谱文件
3. WHEN 文件格式为简化格式时，THE Opening_Book SHALL 解析每行的走法序列（格式："6-11 31-26 11-16 26-21"）
4. WHEN 文件格式为PDN格式时，THE Opening_Book SHALL 解析标准PDN棋谱并提取前10步走法
5. THE Opening_Book SHALL 在加载过程中构建Position_Hash到走法列表的映射
6. WHEN 遇到相同局面的不同走法时，THE Opening_Book SHALL 将所有走法添加到该局面的候选列表
7. THE Opening_Book SHALL 在程序启动时完成加载，加载时间不超过500毫秒
8. WHEN 文件不存在或格式错误时，THE Opening_Book SHALL 使用内置的基础开局库并输出警告信息
9. THE Opening_Book SHALL 在加载完成后输出统计信息（开局线数量、局面数量）
10. FOR ALL 加载的开局走法，走法的合法性 SHALL 在加载时验证

### 需求 2：开局库查询

**用户故事：** 作为AI引擎，我需要快速查询当前局面的开局走法，以便在开局阶段立即响应。

#### 验收标准

1. THE Opening_Book SHALL 使用Position_Hash作为查询键
2. WHEN 当前局面在开局库中时，THE Opening_Book SHALL 返回该局面的候选走法列表
3. WHEN 当前局面不在开局库中时，THE Opening_Book SHALL 返回无效走法（Move()）
4. THE Opening_Book SHALL 在10微秒内完成单次查询
5. WHEN 一个局面有多个候选走法时，THE Opening_Book SHALL 随机选择一个走法以增加变化
6. WHERE 走法权重功能启用时，THE Opening_Book SHALL 根据走法权重进行加权随机选择
7. THE Opening_Book SHALL 提供contains()方法检查局面是否在开局库中
8. THE Opening_Book SHALL 提供get_all_moves()方法返回局面的所有候选走法
9. FOR ALL 返回的走法，走法的合法性 SHALL 得到保证

### 需求 3：开局库集成

**用户故事：** 作为比赛接口，我需要在思考流程中优先使用开局库，以便节省开局阶段的思考时间。

#### 验收标准

1. WHEN 轮到AI走棋时，THE CompetitionInterface SHALL 首先查询开局库
2. WHEN 开局库命中时，THE CompetitionInterface SHALL 直接使用开局库走法，不启动搜索引擎
3. WHEN 开局库未命中时，THE CompetitionInterface SHALL 切换到搜索引擎进行正常搜索
4. THE CompetitionInterface SHALL 在开局库命中时输出"Opening book hit!"信息
5. THE CompetitionInterface SHALL 记录开局库命中的走法用时为1毫秒
6. THE CompetitionInterface SHALL 支持通过配置启用或禁用开局库功能
7. WHEN 开局库被禁用时，THE CompetitionInterface SHALL 始终使用搜索引擎
8. THE CompetitionInterface SHALL 在残局库查询之后、搜索引擎之前查询开局库

### 需求 4：开局数据准备

**用户故事：** 作为开发者，我需要准备高质量的开局数据，以便开局库能够提供有效的走法建议。

#### 验收标准

1. THE Opening_Book SHALL 包含至少50个常见开局变化
2. THE Opening_Book SHALL 覆盖开局阶段的前10步走法
3. WHERE 使用下载数据方式时，THE Opening_Book SHALL 从公开的PDN对局库中提取开局走法
4. WHERE 使用自我对弈方式时，THE Opening_Book SHALL 通过500局自我对弈生成开局数据
5. THE Opening_Book SHALL 过滤掉明显失误的开局走法（评估分数低于-100）
6. THE Opening_Book SHALL 优先包含平衡的开局走法（评估分数在-50到+50之间）
7. THE Opening_Book SHALL 包含多样化的开局变化，避免单一开局路线
8. THE Opening_Book SHALL 提供数据生成工具脚本（generate_opening_book.cpp或Python脚本）

### 需求 5：开局库性能优化

**用户故事：** 作为AI引擎，我需要高效的开局库实现，以便开局库不成为性能瓶颈。

#### 验收标准

1. THE Opening_Book SHALL 使用std::unordered_map存储Position_Hash到走法列表的映射
2. THE Opening_Book SHALL 在查询时使用O(1)平均时间复杂度
3. THE Opening_Book SHALL 占用内存不超过10MB（存储1000个局面）
4. THE Opening_Book SHALL 在加载时预分配足够的哈希表空间以避免重新分配
5. THE Opening_Book SHALL 使用移动语义避免不必要的走法复制
6. THE Opening_Book SHALL 在多线程环境下保证查询的线程安全性（只读操作）

### 需求 6：开局库统计和调试

**用户故事：** 作为开发者，我需要开局库的统计信息，以便评估开局库的质量和使用效果。

#### 验收标准

1. THE Opening_Book SHALL 记录开局库的查询次数
2. THE Opening_Book SHALL 记录开局库的命中次数
3. THE Opening_Book SHALL 提供get_hit_rate()方法计算命中率
4. THE Opening_Book SHALL 提供get_statistics()方法返回详细统计信息
5. WHERE 调试模式启用时，THE Opening_Book SHALL 输出每次查询的结果（命中或未命中）
6. WHERE 调试模式启用时，THE Opening_Book SHALL 输出每个局面的候选走法数量
7. THE Opening_Book SHALL 在程序结束时输出开局库使用统计（总查询、命中率、节省时间）

### 需求 7：开局库文件格式

**用户故事：** 作为开发者，我需要清晰的开局库文件格式，以便手动编辑和维护开局库。

#### 验收标准

1. THE Opening_Book SHALL 支持简化文本格式（每行一个开局线）
2. THE Opening_Book SHALL 支持注释行（以#开头的行）
3. THE Opening_Book SHALL 支持空行（用于分隔不同开局类型）
4. THE Opening_Book SHALL 使用标准走法表示格式（"6-11"表示普通移动，"15x24x33"表示跳吃）
5. WHERE 使用加权格式时，THE Opening_Book SHALL 支持走法权重标注（格式："6-11:80 7-12:20"）
6. THE Opening_Book SHALL 提供示例文件opening_book_example.txt展示文件格式
7. THE Opening_Book SHALL 在文件格式错误时输出详细的错误信息（行号、错误类型）

### 需求 8：开局库生成工具

**用户故事：** 作为开发者，我需要自动生成开局库的工具，以便快速构建和更新开局库。

#### 验收标准

1. THE Opening_Book SHALL 提供generate_from_self_play()方法通过自我对弈生成开局数据
2. WHEN 生成开局库时，THE Opening_Book SHALL 运行指定数量的自我对弈局（默认500局）
3. WHEN 生成开局库时，THE Opening_Book SHALL 记录每局的前10步走法
4. WHEN 生成开局库时，THE Opening_Book SHALL 过滤掉评估分数异常的走法
5. WHEN 生成开局库时，THE Opening_Book SHALL 统计每个走法的出现频率作为权重
6. THE Opening_Book SHALL 提供save_to_file()方法将开局库保存到文件
7. THE Opening_Book SHALL 提供merge()方法合并多个开局库
8. THE Opening_Book SHALL 提供prune()方法删除低质量或低频率的走法

### 需求 9：开局库验证

**用户故事：** 作为开发者，我需要验证开局库的正确性，以便确保开局库不包含非法走法。

#### 验收标准

1. THE Opening_Book SHALL 在加载时验证每个走法的合法性
2. WHEN 发现非法走法时，THE Opening_Book SHALL 跳过该走法并输出警告
3. THE Opening_Book SHALL 提供validate()方法检查开局库的完整性
4. THE Opening_Book SHALL 检测并报告重复的开局线
5. THE Opening_Book SHALL 检测并报告孤立的局面（没有前驱局面）
6. THE Opening_Book SHALL 提供test_coverage()方法测试开局库的覆盖范围
7. THE Opening_Book SHALL 提供simulate_games()方法模拟使用开局库的对局

### 需求 10：开局库效果评估

**用户故事：** 作为开发者，我需要评估开局库的实际效果，以便验证开局库是否达到预期目标。

#### 验收标准

1. THE Opening_Book SHALL 在使用开局库的对局中记录开局阶段的用时
2. THE Opening_Book SHALL 统计开局库命中后的胜率
3. THE Opening_Book SHALL 统计开局库节省的总思考时间
4. WHEN 开局库命中时，THE Opening_Book SHALL 记录走法的评估分数（通过后续搜索验证）
5. THE Opening_Book SHALL 提供compare_with_search()方法比较开局库走法与搜索引擎走法的差异
6. THE Opening_Book SHALL 在测试对局中验证开局库走法的质量不低于深度5的搜索结果
7. THE Opening_Book SHALL 在100局测试对局中验证平均节省至少25秒思考时间

### 需求 11：开局库扩展性

**用户故事：** 作为开发者，我需要可扩展的开局库设计，以便未来添加更多功能。

#### 验收标准

1. THE Opening_Book SHALL 支持动态添加新的开局线（add_line()方法）
2. THE Opening_Book SHALL 支持从多个文件加载开局库（load_multiple_files()方法）
3. THE Opening_Book SHALL 支持导出开局库到不同格式（export_to_pgn()、export_to_json()）
4. WHERE 学习功能启用时，THE Opening_Book SHALL 支持从实际对局中学习新的开局走法
5. WHERE 学习功能启用时，THE Opening_Book SHALL 根据走法的胜率调整走法权重
6. THE Opening_Book SHALL 提供clear()方法清空开局库
7. THE Opening_Book SHALL 提供rebuild()方法重建开局库索引

### 需求 12：开局库错误处理

**用户故事：** 作为AI引擎，我需要健壮的错误处理，以便开局库异常不影响程序运行。

#### 验收标准

1. WHEN 开局库文件不存在时，THE Opening_Book SHALL 使用内置开局库并继续运行
2. WHEN 开局库文件损坏时，THE Opening_Book SHALL 跳过损坏的条目并加载其余数据
3. WHEN 内存不足时，THE Opening_Book SHALL 降级到最小开局库（仅初始局面）
4. WHEN 查询超时时，THE Opening_Book SHALL 返回无效走法并切换到搜索引擎
5. THE Opening_Book SHALL 捕获所有异常并避免程序崩溃
6. THE Opening_Book SHALL 在错误情况下输出详细的错误信息和堆栈跟踪
7. THE Opening_Book SHALL 提供is_loaded()方法检查开局库是否成功加载

### 需求 13：开局库配置

**用户故事：** 作为用户，我需要灵活的开局库配置，以便根据不同场景调整开局库行为。

#### 验收标准

1. THE Opening_Book SHALL 支持配置开局库文件路径
2. THE Opening_Book SHALL 支持配置开局库深度（最大步数）
3. THE Opening_Book SHALL 支持配置是否启用随机选择
4. THE Opening_Book SHALL 支持配置是否启用走法权重
5. THE Opening_Book SHALL 支持配置最小走法权重阈值
6. THE Opening_Book SHALL 支持配置是否启用学习功能
7. THE Opening_Book SHALL 从配置文件或命令行参数读取配置

### 需求 14：开局库文档

**用户故事：** 作为开发者和用户，我需要完整的开局库文档，以便理解和使用开局库功能。

#### 验收标准

1. THE Opening_Book SHALL 提供README_OPENING_BOOK.md文档说明开局库功能
2. THE Opening_Book SHALL 在文档中说明开局库的设计原理
3. THE Opening_Book SHALL 在文档中提供文件格式说明和示例
4. THE Opening_Book SHALL 在文档中提供使用指南（如何加载、查询、生成）
5. THE Opening_Book SHALL 在文档中提供性能优化建议
6. THE Opening_Book SHALL 在文档中提供常见问题解答（FAQ）
7. THE Opening_Book SHALL 在代码中为关键函数提供详细注释

### 需求 15：开局库测试

**用户故事：** 作为开发者，我需要完整的测试覆盖，以便确保开局库功能正确可靠。

#### 验收标准

1. THE Opening_Book SHALL 提供单元测试覆盖load()、probe()、save()等核心方法
2. THE Opening_Book SHALL 提供集成测试验证与CompetitionInterface的集成
3. THE Opening_Book SHALL 提供性能测试验证查询速度和内存占用
4. THE Opening_Book SHALL 提供正确性测试验证走法合法性
5. THE Opening_Book SHALL 提供压力测试验证大规模开局库的性能
6. THE Opening_Book SHALL 提供回归测试验证开局库更新后的兼容性
7. THE Opening_Book SHALL 在所有测试通过后才允许提交代码

### 需求 16：开局库与搜索引擎协同

**用户故事：** 作为AI引擎，我需要开局库与搜索引擎协同工作，以便平滑过渡到中局阶段。

#### 验收标准

1. WHEN 开局库未命中时，THE CompetitionInterface SHALL 无缝切换到搜索引擎
2. WHEN 开局库命中但走法质量可疑时，THE CompetitionInterface SHALL 使用搜索引擎验证走法
3. WHERE 混合模式启用时，THE CompetitionInterface SHALL 同时查询开局库和搜索引擎，选择更优走法
4. THE CompetitionInterface SHALL 在开局库覆盖范围的边界使用更长的搜索时间
5. THE CompetitionInterface SHALL 记录开局库最后命中的步数
6. THE CompetitionInterface SHALL 在开局库未命中后调整时间分配策略

### 需求 17：开局库数据质量

**用户故事：** 作为AI引擎，我需要高质量的开局数据，以便开局库真正提升棋力而不是引入错误。

#### 验收标准

1. THE Opening_Book SHALL 只包含评估分数在合理范围内的走法（-100到+100）
2. THE Opening_Book SHALL 避免包含明显的战术失误走法
3. THE Opening_Book SHALL 优先包含大师级对局中的开局走法
4. THE Opening_Book SHALL 包含多样化的开局风格（进攻型、防守型、平衡型）
5. THE Opening_Book SHALL 定期更新以包含最新的开局理论
6. THE Opening_Book SHALL 提供quality_score()方法评估开局库的整体质量
7. THE Opening_Book SHALL 在质量评分低于阈值时输出警告

### 需求 18：开局库与时间管理协同

**用户故事：** 作为时间管理器，我需要知道开局库的使用情况，以便更好地分配剩余时间。

#### 验收标准

1. WHEN 开局库命中时，THE TimeManager SHALL 将节省的时间累加到剩余时间
2. THE TimeManager SHALL 根据开局库覆盖深度调整开局阶段的时间分配
3. THE TimeManager SHALL 在开局库未命中后增加中局阶段的时间预算
4. THE TimeManager SHALL 记录开局库节省的总时间用于统计
5. THE TimeManager SHALL 在时间紧张时优先依赖开局库而不是搜索

### 需求 19：开局库可视化

**用户故事：** 作为开发者，我需要可视化开局库的内容，以便理解和调试开局库。

#### 验收标准

1. WHERE 可视化功能启用时，THE Opening_Book SHALL 提供print_tree()方法打印开局树结构
2. WHERE 可视化功能启用时，THE Opening_Book SHALL 提供export_to_graphviz()方法导出为图形格式
3. WHERE 可视化功能启用时，THE Opening_Book SHALL 显示每个局面的候选走法数量
4. WHERE 可视化功能启用时，THE Opening_Book SHALL 显示每个走法的权重和统计信息
5. WHERE 可视化功能启用时，THE Opening_Book SHALL 高亮显示最常用的开局路径

### 需求 20：开局库性能基准

**用户故事：** 作为开发者，我需要性能基准测试，以便验证开局库达到预期性能目标。

#### 验收标准

1. THE Opening_Book SHALL 在标准硬件上完成100,000次查询在1秒内
2. THE Opening_Book SHALL 加载1000个局面的开局库在500毫秒内
3. THE Opening_Book SHALL 占用内存不超过10MB（1000个局面）
4. THE Opening_Book SHALL 在开局阶段（前10步）的平均响应时间不超过1毫秒
5. THE Opening_Book SHALL 在100局测试对局中验证平均节省25-30秒思考时间
6. THE Opening_Book SHALL 在100局测试对局中验证Elo提升40-60分
7. THE Opening_Book SHALL 提供benchmark()方法运行性能基准测试

