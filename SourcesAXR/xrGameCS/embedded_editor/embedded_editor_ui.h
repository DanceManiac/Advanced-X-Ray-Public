#pragma once

struct CUIDebugState;

class CUIDebuggable
{
public:
    virtual ~CUIDebuggable();

    void RegisterDebuggable();
    void UnregisterDebuggable();

    virtual pcstr GetDebugType() = 0;

    virtual bool FillDebugTree(const CUIDebugState& debugState) = 0;
    virtual void FillDebugInfo() = 0;
};

inline pcstr CUIDebuggable::GetDebugType() { return "CUIDebuggable"; }

struct CUIDebugState
{
    CUIDebuggable* selected{};
    mutable CUIDebuggable* newSelected{};
    bool drawWndRects{ true };
    bool coloredRects{ true };

    void select(CUIDebuggable* debuggable) const
    {
        if (selected == debuggable)
            newSelected = nullptr;
        else
            newSelected = debuggable;
    }
};

class CUIDebugger
{
public:
    xr_vector<CUIDebuggable*> m_root_windows;

    CUIDebugState m_state;

    CUIDebugger();

    void Register(CUIDebuggable* debuggable);
    void Unregister(CUIDebuggable* debuggable);

    //void OnFrame() override;

    [[nodiscard]]
    CUIDebuggable* GetSelected() const { return m_state.selected; }
    void SetSelected(CUIDebuggable* debuggable);

    [[nodiscard]]
    bool ShouldDrawRects() const { return m_state.drawWndRects; }

private:
    //pcstr tool_name() override { return "UI Debugger"; }
};

void ShowUIEditor(bool& show);
bool UIEditor_MouseWheel(float wheel);