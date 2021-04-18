#include "ImageManager.hpp"
namespace daxa {

    ImageManager::WeakHandle::WeakHandle(ImageSlot* imgSlot) : imgSlot{ imgSlot } {}

    ReadWriteLock<ImageManager::ImageTuple> ImageManager::WeakHandle::get()     {
        DAXA_ASSERT(imgSlot);
        DAXA_ASSERT(imgSlot->framesSinceZeroRefs < ImageManager::SLOT_SURVIVAL_FRAMES);
        return imgSlot->imageMut.lock();
    }

    ReadOnlyLock<ImageManager::ImageTuple> ImageManager::WeakHandle::getConst() const     {
        DAXA_ASSERT(imgSlot);
        DAXA_ASSERT(imgSlot->framesSinceZeroRefs < ImageManager::SLOT_SURVIVAL_FRAMES);
        return imgSlot->imageMut.lockReadOnly();
    }

    OwningMutex<ImageManager::ImageTuple>& ImageManager::WeakHandle::getMtx()     {
        DAXA_ASSERT(imgSlot);
        DAXA_ASSERT(imgSlot->framesSinceZeroRefs < ImageManager::SLOT_SURVIVAL_FRAMES);
        return imgSlot->imageMut;
    }

    ImageManager::Handle ImageManager::WeakHandle::getStrongHandle() const     {
        return Handle{ imgSlot };
    }

    ImageManager::Handle::Handle(ImageSlot* slot) 	{
        imgSlot = slot;
        slot->refCount++;
    }
    ImageManager::Handle::Handle(const Handle& other) 	{
        this->imgSlot = other.imgSlot;
        ++this->imgSlot->refCount;
    }
    ImageManager::Handle::Handle(Handle&& other) 	{
        std::swap(this->imgSlot, other.imgSlot);
    }
    ImageManager::Handle::~Handle() 	{
        if (imgSlot) {
            --imgSlot->refCount;
        }
    }
    ReadWriteLock<ImageManager::ImageTuple> ImageManager::Handle::get() 	{
        assert(imgSlot);
        return imgSlot->imageMut.lock();
    }

    ReadOnlyLock<ImageManager::ImageTuple> ImageManager::Handle::getConst() const 	{
        assert(imgSlot);
        return imgSlot->imageMut.lockReadOnly();
    }

    OwningMutex<ImageManager::ImageTuple>& ImageManager::Handle::getMtx()     {
        DAXA_ASSERT(imgSlot);
        return imgSlot->imageMut;
    }

    ImageManager::WeakHandle ImageManager::Handle::getWeakHandle() const     {
        return WeakHandle{ imgSlot };
    }

    void ImageManager::DestroyJob::execute() {
        auto manager = managerMtx->lock();
        auto destroyQ = std::move(manager->destroyQ);
        decltype(destroyQ) leftOverQ;
        manager->destroyQ = {};
        manager.unlock();

        for (auto* slot : destroyQ) {
            if (slot->refCount == 0) {
                if (auto imgOpt = slot->imageMut.tryLock()) {
                    auto& img = *imgOpt.value();
                    img = {};
                }
                else {
                    leftOverQ.push_back(slot);
                }
            }
            else {
                leftOverQ.push_back(slot);
            }
        }

        manager.lock();
        manager->destroyQ.insert(manager->destroyQ.end(), leftOverQ.begin(), leftOverQ.end());
    }

    void ImageManager::CreateJob::execute()     {
        auto manager = managerMtx->lock();
        auto createQ = std::move(manager->createQ);
        manager->createQ = {};
        decltype(createQ) leftOverQ;

        auto cmd = manager->cmdPool.getElement();
        auto fence = manager->fencePool.get();
        manager.unlock();

        for (auto& [path, slot] : createQ) {
            if (auto lockOpt = slot->imageMut.tryLock()) {
                auto& img = *lockOpt.value();
                img.image = loadImage(cmd, fence, path);
                img.tableIndex = -2;    // TODO add entry in table index
            }
            else {
                leftOverQ.push_back({ path,slot });
            }
        }

        manager.lock();
        manager->createQ.insert(manager->createQ.end(), leftOverQ.begin(), leftOverQ.end());
    }

    ImageManager::Handle ImageManager::getHandle(const std::string& alias)     {
        ImageSlot* slot{ nullptr };
        if (aliasToSlot.contains(alias)) {
            slot = aliasToSlot[alias];
        }
        else {
            imageSlots.emplace_back(std::make_unique<ImageSlot>());
            slot = imageSlots.back().get();
            aliasToSlot.insert({ alias,slot });
            slotToAlias.insert({ slot, alias });
            createQ.push_back({ alias,slot });
        }
        return { slot };
    }

    void ImageManager::update()     {
        cmdPool.flush();
        for (auto& slot : imageSlots) {
            if (slot) {
                if (slot->refCount == 0) {
                    slot->framesSinceZeroRefs++;
                    if (slot->framesSinceZeroRefs >= SLOT_SURVIVAL_FRAMES) {
                        destroyQ.emplace_back(slot.get());
                    }
                }
                else {
                    slot->framesSinceZeroRefs = 0;
                }
            }
        }
    }
    void ImageManager::clearRegestry()     {
        auto delBegin = std::remove_if(imageSlots.begin(), imageSlots.end(), [](std::unique_ptr<ImageSlot>& slot) { return slot->framesSinceZeroRefs >= SLOT_SURVIVAL_FRAMES; });
        for (auto iter = delBegin; iter != imageSlots.end(); ++iter) {
            const auto& alias = slotToAlias[iter->get()];
            slotToAlias.erase(iter->get());
            aliasToSlot.erase(alias);
        }
        imageSlots.erase(delBegin, imageSlots.end());
    }
}
