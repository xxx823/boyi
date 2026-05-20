# Bugfix Requirements Document

## Introduction

This document addresses four critical bugs in the international checkers AI implementation that affect game correctness, rule compliance, and program stability. These bugs range from memory safety issues to rule violations that impact the AI's tactical capabilities and tournament compliance.

**Bugs Summary:**
1. **Array Overflow Risk**: Captures array limited to 12 but can reach 20 in 100-square checkers
2. **Rule Violation - Backward Captures**: Normal pieces incorrectly prohibited from capturing backward
3. **Rule Violation - Landing on Captured Pieces**: Allows landing on captured but not yet removed pieces
4. **Rule Discrepancy - Draw Rule**: Implements 50-move rule instead of required 40-move rule

## Bug Analysis

### Current Behavior (Defect)

#### Bug 1: Array Overflow Risk

1.1 WHEN a capture sequence reaches 13 or more consecutive jumps THEN the system writes beyond the captures array bounds causing memory corruption

1.2 WHEN the captures array (size 12) is exceeded THEN the system exhibits undefined behavior including potential program crashes

#### Bug 2: Rule Violation - Backward Captures

1.3 WHEN a normal (non-king) piece has an opportunity to capture backward in the `can_capture_from` function THEN the system incorrectly skips this direction due to direction restrictions

1.4 WHEN the AI performs quiescence search THEN the system misses backward capture opportunities for normal pieces, creating tactical blind spots

#### Bug 3: Rule Violation - Landing on Captured Pieces

1.5 WHEN generating man captures in `generate_man_captures` function THEN the system treats captured pieces as empty squares by using `uint64_t empty = board.get_empty_squares() | captured`

1.6 WHEN a multi-jump capture sequence is evaluated THEN the system allows landing on positions occupied by captured but not yet removed pieces

#### Bug 4: Rule Discrepancy - Draw Rule

1.7 WHEN the `is_draw` method checks for draw conditions THEN the system uses a threshold of 100 half-moves (50 full moves)

1.8 WHEN tournament rules require a 40-move draw rule THEN the system incorrectly allows games to continue beyond the proper draw threshold

### Expected Behavior (Correct)

#### Bug 1: Array Overflow Risk - Fixed

2.1 WHEN a capture sequence reaches up to 20 consecutive jumps THEN the system SHALL safely store all captures in the expanded captures array without memory corruption

2.2 WHEN the captures array is accessed THEN the system SHALL have sufficient capacity (20 elements) to handle the theoretical maximum for 100-square international checkers

#### Bug 2: Rule Violation - Backward Captures - Fixed

2.3 WHEN a normal (non-king) piece evaluates capture opportunities in `can_capture_from` THEN the system SHALL check all four directions (forward and backward) for capture possibilities

2.4 WHEN the AI performs quiescence search THEN the system SHALL correctly identify and evaluate backward capture moves for normal pieces according to international checkers rules

#### Bug 3: Rule Violation - Landing on Captured Pieces - Fixed

2.5 WHEN generating man captures in `generate_man_captures` function THEN the system SHALL use only truly empty squares (`board.get_empty_squares()`) without including captured pieces

2.6 WHEN a multi-jump capture sequence is evaluated THEN the system SHALL prevent landing on positions occupied by captured pieces that remain on the board until the move completes

#### Bug 4: Rule Discrepancy - Draw Rule - Fixed

2.7 WHEN the `is_draw` method checks for draw conditions THEN the system SHALL use a threshold of 80 half-moves (40 full moves)

2.8 WHEN tournament rules require a 40-move draw rule THEN the system SHALL correctly declare a draw after 40 moves without captures or pawn moves

### Unchanged Behavior (Regression Prevention)

#### General Capture Mechanics

3.1 WHEN a valid capture sequence of 12 or fewer jumps occurs THEN the system SHALL CONTINUE TO correctly record and execute the capture sequence

3.2 WHEN king pieces evaluate capture opportunities THEN the system SHALL CONTINUE TO check all four directions as currently implemented

3.3 WHEN the move generator creates capture moves THEN the system SHALL CONTINUE TO properly track the number of captures in `num_captures`

#### Forward Captures for Normal Pieces

3.4 WHEN a normal piece has a forward capture opportunity THEN the system SHALL CONTINUE TO correctly identify and generate this capture move

3.5 WHEN the `can_capture_from` function evaluates forward directions for normal pieces THEN the system SHALL CONTINUE TO work as currently implemented

#### Empty Square Detection

3.6 WHEN the board evaluates truly empty squares (no pieces present) THEN the system SHALL CONTINUE TO correctly identify these squares using `get_empty_squares()`

3.7 WHEN generating non-capture moves THEN the system SHALL CONTINUE TO use empty square detection without modification

#### Draw Detection - Three-Fold Repetition

3.8 WHEN the same position occurs three times THEN the system SHALL CONTINUE TO correctly declare a draw based on position repetition

3.9 WHEN the `count_repetitions` method tracks position history THEN the system SHALL CONTINUE TO function as currently implemented

#### Move Execution and Board State

3.10 WHEN any valid move is executed THEN the system SHALL CONTINUE TO correctly update the board state, hash values, and game history

3.11 WHEN the halfmove_clock is reset after a capture THEN the system SHALL CONTINUE TO reset to zero as currently implemented

3.12 WHEN the halfmove_clock is incremented after a non-capture move THEN the system SHALL CONTINUE TO increment by one as currently implemented
