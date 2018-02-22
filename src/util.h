#ifndef UTIL_H
#define UTIL_H

static float RandomFloat(float start, float end)
{
    return (rand() / (float)RAND_MAX * end) + start;
}

static int RandomInt(int start, int end)
{
    return (rand() % end) + start;
}

static glm::vec4 RandomColor()
{
    return glm::vec4((float)RandomInt(0, 255) / 255.0f, (float)RandomInt(0, 255) / 255.0f, (float)RandomInt(0, 255) / 255.0f, (float)RandomInt(100, 255) / 255.0f);
}

#endif