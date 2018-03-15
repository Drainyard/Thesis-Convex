#define Time(msg, func) \
{\
    auto before = glfwGetTime();\
    func;\
    auto after = glfwGetTime();\
    auto total = after - before;\
    Log_A("%s took %f time\n",msg, total);\
}