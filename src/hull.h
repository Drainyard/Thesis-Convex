#ifndef HULL_H
#define HULL_H


enum HullType
{
    QH,
    Inc,
    RInc
};

struct vertex;

struct timer
{
    double timerInit;
    double currentTime;
    bool running;
};

struct hull
{
    vertex* vertices;
    int numberOfPoints;
    
    qh_context qhContext;
    qh_context stepQhContext;
    qh_context timedStepQhContext;
    
    timer qhTimer;
    
    HullType currentHullType;
};

static void InitializeHull(hull& h, vertex* vertices, int numberOfPoints, HullType hullType)
{
    h.vertices = vertices;
    h.numberOfPoints = numberOfPoints;
    h.qhContext.initialized = false;
    h.stepQhContext.initialized = false;
    h.timedStepQhContext.initialized = false;
    h.currentHullType = hullType;
}

static void ReinitializeHull(hull& h, vertex* vertices, int numberOfPoints)
{
    h.vertices = vertices;
    h.numberOfPoints = numberOfPoints;
    h.qhContext.initialized = false;
    h.stepQhContext.initialized = false;
    h.timedStepQhContext.initialized = false;
}

static void UpdateHull(render_context& renderContext, hull& h, HullType hullType, double deltaTime)
{
    h.currentHullType = hullType;
    
    switch(h.currentHullType)
    {
        case QH:
        {
            auto& qhContext = h.timedStepQhContext;
            if(h.qhTimer.running)
            {
                if(h.qhTimer.currentTime <= 0.0)
                {
                    QuickHullStep(renderContext, qhContext);
                    h.qhTimer.currentTime = h.qhTimer.timerInit;
                }
                else
                {
                    h.qhTimer.currentTime -= deltaTime;
                }
            }
        }
        break;
        case Inc:
        {
        }
        break;
        case RInc:
        {
        }
        break;
    }
}

static mesh& FullHull(render_context& renderContext, hull& h)
{
    switch(h.currentHullType)
    {
        case QH:
        {
            auto& qhContext = h.qhContext;
            if(!qhContext.initialized)
            {
                InitializeQHContext(qhContext, h.vertices, h.numberOfPoints);
            }
            //TIME_START;
            auto& res = QuickHull(renderContext, qhContext.vertices, qhContext.numberOfPoints);
            //TIME_END("Full hull\n");
            //QuickHull(renderContext, qhContext);
            return res;
        }
        break;
        case Inc:
        {
        }
        break;
        case RInc:
        {
        }
        break;
    }
    
}

static mesh& StepHull(render_context& renderContext, hull& h)
{
    switch(h.currentHullType)
    {
        case QH:
        {
            auto& qhContext = h.stepQhContext;
            if(!qhContext.initialized)
            {
                InitializeQHContext(qhContext, h.vertices, h.numberOfPoints);
            }
            QuickHullStep(renderContext, qhContext);
            return qhContext.m;
        }
        break;
        case Inc:
        {
        }
        break;
        case RInc:
        {
        }
        break;
    }
}

static mesh& TimedStepHull(hull& h)
{
    switch(h.currentHullType)
    {
        case QH:
        {
            auto& qhContext = h.timedStepQhContext;
            if(!qhContext.initialized)
            {
                InitializeQHContext(qhContext, h.vertices, h.numberOfPoints);
            }
            
            h.qhTimer.running = !h.qhTimer.running;
            return qhContext.m;
        }
        break;
        case Inc:
        {
        }
        break;
        case RInc:
        {
        }
        break;
    }
}

#endif


