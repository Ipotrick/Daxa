#pragma once

#include <daxa/swapchain.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    /// I (pahrens) am going to document the internals here as wsi is really confussng and strange in vulkan.
    /// Every frame we get a swapcahin image index. This index can be non sequential in the case of mail box presentation and other modes.
    /// This means we need to aquire a new index every frame to know what swapchain image to use.
    ///
    /// IMPORTANT INFORMATION REGUARDING SEMAPHORES IN WSI:
    /// binary semaphore in aquire MUST be unsignaled when recording actions ON THE CPU!
    ///
    /// We need two binary semaphores here:
    /// The acquire semaphore and the present semaphore.
    /// The present semaphore is signaled in the last submission that uses the swapchain image and waited on in the present.
    /// The acquire semaphore is signaled when the swapchain image is ready to be used.
    /// This also means that the previous presentation of the image is finished and the semaphore used in the present is unsignaled.
    /// Unformtunately there is NO other way to know when a present finishes (or the corresponding semaphore is unsignaled). 
    /// This means that in order to be able to reuse binary semaphores used in presentation,
    /// one MUST pair them with the image they are used to present.
    /// 
    /// One can then rely on the aquire semaphore of the image beeing signaled to indicate that the present semaphore is able to be reused,
    /// As a swapchain images acquire sema is signaled when it is available and its previous present is completed.
    /// 
    /// In order to reuse the the acquire semaphore we must set a limit in frames in flight and wait on the cpu to limit the frames in flight.
    /// When we have this wait in place we can savely reuse the acquire semaphores with a linearly increasing index corresponding to the frame.
    /// This means the scquire semaphores are not tied to the number of swapchain images like present semaphores but to the number of frames in flight!!
    ///
    /// To limit the frames in flight we employ a timeline semaphore that must be signaled in a submission that uses or after one that uses the swpchain image. 
    struct ImplSwapchain final : ManagedSharedState
    {
        ManagedWeakPtr impl_device = {};
        SwapchainInfo info = {};
        VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
        VkSurfaceKHR vk_surface = {};
        VkSurfaceFormatKHR vk_surface_format = {};
        std::vector<ImageId> images = {};
        std::vector<BinarySemaphore> acquire_semaphores = {};
        std::vector<BinarySemaphore> present_semaphores = {};
        // Monothonicaly increasing frame index.
        usize cpu_frame_timeline = {};
        // cpu_frame_timeline % frames in flight. used to index the aquire semaphores.
        usize aquire_semaphore_index = {};
        // Gpu timeline semaphore used to track how far behind the gpu is.
        // Used to limit frames in flight.
        TimelineSemaphore gpu_frame_timeline;
        // This is the swapchain image index that acquire returns. THis is not nessecarily linear.
        // This index must be used for present semaphores as they are paired to the images.
        u32 current_image_index = {};

        auto get_index_of_image(ImageId image) const -> usize;

        ImplSwapchain(ManagedWeakPtr device_impl, SwapchainInfo info);
        ~ImplSwapchain();

        void recreate();
        void cleanup();
        void recreate_surface();
    };
} // namespace daxa
