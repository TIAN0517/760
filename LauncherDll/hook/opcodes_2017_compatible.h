#pragma once

/**
 * 2017 Compatible Opcodes Definition
 * 為舊版 LinLogin.bin (2017/6/7) 設計的簡化 opcodes
 * 
 * 策略: 最小修改原則
 * - 只修改確實需要的關鍵登入 opcodes
 * - 保持其他值不變，避免意外衝突
 * - 移除可能導致崩潰的新功能
 */

// ============================================================================
// 2017 COMPATIBLE CLIENT OPCODES (簡化版)
// ============================================================================

// 保留原始加密標記
static const unsigned char FIRST_PACKET[] = {3, (unsigned char)-35, 10, 51, (unsigned char)-57, 47, (unsigned char)-16, 59};

// === 核心登入 Opcodes (最小必要修改) ===
#define C_VERSION                       245 // Java需要: 245 (原: 3)
#define C_LOGIN                         71  // Java需要: 71 (原: 52)  
#define C_CLIENT_READY                  144 // Java需要: 144 (原: 32)

// === 保持原值 (避免2017版本衝突) ===
#define C_ENTER_WORLD                   163 // 保持原值 (不改為131)
#define C_LOGOUT                        202 // 保持原值 (不改為56)
#define C_ONOFF                         28  // 保持原值 (不改為84)
#define C_ALIVE                         158 // 保持原值 (不改為149)

// === 移除可能導致崩潰的新功能 ===
// 註解掉可能不相容的新 opcodes
// #define C_EXTENDED_PROTOBUF          230 // 可能導致2017版本崩潰

// === 其他常用 Opcodes (保持原值) ===
#define C_BAN_MEMBER                    0
#define C_DELETE_BOOKMARK               4
#define C_EXIT_GHOST                    5
#define C_BUY_SPELL                     6
#define C_REQUEST_ROLL                  7
#define C_MONITOR_CONTROL               9
#define C_SMS                           10
#define C_LEAVE_PLEDGE                  11
#define C_CHANGE_PASSWORD               12
#define C_QUERY_CASTLE_SECURITY         14
#define C_READ_NOTICE                   15
#define C_NEW_ACCOUNT                   16
#define C_KICK                          17
#define C_GET                           22
#define C_CREATE_CUSTOM_CHARACTER       23
#define C_LEAVE_PARTY                   24
#define C_DELETE_CHARACTER              26
#define C_LOGIN_RESULT                  27

// === 基本動作 Opcodes ===
#define C_ATTACK                        29
#define C_MOVE                          30
#define C_SAY                           31
#define C_RANK                          33
#define C_BOARD_READ                    34
#define C_CRAFT                         35

// === 物品系統 ===
#define C_DROP                          36
#define C_TRADE                         37
#define C_PICKUP                        38
#define C_USE_ITEM                      39

// ============================================================================
// SERVER OPCODES (最小修改)
// ============================================================================

// 只保留必要的伺服器回應 opcodes
#define S_LOGIN_RESULT                  0
#define S_DISCONNECT                    1
#define S_CHAR_PACK                     5
#define S_NEW_CHAR_PACK                 6
#define S_DELETE_CHAR_OK                7

// 保持基本伺服器功能
#define S_VERSION                       25
#define S_ALIVE                         27
#define S_PING                          28

/**
 * 使用說明:
 * 1. 這個版本專為 2017年的 LinLogin.bin 設計
 * 2. 保持了原始架構，只修改必要的登入相關 opcodes
 * 3. 移除了可能導致崩潰的新功能
 * 4. 如果仍然崩潰，可能需要完全使用原始 opcodes
 */