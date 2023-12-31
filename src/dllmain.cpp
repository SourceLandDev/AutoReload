#pragma comment(lib, "../SDK/lib/bedrock_server_api.lib")
#pragma comment(lib, "../SDK/lib/bedrock_server_var.lib")
#pragma comment(lib, "../SDK/lib/SymDBHelper.lib")
#pragma comment(lib, "../SDK/lib/LiteLoader.lib")

#include <llapi/mc/ActorUniqueID.hpp>
#include <llapi/mc/Level.hpp>
#include <llapi/mc/Player.hpp>
#include <llapi/mc/Container.hpp>
#include <llapi/mc/ItemStack.hpp>
#include <llapi/GlobalServiceAPI.h>
#include <llapi/ScheduleAPI.h>

void reloadItem(Player& player, int id, int auxValue, bool isOffhand = false) {
    ActorUniqueID const& uniqueID = player.getOrCreateUniqueID();
    Schedule::nextTick([uniqueID, id, auxValue, isOffhand]() {
        Player* player = Global<Level>->getPlayer(uniqueID);
        if (!player || !(isOffhand ? player->getOffhandSlot() : player->getSelectedItem()).isNull()) {
            return;
        }
        Container& inventory = player->getInventory();
        for (int i = 0; i < inventory.getContainerSize(); ++i) {
            if (!isOffhand && i == player->getSelectedItemSlot()) {
                continue;
            }
            ItemStack const& itemStack = inventory.getItem(i);
            if (!itemStack.sameItem(id, auxValue)) {
                continue;
            }
            if (!isOffhand && i < 9) {
                player->setSelectedSlot(i);
                return;
            }
            ItemStack newItemStack = itemStack.clone();
            inventory.setItem(i, ItemStack::EMPTY_ITEM);
            if (isOffhand) {
                player->setOffhandSlot(newItemStack);
            }
            else {
                player->setSelectedItem(newItemStack);
            }
            player->sendInventory(true);
            return;
        }
        });
}

#include <llapi/mc/ItemStackBase.hpp>
#include <llapi/mc/Actor.hpp>
#include <llapi/HookAPI.h>

TInstanceHook(void, "?useItem@Player@@UEAAXAEAVItemStackBase@@W4ItemUseMethod@@_N@Z", Player, ItemStackBase& a2, ItemUseMethod a3, bool a4) {
    int id = a2.getId();
    int auxValue = a2.getAuxValue();
    original(this, a2, a3, a4);
    if (!a2.isNull()) {
        return;
    }
    reloadItem(*this, id, auxValue);
}
TInstanceHook(bool, "?hurtAndBreak@ItemStackBase@@QEAA_NHPEAVActor@@@Z", ItemStackBase, int a2, Actor& a3) {
    int id = getId();
    int auxValue = getAuxValue();
    bool result = original(this, a2, a3);
    if (!isNull() || !a3.isPlayer()) {
        return result;
    }
    reloadItem((Player&)a3, id, auxValue);
    return result;
}
TInstanceHook(bool, "?consumeTotem@Player@@UEAA_NXZ", Player) {
    int id = -1;
    int auxValue = -1;
    const ItemStack& handSlot = getSelectedItem();
    const ItemStack& offhandSlot = getOffhandSlot();
    bool isOffhand = false;
    if (handSlot.getTypeName() == "minecraft:totem_of_undying") {
        id = handSlot.getId();
        auxValue = handSlot.getAuxValue();
    }
    else if (offhandSlot.getTypeName() == "minecraft:totem_of_undying") {
        id = offhandSlot.getId();
        auxValue = offhandSlot.getAuxValue();
        isOffhand = true;
    }
    bool result = original(this);
    if (!(isOffhand ? offhandSlot.isNull() : handSlot.isNull())) {
        return result;
    }
    reloadItem(*this, id, auxValue, isOffhand);
    return result;
}
