# Checkers Critical Bugs Fix Design

## Overview

This design addresses four critical bugs in the international checkers AI implementation that affect game correctness, rule compliance, and program stability. The bugs are:

1. **Array Overflow Risk**: The `captures` array in the `Move` structure is limited to 12 elements but can theoretically reach 20 in 100-square international checkers, risking memory corruption
2. **Rule Violation - Backward Captures**: Normal pieces are incorrectly prohibited from capturing backward in the `can_capture_from` function, violating international checkers rules
3. **Rule Violation - Landing on Captured Pieces**: The `generate_man_captures` function allows landing on captured but not yet removed pieces by treating them as empty squares
4. **Rule Discrepancy - Draw Rule**: The `is_draw` method uses a threshold of 100 half-moves (50 full moves) instead of the required 80 half-moves (40 full moves)

The fix approach is surgical and minimal: expand the array size, remove the backward capture restriction, fix the empty square calculation, and adjust the draw threshold. These changes are localized to specific functions and do not require architectural changes.

## Glossary

- **Bug_Condition (C)**: The conditions that trigger each of the four bugs
- **Property (P)**: The desired correct behavior after fixing each bug
- **Preservation**: Existing functionality that must remain unchanged by the fixes
- **Move Structure**: The data structure in `boyi/boyi.cpp` that represents a chess move, containing from/to positions, captures array, and metadata
- **captures array**: Array within Move structure that stores positions of captured pieces during multi-jump sequences
- **can_capture_from**: Function that checks if a piece at a given square can perform a capture move
- **generate_man_captures**: Recursive function that generates all possible capture sequences for normal (non-king) pieces
- **is_draw**: Method in GameState class that determines if the current position is a draw based on repetition and move count rules
- **halfmove_clock**: Counter tracking the number of half-moves (individual player moves) since the last capture or pawn move, used for draw detection

## Bug Details

### Bug 1: Array Overflow Risk

The bug manifests when a capture sequence reaches 13 or more consecutive jumps. The `Move` structure's `captures` array has only 12 elements, but international checkers on a 100-square board can theoretically produce capture sequences up to 20 jumps.

**Formal Specification:**
```
FUNCTION isBugCondition_ArrayOverflow(captureSequence)
  INPUT: captureSequence of type CaptureMove
  OUTPUT: boolean
  
  RETURN captureSequence.num_captures >= 13
         AND captureSequence.num_captures <= 20
         AND arraySize(Move.captures) == 12
END FUNCTION
```

**Examples:**
- A capture sequence with 13 jumps writes to `captures[12]`, which is out of bounds → memory corruption
- A capture sequence with 20 jumps writes to `captures[19]`, which is 8 elements beyond the array → severe memory corruption
- A capture sequence with 12 jumps writes to `captures[11]`, which is the last valid element → works correctly (no bug)
- Edge case: A capture sequence with 21 jumps would exceed even the expanded array → expected behavior is to handle up to 20

### Bug 2: Rule Violation - Backward Captures

The bug manifests when a normal (non-king) piece has an opportunity to capture backward. The `can_capture_from` function incorrectly skips backward directions for normal pieces, even though international checkers rules allow backward captures.

**Formal Specification:**
```
FUNCTION isBugCondition_BackwardCapture(piece, captureOpportunity)
  INPUT: piece of type Piece, captureOpportunity of type CaptureDirection
  OUTPUT: boolean
  
  RETURN piece.isKing == false
         AND captureOpportunity.direction IN [BACKWARD_LEFT, BACKWARD_RIGHT]
         AND opponentPieceExists(piece.position + captureOpportunity.direction)
         AND emptySquareExists(piece.position + 2 * captureOpportunity.direction)
         AND NOT captureIsGenerated(piece, captureOpportunity)
END FUNCTION
```

**Examples:**
- Black normal piece at square 25 with white piece at 20 and empty square at 14 → should capture backward but doesn't (bug)
- White normal piece at square 25 with black piece at 30 and empty square at 36 → should capture backward but doesn't (bug)
- King piece at square 25 with opponent at 20 and empty at 14 → captures backward correctly (no bug, kings already work)
- Edge case: Normal piece at edge of board with backward capture opportunity → should still be evaluated for capture

### Bug 3: Rule Violation - Landing on Captured Pieces

The bug manifests when generating multi-jump capture sequences. The `generate_man_captures` function uses `uint64_t empty = board.get_empty_squares() | captured`, which treats captured pieces as empty squares, allowing landing on positions occupied by captured but not yet removed pieces.

**Formal Specification:**
```
FUNCTION isBugCondition_LandingOnCaptured(move, capturedPieces)
  INPUT: move of type MultiJumpMove, capturedPieces of type BitBoard
  OUTPUT: boolean
  
  RETURN move.isMultiJump == true
         AND EXISTS intermediateSquare IN move.jumpSequence WHERE
             (intermediateSquare IN capturedPieces)
             AND (intermediateSquare != move.finalSquare)
END FUNCTION
```

**Examples:**
- Piece jumps from 15→24 (capturing 19), then attempts 24→33 where 24 was just captured → should be invalid but is allowed (bug)
- Piece jumps from 15→24→33 (capturing 19, 28), landing squares 24 and 33 are truly empty → works correctly (no bug)
- Piece jumps from 15→24 (capturing 19), then attempts 24→15 where 15 is the original position → should be invalid (different rule, not this bug)
- Edge case: Three-jump sequence where middle landing square was captured in first jump → should be prevented

### Bug 4: Rule Discrepancy - Draw Rule

The bug manifests when checking for draw conditions based on the 50-move rule. The `is_draw` method uses `halfmove_clock >= 100` (50 full moves), but tournament rules require a 40-move rule (80 half-moves).

**Formal Specification:**
```
FUNCTION isBugCondition_DrawRule(gameState)
  INPUT: gameState of type GameState
  OUTPUT: boolean
  
  RETURN gameState.halfmove_clock >= 80
         AND gameState.halfmove_clock < 100
         AND NOT gameState.is_draw()
         AND noCaptures(gameState, last_80_halfmoves)
END FUNCTION
```

**Examples:**
- Game reaches 80 half-moves (40 full moves) without capture → should be draw but continues (bug)
- Game reaches 90 half-moves (45 full moves) without capture → should be draw but continues (bug)
- Game reaches 100 half-moves (50 full moves) without capture → correctly declared draw (no bug, but threshold is wrong)
- Edge case: Game reaches 79 half-moves without capture → should continue playing (not a draw yet)

## Expected Behavior

### Preservation Requirements

**Unchanged Behaviors:**

**Bug 1 - Array Overflow:**
- Capture sequences of 12 or fewer jumps must continue to work exactly as before
- The Move structure's other fields (from, to, num_captures, is_promotion, score) must remain unchanged
- Move serialization and deserialization (to_string, from_string) must continue to work

**Bug 2 - Backward Captures:**
- Forward captures for normal pieces must continue to work exactly as before
- King piece capture generation (all directions) must remain unchanged
- The `generate_man_captures` function's forward capture logic must be preserved

**Bug 3 - Landing on Captured Pieces:**
- Single-jump captures must continue to work exactly as before
- Multi-jump captures where all landing squares are truly empty must work as before
- King capture generation must remain unchanged

**Bug 4 - Draw Rule:**
- Three-fold repetition draw detection must continue to work exactly as before
- Draw detection when a player has no pieces must remain unchanged
- Draw detection when a player has no legal moves must remain unchanged
- The halfmove_clock increment and reset logic must remain unchanged

**Scope:**

All inputs that do NOT involve the specific bug conditions should be completely unaffected by these fixes. This includes:
- Normal moves (non-captures) for all piece types
- Capture sequences within the original 12-element limit
- Forward captures for normal pieces
- All king piece movements and captures
- Board state management and hash calculations
- Search engine and evaluation functions
- Time management and opening book functionality

## Hypothesized Root Cause

Based on the bug descriptions and code analysis, the root causes are:

### Bug 1: Array Overflow Risk
1. **Insufficient Array Size**: The captures array was sized for typical games (12 elements) but not for the theoretical maximum in 100-square international checkers (20 jumps)
2. **No Bounds Checking**: The code does not validate `num_captures` against the array size before writing to `captures[num_captures++]`
3. **Historical Limitation**: The original implementation may have been designed for 64-square checkers where 12 captures is sufficient

### Bug 2: Rule Violation - Backward Captures
1. **Incorrect Direction Filtering**: Lines 2562-2563 in `can_capture_from` explicitly skip backward directions for normal pieces:
   ```cpp
   if (is_black && (offset == -4 || offset == -6)) continue;  // 黑方不能向下吃
   if (!is_black && (offset == 6 || offset == 4)) continue;   // 白方不能向上吃
   ```
2. **Confusion with Movement Rules**: The code may have confused normal piece movement (forward only) with capture rules (all directions allowed)
3. **Incomplete Rule Implementation**: International checkers rules allow normal pieces to capture backward, but this was not implemented

### Bug 3: Rule Violation - Landing on Captured Pieces
1. **Incorrect Empty Square Calculation**: Line 2617 in `generate_man_captures` uses:
   ```cpp
   uint64_t empty = board.get_empty_squares() | captured;
   ```
   This treats captured pieces as empty by OR-ing them into the empty squares bitboard
2. **Misunderstanding of Capture Mechanics**: Captured pieces should remain on the board until the entire move sequence completes, so they should NOT be treated as empty landing squares
3. **Bitboard Logic Error**: The `|` operator adds captured squares to empty squares, when it should only use truly empty squares

### Bug 4: Rule Discrepancy - Draw Rule
1. **Wrong Threshold Value**: Line 2296 in `is_draw` uses:
   ```cpp
   if (halfmove_clock >= 100) {  // 双方各50步
   ```
   This implements a 50-move rule instead of the required 40-move rule
2. **Incorrect Rule Reference**: The comment mentions "50步规则" (50-move rule), suggesting the wrong rule was referenced
3. **Tournament Rule Mismatch**: International checkers tournament rules specify 40 moves, not 50

## Correctness Properties

Property 1: Bug Condition - Array Overflow Prevention

_For any_ capture sequence where the number of captures is between 13 and 20 (inclusive), the fixed Move structure SHALL safely store all capture positions in the expanded captures array without memory corruption, buffer overflow, or undefined behavior.

**Validates: Requirements 2.1, 2.2**

Property 2: Bug Condition - Backward Captures Enabled

_For any_ normal (non-king) piece that has a backward capture opportunity (opponent piece adjacent in backward direction with empty square beyond), the fixed can_capture_from function SHALL correctly identify this capture opportunity and return true, enabling the move generator to create the backward capture move.

**Validates: Requirements 2.3, 2.4**

Property 3: Bug Condition - Landing Square Validation

_For any_ multi-jump capture sequence, the fixed generate_man_captures function SHALL only allow landing on truly empty squares (squares with no pieces), preventing landing on positions occupied by captured pieces that remain on the board until the move completes.

**Validates: Requirements 2.5, 2.6**

Property 4: Bug Condition - Draw Rule Threshold

_For any_ game state where the halfmove_clock reaches 80 or more (40 full moves without capture or pawn move), the fixed is_draw method SHALL correctly return true, declaring the game a draw according to international checkers tournament rules.

**Validates: Requirements 2.7, 2.8**

Property 5: Preservation - Existing Capture Sequences

_For any_ capture sequence with 12 or fewer jumps, the fixed code SHALL produce exactly the same behavior as the original code, preserving all existing capture generation, move execution, and game state updates.

**Validates: Requirements 3.1, 3.2, 3.3**

Property 6: Preservation - Forward Captures and King Moves

_For any_ forward capture by a normal piece or any capture by a king piece, the fixed code SHALL produce exactly the same behavior as the original code, preserving all existing move generation logic for these cases.

**Validates: Requirements 3.4, 3.5**

Property 7: Preservation - Non-Capture Functionality

_For any_ non-capture move, board state operation, or draw detection based on repetition, the fixed code SHALL produce exactly the same behavior as the original code, preserving all functionality unrelated to the four specific bugs.

**Validates: Requirements 3.6, 3.7, 3.8, 3.9, 3.10, 3.11, 3.12**

## Fix Implementation

### Changes Required

Assuming our root cause analysis is correct:

**File**: `boyi/boyi.cpp`

### Bug 1: Array Overflow Risk

**Location**: Move structure definition (lines 35-36)

**Specific Changes**:
1. **Expand captures array**: Change `int captures[12]` to `int captures[20]`
   - Line 36: `int captures[12];` → `int captures[20];`
   - Line 42: Loop initialization `for (int i = 0; i < 12; ++i)` → `for (int i = 0; i < 20; ++i)`
   - Line 49: Loop initialization `for (int i = 0; i < 12; ++i)` → `for (int i = 0; i < 20; ++i)`

2. **Update array size references**: Ensure all loops and bounds checks use the new size
   - Line 90: Loop condition `for (size_t i = 1; i < squares.size() - 1 && i <= 12; ++i)` → `&& i <= 20`

**Rationale**: Expanding from 12 to 20 elements provides sufficient capacity for the theoretical maximum capture sequence in 100-square international checkers while maintaining memory efficiency.

### Bug 2: Rule Violation - Backward Captures

**Function**: `MoveGenerator::can_capture_from` (lines 2541-2585)

**Specific Changes**:
1. **Remove backward direction restrictions**: Delete lines 2562-2563 that skip backward directions for normal pieces
   - Delete line 2562: `if (is_black && (offset == -4 || offset == -6)) continue;  // 黑方不能向下吃`
   - Delete line 2563: `if (!is_black && (offset == 6 || offset == 4)) continue;   // 白方不能向上吃`

2. **Keep king check**: The `if (!is_king)` block should remain but be empty or removed entirely since it no longer contains any logic

**Rationale**: International checkers rules explicitly allow normal pieces to capture backward. The direction restriction was incorrect and removing it aligns the code with the official rules.

### Bug 3: Rule Violation - Landing on Captured Pieces

**Function**: `MoveGenerator::generate_man_captures` (lines 2610-2670)

**Specific Changes**:
1. **Fix empty square calculation**: Change line 2617 to use only truly empty squares
   - Line 2617: `uint64_t empty = board.get_empty_squares() | captured;` → `uint64_t empty = board.get_empty_squares();`

2. **Remove captured mask from empty calculation**: The `captured` bitboard should only be used to check if a piece has already been captured in the current sequence, not to mark squares as empty

**Rationale**: Captured pieces remain on the board until the entire move sequence completes. Treating them as empty squares violates the rules and allows invalid landing positions.

### Bug 4: Rule Discrepancy - Draw Rule

**Method**: `GameState::is_draw` (lines 2290-2300)

**Specific Changes**:
1. **Update draw threshold**: Change the halfmove_clock threshold from 100 to 80
   - Line 2296: `if (halfmove_clock >= 100) {  // 双方各50步` → `if (halfmove_clock >= 80) {  // 双方各40步`

2. **Update comment**: Change the comment to reflect the correct rule
   - Line 2296 comment: `// 双方各50步` → `// 双方各40步`

**Rationale**: International checkers tournament rules specify a 40-move rule (80 half-moves), not 50 moves. This change aligns the implementation with official tournament standards.

## Testing Strategy

### Validation Approach

The testing strategy follows a two-phase approach: first, surface counterexamples that demonstrate each bug on unfixed code, then verify the fixes work correctly and preserve existing behavior. Each bug will be tested independently with its own exploratory, fix checking, and preservation checking tests.

### Exploratory Bug Condition Checking

**Goal**: Surface counterexamples that demonstrate each bug BEFORE implementing the fixes. Confirm or refute the root cause analysis. If we refute, we will need to re-hypothesize.

**Test Plan**: Write tests that create scenarios triggering each bug condition. Run these tests on the UNFIXED code to observe failures and understand the root causes.

**Test Cases**:

**Bug 1: Array Overflow Risk**
1. **13-Jump Capture Test**: Create a board position with a 13-jump capture sequence (will fail on unfixed code with array overflow)
2. **20-Jump Capture Test**: Create a board position with a 20-jump capture sequence (will fail on unfixed code with severe overflow)
3. **12-Jump Capture Test**: Create a board position with a 12-jump capture sequence (should work on unfixed code, boundary test)
4. **Memory Corruption Detection**: Use memory sanitizers or valgrind to detect out-of-bounds writes (will detect corruption on unfixed code)

**Bug 2: Rule Violation - Backward Captures**
1. **Black Backward Capture Test**: Create position where black normal piece can capture backward (will fail on unfixed code - capture not generated)
2. **White Backward Capture Test**: Create position where white normal piece can capture backward (will fail on unfixed code - capture not generated)
3. **King Backward Capture Test**: Create position where king captures backward (should work on unfixed code - kings already work)
4. **Multi-Jump with Backward Test**: Create position requiring backward capture in multi-jump sequence (will fail on unfixed code)

**Bug 3: Rule Violation - Landing on Captured Pieces**
1. **Landing on Captured Square Test**: Create multi-jump where intermediate landing square was just captured (will fail on unfixed code - invalid move allowed)
2. **Three-Jump Sequence Test**: Create three-jump sequence where second landing square was captured in first jump (will fail on unfixed code)
3. **Valid Multi-Jump Test**: Create multi-jump where all landing squares are truly empty (should work on unfixed code)
4. **Captured Piece Removal Test**: Verify captured pieces are removed only after complete move sequence (may fail on unfixed code)

**Bug 4: Rule Discrepancy - Draw Rule**
1. **80 Half-Move Test**: Create game state with halfmove_clock = 80 (will fail on unfixed code - not declared draw)
2. **90 Half-Move Test**: Create game state with halfmove_clock = 90 (will fail on unfixed code - not declared draw)
3. **100 Half-Move Test**: Create game state with halfmove_clock = 100 (should work on unfixed code - declared draw)
4. **79 Half-Move Test**: Create game state with halfmove_clock = 79 (should work on unfixed code - not draw yet)

**Expected Counterexamples**:
- **Bug 1**: Memory corruption detected by sanitizers, undefined behavior, potential crashes
- **Bug 2**: Backward capture moves not generated, `can_capture_from` returns false for backward opportunities
- **Bug 3**: Invalid moves allowed, pieces landing on captured squares, rule violations
- **Bug 4**: Games continuing past 40-move threshold, incorrect draw detection timing

### Fix Checking

**Goal**: Verify that for all inputs where each bug condition holds, the fixed functions produce the expected behavior.

**Pseudocode:**

**Bug 1: Array Overflow**
```
FOR ALL captureSequence WHERE isBugCondition_ArrayOverflow(captureSequence) DO
  move := createMove(captureSequence)
  ASSERT move.num_captures <= 20
  ASSERT NO memory_corruption_detected()
  ASSERT move.captures[move.num_captures - 1] is_valid
END FOR
```

**Bug 2: Backward Captures**
```
FOR ALL position WHERE isBugCondition_BackwardCapture(position) DO
  result := can_capture_from_fixed(position.piece.square)
  ASSERT result == true
  moves := generate_moves_fixed(position)
  ASSERT EXISTS move IN moves WHERE move.is_backward_capture
END FOR
```

**Bug 3: Landing on Captured Pieces**
```
FOR ALL multiJump WHERE isBugCondition_LandingOnCaptured(multiJump) DO
  moves := generate_man_captures_fixed(multiJump.startPosition)
  ASSERT NOT EXISTS move IN moves WHERE 
    move.lands_on_captured_square(multiJump.capturedPieces)
END FOR
```

**Bug 4: Draw Rule**
```
FOR ALL gameState WHERE isBugCondition_DrawRule(gameState) DO
  result := is_draw_fixed(gameState)
  ASSERT result == true
  ASSERT gameState.halfmove_clock >= 80
END FOR
```

### Preservation Checking

**Goal**: Verify that for all inputs where the bug conditions do NOT hold, the fixed functions produce the same results as the original functions.

**Pseudocode:**

**General Preservation**
```
FOR ALL input WHERE NOT (isBugCondition_ArrayOverflow(input) OR 
                         isBugCondition_BackwardCapture(input) OR
                         isBugCondition_LandingOnCaptured(input) OR
                         isBugCondition_DrawRule(input)) DO
  ASSERT fixed_function(input) == original_function(input)
END FOR
```

**Testing Approach**: Property-based testing is recommended for preservation checking because:
- It generates many test cases automatically across the input domain
- It catches edge cases that manual unit tests might miss
- It provides strong guarantees that behavior is unchanged for all non-buggy inputs
- It can test the interaction between different game components

**Test Plan**: Observe behavior on UNFIXED code first for non-bug scenarios, then write property-based tests capturing that behavior.

**Test Cases**:

**Preservation Test 1: Short Capture Sequences**
- Observe that capture sequences with ≤12 jumps work correctly on unfixed code
- Write property test generating random positions with short capture sequences
- Verify fixed code produces identical results

**Preservation Test 2: Forward Captures**
- Observe that forward captures for normal pieces work correctly on unfixed code
- Write property test generating random positions with forward capture opportunities
- Verify fixed code produces identical results

**Preservation Test 3: King Movements**
- Observe that all king movements and captures work correctly on unfixed code
- Write property test generating random positions with king pieces
- Verify fixed code produces identical results

**Preservation Test 4: Draw Detection - Repetition**
- Observe that three-fold repetition draw detection works correctly on unfixed code
- Write property test generating random game sequences with repetitions
- Verify fixed code produces identical results

**Preservation Test 5: Non-Capture Moves**
- Observe that all non-capture moves work correctly on unfixed code
- Write property test generating random positions with quiet moves
- Verify fixed code produces identical results

**Preservation Test 6: Board State Management**
- Observe that make_move/unmake_move work correctly on unfixed code
- Write property test generating random move sequences
- Verify fixed code maintains identical board states and hash values

### Unit Tests

**Bug 1: Array Overflow Risk**
- Test Move structure with 13, 15, 18, and 20 captures
- Test array bounds with valid indices (0-19)
- Test Move serialization/deserialization with long capture sequences
- Test memory safety with sanitizers

**Bug 2: Rule Violation - Backward Captures**
- Test `can_capture_from` with backward capture opportunities for black pieces
- Test `can_capture_from` with backward capture opportunities for white pieces
- Test move generation includes backward captures
- Test multi-jump sequences requiring backward captures

**Bug 3: Rule Violation - Landing on Captured Pieces**
- Test `generate_man_captures` rejects landing on captured squares
- Test multi-jump sequences with various captured piece configurations
- Test that captured pieces remain on board during move sequence
- Test final board state after multi-jump completion

**Bug 4: Rule Discrepancy - Draw Rule**
- Test `is_draw` returns true at halfmove_clock = 80
- Test `is_draw` returns true at halfmove_clock = 85, 90, 95
- Test `is_draw` returns false at halfmove_clock = 79
- Test draw detection with other conditions (repetition, no pieces)

### Property-Based Tests

**Bug 1: Array Overflow Risk**
- Generate random capture sequences with 1-20 jumps, verify no memory corruption
- Generate random board positions, verify all generated moves have valid capture arrays
- Generate random move sequences, verify Move structure integrity throughout game

**Bug 2: Rule Violation - Backward Captures**
- Generate random board positions with backward capture opportunities, verify captures are generated
- Generate random game sequences, verify backward captures are considered in search
- Generate random positions with mixed forward/backward captures, verify all are found

**Bug 3: Rule Violation - Landing on Captured Pieces**
- Generate random multi-jump positions, verify no landing on captured squares
- Generate random capture sequences, verify captured pieces remain until move completes
- Generate random board positions, verify empty square calculation is correct

**Bug 4: Rule Discrepancy - Draw Rule**
- Generate random game sequences reaching 80+ half-moves, verify draw detection
- Generate random positions with various halfmove_clock values, verify correct draw threshold
- Generate random games with captures/promotions, verify halfmove_clock resets correctly

**Preservation Properties**
- Generate random positions with ≤12 captures, verify identical behavior before/after fix
- Generate random positions with forward captures only, verify identical behavior
- Generate random positions with king pieces, verify identical behavior
- Generate random game sequences with <80 half-moves, verify identical draw detection

### Integration Tests

**Full Game Flow Testing**
- Play complete games with fixed code, verify no crashes or memory corruption
- Test tournament scenarios with long capture sequences
- Test endgame scenarios approaching draw conditions
- Test AI search with backward capture opportunities

**Cross-Component Testing**
- Test interaction between move generation and search engine
- Test interaction between board state and draw detection
- Test interaction between capture generation and evaluation
- Test interaction between time management and iterative deepening

**Regression Testing**
- Run existing test suite (if any) on fixed code
- Compare game outcomes between fixed and unfixed code for non-bug positions
- Verify performance characteristics remain similar
- Verify AI playing strength is maintained or improved
