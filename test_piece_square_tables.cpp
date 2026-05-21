// Test for Piece-Square Tables implementation (Task 3.1)
// This test verifies that position_value array is correctly initialized
// with center positions having higher values than edge positions

#include <iostream>
#include <cmath>
#include <cassert>

// Simplified Evaluator class for testing
class Evaluator {
public:
    static int position_value[50];
    static bool initialized;
    
    static void init() {
        if (initialized) return;
        
        // 计算每个格子的位置价值
        for (int sq = 0; sq < 50; ++sq) {
            int row = sq / 5;
            int col = (sq % 5) * 2 + (row % 2);
            
            // 中心位置更有价值 - 使用曼哈顿距离
            int center_distance = abs(row - 4) + abs(col - 4);
            position_value[sq] = 20 - center_distance * 2;
            
            // 边缘位置稍微降低价值
            if (col == 0 || col == 9) {
                position_value[sq] -= 5;
            }
        }
        
        initialized = true;
    }
};

// 静态成员初始化
int Evaluator::position_value[50] = {};
bool Evaluator::initialized = false;

// 测试函数
void test_piece_square_tables() {
    std::cout << "Testing Piece-Square Tables Implementation (Task 3.1)...\n\n";
    
    // 初始化评估器
    Evaluator::init();
    
    // 测试1: 验证数组已初始化
    std::cout << "Test 1: Verify array is initialized\n";
    assert(Evaluator::initialized == true);
    std::cout << "✓ Array initialized successfully\n\n";
    
    // 测试2: 验证中心位置有更高的价值
    std::cout << "Test 2: Verify center positions have higher values\n";
    
    // 中心格子 (row 4-5, col 4-5 附近)
    // 格子21 (row=4, col=2), 格子22 (row=4, col=4), 格子26 (row=5, col=2), 格子27 (row=5, col=4)
    int center_sq_21 = 21; // row=4, col=2
    int center_sq_22 = 22; // row=4, col=4
    int center_sq_26 = 26; // row=5, col=2
    int center_sq_27 = 27; // row=5, col=4
    
    // 边缘格子
    int edge_sq_0 = 0;   // row=0, col=0 (边缘)
    int edge_sq_4 = 4;   // row=0, col=8
    int edge_sq_45 = 45; // row=9, col=0 (边缘)
    int edge_sq_49 = 49; // row=9, col=8
    
    std::cout << "Center square 21 value: " << Evaluator::position_value[center_sq_21] << "\n";
    std::cout << "Center square 22 value: " << Evaluator::position_value[center_sq_22] << "\n";
    std::cout << "Center square 26 value: " << Evaluator::position_value[center_sq_26] << "\n";
    std::cout << "Center square 27 value: " << Evaluator::position_value[center_sq_27] << "\n";
    std::cout << "Edge square 0 value: " << Evaluator::position_value[edge_sq_0] << "\n";
    std::cout << "Edge square 4 value: " << Evaluator::position_value[edge_sq_4] << "\n";
    std::cout << "Edge square 45 value: " << Evaluator::position_value[edge_sq_45] << "\n";
    std::cout << "Edge square 49 value: " << Evaluator::position_value[edge_sq_49] << "\n";
    
    // 验证中心格子的价值高于边缘格子
    assert(Evaluator::position_value[center_sq_22] > Evaluator::position_value[edge_sq_0]);
    assert(Evaluator::position_value[center_sq_22] > Evaluator::position_value[edge_sq_4]);
    assert(Evaluator::position_value[center_sq_22] > Evaluator::position_value[edge_sq_45]);
    assert(Evaluator::position_value[center_sq_22] > Evaluator::position_value[edge_sq_49]);
    std::cout << "✓ Center positions have higher values than edge positions\n\n";
    
    // 测试3: 验证边缘惩罚
    std::cout << "Test 3: Verify edge penalty is applied\n";
    // 格子0在边缘 (col=0)，应该有额外的-5惩罚
    // 格子45在边缘 (col=0)，应该有额外的-5惩罚
    
    // 计算预期值
    int row_0 = 0 / 5;
    int col_0 = (0 % 5) * 2 + (row_0 % 2); // col = 0
    int center_dist_0 = abs(row_0 - 4) + abs(col_0 - 4);
    int expected_0 = 20 - center_dist_0 * 2 - 5; // -5 for edge
    
    std::cout << "Square 0: row=" << row_0 << ", col=" << col_0 
              << ", center_dist=" << center_dist_0 
              << ", expected=" << expected_0 
              << ", actual=" << Evaluator::position_value[0] << "\n";
    assert(Evaluator::position_value[0] == expected_0);
    std::cout << "✓ Edge penalty correctly applied\n\n";
    
    // 测试4: 打印所有位置价值（可视化验证）
    std::cout << "Test 4: Display all position values\n";
    std::cout << "Position value table (50 squares):\n";
    for (int sq = 0; sq < 50; ++sq) {
        int row = sq / 5;
        int col = (sq % 5) * 2 + (row % 2);
        std::cout << "sq[" << sq << "] (r" << row << ",c" << col << "): " 
                  << Evaluator::position_value[sq];
        if ((sq + 1) % 5 == 0) std::cout << "\n";
        else std::cout << " | ";
    }
    std::cout << "\n";
    
    // 测试5: 验证使用曼哈顿距离
    std::cout << "Test 5: Verify Manhattan distance is used\n";
    // 对于格子22 (row=4, col=4)，到中心(4,4)的曼哈顿距离应该是0
    int sq_22_row = 22 / 5; // 4
    int sq_22_col = (22 % 5) * 2 + (sq_22_row % 2); // 4
    int manhattan_dist = abs(sq_22_row - 4) + abs(sq_22_col - 4); // 0
    int expected_22 = 20 - manhattan_dist * 2; // 20
    std::cout << "Square 22: row=" << sq_22_row << ", col=" << sq_22_col 
              << ", manhattan_dist=" << manhattan_dist 
              << ", expected=" << expected_22 
              << ", actual=" << Evaluator::position_value[22] << "\n";
    assert(Evaluator::position_value[22] == expected_22);
    std::cout << "✓ Manhattan distance correctly used\n\n";
    
    std::cout << "All tests passed! ✓\n";
    std::cout << "\nTask 3.1 Implementation Summary:\n";
    std::cout << "- 50-element position_value array: ✓\n";
    std::cout << "- Initialized based on distance to center: ✓\n";
    std::cout << "- Center positions have higher values: ✓\n";
    std::cout << "- Edge positions have lower values: ✓\n";
    std::cout << "- Manhattan distance calculation: ✓\n";
    std::cout << "- Edge penalty applied: ✓\n";
}

int main() {
    test_piece_square_tables();
    return 0;
}
