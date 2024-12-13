#pragma once

#define SHEET_USERDATA_TYPE_NONE 0
#define SHEET_USERDATA_TYPE_MOVING 1 // 移动窗口

typedef struct {
    unsigned char m_type;
    void *m_data;
} sheet_userdata_t;

typedef struct {
    void(m_cb *)(void *); // 移动窗口回调函数
    void *m_data;         // 毁掉函数的参数
} sheet_userdata_type_moving;
