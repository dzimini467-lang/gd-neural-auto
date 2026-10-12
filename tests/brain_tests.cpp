#include "../src/Brain.hpp"
#include <cassert>
#include <iostream>
#include <string>

int main() {
    Brain brain;
    assert(brain.serializeBest().empty());
    brain.beginAttempt(true, false);
    assert(!brain.desiredHold(0));
    assert(brain.finishAttempt(100, false));
    assert(brain.bestSlot() == 100);
    assert(!brain.finishAttempt(50, false));
    for (int n = 0; n < 50; ++n) {
        brain.beginAttempt(true, false);
        assert(!brain.desiredHold(-1));
        assert(!brain.desiredHold(Brain::MaxSlots));
    }
    assert(brain.finishAttempt(150, false));
    auto saved = brain.serializeBest();
    Brain restored;
    restored.load(saved, 150, false);
    assert(restored.bestSlot() == 150);
    assert(restored.serializeBest() == saved);
    restored.beginAttempt(false, true);
    restored.observeDemo(0, false);
    restored.observeDemo(5, true);
    restored.observeDemo(10, false);
    assert(restored.finishAttempt(200, true));
    assert(restored.solved());
    assert(restored.serializeBest().substr(5, 5) == "11111");
    Brain corrupt;
    corrupt.load("101oops", 7, false);
    assert(corrupt.serializeBest() == "101");
    std::cout << "All Brain tests passed\n";
}
