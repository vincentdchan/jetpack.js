//
// Created by Duzhong Chen on 2021/3/25.
//

#include "Location.h"
#include <vector>
#include <robin_hood.h>
#include <cassert>
#include <mutex>

class FileIndexTable {
public:
    FileIndexTable() noexcept = default;

    inline const UString& IndexToFile(uint32_t n) {
        assert(n < indexToFileTable.size());
        std::lock_guard guard(m);
        return indexToFileTable[n];
    }

    uint32_t FileToIndex(const UString& filePath) {
        std::lock_guard guard(m);
        auto iter = fileToIndexTable.find(filePath);
        if (iter != fileToIndexTable.end()) {
            return iter->second;
        }

        uint32_t index = indexToFileTable.size();
        fileToIndexTable[filePath] = index;
        indexToFileTable.push_back(filePath);
        return index;
    }

private:
    robin_hood::unordered_map<UString, uint32_t> fileToIndexTable;
    std::vector<UString> indexToFileTable;
    std::mutex m;

};

namespace FileIndex {

    static FileIndexTable fileIndexTable;

    uint32_t fileIndexOfFile(const UString& filePath) {
        return fileIndexTable.FileToIndex(filePath);
    }

    UString fileOfFileIndex(uint32_t index) {
        return fileIndexTable.IndexToFile(index);
    }

}
