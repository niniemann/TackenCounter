#ifndef LOGENTRY_HPP_
#define LOGENTRY_HPP_

#include <string>

enum struct PlayerState {
    SKIP = 0, // the player didn't play this round
    WON,
    LOST
};

struct LogEntry {
    // the base value of the game, without bock modifiers
    int baseValue;

    // if this game starts a bock(y), e.g. because its baseValue is 0 etc.
    bool startsBock;

    // one result for every player (min 4, max 5 -- in case of 4 player the last one always skips)
    PlayerState results[5];

    // just a field to store the cumulative sum of the players value in
    // dynamically set by the model, not to be persisted
    int cumSum[5];
};


#endif /* include guard: LOGENTRY_HPP_ */
