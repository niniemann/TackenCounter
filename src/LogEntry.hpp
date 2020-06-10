#ifndef LOGENTRY_HPP_
#define LOGENTRY_HPP_

#include <string>
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>

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

    // if the bock-trigger is suppressed, e.g. for exceeding the bock limit
    bool bockTriggerSuppressed = false;

    // one result for every player (min 4, max 5 -- in case of 4 player the last one always skips)
    PlayerState results[5];

    // just a field to store the cumulative sum of the players value in
    // dynamically set by the model, not to be persisted
    int cumSum[5];

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(
            cereal::make_nvp<Archive>("baseValue", baseValue),
            cereal::make_nvp<Archive>("startsBock", startsBock),
            cereal::make_nvp<Archive>("results", results)
        );
    }
};


#endif /* include guard: LOGENTRY_HPP_ */
