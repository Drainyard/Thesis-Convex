#define MAX_TIMERS 4096

static unsigned long long TIMERS_before[MAX_TIMERS] = {};
static int counter = 0;

int currentTimeMicro()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
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

double endTimer(int index)
{
    auto after = currentTimeMicro();
    return after - TIMERS_before[index];
}

#define TIME_END(index, msg) \
log_a("%s took %f time\n", msg, endTimer(index));
