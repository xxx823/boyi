# MVV-LVA吃子排序实现说明

## 任务概述
实现任务1.2：在quiescence_search中为吃子走法添加MVV-LVA排序

## 实现位置
文件：`boyi/boyi.cpp`
方法：`SearchEngine::quiescence_search`
行数：约1728-1773行

## MVV-LVA算法说明

MVV-LVA = Most Valuable Victim - Least Valuable Attacker（最有价值的受害者 - 最少价值的攻击者）

### 排序规则
1. **优先吃掉价值高的棋子**：王 > 兵
2. **当吃掉同样价值的棋子时，优先使用价值低的棋子去吃**：兵吃 > 王吃

### 实现细节

#### 1. 棋子价值定义
- **王（King）**：300分
- **兵（Man）**：100分

#### 2. MVV-LVA分数计算公式
```cpp
mvv_lva_score = victim_value * 10 - attacker_value
```

其中：
- `victim_value`：所有被吃棋子的总价值（可能是连续吃子）
- `attacker_value`：攻击者的价值（王=300，兵=100）
- 乘以10是为了让被吃棋子的价值占主导地位

#### 3. 排序示例

假设有以下吃子走法：
- A: 兵吃王（victim=300, attacker=100）→ score = 300*10 - 100 = 2900
- B: 王吃王（victim=300, attacker=300）→ score = 300*10 - 300 = 2700
- C: 兵吃兵（victim=100, attacker=100）→ score = 100*10 - 100 = 900
- D: 王吃兵（victim=100, attacker=300）→ score = 100*10 - 300 = 700

排序结果：A > B > C > D

这正确实现了MVV-LVA规则：
- 优先吃王（A、B优先于C、D）
- 同样吃王时，兵吃优先于王吃（A > B）
- 同样吃兵时，兵吃优先于王吃（C > D）

## 代码实现

### 关键代码段

```cpp
// MVV-LVA排序：Most Valuable Victim - Least Valuable Attacker
// 吃王优先级高于吃兵，使用较小价值的棋子吃较大价值的棋子优先
for (Move& move : capture_moves) {
    int mvv_lva_score = 0;
    
    // 确定攻击者类型（from位置的棋子）
    uint64_t from_mask = 1ULL << move.from;
    bool attacker_is_king = false;
    
    if (board.current_player == 1) {
        // 黑方攻击
        attacker_is_king = (board.black_kings & from_mask) != 0;
    } else {
        // 白方攻击
        attacker_is_king = (board.white_kings & from_mask) != 0;
    }
    
    // 计算被吃棋子的总价值（Victim价值）
    int victim_value = 0;
    uint64_t opponent_kings = (board.current_player == 1) ? board.white_kings : board.black_kings;
    
    for (int i = 0; i < move.num_captures; ++i) {
        if (move.captures[i] >= 0 && move.captures[i] < 50) {
            uint64_t capture_mask = 1ULL << move.captures[i];
            if (opponent_kings & capture_mask) {
                victim_value += 300;  // 吃王价值高
            } else {
                victim_value += 100;  // 吃兵价值低
            }
        }
    }
    
    // 计算攻击者价值（Attacker价值，用于减分）
    int attacker_value = attacker_is_king ? 300 : 100;
    
    // MVV-LVA分数 = 被吃棋子价值 * 10 - 攻击者价值
    // 乘以10是为了让victim价值占主导地位
    mvv_lva_score = victim_value * 10 - attacker_value;
    
    move.score = mvv_lva_score;
}

// 按MVV-LVA分数降序排序
std::sort(capture_moves.begin(), capture_moves.end(), [](const Move& a, const Move& b) {
    return a.score > b.score;
});
```

## 验收标准验证

根据需求1.4：
> WHEN Quiescence_Search生成Capture_Move，THE Quiescence_Search SHALL按MVV-LVA顺序排序（Most Valuable Victim - Least Valuable Attacker）

✅ **已实现**：
1. 在生成吃子走法后，立即进行MVV-LVA排序
2. 吃王优先级高于吃兵（王价值300 > 兵价值100）
3. 使用较小价值的棋子吃较大价值的棋子优先（通过减去攻击者价值实现）
4. 支持连续吃子的情况（累加所有被吃棋子的价值）

## 预期效果

1. **提升剪枝效率**：优先搜索最有价值的吃子走法，更容易触发beta剪枝
2. **减少搜索节点**：通过更好的走法排序，减少需要搜索的节点数
3. **避免水平线效应**：在静止搜索中优先考虑最重要的战术走法

## 测试建议

可以通过以下方式验证实现：
1. 创建包含多个吃子选项的测试局面（兵吃王、王吃王、兵吃兵、王吃兵）
2. 运行quiescence_search并观察走法搜索顺序
3. 验证吃王走法确实优先于吃兵走法
4. 验证同样吃子价值时，兵吃优先于王吃

## 相关需求
- 需求1.4：MVV-LVA吃子排序
- 设计文档：完善静态搜索模块
- 任务1.2：实现MVV-LVA吃子排序
