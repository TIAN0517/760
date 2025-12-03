#pragma once

/**
 * Client/Server Opcodes Definition
 * 對應 Java 中 OpcodesClient 和 OpcodesServer 類的操作碼
 * 版本: 1705042503
 * 
 * 此檔案已根據Java核心進行同步更新，確保
 * C++登入器與Java伺服器之間的協議一致性
 */

// ============================================================================
// CLIENT OPCODES (客戶端操作碼)
// ============================================================================

// First packet bytes (XOR 密鑰或加密標記) - 對應 Java OpcodesClient._firstPacket
static const unsigned char FIRST_PACKET[] = {3, (unsigned char)-35, 10, 51, (unsigned char)-57, 47, (unsigned char)-16, 59};

// Client Command Opcodes - 按Java OpcodesClient順序排列
#define C_BAN_MEMBER                    0   // 封禁成員
#define C_VERSION                       3   // 版本檢查
#define C_DELETE_BOOKMARK               4   // 刪除書籤
#define C_EXIT_GHOST                    5   // 退出幽靈模式
#define C_BUY_SPELL                     6   // 購買魔法
#define C_REQUEST_ROLL                  7   // 請求骰子
#define C_MONITOR_CONTROL               9   // 監控控制
#define C_SMS                           10  // 簡訊
#define C_LEAVE_PLEDGE                  11  // 離開誓言
#define C_CHANGE_PASSWORD               12  // 修改密碼
#define C_QUERY_CASTLE_SECURITY         14  // 查詢城堡安全
#define C_READ_NOTICE                   15  // 讀取公告
#define C_NEW_ACCOUNT                   16  // 新帳戶
#define C_KICK                          17  // 踢出
#define C_EXTENDED_PROTOBUF             21  // 擴展Protobuf封包
#define C_GET                           22  // 獲取物品
#define C_CREATE_CUSTOM_CHARACTER       23  // 創建自定義角色
#define C_LEAVE_PARTY                   24  // 離開隊伍
#define C_DELETE_CHARACTER              26  // 刪除角色
#define C_LOGIN_RESULT                  27  // 登入結果
#define C_ONOFF                         28  // 開/關
#define C_EXCHANGE_SPELL                29  // 交換魔法
#define C_CHANGE_CASTLE_SECURITY        30  // 修改城堡安全
#define C_CLIENT_READY                  32  // 客戶端準備就緒
#define C_UPLOAD_EMBLEM                 33  // 上傳紋章
#define C_BUYABLE_SPELL                 35  // 可購買魔法
#define C_RESTART                       37  // 重新啟動
#define C_CHECK_INVENTORY               38  // 檢查庫存
#define C_EXCHANGEABLE_SPELL            41  // 可交換魔法
#define C_ARCHERARRANGE                 42  // 弓手排列
#define C_BOARD_LIST                    44  // 佈告欄列表
#define C_MARRIAGE                      45  // 婚姻
#define C_CHANGE_ACCOUNTINFO            46  // 修改帳戶信息
#define C_JOIN_PLEDGE                   48  // 加入誓言
#define C_THROW                         49  // 丟擲
#define C_TAX                           50  // 稅收
#define C_WITHDRAW                      51  // 取出
#define C_LOGIN                         52  // 登入
#define C_SAVEIO                        53  // 保存I/O
#define C_SAY                           54  // 說話
#define C_GIVE                          56  // 給予
#define C_MERCENARYEMPLOY               57  // 雇用傭兵
#define C_DEAD_RESTART                  58  // 死亡重啟
#define C_ADD_XCHG                      59  // 添加交換
#define C_VOICE_CHAT                    60  // 語音聊天
#define C_PLATE                         61  // 盤子
#define C_WHO_PLEDGE                    64  // 誰是誓言
#define C_EMBLEM                        67  // 紋章
#define C_REMOVE_BUDDY                  70  // 移除好友
#define C_ALT_ATTACK                    72  // 備選攻擊
#define C_MERCENARYARRANGE              76  // 傭兵排列
#define C_ENTER_PORTAL                  77  // 進入傳送門
#define C_CHAT_PARTY_CONTROL            82  // 聊天隊伍控制
#define C_SERVER_SELECT                 85  // 伺服器選擇
#define C_CHANGE_DIRECTION              86  // 改變方向
#define C_QUIT                          87  // 退出
#define C_LOGIN_TEST                    89  // 登入測試
#define C_TELEPORT                      92  // 傳送
#define C_FIX                           94  // 修復
#define C_DROP                          96  // 掉落
#define C_RANK_CONTROL                  97  // 排名控制
#define C_BAN                           100 // 封禁
#define C_WANTED                        101 // 通緝
#define C_TITLE                         103 // 標題
#define C_SUMMON                        104 // 召喚
#define C_MERCENARYNAME                 106 // 傭兵名稱
#define C_CREATE_PLEDGE                 107 // 創建誓言
#define C_CONTROL_WEATHER               108 // 控制天氣
#define C_MOVE                          110 // 移動
#define C_START_CASTING                 112 // 開始施法
#define C_DIALOG                        115 // 對話
#define C_MERCENARYSELECT               116 // 傭兵選擇
#define C_TELEPORT_USER                 118 // 傳送用戶
#define C_EXTENDED_HYBRID               119 // 擴展混合
#define C_HYPERTEXT_INPUT_RESULT        120 // 超文本輸入結果
#define C_REGISTER_QUIZ                 122 // 註冊測驗
#define C_INVITE_PARTY_TARGET           123 // 邀請隊伍目標
#define C_BUY_SELL                      124 // 買/賣
#define C_PLEDGE_WATCH                  125 // 誓言監視
#define C_CHANNEL                       128 // 頻道
#define C_ACTION                        129 // 動作
#define C_NPC_ITEM_CONTROL              130 // NPC物品控制
#define C_QUERY_PERSONAL_SHOP           131 // 查詢個人商店
#define C_SAVE                          134 // 保存
#define C_USE_SPELL                     136 // 使用魔法
#define C_ACCEPT_XCHG                   137 // 接受交換
#define C_USE_ITEM                      141 // 使用物品
#define C_HACTION                       142 // 行動
#define C_FAR_ATTACK                    146 // 遠距攻擊
#define C_OPEN                          147 // 打開
#define C_EXCLUDE                       149 // 排除
#define C_ADDR                          150 // 位址
#define C_WHO                           154 // 誰
#define C_MAIL                          156 // 郵件
#define C_ALIVE                         158 // 存活
#define C_ADD_BUDDY                     159 // 添加好友
#define C_ENTER_WORLD                   163 // 進入世界
#define C_CHAT                          164 // 聊天
#define C_EXTENDED                      169 // 擴展
#define C_SLAVE_CONTROL                 171 // 奴隸控制
#define C_SELECTABLE_TIME               172 // 可選時間
#define C_SELECT_TIME                   174 // 選擇時間
#define C_PERSONAL_SHOP                 176 // 個人商店
#define C_RETURN_SUMMON                 181 // 返回召喚
#define C_ENTER_SHIP                    184 // 進入船隻
#define C_BLINK                         186 // 眨眼
#define C_BOOKMARK                      187 // 書籤
#define C_BOARD_DELETE                  189 // 佈告欄刪除
#define C_BOARD_WRITE                   190 // 佈告欄寫入
#define C_ANSWER                        191 // 回答
#define C_WISH                          193 // 願望
#define C_BOARD_READ                    194 // 佈告欄讀取
#define C_CANCEL_XCHG                   196 // 取消交換
#define C_INCLUDE                       198 // 包含
#define C_CHECK_PK                      200 // 檢查PK
#define C_DESTROY_ITEM                  201 // 銷毀物品
#define C_LOGOUT                        202 // 登出
#define C_SHIFT_SERVER                  203 // 移動伺服器
#define C_TELL                          207 // 告訴
#define C_INVITE_PARTY                  209 // 邀請隊伍
#define C_GOTO_MAP                      215 // 前往地圖
#define C_ASK_XCHG                      219 // 詢問交換
#define C_BOOK                          222 // 書籍
#define C_BUILDER_CONTROL               225 // 建築者控制
#define C_MATCH_MAKING                  226 // 配對
#define C_DUEL                          227 // 決鬥
#define C_DEPOSIT                       230 // 存款
#define C_WHO_PARTY                     233 // 誰在隊伍
#define C_GOTO_PORTAL                   234 // 前往傳送門
#define C_READ_NEWS                     235 // 讀取新聞
#define C_QUERY_BUDDY                   242 // 查詢好友
#define C_ATTACK_CONTINUE               244 // 持續攻擊
#define C_WAR                           246 // 戰爭
#define C_BANISH_PARTY                  247 // 放逐隊伍
#define C_SILENCE                       249 // 沉默
#define C_SHUTDOWN                      250 // 關閉
#define C_FIXABLE_ITEM                  251 // 可修復物品
#define C_ATTACK                        252 // 攻擊
#define C_WAREHOUSE_CONTROL             253 // 倉庫控制

// ============================================================================
// 舊版本的備選定義 (用於相容性，請參考Java註釋中的版本)
// ============================================================================
/*
#define C_BANCLAN                       0
#define C_CLIENTVERSION                 3
#define C_BOOKMARKDELETE                4
#define C_EXIT_GHOST                    5
#define C_SKILLBUYOK                    6
#define C_LEAVECLANE                    11
#define C_EXTENDED_PROTOBUF             21
#define C_PICKUPITEM                    22
#define C_NEWCHAR                       23
#define C_LEAVEPARTY                    24
#define C_DELETECHAR                    26
#define C_LOGINTOSERVEROK               28
#define C_SKILLBUYITEMOK                29
#define C_CLIENT_READY                  32
#define C_EMBLEMUPLOAD                  33
#define C_SKILLBUY                      35
#define C_CHANGECHAR                    37
#define C_PETMENU                       38
#define C_SKILLBUYITEM                  41
#define C_BOARDBACK                     44
#define C_PROPOSE                       45
#define C_JOINCLAN                      48
#define C_FISHCLICK                     49
#define C_TAXRATE                       50
#define C_DRAWAL                        51
#define C_LOGINPACKET                   52
#define C_CHARACTERCONFIG               53
#define C_GIVEITEM                      56
#define C_RESTART                       58
#define C_TRADEADDITEM                  59
#define C_CHARRESET                     60
#define C_BOARD                         61
#define C_PLEDGE                        64
#define C_EMBLEMDOWNLOAD                67
#define C_DELBUDDY                      70
#define C_ENTERPORTAL                   77
#define C_CAHTPARTY                     82
#define C_CHANGEHEADING                 86
#define C_QUITGAME                      87
#define C_SELECTLIST                    94
#define C_DROPITEM                      96
#define C_RANK                          97
#define C_CHAT                          100
#define C_TITLE                         103
#define C_CREATECLAN                    107
#define C_MOVECHAR                      110
#define C_NPCTALK                       115
#define C_CALL                          118
#define C_AMOUNT                        120
#define C_CREATEPARTY                   123
#define C_RESULT                        124
#define C_PLEDGE_WATCH                  125
#define C_ATTACK                        252
#define C_SENDLOCATION                  128
#define C_EXTCOMMAND                    129
#define C_USEPETITEM                    130
#define C_PRIVATESHOPLIST               131
#define C_USESKILL                      136
#define C_TRADEADDOK                    137
#define C_USEITEM                       141
#define C_NPCACTION                     142
#define C_DOOR                          147
#define C_EXCLUDE                       149
#define C_WHO                           154
#define C_MAIL                          156
#define C_KEEPALIVE                     158
#define C_ADDBUDDY                      159
#define C_LOGINTOSERVER                 163
#define C_CHATGLOBAL                    164
#define C_SELECTTARGET                  171
#define C_C_PERSONAL_SHOP               176
#define C_TELEPORT                      181
#define C_TELEPORTLOCK                  186
#define C_BOOKMARK                      187
#define C_BOARDDELETE                   189
#define C_BOARDWRITE                    190
#define C_ATTR                          191
#define C_BOARDREAD                     194
#define C_TRADEADDCANCEL                196
#define C_CHECKPK                       200
#define C_DELETEINVENTORYITEM           201
#define C_LOGOUT                        202
#define C_BEANFUNLOGINPACKET            203
#define C_CHATWHISPER                   207
#define C_SHIP                          215
#define C_TRADE                         219
#define C_ARROWATTACK                   146
#define C_FIGHT                         227
#define C_DEPOSIT                       230
#define C_PARTYLIST                     233
#define C_DUNGEONTELEPORT               234
#define C_COMMONCLICK                   235
#define C_BUDDYLIST                     242
#define C_ATTACKRUNING                  244
#define C_WAR                           246
#define C_BANPARTY                      247
#define C_FIRE_SMITH_P                  250
#define C_FIX_WEAPON_LIST               251
*/

// ============================================================================
// 匹配表：用於轉換舊opcode到新版本
// 如果需要支援舊客戶端，使用這些定義進行對映
// ============================================================================

// 舊版本到新版本的映射
#define C_OPCODE_BANCLAN                C_BAN_MEMBER
#define C_OPCODE_CLIENTVERSION          C_VERSION
#define C_OPCODE_BOOKMARKDELETE         C_DELETE_BOOKMARK
#define C_OPCODE_SKILLBUYOK             C_BUY_SPELL
#define C_OPCODE_LEAVECLANE             C_LEAVE_PLEDGE
#define C_OPCODE_PICKUPITEM             C_GET
#define C_OPCODE_NEWCHAR                C_CREATE_CUSTOM_CHARACTER
#define C_OPCODE_LEAVEPARTY             C_LEAVE_PARTY
#define C_OPCODE_DELETECHAR             C_DELETE_CHARACTER
#define C_OPCODE_LOGINTOSERVEROK        C_LOGIN_RESULT
#define C_OPCODE_SKILLBUYITEMOK         C_EXCHANGE_SPELL
#define C_OPCODE_EMBLEMUPLOAD           C_UPLOAD_EMBLEM
#define C_OPCODE_SKILLBUY               C_BUYABLE_SPELL
#define C_OPCODE_CHANGECHAR             C_SERVER_SELECT
#define C_OPCODE_PETMENU                C_CHECK_INVENTORY
#define C_OPCODE_SKILLBUYITEM           C_EXCHANGEABLE_SPELL
#define C_OPCODE_BOARDBACK              C_BOARD_LIST
#define C_OPCODE_PROPOSE                C_MARRIAGE
#define C_OPCODE_JOINCLAN               C_JOIN_PLEDGE
#define C_OPCODE_FISHCLICK              C_THROW
#define C_OPCODE_TAXRATE                C_TAX
#define C_OPCODE_DRAWAL                 C_WITHDRAW
#define C_OPCODE_LOGINPACKET            C_LOGIN
#define C_OPCODE_CHARACTERCONFIG        C_SAVEIO
#define C_OPCODE_GIVEITEM               C_GIVE
#define C_OPCODE_TRADEADDITEM           C_ADD_XCHG
#define C_OPCODE_CHARRESET              C_VOICE_CHAT
#define C_OPCODE_BOARD                  C_PLATE
#define C_OPCODE_PLEDGE                 C_WHO_PLEDGE
#define C_OPCODE_EMBLEMDOWNLOAD         C_EMBLEM
#define C_OPCODE_DELBUDDY               C_REMOVE_BUDDY
#define C_OPCODE_ENTERPORTAL            C_ENTER_PORTAL
#define C_OPCODE_CAHTPARTY              C_CHAT_PARTY_CONTROL
#define C_OPCODE_CHANGEHEADING          C_CHANGE_DIRECTION
#define C_OPCODE_QUITGAME               C_QUIT
#define C_OPCODE_SELECTLIST             C_FIX
#define C_OPCODE_DROPITEM               C_DROP
#define C_OPCODE_RANK                   C_RANK_CONTROL
#define C_OPCODE_CHAT                   C_BAN
#define C_OPCODE_CREATECLAN             C_CREATE_PLEDGE
#define C_OPCODE_MOVECHAR               C_MOVE
#define C_OPCODE_NPCTALK                C_DIALOG
#define C_OPCODE_CALL                   C_TELEPORT_USER
#define C_OPCODE_AMOUNT                 C_HYPERTEXT_INPUT_RESULT
#define C_OPCODE_CREATEPARTY            C_INVITE_PARTY_TARGET
#define C_OPCODE_RESULT                 C_BUY_SELL
#define C_OPCODE_SENDLOCATION           C_CHANNEL
#define C_OPCODE_EXTCOMMAND             C_ACTION
#define C_OPCODE_USEPETITEM             C_NPC_ITEM_CONTROL
#define C_OPCODE_PRIVATESHOPLIST        C_QUERY_PERSONAL_SHOP
#define C_OPCODE_USESKILL               C_USE_SPELL
#define C_OPCODE_TRADEADDOK             C_ACCEPT_XCHG
#define C_OPCODE_USEITEM                C_USE_ITEM
#define C_OPCODE_NPCACTION              C_HACTION
#define C_OPCODE_ARROWATTACK            C_FAR_ATTACK
#define C_OPCODE_DOOR                   C_OPEN
#define C_OPCODE_EXCLUDE                C_EXCLUDE
#define C_OPCODE_WHO                    C_WHO
#define C_OPCODE_MAIL                   C_MAIL
#define C_OPCODE_KEEPALIVE              C_ALIVE
#define C_OPCODE_ADDBUDDY               C_ADD_BUDDY
#define C_OPCODE_LOGINTOSERVER          C_ENTER_WORLD
#define C_OPCODE_CHATGLOBAL             C_CHAT
#define C_OPCODE_SELECTTARGET           C_SLAVE_CONTROL
#define C_OPCODE_C_PERSONAL_SHOP        C_PERSONAL_SHOP
#define C_OPCODE_TELEPORT               C_ADDR
#define C_OPCODE_TELEPORTLOCK           C_BLINK
#define C_OPCODE_BOOKMARK               C_BOOKMARK
#define C_OPCODE_BOARDDELETE            C_BOARD_DELETE
#define C_OPCODE_BOARDWRITE             C_BOARD_WRITE
#define C_OPCODE_ATTR                   C_ANSWER
#define C_OPCODE_BOARDREAD              C_BOARD_READ
#define C_OPCODE_TRADEADDCANCEL         C_CANCEL_XCHG
#define C_OPCODE_CHECKPK                C_CHECK_PK
#define C_OPCODE_DELETEINVENTORYITEM    C_DESTROY_ITEM
#define C_OPCODE_LOGOUT                 C_LOGOUT
#define C_OPCODE_BEANFUNLOGINPACKET     C_SHIFT_SERVER
#define C_OPCODE_CHATWHISPER            C_TELL
#define C_OPCODE_SHIP                   C_GOTO_MAP
#define C_OPCODE_TRADE                  C_ASK_XCHG
#define C_OPCODE_FIGHT                  C_DUEL
#define C_OPCODE_DEPOSIT                C_DEPOSIT
#define C_OPCODE_PARTYLIST              C_WHO_PARTY
#define C_OPCODE_DUNGEONTELEPORT        C_GOTO_PORTAL
#define C_OPCODE_COMMONCLICK            C_READ_NEWS
#define C_OPCODE_BUDDYLIST              C_QUERY_BUDDY
#define C_OPCODE_ATTACKRUNING           C_ATTACK_CONTINUE
#define C_OPCODE_WAR                    C_WAR
#define C_OPCODE_BANPARTY               C_BANISH_PARTY
#define C_OPCODE_FIRE_SMITH_P           C_SHUTDOWN
#define C_OPCODE_FIX_WEAPON_LIST        C_FIXABLE_ITEM

// ============================================================================
// SERVER OPCODES (伺服器操作碼)
// ============================================================================
// 這些是伺服器傳送給客戶端的操作碼
// 對應於Java中可能存在的OpcodesServer類

#define S_OPCODE_EMBLEM                 2
#define S_OPCODE_POLY                   3
#define S_OPCODE_PACKETBOX              5
#define S_OPCODE_WAR                    6
#define S_OPCODE_SHOWRETRIEVELIST       7
#define S_OPCODE_GLOBALCHAT             8
#define S_OPCODE_POISON                 10
#define S_OPCODE_REMOVE_OBJECT          11
#define S_OPCODE_ABILITY                12
#define S_OPCODE_COMMONNEWS             14
#define S_OPCODE_TELEPORTLOCK           16
#define S_OPCODE_WHISPERCHAT            17
#define S_OPCODE_NEWMASTER              18
#define S_OPCODE_OWNCHARSTATUS          19
#define S_OPCODE_ADDITEM                20
#define S_OPCODE_CRAFTSYSTEM            24
#define S_OPCODE_IDENTIFYDESC           30
#define S_OPCODE_ITEMSTATUS             32
#define S_OPCODE_DEPOSIT                33
#define S_OPCODE_OBJECTPACK             34
#define S_OPCODE_LOGINTOGAME            36
#define S_OPCODE_UPDATECLANID           37
#define S_OPCODE_SKILLBUYITEM           42
#define S_OPCODE_INITPACKET             46
#define S_OPCODE_RESTART                47
#define S_OPCODE_DRAWAL                 48
#define S_OPCODE_EFFECTLOCATION         49
#define S_OPCODE_ITEMCOLOR              52
#define S_OPCODE_CHANGEHEADING          53
#define S_OPCODE_MATERIAL               57

#endif // OPCODES_H
