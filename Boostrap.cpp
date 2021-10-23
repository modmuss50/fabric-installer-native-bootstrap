#include "Boostrap.h"

#include <iostream>
#include <vector>
#include <shlwapi.h>

namespace Installer {
    constexpr LPCWSTR ERROR_TITLE = L"Fabric Installer";
    constexpr LPCWSTR ERROR_MESSAGE = L"The Fabric Installer could not find a valid Java installation.\n\nWould you like to open the Fabric wiki to find out how to fix this?\n\nURL: https://fabricmc.net/wiki/player:tutorials:java:windows";
    constexpr LPCWSTR ERROR_URL = L"https://fabricmc.net/wiki/player:tutorials:java:windows";

    constexpr LPCWSTR REG_PATH = LR"(SOFTWARE\Mojang\InstalledProducts\Minecraft Launcher)";
    constexpr LPCWSTR REG_KEY = L"InstallLocation";

    constexpr LPCWSTR JAVA_HOME_ENV_KEY = L"JAVA_HOME";
    constexpr LPCWSTR JAVA_HOME_PATH = L"bin/javaw.exe";

    constexpr LPCWSTR JAVA_PATH = L"javaw";

    std::vector<LPCWSTR> MC_JAVA_PATHS = {
        LR"(runtime\java-runtime-alpha\windows-x64\java-runtime-alpha\bin\javaw.exe)",
        LR"(runtime\java-runtime-alpha\windows-x86\java-runtime-alpha\bin\javaw.exe)",
        LR"(runtime\jre-legacy\windows-x64\jre-legacy\bin\javaw.exe)",
        LR"(runtime\jre-legacy\windows-x86\jre-legacy\bin\javaw.exe)",
        LR"(runtime\jre-x64\bin\javaw.exe)",
        LR"(runtime\jre-x86\bin\javaw.exe)",
    };

    void Boostrap::launch()
    {
        // Try and launch using the java downloaded via the minecraft launcher.
        if (const auto opt = getMinecraftInstallationDirectory()) {
            launchUsingMinecraftJava(opt.value());
        }
        else {
            std::wcout << L"Could not find minecraft installation directory from the registry." << std::endl;
        }

        // Try using java in JAVA_HOME
        if (const auto javaHome = getEnvVar(JAVA_HOME_ENV_KEY)) {
            std::wstring path = javaHome.value() + JAVA_HOME_PATH;
            tryLaunch(path, false);
        }

        // Last try using javaw on the path.
        tryLaunch(JAVA_PATH, false);

        // Give up and show an error.
        showMissingJavaError();
    }

    void Boostrap::showMissingJavaError()
    {
        int result = MessageBoxW(
            NULL,
            ERROR_MESSAGE,
            ERROR_TITLE,
            MB_ICONWARNING | MB_YESNO
        );

        if (result == IDYES) {
            ShellExecute(0, 0, ERROR_URL, 0, 0, SW_SHOW);
        }
    }

    // See: https://docs.microsoft.com/en-us/archive/msdn-magazine/2017/may/c-use-modern-c-to-access-the-windows-registry#reading-a-string-value-from-the-registry
    std::optional<std::wstring> Boostrap::getMinecraftInstallationDirectory()
    {
        DWORD dataSize{};
        LONG retCode = ::RegGetValue(
            HKEY_CURRENT_USER,
            REG_PATH,
            REG_KEY,
            RRF_RT_REG_SZ,
            nullptr,
            nullptr,
            &dataSize
        );

        if (retCode != ERROR_SUCCESS)
        {
            return std::nullopt;
        }

        std::wstring data;
        data.resize(dataSize / sizeof(wchar_t));

        retCode = ::RegGetValue(
            HKEY_CURRENT_USER,
            REG_PATH,
            REG_KEY,
            RRF_RT_REG_SZ,
            nullptr,
            &data[0],
            &dataSize
        );

        if (retCode != ERROR_SUCCESS)
        {
            return std::nullopt;
        }

        DWORD stringLengthInWchars = dataSize / sizeof(wchar_t);
        stringLengthInWchars--; // Exclude the NUL written by the Win32 API
        data.resize(stringLengthInWchars);

        return data;
    }

    void Boostrap::launchUsingMinecraftJava(const std::wstring& launcherDir)
    {
        for (const LPCWSTR path : MC_JAVA_PATHS) {
            const std::wstring fullPath = launcherDir + path;
            tryLaunch(fullPath);
        }
    }

    void Boostrap::tryLaunch(const std::wstring& javaPath, bool checkExists)
    {
        if (checkExists && !::PathFileExistsW(javaPath.c_str())) {
            return;
        }

        std::wstring args = L"-jar " + getModuleFileName();
        wchar_t* arg_concat = const_cast<wchar_t*>(args.c_str());

        STARTUPINFO info = { sizeof(info) };
        PROCESS_INFORMATION processInfo;
        if (::CreateProcessW(
            javaPath.c_str(),
            arg_concat,
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            nullptr,
            &info,
            &processInfo
        )) {
            // No need to wait around, close these and exit.
            ::CloseHandle(processInfo.hProcess);
            ::CloseHandle(processInfo.hThread);
            exit(0);
        }

        // Something didnt work out here, contiune on.
    }

    bool Boostrap::checkVersionExitCode(const std::wstring& javaPath)
    {
        bool valid = false;
        STARTUPINFO info = { sizeof(info) };
        PROCESS_INFORMATION processInfo;

        std::wstring args = L"--version";
        wchar_t* arg_concat = const_cast<wchar_t*>(args.c_str());

        // Create the child process
        if (::CreateProcessW(
            javaPath.c_str(),
            arg_concat,
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            nullptr,
            &info,
            &processInfo)) {

            // Wait for exit
            ::WaitForSingleObject(processInfo.hProcess, INFINITE);

            // Read exit code
            DWORD exitCode;
            ::GetExitCodeProcess(processInfo.hProcess, &exitCode);
            if (exitCode == 0) {
                // Exit code 0, --version worked ok.
                valid = true;
            }

            ::CloseHandle(processInfo.hThread);
            ::CloseHandle(processInfo.hProcess);
        }

        return valid;
    }

    std::optional<std::wstring> Boostrap::getEnvVar(const std::wstring& key)
    {
        // Read the size of the env var
        DWORD size = ::GetEnvironmentVariableW(key.c_str(), nullptr, 0);
        if (!size || ::GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
            return std::nullopt;
        }

        // Read the env var
        std::wstring value(size, L'\0');
        size = ::GetEnvironmentVariableW(key.c_str(), &value[0], size);
        if (!size || size >= value.size()) {
            return std::nullopt;
        }

        value.resize(size);
        return value;
    }

    std::wstring Boostrap::getModuleFileName()
    {
        wchar_t moduleFileName[MAX_PATH] = { 0 };
        ::GetModuleFileNameW(nullptr, moduleFileName, MAX_PATH);
        return moduleFileName;
    }
}
