#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <string>

FuncItem funcItem[nbFunc];

NppData nppData;

void pluginInit(HANDLE)
{
}

void pluginCleanUp()
{
}

void commandMenuInit()
{
    setCommand(0, TEXT("Format Script"), formatScript, false);
}

void commandMenuCleanUp()
{
}


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
    int which = 0; 
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
            out.append(indent * 4, ' '); 
        };

    for (char c : input)
    {
        if (c == '?')
        {
            out += " ?";

            out += '\n';
            ++indent;
            emitIndent();
        }
        else if (c == ':')
        {
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


