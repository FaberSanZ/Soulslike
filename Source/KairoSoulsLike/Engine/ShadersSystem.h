#pragma once

#include <Windows.h>
#include <dxcapi.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#pragma comment(lib, "dxcompiler.lib")

namespace Engine
{

    class ShadersSystem final
    {
    public:
        ShadersSystem()
        {
            ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)), "Failed to create the DXC compiler.");
            const HRESULT result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
            if (FAILED(result))
            {
                Release(m_compiler);
                ThrowIfFailed(result, "Failed to create the DXC utilities.");
            }
        }

        ~ShadersSystem()
        {
            Release(m_utils);
            Release(m_compiler);
        }

        ShadersSystem(const ShadersSystem&) = delete;
        ShadersSystem& operator=(const ShadersSystem&) = delete;

        IDxcBlob* Compile(const std::wstring& shaderPath, const std::wstring& entryPoint, const std::wstring& targetProfile)
        {
            const std::wstring resolvedPath = ResolveShaderPath(shaderPath);
            if (resolvedPath.empty())
            {
                OutputDebugStringW((L"Shader not found: " + shaderPath + L"\n").c_str());
                AppendShaderLog("Shader not found: " + std::filesystem::path(shaderPath).string());
                return nullptr;
            }

            IDxcBlobEncoding* source = nullptr;
            const HRESULT loadResult = m_utils->LoadFile(resolvedPath.c_str(), nullptr, &source);
            if (FAILED(loadResult) || !source)
            {
                OutputDebugStringW((L"Failed to load shader: " + resolvedPath + L"\n").c_str());
                AppendShaderLog("Failed to load shader: " + std::filesystem::path(resolvedPath).string());
                return nullptr;
            }

            IDxcIncludeHandler* includeHandler = nullptr;
            const HRESULT includeResult = m_utils->CreateDefaultIncludeHandler(&includeHandler);
            if (FAILED(includeResult))
            {
                Release(source);
                ThrowIfFailed(includeResult, "Failed to create the DXC include handler.");
            }

            LPCWSTR arguments[] = { resolvedPath.c_str(), L"-E", entryPoint.c_str(), L"-T", targetProfile.c_str(), L"-Zi", L"-Qembed_debug", L"-Od" };

            IDxcOperationResult* operation = nullptr;
            const HRESULT compileResult = m_compiler->Compile(source, resolvedPath.c_str(), entryPoint.c_str(), targetProfile.c_str(), arguments, _countof(arguments), nullptr, 0,
                includeHandler, &operation);
            Release(includeHandler);
            Release(source);
            ThrowIfFailed(compileResult, "DXC failed to start shader compilation.");

            HRESULT status = E_FAIL;
            const HRESULT statusResult = operation->GetStatus(&status);
            if (FAILED(statusResult))
            {
                Release(operation);
                ThrowIfFailed(statusResult, "DXC failed to report shader compilation status.");
            }

            if (FAILED(status))
            {
                IDxcBlobEncoding* errors = nullptr;
                operation->GetErrorBuffer(&errors);
                if (errors)
                {
                    const auto* message = static_cast<const char*>(errors->GetBufferPointer());
                    OutputDebugStringA(message);
                    AppendShaderLog(std::string("DXC error: ") + message);
                }
                else
                {
                    AppendShaderLog("DXC error: unknown");
                }

                Release(errors);
                Release(operation);
                return nullptr;
            }

            IDxcBlob* shader = nullptr;
            const HRESULT result = operation->GetResult(&shader);
            Release(operation);
            ThrowIfFailed(result, "DXC did not return compiled shader bytecode.");
            return shader;
        }

    private:
        template <typename T> static void Release(T*& object)
        {
            if (object)
            {
                object->Release();
                object = nullptr;
            }
        }

        static void ThrowIfFailed(HRESULT result, const char* message)
        {
            if (FAILED(result))
                throw std::runtime_error(message);
        }

        static void AppendShaderLog(const std::string& line)
        {
            const std::filesystem::path logPath = std::filesystem::temp_directory_path() / "gametools_shader.log";
            std::ofstream output(logPath, std::ios::app);
            if (output.is_open())
                output << line << '\n';
        }

        static std::wstring ResolveShaderPath(const std::wstring& shaderPath)
        {
            const std::filesystem::path input(shaderPath);
            if (std::filesystem::exists(input))
                return input.wstring();

            wchar_t modulePath[MAX_PATH]{};
            GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
            std::filesystem::path parent = std::filesystem::path(modulePath).parent_path();

            std::filesystem::path candidate = (parent / input).lexically_normal();
            if (std::filesystem::exists(candidate))
                return candidate.wstring();

            for (int i = 0; i < 4; ++i)
            {
                parent = parent.parent_path();
                candidate = (parent / input).lexically_normal();
                if (std::filesystem::exists(candidate))
                    return candidate.wstring();
            }

            return {};
        }

        IDxcCompiler* m_compiler = nullptr;
        IDxcUtils* m_utils = nullptr;
    };

}
