#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <string>

FuncItem funcItem[nbFunc];

NppData nppData;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

void commandMenuInit()
{
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    setCommand(0, TEXT("Format Script"), formatScript, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}
//---------------------//
//-- HELPER METHODS --//
//-------------------//
HWND getCurrentScintilla()
{
    int which = 0; // 0 = main, 1 = secondary
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);

    return (which == 0) ? nppData._scintillaMainHandle
        : nppData._scintillaSecondHandle;
}

std::string getAllText()
{
    HWND cur = getCurrentScintilla();
    if (!cur)
        return "";

    int len = (int)::SendMessage(cur, SCI_GETTEXTLENGTH, 0, 0);

    std::string text;
    text.resize(len + 1);

    ::SendMessage(cur, SCI_GETTEXT, len + 1, (LPARAM)text.data());

    return text;
}
std::string formatTernary(const std::string& input)
{
    std::string out;
    int indent = 0;

    auto emitIndent = [&out, &indent]()
        {
            out.append(indent * 4, ' '); // 4 spaces per indent level
        };

    for (char c : input)
    {
        if (c == '?')
        {
            // space before ? for readability
            out += " ?";

            // go to a new line and increase indent for the following part
            out += '\n';
            ++indent;
            emitIndent();
        }
        else if (c == ':')
        {
            // newline before : and reduce indent
            out += '\n';
            if (indent > 0)
                --indent;
            emitIndent();
            out += ": ";
        }
        else
        {
            out += c;
        }
    }

    return out;
}
//---------------------------//
//-- ASSOCIATED FUNCTIONS --//
//-------------------------//
void formatScript()
{
    std::string text = getAllText();
    std::string formatted = formatTernary(text);

    HWND cur = getCurrentScintilla();
    if (!cur)
        return;

    ::SendMessage(cur, SCI_BEGINUNDOACTION, 0, 0);
    ::SendMessage(cur, SCI_SETTEXT, 0, (LPARAM)formatted.c_str());
    ::SendMessage(cur, SCI_ENDUNDOACTION, 0, 0);
}


