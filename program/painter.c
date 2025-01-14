#include "api.h"
#include "colo8.h"
#include "pdef.h"
#include "pdraw.h"
#include "ptimer.h"
#include "putil.h"

#define DIRECTION_UP 1
#define DIRECTION_DOWN 2
#define DIRECTION_LEFT 4
#define DIRECTION_RIGHT 8

#define MOVE_BOX_COUNT 10 // 盒子数量
#define MIN_BOX_SIZE 16   // 盒子最小宽高
#define MAX_BOX_SIZE 24   // 盒子最大宽高
#define MIN_VELOCITY 4    // 最小移动像素
#define MAX_VELOCITY 8    // 最大移动像素

#define RAND_SEED_X 470387
#define RAND_SEED_Y 489237
#define RAND_SEED_SIZE 234098
#define RAND_SEED_COLOR 38732
#define RAND_SEED_VELOCITY 965723
#define RAND_SEED_DIRECTION 68320

typedef struct {
    unsigned int m_x;          // 位置
    unsigned int m_y;          // 大小
    unsigned int m_size;       // 盒子大小
    unsigned char m_color;     // 颜色
    unsigned char m_velocity;  // 速度
    unsigned char m_direction; // 方向
} moving_box_t;

static void _draw_body_background(unsigned int win, unsigned int w,
                                  unsigned int h) {
    api_draw_box_in_window(
        win, WINDOW_BORDER_SIZE, WINDOW_TITLE_BAR_HEIGHT + WINDOW_BORDER_SIZE,
        w - WINDOW_BORDER_SIZE - 1, h - WINDOW_BORDER_SIZE - 1, COLOR_BLACK);
}

static void _refresh_body(unsigned int win, unsigned char *winbuf,
                          unsigned int w, unsigned int h) {
    api_cover_window_sheet(win, 0, 0, w - 1, h - 1, winbuf);
    api_refresh_window(win, 0, 0, w, h);
}

static void _init_game(unsigned int w, unsigned int h, moving_box_t *boxs) {
    for (unsigned int i = 0; i < MOVE_BOX_COUNT; i++) {
        unsigned int size = MIN_BOX_SIZE + api_rand_uint(i * RAND_SEED_SIZE) %
                                               (MAX_BOX_SIZE - MIN_BOX_SIZE);

        unsigned int x = max_unsigned(WINDOW_BORDER_SIZE,
                                      api_rand_uint(i * RAND_SEED_X) %
                                          (w - size - WINDOW_BORDER_SIZE));
        unsigned int y = max_unsigned(
            WINDOW_TITLE_BAR_HEIGHT + WINDOW_BORDER_SIZE,
            api_rand_uint(i * RAND_SEED_Y) % (h - size - WINDOW_BORDER_SIZE));

        unsigned char color = (unsigned char)max_unsigned(
            1, (api_rand_uint(i * RAND_SEED_COLOR) % (COLOR_VISIBLE_MAX + 1)));

        unsigned char velocity =
            (unsigned char)(MIN_VELOCITY +
                            api_rand_uint(i * RAND_SEED_VELOCITY) %
                                (MAX_VELOCITY + 1 - MIN_VELOCITY));

        unsigned char direction =
            (unsigned char)((api_rand_uint(i * RAND_SEED_DIRECTION) % 2 == 0
                                 ? DIRECTION_UP
                                 : DIRECTION_DOWN) |
                            (api_rand_uint(i * RAND_SEED_DIRECTION * 2) % 2 == 0
                                 ? DIRECTION_LEFT
                                 : DIRECTION_RIGHT));

        boxs[i].m_x = x, boxs[i].m_y = y;
        boxs[i].m_size = size, boxs[i].m_color = color;
        boxs[i].m_velocity = velocity, boxs[i].m_direction = direction;
    }
}

static void _draw_scene(unsigned int win, unsigned char *winbuf, unsigned int w,
                        unsigned int h, moving_box_t *boxs) {
    for (unsigned int i = 0; i < MOVE_BOX_COUNT; i++) {
        moving_box_t *p = &boxs[i];
        boxfill8(winbuf, w, p->m_color, p->m_x, p->m_y, p->m_x + p->m_size,
                 p->m_y + p->m_size);
    }
    _refresh_body(win, winbuf, w, h);
}

static void _update_scene(unsigned int w, unsigned int h, moving_box_t *boxs) {
    unsigned int sx = WINDOW_BORDER_SIZE,
                 sy = WINDOW_TITLE_BAR_HEIGHT + WINDOW_BORDER_SIZE;
    unsigned int ex = w - WINDOW_BORDER_SIZE - 1,
                 ey = h - WINDOW_BORDER_SIZE - 1;

    for (unsigned int i = 0; i < MOVE_BOX_COUNT; i++) {
        int x, y;
        moving_box_t *p = &boxs[i];

        // 检测与上下边框是否碰撞
        if (p->m_direction & DIRECTION_UP) {
            y = (int)p->m_y - p->m_velocity;
            if (y < (int)sy) {
                p->m_y = sy;
                p->m_direction = p->m_direction ^ DIRECTION_UP | DIRECTION_DOWN;
            } else {
                p->m_y = (unsigned int)y;
            }
        } else if (p->m_direction & DIRECTION_DOWN) {
            y = (int)p->m_y + p->m_velocity;
            if (y > (int)(ey - p->m_size)) {
                p->m_y = ey - p->m_size;
                p->m_direction = p->m_direction ^ DIRECTION_DOWN | DIRECTION_UP;
            } else {
                p->m_y = (unsigned int)y;
            }
        }

        // 检测与左右边框是否碰撞
        if (p->m_direction & DIRECTION_LEFT) {
            x = (int)p->m_x - p->m_velocity;
            if (x < (int)sx) {
                p->m_x = sx;
                p->m_direction =
                    p->m_direction ^ DIRECTION_LEFT | DIRECTION_RIGHT;
            } else {
                p->m_x = (unsigned int)x;
            }
        } else if (p->m_direction & DIRECTION_RIGHT) {
            x = (int)p->m_x + p->m_velocity;
            if (x > (int)(ex - p->m_size)) {
                p->m_x = ex - p->m_size;
                p->m_direction =
                    p->m_direction ^ DIRECTION_RIGHT | DIRECTION_LEFT;
            } else {
                p->m_x = (unsigned int)x;
            }
        }
    }
}

static void _run_game(unsigned int win, unsigned char *winbuf, unsigned int w,
                      unsigned int h, unsigned int timer, moving_box_t *boxs) {
    while (true) {
        if (api_is_close_window())
            break;

        _draw_scene(win, winbuf, w, h, boxs);
        _update_scene(w, h, boxs);

        timer_wait(timer);
    }
}

void main(void) {
    unsigned int w = 400, h = 300;
    unsigned char winbuf[400 * 300];
    moving_box_t boxs[MOVE_BOX_COUNT];
    unsigned int win = api_new_window(100, 100, w, h, "Paiter");
    _draw_body_background(win, w, h);

    api_dump_window_sheet(win, 0, 0, w - 1, h - 1, winbuf);

    unsigned int timer = api_timer_alloc();
    api_timer_set(timer, 4, TIMER_MAX_RUN_COUNTS);

    _init_game(w, h, (moving_box_t *)&boxs);
    _run_game(win, winbuf, w, h, timer, (moving_box_t *)boxs);

    api_timer_free(timer);
    api_close_window(win);
}
