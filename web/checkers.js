// API配置
const API_BASE_URL = 'http://localhost:3000/api';

// 国际跳棋游戏逻辑
class CheckersGame {
    constructor() {
        this.board = this.initBoard();
        this.currentPlayer = 1; // 1=黑方, -1=白方
        this.selectedSquare = null;
        this.validMoves = [];
        this.moveHistory = [];
        this.gameOver = false;
        this.playerColor = 1; // 玩家默认执黑
        this.aiLevel = 3; // AI思考时间（秒）
        this.sessionId = null; // 服务器会话ID
    }

    // 初始化棋盘
    initBoard() {
        const board = Array(50).fill(null);
        
        // 黑方棋子（前4行，1-20号位）
        for (let i = 0; i < 20; i++) {
            board[i] = { color: 1, isKing: false }; // 1=黑方
        }
        
        // 白方棋子（后4行，31-50号位）
        for (let i = 30; i < 50; i++) {
            board[i] = { color: -1, isKing: false }; // -1=白方
        }
        
        return board;
    }

    // 获取格子索引（从2D坐标转换）
    // 国际跳棋：黑格编号1-50，对应数组索引0-49
    getSquareIndex(row, col) {
        // 只有黑格有效（row + col为奇数）
        if ((row + col) % 2 === 0) return -1;
        
        // 计算该行的第几个黑格（0-4）
        const blackSquareInRow = Math.floor(col / 2);
        // 返回索引（0-49）
        return row * 5 + blackSquareInRow;
    }

    // 从索引获取2D坐标
    getSquareCoords(index) {
        const row = Math.floor(index / 5);
        const blackSquareInRow = index % 5;
        // 偶数行（0,2,4,6,8）：黑格在列1,3,5,7,9
        // 奇数行（1,3,5,7,9）：黑格在列0,2,4,6,8
        const col = (row % 2 === 0) ? (blackSquareInRow * 2 + 1) : (blackSquareInRow * 2);
        return { row, col };
    }

    // 获取棋子数量
    getPieceCount(color) {
        return this.board.filter(piece => piece && piece.color === color).length;
    }

    // 生成所有合法走法（简化版）
    generateMoves(color) {
        const moves = [];
        
        // 遍历所有己方棋子
        for (let from = 0; from < 50; from++) {
            const piece = this.board[from];
            if (!piece || piece.color !== color) continue;
            
            // 获取该棋子的所有走法
            const pieceMoves = this.generatePieceMoves(from, piece);
            moves.push(...pieceMoves);
        }
        
        // 检查是否有吃子走法（强制吃子规则）
        const captureMoves = moves.filter(m => m.captures && m.captures.length > 0);
        if (captureMoves.length > 0) {
            // 只返回吃子走法，并且是吃子最多的
            const maxCaptures = Math.max(...captureMoves.map(m => m.captures.length));
            return captureMoves.filter(m => m.captures.length === maxCaptures);
        }
        
        return moves;
    }

    // 生成单个棋子的走法
    generatePieceMoves(from, piece) {
        const moves = [];
        const { row, col } = this.getSquareCoords(from);
        
        console.log(`生成走法 - 索引:${from}, 编号:${from+1}, 坐标:(${row},${col}), 棋子:`, piece);
        
        // 先检查是否有吃子机会
        const captureMoves = this.generateCaptureMoves(from, piece, row, col, [], new Set());
        
        console.log(`  吃子走法数量: ${captureMoves.length}`);
        
        // 如果有吃子机会，只返回吃子走法（强制吃子规则）
        if (captureMoves.length > 0) {
            return captureMoves;
        }
        
        // 没有吃子机会，生成普通移动
        // 四个对角线方向
        const directions = [
            { dr: -1, dc: -1 }, // 左上
            { dr: -1, dc: 1 },  // 右上
            { dr: 1, dc: -1 },  // 左下
            { dr: 1, dc: 1 }    // 右下
        ];
        
        for (const dir of directions) {
            // 普通棋子非吃子时只能向前
            if (!piece.isKing) {
                // 黑方（color=1）在顶部，向下移动（row增大，dr>0）
                // 白方（color=-1）在底部，向上移动（row减小，dr<0）
                if (piece.color === 1 && dir.dr < 0) {
                    console.log(`  跳过方向(${dir.dr},${dir.dc}) - 黑方不能向上`);
                    continue;
                }
                if (piece.color === -1 && dir.dr > 0) {
                    console.log(`  跳过方向(${dir.dr},${dir.dc}) - 白方不能向下`);
                    continue;
                }
            }
            
            if (piece.isKing) {
                // 王可以移动任意距离
                for (let dist = 1; dist < 10; dist++) {
                    const newRow = row + dir.dr * dist;
                    const newCol = col + dir.dc * dist;
                    
                    if (newRow < 0 || newRow >= 10 || newCol < 0 || newCol >= 10) break;
                    
                    const toIndex = this.getSquareIndex(newRow, newCol);
                    if (toIndex === -1) break;
                    
                    if (this.board[toIndex]) break; // 遇到棋子停止
                    
                    moves.push({
                        from: from,
                        to: toIndex,
                        captures: []
                    });
                }
            } else {
                // 普通棋子只能移动一格
                const newRow = row + dir.dr;
                const newCol = col + dir.dc;
                
                console.log(`  尝试方向(${dir.dr},${dir.dc}): (${row},${col}) -> (${newRow},${newCol})`);
                
                if (newRow < 0 || newRow >= 10 || newCol < 0 || newCol >= 10) {
                    console.log(`    越界`);
                    continue;
                }
                
                const toIndex = this.getSquareIndex(newRow, newCol);
                console.log(`    目标索引: ${toIndex}`);
                
                if (toIndex === -1) {
                    console.log(`    白格，无效`);
                    continue;
                }
                
                if (!this.board[toIndex]) {
                    console.log(`    空格，可以移动！`);
                    moves.push({
                        from: from,
                        to: toIndex,
                        captures: []
                    });
                } else {
                    console.log(`    有棋子，不能移动`);
                }
            }
        }
        
        console.log(`  普通走法数量: ${moves.length}`);
        return moves;
    }
    
    // 生成吃子走法（支持连续跳吃）
    generateCaptureMoves(from, piece, row, col, capturedSoFar, visitedSquares) {
        const moves = [];
        const directions = [
            { dr: -1, dc: -1 }, // 左上
            { dr: -1, dc: 1 },  // 右上
            { dr: 1, dc: -1 },  // 左下
            { dr: 1, dc: 1 }    // 右下
        ];
        
        let foundCapture = false;
        
        for (const dir of directions) {
            if (piece.isKing) {
                // 王的吃子：可以跳过任意距离的对手棋子
                for (let dist = 1; dist < 10; dist++) {
                    const enemyRow = row + dir.dr * dist;
                    const enemyCol = col + dir.dc * dist;
                    
                    if (enemyRow < 0 || enemyRow >= 10 || enemyCol < 0 || enemyCol >= 10) break;
                    
                    const enemyIndex = this.getSquareIndex(enemyRow, enemyCol);
                    if (enemyIndex === -1) break;
                    
                    const enemyPiece = this.board[enemyIndex];
                    
                    // 遇到己方棋子或已吃掉的棋子，停止
                    if (enemyPiece && enemyPiece.color === piece.color) break;
                    if (capturedSoFar.includes(enemyIndex)) break;
                    
                    // 遇到对方棋子
                    if (enemyPiece && enemyPiece.color !== piece.color) {
                        // 检查跳过后的落点
                        for (let landDist = 1; landDist < 10; landDist++) {
                            const landRow = enemyRow + dir.dr * landDist;
                            const landCol = enemyCol + dir.dc * landDist;
                            
                            if (landRow < 0 || landRow >= 10 || landCol < 0 || landCol >= 10) break;
                            
                            const landIndex = this.getSquareIndex(landRow, landCol);
                            if (landIndex === -1) break;
                            
                            // 落点必须为空且未访问过
                            if (this.board[landIndex] || visitedSquares.has(landIndex)) break;
                            if (capturedSoFar.includes(landIndex)) break;
                            
                            foundCapture = true;
                            const newCaptured = [...capturedSoFar, enemyIndex];
                            const newVisited = new Set(visitedSquares);
                            newVisited.add(from);
                            
                            // 尝试继续跳吃
                            const furtherMoves = this.generateCaptureMoves(
                                landIndex, piece, landRow, landCol, newCaptured, newVisited
                            );
                            
                            if (furtherMoves.length > 0) {
                                moves.push(...furtherMoves);
                            } else {
                                moves.push({
                                    from: from,
                                    to: landIndex,
                                    captures: newCaptured
                                });
                            }
                        }
                        break; // 遇到对手棋子后停止继续搜索
                    }
                }
            } else {
                // 普通棋子的吃子：只能跳一格
                const enemyRow = row + dir.dr;
                const enemyCol = col + dir.dc;
                
                if (enemyRow < 0 || enemyRow >= 10 || enemyCol < 0 || enemyCol >= 10) continue;
                
                const enemyIndex = this.getSquareIndex(enemyRow, enemyCol);
                if (enemyIndex === -1) continue;
                
                const enemyPiece = this.board[enemyIndex];
                
                // 必须是对方棋子且未被吃掉
                if (!enemyPiece || enemyPiece.color === piece.color) continue;
                if (capturedSoFar.includes(enemyIndex)) continue;
                
                // 检查落点
                const landRow = enemyRow + dir.dr;
                const landCol = enemyCol + dir.dc;
                
                if (landRow < 0 || landRow >= 10 || landCol < 0 || landCol >= 10) continue;
                
                const landIndex = this.getSquareIndex(landRow, landCol);
                if (landIndex === -1) continue;
                
                // 落点必须为空且未访问过
                if (this.board[landIndex] || visitedSquares.has(landIndex)) continue;
                if (capturedSoFar.includes(landIndex)) continue;
                
                foundCapture = true;
                const newCaptured = [...capturedSoFar, enemyIndex];
                const newVisited = new Set(visitedSquares);
                newVisited.add(from);
                
                // 尝试继续跳吃
                const furtherMoves = this.generateCaptureMoves(
                    landIndex, piece, landRow, landCol, newCaptured, newVisited
                );
                
                if (furtherMoves.length > 0) {
                    moves.push(...furtherMoves);
                } else {
                    moves.push({
                        from: from,
                        to: landIndex,
                        captures: newCaptured
                    });
                }
            }
        }
        
        return moves;
    }

    // 执行走法
    makeMove(move) {
        if (this.gameOver) return false;
        
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
        const { row } = this.getSquareCoords(move.to);
        // 黑方到达第0行（顶部），白方到达第9行（底部）
        if (!piece.isKing) {
            if (piece.color === 1 && row === 0) {
                piece.isKing = true;
            } else if (piece.color === -1 && row === 9) {
                piece.isKing = true;
            }
        }
        
        // 记录走法
        this.moveHistory.push({
            move: move,
            player: this.currentPlayer,
            notation: this.moveToNotation(move)
        });
        
        // 切换玩家
        this.currentPlayer = -this.currentPlayer;
        
        // 检查游戏结束
        this.checkGameOver();
        
        return true;
    }

    // 走法转换为记谱法
    moveToNotation(move) {
        let notation = `${move.from + 1}`;
        
        if (move.captures && move.captures.length > 0) {
            notation += 'x';
            for (const capture of move.captures) {
                notation += `${capture + 1}x`;
            }
        } else {
            notation += '-';
        }
        
        notation += `${move.to + 1}`;
        return notation;
    }

    // 检查游戏是否结束
    checkGameOver() {
        const moves = this.generateMoves(this.currentPlayer);
        
        if (moves.length === 0) {
            this.gameOver = true;
            return this.currentPlayer === 1 ? -1 : 1; // 返回获胜方
        }
        
        const blackCount = this.getPieceCount(1);
        const whiteCount = this.getPieceCount(-1);
        
        if (blackCount === 0) {
            this.gameOver = true;
            return -1; // 白方获胜
        }
        
        if (whiteCount === 0) {
            this.gameOver = true;
            return 1; // 黑方获胜
        }
        
        return 0; // 游戏继续
    }

    // 重置游戏
    async reset() {
        this.board = this.initBoard();
        this.currentPlayer = 1;
        this.selectedSquare = null;
        this.validMoves = [];
        this.moveHistory = [];
        this.gameOver = false;
        
        // 创建新的服务器会话
        try {
            const response = await fetch(`${API_BASE_URL}/game/new`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' }
            });
            const data = await response.json();
            if (data.success) {
                this.sessionId = data.sessionId;
                console.log('新游戏会话已创建:', this.sessionId);
            }
        } catch (error) {
            console.error('创建游戏会话失败:', error);
            // 即使服务器不可用，也可以继续游戏
        }
    }
}

// UI控制器
class CheckersUI {
    constructor() {
        this.game = new CheckersGame();
        this.boardElement = document.getElementById('board');
        this.messageElement = document.getElementById('message');
        this.aiThinking = false;
        
        this.initBoard();
        this.initControls();
        this.updateUI();
    }

    // 初始化棋盘UI
    initBoard() {
        this.boardElement.innerHTML = '';
        
        for (let row = 0; row < 10; row++) {
            for (let col = 0; col < 10; col++) {
                const square = document.createElement('div');
                square.className = 'square';
                
                // 设置格子颜色
                // 黑格：row + col 为奇数
                if ((row + col) % 2 === 1) {
                    square.classList.add('dark');
                    
                    const index = this.game.getSquareIndex(row, col);
                    square.dataset.index = index;
                    
                    square.addEventListener('click', () => this.handleSquareClick(index));
                } else {
                    square.classList.add('light');
                }
                
                this.boardElement.appendChild(square);
            }
        }
    }

    // 初始化控制按钮
    initControls() {
        document.getElementById('newGameBtn').addEventListener('click', () => this.newGame());
        document.getElementById('undoBtn').addEventListener('click', () => this.undo());
        document.getElementById('hintBtn').addEventListener('click', () => this.showHint());
        
        document.getElementById('playerColor').addEventListener('change', (e) => {
            this.game.playerColor = e.target.value === 'black' ? 1 : -1;
            this.newGame();
        });
        
        document.getElementById('aiLevel').addEventListener('change', (e) => {
            this.game.aiLevel = parseInt(e.target.value);
        });
    }

    // 处理格子点击
    handleSquareClick(index) {
        if (this.game.gameOver || this.aiThinking) return;
        
        // 如果不是玩家回合，不响应
        if (this.game.currentPlayer !== this.game.playerColor) return;
        
        const piece = this.game.board[index];
        
        // 选择己方棋子
        if (piece && piece.color === this.game.currentPlayer) {
            this.selectSquare(index);
        }
        // 移动到目标格子
        else if (this.game.selectedSquare !== null) {
            const move = this.game.validMoves.find(m => m.to === index);
            if (move) {
                this.makeMove(move);
            }
        }
    }

    // 选择格子
    selectSquare(index) {
        this.game.selectedSquare = index;
        
        // 生成该棋子的合法走法
        const piece = this.game.board[index];
        console.log('=== 选择棋子 ===');
        console.log('索引:', index, '编号:', index + 1);
        console.log('棋子:', piece);
        console.log('当前玩家:', this.game.currentPlayer);
        
        const allMoves = this.game.generateMoves(this.game.currentPlayer);
        console.log('所有合法走法:', allMoves.length);
        
        this.game.validMoves = allMoves.filter(m => m.from === index);
        console.log('该棋子的走法:', this.game.validMoves);
        
        // 显示可移动位置的提示消息
        if (this.game.validMoves.length > 0) {
            const movePositions = this.game.validMoves.map(m => m.to + 1).join(', ');
            const hasCaptures = this.game.validMoves.some(m => m.captures && m.captures.length > 0);
            const moveType = hasCaptures ? '\u5403\u5b50' : '\u79fb\u52a8'; // "吃子" : "移动"
            this.showMessage(`\ud83d\udca1 \u53ef${moveType}\u5230\u4f4d\u7f6e: ${movePositions}`, 'info'); // "💡 可...到位置:"
        } else {
            this.showMessage('\u26a0\ufe0f \u8be5\u68cb\u5b50\u65e0\u6cd5\u79fb\u52a8', 'warning'); // "⚠️ 该棋子无法移动"
        }
        
        this.updateBoard();
    }

    // 执行走法
    makeMove(move) {
        if (this.game.makeMove(move)) {
            this.game.selectedSquare = null;
            this.game.validMoves = [];
            this.updateUI();
            
            // 如果游戏未结束且轮到AI，让AI思考
            if (!this.game.gameOver && this.game.currentPlayer !== this.game.playerColor) {
                setTimeout(() => this.aiMove(), 500);
            }
        }
    }

    // AI走棋
    async aiMove() {
        console.log('=== AI开始思考 ===');
        console.log('当前玩家:', this.game.currentPlayer);
        console.log('AI颜色:', -this.game.playerColor);
        
        this.aiThinking = true;
        this.showMessage('\ud83e\udd16 AI\u6b63\u5728\u601d\u8003...', 'info'); // "🤖 AI正在思考..."
        document.getElementById('aiThinking').classList.add('active');
        
        try {
            const startTime = Date.now();
            
            console.log('尝试调用后端API...');
            // 调用后端API获取AI走法
            const response = await fetch(`${API_BASE_URL}/game/ai-move`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    sessionId: this.game.sessionId,
                    timeLimit: this.game.aiLevel * 1000
                })
            });
            
            const data = await response.json();
            console.log('后端响应:', data);
            
            if (!data.success) {
                throw new Error(data.error || 'AI\u8d70\u6cd5\u5931\u8d25'); // "AI走法失败"
            }
            
            const thinkTime = Date.now() - startTime;
            
            // 更新AI思考信息
            document.getElementById('searchDepth').textContent = data.aiInfo.searchDepth || '-';
            document.getElementById('nodesSearched').textContent = 
                (data.aiInfo.nodesSearched || 0).toLocaleString();
            document.getElementById('evalScore').textContent = 
                (data.aiInfo.evalScore > 0 ? '+' : '') + (data.aiInfo.evalScore || 0).toFixed(2);
            document.getElementById('thinkTime').textContent = `${thinkTime}ms`;
            
            // 更新游戏状态
            this.game.board = data.board;
            this.game.currentPlayer = data.currentPlayer;
            this.game.moveHistory.push({
                move: data.move,
                player: -this.game.currentPlayer,
                notation: this.game.moveToNotation(data.move)
            });
            
        } catch (error) {
            console.error('AI\u8d70\u6cd5\u9519\u8bef:', error); // "AI走法错误"
            console.log('\u964d\u7ea7\u5230\u672c\u5730AI...'); // "降级到本地AI..."
            
            // 降级到本地AI
            await this.localAIMove();
        }
        
        this.aiThinking = false;
        document.getElementById('aiThinking').classList.remove('active');
        this.updateUI();
        
        console.log('=== AI思考完成 ===');
    }
    
    // 本地AI走法（备用）
    async localAIMove() {
        console.log('=== 本地AI开始 ===');
        await this.simulateAIThinking();
        
        const moves = this.game.generateMoves(this.game.currentPlayer);
        console.log('本地AI生成的走法数量:', moves.length);
        
        if (moves.length > 0) {
            const randomMove = moves[Math.floor(Math.random() * moves.length)];
            console.log('本地AI选择的走法:', randomMove);
            
            const success = this.game.makeMove(randomMove);
            console.log('走法执行结果:', success);
            
            document.getElementById('searchDepth').textContent = '\u672c\u5730AI'; // "本地AI"
            document.getElementById('nodesSearched').textContent = moves.length;
            document.getElementById('evalScore').textContent = '0.0';
            document.getElementById('thinkTime').textContent = `${this.game.aiLevel * 1000}ms`;
        } else {
            console.error('本地AI没有合法走法！');
            this.showMessage('\u274c AI\u65e0\u6cd5\u79fb\u52a8\uff0c\u6e38\u620f\u7ed3\u675f', 'error'); // "❌ AI无法移动，游戏结束"
            this.game.gameOver = true;
        }
        
        console.log('=== 本地AI完成 ===');
    }

    // 模拟AI思考延迟
    simulateAIThinking() {
        return new Promise(resolve => {
            setTimeout(resolve, this.game.aiLevel * 1000);
        });
    }

    // 更新UI
    updateUI() {
        this.updateBoard();
        this.updateStatus();
        this.updateHistory();
        this.checkGameStatus();
    }

    // 更新棋盘显示
    updateBoard() {
        const squares = this.boardElement.querySelectorAll('.square.dark');
        
        squares.forEach(square => {
            const index = parseInt(square.dataset.index);
            square.innerHTML = '';
            square.classList.remove('selected', 'valid-move', 'capture', 'can-select');
            
            // 显示棋子
            const piece = this.game.board[index];
            if (piece) {
                const pieceElement = document.createElement('div');
                pieceElement.className = `piece ${piece.color === 1 ? 'black' : 'white'}`;
                if (piece.isKing) {
                    pieceElement.classList.add('king');
                }
                square.appendChild(pieceElement);
                
                // 如果是当前玩家的棋子，添加可选择的视觉提示
                if (piece.color === this.game.currentPlayer && 
                    this.game.currentPlayer === this.game.playerColor &&
                    !this.aiThinking && !this.game.gameOver) {
                    square.classList.add('can-select');
                }
            }
            
            // 高亮选中的格子
            if (index === this.game.selectedSquare) {
                square.classList.add('selected');
            }
            
            // 高亮可移动的格子，并区分普通移动和吃子
            const validMove = this.game.validMoves.find(m => m.to === index);
            if (validMove) {
                square.classList.add('valid-move');
                
                // 创建移动提示容器
                const moveIndicator = document.createElement('div');
                moveIndicator.className = 'move-indicator';
                
                // 如果是吃子走法，添加capture类和特殊标记
                if (validMove.captures && validMove.captures.length > 0) {
                    square.classList.add('capture');
                    moveIndicator.innerHTML = `
                        <div class="move-icon capture-icon">\u00d7</div>
                        <div class="move-label">\u4f4d\u7f6e ${index + 1}</div>
                    `;
                } else {
                    // 普通移动
                    moveIndicator.innerHTML = `
                        <div class="move-icon move-icon-normal">\u2713</div>
                        <div class="move-label">\u4f4d\u7f6e ${index + 1}</div>
                    `;
                }
                
                square.appendChild(moveIndicator);
            }
        });
    }

    // 更新状态显示
    updateStatus() {
        document.getElementById('currentPlayer').textContent = 
            this.game.currentPlayer === 1 ? '\u9ed1\u65b9' : '\u767d\u65b9'; // "黑方" : "白方"
        document.getElementById('blackPieces').textContent = 
            this.game.getPieceCount(1);
        document.getElementById('whitePieces').textContent = 
            this.game.getPieceCount(-1);
        document.getElementById('moveCount').textContent = 
            this.game.moveHistory.length;
    }

    // 更新走法历史
    updateHistory() {
        const historyElement = document.getElementById('moveHistory');
        
        if (this.game.moveHistory.length === 0) {
            historyElement.innerHTML = '<p class="empty-history">\u6682\u65e0\u8d70\u6cd5\u8bb0\u5f55</p>'; // "暂无走法记录"
            return;
        }
        
        historyElement.innerHTML = '';
        
        this.game.moveHistory.forEach((entry, index) => {
            const moveItem = document.createElement('div');
            moveItem.className = `move-item ${entry.player === 1 ? 'black' : 'white'}`;
            const playerName = entry.player === 1 ? '\u9ed1\u65b9' : '\u767d\u65b9'; // "黑方" : "白方"
            moveItem.innerHTML = `
                <span class="move-number">${index + 1}.</span>
                <span>${playerName}: ${entry.notation}</span>
            `;
            historyElement.appendChild(moveItem);
        });
        
        // 滚动到底部
        historyElement.scrollTop = historyElement.scrollHeight;
    }

    // 检查游戏状态
    checkGameStatus() {
        if (this.game.gameOver) {
            const winner = this.game.checkGameOver();
            if (winner === 1) {
                this.showMessage('\ud83c\udf89 \u9ed1\u65b9\u83b7\u80dc\uff01', 'success'); // "🎉 黑方获胜！"
            } else if (winner === -1) {
                this.showMessage('\ud83c\udf89 \u767d\u65b9\u83b7\u80dc\uff01', 'success'); // "🎉 白方获胜！"
            } else {
                this.showMessage('\ud83e\udd1d \u548c\u68cb\uff01', 'info'); // "🤝 和棋！"
            }
        } else {
            const player = this.game.currentPlayer === 1 ? '\u9ed1\u65b9' : '\u767d\u65b9'; // "黑方" : "白方"
            this.showMessage(`\u8f6e\u5230${player}\u8d70\u68cb`, 'info'); // "轮到...走棋"
        }
    }

    // 显示消息
    showMessage(text, type = 'info') {
        this.messageElement.textContent = text;
        this.messageElement.className = `message ${type}`;
    }

    // 新游戏
    async newGame() {
        await this.game.reset();
        this.updateUI();
        this.showMessage('\ud83c\udfae \u65b0\u6e38\u620f\u5f00\u59cb\uff01', 'success'); // "🎮 新游戏开始！"
        
        // 如果AI先手，让AI走第一步
        if (this.game.playerColor === -1) {
            setTimeout(() => this.aiMove(), 500);
        }
    }

    // 悔棋
    undo() {
        if (this.game.moveHistory.length === 0) {
            this.showMessage('\u6ca1\u6709\u53ef\u4ee5\u6094\u68cb\u7684\u8d70\u6cd5', 'warning'); // "没有可以悔棋的走法"
            return;
        }
        
        // 简化实现：重新开始游戏
        this.showMessage('\u6094\u68cb\u529f\u80fd\u5f00\u53d1\u4e2d...', 'warning'); // "悔棋功能开发中..."
    }

    // 提示
    async showHint() {
        if (this.game.gameOver || this.aiThinking) return;
        
        try {
            // 尝试从服务器获取提示
            const response = await fetch(`${API_BASE_URL}/game/hint`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    sessionId: this.game.sessionId
                })
            });
            
            const data = await response.json();
            
            if (data.success && data.hint) {
                const hint = data.hint;
                this.showMessage(`\ud83d\udca1 \u63d0\u793a\uff1a\u4ece ${hint.from + 1} \u79fb\u52a8\u5230 ${hint.to + 1}`, 'info'); // "💡 提示：从 ... 移动到 ..."
                
                // 高亮提示走法
                this.game.selectedSquare = hint.from;
                this.game.validMoves = [hint];
                this.updateBoard();
                return;
            }
        } catch (error) {
            console.error('\u83b7\u53d6\u63d0\u793a\u5931\u8d25:', error); // "获取提示失败"
        }
        
        // 降级到本地提示
        const moves = this.game.generateMoves(this.game.currentPlayer);
        if (moves.length > 0) {
            const hint = moves[0];
            this.showMessage(`\ud83d\udca1 \u63d0\u793a\uff1a\u4ece ${hint.from + 1} \u79fb\u52a8\u5230 ${hint.to + 1}`, 'info'); // "💡 提示：从 ... 移动到 ..."
            
            // 高亮提示走法
            this.game.selectedSquare = hint.from;
            this.game.validMoves = [hint];
            this.updateBoard();
        }
    }
}

// 页面加载完成后初始化
document.addEventListener('DOMContentLoaded', () => {
    const ui = new CheckersUI();
});
