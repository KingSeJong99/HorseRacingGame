#include "HorseFactory.h"

horseracing::Horse* horseracing::HorseFactory::CreateHorse(
    const HorseStats& stats, const std::string& name)
{
    Horse* newHorse = new Horse(stats, name);

    return newHorse;
}
