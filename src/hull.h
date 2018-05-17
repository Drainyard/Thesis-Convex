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
    
    dac_context dacContext;
    dac_context stepDacContext;
    dac_context timedStepDacContext;
    
    timer dacTimer;
    
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

void WriteHullToCSV(const char *filename, int facesAdded, int totalFaceCount, int vertexCount, int pointsProcessed, int distanceQueryCount, int sidednessQueries, int verticesInHull, unsigned long long nstimeSpent, GeneratorType generateType)
{
    char *fullFilename = concat(filename, ".csv");
    
    auto fileExists = FileExists(fullFilename);
    
    FILE *f = fopen(fullFilename, "a+");
    free(fullFilename);
    
    if (f)
    {
        if (!fileExists)
        {
            fprintf(f, "input vertices, faces added, faces in hull, points processed, distance queries, sidednessQueries, vertices in hull, time spent, point distribution\n");
        }
        
        fprintf(f, "%d, %d, %d, %d, %d, %d, %d, %lld, %s\n", vertexCount, facesAdded, totalFaceCount, pointsProcessed, distanceQueryCount, sidednessQueries, verticesInHull, nstimeSpent, GetGeneratorTypeString(generateType));
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

static void RunFullHullTest(TestSet &testSet, glm::vec3 offset)
{
    auto vertexAmounts = testSet.testSet;
    auto iterations = testSet.count;
    auto genType = testSet.genType;
    
    vertex *vertices = nullptr;
    
    auto seed = time(NULL);
    point_generator generator;
    std::random_device rd{};
    std::mt19937 gen{rd()};
    gen.seed(seed);
    generator.gen = gen;
    
    qh_context qhContext = {};
    
    log_a("Count: %zd\n", testSet.count);
    for(size_t i = 0; i < testSet.count; i++)
    {
        int addedFaces = 0;
        int numFaces = 0;
        int pointsProcessed = 0;
        int distanceQueries = 0;
        int sidednessQueries = 0;
        int verticesOnHull = 0;
        unsigned long long timeSpent = 0;
        
        int numForAvg = Max(1, testSet.iterations);
        auto n = vertexAmounts[i];
        
        log_a("Num: %d\n", n);
        initPointGenerator(generator, genType, n);
        
        for (int j = 0; j < numForAvg; j++)
        {
            log_a("%d \n", j);
            
            vertices = generate(generator, 0.0f, 5000.0f, offset);
            
            qhInitializeContext(qhContext, vertices, n);
            auto timerIndex = startTimer();
            qhFullHull(qhContext);
            qhContext.qHull.processingState.timeSpent = endTimer(timerIndex);
            
            qhContext.initialized = false;
            if(qhContext.qHull.failed)
            {
                free(vertices);
                j--;
                continue;
            }
            
            addedFaces += qhContext.qHull.processingState.addedFaces;
            numFaces += (int)qhContext.qHull.faces.size;
            pointsProcessed += qhContext.qHull.processingState.pointsProcessed;
            distanceQueries += qhContext.qHull.processingState.distanceQueryCount;
            sidednessQueries += qhContext.qHull.processingState.sidednessQueries;
            verticesOnHull += qhContext.qHull.processingState.verticesInHull;
            timeSpent += qhContext.qHull.processingState.timeSpent;
        }
        
        WriteHullToCSV("../data/qh_hull_out", addedFaces / numForAvg, numFaces / numForAvg, n, pointsProcessed / numForAvg, distanceQueries / numForAvg, sidednessQueries / numForAvg,  verticesOnHull / numForAvg, timeSpent / numForAvg, genType);
        
        addedFaces = 0;
        numFaces = 0;
        pointsProcessed = 0;
        distanceQueries = 0;
        sidednessQueries = 0;
        verticesOnHull = 0;
        timeSpent = 0.0;
    }
    log_a("Done\n");
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
                auto &qhContext = h.qhContext;
                if (!qhContext.initialized)
                {
                    qhInitializeContext(qhContext, h.vertices, h.numberOfPoints);
                }
                
                auto timerIndex = startTimer();
                qhFullHull(qhContext);
                TIME_END(timerIndex, "Full quick hull");
                
                WriteHullToCSV("qh_hull_out", qhContext.qHull.processingState.addedFaces, (int)qhContext.qHull.faces.size, h.numberOfPoints, qhContext.qHull.processingState.pointsProcessed, qhContext.qHull.processingState.distanceQueryCount,
                               qhContext.qHull.processingState.sidednessQueries, qhContext.qHull.processingState.verticesInHull, qhContext.qHull.processingState.timeSpent, h.pointGenerator.type);
                
                return qhConvertToMesh(renderContext, qhContext.qHull, h.vertices);
            }
            
            auto timerIndex = startTimer();
            qhFullHull(qhContext);
            TIME_END(timerIndex, "Full quick hull");
            
            WriteHullToCSV("qh_hull_out", qhContext.qHull.processingState.addedFaces, (int)qhContext.qHull.faces.size, h.numberOfPoints, qhContext.qHull.processingState.pointsProcessed, qhContext.qHull.processingState.distanceQueryCount,
                           qhContext.qHull.processingState.sidednessQueries,qhContext.qHull.processingState.verticesInHull, qhContext.qHull.processingState.timeSpent, h.pointGenerator.type);
            
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
        case Dac:
        {
            auto &dacContext = h.dacContext;
            if (!dacContext.initialized)
            {
                dacInitializeContext(dacContext, h.vertices, h.numberOfPoints);
            }
            auto timerIndex = startTimer();
            dacConstructFullHull(dacContext);
            TIME_END(timerIndex, "Full dac hull");
            return dacConvertToMesh(dacContext, renderContext);
        }
        break;
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
        case Dac:
        {
            /*auto &dacContext = h.dacContext;
            if (!dacContext.initialized)
            {
                dacInitializeContext(dacContext, h.vertices, h.numberOfPoints);
            }
            dacHullStep(dacContext);
            return dacConvertToMesh(dacContext, renderContext);
            */
        }
        break;
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
