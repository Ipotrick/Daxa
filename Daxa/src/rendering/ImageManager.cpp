#include "ImageManager.hpp"
namespace daxa {
    ImageManager::ImageManager(vk::Device device, u16 maxImages) : device{ device }, MAX_IMAGE_SLOTS{ maxImages }, imageSlots(maxImages) {
        for (u16 i = 0; i < MAX_IMAGE_SLOTS; i++) {
            freeImageSlots.emplace_back(MAX_IMAGE_SLOTS-1-i);
        }
    }

    ImageManager::Handle::Handle(ImageManager* manager, u16 index) :
        manager{manager},
        index{index}
    {
        ++manager->imageSlots[index].refCount;
    }
    ImageManager::Handle::Handle(const Handle& other) {
        this->manager = other.manager;
        this->index = other.index;
        ++this->manager->imageSlots[this->index].refCount;
    }
    ImageManager::Handle::Handle(Handle&& other) {
        this->manager = other.manager;
        this->index = other.index;
        other.manager = nullptr;
        other.index = ~static_cast<u16>(0);
    }
    ImageManager::Handle::~Handle() {
        if (manager) {
            --manager->imageSlots[index].refCount;
        }
    }
    ReadWriteLock<Image> ImageManager::Handle::get() {
        assert(manager);
        return manager->imageSlots[index].imageMut.lock();
    }
    ReadOnlyLock<Image> ImageManager::Handle::getConst() const 	{
        assert(manager);
        return manager->imageSlots[index].imageMut.lockReadOnly();
    }
    OwningMutex<Image>& ImageManager::Handle::getMtx() {
        DAXA_ASSERT(manager);
        return manager->imageSlots[index].imageMut;
    }

    bool ImageManager::DestroyJob::execute() {
        auto manager = managerMtx->lock();
        auto destroyQ = std::move(manager->destroyQ);
        manager->destroyQ = {};
        auto& slots = manager->imageSlots;
        manager.unlock();

        std::vector<u16> freeIndices;

        for (u16 slotIndex : destroyQ) {
            auto imgLock = slots[slotIndex].imageMut.lock();
            *imgLock = {};
            freeIndices.push_back(slotIndex);
        }

        manager.lock();
        manager->freeImageSlots.insert(manager->freeImageSlots.end(), freeIndices.begin(), freeIndices.end());
        return false;
    }

    bool ImageManager::CreateJob::execute() {
        auto manager = managerMtx->lock();
        auto createQ = std::move(manager->createQ);
        manager->createQ = {};
        auto& imageSlots = manager->imageSlots;

        auto cmd = manager->cmdPool.getElement();
        auto fence = manager->fencePool.get();
        manager.unlock();

        for (auto& [path, slot] : createQ) {
            auto imgLock = imageSlots[slot].imageMut.lock();
            *imgLock = std::move(loadImage2d(cmd, fence, path));
        }
        return false;
    }

    ImageManager::Handle ImageManager::getHandle(const std::string& alias)
    {
        if (!aliasToSlot.contains(alias)) {
            DAXA_ALLWAYS_ASSERT(!freeImageSlots.empty());
            u16 index = freeImageSlots.back();
            aliasToSlot[alias] = index;
            slotToAlias[index] = alias;
        }
        u16 index = aliasToSlot[alias];
        imageSlots[index].leftSurvivalFrames = SLOT_SURVIVAL_FRAMES;        // refresh surival timer
        return Handle(this, index);
    }

    ImageManager::Handle ImageManager::getHandle(u16 index) {
        DAXA_ALLWAYS_ASSERT(imageSlots[index]);
        return Handle(this, index);
    }

    void ImageManager::update()
    {
        cmdPool.flush();
        for (u16 i = 0; i < MAX_IMAGE_SLOTS; i++) {
            if (imageSlots[i].refCount == 0 && imageSlots[i].leftSurvivalFrames > 0) {
                --imageSlots[i].leftSurvivalFrames;
                if (imageSlots[i].leftSurvivalFrames == 0) {
                    destroyQ.push_back(i);
                    aliasToSlot.erase(slotToAlias[i]);
                    slotToAlias.erase(i);
                }
            }
        }
    }

    std::vector<vk::Image> ImageManager::getImages() const {
        std::vector<vk::Image> images(MAX_IMAGE_SLOTS);
        for (u16 i = 0; i < MAX_IMAGE_SLOTS; i++) {
            images[i] = imageSlots[i].imageMut.lockReadOnly()->image;
        }
        return images;
    }
}
