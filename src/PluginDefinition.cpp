#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <string>
#include <stack>
#include <unordered_map>

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


bool setCommand(size_t index, TCHAR* cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey* sk, bool check0nInit)
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

std::vector<std::unordered_map<int,int>> createMapping(const std::string& input) {
	std::stack<int> identifierStack;
	std::stack<int> paranthesisStack;

	std::unordered_map<int, int> pairIdentifierMap;
	std::unordered_map<int, int> pairParanthesisMap;

	int curIndex = 0;
	for (char c : input)
	{
		if (c == '?')
		{
			identifierStack.push(curIndex);
		}
		else if (c == ':')
		{
			pairIdentifierMap[identifierStack.top()] = curIndex;
			identifierStack.pop();

		}
		else if (c == '(')
		{
			paranthesisStack.push(curIndex);
		}
		else if (c == ')') {
			pairParanthesisMap[paranthesisStack.top()] = curIndex;
			paranthesisStack.pop();
		}
		curIndex++;
	}

	std::vector<std::unordered_map<int, int>> result;
	result.push_back(std::move(pairIdentifierMap));
	result.push_back(std::move(pairParanthesisMap));

	return result;
}
std::string formatTernary(const std::string& input)
{
	std::string out;
	int indent = 0;
	int index = -1;

	auto emitIndent = [&out, &indent]()
		{
			out.append(indent * 4, ' ');
		};

	for (char c : input)
	{
		bool doIntend = false;
		index++;
		if (c == '?')
		{
			if (index != 0 && input[index - 1] != ' ') {
				out += " ";
			}

			out += "?";

			if (index != input.length() - 1 && input[index + 1] != '\n') {
				out += '\n';
				doIntend = true;
			}

			if (doIntend) {
				++indent;
				emitIndent();
			}
		}
		else if (c == ':')
		{
			if (index != 0 && input[index - 1] != '\n') {
				out += '\n';
				doIntend = true;
			}
			if (indent > 0)
				--indent;
			if (doIntend)
				emitIndent();
			out += ":";
			if (index != input.length() - 1 && input[index + 1] != ' ') {
				out += " ";
			}
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
	auto maps = createMapping(text);
	auto& identifierMap = maps[0];
	auto& parenthesisMap = maps[1];
	//std::string formatted = formatTernary(text);

	std::string msg =
		"Identifier pairs: " + std::to_string(identifierMap.size()) + "\r\n" +
		"Parenthesis pairs: " + std::to_string(parenthesisMap.size());

	MessageBoxA(
		nppData._nppHandle,         
		msg.c_str(),                
		"SFSScript - Mapping Counts",
		MB_OK | MB_ICONINFORMATION
	);

	//HWND cur = getCurrentScintilla();
	//if (!cur)
	//	return;

	//::SendMessage(cur, SCI_BEGINUNDOACTION, 0, 0);
	//::SendMessage(cur, SCI_SETTEXT, 0, (LPARAM)formatted.c_str());
	//::SendMessage(cur, SCI_ENDUNDOACTION, 0, 0);
}


