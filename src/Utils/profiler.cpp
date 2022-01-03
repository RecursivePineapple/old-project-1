
#ifdef DEBUG

#include "profiler.hpp"

int _profiler::calls_lock;
std::map<unsigned int, _profiler::_profinfo> _profiler::calls;

#endif
