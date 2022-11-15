#pragma once

struct GameMainThread
{
    static GameMainThread& Get();

    void AddBaseInitializationTask(const std::function<bool()>& aFunction);
    void AddInitializationTask(const std::function<bool()>& aFunction);
    void AddRunningTask(const std::function<bool()>& aFunction);
    void AddShutdownTask(const std::function<bool()>& aFunction);
    void AddGenericTask(const std::function<bool()>& aFunction);

    ~GameMainThread();

private:
    GameMainThread();

    void Hook();
    void Unhook();

    using TStateTick = bool(RED4ext::IGameState*, RED4ext::CGameApplication*);

    static bool HookStateTick(RED4ext::IGameState* apThisState, RED4ext::CGameApplication* apGameApplication);

    // helper task queue which executes added tasks each drain until they are finished
    struct RepeatedTaskQueue
    {
        void AddTask(const std::function<bool()>& aFunction);
        void Drain();

    private:
        std::recursive_mutex m_mutex;
        TiltedPhoques::Vector<std::function<bool()>> m_tasks;
    };

    struct StateTickOverride
    {
        StateTickOverride(const uintptr_t acOffset, const char* acpRealFunctionName);

        bool OnTick(RED4ext::IGameState*, RED4ext::CGameApplication*);

        const uintptr_t Offset;
        const char* RealFunctionName;

        uint8_t* Location = nullptr;
        TStateTick* RealFunction = nullptr;
        RepeatedTaskQueue Tasks;
    };

    std::array<StateTickOverride, 4> m_stateTickOverrides {
        StateTickOverride(CyberEngineTweaks::Addresses::CBaseInitializationState_OnTick, "CBaseInitializationState::OnTick"),
        StateTickOverride(CyberEngineTweaks::Addresses::CInitializationState_OnTick, "CInitializationState::OnTick"),
        StateTickOverride(CyberEngineTweaks::Addresses::CRunningState_OnTick, "CRunningState::OnTick"),
        StateTickOverride(CyberEngineTweaks::Addresses::CShutdownState_OnTick, "CShutdownState::OnTick")
    };

    RepeatedTaskQueue m_genericQueue;
};
