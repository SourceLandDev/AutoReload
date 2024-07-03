#include "plugin/AutoReload.h"

#include <memory>

#include "ll/api/plugin/NativePlugin.h"
#include "ll/api/plugin/RegisterHelper.h"

namespace auto_reload {

static std::unique_ptr<AutoReload> instance;

AutoReload& AutoReload::getInstance() { return *instance; }

bool AutoReload::load() {
    return true;
}

bool AutoReload::enable() {
    return true;
}

bool AutoReload::disable() {
    return true;
}

} // namespace auto_reload

LL_REGISTER_PLUGIN(auto_reload::AutoReload, auto_reload::instance);
