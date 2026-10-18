#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

/** Readers and writers of Harmonix archives and compiled DataArray files. */
namespace Tools::Ark {

/**
 * One element of a DataArray, as a compiled DataArray file stores it.
 *
 * A symbol and a quoted string share one kind, because the compiled file does not tell them apart.
 */
struct DataNode {
    /** Kind of element, numbered as the two-bit tags of the compiled file. */
    enum class Type : std::uint8_t {
        Int = 0,    /*!< A 32-bit integer. */
        Symbol = 1, /*!< A symbol or a string. */
        Float = 2,  /*!< A 32-bit float. */
        Array = 3,  /*!< A nested array. */
    };

    Type type = Type::Int;          /*!< Kind of element. */
    std::uint32_t value = 0;        /*!< The integer, or the bits of the float. */
    std::string symbol;             /*!< The symbol or string. */
    std::vector<DataNode> children; /*!< The elements of an array. */
    std::uint16_t line = 0;         /*!< Source line of an array. */
    std::uint16_t sorted = 0;       /*!< Whether the game sorted an array before it was saved. */
    std::int32_t file = 0;          /*!< Index of an array's source file in the file table. */

    /**
     * Make an integer element.
     *
     * @param value The integer.
     * @return The element.
     */
    static DataNode makeInt(std::int32_t value);

    /**
     * Make a symbol element.
     *
     * @param symbol The symbol or string.
     * @return The element.
     */
    static DataNode makeSymbol(std::string symbol);

    /**
     * Make a float element.
     *
     * @param value The float.
     * @return The element.
     */
    static DataNode makeFloat(float value);

    /**
     * Make an empty array element.
     *
     * @param line Source line.
     * @param file Index of the source file in the file table.
     * @return The element.
     */
    static DataNode makeArray(std::uint16_t line, std::int32_t file);

    /**
     * Report whether the element is an array whose first element is a symbol.
     *
     * @param key The symbol.
     * @return Whether the first element is the symbol.
     */
    [[nodiscard]] bool isKeyed(std::string_view key) const;

    /**
     * Find a child array by the symbol that opens it, the way `DataArray::FindArray` does.
     *
     * @param key The symbol.
     * @return The first matching child, or null when there is none.
     */
    DataNode *findArray(std::string_view key);

    /** @copydoc findArray */
    [[nodiscard]] const DataNode *findArray(std::string_view key) const;

    /**
     * Find the index of a child array by the symbol that opens it.
     *
     * @param key The symbol.
     * @return The index, or -1 when there is no match.
     */
    [[nodiscard]] int indexOf(std::string_view key) const;

    /**
     * Change the file index of the element and of every array below it.
     *
     * @param map Callable taking an old index and returning the new one.
     */
    template <typename Map>
    void remapFiles(const Map &map) {
        if (type != Type::Array) {
            return;
        }
        file = map(file);
        for (auto &child : children) {
            child.remapFiles(map);
        }
    }

    /**
     * Compare two elements, their kinds, values, and arrays.
     *
     * @param other The other element.
     * @return Whether they are equal.
     */
    bool operator==(const DataNode &other) const = default;
};

} // namespace Tools::Ark
