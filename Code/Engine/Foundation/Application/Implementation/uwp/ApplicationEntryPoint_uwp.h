
#pragma once

/// \file

#include <Foundation/Logging/Log.h>
#include <Foundation/Memory/MemoryTracker.h>

// Disable C++/CX adds.
#pragma warning(disable : 4447)

class ezApplication;
extern EZ_FOUNDATION_DLL ezResult ezUWPRun(ezApplication* pApp);

namespace ezApplicationDetails
{
  EZ_FOUNDATION_DLL ezResult InitializeWinrt();
  EZ_FOUNDATION_DLL void UninitializeWinrt();

  template <typename AppClass, typename... Args>
  int EntryFunc(Args&&... arguments)
  {
    alignas(EZ_ALIGNMENT_OF(AppClass)) static char appBuffer[sizeof(AppClass)]; // Not on the stack to cope with smaller stacks.

    if (InitializeWinrt().Failed())
    {
      return 1;
    }

    AppClass* pApp = new (appBuffer) AppClass(std::forward<Args>(arguments)...);

    ezUWPRun(pApp).IgnoreResult();

    const int iReturnCode = pApp->GetReturnCode();
    if (iReturnCode != 0)
    {
      std::string text = pApp->TranslateReturnCode();
      if (!text.empty())
        printf("Return Code: '%s'\n", text.c_str());
    }

    UninitializeWinrt();

    return iReturnCode;
  }
} // namespace ezApplicationDetails

/// \brief Same as EZ_APPLICATION_ENTRY_POINT but should be used for applications that shall always show a console window.
#define EZ_CONSOLEAPP_ENTRY_POINT(AppClass, ...)                                                                                 \
  alignas(EZ_ALIGNMENT_OF(AppClass)) static char appBuffer[sizeof(AppClass)]; /* Not on the stack to cope with smaller stacks */ \
                                                                                                                                 \
  EZ_APPLICATION_ENTRY_POINT_CODE_INJECTION                                                                                      \
  int main(int argc, const char** argv) { return ::ezApplicationDetails::EntryFunc<AppClass>(__VA_ARGS__); }

/// \brief This macro allows for easy creation of application entry points (since they can't be placed in DLLs)
///
/// Just use the macro in a cpp file of your application and supply your app class (must be derived from ezApplication).
/// The additional (optional) parameters are passed to the constructor of your app class.
#define EZ_APPLICATION_ENTRY_POINT(AppClass, ...)                                                   \
  EZ_APPLICATION_ENTRY_POINT_CODE_INJECTION                                                         \
  int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) \
  {                                                                                                 \
    return ::ezApplicationDetails::EntryFunc<AppClass>(__VA_ARGS__);                                \
  }
