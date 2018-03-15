#define Time(msg, func) \
{\
    auto before = glfwGetTime();\
    func;\
    auto after = glfwGetTime();\
    auto total = after - before;\
    Log_A("%s took %f time\n",msg, total);\
}

#define TIME_START auto before = glfwGetTime();

#define TIME_END(msg) auto after = glfwGetTime(); \
auto total = after - before;\
    Log_A("%s took %f time\n",msg, total);