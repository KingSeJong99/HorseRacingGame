#include "HorseFactory.h"

horseracing::Horse* horseracing::HorseFactory::CreateHorse(
    const HorseStats& stats, const std::wstring& name)
{
    Horse* newHorse = new Horse(stats, name);

    return newHorse;
}
