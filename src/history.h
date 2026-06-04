#ifndef HISTORY_H
#define HISTORY_H

#include "canvas.h"
#include "layer.h"
#include <vector>

class History {
public:
    History();

    // Saves the state of the canvas layers to the undo stack.
    // Clears the redo stack as this represents a new branch of edits.
    void pushState(const Canvas& canvas);

    // Pops the top state off the undo stack, pushes current canvas to redo, and restores the previous canvas state.
    bool undo(Canvas& canvas);

    // Pops the top state off the redo stack, pushes current canvas to undo, and restores the next canvas state.
    bool redo(Canvas& canvas);

    // Clears all history records.
    void clear();

    // Check availability of undo and redo
    bool canUndo() const { return !undoStack.empty(); }
    bool canRedo() const { return !redoStack.empty(); }

private:
    // Storing snapshots of both layers
    std::vector<std::vector<Layer>> undoStack;
    std::vector<std::vector<Layer>> redoStack;

    // History cap limits to keep memory consumption low
    const size_t maxHistorySteps = 50;
};

#endif // HISTORY_H
