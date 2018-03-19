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
    
    //inc_context incContext;
    //inc_context stepIncContext;
    //inc_context timedStepIncContext;
    
    timer incTimer;
    
    HullType currentHullType;
};

static void InitializeHull(hull& h, vertex* vertices, int numberOfPoints, HullType hullType)
{
    h.vertices = vertices;
    h.numberOfPoints = numberOfPoints;
    
    h.qhContext.initialized = false;
    h.stepQhContext.initialized = false;
    h.timedStepQhContext.initialized = false;
    
    //h.incContext.initialized = false;
    //h.stepIncContext.initialized = false;
    //h.timedStepIncContext.initialized = false;
    
    h.currentHullType = hullType;
}

static void ReinitializeHull(hull& h, vertex* vertices, int numberOfPoints)
{
    h.vertices = vertices;
    h.numberOfPoints = numberOfPoints;
    
    h.qhContext.initialized = false;
    h.stepQhContext.initialized = false;
    h.timedStepQhContext.initialized = false;
    
    //h.incContext.initialized = false;
    //h.stepIncContext.initialized = false;
    //h.timedStepIncContext.initialized = false;
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
            /*auto& incContext = h.timedStepIncContext;
            if(h.incTimer.running)
            {
                if(h.incTimer.currentTime <= 0.0)
                {
                    IncHullStep(renderContext, incContext);
                    h.incTimer.currentTime = h.incTimer.timerInit;
                }
                else
                {
                    h.incTimer.currentTime -= deltaTime;
                }
            }*/
            
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
            TIME_START;
            //QuickHull(renderContext, qhContext);
            auto& res = QuickHull(renderContext, qhContext.vertices, qhContext.numberOfPoints);
            TIME_END("Full hull");
            return res;
        }
        break;
        case Inc:
        {
            /*auto& incContext = h.incContext;
            if(!incContext.initialized)
            {
                InitializeIncContext(incContext, h.vertices, h.numberOfPoints);
            }
            auto& res = IncHull(renderContext, incContext);
            
            return res;*/
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
            /*auto& incContext = h.stepIncContext;
            if(!incContext.initialized)
            {
                InitializeIncContext(incContext, h.vertices, h.numberOfPoints);
            }
            IncHullStep(renderContext, incContext);
            return incContext.m;*/
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
            /*auto& incContext = h.timedStepIncContext;
            if(!incContext.initialized)
            {
                InitializeIncContext(incContext, h.vertices, h.numberOfPoints);
            }
            
            h.incTimer.running = !h.incTimer.running;
            return incContext.m;*/
        }
        break;
        case RInc:
        {
        }
        break;
    }
}

#endif


