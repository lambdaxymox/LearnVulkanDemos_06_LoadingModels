#include <vulkan/vulkan.h>

#include "engine.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <chrono>

#include <fmt/core.h>
#include <fmt/ostream.h>
/*
#include <vulkan_engine/vulkan_platform.h>
#include <vulkan_engine/vulkan_platform_impl_fmt.h>
*/
#include <unordered_set>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif // GLFW_INCLUDE_VULKAN

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <stb/stb_image.h>
#include <tiny_obj_loader/tiny_obj_loader.h>


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = std::string { "assets/viking_room/viking_room.obj" };
const std::string TEXTURE_PATH = std::string { "assets/viking_room/viking_room.png" };
/*
const std::vector<std::string> VALIDATION_LAYERS = std::vector<std::string> { 
    VulkanEngine::Constants::VK_LAYER_KHRONOS_validation
};

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
const bool ENABLE_DEBUGGING_EXTENSIONS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
const bool ENABLE_DEBUGGING_EXTENSIONS = true;
#endif
*/
const int MAX_FRAMES_IN_FLIGHT = 2;


using Engine = VulkanEngine::Engine;
/*
using VulkanInstanceProperties = VulkanEngine::VulkanPlatform::VulkanInstanceProperties;
using PhysicalDeviceProperties = VulkanEngine::VulkanPlatform::PhysicalDeviceProperties;
using Platform = VulkanEngine::VulkanPlatform::PlatformInfoProvider::Platform;
using PlatformInfoProvider = VulkanEngine::VulkanPlatform::PlatformInfoProvider;
*/
/*
struct QueueFamilyIndices final {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails final {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanInstanceSpec final {
    public:
        explicit VulkanInstanceSpec() = default;
        explicit VulkanInstanceSpec(
            std::vector<std::string> instanceExtensions,
            std::vector<std::string> instanceLayers,
            VkInstanceCreateFlags instanceCreateFlags,
            std::string applicationName,
            std::string engineName
        ) 
            : m_instanceExtensions { instanceExtensions }
            , m_instanceLayers { instanceLayers }
            , m_instanceCreateFlags { instanceCreateFlags }
            , m_applicationName { applicationName }
            , m_engineName { engineName }
        {
        }
        ~VulkanInstanceSpec() = default;

        const std::vector<std::string>& instanceExtensions() const {
            return m_instanceExtensions;
        }

        const std::vector<std::string>& instanceLayers() const {
            return m_instanceLayers;
        }

        VkInstanceCreateFlags instanceCreateFlags() const {
            return m_instanceCreateFlags;
        }

        const std::string& applicationName() const {
            return m_applicationName;
        }

        const std::string& engineName() const {
            return m_engineName;
        }

        bool areValidationLayersEnabled() const {
            auto found = std::find(
                m_instanceLayers.begin(), 
                m_instanceLayers.end(), 
                VulkanEngine::Constants::VK_LAYER_KHRONOS_validation
            );
        
            return found != m_instanceLayers.end();
        }
    private:
        std::vector<std::string> m_instanceExtensions;
        std::vector<std::string> m_instanceLayers;
        VkInstanceCreateFlags m_instanceCreateFlags;
        std::string m_applicationName;
        std::string m_engineName;
};

class InstanceSpecProvider final {
    public:
        explicit InstanceSpecProvider() = default;
        explicit InstanceSpecProvider(bool enableValidationLayers, bool enableDebuggingExtensions)
            : m_enableValidationLayers { enableValidationLayers }
            , m_enableDebuggingExtensions { enableDebuggingExtensions }
        {
        }

        ~InstanceSpecProvider() {
            m_enableValidationLayers = false;
            m_enableDebuggingExtensions = false;
        }

        VulkanInstanceSpec createInstanceSpec() const {
            auto instanceExtensions = this->getInstanceExtensions();
            auto instanceLayers = this->getInstanceLayers();
            auto instanceCreateFlags = this->minInstanceCreateFlags();
            auto applicationName = std::string { "" };
            auto engineName = std::string { "" };

            return VulkanInstanceSpec {
                instanceExtensions,
                instanceLayers,
                instanceCreateFlags,
                applicationName,
                engineName
            };
        }
    private:
        bool m_enableValidationLayers;
        bool m_enableDebuggingExtensions;

        enum class Platform {
            Apple,
            Linux,
            Windows,
            Unknown
        };

        Platform getOperatingSystem() const {
            #if defined(__APPLE__) || defined(__MACH__)
            return Platform::Apple;
            #elif defined(__LINUX__)
            return Platform::Linux;
            #elif defined(_WIN32)
            return Platform::Windows;
            #else
            return Platform::Unknown;
            #endif
        }

        VkInstanceCreateFlags minInstanceCreateFlags() const {
            auto instanceCreateFlags = 0;
            if (this->getOperatingSystem() == Platform::Apple) {
                instanceCreateFlags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            }

            return instanceCreateFlags;
        }

        std::vector<std::string> getWindowSystemInstanceRequirements() const {        
            uint32_t requiredExtensionCount = 0;
            const char** requiredExtensionNames = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);
            auto requiredExtensions = std::vector<std::string> {};
            for (int i = 0; i < requiredExtensionCount; i++) {
                requiredExtensions.emplace_back(std::string(requiredExtensionNames[i]));
            }

            return requiredExtensions;
        }

        std::vector<std::string> getInstanceExtensions() const {
            auto instanceExtensions = this->getWindowSystemInstanceRequirements();
            instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            if (m_enableDebuggingExtensions) {
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return instanceExtensions;
        }

        std::vector<std::string> getInstanceLayers() const {
            auto instanceLayers = std::vector<std::string> {};
            if (m_enableValidationLayers) {
                instanceLayers.push_back(VulkanEngine::Constants::VK_LAYER_KHRONOS_validation);
            }

            return instanceLayers;
        }
};

class SystemFactory final {
    public:
        explicit SystemFactory() = default;

        VkInstance create(const VulkanInstanceSpec& instanceSpec) {
            if (instanceSpec.areValidationLayersEnabled() && !m_infoProvider->areValidationLayersSupported()) {
                throw std::runtime_error("validation layers requested, but not available!");
            }

            auto instanceInfo = m_infoProvider->getVulkanInstanceInfo();
            auto instanceExtensions = instanceSpec.instanceExtensions();
            auto instanceLayers = instanceSpec.instanceLayers();
            auto missingExtensions = m_infoProvider->detectMissingInstanceExtensions(
                instanceInfo,
                instanceExtensions
            );
            auto missingLayers = m_infoProvider->detectMissingInstanceLayers(
                instanceInfo,
                instanceLayers
            );
            if (missingExtensions.empty() && !missingLayers.empty()) {
                auto errorMessage = std::string { "Vulkan does not have the required extensions on this system:\n" };
                for (const auto& extensionName : missingExtensions) {
                    errorMessage.append(extensionName);
                    errorMessage.append("\n");
                }

                throw std::runtime_error { errorMessage };
            }

            if (!missingExtensions.empty() && missingLayers.empty()) {
                auto errorMessage = std::string { "Vulkan does not have the required layers on this system:\n" };
                for (const auto& layerName : missingLayers) {
                    errorMessage.append(layerName);
                    errorMessage.append("\n");
                }

                throw std::runtime_error { errorMessage };
            }

            if (!missingExtensions.empty() && !missingLayers.empty()) {
                auto errorMessage = std::string { "Vulkan does not have the required extensions on this system:\n" };
                for (const auto& extensionName : missingExtensions) {
                    errorMessage.append(extensionName);
                    errorMessage.append("\n");
                }

                errorMessage.append(std::string { "Vulkan does not have the required layers on this system:\n" });
                for (const auto& layerName : missingLayers) {
                    errorMessage.append(layerName);
                    errorMessage.append("\n");
                }

                throw std::runtime_error { errorMessage };
            }

            auto instanceCreateFlags = instanceSpec.instanceCreateFlags();
            auto enabledLayerNames = SystemFactory::convertToCStrings(instanceLayers);
            auto enabledExtensionNames = SystemFactory::convertToCStrings(instanceExtensions);

            const auto appInfo = VkApplicationInfo {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = instanceSpec.applicationName().data(),
                .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                .pEngineName = instanceSpec.engineName().data(),
                .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                .apiVersion = VK_API_VERSION_1_3,
            };
            const auto flags = (VkInstanceCreateFlags {}) | instanceCreateFlags;
            const auto createInfo = VkInstanceCreateInfo {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo = &appInfo,
                .flags = flags,
                .enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.size()),
                .ppEnabledLayerNames = enabledLayerNames.data(),
                .enabledExtensionCount = static_cast<uint32_t>(enabledExtensionNames.size()),
                .ppEnabledExtensionNames = enabledExtensionNames.data(),
            };

            auto instance = VkInstance {};
            const auto result = vkCreateInstance(&createInfo, nullptr, &instance);
            if (result != VK_SUCCESS) {
                throw std::runtime_error(fmt::format("Failed to create Vulkan instance."));
            }

            return instance;
        }
    private:
        PlatformInfoProvider* m_infoProvider;

        static std::vector<const char*> convertToCStrings(const std::vector<std::string>& strings) {
            auto cStrings = std::vector<const char*> {};
            cStrings.reserve(strings.size());
            std::transform(
                strings.begin(), 
                strings.end(), 
                std::back_inserter(cStrings),
                [](const std::string& string) { return string.c_str(); }
            );
    
            return cStrings;
        }
};

class PhysicalDeviceSpec final {
    public:
        explicit PhysicalDeviceSpec() = default;
        explicit PhysicalDeviceSpec(const std::vector<std::string>& requiredExtensions, bool hasGraphicsFamily, bool hasPresentFamily)
            : m_requiredExtensions { requiredExtensions }
            , m_hasGraphicsFamily { hasGraphicsFamily }
            , m_hasPresentFamily { hasPresentFamily }
        {
        }
        ~PhysicalDeviceSpec() = default;

        const std::vector<std::string>& requiredExtensions() const {
            return m_requiredExtensions;
        }

        bool hasGraphicsFamily() const {
            return m_hasGraphicsFamily;
        }

        bool hasPresentFamily() const {
            return m_hasPresentFamily;
        }
    private:
        std::vector<std::string> m_requiredExtensions;
        bool m_hasGraphicsFamily;
        bool m_hasPresentFamily;
    };

class PhysicalDeviceSpecProvider final {
    public:
        explicit PhysicalDeviceSpecProvider() = default;

        PhysicalDeviceSpec createPhysicalDeviceSpec() const {
            auto requiredExtensions = this->getPhysicalDeviceRequirements();

            return PhysicalDeviceSpec { requiredExtensions, true, true };
        }
    private:
        enum class Platform {
            Apple,
            Linux,
            Windows,
            Unknown
        };

        Platform getOperatingSystem() const {
            #if defined(__APPLE__) || defined(__MACH__)
            return Platform::Apple;
            #elif defined(__LINUX__)
            return Platform::Linux;
            #elif defined(_WIN32)
            return Platform::Windows;
            #else
            return Platform::Unknown;
            #endif
        }

        std::vector<std::string> getPhysicalDeviceRequirements() const {
            auto physicalDeviceExtensions = std::vector<std::string> {};
            if (this->getOperatingSystem() == Platform::Apple) {
                physicalDeviceExtensions.push_back(VulkanEngine::Constants::VK_KHR_portability_subset);
            }

            physicalDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

            return physicalDeviceExtensions;
        }
};

class PhysicalDeviceSelector final {
    public:
        explicit PhysicalDeviceSelector(VkInstance instance, PlatformInfoProvider* infoProvider)
            : m_instance { instance }
            , m_infoProvider { infoProvider }
        {
        }

        ~PhysicalDeviceSelector() {
            m_instance = VK_NULL_HANDLE;
            m_infoProvider = nullptr;
        }

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const {
            auto indices = QueueFamilyIndices {};

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

            auto queueFamilies = std::vector<VkQueueFamilyProperties> { queueFamilyCount };
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }

                if (indices.isComplete()) {
                    break;
                }

                i++;
            }

            return indices;
        }

        bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<std::string>& requiredExtensions) const {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

            auto availableExtensions = std::vector<VkExtensionProperties> { extensionCount };
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

            auto remainingExtensions = std::set<std::string> { requiredExtensions.begin(), requiredExtensions.end() };
            for (const auto& extension : availableExtensions) {
                for (const auto& requiredExtension : requiredExtensions) {
                    remainingExtensions.erase(extension.extensionName);
                }
            }

            return remainingExtensions.empty();
        }

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const {
            auto details = SwapChainSupportDetails {};
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

            uint32_t formatCount = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
            }

            return details;
        }

        bool isPhysicalDeviceCompatible(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const PhysicalDeviceSpec& physicalDeviceSpec) const {
            QueueFamilyIndices indices = this->findQueueFamilies(physicalDevice, surface);

            bool areRequiredExtensionsSupported = this->checkDeviceExtensionSupport(
                physicalDevice,
                physicalDeviceSpec.requiredExtensions()
            );

            bool swapChainCompatible = false;
            if (areRequiredExtensionsSupported) {
                SwapChainSupportDetails swapChainSupport = this->querySwapChainSupport(physicalDevice, surface);
                swapChainCompatible = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            auto supportedFeatures = VkPhysicalDeviceFeatures {};
            vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

            return indices.isComplete() && areRequiredExtensionsSupported && swapChainCompatible && supportedFeatures.samplerAnisotropy;
        }

        std::vector<VkPhysicalDevice> findAllPhysicalDevices() const {
            uint32_t physicalDeviceCount = 0;
            vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);

            auto physicalDevices = std::vector<VkPhysicalDevice> { physicalDeviceCount };
            vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());

            return physicalDevices;
        }

        std::vector<VkPhysicalDevice> findCompatiblePhysicalDevices(VkSurfaceKHR surface, const PhysicalDeviceSpec& physicalDeviceSpec) const {
            auto physicalDevices = this->findAllPhysicalDevices();
            if (physicalDevices.empty()) {
                throw std::runtime_error("failed to find GPUs with Vulkan support!");
            }

            auto compatiblePhysicalDevices = std::vector<VkPhysicalDevice> {};
            for (const auto& physicalDevice : physicalDevices) {
                if (this->isPhysicalDeviceCompatible(physicalDevice, surface, physicalDeviceSpec)) {
                    compatiblePhysicalDevices.emplace_back(physicalDevice);
                }
            }

            return compatiblePhysicalDevices;
        }

        VkPhysicalDevice selectPhysicalDeviceForSurface(VkSurfaceKHR surface, const PhysicalDeviceSpec& physicalDeviceSpec) const {
            auto physicalDevices = this->findCompatiblePhysicalDevices(surface, physicalDeviceSpec);
            if (physicalDevices.empty()) {
                throw std::runtime_error("failed to find a suitable GPU!");
            }

            VkPhysicalDevice selectedPhysicalDevice = physicalDevices[0];

            return selectedPhysicalDevice;
        }
    private:
        VkInstance m_instance;
        PlatformInfoProvider* m_infoProvider;
};

class LogicalDeviceSpec final {
    public:
        explicit LogicalDeviceSpec() = default;
        explicit LogicalDeviceSpec(const std::vector<std::string>& requiredExtensions, bool requireSamplerAnisotropy)
            : m_requiredExtensions { requiredExtensions }
            , m_requireSamplerAnisotropy { requireSamplerAnisotropy }
        {
        }
        ~LogicalDeviceSpec() = default;

        const std::vector<std::string>& requiredExtensions() const {
            return m_requiredExtensions;
        }

        bool requireSamplerAnisotropy() const {
            return m_requireSamplerAnisotropy;
        }
    private:
        std::vector<std::string> m_requiredExtensions;
        bool m_requireSamplerAnisotropy;
};

class LogicalDeviceSpecProvider final {
    public:
        explicit LogicalDeviceSpecProvider() = default;
        explicit LogicalDeviceSpecProvider(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) 
            : m_physicalDevice { physicalDevice }
            , m_surface { surface }
        {
        }

        ~LogicalDeviceSpecProvider() {
            m_physicalDevice = VK_NULL_HANDLE;
            m_surface = VK_NULL_HANDLE;
        }

        LogicalDeviceSpec createLogicalDeviceSpec() const {
            auto requiredExtensions = this->getLogicalDeviceRequirements();
            bool requireSamplerAnisotropy = true;

            return LogicalDeviceSpec { requiredExtensions, requireSamplerAnisotropy };
        }
    private:
        VkPhysicalDevice m_physicalDevice;
        VkSurfaceKHR m_surface;

        enum class Platform {
            Apple,
            Linux,
            Windows,
            Unknown
        };

        Platform getOperatingSystem() const {
            #if defined(__APPLE__) || defined(__MACH__)
            return Platform::Apple;
            #elif defined(__LINUX__)
            return Platform::Linux;
            #elif defined(_WIN32)
            return Platform::Windows;
            #else
            return Platform::Unknown;
            #endif
        }

        std::vector<std::string> getLogicalDeviceRequirements() const {
            auto logicalDeviceExtensions = std::vector<std::string> {};
            if (this->getOperatingSystem() == Platform::Apple) {
                logicalDeviceExtensions.push_back(VulkanEngine::Constants::VK_KHR_portability_subset);
            }

            logicalDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

            return logicalDeviceExtensions;
        }
};

class LogicalDeviceFactory final {
    public:
        explicit LogicalDeviceFactory(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, PlatformInfoProvider* infoProvider)
            : m_physicalDevice { physicalDevice }
            , m_surface { surface }
            , m_infoProvider { infoProvider }
        {
        }

        ~LogicalDeviceFactory() {
            m_physicalDevice = VK_NULL_HANDLE;
            m_surface = VK_NULL_HANDLE;
            m_infoProvider = nullptr;
        }

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const {
            auto indices = QueueFamilyIndices {};

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

            auto queueFamilies = std::vector<VkQueueFamilyProperties> { queueFamilyCount };
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }

                if (indices.isComplete()) {
                    break;
                }

                i++;
            }

            return indices;
        }

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const {
            auto details = SwapChainSupportDetails {};
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

            uint32_t formatCount = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
            }

            return details;
        }

        std::tuple<VkDevice, VkQueue, VkQueue> createLogicalDevice(const LogicalDeviceSpec& logicalDeviceSpec) {
            const auto indices = this->findQueueFamilies(m_physicalDevice, m_surface);
            const auto uniqueQueueFamilies = std::set<uint32_t> {
                indices.graphicsFamily.value(), 
                indices.presentFamily.value()
            };
            const float queuePriority = 1.0f;
            auto queueCreateInfos = std::vector<VkDeviceQueueCreateInfo> {};
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                const auto queueCreateInfo = VkDeviceQueueCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = queueFamily,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority,
                };

                queueCreateInfos.push_back(queueCreateInfo);
            }

            const auto requireSamplerAnisotropy = [&logicalDeviceSpec]() { 
                if (logicalDeviceSpec.requireSamplerAnisotropy()) {    
                    return VK_TRUE;
                } else {
                    return VK_FALSE;
                }
            }();
            const auto deviceExtensionProperties = m_infoProvider->getAvailableVulkanDeviceExtensions(m_physicalDevice);
            const auto missingExtensions = m_infoProvider->detectMissingRequiredDeviceExtensions(
                deviceExtensionProperties, 
                logicalDeviceSpec.requiredExtensions()
            );
            if (!missingExtensions.empty()) {
                auto errorMessage = std::string { "Vulkan does not have the required extensions on this system: " };
                for (const auto& extension : missingExtensions) {
                    errorMessage.append(extension);
                    errorMessage.append("\n");
                }

                throw std::runtime_error(errorMessage);
            }

            const auto enabledExtensions = LogicalDeviceFactory::convertToCStrings(logicalDeviceSpec.requiredExtensions());
            const auto validationLayersCStrings = []() {
                if (ENABLE_VALIDATION_LAYERS) {
                    return LogicalDeviceFactory::convertToCStrings(VALIDATION_LAYERS);
                } else {
                    return std::vector<const char*> {};
                }
            }();

            const auto deviceFeatures = VkPhysicalDeviceFeatures {
                .samplerAnisotropy = requireSamplerAnisotropy,
            };

            const auto createInfo = VkDeviceCreateInfo {
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
                .pQueueCreateInfos = queueCreateInfos.data(),
                .pEnabledFeatures = &deviceFeatures,
                .enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
                .ppEnabledExtensionNames = enabledExtensions.data(),
                .enabledLayerCount = static_cast<uint32_t>(validationLayersCStrings.size()),
                .ppEnabledLayerNames = validationLayersCStrings.data(),
            };

            auto device = VkDevice {};
            const auto result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &device);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create logical device!");
            }

            auto graphicsQueue = VkQueue {};
            vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        
            auto presentQueue = VkQueue {};
            vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

            return std::make_tuple(device, graphicsQueue, presentQueue);
        }
    private:
        VkPhysicalDevice m_physicalDevice;
        VkSurfaceKHR m_surface;
        PlatformInfoProvider* m_infoProvider;

        static std::vector<const char*> convertToCStrings(const std::vector<std::string>& strings) {
            auto cStrings = std::vector<const char*> {};
            cStrings.reserve(strings.size());
            std::transform(
                strings.begin(), 
                strings.end(), 
                std::back_inserter(cStrings),
                [](const std::string& string) { return string.c_str(); }
            );
    
            return cStrings;
        }
};

class VulkanDebugMessenger final {
    public:
        explicit VulkanDebugMessenger()
            : m_instance { VK_NULL_HANDLE }
            , m_debugMessenger { VK_NULL_HANDLE }
        {
        }

        ~VulkanDebugMessenger() {
            this->cleanup();
        }

        static VulkanDebugMessenger* create(VkInstance instance) {
            if (instance == VK_NULL_HANDLE) {
                throw std::invalid_argument { "Got an empty `VkInstance` handle" };
            }

            uint32_t physicalDeviceCount = 0;
            const auto resultEnumeratePhysicalDevices = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
            if (resultEnumeratePhysicalDevices != VK_SUCCESS) {
                throw std::invalid_argument { "Got an invalid `VkInstance` handle" };
            }

            const auto createInfo = VkDebugUtilsMessengerCreateInfoEXT {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = 
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = 
                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = debugCallback,
            };

            auto debugMessenger = static_cast<VkDebugUtilsMessengerEXT>(nullptr);
            const auto result = VulkanDebugMessenger::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger);
            if (result != VK_SUCCESS) {
                throw std::runtime_error { "failed to set up debug messenger!" };
            }

            auto vulkanDebugMessenger = new VulkanDebugMessenger {};
            vulkanDebugMessenger->m_instance = instance;
            vulkanDebugMessenger->m_debugMessenger = debugMessenger;

            return vulkanDebugMessenger;
        }

        static VkResult CreateDebugUtilsMessengerEXT(
            VkInstance instance, 
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
            const VkAllocationCallbacks* pAllocator, 
            VkDebugUtilsMessengerEXT* pDebugMessenger
        ) {
            auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
            );

            if (func != nullptr) {
                return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
            } else {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }

        static void DestroyDebugUtilsMessengerEXT(
            VkInstance instance, 
            VkDebugUtilsMessengerEXT debugMessenger, 
            const VkAllocationCallbacks* pAllocator
        ) {
            auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
            );

            if (func != nullptr) {
                func(instance, debugMessenger, pAllocator);
            }
        }

        static const std::string& messageSeverityToString(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity) {
            static const std::string MESSAGE_SEVERITY_INFO  = std::string { "INFO " };
            static const std::string MESSAGE_SEVERITY_WARN  = std::string { "WARN " };
            static const std::string MESSAGE_SEVERITY_ERROR = std::string { "ERROR" };

            if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
                return MESSAGE_SEVERITY_ERROR;
            } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                return MESSAGE_SEVERITY_WARN;
            } else {
                return MESSAGE_SEVERITY_INFO;
            }
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        ) {
            auto messageSeverityString = VulkanDebugMessenger::messageSeverityToString(messageSeverity);
            fmt::println(std::cerr, "[{}] {}", messageSeverityString, pCallbackData->pMessage);

            return VK_FALSE;
        }

        void cleanup() {
            if (m_instance == VK_NULL_HANDLE) {
                return;
            }

            if (m_debugMessenger != VK_NULL_HANDLE) {
                VulkanDebugMessenger::DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
            }

            m_debugMessenger = VK_NULL_HANDLE;
            m_instance = VK_NULL_HANDLE;
        }
    private:
        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;
};

class SurfaceProvider final {
    public:
        explicit SurfaceProvider() = delete;
        explicit SurfaceProvider(VkInstance instance, GLFWwindow* window)
            : m_instance { instance }
            , m_window { window }
        {
        }

        ~SurfaceProvider() {
            m_instance = VK_NULL_HANDLE;
            m_window = nullptr;
        }

        VkSurfaceKHR createSurface() {
            auto surface = VkSurfaceKHR {};
            const auto result = glfwCreateWindowSurface(m_instance, m_window, nullptr, &surface);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }

            return surface;
        }
    private:
        VkInstance m_instance;
        GLFWwindow* m_window;
};

class WindowSystem final {
    public:
        explicit WindowSystem() = default;
        explicit WindowSystem(VkInstance instance) : m_instance { instance } {}
        ~WindowSystem() {
            m_framebufferResized = false;
            m_windowExtent = VkExtent2D { 0, 0 };
            m_surface = VK_NULL_HANDLE;

            glfwDestroyWindow(m_window);

            m_instance = VK_NULL_HANDLE;
        }

        static WindowSystem* create(VkInstance instance) {
            return new WindowSystem { instance };
        }

        void createWindow(uint32_t width, uint32_t height, const std::string& title) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

            auto window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
            glfwSetWindowUserPointer(window, this);
            glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

            m_window = window;
            m_windowExtent = VkExtent2D { width, height };
        }

        GLFWwindow* getWindow() const {
            return m_window;
        }

        SurfaceProvider createSurfaceProvider() {
            return SurfaceProvider { m_instance, m_window};
        }

        bool hasFramebufferResized() const {
            return m_framebufferResized;
        }

        void setFramebufferResized(bool framebufferResized) {
            m_framebufferResized = framebufferResized;
        }

        void setWindowTitle(const std::string& title) {
            glfwSetWindowTitle(m_window, title.data());
        }
    private:
        VkInstance m_instance;
        GLFWwindow* m_window;
        VkSurfaceKHR m_surface;
        VkExtent2D m_windowExtent;
        bool m_framebufferResized;

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
            auto windowSystem = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
            windowSystem->m_framebufferResized = true;
            windowSystem->m_windowExtent = VkExtent2D { 
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
        }
};

class GpuDevice final {
    public:
        explicit GpuDevice() = delete;
        explicit GpuDevice(
            VkInstance instance,
            VkPhysicalDevice physicalDevice, 
            VkDevice device, 
            VkQueue graphicsQueue,
            VkQueue presentQueue,
            VkCommandPool commandPool
        )   : m_instance { instance }
            , m_physicalDevice { physicalDevice }
            , m_device { device }
            , m_graphicsQueue { graphicsQueue }
            , m_presentQueue { presentQueue }
            , m_commandPool { commandPool }
            , m_shaderModules { std::unordered_set<VkShaderModule> {} }
        {
            m_msaaSamples = GpuDevice::getMaxUsableSampleCount(physicalDevice);
        }

        ~GpuDevice() {
            for (const auto& shaderModule : m_shaderModules) {
                vkDestroyShaderModule(m_device, shaderModule, nullptr);
            }

            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
            vkDestroyCommandPool(m_device, m_commandPool, nullptr);
            vkDestroyDevice(m_device, nullptr);

            m_surface = VK_NULL_HANDLE;
            m_commandPool = VK_NULL_HANDLE;
            m_presentQueue = VK_NULL_HANDLE;
            m_graphicsQueue = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
            m_physicalDevice = VK_NULL_HANDLE;
            m_instance = VK_NULL_HANDLE;
        }

        VkPhysicalDevice getPhysicalDevice() const {
            return m_physicalDevice;
        }

        VkDevice getLogicalDevice() const {
            return m_device;
        }

        VkQueue getGraphicsQueue() const {
            return m_graphicsQueue;
        }

        VkQueue getPresentQueue() const {
            return m_presentQueue;
        }

        VkCommandPool getCommandPool() const {
            return m_commandPool;
        }

        VkSampleCountFlagBits getMsaaSamples() const {
            return m_msaaSamples;
        }

        static VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice) {
            VkPhysicalDeviceProperties physicalDeviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

            VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
            if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
            if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
            if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
            if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
            if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
            if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

            return VK_SAMPLE_COUNT_1_BIT;
        }

        VkSurfaceKHR createRenderSurface(SurfaceProvider& surfaceProvider) {
            auto surface = surfaceProvider.createSurface();

            m_surface = surface;

            return surface;
        }

        VkShaderModule createShaderModuleFromFile(const std::string& fileName) {
            auto shaderCode = this->loadShaderFromFile(fileName);

            return this->createShaderModule(shaderCode);
        }

        VkShaderModule createShaderModule(std::istream& stream) {
            auto shaderCode = this->loadShader(stream);

            return this->createShaderModule(shaderCode);
        }

        VkShaderModule createShaderModule(const std::vector<char>& code) {
            auto createInfo = VkShaderModuleCreateInfo {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t*>(code.data()),
                .pNext = nullptr,
                .flags = 0,
            };

            auto shaderModule = VkShaderModule {};
            const auto result = vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create shader module!");
            }

            m_shaderModules.insert(shaderModule);

            return shaderModule;
        }
    private:
        VkInstance m_instance;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
        VkCommandPool m_commandPool;
        VkSurfaceKHR m_surface;
        VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

        std::unordered_set<VkShaderModule> m_shaderModules;

        std::vector<char> loadShader(std::istream& stream) {
            size_t shaderSize = static_cast<size_t>(stream.tellg());
            auto buffer = std::vector<char>(shaderSize);

            stream.seekg(0);
            stream.read(buffer.data(), shaderSize);

            return buffer;
        }

        std::ifstream openShaderFile(const std::string& fileName) {
            auto file = std::ifstream { fileName, std::ios::ate | std::ios::binary };

            if (!file.is_open()) {
                throw std::runtime_error("failed to open file!");
            }

            return file;
        }

        std::vector<char> loadShaderFromFile(const std::string& fileName) {
            auto stream = this->openShaderFile(fileName);
            auto shader = this->loadShader(stream);
            stream.close();

            return shader;
        }
};

class GpuDeviceInitializer final {
    public:
        explicit GpuDeviceInitializer(VkInstance instance)
            : m_instance { instance }
        {
            m_infoProvider = new PlatformInfoProvider {};
        }

        ~GpuDeviceInitializer() {
            vkDestroySurfaceKHR(m_instance, m_dummySurface, nullptr);

            m_instance = VK_NULL_HANDLE;
        }

        GpuDevice* createGpuDevice() {
            this->createDummySurface();
            this->selectPhysicalDevice();
            this->createLogicalDevice();
            this->createCommandPool();

            auto gpuDevice = new GpuDevice { m_instance, m_physicalDevice, m_device, m_graphicsQueue, m_presentQueue, m_commandPool };

            return gpuDevice;
        }
    private:
        VkInstance m_instance;
        PlatformInfoProvider* m_infoProvider;
        VkSurfaceKHR m_dummySurface;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
        VkCommandPool m_commandPool;

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const {
            auto indices = QueueFamilyIndices {};

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

            auto queueFamilies = std::vector<VkQueueFamilyProperties> { queueFamilyCount };
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }

                if (indices.isComplete()) {
                    break;
                }

                i++;
            }

            return indices;
        }

        void createDummySurface() {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

            auto dummyWindow = glfwCreateWindow(1, 1, "DUMMY WINDOW", nullptr, nullptr);
            auto dummySurface = VkSurfaceKHR {};
            const auto result = glfwCreateWindowSurface(m_instance, dummyWindow, nullptr, &dummySurface);
            if (result != VK_SUCCESS) {
                glfwDestroyWindow(dummyWindow);
                dummyWindow = nullptr;

                throw std::runtime_error("failed to create window surface!");
            }

            glfwDestroyWindow(dummyWindow);
            dummyWindow = nullptr;

            m_dummySurface = dummySurface;
        }

        void selectPhysicalDevice() {
            auto physicalDeviceSpecProvider = PhysicalDeviceSpecProvider {};
            auto physicalDeviceSpec = physicalDeviceSpecProvider.createPhysicalDeviceSpec();
            auto physicalDeviceSelector = PhysicalDeviceSelector { m_instance, m_infoProvider };
            auto selectedPhysicalDevice = physicalDeviceSelector.selectPhysicalDeviceForSurface(
                m_dummySurface,
                physicalDeviceSpec
            );

            m_physicalDevice = selectedPhysicalDevice;
        }

        void createLogicalDevice() {
            auto logicalDeviceSpecProvider = LogicalDeviceSpecProvider { m_physicalDevice, m_dummySurface };
            auto logicalDeviceSpec = logicalDeviceSpecProvider.createLogicalDeviceSpec();
            auto factory = LogicalDeviceFactory { m_physicalDevice, m_dummySurface, m_infoProvider };
            auto [device, graphicsQueue, presentQueue] = factory.createLogicalDevice(logicalDeviceSpec);

            m_device = device;
            m_graphicsQueue = graphicsQueue;
            m_presentQueue = presentQueue;
        }

        void createCommandPool() {
            auto queueFamilyIndices = this->findQueueFamilies(m_physicalDevice, m_dummySurface);
            const auto poolInfo = VkCommandPoolCreateInfo {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
            };

            auto commandPool = VkCommandPool {};
            const auto result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &commandPool);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create command pool!");
            }

            m_commandPool = commandPool;
        }
};

class Engine final {
    public:
        explicit Engine() = default;
        ~Engine() {
            delete m_windowSystem;
            delete m_gpuDevice;
            delete m_debugMessenger;

            vkDestroyInstance(m_instance, nullptr);

            delete m_systemFactory;
            delete m_infoProvider;

            glfwTerminate();
        }

        static std::unique_ptr<Engine> createDebugMode() {
            return Engine::create(true);
        }

        static std::unique_ptr<Engine> createReleaseMode() {
            return Engine::create(false);
        }

        VkInstance getInstance() const {
            return m_instance;
        }

        VkPhysicalDevice getPhysicalDevice() const {
            return m_gpuDevice->getPhysicalDevice();
        }

        VkDevice getLogicalDevice() const {
            return m_gpuDevice->getLogicalDevice();
        }

        VkQueue getGraphicsQueue() const {
            return m_gpuDevice->getGraphicsQueue();
        }

        VkQueue getPresentQueue() const {
            return m_gpuDevice->getPresentQueue();
        }

        VkCommandPool getCommandPool() const {
            return m_gpuDevice->getCommandPool();
        }

        VkSurfaceKHR getSurface() const {
            return m_surface;
        }

        VkSampleCountFlagBits getMsaaSamples() const {
            return m_gpuDevice->getMsaaSamples();
        }

        GLFWwindow* getWindow() const {
            return m_windowSystem->getWindow();
        }

        bool hasFramebufferResized() const {
            return m_windowSystem->hasFramebufferResized();
        }

        void setFramebufferResized(bool framebufferResized) {
            m_windowSystem->setFramebufferResized(framebufferResized);
        }

        bool isInitialized() const {
            return m_instance != VK_NULL_HANDLE;
        }

        void createGLFWLibrary() {
            const auto result = glfwInit();
            if (!result) {
                glfwTerminate();

                auto errorMessage = std::string { "Failed to initialize GLFW" };

                throw std::runtime_error { errorMessage };
            }
        }

        void createInfoProvider() {
            auto infoProvider = new PlatformInfoProvider {};

            m_infoProvider = infoProvider;
        }

        void createSystemFactory() {
            auto systemFactory = new SystemFactory {};

            m_systemFactory = systemFactory;
        }

        void createInstance() {
            auto instanceSpecProvider = InstanceSpecProvider { m_enableValidationLayers, m_enableDebuggingExtensions };
            auto instanceSpec = instanceSpecProvider.createInstanceSpec();
            auto instance = m_systemFactory->create(instanceSpec);
        
            m_instance = instance;
        }

        void createWindowSystem() {
            auto windowSystem = WindowSystem::create(m_instance);

            m_windowSystem = windowSystem;
        }

        void createDebugMessenger() {
            if (!m_enableValidationLayers) {
                return;
            }

            auto debugMessenger = VulkanDebugMessenger::create(m_instance);

            m_debugMessenger = debugMessenger;
        }

        void createWindow(uint32_t width, uint32_t height, const std::string& title) {
            m_windowSystem->createWindow(width, height, title);
       
            this->createRenderSurface();
        }

        void createGpuDevice() {
            auto gpuDeviceInitializer = GpuDeviceInitializer { m_instance };
            auto gpuDevice = gpuDeviceInitializer.createGpuDevice();

            m_gpuDevice = gpuDevice;
        }

        void createRenderSurface() {
            auto surfaceProvider = m_windowSystem->createSurfaceProvider();
            auto surface = m_gpuDevice->createRenderSurface(surfaceProvider);

            m_surface = surface;
        }

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const {
            auto indices = QueueFamilyIndices {};

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

            auto queueFamilies = std::vector<VkQueueFamilyProperties> { queueFamilyCount };
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }

                if (indices.isComplete()) {
                    break;
                }

                i++;
            }

            return indices;
        }

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const {
            auto details = SwapChainSupportDetails {};
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

            uint32_t formatCount = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
            }

            return details;
        }

        VkShaderModule createShaderModuleFromFile(const std::string& fileName) {
            return m_gpuDevice->createShaderModuleFromFile(fileName);
        }

        VkShaderModule createShaderModule(std::istream& stream) {
            return m_gpuDevice->createShaderModule(stream);
        }

        VkShaderModule createShaderModule(std::vector<char>& code) {
            return m_gpuDevice->createShaderModule(code);
        }
    private:
        PlatformInfoProvider* m_infoProvider;
        SystemFactory* m_systemFactory;
        VkInstance m_instance;
        VulkanDebugMessenger* m_debugMessenger;
        WindowSystem* m_windowSystem;
        VkSurfaceKHR m_surface;

        GpuDevice* m_gpuDevice;

        bool m_enableValidationLayers; 
        bool m_enableDebuggingExtensions;

        static std::unique_ptr<Engine> create(bool enableDebugging) {
            auto newEngine = std::make_unique<Engine>();

            if (enableDebugging) {
                newEngine->m_enableValidationLayers = true;
                newEngine->m_enableDebuggingExtensions = true;
            } else {
                newEngine->m_enableValidationLayers = false;
                newEngine->m_enableDebuggingExtensions = false;
            }

            newEngine->createGLFWLibrary();
            newEngine->createInfoProvider();
            newEngine->createSystemFactory();
            newEngine->createInstance();
            newEngine->createDebugMessenger();
            newEngine->createGpuDevice();
            newEngine->createWindowSystem();

            return newEngine;
        }
};
*/
class StbTextureImage final {
    public:
        explicit StbTextureImage() = default;
        ~StbTextureImage() {
            if (m_pixels != nullptr) {
                stbi_image_free(m_pixels);

                m_width = 0;
                m_height = 0;
                m_channels = 0;
            }
        }

        const stbi_uc& pixels() const {
            return *m_pixels;
        }

        inline uint32_t width() const noexcept {
            return m_width;
        }

        inline uint32_t height() const noexcept {
            return m_height;
        }

        inline uint32_t channels() const noexcept {
            return m_channels;
        }
    private:
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_channels;
        stbi_uc* m_pixels;

        friend class StbTextureLoader;
};

class StbTextureLoader final {
    public:
        explicit StbTextureLoader(const std::string& filePath) : m_filePath { filePath } {}

        StbTextureImage load() {
            int textureWidth = 0;
            int textureHeight = 0;
            int textureChannels = 0;
            stbi_uc* pixels = stbi_load(m_filePath.c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
        
            const auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(textureWidth, textureHeight)))) + 1;

            if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }

            auto textureImage = StbTextureImage {};
            textureImage.m_pixels = pixels;
            textureImage.m_width = textureWidth;
            textureImage.m_height = textureHeight;
            textureImage.m_channels = textureChannels;

            return textureImage;
        }
    private:
        const std::string& m_filePath;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        const auto bindingDescription = VkVertexInputBindingDescription {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        const auto attributeDescriptions = std::array<VkVertexInputAttributeDescription, 3> {
            VkVertexInputAttributeDescription {
                .binding = 0,
                .location = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, position),
            },
            VkVertexInputAttributeDescription {
                .binding = 0,
                .location = 1,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color),
            },
            VkVertexInputAttributeDescription {
                .binding = 0,
                .location = 2,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, texCoord),
            },
        };

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return position == other.position && color == other.color && texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

class Mesh final {
    public:
        explicit Mesh() = default;
        explicit Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
            : m_vertices { vertices }
            , m_indices { indices }
        {
        }

        explicit Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices)
            : m_vertices { vertices }
            , m_indices { indices }
        {
        }

        const std::vector<Vertex>& vertices() const {
            return m_vertices;
        }

        const std::vector<uint32_t>& indices() const {
            return m_indices;
        }
    private:
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
};

class MeshLoader final {
    public:
        explicit MeshLoader() = default;

        Mesh load(const std::string& filePath) const {
            auto attrib = tinyobj::attrib_t {};
            auto shapes = std::vector<tinyobj::shape_t> {};
            auto materials = std::vector<tinyobj::material_t> {};
            auto warn = std::string {}; 
            auto err = std::string {};

            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
                throw std::runtime_error(warn + err);
            }

            auto uniqueVertices = std::unordered_map<Vertex, uint32_t> {};
            auto vertices = std::vector<Vertex> {};
            auto indices = std::vector<uint32_t> {};
            for (const auto& shape : shapes) {
                for (const auto& index : shape.mesh.indices) {
                    const auto vertex = Vertex {
                        .position = glm::vec3 {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2]
                        },
                        .texCoord = glm::vec2 {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                        },
                        .color = glm::vec3 { 1.0f, 1.0f, 1.0f },
                    };

                    if (uniqueVertices.count(vertex) == 0) {
                        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                        vertices.push_back(vertex);
                    }

                    indices.push_back(uniqueVertices[vertex]);
                }
            }

            return Mesh { std::move(vertices), std::move(indices) };
        }
};

/// @brief The uniform buffer object for distpaching camera data to the GPU.
///
/// @note Vulkan expects data to be aligned in a specific way. For example,
/// let `T` be a data type.
///
/// @li If `T` is a scalar, `align(T) == sizeof(T)`
/// @li If `T` is a scalar, `align(vec2<T>) == 2 * sizeof(T)`
/// @li If `T` is a scalar, `align(vec3<T>) == 4 * sizeof(T)`
/// @li If `T` is a scalar, `align(vec4<T>) == 4 * sizeof(T)`
/// @li If `T` is a scalar, `align(mat4<T>) == 4 * sizeof(T)`
/// @li If `T` is a structure type, `align(T) == max(align(members(T)))`
///
/// In particular, each data type is a nice multiple of the alignment of the largest
/// scalar type constituting that data type. See the specification
/// https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap15.html#interfaces-resources-layout
/// for more details.
struct UniformBufferObject {
    glm::mat4x4 model;
    glm::mat4x4 view;
    glm::mat4x4 proj;
};

class App {
    public:
        void run() {
            this->initEngine();
            this->mainLoop();
            this->cleanup();
        }
    private:
        std::unique_ptr<Engine> m_engine;

        VkImage m_depthImage;
        VkDeviceMemory m_depthImageMemory;
        VkImageView m_depthImageView;

        VkImage m_textureImage;
        VkDeviceMemory m_textureImageMemory;
        VkImageView m_textureImageView;
        VkSampler m_textureSampler;

        Mesh m_mesh;
        VkBuffer m_vertexBuffer;
        VkDeviceMemory m_vertexBufferMemory;
        VkBuffer m_indexBuffer;
        VkDeviceMemory m_indexBufferMemory;

        std::vector<VkBuffer> m_uniformBuffers;
        std::vector<VkDeviceMemory> m_uniformBuffersMemory;
        std::vector<void*> m_uniformBuffersMapped;

        VkDescriptorPool m_descriptorPool;
        std::vector<VkDescriptorSet> m_descriptorSets;
        VkDescriptorSetLayout m_descriptorSetLayout;

        std::vector<VkCommandBuffer> m_commandBuffers;

        VkRenderPass m_renderPass;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_graphicsPipeline;

        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;

        VkSwapchainKHR m_swapChain;
        std::vector<VkImage> m_swapChainImages;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;
        std::vector<VkImageView> m_swapChainImageViews;
        std::vector<VkFramebuffer> m_swapChainFramebuffers;
    
        uint32_t m_currentFrame = 0;

        bool m_enableValidationLayers { false };
        bool m_enableDebuggingExtensions { false };

        void cleanup() {
            if (m_engine->isInitialized()) {
                this->cleanupSwapChain();

                vkDestroyPipeline(m_engine->getLogicalDevice(), m_graphicsPipeline, nullptr);
                vkDestroyPipelineLayout(m_engine->getLogicalDevice(), m_pipelineLayout, nullptr);
                vkDestroyRenderPass(m_engine->getLogicalDevice(), m_renderPass, nullptr);

                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    vkDestroySemaphore(m_engine->getLogicalDevice(), m_renderFinishedSemaphores[i], nullptr);
                    vkDestroySemaphore(m_engine->getLogicalDevice(), m_imageAvailableSemaphores[i], nullptr);
                    vkDestroyFence(m_engine->getLogicalDevice(), m_inFlightFences[i], nullptr);
                }

                vkDestroyDescriptorPool(m_engine->getLogicalDevice(), m_descriptorPool, nullptr);

                vkDestroySampler(m_engine->getLogicalDevice(), m_textureSampler, nullptr);
                vkDestroyImageView(m_engine->getLogicalDevice(), m_textureImageView, nullptr);

                vkDestroyImage(m_engine->getLogicalDevice(), m_textureImage, nullptr);
                vkFreeMemory(m_engine->getLogicalDevice(), m_textureImageMemory, nullptr);

                vkDestroyDescriptorSetLayout(m_engine->getLogicalDevice(), m_descriptorSetLayout, nullptr);

                vkDestroyBuffer(m_engine->getLogicalDevice(), m_indexBuffer, nullptr);
                vkFreeMemory(m_engine->getLogicalDevice(), m_indexBufferMemory, nullptr);

                vkDestroyBuffer(m_engine->getLogicalDevice(), m_vertexBuffer, nullptr);
                vkFreeMemory(m_engine->getLogicalDevice(), m_vertexBufferMemory, nullptr);

                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    vkDestroyBuffer(m_engine->getLogicalDevice(), m_uniformBuffers[i], nullptr);
                    vkFreeMemory(m_engine->getLogicalDevice(), m_uniformBuffersMemory[i], nullptr);
                }
            }
        }

        void createEngine() {
            auto engine = Engine::createDebugMode();
            engine->createWindow(WIDTH, HEIGHT, "Loading Models");

            m_engine = std::move(engine);
        }

        void initEngine() {
            this->createEngine();

            this->createTextureImage(TEXTURE_PATH);
            this->createTextureImageView();
            this->createTextureSampler();
            this->loadModel(MODEL_PATH);
            this->createVertexBuffer();
            this->createIndexBuffer();
            this->createDescriptorSetLayout();
            this->createUniformBuffers();
            this->createDescriptorPool();
            this->createDescriptorSets();
            this->createCommandBuffers();
            this->createSwapChain();
            this->createImageViews();
            this->createRenderPass();
            this->createGraphicsPipeline();
            this->createDepthResources();
            this->createFramebuffers();
            this->createRenderingSyncObjects();
        }

        void mainLoop() {
            while (!glfwWindowShouldClose(m_engine->getWindow())) {
                glfwPollEvents();
                this->draw();
            }

            vkDeviceWaitIdle(m_engine->getLogicalDevice());
        }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(m_engine->getPhysicalDevice(), &memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }

            throw std::runtime_error("failed to find suitable memory type!");
        }

        std::tuple<VkBuffer, VkDeviceMemory> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
            const auto bufferInfo = VkBufferCreateInfo {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = size,
                .usage = usage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            };

            auto buffer = VkBuffer {};
            const auto resultCreateBuffer = vkCreateBuffer(m_engine->getLogicalDevice(), &bufferInfo, nullptr, &buffer);
            if (resultCreateBuffer != VK_SUCCESS) {
                throw std::runtime_error("failed to create buffer!");
            }

            auto memRequirements = VkMemoryRequirements {};
            vkGetBufferMemoryRequirements(m_engine->getLogicalDevice(), buffer, &memRequirements);

            const auto allocInfo = VkMemoryAllocateInfo {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = memRequirements.size,
                .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
            };

            auto bufferMemory = VkDeviceMemory {};
            const auto resultAllocateMemory = vkAllocateMemory(m_engine->getLogicalDevice(), &allocInfo, nullptr, &bufferMemory);
            if (resultAllocateMemory != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate buffer memory!");
            }

            vkBindBufferMemory(m_engine->getLogicalDevice(), buffer, bufferMemory, 0);

            return std::make_tuple(buffer, bufferMemory);
        }

        std::tuple<VkImage, VkDeviceMemory> createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
            const auto imageInfo = VkImageCreateInfo {
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType = VK_IMAGE_TYPE_2D,
                .extent.width = width,
                .extent.height = height,
                .extent.depth = 1,
                .mipLevels = 1,
                .arrayLayers = 1,
                .format = format,
                .tiling = tiling,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .usage = usage,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            };

            auto image = VkImage {};
            const auto resultCreateImage = vkCreateImage(m_engine->getLogicalDevice(), &imageInfo, nullptr, &image);
            if (resultCreateImage != VK_SUCCESS) {
                throw std::runtime_error("failed to create image!");
            }

            auto memRequirements = VkMemoryRequirements {};
            vkGetImageMemoryRequirements(m_engine->getLogicalDevice(), image, &memRequirements);

            const auto allocInfo = VkMemoryAllocateInfo {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = memRequirements.size,
                .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
            };

            auto imageMemory = VkDeviceMemory {};
            const auto resultAllocateMemory = vkAllocateMemory(m_engine->getLogicalDevice(), &allocInfo, nullptr, &imageMemory);
            if (resultAllocateMemory != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate image memory!");
            }

            vkBindImageMemory(m_engine->getLogicalDevice(), image, imageMemory, 0);

            return std::make_tuple(image, imageMemory);
        }

        VkCommandBuffer beginSingleTimeCommands() {
            const auto allocInfo = VkCommandBufferAllocateInfo {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandPool = m_engine->getCommandPool(),
                .commandBufferCount = 1,
            };

            auto commandBuffer = VkCommandBuffer {};
            vkAllocateCommandBuffers(m_engine->getLogicalDevice(), &allocInfo, &commandBuffer);

            const auto beginInfo = VkCommandBufferBeginInfo {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            };

            vkBeginCommandBuffer(commandBuffer, &beginInfo);

            return commandBuffer;
        }

        void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
            vkEndCommandBuffer(commandBuffer);

            const auto submitInfo = VkSubmitInfo {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer,
            };

            vkQueueSubmit(m_engine->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(m_engine->getGraphicsQueue());

            vkFreeCommandBuffers(m_engine->getLogicalDevice(), m_engine->getCommandPool(), 1, &commandBuffer);
        }

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
            const VkCommandBuffer commandBuffer = this->beginSingleTimeCommands();

            const auto copyRegion = VkBufferCopy {
                .size = size
            };
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            this->endSingleTimeCommands(commandBuffer);
        }

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
            const VkCommandBuffer commandBuffer = this->beginSingleTimeCommands();

            const auto [srcAccessMask, dstAccessMask] = [oldLayout, newLayout]() -> std::tuple<VkAccessFlags, VkAccessFlags> {
                if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                    return std::make_tuple(
                        0, 
                        VK_ACCESS_TRANSFER_WRITE_BIT
                    );
                } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                    return std::make_tuple(
                        VK_ACCESS_TRANSFER_WRITE_BIT,
                        VK_ACCESS_SHADER_READ_BIT
                    );
                } else {
                    throw std::invalid_argument("unsupported layout transition!");
                }
            }();
            const auto [sourceStage, destinationStage] = [oldLayout, newLayout]() -> std::tuple<VkPipelineStageFlags, VkPipelineStageFlags> {
                if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                    return std::make_tuple(
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        VK_PIPELINE_STAGE_TRANSFER_BIT
                    );
                } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                    return std::make_tuple(
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                    );
                } else {
                    throw std::invalid_argument("unsupported layout transition!");
                }
            }();

            const auto barrier = VkImageMemoryBarrier {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .oldLayout = oldLayout,
                .newLayout = newLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = image,
                .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .subresourceRange.baseMipLevel = 0,
                .subresourceRange.levelCount = 1,
                .subresourceRange.baseArrayLayer = 0,
                .subresourceRange.layerCount = 1,
                .srcAccessMask = srcAccessMask,
                .dstAccessMask = dstAccessMask,
            };

            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            this->endSingleTimeCommands(commandBuffer);
        }

        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
            const VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            const auto region = VkBufferImageCopy {
                .bufferOffset = 0,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
                .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .imageSubresource.mipLevel = 0,
                .imageSubresource.baseArrayLayer = 0,
                .imageSubresource.layerCount = 1,
                .imageOffset = VkOffset3D { 0, 0, 0 },
                .imageExtent = VkExtent3D { width, height, 1 }
            };

            vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            endSingleTimeCommands(commandBuffer);
        }

        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
            const auto viewInfo = VkImageViewCreateInfo {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = format,
                .components = VkComponentMapping { VK_COMPONENT_SWIZZLE_IDENTITY }, // Optional
                .subresourceRange.aspectMask = aspectFlags,
                .subresourceRange.baseMipLevel = 0,
                .subresourceRange.levelCount = 1,
                .subresourceRange.baseArrayLayer = 0,
                .subresourceRange.layerCount = 1,
            };

            auto imageView = VkImageView {};
            const auto result = vkCreateImageView(m_engine->getLogicalDevice(), &viewInfo, nullptr, &imageView);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image view!");
            }

            return imageView;
        }

        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
            for (VkFormat format : candidates) {
                auto props = VkFormatProperties {};
                vkGetPhysicalDeviceFormatProperties(m_engine->getPhysicalDevice(), format, &props);

                if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                    return format;
                } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                    return format;
                }
            }

            throw std::runtime_error("failed to find supported format!");
        }

        VkFormat findDepthFormat() {
            auto candidates = std::vector<VkFormat> { 
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT
            };
            auto tiling = VK_IMAGE_TILING_OPTIMAL;
            auto features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        
            return this->findSupportedFormat(candidates, tiling, features);
        }

        bool hasStencilComponent(VkFormat format) {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

        void createDepthResources() {
            const VkFormat depthFormat = this->findDepthFormat();

            const auto [depthImage, depthImageMemory] = this->createImage(
                m_swapChainExtent.width,
                m_swapChainExtent.height,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );
            const auto depthImageView = this->createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

            m_depthImage = depthImage;
            m_depthImageMemory = depthImageMemory;
            m_depthImageView = depthImageView;
        }

        void createTextureImage(const std::string& filePath) {
            auto textureLoader = StbTextureLoader { filePath };
            const auto stbTextureImage = textureLoader.load();
            const auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(stbTextureImage.width(), stbTextureImage.height())))) + 1;
            const auto depth = stbTextureImage.channels() + 1;
            const auto imageSize =  VkDeviceSize { stbTextureImage.width() * stbTextureImage.height() * depth };

            auto [stagingBuffer, stagingBufferMemory] = this->createBuffer(
                imageSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );

            void* data;
            vkMapMemory(m_engine->getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, &stbTextureImage.pixels(), static_cast<size_t>(imageSize));
            vkUnmapMemory(m_engine->getLogicalDevice(), stagingBufferMemory);

            const auto [textureImage, textureImageMemory] = this->createImage(
                stbTextureImage.width(),
                stbTextureImage.height(),
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );

            this->transitionImageLayout(
                textureImage,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            );
            this->copyBufferToImage(
                stagingBuffer,
                textureImage,
                static_cast<uint32_t>(stbTextureImage.width()),
                static_cast<uint32_t>(stbTextureImage.height())
            );
            this->transitionImageLayout(
                textureImage,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );

            vkDestroyBuffer(m_engine->getLogicalDevice(), stagingBuffer, nullptr);
            vkFreeMemory(m_engine->getLogicalDevice(), stagingBufferMemory, nullptr);

            m_textureImage = textureImage;
            m_textureImageMemory = textureImageMemory;
        }

        void createTextureImageView() {
            const auto textureImageView = createImageView(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

            m_textureImageView = textureImageView;
        }

        void createTextureSampler() {
            auto properties = VkPhysicalDeviceProperties {};
            vkGetPhysicalDeviceProperties(m_engine->getPhysicalDevice(), &properties);

            const auto samplerInfo = VkSamplerCreateInfo {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .anisotropyEnable = VK_TRUE,
                .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
                .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                .unnormalizedCoordinates = VK_FALSE,
                .compareEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_ALWAYS,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                // // Use these parameters to disable anisotropic filtering.
                // .anisotropyEnable = VK_FALSE,
                // .maxAnisotropy = 1.0f,
            };

            auto textureSampler = VkSampler {};
            const auto result = vkCreateSampler(m_engine->getLogicalDevice(), &samplerInfo, nullptr, &textureSampler);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture sampler!");
            }

            m_textureSampler = textureSampler;
        }

        void loadModel(const std::string& filePath) {
            const auto meshLoader = MeshLoader {};
            const auto mesh = meshLoader.load(filePath);

            m_mesh = std::move(mesh);
        }

        void createVertexBuffer() {
            const auto bufferSize = VkDeviceSize { sizeof(m_mesh.vertices()[0]) * m_mesh.vertices().size() };
            VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            VkMemoryPropertyFlags stagingBufferPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            auto [stagingBuffer, stagingBufferMemory] = this->createBuffer(
                bufferSize,
                stagingBufferUsageFlags, 
                stagingBufferPropertyFlags
            );

            void* data;
            vkMapMemory(m_engine->getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, m_mesh.vertices().data(), static_cast<size_t>(bufferSize));
            vkUnmapMemory(m_engine->getLogicalDevice(), stagingBufferMemory);

            VkBufferUsageFlags vertexBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            VkMemoryPropertyFlags vertexBufferPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

            const auto [vertexBuffer, vertexBufferMemory] = this->createBuffer(
                bufferSize,
                vertexBufferUsageFlags,
                vertexBufferPropertyFlags
            );

            this->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

            vkDestroyBuffer(m_engine->getLogicalDevice(), stagingBuffer, nullptr);
            vkFreeMemory(m_engine->getLogicalDevice(), stagingBufferMemory, nullptr);

            m_vertexBuffer = vertexBuffer;
            m_vertexBufferMemory = vertexBufferMemory;
        }

        void createIndexBuffer() {
            const auto bufferSize = VkDeviceSize { sizeof(m_mesh.indices()[0]) * m_mesh.indices().size() };
            VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            VkMemoryPropertyFlags stagingBufferPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            auto [stagingBuffer, stagingBufferMemory] = this->createBuffer(
                bufferSize,
                stagingBufferUsageFlags, 
                stagingBufferPropertyFlags
            );
        
            void* data;
            vkMapMemory(m_engine->getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        
            memcpy(data, m_mesh.indices().data(), static_cast<size_t>(bufferSize));
        
            vkUnmapMemory(m_engine->getLogicalDevice(), stagingBufferMemory);

            VkBufferUsageFlags vertexBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            VkMemoryPropertyFlags vertexBufferPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            const auto [indexBuffer, indexBufferMemory] = this->createBuffer(bufferSize, vertexBufferUsageFlags, vertexBufferPropertyFlags);

            this->copyBuffer(stagingBuffer, indexBuffer, bufferSize);

            vkDestroyBuffer(m_engine->getLogicalDevice(), stagingBuffer, nullptr);
            vkFreeMemory(m_engine->getLogicalDevice(), stagingBufferMemory, nullptr);

            m_indexBuffer = indexBuffer;
            m_indexBufferMemory = indexBufferMemory;
        }

        void createDescriptorSetLayout() {
            const auto uboLayoutBinding = VkDescriptorSetLayoutBinding {
                .binding = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImmutableSamplers = nullptr,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            };
            const auto samplerLayoutBinding = VkDescriptorSetLayoutBinding {
                .binding = 1,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImmutableSamplers = nullptr,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            };
            const auto bindings = std::array<VkDescriptorSetLayoutBinding, 2> { uboLayoutBinding, samplerLayoutBinding };
            const auto layoutInfo = VkDescriptorSetLayoutCreateInfo {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(bindings.size()),
                .pBindings = bindings.data(),
            };

            auto descriptorSetLayout = VkDescriptorSetLayout {};
            const auto result = vkCreateDescriptorSetLayout(m_engine->getLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }

            m_descriptorSetLayout = descriptorSetLayout;
        }

        void createUniformBuffers() {
            const auto bufferSize = VkDeviceSize { sizeof(UniformBufferObject) };
            auto uniformBuffers = std::vector<VkBuffer> { MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE };
            auto uniformBuffersMemory = std::vector<VkDeviceMemory> { MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE };
            auto uniformBuffersMapped = std::vector<void*> { MAX_FRAMES_IN_FLIGHT, nullptr };

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            
                const auto [uniformBuffer, uniformBufferMemory] = this->createBuffer(bufferSize, usageFlags, propertyFlags);
                void* uniformBufferMapped;      
                vkMapMemory(m_engine->getLogicalDevice(), uniformBufferMemory, 0, bufferSize, 0, &uniformBufferMapped);

                uniformBuffers[i] = uniformBuffer;
                uniformBuffersMemory[i] = uniformBufferMemory;
                uniformBuffersMapped[i] = uniformBufferMapped;
            }

            m_uniformBuffers = std::move(uniformBuffers);
            m_uniformBuffersMemory = std::move(uniformBuffersMemory);
            m_uniformBuffersMapped = std::move(uniformBuffersMapped);
        }

        void createDescriptorPool() {
            const auto poolSizes = std::array<VkDescriptorPoolSize, 2> {
                VkDescriptorPoolSize {
                    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
                },
                VkDescriptorPoolSize {
                    .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
                },
            };
            const auto poolInfo = VkDescriptorPoolCreateInfo {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                .pPoolSizes = poolSizes.data(),
                .maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
            };

            auto descriptorPool = VkDescriptorPool {};
            const auto result = vkCreateDescriptorPool(m_engine->getLogicalDevice(), &poolInfo, nullptr, &descriptorPool);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }

            m_descriptorPool = descriptorPool;
        }

        void createDescriptorSets() {
            const auto layouts = std::vector<VkDescriptorSetLayout> { MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout };
            const auto allocInfo = VkDescriptorSetAllocateInfo {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = m_descriptorPool,
                .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
                .pSetLayouts = layouts.data(),
            };

            auto descriptorSets = std::vector<VkDescriptorSet> { MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE };
            const auto result = vkAllocateDescriptorSets(m_engine->getLogicalDevice(), &allocInfo, descriptorSets.data());
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            for (size_t i = 0; i < descriptorSets.size(); i++) {
                const auto bufferInfo = VkDescriptorBufferInfo {
                    .buffer = m_uniformBuffers[i],
                    .offset = 0,
                    .range = sizeof(UniformBufferObject),
                };

                const auto imageInfo = VkDescriptorImageInfo {
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    .imageView = m_textureImageView,    
                    .sampler = m_textureSampler,
                };

                const auto descriptorWrites = std::array<VkWriteDescriptorSet, 2> {
                    VkWriteDescriptorSet {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = descriptorSets[i],
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .descriptorCount = 1,
                        .pBufferInfo = &bufferInfo,
                    },
                    VkWriteDescriptorSet {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = descriptorSets[i],
                        .dstBinding = 1,
                        .dstArrayElement = 0,
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .descriptorCount = 1,
                        .pImageInfo = &imageInfo,
                    },
                };

                vkUpdateDescriptorSets(
                    m_engine->getLogicalDevice(),
                    static_cast<uint32_t>(descriptorWrites.size()),
                    descriptorWrites.data(),
                    0,
                    nullptr
                );
            }

            m_descriptorSets = std::move(descriptorSets);
        }

        void createCommandBuffers() {
            auto commandBuffers = std::vector<VkCommandBuffer> { MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE };
        
            const auto allocInfo = VkCommandBufferAllocateInfo {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = m_engine->getCommandPool(),
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
            };

            const auto result = vkAllocateCommandBuffers(m_engine->getLogicalDevice(), &allocInfo, commandBuffers.data());
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate command buffers!");
            }

            m_commandBuffers = std::move(commandBuffers);
        }

        VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
            for (const auto& availableFormat : availableFormats) {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return availableFormat;
                }
            }

            return availableFormats[0];
        }

        VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
            for (const auto& availablePresentMode : availablePresentModes) {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return availablePresentMode;
                }
            }

            // We would probably want to use `VK_PRESENT_MODE_FIFO_KHR` on mobile devices.
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return capabilities.currentExtent;
            } else {
                int _width, _height;
                glfwGetWindowSize(m_engine->getWindow(), &_width, &_height);

                const uint32_t width = std::clamp(
                    static_cast<uint32_t>(_width),
                    capabilities.minImageExtent.width,
                    capabilities.maxImageExtent.width
                );
                const uint32_t height = std::clamp(
                    static_cast<uint32_t>(_height), 
                    capabilities.minImageExtent.height, 
                    capabilities.maxImageExtent.height
                );
                const auto actualExtent = VkExtent2D {
                    .width = width,
                    .height = height,
                };

                return actualExtent;
            }
        }

        void createSwapChain() {
            const auto swapChainSupport = m_engine->querySwapChainSupport(m_engine->getPhysicalDevice(), m_engine->getSurface());
            const auto surfaceFormat = this->selectSwapSurfaceFormat(swapChainSupport.formats);
            const auto presentMode = this->selectSwapPresentMode(swapChainSupport.presentModes);
            const auto extent = this->selectSwapExtent(swapChainSupport.capabilities);
            const auto imageCount = [&swapChainSupport]() {
                uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
                if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                    return swapChainSupport.capabilities.maxImageCount;
                }

                return imageCount;
            }();
            const auto indices = m_engine->findQueueFamilies(m_engine->getPhysicalDevice(), m_engine->getSurface());
            auto queueFamilyIndices = std::array<uint32_t, 2> { 
                indices.graphicsAndComputeFamily.value(),
                indices.presentFamily.value()
            };
            const auto imageSharingMode = [&indices]() -> VkSharingMode {
                if (indices.graphicsAndComputeFamily != indices.presentFamily) {
                    return VK_SHARING_MODE_CONCURRENT;
                } else {
                    return VK_SHARING_MODE_EXCLUSIVE;
                }
            }();
            const auto [queueFamilyIndicesPtr, queueFamilyIndexCount] = [&indices, &queueFamilyIndices]() -> std::tuple<const uint32_t*, uint32_t> {
                if (indices.graphicsAndComputeFamily != indices.presentFamily) {
                    const auto data = queueFamilyIndices.data();
                    const auto size = static_cast<uint32_t>(queueFamilyIndices.size());
                
                    return std::make_tuple(data, size);
                } else {
                    const auto data = static_cast<uint32_t*>(nullptr);
                    const auto size = static_cast<uint32_t>(0);

                    return std::make_tuple(data, size);
                }
            }();
            
            const auto createInfo = VkSwapchainCreateInfoKHR {
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .surface = m_engine->getSurface(),
                .minImageCount = imageCount,
                .imageFormat = surfaceFormat.format,
                .imageColorSpace = surfaceFormat.colorSpace,
                .imageExtent = extent,
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .imageSharingMode = imageSharingMode,
                .queueFamilyIndexCount = queueFamilyIndexCount,
                .pQueueFamilyIndices = queueFamilyIndices.data(),
                .preTransform = swapChainSupport.capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = presentMode,
                .clipped = VK_TRUE,
                .oldSwapchain = VK_NULL_HANDLE,
            };

            auto swapChain = VkSwapchainKHR {};
            const auto result = vkCreateSwapchainKHR(m_engine->getLogicalDevice(), &createInfo, nullptr, &swapChain);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create swap chain!");
            }

            uint32_t swapChainImageCount = 0;
            vkGetSwapchainImagesKHR(m_engine->getLogicalDevice(), swapChain, &swapChainImageCount, nullptr);
        
            auto swapChainImages = std::vector<VkImage> { swapChainImageCount, VK_NULL_HANDLE };
            vkGetSwapchainImagesKHR(m_engine->getLogicalDevice(), swapChain, &swapChainImageCount, swapChainImages.data());

            m_swapChain = swapChain;
            m_swapChainImages = std::move(swapChainImages);
            m_swapChainImageFormat = surfaceFormat.format;
            m_swapChainExtent = extent;
        }

        void createImageViews() {
            auto swapChainImageViews = std::vector<VkImageView> { m_swapChainImages.size(), VK_NULL_HANDLE };
            for (size_t i = 0; i < m_swapChainImages.size(); i++) {
                auto swapChainImageView = this->createImageView(m_swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
                swapChainImageViews[i] = swapChainImageView;
            }

            m_swapChainImageViews = std::move(swapChainImageViews);
        }

        void createRenderPass() {
            const auto colorAttachment = VkAttachmentDescription {
                .format = m_swapChainImageFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            };
            const auto depthAttachment = VkAttachmentDescription {
                .format = this->findDepthFormat(),
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            };
            const auto colorAttachmentRef = VkAttachmentReference {
                .attachment = 0,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            };
            const auto depthAttachmentRef = VkAttachmentReference {
                .attachment = 1,
                .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            };
            const auto subpass = VkSubpassDescription {
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentRef,
                .pDepthStencilAttachment = &depthAttachmentRef,
            };

            const auto dependency = VkSubpassDependency {
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                .srcAccessMask = 0,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            };

            const auto attachments = std::array<VkAttachmentDescription, 2> {
                colorAttachment,
                depthAttachment
            };
            const auto renderPassInfo = VkRenderPassCreateInfo {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 1,
                .pDependencies = &dependency,
            };

            auto renderPass = VkRenderPass {};
            const auto result = vkCreateRenderPass(m_engine->getLogicalDevice(), &renderPassInfo, nullptr, &renderPass);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create render pass!");
            }

            m_renderPass = renderPass;
        }

        void createGraphicsPipeline() {
            const auto vertexShaderModule = m_engine->createShaderModuleFromFile("shaders/shader.vert.hlsl.spv");
            const auto fragmentShaderModule = m_engine->createShaderModuleFromFile("shaders/shader.frag.hlsl.spv");

            const auto vertexShaderStageInfo = VkPipelineShaderStageCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = vertexShaderModule,
                .pName = "main",
            };
            const auto fragmentShaderStageInfo = VkPipelineShaderStageCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = fragmentShaderModule,
                .pName = "main",
            };
            const auto shaderStages = std::array<VkPipelineShaderStageCreateInfo, 2> {
                vertexShaderStageInfo,
                fragmentShaderStageInfo
            };
            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();
            const auto vertexInputInfo = VkPipelineVertexInputStateCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 1,
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
                .pVertexBindingDescriptions = &bindingDescription,
                .pVertexAttributeDescriptions = attributeDescriptions.data(),
            };
            const auto inputAssembly = VkPipelineInputAssemblyStateCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .primitiveRestartEnable = VK_FALSE,
            };

            // Without dynamic state, the viewport and scissor rectangle need to be set 
            // in the pipeline using the `VkPipelineViewportStateCreateInfo` struct. This
            // makes the viewport and scissor rectangle for this pipeline immutable.
            // Any changes to these values would require a new pipeline to be created with
            // the new values.
            // ```
            // const auto viewportState = VkPipelineViewportStateCreateInfo {
            //     .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            //     .viewportCount = 1,
            //     .pViewports = &viewport,
            //     .scissorCount = 1,
            //     .pScissors = &scissor,
            // };
            // ```
            const auto viewportState = VkPipelineViewportStateCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .scissorCount = 1,
            };
            const auto rasterizer = VkPipelineRasterizationStateCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .lineWidth = 1.0f,
                .cullMode = VK_CULL_MODE_BACK_BIT,
                // .frontFace = VK_FRONT_FACE_CLOCKWISE,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
            };
            const auto multisampling = VkPipelineMultisampleStateCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .sampleShadingEnable = VK_FALSE,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .pSampleMask = nullptr,            // Optional.
                .alphaToCoverageEnable = VK_FALSE, // Optional.
                .alphaToOneEnable = VK_FALSE,      // Optional.
            };
            const auto depthStencil = VkPipelineDepthStencilStateCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .depthTestEnable = VK_TRUE,
                .depthWriteEnable = VK_TRUE,
                .depthCompareOp = VK_COMPARE_OP_LESS,
                .depthBoundsTestEnable = VK_FALSE,
                .stencilTestEnable = VK_FALSE,
            };
            const auto colorBlendAttachment = VkPipelineColorBlendAttachmentState {
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                .blendEnable = VK_FALSE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional
                .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
                .colorBlendOp = VK_BLEND_OP_ADD,             // Optional
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
                .alphaBlendOp = VK_BLEND_OP_ADD,             // Optional
                // // Alpha blending:
                // // finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor,
                // // finalColor.a = newAlpha.a,
                // .blendEnable = VK_TRUE,
                // .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                // .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                // .colorBlendOp = VK_BLEND_OP_ADD,
                // .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                // .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                // .alphaBlendOp = VK_BLEND_OP_ADD,
            };
            const auto colorBlending = VkPipelineColorBlendStateCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &colorBlendAttachment,
                .blendConstants[0] = 0.0f,
                .blendConstants[1] = 0.0f,
                .blendConstants[2] = 0.0f,
                .blendConstants[3] = 0.0f,
            };

            const auto dynamicStates = std::vector<VkDynamicState> {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };
            const auto dynamicState = VkPipelineDynamicStateCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
                .pDynamicStates = dynamicStates.data(),
            };

            const auto pipelineLayoutInfo = VkPipelineLayoutCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = 1,
                .pSetLayouts = &m_descriptorSetLayout,
                .pushConstantRangeCount = 0,    // Optional
                .pPushConstantRanges = nullptr, // Optional
            };

            auto pipelineLayout = VkPipelineLayout {};
            const auto resultCreatePipelineLayout = vkCreatePipelineLayout(m_engine->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
            if (resultCreatePipelineLayout != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            const auto pipelineInfo = VkGraphicsPipelineCreateInfo {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .stageCount = 2,
                .pStages = shaderStages.data(),
                .pVertexInputState = &vertexInputInfo,
                .pInputAssemblyState = &inputAssembly,
                .pViewportState = &viewportState,
                .pRasterizationState = &rasterizer,
                .pMultisampleState = &multisampling,
                .pDepthStencilState = &depthStencil,
                .pColorBlendState = &colorBlending,
                .pDynamicState = &dynamicState,
                .layout = pipelineLayout,
                .renderPass = m_renderPass,
                .subpass = 0,
                .basePipelineHandle = VK_NULL_HANDLE, // Optional
                .basePipelineIndex = -1,              // Optional
            };

            auto graphicsPipeline = VkPipeline {};
            const auto resultCreateGraphicsPipeline = vkCreateGraphicsPipelines(
                m_engine->getLogicalDevice(), 
                VK_NULL_HANDLE, 
                1, 
                &pipelineInfo, 
                nullptr, 
                &graphicsPipeline
            );
        
            if (resultCreateGraphicsPipeline != VK_SUCCESS) {
                throw std::runtime_error("failed to create graphics pipeline!");
            }

            /*
            vkDestroyShaderModule(m_engine->getLogicalDevice(), fragmentShaderModule, nullptr);
            vkDestroyShaderModule(m_engine->getLogicalDevice(), vertexShaderModule, nullptr);
            */

            m_pipelineLayout = pipelineLayout;
            m_graphicsPipeline = graphicsPipeline;
        }

        void createFramebuffers() {
            auto swapChainFramebuffers = std::vector<VkFramebuffer> { m_swapChainImageViews.size(), VK_NULL_HANDLE };
            for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
                const auto attachments = std::array<VkImageView, 2> {
                    m_swapChainImageViews[i],
                    m_depthImageView,
                };

                const auto framebufferInfo = VkFramebufferCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = m_renderPass,
                    .attachmentCount = static_cast<uint32_t>(attachments.size()),
                    .pAttachments = attachments.data(),
                    .width = m_swapChainExtent.width,
                    .height = m_swapChainExtent.height,
                    .layers = 1,
                };

                auto swapChainFramebuffer = VkFramebuffer {};
                const auto result = vkCreateFramebuffer(
                    m_engine->getLogicalDevice(),
                    &framebufferInfo,
                    nullptr,
                    &swapChainFramebuffer
                );

                if (result != VK_SUCCESS) {
                    throw std::runtime_error("failed to create framebuffer!");
                }

                swapChainFramebuffers[i] = swapChainFramebuffer;
            }

            m_swapChainFramebuffers = std::move(swapChainFramebuffers);
        }

        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
            const auto beginInfo = VkCommandBufferBeginInfo {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = 0,                  // Optional.
                .pInheritanceInfo = nullptr, // Optional.
            };

            const auto resultBeginCommandBuffer = vkBeginCommandBuffer(commandBuffer, &beginInfo);
            if (resultBeginCommandBuffer != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            // NOTE: The order of `clearValues` should be identical to the order of the attachments
            // in the render pass.
            const auto clearValues = std::array<VkClearValue, 2> {
                VkClearValue { .color = VkClearColorValue { { 0.0f, 0.0f, 0.0f, 1.0f } } },
                VkClearValue { .depthStencil = VkClearDepthStencilValue { 1.0f, 0 } },
            };

            const auto renderPassInfo = VkRenderPassBeginInfo {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = m_renderPass,
                .framebuffer = m_swapChainFramebuffers[imageIndex],
                .renderArea.offset = VkOffset2D { 0, 0 },
                .renderArea.extent = m_swapChainExtent,
                .clearValueCount = static_cast<uint32_t>(clearValues.size()),
                .pClearValues = clearValues.data(),
            };

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

            const auto viewport = VkViewport {
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(m_swapChainExtent.width),
                .height = static_cast<float>(m_swapChainExtent.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            };
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            const auto scissor = VkRect2D {
                .offset = VkOffset2D { 0, 0 },
                .extent = m_swapChainExtent,
            };
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            const auto vertexBuffers = std::array<VkBuffer, 1> { m_vertexBuffer };
            const auto offsets = std::array<VkDeviceSize, 1> { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers.data(), offsets.data());

            vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineLayout,
                0,
                1,
                &m_descriptorSets[m_currentFrame],
                0,
                nullptr
            );
        
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_mesh.indices().size()), 1, 0, 0, 0);

            vkCmdEndRenderPass(commandBuffer);

            const auto resultEndCommandBuffer = vkEndCommandBuffer(commandBuffer);
            if (resultEndCommandBuffer != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }

        void createRenderingSyncObjects() {
            auto imageAvailableSemaphores = std::vector<VkSemaphore> { MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE };
            auto renderFinishedSemaphores = std::vector<VkSemaphore> { MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE };
            auto inFlightFences = std::vector<VkFence> { MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE };

            const auto semaphoreInfo = VkSemaphoreCreateInfo {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            const auto fenceInfo = VkFenceCreateInfo {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT,
            };

            for (size_t i = 0; i < imageAvailableSemaphores.size(); i++) {
                const auto result = vkCreateSemaphore(m_engine->getLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
                if (result != VK_SUCCESS) {
                    throw std::runtime_error("failed to create image-available semaphore synchronization object");
                }
            }

            for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) {
                const auto result = vkCreateSemaphore(m_engine->getLogicalDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
                if (result != VK_SUCCESS) {
                    throw std::runtime_error("failed to create render-finished semaphore synchronization object");
                }
            }

            for (size_t i = 0; i < inFlightFences.size(); i++) {
                const auto result = vkCreateFence(m_engine->getLogicalDevice(), &fenceInfo, nullptr, &inFlightFences[i]);
                if (result != VK_SUCCESS) {
                    throw std::runtime_error("failed to create in-flight fence synchronization object");
                }
            }

            m_imageAvailableSemaphores = std::move(imageAvailableSemaphores);
            m_renderFinishedSemaphores = std::move(renderFinishedSemaphores);
            m_inFlightFences = std::move(inFlightFences);
        }

        void updateUniformBuffer(uint32_t currentImage) {
            static auto startTime = std::chrono::high_resolution_clock::now();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            auto ubo = UniformBufferObject {
                .model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                .view = glm::lookAt(
                    glm::vec3(2.0f, 2.0f, 2.0f),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f)
                ),
                .proj = glm::perspective(
                    glm::radians(45.0f),
                    m_swapChainExtent.width / (float) m_swapChainExtent.height,
                    0.1f,
                    10.0f
                ),
            };
            ubo.proj[1][1] *= -1;

            memcpy(m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
        }

        void draw() {
            vkWaitForFences(m_engine->getLogicalDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

            uint32_t imageIndex;
            const auto resultAcquireNextImageKHR = vkAcquireNextImageKHR(
                m_engine->getLogicalDevice(), 
                m_swapChain, 
                UINT64_MAX, 
                m_imageAvailableSemaphores[m_currentFrame], 
                VK_NULL_HANDLE, 
                &imageIndex
            );

            if (resultAcquireNextImageKHR == VK_ERROR_OUT_OF_DATE_KHR) {
                this->recreateSwapChain();
                return;
            } else if (resultAcquireNextImageKHR != VK_SUCCESS && resultAcquireNextImageKHR != VK_SUBOPTIMAL_KHR) {
                throw std::runtime_error("failed to acquire swap chain image!");
            }

            this->updateUniformBuffer(m_currentFrame);

            vkResetFences(m_engine->getLogicalDevice(), 1, &m_inFlightFences[m_currentFrame]);

            vkResetCommandBuffer(m_commandBuffers[m_currentFrame], /* VkCommandBufferResetFlagBits */ 0);
            this->recordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);

            auto waitSemaphores = std::array<VkSemaphore, 1> { m_imageAvailableSemaphores[m_currentFrame] };
            auto waitStages = std::array<VkPipelineStageFlags, 1> { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            auto signalSemaphores = std::array<VkSemaphore, 1> { m_renderFinishedSemaphores[m_currentFrame] };

            const auto submitInfo = VkSubmitInfo {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = waitSemaphores.data(),
                .pWaitDstStageMask = waitStages.data(),
                .commandBufferCount = 1,
                .pCommandBuffers = &m_commandBuffers[m_currentFrame],
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = signalSemaphores.data(),
            };

            const auto resultQueueSubmit = vkQueueSubmit(m_engine->getGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]);
            if (resultQueueSubmit != VK_SUCCESS) {
                throw std::runtime_error("failed to submit draw command buffer!");
            }

            const auto swapChains = std::array<VkSwapchainKHR, 1> { m_swapChain };

            const auto presentInfo = VkPresentInfoKHR {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = signalSemaphores.data(),
                .swapchainCount = 1,
                .pSwapchains = swapChains.data(),
                .pImageIndices = &imageIndex,
            };

            const auto resultQueuePresentKHR = vkQueuePresentKHR(m_engine->getPresentQueue(), &presentInfo);
            if (resultQueuePresentKHR == VK_ERROR_OUT_OF_DATE_KHR || resultQueuePresentKHR == VK_SUBOPTIMAL_KHR || m_engine->hasFramebufferResized()) {
                m_engine->setFramebufferResized(false);
                this->recreateSwapChain();
            } else if (resultQueuePresentKHR != VK_SUCCESS) {
                throw std::runtime_error("failed to present swap chain image!");
            }

            m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        void cleanupSwapChain() {
            vkDestroyImageView(m_engine->getLogicalDevice(), m_depthImageView, nullptr);
            vkDestroyImage(m_engine->getLogicalDevice(), m_depthImage, nullptr);
            vkFreeMemory(m_engine->getLogicalDevice(), m_depthImageMemory, nullptr);

            for (size_t i = 0; i < m_swapChainFramebuffers.size(); i++) {
                vkDestroyFramebuffer(m_engine->getLogicalDevice(), m_swapChainFramebuffers[i], nullptr);
            }

            for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
                vkDestroyImageView(m_engine->getLogicalDevice(), m_swapChainImageViews[i], nullptr);
            }

            vkDestroySwapchainKHR(m_engine->getLogicalDevice(), m_swapChain, nullptr);
        }

        void recreateSwapChain() {
            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(m_engine->getWindow(), &width, &height);
            while (width == 0 || height == 0) {
                glfwGetFramebufferSize(m_engine->getWindow(), &width, &height);
                glfwWaitEvents();
            }

            vkDeviceWaitIdle(m_engine->getLogicalDevice());

            this->cleanupSwapChain();
            this->createSwapChain();
            this->createImageViews();
            this->createDepthResources();
            this->createFramebuffers();
        }
};

int main() {
    auto app = App {};

    try {
        app.run();
    } catch (const std::exception& exception) {
        fmt::println(std::cerr, "{}", exception.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
