#pragma once
#include <map>
#include <vector>

void initGSCustomLanguage();
void deinitGSCustomLanguage();
char* add_cartographer_label(int label_menu_id, int label_id, char* label);
char* add_cartographer_label(int label_menu_id, int label_id, char* label, bool is_dynamic);
char* add_cartographer_label(int label_menu_id, int label_id, int labelBufferLen);
char* add_cartographer_label(int label_menu_id, int label_id, int labelBufferLen, bool is_dynamic);
char* H2CustomLanguageGetLabel(int label_menu_id, int label_id);
void setCustomLanguage(int);
void setCustomLanguage(int, int);
bool reloadCustomLanguages();
void saveCustomLanguages();
void combineCartographerLabels(int menuId, int lbl1, int lbl2, int lblCmb);

typedef struct {
	int lang_base;
	int lang_variant;
	bool other;
	char* lang_name;
	char* font_table_filename;
	std::map<int, std::map<int, char*>>* label_map;
} custom_language;

extern std::vector<custom_language*> custom_languages;

