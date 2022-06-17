#include "getuniquecountedarray.h"
#include <glm/gtc/type_precision.hpp>

std::mutex CountedDBBase::g_mutex;

template<>
std::vector<ConstSizedArray<glm::i16vec4>> CountedDB<glm::i16vec4>::g_database;
template<>
std::vector<ConstSizedArray<glm::u16vec4>> CountedDB<glm::u16vec4>::g_database;

