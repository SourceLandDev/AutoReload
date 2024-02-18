#include <ll/api/schedule/Scheduler.h>
#include <ll/api/service/Bedrock.h>
#include <mc/world/ActorUniqueID.h>
#include <mc/world/Container.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/item/registry/ItemStack.h>
#include <mc/world/level/Level.h>

ll::schedule::GameTickScheduler scheduler;

void reloadItem(Player& player, int id, int auxValue, bool isOffhand = false) {
    ActorUniqueID const& uniqueID = player.getOrCreateUniqueID();
    scheduler.add<ll::schedule::DelayTask>(ll::chrono::ticks(1), [uniqueID, id, auxValue, isOffhand]() {
        Player* player = ll::service::bedrock::getLevel()->getPlayer(uniqueID);
        if (!player || !(isOffhand ? player->getOffhandSlot() : player->getSelectedItem()).isNull()) {
            return;
        }
        Container& inventory        = player->getInventory();
        int        selectedItemSlot = player->getSelectedItemSlot();
        for (int i = 0; i < inventory.getContainerSize(); ++i) {
            if (!isOffhand && i == selectedItemSlot) {
                continue;
            }
            ItemStack const& itemStack = inventory.getItem(i);
            if (!itemStack.sameItem(id, auxValue)) {
                continue;
            }
            if (!isOffhand && i < 9) {
                player->setSelectedSlot(i);
                break;
            }
            ItemStack newItemStack = itemStack.clone();
            inventory.setItem(i, ItemStack::EMPTY_ITEM);
            if (isOffhand) {
                player->setOffhandSlot(newItemStack);
            } else {
                player->setSelectedItem(newItemStack);
            }
            player->sendInventory(true);
            break;
        }
    });
}

#include <ll/api/memory/Hook.h>
#include <mc/deps/core/string/HashedString.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/item/ItemStackBase.h>
#include <mc/world/item/VanillaItemNames.h>

LL_AUTO_TYPE_INSTANCE_HOOK(
    UseItemHook,
    ll::memory::HookPriority::Normal,
    Player,
    "?useItem@Player@@UEAAXAEAVItemStackBase@@W4ItemUseMethod@@_N@Z",
    void,
    ItemStackBase& a2,
    ItemUseMethod  a3,
    bool           a4
) {
    int id       = a2.getId();
    int auxValue = a2.getAuxValue();
    origin(a2, a3, a4);
    if (!a2.isNull()) {
        return;
    }
    reloadItem(*this, id, auxValue);
}
LL_AUTO_TYPE_INSTANCE_HOOK(
    HurtAndBreakHook,
    ll::memory::HookPriority::Normal,
    ItemStackBase,
    "?hurtAndBreak@ItemStackBase@@QEAA_NHPEAVActor@@@Z",
    bool,
    int    a2,
    Actor& a3
) {
    int  id       = getId();
    int  auxValue = getAuxValue();
    bool result   = origin(a2, a3);
    if (isNull() &&& a3 && a3.isPlayer()) {
        reloadItem((Player&)a3, id, auxValue);
    }
    return result;
}
LL_AUTO_TYPE_INSTANCE_HOOK(
    ConsumeTotemHook,
    ll::memory::HookPriority::Normal,
    Player,
    "?consumeTotem@Player@@UEAA_NXZ",
    bool
) {
    int              id          = -1;
    int              auxValue    = -1;
    bool             isOffhand   = false;
    ItemStack const& handSlot    = getSelectedItem();
    ItemStack const& offhandSlot = getOffhandSlot();
    if (handSlot.getFullNameHash() == VanillaItemNames::TotemOfUndying) {
        id       = handSlot.getId();
        auxValue = handSlot.getAuxValue();
    } else if (offhandSlot.getFullNameHash() == VanillaItemNames::TotemOfUndying) {
        id        = offhandSlot.getId();
        auxValue  = offhandSlot.getAuxValue();
        isOffhand = true;
    }
    bool result = origin();
    if (isOffhand ? offhandSlot.isNull() : handSlot.isNull()) {
        reloadItem(*this, id, auxValue, isOffhand);
    }
    return result;
}