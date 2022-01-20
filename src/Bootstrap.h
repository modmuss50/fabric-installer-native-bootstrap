#pragma once

#include <memory>

#include "utils/ISystemHelper.h"

namespace Bootstrap {
    class Bootstrap {
    public:
        explicit Bootstrap(const std::shared_ptr<Utils::ISystemHelper> &systemHelper);

    public:
        void launch();

    private:
        bool launchMinecraftLauncher();
        bool launchSystemJava();

        bool attemptLaunch(const std::wstring& path, bool checkExists);

        void showErrorMessage();

    private:
        std::shared_ptr<Utils::ISystemHelper> systemHelper;
    };
}

