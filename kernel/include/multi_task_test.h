#pragma once

#ifdef __MULTI_TASK_TEST_WITHOUT_SCHEDUL__
/*************************************单次任务切换测试****************************/
// 在main任务的定时回调函数中调用
void multi_task_test_in_main_timer_callback(void);

// 测试
void multi_task_test(void);

/*************************************交替打印AB测试****************************/
// 在main任务的定时回调函数中调用
void multi_task_test_in_main_timer_callback_auto(void);

// 测试
void multi_task_test_auto(void);
#endif
