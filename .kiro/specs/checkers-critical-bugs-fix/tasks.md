# Implementation Plan

## Phase 1: Bug Condition Exploration Tests (BEFORE Fix)

- [ ] 1. Write bug condition exploration tests for all 4 bugs
  - **Property 1: Bug Condition** - Critical Bugs Exploration
  - **CRITICAL**: These tests MUST FAIL on unfixed code - failures confirm the bugs exist
  - **DO NOT attempt to fix the tests or the code when they fail**
  - **NOTE**: These tests encode the expected behavior - they will validate the fixes when they pass after implementation
  - **GOAL**: Surface counterexamples that demonstrate each bug exists
  
  - [ ] 1.1 Bug 1: Array Overflow Risk - 13+ Jump Capture Test
    - **Property 1: Bug Condition** - Array Overflow with 13+ Captures
    - Create board position with 13-jump capture sequence
    - Test that Move structure can store all 13 captures without memory corruption
    - Run test on UNFIXED code with memory sanitizer if available
    - **EXPECTED OUTCOME**: Test FAILS with array overflow or memory corruption (confirms bug exists)
    - Document counterexamples found (e.g., "13-jump sequence writes to captures[12] which is out of bounds")
    - _Requirements: 1.1, 1.2_
  
  - [ ] 1.2 Bug 2: Backward Captures - Normal Piece Backward Capture Test
    - **Property 1: Bug Condition** - Backward Capture Opportunity Missed
    - Create position where black normal piece at square 25 has backward capture opportunity (opponent at 20, empty at 14)
    - Test that can_capture_from(25) returns true
    - Test that generate_moves includes the backward capture move
    - Run test on UNFIXED code
    - **EXPECTED OUTCOME**: Test FAILS - can_capture_from returns false, backward capture not generated (confirms bug exists)
    - Document counterexamples found (e.g., "Black piece at 25 cannot capture backward to 14 despite valid opportunity")
    - _Requirements: 1.3, 1.4_
  
  - [ ] 1.3 Bug 3: Landing on Captured Pieces - Multi-Jump Invalid Landing Test
    - **Property 1: Bug Condition** - Landing on Captured Square Allowed
    - Create multi-jump position where piece jumps 15→24 (capturing 19), then attempts 24→33 where 24 was just captured
    - Test that generate_man_captures does NOT allow landing on captured square 24
    - Run test on UNFIXED code
    - **EXPECTED OUTCOME**: Test FAILS - invalid move is generated, landing on captured square is allowed (confirms bug exists)
    - Document counterexamples found (e.g., "Multi-jump allows landing on square 24 which contains a captured piece")
    - _Requirements: 1.5, 1.6_
  
  - [ ] 1.4 Bug 4: Draw Rule Discrepancy - 40-Move Rule Test
    - **Property 1: Bug Condition** - Draw Not Declared at 80 Half-Moves
    - Create game state with halfmove_clock = 80 (40 full moves without capture)
    - Test that is_draw() returns true
    - Run test on UNFIXED code
    - **EXPECTED OUTCOME**: Test FAILS - is_draw returns false at 80 half-moves (confirms bug exists)
    - Document counterexamples found (e.g., "Game continues at 80 half-moves instead of declaring draw")
    - _Requirements: 1.7, 1.8_

## Phase 2: Preservation Property Tests (BEFORE Fix)

- [ ] 2. Write preservation property tests (BEFORE implementing fix)
  - **Property 2: Preservation** - Existing Functionality Preservation
  - **IMPORTANT**: Follow observation-first methodology
  - Observe behavior on UNFIXED code for non-buggy inputs
  - Write property-based tests capturing observed behavior patterns
  - Property-based testing generates many test cases for stronger guarantees
  
  - [ ] 2.1 Preservation: Short Capture Sequences (≤12 jumps)
    - **Property 2: Preservation** - Short Capture Sequences Work Correctly
    - Observe: Capture sequences with 12 or fewer jumps work correctly on unfixed code
    - Write property-based test: for all capture sequences with num_captures ≤ 12, Move structure stores captures correctly
    - Test Move.to_string() and Move.from_string() work correctly
    - Run tests on UNFIXED code
    - **EXPECTED OUTCOME**: Tests PASS (confirms baseline behavior to preserve)
    - _Requirements: 3.1, 3.2, 3.3_
  
  - [ ] 2.2 Preservation: Forward Captures for Normal Pieces
    - **Property 2: Preservation** - Forward Captures Work Correctly
    - Observe: Normal pieces capture forward correctly on unfixed code
    - Write property-based test: for all positions with forward capture opportunities, can_capture_from returns true and moves are generated
    - Test both black and white normal pieces
    - Run tests on UNFIXED code
    - **EXPECTED OUTCOME**: Tests PASS (confirms baseline behavior to preserve)
    - _Requirements: 3.4, 3.5_
  
  - [ ] 2.3 Preservation: King Piece Movements and Captures
    - **Property 2: Preservation** - King Movements Work Correctly
    - Observe: King pieces move and capture in all directions correctly on unfixed code
    - Write property-based test: for all king positions, all four directions are checked for captures
    - Test generate_king_captures function
    - Run tests on UNFIXED code
    - **EXPECTED OUTCOME**: Tests PASS (confirms baseline behavior to preserve)
    - _Requirements: 3.2_
  
  - [ ] 2.4 Preservation: Valid Multi-Jump Sequences
    - **Property 2: Preservation** - Valid Multi-Jumps Work Correctly
    - Observe: Multi-jump sequences where all landing squares are truly empty work correctly on unfixed code
    - Write property-based test: for all valid multi-jump positions, moves are generated correctly
    - Test that captured pieces are removed after move completes
    - Run tests on UNFIXED code
    - **EXPECTED OUTCOME**: Tests PASS (confirms baseline behavior to preserve)
    - _Requirements: 3.6, 3.7_
  
  - [ ] 2.5 Preservation: Draw Detection - Three-Fold Repetition
    - **Property 2: Preservation** - Repetition Draw Works Correctly
    - Observe: Three-fold repetition draw detection works correctly on unfixed code
    - Write property-based test: for all game sequences with 3+ position repetitions, is_draw returns true
    - Test count_repetitions method
    - Run tests on UNFIXED code
    - **EXPECTED OUTCOME**: Tests PASS (confirms baseline behavior to preserve)
    - _Requirements: 3.8, 3.9_
  
  - [ ] 2.6 Preservation: Halfmove Clock Management
    - **Property 2: Preservation** - Halfmove Clock Works Correctly
    - Observe: halfmove_clock increments and resets correctly on unfixed code
    - Write property-based test: for all moves, halfmove_clock resets to 0 after capture, increments by 1 after non-capture
    - Test make_move and unmake_move
    - Run tests on UNFIXED code
    - **EXPECTED OUTCOME**: Tests PASS (confirms baseline behavior to preserve)
    - _Requirements: 3.10, 3.11, 3.12_

## Phase 3: Implementation

- [ ] 3. Fix Bug 1: Array Overflow Risk

  - [ ] 3.1 Expand captures array from 12 to 20 elements
    - Change line 36 in Move structure: `int captures[12];` → `int captures[20];`
    - Update line 42 loop: `for (int i = 0; i < 12; ++i)` → `for (int i = 0; i < 20; ++i)`
    - Update line 49 loop: `for (int i = 0; i < 12; ++i)` → `for (int i = 0; i < 20; ++i)`
    - Update line 90 condition: `&& i <= 12` → `&& i <= 20`
    - _Bug_Condition: isBugCondition_ArrayOverflow(captureSequence) where captureSequence.num_captures >= 13 AND captureSequence.num_captures <= 20_
    - _Expected_Behavior: For any capture sequence with 13-20 jumps, Move structure safely stores all captures without memory corruption_
    - _Preservation: Capture sequences with ≤12 jumps continue to work exactly as before_
    - _Requirements: 2.1, 2.2, 3.1, 3.2, 3.3_

  - [ ] 3.2 Verify Bug 1 exploration test now passes
    - **Property 1: Expected Behavior** - Array Overflow Prevention
    - **IMPORTANT**: Re-run the SAME test from task 1.1 - do NOT write a new test
    - The test from task 1.1 encodes the expected behavior
    - When this test passes, it confirms the expected behavior is satisfied
    - Run 13-jump capture test from step 1.1
    - **EXPECTED OUTCOME**: Test PASSES (confirms bug is fixed)
    - _Requirements: Expected Behavior Properties from design - Property 1_

  - [ ] 3.3 Verify Bug 1 preservation tests still pass
    - **Property 2: Preservation** - Short Capture Sequences Preserved
    - **IMPORTANT**: Re-run the SAME test from task 2.1 - do NOT write a new test
    - Run short capture sequence tests from step 2.1
    - **EXPECTED OUTCOME**: Tests PASS (confirms no regressions)

- [ ] 4. Fix Bug 2: Rule Violation - Backward Captures

  - [ ] 4.1 Remove backward direction restrictions in can_capture_from
    - Delete line 2562: `if (is_black && (offset == -4 || offset == -6)) continue;  // 黑方不能向下吃`
    - Delete line 2563: `if (!is_black && (offset == 6 || offset == 4)) continue;   // 白方不能向上吃`
    - Keep the direction loop checking all 4 directions for all pieces
    - _Bug_Condition: isBugCondition_BackwardCapture(piece, captureOpportunity) where piece.isKing == false AND captureOpportunity.direction IN [BACKWARD_LEFT, BACKWARD_RIGHT]_
    - _Expected_Behavior: For any normal piece with backward capture opportunity, can_capture_from returns true and move is generated_
    - _Preservation: Forward captures for normal pieces and all king captures continue to work exactly as before_
    - _Requirements: 2.3, 2.4, 3.4, 3.5_

  - [ ] 4.2 Verify Bug 2 exploration test now passes
    - **Property 1: Expected Behavior** - Backward Captures Enabled
    - **IMPORTANT**: Re-run the SAME test from task 1.2 - do NOT write a new test
    - Run backward capture test from step 1.2
    - **EXPECTED OUTCOME**: Test PASSES (confirms bug is fixed)
    - _Requirements: Expected Behavior Properties from design - Property 2_

  - [ ] 4.3 Verify Bug 2 preservation tests still pass
    - **Property 2: Preservation** - Forward Captures and King Moves Preserved
    - **IMPORTANT**: Re-run the SAME tests from tasks 2.2 and 2.3 - do NOT write new tests
    - Run forward capture tests from step 2.2
    - Run king movement tests from step 2.3
    - **EXPECTED OUTCOME**: Tests PASS (confirms no regressions)

- [ ] 5. Fix Bug 3: Rule Violation - Landing on Captured Pieces

  - [ ] 5.1 Fix empty square calculation in generate_man_captures
    - Change line 2617: `uint64_t empty = board.get_empty_squares() | captured;` → `uint64_t empty = board.get_empty_squares();`
    - Remove the `| captured` operation that incorrectly treats captured pieces as empty
    - Keep the captured bitboard for checking if a piece was already captured in the sequence
    - _Bug_Condition: isBugCondition_LandingOnCaptured(move, capturedPieces) where move.isMultiJump == true AND intermediateSquare IN capturedPieces_
    - _Expected_Behavior: For any multi-jump sequence, only truly empty squares are valid landing positions_
    - _Preservation: Single-jump captures and valid multi-jumps where all landing squares are empty continue to work exactly as before_
    - _Requirements: 2.5, 2.6, 3.6, 3.7_

  - [ ] 5.2 Verify Bug 3 exploration test now passes
    - **Property 1: Expected Behavior** - Landing Square Validation
    - **IMPORTANT**: Re-run the SAME test from task 1.3 - do NOT write a new test
    - Run landing on captured square test from step 1.3
    - **EXPECTED OUTCOME**: Test PASSES (confirms bug is fixed)
    - _Requirements: Expected Behavior Properties from design - Property 3_

  - [ ] 5.3 Verify Bug 3 preservation tests still pass
    - **Property 2: Preservation** - Valid Multi-Jumps Preserved
    - **IMPORTANT**: Re-run the SAME test from task 2.4 - do NOT write a new test
    - Run valid multi-jump tests from step 2.4
    - **EXPECTED OUTCOME**: Tests PASS (confirms no regressions)

- [ ] 6. Fix Bug 4: Rule Discrepancy - Draw Rule

  - [ ] 6.1 Update draw threshold from 100 to 80 half-moves
    - Change line 2296 (actually line 1749 based on grep): `if (halfmove_clock >= 100) {  // 双方各50步` → `if (halfmove_clock >= 80) {  // 双方各40步`
    - Update comment to reflect correct 40-move rule
    - _Bug_Condition: isBugCondition_DrawRule(gameState) where gameState.halfmove_clock >= 80 AND gameState.halfmove_clock < 100_
    - _Expected_Behavior: For any game state with halfmove_clock >= 80, is_draw returns true_
    - _Preservation: Three-fold repetition draw detection and halfmove_clock management continue to work exactly as before_
    - _Requirements: 2.7, 2.8, 3.8, 3.9, 3.10, 3.11, 3.12_

  - [ ] 6.2 Verify Bug 4 exploration test now passes
    - **Property 1: Expected Behavior** - Draw Rule Threshold
    - **IMPORTANT**: Re-run the SAME test from task 1.4 - do NOT write a new test
    - Run 40-move rule test from step 1.4
    - **EXPECTED OUTCOME**: Test PASSES (confirms bug is fixed)
    - _Requirements: Expected Behavior Properties from design - Property 4_

  - [ ] 6.3 Verify Bug 4 preservation tests still pass
    - **Property 2: Preservation** - Repetition Draw and Clock Management Preserved
    - **IMPORTANT**: Re-run the SAME tests from tasks 2.5 and 2.6 - do NOT write new tests
    - Run three-fold repetition tests from step 2.5
    - Run halfmove clock tests from step 2.6
    - **EXPECTED OUTCOME**: Tests PASS (confirms no regressions)

## Phase 4: Final Validation

- [ ] 7. Checkpoint - Ensure all tests pass
  - Re-run all exploration tests (tasks 1.1-1.4) - all should PASS after fixes
  - Re-run all preservation tests (tasks 2.1-2.6) - all should still PASS
  - Verify no memory corruption with sanitizers if available
  - Verify all 4 bugs are fixed and no regressions introduced
  - Ask the user if questions arise or if additional testing is needed
