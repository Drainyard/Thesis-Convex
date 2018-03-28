#define Time(msg, func) \
{\
    auto before = glfwGetTime();\
    func;\
    auto after = glfwGetTime();\
    auto total = after - before;\
    Log_A("%s took %f time\n",msg, total);\
}

static double TIMER_before = 0.0;
static double TIMER_after = 0.0;
static double TIMER_total = 0.0;

#define TIME_START TIMER_before = glfwGetTime();

#define TIME_END(msg) TIMER_after = glfwGetTime(); \
TIMER_total = TIMER_after - TIMER_before;\
Log_A("%s took %f time\n",msg, TIMER_total);

#define TIME_START_ACC TIMER_before = glfwGetTime();

#define TIME_END_ACC(msg) TIMER_after = glfwGetTime(); \
static double TIMER_total_acc = 0.0; \
TIMER_total_acc += TIMER_after - TIMER_before;\
Log_A("%s took %f time total\n", msg, TIMER_total_acc);
