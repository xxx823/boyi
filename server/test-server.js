// 简化的测试服务器 - 用于验证基本功能
const express = require('express');
const cors = require('cors');
const path = require('path');

const app = express();
const PORT = 3000;

// 中间件
app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, '../web')));

// 测试路由
app.get('/api/health', (req, res) => {
    res.json({
        status: 'ok',
        message: 'Server is running',
        timestamp: new Date().toISOString()
    });
});

// 启动服务器
app.listen(PORT, () => {
    console.log('');
    console.log('========================================');
    console.log('  Test Server Started');
    console.log('========================================');
    console.log(`  URL: http://localhost:${PORT}`);
    console.log(`  Health: http://localhost:${PORT}/api/health`);
    console.log('========================================');
    console.log('');
    console.log('Press Ctrl+C to stop');
    console.log('');
});

// 错误处理
app.use((err, req, res, next) => {
    console.error('Error:', err);
    res.status(500).json({
        error: err.message
    });
});

process.on('SIGINT', () => {
    console.log('\nShutting down...');
    process.exit(0);
});
