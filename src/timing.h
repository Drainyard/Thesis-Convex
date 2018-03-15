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
