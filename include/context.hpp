#include "global.hpp"

class CCPlus::Context : public CCPlus::Object {
public:
    static Context& getContext();
    static Context* singletonInstance;

    void begin(const std::string& tmlPath, const std::string& storagePath, int fps);
    void end();
    
    std::string getFootagePath(const std::string& relativePath);
    std::string getStoragePath(const std::string& relativePath);

    std::string tmlDir, storagePath;
    std::map<std::string, Renderables*> renderables;
    int fps;
};

