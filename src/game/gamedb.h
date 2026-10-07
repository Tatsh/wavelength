#pragma once

/**
 * Settings of the game being set up or played, among them the rule set and the timing the world
 * publishes.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the "db" section of the
 * configuration Init() reads. The one instance is the function-local static of shared(), and
 * TheGameDb addresses it. Its members are not yet declared. WorldMgr::Load() reads the rule set
 * from `+0x50`, and WorldMgr::UpdateTime() writes the two timing floats at `+0x158` and `+0x15c`.
 */
class GameDb {
public:
    /**
     * Construct the database with its defaults.
     *
     * @ghidraAddress NTSC-U/C: 0x0026cf50
     * @ghidraAddress PAL: 0x00276af0
     */
    GameDb();

    /**
     * Report the single instance, constructing it on first use.
     *
     * @return The instance.
     * @ghidraAddress NTSC-U/C: 0x0026e4c0
     * @ghidraAddress PAL: 0x00278060
     */
    static GameDb *shared();

    /**
     * Read the "db" section of the configuration.
     *
     * @ghidraAddress NTSC-U/C: 0x0026e1b0
     * @ghidraAddress PAL: 0x00277d50
     */
    void Init();

    /**
     * Release what Init() created.
     *
     * @ghidraAddress NTSC-U/C: 0x0026e3f0
     * @ghidraAddress PAL: 0x00277f90
     */
    void Terminate();

    /**
     * Service the database once per frame.
     *
     * @ghidraAddress NTSC-U/C: 0x0026e518
     * @ghidraAddress PAL: 0x002780b8
     */
    void Poll();
};

/**
 * The game database, GameDb::shared() as the unit's static initialiser stored it.
 *
 * @ghidraAddress NTSC-U/C: 0x00440d44
 */
extern GameDb *TheGameDb;
