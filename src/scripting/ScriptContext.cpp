#include <stdafx.h>

#include "ScriptContext.h"

ScriptContext::ScriptContext(sol::state_view aStateView, const std::filesystem::path& acPath)
    : m_lua(aStateView)
    , m_env(aStateView, sol::create, aStateView.globals())
    , m_name(relative(acPath, Paths::Get().ModsRoot()).string())
{
    m_env["registerForEvent"] = [this](const std::string& acName, sol::function aCallback)
    {
        if(acName == "onInit")
            m_onInit = aCallback;
        else if(acName == "onShutdown")
            m_onShutdown = aCallback;
        else if(acName == "onUpdate")
            m_onUpdate = aCallback;
        else if(acName == "onDraw")
            m_onDraw = aCallback;
        else if(acName == "onOverlayOpen")
            m_onOverlayOpen = aCallback;
        else if(acName == "onOverlayClose")
            m_onOverlayClose = aCallback;
        else
            Logger::ErrorToModsFmt("Tried to register unknown handler '{}'!", acName);
    };

    m_env["registerHotkey"] = [this](const std::string& acID, const std::string& acDescription, sol::function aCallback)
    {
        if (acID.empty() ||
            (std::find_if(acID.cbegin(), acID.cend(), [](char c){ return !(isalpha(c) || isdigit(c) || c == '_'); }) != acID.cend()))
        {
            Logger::ErrorToModsFmt("Tried to register hotkey with incorrect ID format '{}'! ID needs to be alphanumeric without any whitespace or special characters (exception being '_' which is allowed in ID)!", acID);
            return;
        }

        if (acDescription.empty())
        {
            Logger::ErrorToModsFmt("Tried to register hotkey with empty description! (ID of hotkey handler: {})", acID);
            return;
        }
        
        std::string vkBindID = m_name + '.' + acID;
        VKBind vkBind = { vkBindID, acDescription, [aCallback]()
        {
            // TODO: proper exception handling!
            try
            {
                if (aCallback)
                    aCallback();
            }
            catch(std::exception& e)
            {
                Logger::ErrorToMods(e.what());
            }
        }};
        m_vkBindInfos.emplace_back(VKBindInfo{vkBind});
    };

    // TODO: proper exception handling!
    try
    {
        const auto path = acPath / "init.lua";
        const auto result = m_lua.script_file(path.string(), m_env);

        if (result.valid())
        {
            m_initialized = true;
            m_object = result;
        }
        else
        {
            sol::error err = result;
            Logger::ErrorToMods(err.what());
        }
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}

ScriptContext::ScriptContext(ScriptContext&& other) noexcept : ScriptContext(other)
{
    other.m_initialized = false;
}

ScriptContext::~ScriptContext()
{
    if (m_initialized)
        TriggerOnShutdown();
}

bool ScriptContext::IsValid() const
{
    return m_initialized;
}

const std::vector<VKBindInfo>& ScriptContext::GetBinds() const
{
    return m_vkBindInfos;
}

void ScriptContext::TriggerOnInit() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onInit)
            m_onInit();
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}

void ScriptContext::TriggerOnUpdate(float aDeltaTime) const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onUpdate)
            m_onUpdate(aDeltaTime);
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}

void ScriptContext::TriggerOnDraw() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onDraw)
            m_onDraw();
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}
    
void ScriptContext::TriggerOnOverlayOpen() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onOverlayOpen)
            m_onOverlayOpen();
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}
void ScriptContext::TriggerOnOverlayClose() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onOverlayClose)
            m_onOverlayClose();
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}

sol::object ScriptContext::GetRootObject() const
{
    return m_object;
}

void ScriptContext::TriggerOnShutdown() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onShutdown)
            m_onShutdown();
    }
    catch(std::exception& e)
    {
        Logger::ErrorToMods(e.what());
    }
}
