#pragma once
#include "global.hpp"

// @ Context objects contains data that should be shared between process stages
class CCPlus::Context {
public:
    Context(const std::string& storagePath);
    
    // operations
    
    // access
    const std::string& getStoragePath() const;
    
    // inquery
private:
    
    // data
    std::string storagePath;
    
    // operations
};