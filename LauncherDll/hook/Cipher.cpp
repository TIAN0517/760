#include "stdafx.h"
#include "Cipher.h"

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <algorithm>

// 說明：這支是「跟遊戲連線用」的版本
// - initKeys(int key): 根據 S_KEY 產生 eb / db / hb / ch / ecc
// - encryptClient(...): 客戶端送出去用這個加密
// - decryptClient(...): 如果要解回「客戶端自己送的」封包才用這個，實務上比較少用
// - decryptServer(...): **重點**，用來解伺服器回來的封包，要「吃狀態」才會跟伺服器對得起來
// - decodeBD(...): 額外的 BD 封包保護還原（可選）
//
// 注意：這裡的 decryptServer 是「有狀態」的版本，跟你前面那支 LauncherDll 一致

Cipher::Cipher() {
    std::memset(eb, 0, sizeof(eb));
    std::memset(db, 0, sizeof(db));
    std::memset(hb, 0, sizeof(hb));
    std::memset(ch, 0, sizeof(ch));
    ecc = 0;
}

void Cipher::initKeys(int key) {
    // 1) 先算出 2 組 32bit key，照原版的異或＋rotate
    uint32_t keys[2];
    keys[0] = static_cast<uint32_t>(key) ^ _1;
    keys[0] = (keys[0] << 19) | (keys[0] >> (32 - 19));
    keys[1] = _2 ^ keys[0] ^ _3;

    // 2) 填 eb / db（前 8 bytes）
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 4; ++j) {
            uint8_t val = static_cast<uint8_t>((keys[i] >> (j * 8)) & 0xFF);
            db[i * 4 + j] = val;
            eb[i * 4 + j] = val;
        }
    }

    // 3) 產生 RC4-like 的表 hb / ch
    for (int i = 0; i < 256; ++i) {
        hb[i] = static_cast<uint8_t>(i);
    }
    int temp = 0;
    for (int j = 0; j < 256; ++j) {
        temp = (hb[j] + temp + eb[j % 8]) & 0xFF;
        std::swap(hb[j], hb[temp]);
    }
    // ch 是真正運行時要吃掉的表
    std::memcpy(ch, hb, 256);
    ecc = 0;
}

// 給 BD / 雜湊用的 RC4 一類的加密，這個本來就會「改 hb 狀態」
std::vector<uint8_t> Cipher::encryptHash(const std::vector<uint8_t>& inData) {
    std::vector<uint8_t> data = inData;
    int length = static_cast<int>(data.size()) + 1;
    int b = 0, c = 0;
    for (int a = 1; a < length; ++a) {
        b = (b + hb[a & 0xFF]) & 0xFF;
        c = a & 0xFF;
        std::swap(hb[c], hb[b]);
        data[a - 1] ^= hb[(hb[b] + hb[c]) & 0xFF];
    }
    return data;
}

// 解「客戶端那邊」的封包（有 4 bytes header 要剝）
std::vector<uint8_t> Cipher::decryptClient(const std::vector<uint8_t>& inData) {
    if (inData.size() < 4) {
        throw std::runtime_error("decryptClient: data too short");
    }
    std::vector<uint8_t> data = inData;

    // 反向還原前 4byte
    data[0] ^= static_cast<uint8_t>(db[5] ^ data[1]);
    data[1] ^= static_cast<uint8_t>(db[4] ^ data[2]);
    data[2] ^= static_cast<uint8_t>(db[3] ^ data[3]);
    data[3] ^= db[2];

    int length = static_cast<int>(data.size());
    for (int i = length - 1; i >= 1; --i) {
        data[i] ^= static_cast<uint8_t>(data[i - 1] ^ db[i & 0x7]);
    }
    data[0] ^= db[0];

    // 去掉前面 4byte header
    int newLen = length - 4;
    if (newLen < 0) {
        newLen = 0;
    }
    std::vector<uint8_t> temp(static_cast<size_t>(newLen));
    if (newLen > 0) {
        std::copy(data.begin() + 4, data.end(), temp.begin());
    }

    // 更新 db 狀態
    update(db, temp.data(), temp.size());
    return temp;
}

// 加「客戶端要送出去」的封包
std::vector<uint8_t> Cipher::encryptClient(const std::vector<uint8_t>& inData) {
    size_t size = inData.size();
    // 前面要多 4 bytes
    std::vector<uint8_t> nd(size + 4);

    if (size > 0) {
        std::memcpy(nd.data() + 4, inData.data(), size);
    }

    // 原本有一個 ecc 累加的寫法，這裡照 DLL 版「不動 ecc」的作法
    nd[0] ^= eb[0];
    for (size_t i = 1; i < nd.size(); ++i) {
        nd[i] ^= static_cast<uint8_t>(nd[i - 1] ^ eb[i & 0x7]);
    }
    nd[3] ^= eb[2];
    nd[2] ^= static_cast<uint8_t>(eb[3] ^ nd[3]);
    nd[1] ^= static_cast<uint8_t>(eb[4] ^ nd[2]);
    nd[0] ^= static_cast<uint8_t>(eb[5] ^ nd[1]);

    // 更新 eb 狀態（注意：是用原始明文 inData）
    update(eb, inData.data(), inData.size());
    return nd;
}

// ⭐ 真正重點：解伺服器回來的封包（要「吃狀態」）
// 和 RC4 類似：每解一包，ch[] / ecc 都會被改掉
std::vector<uint8_t> Cipher::decryptServer(const std::vector<uint8_t>& inData) {
    std::vector<uint8_t> data = inData;
    int length = static_cast<int>(data.size()) + 1;

    int c = ecc;  // 繼承上次的 c
    for (int a = 1; a < length; ++a) {
        int b = a & 0xFF;
        c = (c + ch[b]) & 0xFF;
        std::swap(ch[b], ch[c]); // 真正更新狀態
        data[a - 1] ^= ch[(ch[b] + ch[c]) & 0xFF];
    }
    ecc = c;      // 狀態寫回去
    return data;
}

// debug 用
void Cipher::printEB() {
    std::printf("Cipher.eb: ");
    for (size_t i = 0; i < 8; ++i) {
        std::printf("%02X ", eb[i]);
    }
    std::printf("\n");
}

void Cipher::printDB() {
    std::printf("Cipher.db: ");
    for (size_t i = 0; i < 8; ++i) {
        std::printf("%02X ", db[i]);
    }
    std::printf("\n");
}

// BD (Buffer Defense) 的還原，純粹把「對應表」反轉回去
std::vector<uint8_t> Cipher::decodeBD(const std::vector<uint8_t>& inData) {
    std::vector<uint8_t> decoded(inData.size());

    // 生成反查表：decodeTable[hb[x]] = x
    uint8_t decodeTable[256];
    for (int i = 0; i < 256; ++i) {
        uint8_t encodedValue = hb[i];
        decodeTable[encodedValue] = static_cast<uint8_t>(i);
    }

    for (size_t i = 0; i < inData.size(); ++i) {
        uint8_t encodedByte = inData[i];
        decoded[i] = decodeTable[encodedByte];
    }

    return decoded;
}
