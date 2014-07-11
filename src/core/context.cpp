#include "context.hpp"

using namespace CCPlus;

const Context Context::invalid("");

Context::Context(const std::string& _storagePath):
    storagePath(_storagePath)
{
    
}

const std::string& Context::getStoragePath() const {
    return storagePath;
}