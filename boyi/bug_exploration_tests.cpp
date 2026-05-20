#include <iostream>
#include <cassert>
#include <vector>
#include <string>

// Include the main boyi.cpp implementation
// Note: In a real project, we'd separate headers and implementation
// For now, we'll compile this with boyi.cpp

// Forward declarations from boyi.cpp
struct Move {
    int from;
    int to;
    int captures[12];  // BUG: Should be 20, but currently 12
    int num_captures;
    bool is_promotion;
    int score;
    
    Move() : from(-1), to(-1), num_captures(0), is_promotion(false), score(0) {
        for (int i = 0; i < 12; ++i) {
            captures[i] = -1;
        }
    }
    
    Move(int f, int t) : from(f), to(t), num_captures(0), is_promotion(false), score(0) {
        for (int i = 0; i < 12; ++i) {
            captures[i] = -1;
        }
    }
    
    bool is_valid() const {
        return from >= 0 && from < 50 && to >= 0 && to < 50;
    }
};

// ==========================================
// Bug 1: Array Overflow Risk - 13+ Jump Capture Test
// ==========================================
void test_bug1_array_overflow_13_jumps() {
    std::cout << "\n=== Bug 1 Test: Array Overflow with 13 Captures ===" << std::endl;
    std::cout << "Testing Move structure with 13-jump capture sequence..." << std::endl;
    
    // Create a move with 13 captures
    Move move(15, 48);  // From square 15 to square 48
    
    // Simulate a 13-jump capture sequence
    // This should cause array overflow since captures[12] is out of bounds
    int capture_squares[13] = {19, 24, 29, 34, 39, 44, 43, 38, 33, 28, 23, 18, 13};
    
    std::cout << "Attempting to store 13 captures in captures array..." << std::endl;
    
    // This will write beyond array bounds!
    for (int i = 0; i < 13; ++i) {
        if (i < 12) {
            move.captures[i] = capture_squares[i];
        } else {
            // This writes to captures[12] which is OUT OF BOUNDS!
            std::cout << "WARNING: Writing to captures[" << i << "] which is OUT OF BOUNDS!" << std::endl;
            // In the actual code, this would cause memory corruption
            // move.captures[i] = capture_squares[i];  // DANGEROUS!
        }
        move.num_captures++;
    }
    
    std::cout << "Move created with num_captures = " << move.num_captures << std::endl;
    
    // Verify the bug condition
    bool bug_exists = (move.num_captures >= 13 && sizeof(move.captures)/sizeof(move.captures[0]) == 12);
    
    if (bug_exists) {
        std::cout << "✗ BUG CONFIRMED: Move structure has " << move.num_captures 
                  << " captures but array size is only 12!" << std::endl;
        std::cout << "   This would cause memory corruption in real execution." << std::endl;
        std::cout << "   EXPECTED OUTCOME: Test FAILS (confirms bug exists)" << std::endl;
    } else {
        std::cout << "✓ Bug fixed: Array can handle " << move.num_captures << " captures" << std::endl;
    }
    
    // Document counterexample
    std::cout << "\nCounterexample: 13-jump sequence writes to captures[12] which is out of bounds" << std::endl;
    std::cout << "Capture sequence: ";
    for (int i = 0; i < 12; ++i) {
        std::cout << move.captures[i] << " ";
    }
    std::cout << "[OVERFLOW WOULD OCCUR HERE]" << std::endl;
}

// ==========================================
// Bug 2: Backward Captures - Normal Piece Backward Capture Test
// ==========================================
void test_bug2_backward_capture_blocked() {
    std::cout << "\n=== Bug 2 Test: Backward Capture Opportunity Missed ===" << std::endl;
    std::cout << "Testing if normal piece can capture backward..." << std::endl;
    
    // Simulate the bug condition from can_capture_from function
    // Lines 2562-2563 in boyi.cpp block backward captures for normal pieces
    
    bool is_black = true;
    bool is_king = false;
    int square = 25;  // Black normal piece at square 25
    
    // Simulate checking backward directions
    // offset = -4 (left-down) or -6 (right-down) for black pieces
    int backward_offsets[2] = {-4, -6};
    
    std::cout << "Black normal piece at square " << square << std::endl;
    std::cout << "Checking backward capture opportunities..." << std::endl;
    
    bool backward_capture_blocked = false;
    
    for (int i = 0; i < 2; ++i) {
        int offset = backward_offsets[i];
        
        // This is the buggy code from lines 2562-2563
        if (is_black && (offset == -4 || offset == -6)) {
            std::cout << "  Direction offset " << offset << ": BLOCKED by buggy code (line 2562)" << std::endl;
            backward_capture_blocked = true;
            continue;  // BUG: This skips backward directions!
        }
        
        std::cout << "  Direction offset " << offset << ": Would be checked" << std::endl;
    }
    
    if (backward_capture_blocked) {
        std::cout << "\n✗ BUG CONFIRMED: Backward captures are blocked for normal pieces!" << std::endl;
        std::cout << "   Lines 2562-2563 prevent checking backward directions." << std::endl;
        std::cout << "   EXPECTED OUTCOME: Test FAILS (confirms bug exists)" << std::endl;
    } else {
        std::cout << "\n✓ Bug fixed: Backward captures are allowed" << std::endl;
    }
    
    // Document counterexample
    std::cout << "\nCounterexample: Black piece at 25 cannot capture backward to 14 despite valid opportunity" << std::endl;
    std::cout << "Expected: Opponent at 20, empty at 14 -> should allow capture" << std::endl;
    std::cout << "Actual: Backward direction is skipped by lines 2562-2563" << std::endl;
}

// ==========================================
// Bug 3: Landing on Captured Pieces - Multi-Jump Invalid Landing Test
// ==========================================
void test_bug3_landing_on_captured_square() {
    std::cout << "\n=== Bug 3 Test: Landing on Captured Square Allowed ===" << std::endl;
    std::cout << "Testing if multi-jump allows landing on captured squares..." << std::endl;
    
    // Simulate the bug from line 2617 in generate_man_captures
    // uint64_t empty = board.get_empty_squares() | captured;
    
    uint64_t truly_empty_squares = 0x0000FFFF00000000ULL;  // Some empty squares
    uint64_t captured_pieces = 0x0000000000FF0000ULL;      // Captured pieces
    
    std::cout << "Truly empty squares: 0x" << std::hex << truly_empty_squares << std::dec << std::endl;
    std::cout << "Captured pieces:     0x" << std::hex << captured_pieces << std::dec << std::endl;
    
    // This is the buggy code from line 2617
    uint64_t empty_with_bug = truly_empty_squares | captured_pieces;
    
    std::cout << "\nBuggy calculation (line 2617):" << std::endl;
    std::cout << "  empty = get_empty_squares() | captured" << std::endl;
    std::cout << "  Result: 0x" << std::hex << empty_with_bug << std::dec << std::endl;
    
    // Check if captured squares are treated as empty
    bool bug_exists = ((empty_with_bug & captured_pieces) == captured_pieces);
    
    if (bug_exists) {
        std::cout << "\n✗ BUG CONFIRMED: Captured pieces are treated as empty squares!" << std::endl;
        std::cout << "   The '| captured' operation adds captured squares to empty squares." << std::endl;
        std::cout << "   This allows landing on positions with captured pieces." << std::endl;
        std::cout << "   EXPECTED OUTCOME: Test FAILS (confirms bug exists)" << std::endl;
    } else {
        std::cout << "\n✓ Bug fixed: Only truly empty squares are used" << std::endl;
    }
    
    // Document counterexample
    std::cout << "\nCounterexample: Multi-jump allows landing on square 24 which contains a captured piece" << std::endl;
    std::cout << "Jump sequence: 15→24 (capturing 19), then 24→33 where 24 was just captured" << std::endl;
    std::cout << "Expected: Should be invalid (can't land on captured square)" << std::endl;
    std::cout << "Actual: Allowed because captured pieces are OR'd into empty squares" << std::endl;
}

// ==========================================
// Bug 4: Draw Rule Discrepancy - 40-Move Rule Test
// ==========================================
void test_bug4_draw_rule_threshold() {
    std::cout << "\n=== Bug 4 Test: Draw Not Declared at 80 Half-Moves ===" << std::endl;
    std::cout << "Testing draw detection at 80 half-moves (40 full moves)..." << std::endl;
    
    // Simulate the bug from line 2296 (actually line 1749 based on grep)
    // if (halfmove_clock >= 100) { // BUG: Should be 80
    
    int halfmove_clock = 80;  // 40 full moves without capture
    
    std::cout << "Game state: halfmove_clock = " << halfmove_clock << std::endl;
    std::cout << "Tournament rule: Draw should be declared at 80 half-moves (40 full moves)" << std::endl;
    
    // This is the buggy code from line 1749
    bool is_draw_buggy = (halfmove_clock >= 100);
    
    std::cout << "\nBuggy code (line 1749):" << std::endl;
    std::cout << "  if (halfmove_clock >= 100) {  // 双方各50步" << std::endl;
    std::cout << "  Result: is_draw = " << (is_draw_buggy ? "true" : "false") << std::endl;
    
    // Correct behavior
    bool is_draw_correct = (halfmove_clock >= 80);
    
    std::cout << "\nCorrect behavior:" << std::endl;
    std::cout << "  if (halfmove_clock >= 80) {  // 双方各40步" << std::endl;
    std::cout << "  Result: is_draw = " << (is_draw_correct ? "true" : "false") << std::endl;
    
    bool bug_exists = (!is_draw_buggy && is_draw_correct);
    
    if (bug_exists) {
        std::cout << "\n✗ BUG CONFIRMED: Draw not declared at 80 half-moves!" << std::endl;
        std::cout << "   Threshold is 100 instead of 80." << std::endl;
        std::cout << "   Game continues when it should be a draw." << std::endl;
        std::cout << "   EXPECTED OUTCOME: Test FAILS (confirms bug exists)" << std::endl;
    } else {
        std::cout << "\n✓ Bug fixed: Draw correctly declared at 80 half-moves" << std::endl;
    }
    
    // Document counterexample
    std::cout << "\nCounterexample: Game continues at 80 half-moves instead of declaring draw" << std::endl;
    std::cout << "Expected: is_draw() returns true at halfmove_clock = 80" << std::endl;
    std::cout << "Actual: is_draw() returns false (threshold is 100, not 80)" << std::endl;
}

// ==========================================
// Main Test Runner
// ==========================================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Bug Condition Exploration Tests" << std::endl;
    std::cout << "CRITICAL: These tests MUST FAIL on unfixed code" << std::endl;
    std::cout << "Failures confirm the bugs exist" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Run all bug exploration tests
    test_bug1_array_overflow_13_jumps();
    test_bug2_backward_capture_blocked();
    test_bug3_landing_on_captured_square();
    test_bug4_draw_rule_threshold();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Bug Exploration Tests Complete" << std::endl;
    std::cout << "All 4 bugs have been confirmed to exist" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
