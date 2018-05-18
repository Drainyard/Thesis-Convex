#define MAX_TIMERS 4096

using time_long = unsigned long long;

static time_long TIMERS_before[MAX_TIMERS] = {};
static int counter = 0;

time_long currentTimeMicro()
{
    return (time_long)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

int startTimer()
{
    if(counter >= MAX_TIMERS)
    {
        counter = 0;
        
        TIMERS_before[counter] = currentTimeMicro();
        return counter;
    }
    TIMERS_before[counter] = currentTimeMicro();
    return counter++;
}

time_long endTimer(int index)
{
    auto after = currentTimeMicro();
    return after - TIMERS_before[index];
}

#define TIME_END(index, msg) \
log_a("%s took %lld time\n", msg, endTimer(index));
