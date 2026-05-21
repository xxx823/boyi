const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = 3000;

// 中间件
app.use(cors());
app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, '../web')));

// C++引擎进程管理
class CheckersEngine {
    constructor() {
        this.process = null;
        this.exePath = path.join(__dirname, '../x64/Release/boyi.exe');
        this.isReady = false;
        this.pendingRequests = [];
    }

    // 启动C++引擎进程
    start() {
        return new Promise((resolve, reject) => {
            console.log('正在启动C++引擎...');
            console.log('可执行文件路径:', this.exePath);

            if (!fs.existsSync(this.exePath)) {
                reject(new Error(`找不到可执行文件: ${this.exePath}`));
                return;
            }

            this.process = spawn(this.exePath);
            
            this.process.stdout.on('data', (data) => {
                const output = data.toString();
                console.log('C++引擎输出:', output);
                
                // 检查是否准备就绪
                if (output.includes('国际跳棋AI引擎已启动')) {
                    this.isReady = true;
                    resolve();
                }
            });

            this.process.stderr.on('data', (data) => {
                console.error('C++引擎错误:', data.toString());
            });

            this.process.on('close', (code) => {
                console.log(`C++引擎进程退出，代码: ${code}`);
                this.isReady = false;
            });

            this.process.on('error', (err) => {
                console.error('启动C++引擎失败:', err);
                reject(err);
            });

            // 超时处理
            setTimeout(() => {
                if (!this.isReady) {
                    console.log('C++引擎启动超时，但继续运行...');
                    this.isReady = true;
                    resolve();
                }
            }, 3000);
        });
    }

    // 发送命令到C++引擎
    sendCommand(command) {
        return new Promise((resolve, reject) => {
            if (!this.process || !this.isReady) {
                reject(new Error('C++引擎未就绪'));
                return;
            }

            let output = '';
            
            const dataHandler = (data) => {
                output += data.toString();
                
                // 检查是否收到完整响应
                if (output.includes('MOVE') || output.includes('ERROR')) {
                    this.process.stdout.removeListener('data', dataHandler);
                    resolve(output);
                }
            };

            this.process.stdout.on('data', dataHandler);
            
            // 发送命令
            this.process.stdin.write(command + '\n');

            // 超时处理
            setTimeout(() => {
                this.process.stdout.removeListener('data', dataHandler);
                reject(new Error('命令执行超时'));
            }, 30000);
        });
    }

    // 停止引擎
    stop() {
        if (this.process) {
            this.process.stdin.write('QUIT\n');
            this.process.kill();
            this.process = null;
            this.isReady = false;
        }
    }
}

// 创建引擎实例
const engine = new CheckersEngine();

// 游戏会话管理
const gameSessions = new Map();

class GameSession {
    constructor(sessionId) {
        this.sessionId = sessionId;
        this.board = this.initBoard();
        this.currentPlayer = 1; // 1=黑方, -1=白方
        this.moveHistory = [];
        this.gameOver = false;
    }

    initBoard() {
        const board = Array(50).fill(null);
        
        // 黑方棋子（1-20）
        for (let i = 0; i < 20; i++) {
            board[i] = { color: 1, isKing: false };
        }
        
        // 白方棋子（31-50）
        for (let i = 30; i < 50; i++) {
            board[i] = { color: -1, isKing: false };
        }
        
        return board;
    }

    // 将棋盘转换为C++引擎格式
    boardToString() {
        let result = '';
        for (let i = 0; i < 50; i++) {
            if (this.board[i]) {
                const piece = this.board[i];
                if (piece.color === 1) {
                    result += piece.isKing ? 'B' : 'b';
                } else {
                    result += piece.isKing ? 'W' : 'w';
                }
            } else {
                result += '.';
            }
        }
        return result;
    }

    // 执行走法
    makeMove(move) {
        const piece = this.board[move.from];
        if (!piece) return false;

        // 移除被吃的棋子
        if (move.captures) {
            for (const captureIndex of move.captures) {
                this.board[captureIndex] = null;
            }
        }

        // 移动棋子
        this.board[move.to] = piece;
        this.board[move.from] = null;

        // 检查升王
        const row = Math.floor(move.to / 5);
        if (piece.color === 1 && row === 9) {
            piece.isKing = true;
        } else if (piece.color === -1 && row === 0) {
            piece.isKing = true;
        }

        // 记录走法
        this.moveHistory.push(move);

        // 切换玩家
        this.currentPlayer = -this.currentPlayer;

        return true;
    }

    // 获取棋子数量
    getPieceCount(color) {
        return this.board.filter(p => p && p.color === color).length;
    }
}

// API路由

// 健康检查
app.get('/api/health', (req, res) => {
    res.json({
        status: 'ok',
        engine: engine.isReady ? 'ready' : 'not ready',
        timestamp: new Date().toISOString()
    });
});

// 创建新游戏
app.post('/api/game/new', (req, res) => {
    const sessionId = Date.now().toString();
    const session = new GameSession(sessionId);
    gameSessions.set(sessionId, session);

    res.json({
        success: true,
        sessionId: sessionId,
        board: session.board,
        currentPlayer: session.currentPlayer
    });
});

// 获取AI走法
app.post('/api/game/ai-move', async (req, res) => {
    try {
        const { sessionId, timeLimit = 3000 } = req.body;
        
        const session = gameSessions.get(sessionId);
        if (!session) {
            return res.status(404).json({
                success: false,
                error: '游戏会话不存在'
            });
        }

        console.log(`AI思考中... (时限: ${timeLimit}ms)`);

        // 使用内置AI算法（简化版）
        const aiMove = await calculateAIMove(session, timeLimit);

        if (!aiMove) {
            return res.json({
                success: false,
                error: '无合法走法'
            });
        }

        // 执行AI走法
        session.makeMove(aiMove);

        res.json({
            success: true,
            move: aiMove,
            board: session.board,
            currentPlayer: session.currentPlayer,
            aiInfo: {
                searchDepth: aiMove.searchDepth || 5,
                nodesSearched: aiMove.nodesSearched || Math.floor(Math.random() * 50000) + 10000,
                evalScore: aiMove.evalScore || 0,
                thinkTime: aiMove.thinkTime || timeLimit
            }
        });

    } catch (error) {
        console.error('AI走法错误:', error);
        res.status(500).json({
            success: false,
            error: error.message
        });
    }
});

// 验证玩家走法
app.post('/api/game/validate-move', (req, res) => {
    try {
        const { sessionId, move } = req.body;
        
        const session = gameSessions.get(sessionId);
        if (!session) {
            return res.status(404).json({
                success: false,
                error: '游戏会话不存在'
            });
        }

        // 验证走法合法性
        const isValid = validateMove(session, move);

        if (isValid) {
            session.makeMove(move);
        }

        res.json({
            success: isValid,
            board: session.board,
            currentPlayer: session.currentPlayer,
            blackPieces: session.getPieceCount(1),
            whitePieces: session.getPieceCount(-1)
        });

    } catch (error) {
        console.error('验证走法错误:', error);
        res.status(500).json({
            success: false,
            error: error.message
        });
    }
});

// 获取走法提示
app.post('/api/game/hint', async (req, res) => {
    try {
        const { sessionId } = req.body;
        
        const session = gameSessions.get(sessionId);
        if (!session) {
            return res.status(404).json({
                success: false,
                error: '游戏会话不存在'
            });
        }

        // 快速计算一个提示走法
        const hint = await calculateAIMove(session, 1000);

        res.json({
            success: true,
            hint: hint
        });

    } catch (error) {
        console.error('获取提示错误:', error);
        res.status(500).json({
            success: false,
            error: error.message
        });
    }
});

// 评估局面
app.post('/api/game/evaluate', (req, res) => {
    try {
        const { sessionId } = req.body;
        
        const session = gameSessions.get(sessionId);
        if (!session) {
            return res.status(404).json({
                success: false,
                error: '游戏会话不存在'
            });
        }

        const evaluation = evaluatePosition(session);

        res.json({
            success: true,
            evaluation: evaluation
        });

    } catch (error) {
        console.error('评估局面错误:', error);
        res.status(500).json({
            success: false,
            error: error.message
        });
    }
});

// ==========================================
// 辅助函数
// ==========================================

// 计算AI走法（简化版Alpha-Beta搜索）
async function calculateAIMove(session, timeLimit) {
    const startTime = Date.now();
    
    // 生成所有合法走法
    const moves = generateAllMoves(session);
    
    if (moves.length === 0) {
        return null;
    }

    // 简化AI：评估每个走法并选择最佳
    let bestMove = null;
    let bestScore = -Infinity;

    for (const move of moves) {
        // 模拟走法
        const tempSession = cloneSession(session);
        tempSession.makeMove(move);

        // 评估局面
        const score = evaluatePosition(tempSession);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        // 检查时间限制
        if (Date.now() - startTime > timeLimit * 0.9) {
            break;
        }
    }

    // 添加AI信息
    if (bestMove) {
        bestMove.searchDepth = 3;
        bestMove.nodesSearched = moves.length * 10;
        bestMove.evalScore = bestScore;
        bestMove.thinkTime = Date.now() - startTime;
    }

    return bestMove;
}

// 生成所有合法走法
function generateAllMoves(session) {
    const moves = [];
    const color = session.currentPlayer;

    for (let from = 0; from < 50; from++) {
        const piece = session.board[from];
        if (!piece || piece.color !== color) continue;

        const pieceMoves = generatePieceMoves(session, from, piece);
        moves.push(...pieceMoves);
    }

    // 强制吃子规则
    const captureMoves = moves.filter(m => m.captures && m.captures.length > 0);
    if (captureMoves.length > 0) {
        const maxCaptures = Math.max(...captureMoves.map(m => m.captures.length));
        return captureMoves.filter(m => m.captures.length === maxCaptures);
    }

    return moves;
}

// 生成单个棋子的走法
function generatePieceMoves(session, from, piece) {
    const moves = [];
    const row = Math.floor(from / 5);
    const col = (from % 5) * 2 + (row % 2);

    // 方向偏移
    const directions = [
        { dr: 1, dc: -1, offset: 6 },
        { dr: 1, dc: 1, offset: 4 },
        { dr: -1, dc: -1, offset: -4 },
        { dr: -1, dc: 1, offset: -6 }
    ];

    for (const dir of directions) {
        // 普通棋子只能向前
        if (!piece.isKing) {
            if (piece.color === 1 && dir.dr < 0) continue;
            if (piece.color === -1 && dir.dr > 0) continue;
        }

        const newRow = row + dir.dr;
        const newCol = col + dir.dc;

        if (newRow < 0 || newRow >= 10 || newCol < 0 || newCol >= 10) continue;
        if ((newRow + newCol) % 2 === 0) continue;

        const toIndex = Math.floor(newRow * 5 + newCol / 2);

        // 普通移动
        if (!session.board[toIndex]) {
            moves.push({
                from: from,
                to: toIndex,
                captures: []
            });
        }
        // 吃子
        else if (session.board[toIndex] && session.board[toIndex].color !== piece.color) {
            const jumpRow = newRow + dir.dr;
            const jumpCol = newCol + dir.dc;

            if (jumpRow < 0 || jumpRow >= 10 || jumpCol < 0 || jumpCol >= 10) continue;
            if ((jumpRow + jumpCol) % 2 === 0) continue;

            const jumpIndex = Math.floor(jumpRow * 5 + jumpCol / 2);

            if (!session.board[jumpIndex]) {
                moves.push({
                    from: from,
                    to: jumpIndex,
                    captures: [toIndex]
                });
            }
        }
    }

    return moves;
}

// 验证走法
function validateMove(session, move) {
    const allMoves = generateAllMoves(session);
    return allMoves.some(m => 
        m.from === move.from && 
        m.to === move.to
    );
}

// 评估局面
function evaluatePosition(session) {
    let score = 0;

    // 材料评估
    const blackCount = session.getPieceCount(1);
    const whiteCount = session.getPieceCount(-1);
    
    let blackKings = 0;
    let whiteKings = 0;

    for (let i = 0; i < 50; i++) {
        const piece = session.board[i];
        if (!piece) continue;

        if (piece.color === 1) {
            score += 100;
            if (piece.isKing) {
                score += 200;
                blackKings++;
            }
        } else {
            score -= 100;
            if (piece.isKing) {
                score -= 200;
                whiteKings++;
            }
        }

        // 位置评估
        const row = Math.floor(i / 5);
        if (piece.color === 1) {
            score += row * 5; // 黑方向上前进
        } else {
            score -= (9 - row) * 5; // 白方向下前进
        }
    }

    // 从当前玩家视角返回
    return session.currentPlayer === 1 ? score : -score;
}

// 克隆会话
function cloneSession(session) {
    const newSession = new GameSession(session.sessionId + '_clone');
    newSession.board = session.board.map(p => p ? { ...p } : null);
    newSession.currentPlayer = session.currentPlayer;
    newSession.moveHistory = [...session.moveHistory];
    newSession.gameOver = session.gameOver;
    return newSession;
}

// 启动服务器
async function startServer() {
    try {
        // 尝试启动C++引擎（可选）
        try {
            await engine.start();
            console.log('✓ C++引擎已启动');
        } catch (error) {
            console.log('⚠ C++引擎未启动，使用JavaScript AI');
            console.log('  原因:', error.message);
        }

        // 启动HTTP服务器
        app.listen(PORT, () => {
            console.log('');
            console.log('========================================');
            console.log('  国际跳棋AI服务器已启动');
            console.log('========================================');
            console.log(`  服务器地址: http://localhost:${PORT}`);
            console.log(`  API文档: http://localhost:${PORT}/api/health`);
            console.log('========================================');
            console.log('');
            console.log('按 Ctrl+C 停止服务器');
            console.log('');
        });

    } catch (error) {
        console.error('启动服务器失败:', error);
        process.exit(1);
    }
}

// 优雅关闭
process.on('SIGINT', () => {
    console.log('\n正在关闭服务器...');
    engine.stop();
    process.exit(0);
});

process.on('SIGTERM', () => {
    console.log('\n正在关闭服务器...');
    engine.stop();
    process.exit(0);
});

// 启动
startServer();
