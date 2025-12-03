#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

class Cipher {
public:
    Cipher();

    void initKeys(int key);

    std::vector<uint8_t> encryptHash(const std::vector<uint8_t>& inData);

    std::vector<uint8_t> decryptClient(const std::vector<uint8_t>& inData);

    std::vector<uint8_t> encryptClient(const std::vector<uint8_t>& inData);

    std::vector<uint8_t> decryptServer(const std::vector<uint8_t>& inData);

    // BD (Buffer Defense) decoding - uses same substitution table as Cipher
    std::vector<uint8_t> decodeBD(const std::vector<uint8_t>& inData);

    void printEB();
    void printDB();
    uint32_t ecc;

private:
    static constexpr uint32_t _1 = 0x9c30d539U;
    static constexpr uint32_t _2 = 0x930fd7e2U;
    static constexpr uint32_t _3 = 0x7c72e993U;
    static constexpr uint32_t _4 = 0x287effc3U;
    uint8_t eb[8];
    uint8_t db[8];
    uint8_t hb[256];
    uint8_t ch[256];

    void update(uint8_t* data, const uint8_t* ref, size_t refSize) {
        for (size_t i = 0; i < 4 && i < refSize; ++i) {
            data[i] ^= ref[i];
        }
        uint32_t val = (static_cast<uint32_t>(data[7]) << 24)
            | (static_cast<uint32_t>(data[6]) << 16)
            | (static_cast<uint32_t>(data[5]) << 8)
            | static_cast<uint32_t>(data[4]);
        val += _4;
        for (int i = 0; i < 4; ++i) {
            data[4 + i] = static_cast<uint8_t>((val >> (i * 8)) & 0xFF);
        }
    }
};
