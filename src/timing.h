#define MAX_TIMERS 4096

static double TIMERS_before[MAX_TIMERS] = {};
static int counter = 0;

int startTimer()
{
    if(counter >= MAX_TIMERS)
    {
        counter = 0;
        TIMERS_before[counter] = glfwGetTime();
        return counter;
    }
    TIMERS_before[counter] = glfwGetTime();
    return counter++;
}

double endTimer(int index)
{
    double after = glfwGetTime();
    return after - TIMERS_before[index];
}

#define TIME_END(index, msg) \
log_a("%s took %f time\n", msg, endTimer(index));
