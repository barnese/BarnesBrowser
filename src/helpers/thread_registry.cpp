#include "thread_registry.h"

ThreadRegistry& threadRegistry()
{
    static ThreadRegistry registry;
    return registry;
}
