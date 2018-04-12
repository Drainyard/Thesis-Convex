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
    
    inc_context incContext;
    //inc_context stepIncContext;
    //inc_context timedStepIncContext;
    
    timer incTimer;
    
    HullType currentHullType;
    
    point_generator pointGenerator;
};


void writeHullToFile(const char* filename, const char* typeString, int facesAdded, int totalFaceCount, int vertexCount, int pointsProcessed, int distanceQueryCount, int verticesInHull, double timeSpent, GeneratorType generateType)
{
    FILE* f = fopen(filename, "a+");
    
    if(f)
    {
        fprintf(f, "Hull result for %s\n", typeString);
        fprintf(f, " Faces added:      %d\n", facesAdded);
        fprintf(f, " Faces in hull:    %d\n", totalFaceCount);
        fprintf(f, " Input vertices:   %d\n", vertexCount);
        fprintf(f, " Points processed: %d\n", pointsProcessed);
        fprintf(f, " Distance queries: %d\n", distanceQueryCount);
        fprintf(f, " Vertices in hull: %d\n", verticesInHull);
        fprintf(f, " Time spent:       %lf\n", timeSpent);
        switch(generateType)
        {
            case GeneratorType::InSphere:
            {
                fprintf(f, " Point generation: In Sphere\n");
            }
            break;
            case GeneratorType::OnSphere:
            {
                fprintf(f, " Point generation: On Sphere\n");
            }
            break;
            case GeneratorType::InCube:
            {
                fprintf(f, " Point generation: In Cube\n");
            }
            break;
            case GeneratorType::NormalizedSphere:
            {
                fprintf(f, " Point generation: On Normalized Sphere\n");
            }
            break;
            case GeneratorType::ManyInternal:
            {
                fprintf(f, " Point generation: Many internal, some on sphere\n");
            }
            break;
        }
        fclose(f);
    }
}

static void InitializeHull(hull& h, vertex* vertices, int numberOfPoints, HullType hullType)
{
    h.vertices = vertices;
    h.numberOfPoints = numberOfPoints;
    
    h.qhContext.initialized = false;
    h.stepQhContext.initialized = false;
    h.timedStepQhContext.initialized = false;
    
    h.incContext.initialized = false;
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
    
    h.incContext.initialized = false;
    //h.stepIncContext.initialized = false;
    //h.timedStepIncContext.initialized = false;
}

static mesh* UpdateHull(render_context& renderContext, hull& h, HullType hullType, double deltaTime)
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
                    qhStep(qhContext);
                    h.qhTimer.currentTime = h.qhTimer.timerInit;
                    return &qhConvertToMesh(renderContext, qhContext.qHull, h.vertices);
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
    return nullptr;
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
                qhInitializeContext(qhContext, h.vertices, h.numberOfPoints);
            }
            
            qhFullHull(qhContext);
            
            writeHullToFile("hull_out", "QuickHull", qhContext.qHull.processingState.addedFaces, (int)qhContext.qHull.faces.size(), h.numberOfPoints, qhContext.qHull.processingState.pointsProcessed, qhContext.qHull.processingState.distanceQueryCount, qhContext.qHull.processingState.verticesInHull, qhContext.qHull.processingState.timeSpent, h.pointGenerator.type);
            
            return qhConvertToMesh(renderContext, qhContext.qHull, h.vertices);
        }
        break;
        case Inc:
        {
            auto& incContext = h.incContext;
            if(!incContext.initialized)
            {
                incInitializeContext(incContext, h.vertices, h.numberOfPoints);
            }
            auto timerIndex = startTimer();
            incConstructFullHull();
            TIME_END(timerIndex, "Full hull");
            return incConvertToMesh(renderContext);
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
                qhInitializeContext(qhContext, h.vertices, h.numberOfPoints);
            }
            qhStep(qhContext);
            return qhConvertToMesh(renderContext, qhContext.qHull, h.vertices);
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

static mesh& TimedStepHull(render_context& renderContext, hull& h)
{
    switch(h.currentHullType)
    {
        case QH:
        {
            auto& qhContext = h.timedStepQhContext;
            if(!qhContext.initialized)
            {
                qhInitializeContext(qhContext, h.vertices, h.numberOfPoints);
            }
            
            h.qhTimer.running = !h.qhTimer.running;
            return qhConvertToMesh(renderContext, qhContext.qHull, h.vertices);
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

static bool TimerRunning(hull& h)
{
    return h.qhTimer.running || h.incTimer.running;
}


#endif


