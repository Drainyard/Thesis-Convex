#ifndef HULL_H
#define HULL_H

enum HullType
{
    QH,
    Inc,
    Dac
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
    vertex *vertices;
    int numberOfPoints;
    
    qh_context qhContext;
    qh_context stepQhContext;
    qh_context timedStepQhContext;
    
    timer qhTimer;
    
    inc_context incContext;
    inc_context stepIncContext;
    inc_context timedStepIncContext;
    
    timer incTimer;
    
    //dac_context dacContext;
    //dac_context stepDacContext;
    //dac_context timedStepDacContext;
    
    //timer dacTimer;
    
    HullType currentHullType;
    
    point_generator pointGenerator;
};

const char *GetGeneratorTypeString(GeneratorType type)
{
    
    switch (type)
    {
        case GeneratorType::InSphere:
        {
            return "In Sphere";
        }
        break;
        case GeneratorType::OnSphere:
        {
            return "On Sphere";
        }
        break;
        case GeneratorType::InCube:
        {
            return "In Cube";
        }
        break;
        case GeneratorType::NormalizedSphere:
        {
            return "On Normalized Sphere";
        }
        break;
        case GeneratorType::ManyInternal:
        {
            return "Many internal, some on sphere";
        }
        break;
    }
}

void WriteHullToCSV(const char *filename, const char *typeString, int facesAdded, int totalFaceCount, int vertexCount, int pointsProcessed, int distanceQueryCount, int verticesInHull, double timeSpent, GeneratorType generateType)
{
    char *fullFilename = concat(filename, ".csv");
#if defined(__linux)
    bool fileExists = access(fullFilename, F_OK) != -1;
#else
    auto fileExists = PathFileExists(fullFilename);
#endif
    FILE *f = fopen(fullFilename, "a+");
    
    if (f)
    {
        if (!fileExists)
        {
            fprintf(f, "type, faces added, faces in hull, input vertices, points processed, distance queries, vertices in hull, time spent\n");
        }
        
        fprintf(f, "%s, %d, %d, %d, %d, %d, %d, %lf, %s\n", typeString, facesAdded, totalFaceCount, vertexCount, pointsProcessed, distanceQueryCount, verticesInHull, timeSpent, GetGeneratorTypeString(generateType));
        fclose(f);
    }
    free(fullFilename);
}

void WriteHullToFile(const char *filename, const char *typeString, int facesAdded, int totalFaceCount, int vertexCount, int pointsProcessed, int distanceQueryCount, int verticesInHull, double timeSpent, GeneratorType generateType)
{
    FILE *f = fopen(filename, "a+");
    
    if (f)
    {
        
        fprintf(f, "Hull result for %s\n", typeString);
        fprintf(f, " Faces added:      %d\n", facesAdded);
        fprintf(f, " Faces in hull:    %d\n", totalFaceCount);
        fprintf(f, " Input vertices:   %d\n", vertexCount);
        fprintf(f, " Points processed: %d\n", pointsProcessed);
        fprintf(f, " Distance queries: %d\n", distanceQueryCount);
        fprintf(f, " Vertices in hull: %d\n", verticesInHull);
        fprintf(f, " Time spent:       %lf\n", timeSpent);
        fprintf(f, " Point generation: %s\n", GetGeneratorTypeString(generateType));
        
        fclose(f);
    }
}

static void InitializeHull(hull &h, vertex *vertices, int numberOfPoints, HullType hullType)
{
    h.vertices = vertices;
    h.numberOfPoints = numberOfPoints;
    
    h.qhContext = {};
    h.qhContext.initialized = false;
    h.stepQhContext.initialized = false;
    h.stepQhContext = {};
    h.timedStepQhContext.initialized = false;
    h.timedStepQhContext = {};
    
    h.incContext.initialized = false;
    h.incContext = {};
    
    //h.dacContext.initialized = false;
    //h.dacContext = {};
    
    h.currentHullType = hullType;
}

static void reinitializeHull(hull &h, vertex *vertices, int numberOfPoints)
{
    h.vertices = vertices;
    h.numberOfPoints = numberOfPoints;
    
    h.qhContext.initialized = false;
    h.stepQhContext.initialized = false;
    h.timedStepQhContext.initialized = false;
    
    h.incContext.initialized = false;
}

static mesh *UpdateHull(render_context &renderContext, hull &h, HullType hullType, double deltaTime)
{
    h.currentHullType = hullType;
    
    switch (h.currentHullType)
    {
        case QH:
        {
            auto &qhContext = h.timedStepQhContext;
            if (h.qhTimer.running)
            {
                if (h.qhTimer.currentTime <= 0.0)
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
            static bool init = true;
            
            if (h.incTimer.running)
            {
                if (h.incTimer.currentTime <= 0.0)
                {
                    if (init)
                    {
                        incInitStepHull();
                        init = false;
                    }
                    else
                    {
                        incHullStep();
                    }
                    auto &incContext = h.timedStepIncContext;
                    h.incTimer.currentTime = h.incTimer.timerInit;
                    return &incConvertToMesh(incContext, renderContext);
                }
                else
                {
                    h.incTimer.currentTime -= deltaTime;
                }
            }
        }
        break;
        /*case Dac:
        {
            auto &dacContext = h.timedStepDacContext;
            return &dacConvertToMesh(dacContext, renderContext);
        }
        break;*/
        default:
        break;
    }
    return nullptr;
}

static mesh &FullHull(render_context &renderContext, hull &h)
{
    switch (h.currentHullType)
    {
        case QH:
        {
            auto &qhContext = h.qhContext;
            if (!qhContext.initialized)
            {
                qhInitializeContext(qhContext, h.vertices, h.numberOfPoints);
            }
            
            auto timerIndex = startTimer();
            printf("index: %d\n", timerIndex);
            qhFullHull(qhContext);
            TIME_END(timerIndex, "Full quick hull");
            
            WriteHullToCSV("hull_out", "QuickHull", qhContext.qHull.processingState.addedFaces, (int)qhContext.qHull.faces.size(), h.numberOfPoints, qhContext.qHull.processingState.pointsProcessed, qhContext.qHull.processingState.distanceQueryCount, qhContext.qHull.processingState.verticesInHull, qhContext.qHull.processingState.timeSpent, h.pointGenerator.type);
            
            return qhConvertToMesh(renderContext, qhContext.qHull, h.vertices);
        }
        break;
        case Inc:
        {
            auto &incContext = h.incContext;
            if (!incContext.initialized)
            {
                incInitializeContext(incContext, h.vertices, h.numberOfPoints);
            }
            auto timerIndex = startTimer();
            incConstructFullHull();
            TIME_END(timerIndex, "Full inc hull");
            return incConvertToMesh(incContext, renderContext);
        }
        break;
        /*case Dac:
        {
            auto &dacContext = h.dacContext;
            if (!dacContext.initialized)
            {
                dacInitializeContext(dacContext, h.vertices, h.numberOfPoints);
            }
            auto timerIndex = startTimer();
            dacConstructFullHull();
            TIME_END(timerIndex, "Full dac hull");
            return dacConvertToMesh(dacContext, renderContext);
        }
        break;*/
        default:
        break;
    }
}

static mesh &StepHull(render_context &renderContext, hull &h)
{
    switch (h.currentHullType)
    {
        case QH:
        {
            auto &qhContext = h.stepQhContext;
            if (!qhContext.initialized)
            {
                qhInitializeContext(qhContext, h.vertices, h.numberOfPoints);
            }
            qhStep(qhContext);
            return qhConvertToMesh(renderContext, qhContext.qHull, h.vertices);
        }
        break;
        case Inc:
        {
            static bool init = true;
            auto &incContext = h.stepIncContext;
            if (!incContext.initialized)
            {
                incInitializeContext(incContext, h.vertices, h.numberOfPoints);
            }
            if (init)
            {
                incInitStepHull();
                init = false;
            }
            else
            {
                incHullStep();
            }
            return incConvertToMesh(incContext, renderContext);
        }
        break;
        /*case Dac:
        {
            auto &dacContext = h.stepDacContext;
            return dacConvertToMesh(dacContext, renderContext);
        }
        break;*/
        default:
        break;
    }
}

static mesh &TimedStepHull(render_context &renderContext, hull &h)
{
    switch (h.currentHullType)
    {
        case QH:
        {
            auto &qhContext = h.timedStepQhContext;
            if (!qhContext.initialized)
            {
                qhInitializeContext(qhContext, h.vertices, h.numberOfPoints);
            }
            
            h.qhTimer.running = !h.qhTimer.running;
            return qhConvertToMesh(renderContext, qhContext.qHull, h.vertices);
        }
        break;
        case Inc:
        {
            auto &incContext = h.timedStepIncContext;
            if (!incContext.initialized)
            {
                incInitializeContext(incContext, h.vertices, h.numberOfPoints);
            }
            
            h.incTimer.running = !h.incTimer.running;
            return incConvertToMesh(incContext, renderContext);
        }
        break;
        /*case Dac:
        {
            auto &dacContext = h.timedStepDacContext;
            return dacConvertToMesh(dacContext, renderContext);
        }
        break;*/
        default:
        break;
    }
}

static bool TimerRunning(hull &h)
{
    return h.qhTimer.running || h.incTimer.running;
}

#endif
