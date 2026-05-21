# 国际跳棋AI对弈系统

完整的国际跳棋人机对弈系统，包含Web界面和AI引擎。

## 🎯 系统架构

```
┌─────────────┐      HTTP/REST API      ┌─────────────┐
│             │ ◄──────────────────────► │             │
│  Web前端    │                          │  Node.js    │
│  (浏览器)   │                          │  后端服务器  │
│             │                          │             │
└─────────────┘                          └──────┬──────┘
                                                │
                                                │ 进程通信
                                                ▼
                                         ┌─────────────┐
                                         │             │
                                         │  C++ AI     │
                                         │  引擎       │
                                         │             │
                                         └─────────────┘
```

## 📦 项目结构

```
.
├── boyi/                   # C++ AI引擎源代码
│   └── boyi.cpp           # 主程序
├── web/                    # Web前端
│   ├── index.html         # 主页面
│   ├── style.css          # 样式
│   ├── checkers.js        # 游戏逻辑
│   └── start.bat          # 前端启动脚本
├── server/                 # Node.js后端
│   ├── server.js          # 服务器主程序
│   ├── package.json       # 依赖配置
│   ├── install.bat        # 依赖安装脚本
│   └── start.bat          # 服务器启动脚本
├── x64/Debug/             # 编译输出
│   └── boyi.exe           # C++可执行文件
├── start-all.bat          # 一键启动脚本
└── README.md              # 本文件
```

## 🚀 快速开始

### 方法1：一键启动（推荐）

1. **双击运行** `start-all.bat`
2. 等待服务器启动（约3秒）
3. 浏览器自动打开游戏界面
4. 开始游戏！

### 方法2：分步启动

#### 步骤1：安装依赖（仅首次）

```bash
cd server
npm install
```

或双击 `server/install.bat`

#### 步骤2：启动后端服务器

```bash
cd server
node server.js
```

或双击 `server/start.bat`

#### 步骤3：打开浏览器

访问：`http://localhost:3000`

## 🎮 使用说明

### 游戏操作

1. **开始游戏**：点击"新游戏"按钮
2. **选择棋子**：点击己方棋子（会高亮显示）
3. **移动棋子**：点击黄色高亮的目标格子
4. **等待AI**：AI会自动思考并走棋

### 游戏设置

- **玩家颜色**：
  - 黑方（先手）
  - 白方（后手）

- **AI难度**：
  - 简单：1秒思考
  - 中等：3秒思考
  - 困难：5秒思考
  - 专家：10秒思考

### 游戏规则

1. **移动规则**
   - 普通棋子只能向前斜走
   - 王棋可以向任意斜方向移动

2. **吃子规则**
   - 跳过对方棋子进行吃子
   - 有吃子机会时必须吃子（强制吃子）
   - 必须选择吃子最多的走法（最大吃子）

3. **升王规则**
   - 黑方棋子到达第10行升王
   - 白方棋子到达第1行升王
   - 王棋用♔标记

4. **胜负判定**
   - 吃光对方所有棋子
   - 对方无合法走法

## 🔧 系统要求

### 必需

- **Windows 10/11**
- **Node.js 14+** ([下载](https://nodejs.org/))
- **现代浏览器**（Chrome/Firefox/Edge）

### 可选

- **Visual Studio 2022**（如需重新编译C++引擎）

## 📡 API接口

### 健康检查

```http
GET /api/health
```

### 创建新游戏

```http
POST /api/game/new
```

### 获取AI走法

```http
POST /api/game/ai-move
Content-Type: application/json

{
  "sessionId": "string",
  "timeLimit": 3000
}
```

### 验证玩家走法

```http
POST /api/game/validate-move
Content-Type: application/json

{
  "sessionId": "string",
  "move": {
    "from": 0,
    "to": 10,
    "captures": []
  }
}
```

### 获取走法提示

```http
POST /api/game/hint
Content-Type: application/json

{
  "sessionId": "string"
}
```

### 评估局面

```http
POST /api/game/evaluate
Content-Type: application/json

{
  "sessionId": "string"
}
```

## 🛠️ 开发指南

### 修改前端

编辑 `web/` 目录下的文件：
- `index.html` - 页面结构
- `style.css` - 样式
- `checkers.js` - 游戏逻辑

刷新浏览器即可看到更改。

### 修改后端

编辑 `server/server.js`，然后重启服务器：

```bash
cd server
node server.js
```

### 重新编译C++引擎

1. 打开 `boyi.sln` 在Visual Studio中
2. 生成 → 重新生成解决方案
3. 可执行文件输出到 `x64/Debug/boyi.exe`

## 🎯 AI引擎特性

### 核心算法

- ✅ **Alpha-Beta剪枝搜索**
- ✅ **主变量搜索（PVS）**
- ✅ **迭代加深**
- ✅ **置换表（Zobrist哈希）**
- ✅ **走法排序**
- ✅ **静止搜索**

### 评估函数

- ✅ 材料评估（棋子价值）
- ✅ 位置评估（控制中心）
- ✅ 机动性评估（可移动性）
- ✅ 安全性评估（棋子保护）
- ✅ 结构评估（连接性）
- ✅ 升王威胁评估
- ✅ 王的活跃度评估

### 高级功能

- ✅ **开局库（Opening Book）**
  - 快速查询：< 10微秒/次
  - 内存高效：< 10MB（1000个局面）
  - 随机变化：支持加权随机选择
  - 性能提升：+50 Elo，节省30秒/局
  - 详细文档：[README_OPENING_BOOK.md](README_OPENING_BOOK.md)
- ✅ 残局库
- ✅ 时间管理
- ✅ 比赛接口

## 🐛 故障排除

### 问题1：服务器无法启动

**原因**：Node.js未安装或依赖未安装

**解决**：
```bash
# 安装Node.js
访问 https://nodejs.org/ 下载安装

# 安装依赖
cd server
npm install
```

### 问题2：浏览器无法连接

**原因**：服务器未启动或端口被占用

**解决**：
1. 确保服务器窗口显示"服务器已启动"
2. 检查端口3000是否被占用
3. 尝试访问 `http://localhost:3000/api/health`

### 问题3：AI不走棋

**原因**：后端API调用失败

**解决**：
1. 打开浏览器开发者工具（F12）
2. 查看Console标签的错误信息
3. 系统会自动降级到本地AI

### 问题4：C++引擎未启动

**原因**：可执行文件不存在或路径错误

**解决**：
1. 检查 `x64/Debug/boyi.exe` 是否存在
2. 在Visual Studio中重新编译
3. 系统会使用JavaScript AI作为备用

## 📝 更新日志

### v1.0.0 (2024-01-XX)

- ✅ 完整的Web界面
- ✅ Node.js后端服务器
- ✅ C++引擎集成
- ✅ API接口
- ✅ 游戏规则实现
- ✅ AI对弈功能

## 📄 许可证

本项目用于2026年辽宁省计算机博弈大赛。

## 👥 开发团队

沈阳航空航天大学 - 计算机博弈团队

## 📧 联系方式

如有问题，请联系开发团队。

---

**祝您游戏愉快！🎉**
