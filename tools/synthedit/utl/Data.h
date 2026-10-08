#pragma once

#include <stddef.h>

#include <vector>

#include "utl/BinStream.h"
#include "utl/PrnStream.h"

/**
 * Array of script data: integers, real numbers, symbols, and nested arrays.
 *
 * The RTTI records the class. The object is 0x10 bytes. The nodes and a table of their 2-bit
 * types share one pool allocation, the types packed 16 to a word after the last node. An array is
 * reference counted and deletes itself when its last reference is released. The name of the member
 * that marks an array sorted is inferred.
 */
class DataArray {
public:
    /** The type of a node. */
    enum DataType {
        kDataInt = 0,    /*!< DataNode::i. */
        kDataSymbol = 1, /*!< DataNode::sym. */
        kDataFloat = 2,  /*!< DataNode::f. */
        kDataArray = 3,  /*!< DataNode::array. */
    };

    /** One node. Its type is recorded separately, in the array. */
    union DataNode {
        int i;            /*!< An integer. */
        float f;          /*!< A real number. */
        const char *sym;  /*!< A symbol, from the shared string table. */
        DataArray *array; /*!< A nested array, which the parent holds a reference to. */
    };

    /**
     * Create an array of integer zeroes, with one reference.
     *
     * @param size The number of nodes.
     * @ghidraAddress 0x10012670
     */
    DataArray(int size);

    /**
     * Read an array, with one reference, as a compiled script stores it.
     *
     * @param bs The stream.
     * @param filenames The script file names, which the stored file index selects from.
     * @ghidraAddress 0x10012ca0
     */
    DataArray(BinStream &bs, std::vector<const char *> &filenames);

    /**
     * Release the child arrays and free the nodes.
     *
     * @ghidraAddress 0x100126d0
     */
    ~DataArray();

    /**
     * Allocate from the pool allocator under the name `DataArray`. The control compiles this
     * inline.
     *
     * @param size The size of the object.
     * @return The memory.
     */
    void *operator new(size_t size);

    /**
     * Return memory to the pool allocator.
     *
     * @param mem The memory.
     * @ghidraAddress 0x100122b0
     */
    void operator delete(void *mem);

    /**
     * Write the array in script syntax.
     *
     * An array with nested arrays is written one node to a line, indented.
     *
     * @param s The stream.
     * @ghidraAddress 0x10012140
     */
    void Print(PrnStream &s);

    /**
     * Change the number of nodes. Removed nested arrays are released; added nodes are integer
     * zeroes.
     *
     * @param size The new number of nodes.
     * @ghidraAddress 0x100122c0
     */
    void Resize(int size);

    /**
     * Take a reference.
     *
     * @ghidraAddress 0x100123d0
     */
    void AddRef();

    /**
     * Drop a reference, deleting the array with its last one.
     *
     * @ghidraAddress 0x100123e0
     */
    void Release();

    /**
     * Find the nested array whose first node equals a value. A sorted array is searched by
     * bisection, an unsorted one from the start.
     *
     * @param tag The value of the first node.
     * @return The array, or null.
     * @ghidraAddress 0x10012410
     */
    DataArray *FindArray(int tag);

    /**
     * Find the nested array whose first node is a symbol.
     *
     * @param tag The symbol's text.
     * @param fail Whether a missing array is a failure.
     * @return The array, or null.
     * @ghidraAddress 0x10012520
     */
    DataArray *FindArray(const char *tag, bool fail);

    /**
     * Read the symbol after a tag.
     *
     * @param tag The tag.
     * @param value Receives the symbol.
     * @param fail Whether a missing tag is a failure.
     * @return Whether the tag was found.
     * @ghidraAddress 0x100125a0
     */
    bool FindSymbol(const char *tag, const char **value, bool fail);

    /**
     * Read the integer after a tag.
     *
     * @param tag The tag.
     * @param value Receives the integer.
     * @param fail Whether a missing tag is a failure.
     * @return Whether the tag was found.
     * @ghidraAddress 0x100125d0
     */
    bool FindInt(const char *tag, int *value, bool fail);

    /**
     * Read the real number after a tag. An integer converts.
     *
     * @param tag The tag.
     * @param value Receives the number.
     * @param fail Whether a missing tag is a failure.
     * @return Whether the tag was found.
     * @ghidraAddress 0x10012600
     */
    bool FindFloat(const char *tag, float *value, bool fail);

    /**
     * Read the integer after a tag as a truth value.
     *
     * @param tag The tag.
     * @param value Receives whether the integer is nonzero.
     * @param fail Whether a missing tag is a failure.
     * @return Whether the tag was found.
     * @ghidraAddress 0x10012630
     */
    bool FindBool(const char *tag, bool *value, bool fail);

    /**
     * Report the type of a node.
     *
     * @param i The node.
     * @return A DataType.
     * @ghidraAddress 0x10012720
     */
    int Type(int i);

    /**
     * Read a symbol node. Another type is a failure.
     *
     * @param i The node.
     * @return The symbol.
     * @ghidraAddress 0x10012780
     */
    const char *Sym(int i);

    /**
     * Read a node of any type.
     *
     * @param i The node.
     * @return The node.
     * @ghidraAddress 0x10012840
     */
    DataNode Node(int i);

    /**
     * Read an integer node. Another type is a failure.
     *
     * @param i The node.
     * @return The integer.
     * @ghidraAddress 0x10012890
     */
    int Int(int i);

    /**
     * Read a real number node. An integer converts; another type is a failure.
     *
     * @param i The node.
     * @return The number.
     * @ghidraAddress 0x10012950
     */
    float Float(int i);

    /**
     * Read an array node. Another type is a failure.
     *
     * @param i The node.
     * @return The array.
     * @ghidraAddress 0x10012a20
     */
    DataArray *Array(int i);

    /**
     * Replace a node. A symbol is entered into the shared string table, and an array gains a
     * reference; the array the node held loses one.
     *
     * @param i The node.
     * @param value The value.
     * @param type The value's DataType.
     * @ghidraAddress 0x10012b20
     */
    void SetNode(int i, DataNode value, int type);

    /**
     * Record where the array was defined.
     *
     * @param file The script file.
     * @param line The line.
     * @ghidraAddress 0x10012c60
     */
    void SetFileLine(const char *file, int line);

    /** @return The number of nodes. */
    int Size() const {
        return mSize;
    }

private:
    /**
     * Write one node in script syntax.
     *
     * @param s The stream.
     * @param i The node.
     * @ghidraAddress 0x100120b0
     */
    void PrintNode(PrnStream &s, int i);

    /**
     * Report whether a node is a nested array.
     *
     * @return Whether one is.
     * @ghidraAddress 0x10012210
     */
    bool HasArrays();

    /**
     * Set the type of a node without changing its value.
     *
     * @param i The node.
     * @param type The DataType.
     * @ghidraAddress 0x10012ae0
     */
    void SetType(int i, int type);

    /** @return The first word of the type table. */
    int *TypeWords() {
        return &mNodes[mSize].i;
    }

    DataNode *mNodes;  /*!< The nodes, followed by the type table. */
    const char *mFile; /*!< The script file the array came from, or null. */
    short mSize;       /*!< The number of nodes. */
    short mRefs;       /*!< The number of references. */
    short mLine;       /*!< The line the array came from. */
    short mSorted;     /*!< Whether the nested arrays are sorted by their first node. */
};

/**
 * Create an array of integer zeroes, with one reference.
 *
 * @param size The number of nodes.
 * @return The array.
 * @ghidraAddress 0x10012240
 */
DataArray *NewDataArray(int size);

/**
 * Merge the nested arrays of one array into another by their first node.
 *
 * A nested array of `src` whose tag `dest` lacks is appended to `dest`; one whose tag `dest`
 * already has is merged into that array the same way. Other nodes of `src` are ignored.
 *
 * @param dest The array that receives the nested arrays.
 * @param src The array to merge, or null.
 * @ghidraAddress 0x10012bc0
 */
void DataMergeTags(DataArray *dest, DataArray *src);
