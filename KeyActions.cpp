#include "KeyActions.h"

#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

#include "main.h"
#include "WindowManager.h"
#include "Settings.h"
#include "RenderstateManager.h"

KeyActions KeyActions::instance;

void KeyActions::load() {
	std::ifstream sfile;
	sfile.open(GetDirectoryFile("DSfixKeys.ini"), std::ios::in);
	char buffer[256];
	while(!sfile.eof()) {
		sfile.getline(buffer, 256);
		if(buffer[0] == '#') continue;
		if(sfile.gcount() <= 1) continue;
		string bstring(buffer);
		
		size_t pos = bstring.npos;
		char postChar;
		#define KEY(_name, _val) \
		pos = bstring.find(#_name); \
		postChar = buffer[pos + strlen(#_name)]; \
		if(pos != bstring.npos && (postChar == '\r' || postChar == '\n' || postChar == ' ' || postChar == '\0')) { \
			string action; stringstream ss(bstring); ss >> action; \
			keyBindingMap.insert(make_pair(_val, action)); \
		}
		#include "Keys.def"
		#undef KEY
	}
	sfile.close();
}

void KeyActions::report() {
	SDLOG(0, "= Loaded Keybindings:\n");
	for(IntStrMap::const_iterator i = keyBindingMap.begin(); i != keyBindingMap.end(); ++i) {
		SDLOG(0, " - %p => %s\n", i->first, i->second.c_str());
	}
	SDLOG(0, "=============\n");
	
	SDLOG(5, "= Possible Actions:\n");
	#define ACTION(_name, _action) \
	SDLOG(5, "%s, ", #_name);
 	#include "Actions.def"
	#undef ACTION
	SDLOG(5, "=============\n");
	
	SDLOG(5, "= Possible Keys:\n");
	#define KEY(_name, _val) \
	SDLOG(5, "%s, ", #_name);
 	#include "Keys.def"
	#undef KEY
	SDLOG(5, "=============\n");
}

void KeyActions::performAction(const char* name) {
	#define ACTION(_name, _action) \
	if(strcmp(#_name, name) == 0) _name();
 	#include "Actions.def"
	#undef ACTION
}

void KeyActions::processIO() {
	if(::GetForegroundWindow() != NULL && ::GetActiveWindow() != NULL) {
		for(IntStrMap::const_iterator i = keyBindingMap.begin(); i != keyBindingMap.end(); ++i) {
			if(GetAsyncKeyState(i->first)&1) {
				SDLOG(0, "Action triggered: %s\n", i->second.c_str());
				performAction(i->second.c_str());
			}
		}
	}
}


#define ACTION(_name, _action) \
void KeyActions::##_name() { _action; };
#include "Actions.def"
#undef ACTION
