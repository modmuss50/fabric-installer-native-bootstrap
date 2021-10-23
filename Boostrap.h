#pragma once

#include <Windows.h>

#include <string>
#include <optional>

namespace Installer {
	class Boostrap
	{
	public:
		void launch();

	private:
		void showMissingJavaError();

		std::optional<std::wstring> getMinecraftInstallationDirectory();
		void launchUsingMinecraftJava(const std::wstring& launcherDir);

		void tryLaunch(const std::wstring& javaPath, bool checkExists = true);
		bool checkVersionExitCode(const std::wstring& javaPath);

		std::optional<std::wstring> getEnvVar(const std::wstring& key);

		std::wstring getModuleFileName();
	};
}