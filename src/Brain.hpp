#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

// Geode-independent search algorithm. All distances are measured in
// horizontal world-position slots (10 Geometry Dash units by default).
// This is an evolutionary / hill-climbing trainer, NOT a neural network.
class Brain {
public:
    static constexpr int MaxSlots = 30000;

    Brain();

    // A new attempt builds a candidate from the best policy learned so far.
    void beginAttempt(bool training, bool demonstration);
    bool desiredHold(int slot) const;
    void observeDemo(int slot, bool held);

    // Returns true if we found a new best run and the save should be updated.
    bool finishAttempt(int reachedSlot, bool completed);

    void clear();
    void load(const std::string& bits, int bestSlot, bool solved);
    std::string serializeBest() const;

    int bestSlot() const { return m_bestSlot; }
    int attempts() const { return m_attempts; }
    int improvements() const { return m_improvements; }
    bool solved() const { return m_solved; }
    bool active() const { return m_training || m_demonstration; }

private:
    int clamped(int slot) const;
    void generateCandidate();
    void paintPulse(int start, int length, bool down);

    std::vector<std::uint8_t> m_best;
    std::vector<std::uint8_t> m_candidate;
    std::vector<std::uint8_t> m_demo;

    int m_bestSlot = 0;
    int m_attempts = 0;
    int m_improvements = 0;
    bool m_training = false;
    bool m_demonstration = false;
    bool m_demoPreviousHeld = false;
    bool m_solved = false;
    std::mt19937 m_rng;
};
