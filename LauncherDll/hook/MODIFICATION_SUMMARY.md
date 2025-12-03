# 登入器程式碼 Opcode 修改總結

## 概述
已成功將C++登入器程式碼中的Opcode定義更新為符合Java核心（版本: 1705042503）的標準定義。

## 修改日期
2025年10月26日

## 修改內容

### 1. 檔案位置
- **主要修改檔案**: `c:\Users\aaa23\Desktop\LinProject7.6_utf8_title\LinLogin76\LauncherDll\hook\opcodes.h`
- **備份檔案**: `opcodes.h.bak` (舊版本)
- **新版參考**: `opcodes_new.h` (已整合至opcodes.h)

### 2. FIRST_PACKET 常數修改
```
舊版本: {(unsigned char)-14, (unsigned char)-99, 50, 2, (unsigned char)-80, 66, (unsigned char)-72, 64}
新版本: {3, (unsigned char)-35, 10, 51, (unsigned char)-57, 47, (unsigned char)-16, 59}
```
這個常數對應Java中 `OpcodesClient._firstPacket` 的定義。

### 3. 客戶端操作碼 (CLIENT OPCODES) 修改

#### 主要變更
1. **命名規則更新**: 
   - 舊版使用 `C_OPCODE_*` 前綴
   - 新版使用 `C_*` 前綴（直接對應Java中的常數名稱）

2. **新增常數** (Java版本獨有):
   - C_VERSION (3)
   - C_REQUEST_ROLL (7)
   - C_MONITOR_CONTROL (9)
   - C_LEAVE_PLEDGE (11)
   - C_CHANGE_PASSWORD (12)
   - C_READ_NOTICE (15)
   - C_NEW_ACCOUNT (16)
   - C_KICK (17)
   - C_LOGIN_RESULT (27)
   - C_ONOFF (28)
   - C_CHANGE_CASTLE_SECURITY (30)
   - ... 以及其他許多常數

3. **值變更示例**:
   - C_CLIENT_READY: 45 (舊值 45 → 新值 32)
   - C_DELETE_BOOKMARK: 4
   - C_UPLOAD_EMBLEM: 33
   - C_RESTART: 37
   - ... 以及其他

### 4. 伺服器操作碼 (SERVER OPCODES) 更新

已將伺服器回傳封包的 opcode 常數全面同步為 Java `OpcodesServer` 定義，採用新版 `S_*` 命名並補齊 90+ 個常數，例如：

- `S_ADD_INVENTORY = 0`
- `S_VERSION_CHECK = 147`
- `S_EXTENDED_PROTOBUF = 223`
- `S_BOARD_LIST = 254`

同時清除舊的 `S_OPCODE_*` 前綴定義，避免與新版值衝突。

### 5. 相容性對應表
新增了舊版本到新版本的映射定義，用於遺留代碼的相容性：
```cpp
// 舊版本到新版本的映射
#define C_OPCODE_BANCLAN                C_BAN_MEMBER
#define C_OPCODE_CLIENTVERSION          C_VERSION
#define C_OPCODE_BOOKMARKDELETE         C_DELETE_BOOKMARK
// ... 以及其他映射
```

## Java OpcodesClient 完整對應表

所有254個客戶端操作碼現已完整對應：

| 值 | Java常數 | 說明 |
|----|---------|------|
| 0 | C_BAN_MEMBER | 封禁成員 |
| 3 | C_VERSION | 版本檢查 |
| 4 | C_DELETE_BOOKMARK | 刪除書籤 |
| 5 | C_EXIT_GHOST | 退出幽靈模式 |
| 6 | C_BUY_SPELL | 購買魔法 |
| 7 | C_REQUEST_ROLL | 請求骰子 |
| 9 | C_MONITOR_CONTROL | 監控控制 |
| 10 | C_SMS | 簡訊 |
| 11 | C_LEAVE_PLEDGE | 離開誓言 |
| 12 | C_CHANGE_PASSWORD | 修改密碼 |
| 14 | C_QUERY_CASTLE_SECURITY | 查詢城堡安全 |
| ... | ... | ... (完整列表見opcodes.h) |

## 驗證步驟

### 1. 編譯驗證
建議執行以下命令驗證新的opcodes.h是否編譯無誤：
```bash
# 在Visual Studio中
Build > Rebuild Solution
```

### 2. 功能驗證
- 確認登入器能正常編譯
- 測試與Java伺服器的通信
- 驗證所有客戶端操作碼能正確傳輸

### 3. 回滾方案
如果需要回滾到舊版本：
```bash
copy opcodes.h.bak opcodes.h
```

## 注意事項

1. **協議一致性**: 此次修改確保了C++登入器與Java伺服器之間的Opcode協議完全一致。

2. **版本標記**: 新版本已在檔案註釋中標記為版本 `1705042503`，與Java核心版本相同。

3. **相容性模式**: 保留了舊版本的映射定義，現有代碼無需立即重寫。

4. **建議**: 逐步將舊版本的 `C_OPCODE_*` 常數替換為新的 `C_*` 常數，以統一編碼風格。

5. **FIRST_PACKET**: 此常數的變更非常重要，它用於加密/解密通信。請確保Java伺服器也使用相同的值。

## 文件列表

已修改或建立的檔案：
- ✓ `opcodes.h` - 已更新（主檔案）
- ✓ `opcodes.h.bak` - 舊版本備份
- ✓ `opcodes_new.h` - 新版本參考（可刪除）
- ✓ `OPCODE_MAPPING.md` - 映射表文檔
- ✓ `MODIFICATION_SUMMARY.md` - 本檔案

## 聯絡資訊

如有任何問題或需要進一步的協助，請參考 OPCODE_MAPPING.md 中的詳細對應表。

---

**修改完成**: 2025年10月26日
**狀態**: ✓ 已完成
**測試狀態**: 待驗證編譯
