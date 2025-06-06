# Rubik's Cube Solver

This module provides a Rubik's cube solver that works with the scene system to solve a 3x3x3 Rubik's cube using the layer-by-layer method.

## Features

- **Layer-by-layer solving algorithm**: Implements the classic beginner's method
- **Move notation support**: Uses standard Rubik's cube notation (U, R, F, D, L, B)
- **Scene integration**: Works directly with the existing `Scene` structure
- **Move sequence output**: Returns a list of moves to solve the cube
- **Visual representation**: Can apply solutions back to the visual cube

## Usage

### Basic Usage

```c
#include "cube_solver.h"

// Assuming you have a Scene* scene with a scrambled Rubik's cube
MoveSequence* solution = cube_solver_solve(scene);

if (solution) {
    // Print the solution
    move_sequence_print(solution);
    
    // Apply solution to scene (optional)
    for (int i = 0; i < solution->count; i++) {
        Move move = solution->moves[i];
        FaceIndex face = move_to_face(move);
        RotationDirection direction = move_to_direction(move);
        
        rotate_face_colors(scene->cubeColors, face, direction);
        scene_rotate_face(scene, face, direction);
    }
    
    // Clean up
    move_sequence_destroy(solution);
    free(solution);
}
```

### Getting Cube State

```c
// Get the current cube colors
RGBColor* cube_colors = scene_get_cube_colors(scene);

// Check if cube is solved
if (is_cube_solved(cube_colors)) {
    printf("Cube is already solved!\n");
}
```

### Manual Move Application

```c
// Apply individual moves
Move move = MOVE_R;  // Right face clockwise
FaceIndex face = move_to_face(move);
RotationDirection direction = move_to_direction(move);

rotate_face_colors(scene->cubeColors, face, direction);
```

## API Reference

### Main Functions

#### `MoveSequence* cube_solver_solve(Scene* scene)`
Solves the Rubik's cube and returns a sequence of moves.
- **Parameters**: `scene` - Scene containing the Rubik's cube
- **Returns**: Pointer to MoveSequence with solution, or NULL on error
- **Note**: Caller must free the returned MoveSequence

#### `RGBColor* scene_get_cube_colors(Scene* scene)`
Gets the current cube color state from the scene.
- **Parameters**: `scene` - Scene containing the Rubik's cube
- **Returns**: Pointer to the cube colors array

#### `void rotate_face_colors(RGBColor (*cubeColors)[9], FaceIndex face, RotationDirection direction)`
Rotates a face of the cube and updates adjacent faces.
- **Parameters**: 
  - `cubeColors` - Array of cube colors [6][9]
  - `face` - Face to rotate (FACE_IDX_TOP, etc.)
  - `direction` - Direction to rotate (ROTATE_CLOCKWISE, etc.)

### Move Sequence Functions

#### `void move_sequence_init(MoveSequence* sequence)`
Initializes a move sequence.

#### `void move_sequence_add(MoveSequence* sequence, Move move)`
Adds a move to the sequence.

#### `void move_sequence_destroy(MoveSequence* sequence)`
Frees memory used by the sequence.

#### `void move_sequence_print(const MoveSequence* sequence)`
Prints the sequence in standard notation.

### Utility Functions

#### `bool is_cube_solved(const RGBColor (*cubeColors)[9])`
Checks if the cube is in solved state.

#### `const char* move_to_string(Move move)`
Converts a move to its string representation.

#### `FaceIndex move_to_face(Move move)`
Gets the face index for a move.

#### `RotationDirection move_to_direction(Move move)`
Gets the rotation direction for a move.

## Move Notation

The solver uses standard Rubik's cube notation:

- **U** - Top face clockwise
- **U'** - Top face counterclockwise
- **R** - Right face clockwise
- **R'** - Right face counterclockwise
- **F** - Front face clockwise
- **F'** - Front face counterclockwise
- **L** - Left face clockwise
- **L'** - Left face counterclockwise
- **B** - Back face clockwise
- **B'** - Back face counterclockwise
- **D** - Bottom face clockwise
- **D'** - Bottom face counterclockwise

## Algorithm Details

The solver implements a simplified layer-by-layer approach:

1. **White Cross**: Solves the white cross on the top face
2. **White Corners**: Positions white corners correctly
3. **Middle Layer**: Solves the middle layer edges
4. **Last Layer**: Solves the yellow face and final positioning

**Note**: This is a simplified implementation for demonstration. A production solver would use more sophisticated algorithms like CFOP, Roux, or Kociemba's algorithm for optimal solutions.

## Testing

To test the solver:

```c
#include "solver_test.c"

// Run comprehensive test
int result = test_cube_solver();

// Or demonstrate usage with existing scene
demonstrate_solver_usage(your_scene);
```

## Limitations

1. **Algorithm Efficiency**: Uses a basic layer-by-layer method, not optimal for move count
2. **Simplified Implementation**: Some solving steps use placeholder algorithms
3. **No Optimization**: Does not minimize the number of moves
4. **Error Handling**: Limited validation of cube states

## Future Improvements

- Implement advanced algorithms (CFOP, Kociemba)
- Add move count optimization
- Improve edge case handling
- Add cube state validation
- Implement more sophisticated solving techniques 