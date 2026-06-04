#include "history.h"

History::History() {}

void History::pushState(const Canvas& canvas) {
    // Collect the current state of both layers
    std::vector<Layer> state;
    state.push_back(canvas.getLayer(0));
    state.push_back(canvas.getLayer(1));

    undoStack.push_back(state);

    // If the stack size exceeds the history cap, remove the oldest state
    if (undoStack.size() > maxHistorySteps) {
        undoStack.erase(undoStack.begin());
    }

    // A new edit invalidates any existing redos
    redoStack.clear();
}

bool History::undo(Canvas& canvas) {
    if (undoStack.empty()) return false;

    // Snapshot the current canvas state and push it to the redo stack
    std::vector<Layer> currentState;
    currentState.push_back(canvas.getLayer(0));
    currentState.push_back(canvas.getLayer(1));
    redoStack.push_back(currentState);

    // Retrieve the previous canvas state from the undo stack
    std::vector<Layer> previousState = undoStack.back();
    undoStack.pop_back();

    // Restore the layer pixels
    canvas.getLayer(0) = previousState[0];
    canvas.getLayer(1) = previousState[1];

    return true;
}

bool History::redo(Canvas& canvas) {
    if (redoStack.empty()) return false;

    // Snapshot the current canvas state and push it to the undo stack
    std::vector<Layer> currentState;
    currentState.push_back(canvas.getLayer(0));
    currentState.push_back(canvas.getLayer(1));
    undoStack.push_back(currentState);

    // Retrieve the next canvas state from the redo stack
    std::vector<Layer> nextState = redoStack.back();
    redoStack.pop_back();

    // Restore the layer pixels
    canvas.getLayer(0) = nextState[0];
    canvas.getLayer(1) = nextState[1];

    return true;
}

void History::clear() {
    undoStack.clear();
    redoStack.clear();
}
