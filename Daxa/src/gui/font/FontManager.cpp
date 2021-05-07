#include "FontManager.hpp"

namespace daxa {
    FontManager::FontManager(u16 maxSlots) : MAX_SLOTS{ maxSlots }, slots(maxSlots) {
        for (u16 i = 0; i < MAX_SLOTS; i++) {
            freeSlots.emplace_back(MAX_SLOTS - 1 - i);
        }
    }

    FontManager::Handle::Handle(FontManager* manager, u16 index) :
        manager{ manager },
        index{ index }
    {
        ++manager->slots[index].refCount;
    }
    FontManager::Handle::Handle(const Handle& other) {
        this->manager = other.manager;
        this->index = other.index;
        ++this->manager->slots[this->index].refCount;
    }
    FontManager::Handle::Handle(Handle&& other) {
        this->manager = other.manager;
        this->index = other.index;
        other.manager = nullptr;
        other.index = ~static_cast<u16>(0);
    }
    FontManager::Handle::~Handle() {
        if (manager) {
            --manager->slots[index].refCount;
        }
    }
    ReadWriteLock<Font> FontManager::Handle::get() {
        assert(manager);
        return manager->slots[index].mtx.lock();
    }
    ReadOnlyLock<Font> FontManager::Handle::getConst() const {
        assert(manager);
        return manager->slots[index].mtx.lockReadOnly();
    }
    OwningMutex<Font>& FontManager::Handle::getMtx() {
        DAXA_ASSERT(manager);
        return manager->slots[index].mtx;
    }

    bool FontManager::CreateJob::execute() {
        auto manager = managerMtx->lock();
        auto createQ = std::move(manager->createQ);
        manager->createQ = {};
        auto& slots = manager->slots;
        manager.unlock();

        for (auto& [path, slot] : createQ) {
            auto lck = slots[slot].mtx.lock();
            lck->load(path);
        }
        return false;
    }

    FontManager::Handle FontManager::getHandle(const std::string& alias)     {
        if (!aliasToSlot.contains(alias)) {
            DAXA_ALLWAYS_ASSERT(!freeSlots.empty());
            u16 index = freeSlots.back();
            aliasToSlot[alias] = index;
            slotToAlias[index] = alias;
        }
        u16 index = aliasToSlot[alias];
        slots[index].leftSurvivalFrames = SLOT_SURVIVAL_FRAMES;        // refresh surival timer
        return Handle(this, index);
    }

    FontManager::Handle FontManager::getHandle(u16 index) {
        DAXA_ALLWAYS_ASSERT(slots[index]);
        return Handle(this, index);
    }

    void FontManager::update()     {
        for (u16 i = 0; i < MAX_SLOTS; i++) {
            if (slots[i].refCount == 0 && slots[i].leftSurvivalFrames > 0) {
                --slots[i].leftSurvivalFrames;
                if (slots[i].leftSurvivalFrames == 0) {
                    slots[i].mtx.lock()->unload();
                    freeSlots.push_back(i);
                    aliasToSlot.erase(slotToAlias[i]);
                    slotToAlias.erase(i);
                }
            }
        }
    }
}
