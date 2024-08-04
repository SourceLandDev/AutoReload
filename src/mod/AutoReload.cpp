#include "mod/AutoReload.h"

#include <memory>

#include "ll/api/mod/RegisterHelper.h"

namespace auto_reload {

static std::unique_ptr<AutoReload> instance;

AutoReload& AutoReload::getInstance() { return *instance; }

bool AutoReload::load() {
    // Code for loading the mod goes here.
    return true;
}

bool AutoReload::enable() {
    // Code for enabling the mod goes here.
    return true;
}

bool AutoReload::disable() {
    // Code for disabling the mod goes here.
    return true;
}

} // namespace auto_reload

LL_REGISTER_MOD(auto_reload::AutoReload, auto_reload::instance);
