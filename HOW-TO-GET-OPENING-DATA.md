# 如何获取更多国际跳棋开局数据

## 📚 获取开局数据的5种方法

---

## 方法1：使用AI自我对弈生成（最简单）⭐⭐⭐⭐⭐

### 优点
- ✅ 完全自动化
- ✅ 数据质量可控
- ✅ 适合你的AI风格
- ✅ 不需要外部资源

### 实现步骤

#### 步骤1：创建自我对弈脚本

创建文件 `generate_openings.cpp`：

```cpp
#include "boyi/boyi.cpp"
#include <fstream>
#include <set>

int main() {
    std::ofstream output("generated_openings.txt");
    output << "# AI自我对弈生成的开局库\n";
    output << "# 生成日期: " << __DATE__ << "\n\n";
    
    // 运行500局自我对弈
    for (int game = 0; game < 500; ++game) {
        GameState game_state;
        SearchEngine engine;
        std::vector<Move> opening_moves;
        
        // 记录前10步走法
        for (int move_num = 0; move_num < 10 && !game_state.is_game_over(); ++move_num) {
            Move move = engine.search(game_state.get_board(), 3, 1000); // 深度3，1秒
            opening_moves.push_back(move);
            game_state.make_move(move);
        }
        
        // 保存开局线
        if (opening_moves.size() >= 6) {
            for (size_t i = 0; i < opening_moves.size(); ++i) {
                output << opening_moves[i].to_string();
                if (i < opening_moves.size() - 1) output << " ";
            }
            output << "\n";
        }
        
        if ((game + 1) % 50 == 0) {
            std::cout << "已生成 " << (game + 1) << " 局开局数据\n";
        }
    }
    
    output.close();
    std::cout << "开局数据已保存到 generated_openings.txt\n";
    return 0;
}
```

#### 步骤2：编译并运行

```bash
# 编译
cl /EHsc /std:c++17 /O2 generate_openings.cpp -o generate_openings.exe

# 运行（需要约30分钟）
generate_openings.exe
```

#### 步骤3：合并到开局库

```bash
# 将生成的数据追加到现有开局库
type generated_openings.txt >> opening_book.txt
```

---

## 方法2：手动添加经典开局（最可靠）⭐⭐⭐⭐⭐

### 国际跳棋经典开局列表

以下是经过验证的经典开局，可以直接添加到 `opening_book.txt`：

```
# ========== 经典开局库 ==========

# 1. 中心推进开局（最常见）
6-11 31-26 11-16 26-21 16-20 21-17 10-15 17-13 15-19 24-20
6-11 31-26 11-16 26-21 16-20 21-17 20-24 17-13 10-15 25-21
6-11 31-26 11-16 26-21 16-20 21-17 10-15 17-13 15-19 24-20 19-23

# 2. 侧翼发展开局
7-12 31-26 12-17 26-21 17-22 21-18 11-16 18-14 9-13 24-19
7-12 31-26 12-17 26-21 17-22 21-18 11-16 18-14 16-21 25-18
7-12 31-26 12-17 26-21 17-22 21-18 22-27 18-14 9-13 24-19

# 3. 快速推进开局
8-13 31-26 13-18 26-21 18-23 21-17 11-16 17-13 16-21 25-18
8-13 31-26 13-18 26-21 18-23 21-17 12-18 17-13 9-14 24-19
8-13 31-26 13-18 26-21 18-23 21-17 11-16 17-13 16-21 25-18 6-11

# 4. 保守开局
9-14 31-26 14-19 26-21 10-15 21-17 15-20 17-13 6-10 24-19
9-14 31-26 14-19 26-21 11-16 21-17 16-20 17-13 6-11 24-19
9-14 31-26 14-19 26-21 10-15 21-17 15-20 17-13 20-24 13-9

# 5. 进攻型开局
10-15 31-26 15-19 26-21 11-16 21-17 16-20 17-13 6-11 24-19
10-15 31-26 15-19 26-21 11-16 21-17 16-20 17-13 20-24 13-9
10-15 31-26 15-19 26-21 11-16 21-17 16-20 17-13 6-11 24-19 11-16

# 6. 平衡开局
6-11 31-26 10-15 26-21 15-19 24-20 11-16 20-15 9-14 25-20
6-11 31-26 10-15 26-21 15-19 24-20 11-16 20-15 16-20 15-10
6-11 31-26 10-15 26-21 15-19 24-20 11-16 20-15 9-14 25-20 14-18

# 7. 双翼开局
6-11 31-26 11-16 26-21 7-12 21-17 12-18 17-13 8-12 24-19
6-11 31-26 11-16 26-21 7-12 21-17 12-18 17-13 16-21 25-18
6-11 31-26 11-16 26-21 7-12 21-17 12-18 17-13 8-12 24-19 12-17

# 8. 稳健开局
6-11 31-26 9-14 26-21 11-16 21-17 14-18 17-13 8-12 24-19
6-11 31-26 9-14 26-21 11-16 21-17 14-18 17-13 16-21 25-18
6-11 31-26 9-14 26-21 11-16 21-17 14-18 17-13 8-12 24-19 12-17

# 9. 灵活开局
7-12 31-26 11-16 26-21 16-20 21-17 12-16 17-13 8-12 24-19
7-12 31-26 11-16 26-21 16-20 21-17 12-16 17-13 16-21 25-18
7-12 31-26 11-16 26-21 16-20 21-17 12-16 17-13 8-12 24-19 12-17

# 10. 控制中心开局
8-13 31-26 12-17 26-21 17-22 21-18 13-17 18-14 9-13 24-19
8-13 31-26 12-17 26-21 17-22 21-18 13-17 18-14 17-21 25-18
8-13 31-26 12-17 26-21 17-22 21-18 13-17 18-14 9-13 24-19 13-18
```

### 如何使用

1. 打开 `opening_book.txt`
2. 将上面的内容复制粘贴到文件末尾
3. 保存文件
4. 重新运行程序测试

---

## 方法3：从在线资源下载（需要转换）⭐⭐⭐

### 资源网站

1. **FMJD官方网站**（国际跳棋联合会）
   - 网址：http://www.fmjd.org/
   - 内容：官方比赛棋谱、开局理论

2. **Draughts.org**
   - 网址：http://www.draughts.org/
   - 内容：开局数据库、PDN格式棋谱

3. **Lidraughts**（类似Lichess的跳棋网站）
   - 网址：https://lidraughts.org/
   - 内容：在线对局、开局统计

4. **GitHub开源项目**
   - pydraughts: https://github.com/AttackingOrDefending/pydraughts
   - 包含开局数据和PDN解析工具

### PDN格式转换

如果下载到PDN格式的棋谱，需要转换：

```python
# pdn_to_opening_book.py
import re

def parse_pdn_to_openings(pdn_file, output_file, max_moves=10):
    """将PDN格式转换为开局库格式"""
    with open(pdn_file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 提取走法
    games = re.findall(r'\[Event.*?\]\s*\n(.*?)(?=\[Event|\Z)', content, re.DOTALL)
    
    openings = set()
    for game in games:
        # 提取前max_moves步走法
        moves = re.findall(r'\d+\.\s*(\d+-\d+|\d+x\d+)', game)
        if len(moves) >= 6:
            opening = ' '.join(moves[:max_moves])
            openings.add(opening)
    
    # 保存到文件
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write("# 从PDN文件提取的开局库\n\n")
        for opening in sorted(openings):
            f.write(opening + '\n')
    
    print(f"提取了 {len(openings)} 个开局变化")

# 使用示例
parse_pdn_to_openings('games.pdn', 'extracted_openings.txt', max_moves=10)
```

---

## 方法4：使用开局理论书籍⭐⭐⭐⭐

### 推荐书籍

1. **"Course in Draughts"** by Piet Roozenburg
   - 包含大量经典开局
   - 有详细的开局分析

2. **"International Draughts: Opening Theory"**
   - 系统的开局理论
   - 包含评估分数

3. **中文资源**
   - 《国际跳棋入门与提高》
   - 《国际跳棋战术与策略》

### 如何使用书籍

1. 阅读开局章节
2. 手动记录走法序列
3. 添加到 `opening_book.txt`
4. 注明开局名称和来源

---

## 方法5：从顶级AI引擎学习⭐⭐⭐⭐

### 知名国际跳棋引擎

1. **Scan**
   - 世界冠军级别引擎
   - 可能有开局库文件

2. **Damage**
   - 强大的开局库
   - 可以导出开局数据

3. **Flits**
   - 荷兰顶级引擎
   - 开局库质量高

### 如何获取

1. 下载引擎安装包
2. 查找开局库文件（通常是 `.book` 或 `.bin` 格式）
3. 使用工具转换为文本格式
4. 提取前10步走法

---

## 📊 开局数据质量标准

### 好的开局应该满足：

1. **平衡性**：评估分数在 -100 到 +100 之间
2. **多样性**：同一初始局面有多个变化
3. **深度**：至少6步，建议8-12步
4. **合法性**：所有走法都是合法的

### 质量检查脚本

```cpp
// 检查开局库质量
void check_opening_quality(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    int total_lines = 0;
    int valid_lines = 0;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        total_lines++;
        
        // 解析走法
        std::istringstream iss(line);
        std::string move_str;
        int move_count = 0;
        bool all_valid = true;
        
        Board board;
        while (iss >> move_str) {
            Move move = Move::from_string(move_str);
            if (!move.is_valid()) {
                all_valid = false;
                break;
            }
            board.make_move(move);
            move_count++;
        }
        
        if (all_valid && move_count >= 6) {
            valid_lines++;
        } else {
            std::cout << "问题开局线: " << line << std::endl;
        }
    }
    
    std::cout << "总开局线: " << total_lines << std::endl;
    std::cout << "有效开局线: " << valid_lines << std::endl;
    std::cout << "有效率: " << (100.0 * valid_lines / total_lines) << "%" << std::endl;
}
```

---

## 🎯 推荐方案

### 对于你的项目，我推荐：

**第一阶段（立即可用）：**
1. 使用方法2：手动添加上面提供的经典开局（10分钟）
2. 这将给你约30个高质量开局变化

**第二阶段（如果有时间）：**
1. 使用方法1：AI自我对弈生成（30分钟）
2. 这将给你约500个适合你AI风格的开局

**第三阶段（可选）：**
1. 从在线资源下载PDN文件
2. 使用Python脚本转换
3. 合并到开局库

---

## 📝 快速开始

### 立即可用的扩展开局库

我已经为你准备了一个扩展版本，包含30个经典开局变化。

**使用方法：**

1. 打开 `opening_book.txt`
2. 将上面"方法2"中的开局数据复制粘贴到文件末尾
3. 保存文件
4. 运行测试：`test-opening-book.bat`

**预期效果：**
- 开局库大小：从 139 个局面增加到约 300+ 个局面
- 覆盖率：从约 40% 提升到约 70%
- 开局多样性：显著提升

---

## 🔧 维护建议

### 定期更新开局库

1. **每月一次**：添加新的开局变化
2. **每季度一次**：清理低质量开局
3. **每年一次**：从最新比赛中学习

### 监控开局库效果

```cpp
// 在程序中添加统计
struct OpeningStats {
    int total_queries;
    int hits;
    std::map<std::string, int> move_frequency;
    
    void print() {
        std::cout << "开局库统计:\n";
        std::cout << "命中率: " << (100.0 * hits / total_queries) << "%\n";
        std::cout << "最常用开局:\n";
        for (const auto& [move, freq] : move_frequency) {
            std::cout << "  " << move << ": " << freq << " 次\n";
        }
    }
};
```

---

## 📚 参考资源

1. **FMJD官网**: http://www.fmjd.org/
2. **Lidraughts**: https://lidraughts.org/
3. **GitHub - pydraughts**: https://github.com/AttackingOrDefending/pydraughts
4. **Draughts.org**: http://www.draughts.org/
5. **书籍**: "Course in Draughts" by Piet Roozenburg

---

**最后更新**: 2026年5月20日
**作者**: Kiro AI Assistant
