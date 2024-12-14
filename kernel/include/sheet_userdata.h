#pragma once

#define SHEET_USERDATA_TYPE_NONE 0
#define SHEET_USERDATA_TYPE_MOVING 1 // 移动窗口

typedef struct {
    unsigned char m_type;
    void *m_data;
} sheet_userdata_t;

typedef void (*sheet_userdata_type_moving_cb)(void *);

typedef struct {
    sheet_userdata_type_moving_cb m_cb; // 移动窗口回调函数
    void *m_data; // 自定义参数, 不要设置毁掉函数，程序会异常推出
    const char *m_func_name; // 不使用回调函数时，使用该值调用对应的函数
} sheet_userdata_type_moving_t;

void sheet_userdata_set(sheet_userdata_t *p, unsigned char type, void *data);

sheet_userdata_type_moving_t *sheet_userdata_type_moving_alloc(void);

void sheet_userdata_type_moving_free(sheet_userdata_type_moving_t *p);

void sheet_userdata_type_moving_set(sheet_userdata_type_moving_t *p,
                                    sheet_userdata_type_moving_cb cb,
                                    void *data, const char *func_name);

// 移动鼠标下的图层
void moving_sheet(void);
