#include "Brain.hpp"

#include <algorithm>
#include <random>

Brain::Brain() : m_rng(std::random_device{}()) {}

int Brain::clamped(int slot) const {
    return std::clamp(slot, 0, MaxSlots - 1);
}

void Brain::clear() {
    m_best.clear();
    m_candidate.clear();
    m_demo.clear();
    m_bestSlot = 0;
    m_attempts = 0;
    m_improvements = 0;
    m_training = false;
    m_demonstration = false;
    m_demoPreviousHeld = false;
    m_solved = false;
}

void Brain::load(const std::string& bits, int bestSlot, bool solved) {
    clear();
    m_best.reserve(std::min(bits.size(), static_cast<size_t>(MaxSlots)));
    for (char c : bits) {
        if (m_best.size() >= MaxSlots) break;
        if (c != '0' && c != '1') break; // Ignore corrupted data.
        m_best.push_back(c == '1' ? 1 : 0);
    }
    m_bestSlot = clamped(bestSlot);
    m_solved = solved;
}

std::string Brain::serializeBest() const {
    std::string bits;
    bits.reserve(m_best.size());
    for (std::uint8_t b : m_best) bits.push_back(b ? '1' : '0');
    return bits;
}

void Brain::paintPulse(int start, int length, bool down) {
    if (start < 0) start = 0;
    if (start >= MaxSlots || length <= 0) return;
    auto end = std::min(MaxSlots, start + length);
    if (static_cast<int>(m_candidate.size()) < end)
        m_candidate.resize(end, 0);
    for (int pos = start; pos < end; ++pos)
        m_candidate[pos] = down ? 1 : 0;
}

void Brain::generateCandidate() {
    m_candidate = m_best;
    // Trial #1 from scratch is a no-input baseline, useful to locate the
    // first danger. All later attempts mutate near our last best distance.
    if (m_attempts == 1 && m_best.empty()) return;

    // Periodically explore some distance before the current frontier.
    const int distance = (m_attempts % 9 == 0) ? 52 : 22;
    std::uniform_int_distribution<int> offset(-distance, 10);
    std::uniform_int_distribution<int> duration(2, 21);
    std::bernoulli_distribution hold(0.7);
    const int changes = (m_attempts % 5 == 0) ? 2 : 1;
    for (int i = 0; i < changes; ++i) {
        int start = clamped(m_bestSlot + offset(m_rng));
        int length = duration(m_rng);
        // Set a press or clear an already-recorded press. This can discover
        // both better jump timing and shorter holds.
        paintPulse(start, length, hold(m_rng));
    }
}

void Brain::beginAttempt(bool training, bool demonstration) {
    m_training = training;
    m_demonstration = demonstration && !training;
    m_demo.clear();
    m_demoPreviousHeld = false;
    m_candidate.clear();
    if (m_training) {
        ++m_attempts;
        generateCandidate();
    }
    if (m_demonstration) ++m_attempts;
}

bool Brain::desiredHold(int slot) const {
    if (!m_training || slot < 0 || slot >= MaxSlots) return false;
    return static_cast<size_t>(slot) < m_candidate.size() && m_candidate[slot] != 0;
}

void Brain::observeDemo(int slot, bool held) {
    if (!m_demonstration || slot < 0 || slot >= MaxSlots) return;
    if (static_cast<size_t>(slot) >= m_demo.size())
        m_demo.resize(slot + 1, m_demoPreviousHeld ? 1 : 0);
    m_demo[slot] = held ? 1 : 0;
    m_demoPreviousHeld = held;
}

bool Brain::finishAttempt(int reachedSlot, bool completed) {
    if (!active()) return false;
    reachedSlot = clamped(reachedSlot);
    bool improved = completed && !m_solved;
    if (reachedSlot > m_bestSlot) improved = true;
    if (!improved) return false;

    m_bestSlot = std::max(reachedSlot, m_bestSlot);
    m_solved |= completed;
    ++m_improvements;
    m_best = m_demonstration ? m_demo : m_candidate;
    // Limit persisted data, but keep a few slots after progress.
    if (m_best.size() > static_cast<size_t>(MaxSlots))
        m_best.resize(MaxSlots);
    return true;
}
