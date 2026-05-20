// 国际跳棋Bug修复验证脚本
// 可以在浏览器控制台中运行

console.log('=== 国际跳棋Bug修复验证 ===\n');

// 创建游戏实例
const game = new CheckersGame();

// 测试1: 验证from字段正确
console.log('测试1: 验证连续跳吃from字段');
game.board = Array(50).fill(null);
game.board[10] = { color: 1, isKing: false };
game.board[14] = { color: -1, isKing: false };
game.board[23] = { color: -1, isKing: false };
game.board[32] = { color: -1, isKing: false };
game.currentPlayer = 1;

const moves1 = game.generateMoves(1);
console.log(`  生成走法数: ${moves1.length}`);
console.log(`  第一个走法from: ${moves1[0]?.from} (应该是10)`);
console.log(`  第一个走法to: ${moves1[0]?.to}`);
console.log(`  吃子数: ${moves1[0]?.captures.length}`);
console.log(`  ✓ 测试1: ${moves1[0]?.from === 10 ? '通过' : '失败'}\n`);

// 测试2: 验证路径可以交叉（钻石阵地）
console.log('测试2: 验证路径交叉（钻石阵地5连跳）');
game.board = Array(50).fill(null);
game.board[24] = { color: 1, isKing: false };
game.board[28] = { color: -1, isKing: false };
game.board[37] = { color: -1, isKing: false };
game.board[36] = { color: -1, isKing: false };
game.board[27] = { color: -1, isKing: false };
game.board[33] = { color: -1, isKing: false };
game.currentPlayer = 1;

const moves2 = game.generateMoves(1);
const maxCaptures = Math.max(...moves2.map(m => m.captures.length));
console.log(`  生成走法数: ${moves2.length}`);
console.log(`  最大吃子数: ${maxCaptures} (应该是5)`);
console.log(`  所有走法:`);
moves2.forEach((m, i) => {
    console.log(`    ${i}: from=${m.from}, to=${m.to}, captures=${m.captures.length}个 [${m.captures.join(',')}]`);
});
console.log(`  ✓ 测试2: ${maxCaptures >= 5 ? '通过' : '失败'}\n`);

// 测试3: 验证多个棋子能吃相同数量时都被保留
console.log('测试3: 验证多个棋子吃相同数量');
game.board = Array(50).fill(null);
game.board[15] = { color: 1, isKing: false };
game.board[19] = { color: -1, isKing: false };
game.board[28] = { color: -1, isKing: false };
game.board[37] = { color: -1, isKing: false };

game.board[16] = { color: 1, isKing: false };
game.board[20] = { color: -1, isKing: false };
game.board[29] = { color: -1, isKing: false };
game.board[38] = { color: -1, isKing: false };
game.currentPlayer = 1;

const moves3 = game.generateMoves(1);
const piece1Moves = moves3.filter(m => m.from === 15);
const piece2Moves = moves3.filter(m => m.from === 16);
console.log(`  生成走法数: ${moves3.length}`);
console.log(`  棋子1(位置15)的走法数: ${piece1Moves.length}`);
console.log(`  棋子2(位置16)的走法数: ${piece2Moves.length}`);
console.log(`  ✓ 测试3: ${piece1Moves.length > 0 && piece2Moves.length > 0 ? '通过' : '失败'}\n`);

// 测试4: 验证屏障规则仍然有效
console.log('测试4: 验证屏障规则（不能跳过同一个被吃的棋子两次）');
game.board = Array(50).fill(null);
game.board[10] = { color: 1, isKing: false };
game.board[14] = { color: -1, isKing: false };
game.board[23] = { color: -1, isKing: false };
game.currentPlayer = 1;

const moves4 = game.generateMoves(1);
console.log(`  生成走法数: ${moves4.length}`);
console.log(`  第一个走法吃子: [${moves4[0]?.captures.join(',')}]`);

// 检查是否有重复吃子
let hasBarrierViolation = false;
for (const move of moves4) {
    const uniqueCaptures = new Set(move.captures);
    if (uniqueCaptures.size !== move.captures.length) {
        hasBarrierViolation = true;
        console.log(`  ✗ 发现违反屏障规则的走法: ${move.captures}`);
    }
}
console.log(`  ✓ 测试4: ${!hasBarrierViolation ? '通过' : '失败'}\n`);

// 总结
console.log('=== 验证总结 ===');
const test1Pass = moves1[0]?.from === 10;
const test2Pass = maxCaptures >= 5;
const test3Pass = piece1Moves.length > 0 && piece2Moves.length > 0;
const test4Pass = !hasBarrierViolation;

const allPass = test1Pass && test2Pass && test3Pass && test4Pass;

console.log(`测试1 (from字段): ${test1Pass ? '✓ 通过' : '✗ 失败'}`);
console.log(`测试2 (路径交叉): ${test2Pass ? '✓ 通过' : '✗ 失败'}`);
console.log(`测试3 (多棋子): ${test3Pass ? '✓ 通过' : '✗ 失败'}`);
console.log(`测试4 (屏障规则): ${test4Pass ? '✓ 通过' : '✗ 失败'}`);
console.log(`\n总体结果: ${allPass ? '✅ 所有测试通过' : '❌ 有测试失败'}`);

if (allPass) {
    console.log('\n🎉 恭喜！所有bug已成功修复！');
    console.log('✅ from字段正确指向原始起点');
    console.log('✅ 路径可以交叉，支持复杂连跳');
    console.log('✅ 多个棋子能吃相同数量时都能移动');
    console.log('✅ 屏障规则正确实施');
} else {
    console.log('\n⚠️ 仍有问题需要修复，请检查失败的测试。');
}
