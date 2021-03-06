#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <cstdint>
#include <cassert>
#include <sstream>
#include <time.h>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include "lua.hpp"
int struct_type;// 0 for tree  1 for hash
using namespace std;
struct Table_h{
    vector<string> attrname;
    unordered_map<string, int> attrsize;// 2. define the BookName type
    unordered_map<string, multimap<string, int>> attrvar_i; //1.map<Name, multimap<Jack, back 1>>
    unordered_map<string, vector<string>> attrvar; // 3.map<bookname, 1> output: ICE
    unordered_map<string, multimap<int, int>> attrint_i;
    unordered_map<string, vector<int>> attrint;
    string primkey;
    int rownum;
};
unordered_map<string, struct Table_h*> tables_h;

struct Table
{
    vector<string> attrname;
    map<string, int> attrsize;// 2. define the BookName type
    map<string, multimap<string, int>> attrvar_i; //1.map<Name, multimap<Jack, back 1>>
    map<string, vector<string>> attrvar; // 3.map<bookname, 1> output: ICE
    map<string, multimap<int, int>> attrint_i;
    map<string, vector<int>> attrint;
    string primkey;
    int rownum;
};

map<string, struct Table*> tables;
void create_table_h(lua_State* L)
{
    struct Table_h* currtable = nullptr;
    // Get table name to create
    lua_pushstring(L, "table_name");
    lua_rawget(L, -2);
    if(tables_h.find(string(lua_tostring(L, -1))) == tables_h.end()) {
        currtable = tables_h[string(lua_tostring(L, -1))] = new Table_h();
    } else {
        cout << "error: ";
        cout << "Duplicate table name: " << string(lua_tostring(L, -1)) << endl;
        lua_pop(L, 1);
        return;
    }
    lua_pop(L, 1);
    // Get attribute list
    lua_pushstring(L, "attr");
    lua_rawget(L, -2);
    int attr_num = lua_objlen(L, -1);
    for(auto i = 1; i <= attr_num; i++) {
        lua_rawgeti(L, -1, i);
        currtable->attrname.push_back(string(lua_tostring(L, -1)));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    // Get type list
    lua_pushstring(L, "attrsize");
    lua_rawget(L, -2);
    for(auto i = 1; i <= attr_num; i++) {
        lua_rawgeti(L, -1, i);
        currtable->attrsize[currtable->attrname[i-1]] = lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    // Get primary key
    lua_pushstring(L, "primattr");
    lua_rawget(L, -2);
    if (lua_isnil(L, -1)) {
        // No primary key
        lua_pop(L, 1);
    } else {
        currtable->primkey = string(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    // Initialize row number
    currtable->rownum = 0;
    cout << "done." << endl;
}

void create_table(lua_State* L)
{
    struct Table* currtable = nullptr;
    // Get table name to create
    lua_pushstring(L, "table_name");
    lua_rawget(L, -2);
    if(tables.find(string(lua_tostring(L, -1))) == tables.end()) {
        currtable = tables[string(lua_tostring(L, -1))] = new Table();
    } else {
        cout << "error: ";
        cout << "Duplicate table name: " << string(lua_tostring(L, -1)) << endl;
        lua_pop(L, 1);
        return;
    }
    lua_pop(L, 1);
    // Get attribute list
    lua_pushstring(L, "attr");
    lua_rawget(L, -2);
    int attr_num = lua_objlen(L, -1);
    for(auto i = 1; i <= attr_num; i++) {
        lua_rawgeti(L, -1, i);
        currtable->attrname.push_back(string(lua_tostring(L, -1)));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    // Get type list
    lua_pushstring(L, "attrsize");
    lua_rawget(L, -2);
    for(auto i = 1; i <= attr_num; i++) {
        lua_rawgeti(L, -1, i);
        currtable->attrsize[currtable->attrname[i-1]] = lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    // Get primary key
    lua_pushstring(L, "primattr");
    lua_rawget(L, -2);
    if (lua_isnil(L, -1)) {
        // No primary key
        lua_pop(L, 1);
    } else {
        currtable->primkey = string(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    // Initialize row number
    currtable->rownum = 0;
    cout << "done." << endl;
}
void insert_into_h(lua_State* L)
{
    struct Table_h* currtable = nullptr;
    int value_num = 0;
    vector<string> attr_list;
    deque<string> str_v;
    deque<int> int_v;

    // Get table name to insert into
    lua_pushstring(L, "table_name");
    lua_rawget(L, -2);
    string table_name = string(lua_tostring(L, -1));
    if(tables_h.find(table_name) != tables_h.end()) {
        currtable = tables_h[string(lua_tostring(L, -1))];
        lua_pop(L, 1);
    } else {
        cout << "error: ";
        cout << "Table " << string(lua_tostring(L, -1));
        cout << " does not exist" << endl;
        lua_pop(L, 1);
        return;
    }
    // See if attribute list is provided
    lua_pushstring(L, "attr");
    lua_rawget(L, -2);
    if (lua_objlen(L, -1) == 0) {
        // No attribute list, assume same as schema
        attr_list = currtable->attrname;
        // Pop the empty attribute list
        lua_pop(L, 1);
    } else {
        // Get attribute list
        int attr_num = lua_objlen(L, -1);
        if (attr_num != currtable->attrsize.size()) {
            cout << "error: ";
            cout << "Attribute list length differs from table schema" << endl;
            lua_pop(L, 1);
            return;
        }
        for(int i = 0; i < attr_num; i++) {
            // Get attibutes
            lua_pushinteger(L, i+1);
            lua_gettable(L, -2);
            string name = lua_tostring(L, -1);
            if (currtable->attrsize.find(name) == currtable->attrsize.end()) {
                cout << "error: ";
                cout << "Attribute name " << name;
                cout << " does not exist in table " << table_name << endl;
                // Pop attribute name and list
                lua_pop(L, 2);
                return;
            }
            attr_list.push_back(lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        // Pop attribute list
        lua_pop(L, 1);
    }
    // Get value list
    lua_pushstring(L, "values");
    lua_rawget(L, -2);
    value_num = lua_objlen(L, -1);
    if (value_num != currtable->attrsize.size()) {
        cout << "error: ";
        cout << "Value list length differs from table schema" << endl;
        lua_pop(L, 1);
        return;
    }
    // Get value type list
    lua_pushstring(L, "valuetypes");
    lua_rawget(L, -3);
    vector<string> value_types;
    for(int i = 1; i <= value_num; i++) {
        // Get type
        lua_pushinteger(L, i);
        lua_gettable(L, -2);
        value_types.push_back(lua_tostring(L, -1));
        // Pop type
        lua_pop(L, 1);
    }
    // Pop value type list
    lua_pop(L, 1);
    // Get each value
    for(int i = 0; i < value_num; i++) {
        // Get value
        lua_pushinteger(L, i+1);
        lua_gettable(L, -2);
        if (currtable->attrsize[attr_list[i]] == -1) {
            // Ensure INT value
            if (!lua_isnumber(L, -1) || value_types[i] != "number") {
                cout << "error: Attribute ";
                cout << attr_list[i];
                cout << " is of INT type, cannot take value of `";
                cout << lua_tostring(L, -1) << "'" << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            }
            // Check bounds of INT value
            if (lua_tonumber(L, -1) > 2147483647) {
                cout << "error: Value ";
                cout << static_cast<int64_t>(lua_tonumber(L, -1));
                cout << " is too large for INT type" << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            } else if (lua_tonumber(L, -1) < -2147483648) {
                cout << "error: Value ";
                cout << static_cast<int64_t>(lua_tonumber(L, -1));
                cout << " is too small for INT type" << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            }
            int integer = lua_tointeger(L, -1);
            // Check for duplicate primary key
            if (attr_list[i] == currtable->primkey &&
                currtable->attrint_i[attr_list[i]].find(integer)!=
                currtable->attrint_i[attr_list[i]].end()){
                cout << "error: Primary key value " << integer;
                cout << " exists in table ";
                cout << table_name << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            }
            int_v.push_back(integer);
        } else {
            // Ensure VARCHAR value
            if (!lua_isstring(L, -1) || value_types[i] != "string") {
                cout << "error: Attribute ";
                cout << attr_list[i];
                cout << " is of VARCHAR type, cannot take value of ";
                cout << lua_tointeger(L, -1) << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            }
            string str = lua_tostring(L, -1);
            // Check VARCHAR size
            if (str.length() > currtable->attrsize[attr_list[i]]) {
                cout << "error: String `" << lua_tostring(L, -1);
                cout << "' too long for attribute " << attr_list[i] << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            }
            // Check for duplicate primary key
            if (attr_list[i] == currtable->primkey &&
                currtable->attrvar_i[attr_list[i]].find(str)!=
                currtable->attrvar_i[attr_list[i]].end()){
                cout << "error: Primary key value " << "`" << str << "'";
                cout << " exists in table ";
                cout << table_name << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            }
            str_v.push_back(str);
        }
        // Pop value
        lua_pop(L, 1);
    }
    // Pop value list
    lua_pop(L, 1);
    // Commit to table
    for(int i = 0; i < value_num; i++) {
        string attr_name = attr_list[i];
        if (currtable->attrsize[attr_name] == -1) {
            currtable->attrint[attr_name].push_back(int_v.front());
            currtable->attrint_i[attr_name].insert(
                pair<int,int>(int_v.front(), currtable->rownum));
            int_v.pop_front();
        } else {
            currtable->attrvar[attr_name].push_back(str_v.front());
            currtable->attrvar_i[attr_name].insert(
                pair<string,int>(str_v.front(), currtable->rownum));
            str_v.pop_front();
        }
    }
    currtable->rownum++;
    cout << "done." << endl;
}
void insert_into(lua_State* L)
{
    struct Table* currtable = nullptr;
    int value_num = 0;
    vector<string> attr_list;
    deque<string> str_v;
    deque<int> int_v;

    // Get table name to insert into
    lua_pushstring(L, "table_name");
    lua_rawget(L, -2);
    string table_name = string(lua_tostring(L, -1));
    if(tables.find(table_name) != tables.end()) {
        currtable = tables[string(lua_tostring(L, -1))];
        lua_pop(L, 1);
    } else {
        cout << "error: ";
        cout << "Table " << string(lua_tostring(L, -1));
        cout << " does not exist" << endl;
        lua_pop(L, 1);
        return;
    }
    // See if attribute list is provided
    lua_pushstring(L, "attr");
    lua_rawget(L, -2);
    if (lua_objlen(L, -1) == 0) {
        // No attribute list, assume same as schema
        attr_list = currtable->attrname;
        // Pop the empty attribute list
        lua_pop(L, 1);
    } else {
        // Get attribute list
        int attr_num = lua_objlen(L, -1);
        if (attr_num != currtable->attrsize.size()) {
            cout << "error: ";
            cout << "Attribute list length differs from table schema" << endl;
            lua_pop(L, 1);
            return;
        }
        for(int i = 0; i < attr_num; i++) {
            // Get attibutes
            lua_pushinteger(L, i+1);
            lua_gettable(L, -2);
            string name = lua_tostring(L, -1);
            if (currtable->attrsize.find(name) == currtable->attrsize.end()) {
                cout << "error: ";
                cout << "Attribute name " << name;
                cout << " does not exist in table " << table_name << endl;
                // Pop attribute name and list
                lua_pop(L, 2);
                return;
            }
            attr_list.push_back(lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        // Pop attribute list
        lua_pop(L, 1);
    }
    // Get value list
    lua_pushstring(L, "values");
    lua_rawget(L, -2);
    value_num = lua_objlen(L, -1);
    if (value_num != currtable->attrsize.size()) {
        cout << "error: ";
        cout << "Value list length differs from table schema" << endl;
        lua_pop(L, 1);
        return;
    }
    // Get value type list
    lua_pushstring(L, "valuetypes");
    lua_rawget(L, -3);
    vector<string> value_types;
    for(int i = 1; i <= value_num; i++) {
        // Get type
        lua_pushinteger(L, i);
        lua_gettable(L, -2);
        value_types.push_back(lua_tostring(L, -1));
        // Pop type
        lua_pop(L, 1);
    }
    // Pop value type list
    lua_pop(L, 1);
    // Get each value
    for(int i = 0; i < value_num; i++) {
        // Get value
        lua_pushinteger(L, i+1);
        lua_gettable(L, -2);
        if (currtable->attrsize[attr_list[i]] == -1) {
            // Ensure INT value
            if (!lua_isnumber(L, -1) || value_types[i] != "number") {
                cout << "error: Attribute ";
                cout << attr_list[i];
                cout << " is of INT type, cannot take value of `";
                cout << lua_tostring(L, -1) << "'" << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            }
            // Check bounds of INT value
            if (lua_tonumber(L, -1) > 2147483647) {
                cout << "error: Value ";
                cout << static_cast<int64_t>(lua_tonumber(L, -1));
                cout << " is too large for INT type" << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            } else if (lua_tonumber(L, -1) < -2147483648) {
                cout << "error: Value ";
                cout << static_cast<int64_t>(lua_tonumber(L, -1));
                cout << " is too small for INT type" << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            }
            int integer = lua_tointeger(L, -1);
            // Check for duplicate primary key
            if (attr_list[i] == currtable->primkey &&
                currtable->attrint_i[attr_list[i]].find(integer)!=
                currtable->attrint_i[attr_list[i]].end()){
                cout << "error: Primary key value " << integer;
                cout << " exists in table ";
                cout << table_name << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            }
            int_v.push_back(integer);
        } else {
            // Ensure VARCHAR value
            if (!lua_isstring(L, -1) || value_types[i] != "string") {
                cout << "error: Attribute ";
                cout << attr_list[i];
                cout << " is of VARCHAR type, cannot take value of ";
                cout << lua_tointeger(L, -1) << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            }
            string str = lua_tostring(L, -1);
            // Check VARCHAR size
            if (str.length() > currtable->attrsize[attr_list[i]]) {
                cout << "error: String `" << lua_tostring(L, -1);
                cout << "' too long for attribute " << attr_list[i] << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            }
            // Check for duplicate primary key
            if (attr_list[i] == currtable->primkey &&
                currtable->attrvar_i[attr_list[i]].find(str)!=
                currtable->attrvar_i[attr_list[i]].end()){
                cout << "error: Primary key value " << "`" << str << "'";
                cout << " exists in table ";
                cout << table_name << endl;
                // Pop value and value list
                lua_pop(L, 2);
                return;
            }
            str_v.push_back(str);
        }
        // Pop value
        lua_pop(L, 1);
    }
    // Pop value list
    lua_pop(L, 1);
    // Commit to table
    for(int i = 0; i < value_num; i++) {
        string attr_name = attr_list[i];
        if (currtable->attrsize[attr_name] == -1) {
            currtable->attrint[attr_name].push_back(int_v.front());
            currtable->attrint_i[attr_name].insert(
                pair<int,int>(int_v.front(), currtable->rownum));
            int_v.pop_front();
        } else {
            currtable->attrvar[attr_name].push_back(str_v.front());
            currtable->attrvar_i[attr_name].insert(
                pair<string,int>(str_v.front(), currtable->rownum));
            str_v.pop_front();
        }
    }
    currtable->rownum++;
    cout << "done." << endl;
}
/* Caution: The attrname should be in uppercase! */
struct Table* new_table(vector<string> attrname, vector<int> attrsize)
{
    /* Attribute must have both name and size */
    assert(attrname.size() == attrsize.size());

    struct Table *currtable = new Table();
    /* Attribute names */
    currtable->attrname = attrname;
    /* Attribute sizes */
    for(auto i = 0; i < currtable->attrname.size(); i++) {
        currtable->attrsize[currtable->attrname[i]] = attrsize[i];
    }
    /* Number of rows */
    currtable->rownum = 0;

    return currtable;
}
struct Table_h* new_table_h(vector<string> attrname, vector<int> attrsize)
{
    /* Attribute must have both name and size */
    assert(attrname.size() == attrsize.size());

    struct Table_h *currtable = new Table_h();
    /* Attribute names */
    currtable->attrname = attrname;
    /* Attribute sizes */
    for(auto i = 0; i < currtable->attrname.size(); i++) {
        currtable->attrsize[currtable->attrname[i]] = attrsize[i];
    }
    /* Number of rows */
    currtable->rownum = 0;

    return currtable;
}

/* Caution:
 *   Provide the values in schema order!
 *   Use push_back() on int_v and str_v in order!
 *   Check insert_into() for more information.
 */
void new_row(struct Table* table, deque<int> int_v, deque<string> str_v)
{
    struct Table* currtable = table;

    /* Must provide right number of values */
    assert(currtable->attrsize.size() == int_v.size() + str_v.size());

    /* Insert the row now */
    for(string attr_name: currtable->attrname) {
        if (currtable->attrsize[attr_name] == -1) {
            currtable->attrint[attr_name].push_back(int_v.front());
            currtable->attrint_i[attr_name].insert(
                pair<int,int>(int_v.front(), currtable->rownum));
            int_v.pop_front();
        } else {
            currtable->attrvar[attr_name].push_back(str_v.front());
            currtable->attrvar_i[attr_name].insert(
                pair<string,int>(str_v.front(), currtable->rownum));
            str_v.pop_front();
        }
    }
    /* Increment row number */
    currtable->rownum++;
}
void new_row_h(struct Table_h* table, deque<int> int_v, deque<string> str_v)
{
    struct Table_h* currtable = table;

    /* Must provide right number of values */
    assert(currtable->attrsize.size() == int_v.size() + str_v.size());

    /* Insert the row now */
    for(string attr_name: currtable->attrname) {
        if (currtable->attrsize[attr_name] == -1) {
            currtable->attrint[attr_name].push_back(int_v.front());
            currtable->attrint_i[attr_name].insert(
                pair<int,int>(int_v.front(), currtable->rownum));
            int_v.pop_front();
        } else {
            currtable->attrvar[attr_name].push_back(str_v.front());
            currtable->attrvar_i[attr_name].insert(
                pair<string,int>(str_v.front(), currtable->rownum));
            str_v.pop_front();
        }
    }
    /* Increment row number */
    currtable->rownum++;
}

void print_table(struct Table *table)
{
    // Print attributes
    for(auto str:table->attrname) {
        if (str == table->primkey)
            printf("%15s*(%2d)", str.c_str(), table->attrsize[str]);
        else
            printf("%16s(%2d)", str.c_str(), table->attrsize[str]);
    }
    cout << endl;
    // Print each row
    // Cache the types to make it efficient
    vector<int> attr_size;
    deque<vector<int>> attr_int;
    deque<vector<string>> attr_var;
    for(auto str:table->attrname) {
        attr_size.push_back(table->attrsize[str]);
        if (attr_size.back() == -1)
            attr_int.push_back(table->attrint[str]);
        else
            attr_var.push_back(table->attrvar[str]);
    }
    for(int i = 0; i < table->rownum; i++) {
        for(int j = 0; j < attr_size.size(); j++) {
            if (attr_size[j] == -1) {
                printf("%20d", attr_int.front()[i]);
                attr_int.push_back(attr_int.front());
                attr_int.pop_front();
            } else {
                printf("%20s", attr_var.front()[i].c_str());
                attr_var.push_back(attr_var.front());
                attr_var.pop_front();
            }
        }
        cout << endl;
    }
}
void print_table_h(struct Table_h *table)
{
    // Print attributes
    for(auto str:table->attrname) {
        if (str == table->primkey)
            printf("%15s*(%2d)", str.c_str(), table->attrsize[str]);
        else
            printf("%16s(%2d)", str.c_str(), table->attrsize[str]);
    }
    cout << endl;
    // Print each row
    // Cache the types to make it efficient
    vector<int> attr_size;
    deque<vector<int>> attr_int;
    deque<vector<string>> attr_var;
    for(auto str:table->attrname) {
        attr_size.push_back(table->attrsize[str]);
        if (attr_size.back() == -1)
            attr_int.push_back(table->attrint[str]);
        else
            attr_var.push_back(table->attrvar[str]);
    }
    for(int i = 0; i < table->rownum; i++) {
        for(int j = 0; j < attr_size.size(); j++) {
            if (attr_size[j] == -1) {
                printf("%20d", attr_int.front()[i]);
                attr_int.push_back(attr_int.front());
                attr_int.pop_front();
            } else {
                printf("%20s", attr_var.front()[i].c_str());
                attr_var.push_back(attr_var.front());
                attr_var.pop_front();
            }
        }
        cout << endl;
    }
}
void select (lua_State* L)
{	
	int command_num = lua_objlen(L,-1);
	
	vector<string> SEL_alias;   				// include '*'
	vector<string> SEL_target;					// all select target list
	vector<string> opt;							// function_opt = COUNT or SUM
	
	vector<string> From_table_name;
	vector<string> From_alias;
	
	/* The example for all variation: e.g WHERE authorId = 1 OR pages < 200; 
	   check_where_num=2; where_condition_first_alias = authorId; op_flag_first = "eq";compare_int_first = 1;
	   logical_op = OR;
	   where_condition_second_alias = pages; second = "lt"; compare_int_second = 200
	*/
	vector<int>	check_where_num;
	vector<string> where_condition_first_alias; // it can be alias name, which occurs in this condition(where_condition_first_attrname is not empty), or attribute name.
	vector<string> where_condition_first_attrname;
	vector<string> where_condition_second_alias;
	vector<string> where_condition_second_attrname;
	vector<string> logical_op;
	string op_flag_first;		
	string op_flag_second;
	vector<string> compare_alias_char_first;
	vector<string> compare_attr_char_first;
	vector<string> compare_alias_char_second;
	vector<string> compare_attr_char_second;
	int compare_int_first;
	int compare_int_second;
	// Convert the string to integer
	stringstream convert_stoi;
	/*{key,value} = {alias, table_name};*/
	map<string,string> alias_name_to_table;				
	
	// Extract the lua parser the command
	for (auto i=2;i<=command_num;i++){
		lua_rawgeti(L,-1,i);
		int check_elem = lua_objlen(L,-1);
		//distinguish from "SELECT"(i=2), "From"(i=3), and "Where" query(i=4)
		if (i==2) {													
			for (auto j=1;j<=check_elem;j++){
				lua_rawgeti(L,-1,j);			
				if (j>=2){
					int check_subelem = lua_objlen(L,-1);
					for (auto k=1;k<=check_subelem;k++) {
						if (k==1){
							lua_rawgeti(L,-1,k);
							SEL_alias.push_back(string(lua_tostring(L, -1)));
							lua_pop(L,1);
						} else{
							lua_rawgeti(L,-1,k);
							SEL_target.push_back(string(lua_tostring(L, -1)));
							lua_pop(L,1);
						}
					}
				} else {
					// opt = COUNT or SUM or attr or null
					opt.push_back(string(lua_tostring(L, -1))); 		
				}
				lua_pop(L,1);
			} 
		}
		else if (i==3) {			
			for (auto j=1;j<=check_elem;j++){
				lua_rawgeti(L,-1,j);
				int check_subelem = lua_objlen(L,-1);
				string tmp_table;
				string tmp_alias;
				for (auto k=1;k<=check_subelem;k++) {
					if (k==1){
						lua_rawgeti(L,-1,k);
						tmp_table = lua_tostring(L, -1);
						From_table_name.push_back(tmp_table);
						lua_pop(L,1);
					}
					tmp_alias = tmp_table;
					/* IF there is a true alias define, we erase the first element 
					*  and then we push_back the true alias define.*/
					if (k == 2){
						lua_rawgeti(L,-1,k);
						tmp_alias = lua_tostring(L, -1);
						lua_pop(L,1);
					}
				}
				alias_name_to_table.insert(pair<string,string>(tmp_alias,tmp_table));
				lua_pop(L,1);
			}
		}
		else {
			for (auto j=1;j<=check_elem;j++){
				lua_rawgeti(L,-1,j);
				int check_subelem = lua_objlen(L,-1);
				check_where_num.push_back(check_elem);
				if (j==2){
					logical_op.push_back(string(lua_tostring(L, -1)));  // logical_op = "AND" or "OR"
				} else {
					for (auto k=1;k<=check_subelem;k++) {
						lua_rawgeti(L,-1,k);
						int check_alias = lua_objlen(L,-1);
						for (auto it=1;it<=check_alias;it++) {
							lua_rawgeti(L,-1,it);
							if (j ==1 && k==1 && it ==1){
								where_condition_first_alias.push_back(lua_tostring(L, -1));
								lua_pop(L,1);
							}else if (j == 1 && k==1 && it ==2){
								where_condition_first_attrname.push_back(lua_tostring(L, -1));
								lua_pop(L,1);
							}else if (j == 1 && k==2 && it == 1){
								op_flag_first = lua_tostring(L, -1);
								lua_pop(L,1);
							}else if (j == 1 && k==3 && it ==1){
								compare_alias_char_first.push_back(lua_tostring(L, -1));
								lua_pop(L,1);
							}else if (j == 1 && k==3 && it ==2){
								compare_attr_char_first.push_back(lua_tostring(L, -1));;
								lua_pop(L,1);
							}else if (j ==3 && k==1 && it ==1){
								where_condition_second_alias.push_back((lua_tostring(L, -1)));
								lua_pop(L,1);
							}else if (j==3 && k ==1 && it ==2){
								where_condition_second_attrname.push_back((lua_tostring(L, -1)));
								lua_pop(L,1);
							}else if (j==3 && k ==2 && it ==1){
								op_flag_second = lua_tostring(L, -1);
								lua_pop(L,1);
							}else if (j==3 && k ==3 && it ==1){
								compare_alias_char_second.push_back(lua_tostring(L, -1));
								lua_pop(L,1);
							}else if (j==3 && k ==3 && it ==2){
								compare_attr_char_second.push_back(lua_tostring(L, -1));
								lua_pop(L,1);
							}
						}
						lua_pop(L,1);
					}
				}
				lua_pop(L,1);
			}
			
		}
		lua_pop(L,1);
	}
	// define the number of where conditions(var:check_where_number) 
	int wii = check_where_num.empty();
	int lii = logical_op.empty();
	if(wii ==1){
		check_where_num.push_back(0);
	}else if(wii !=1 && lii!=1){
		check_where_num[0] = check_where_num[0] -1;
	}
	
	
	
	struct Table* currtable = nullptr;
	struct Table* currtable_sec = nullptr;
	struct Table* currtable_tmp = nullptr;
	
	if(tables.find(From_table_name[0]) != tables.end()) {
        currtable = tables[From_table_name[0]];
    }
	
	vector<string> target_list;   			// name of target_list
	vector<string> target_list_combine;		// name with alias and target_list
	vector<int> target_size;	  			// size of target_list
    deque<string> str_v;
    deque<int> int_v;
	vector<int> tmp_i;									// in order to transform vector to deque
	vector<string> tmp_v;
	
	// There is only one table which is selected.
	if(From_table_name.size() == 1){ 
		if(check_where_num[0]==0){
			if (SEL_alias[0] == "*"){
				
				// get the table name (From_table_name[0]), finding the table's attrlist and attrsize
				target_list = currtable->attrname;
				int target_list_number = target_list.size();
				for (int in =0; in<target_list_number; in++){
					int value = currtable-> attrsize[target_list[in]];
					target_size.push_back(value);
				}
			}else{
				int target_number = SEL_alias.size();
				for (int i=0;i<target_number;i++){
					auto tmp = SEL_alias[i];
					target_list.push_back(tmp);
					int value = currtable-> attrsize[tmp];
					target_size.push_back(value);
				}
			}
			auto selecttable = new_table(target_list,target_size);
			int target_list_number = target_list.size();
			
			if(opt[0] == "COUNT"){
				cout<< "COUNT("<<SEL_alias[0]<<")"<<endl;
				cout<< currtable->rownum<<endl;
				return;
			}
			else if (opt[0] == "SUM"){
				int value = target_size[0];
				int sum = 0;
				if (value == -1){
					for(int i = 0; i < currtable->rownum; i++){
						tmp_i = currtable->attrint[target_list[0]];
						sum = sum + tmp_i[i];
					}
					cout << "SUM("<<SEL_alias[0] <<")"<<endl;
					cout << sum<<endl;
					return;
				}else{
					cout<<"ERROR:"<< "We cannot add this attribute."<<endl;
					return;
				}
				
			}
			else{
				for(int i = 0; i < currtable->rownum; i++) {
					for(int in =0; in< target_list_number; in++){
						int value = target_size[in];
						if (value == -1){
							tmp_i = currtable->attrint[target_list[in]];
							int_v.push_back(tmp_i[i]);
						}else{
							tmp_v = currtable->attrvar[target_list[in]];
							str_v.push_back(tmp_v[i]);
						}
					}
					new_row(selecttable, int_v, str_v);
					for(int in =0; in< target_list_number; in++){
						int value = target_size[in];
						if (value == -1){
							int_v.pop_front();
						}else{
							str_v.pop_front();
						}
					}
				}
				print_table(selecttable);
			}
		}
		else if (check_where_num[0]==1){
			// Create Selected target table
			if (SEL_alias[0] == "*"){
				// get the table name (From_table_name[0]), finding the table's attrlist and attrsize
				target_list = currtable->attrname;
				int target_list_number = target_list.size();
				for (int in =0; in<target_list_number; in++){
					int value = currtable-> attrsize[target_list[in]];
					target_size.push_back(value);
				}
			}else{
				int target_number = SEL_alias.size();
				for (int i=0;i<target_number;i++){
					auto tmp = SEL_alias[i];
					target_list.push_back(tmp);
					int value = currtable-> attrsize[tmp];
					target_size.push_back(value);
				}
			}
			auto selecttable = new_table(target_list,target_size);
			
			// Finding the index and Doing Where condition transform(char to int)
			int target_list_number = target_list.size();
			string attr_tmp = where_condition_first_alias[0];
			int value = currtable-> attrsize[attr_tmp];
			multimap<int,int>::iterator it,itlow,itup,eqit;
			multimap<string,int>::iterator it_var;
			set<int> index_i;
			set<int> :: iterator index_it;
			
			if (value== -1){
				// Converting the string to integer if the attribute is the integer
				convert_stoi << compare_alias_char_first[0];
				convert_stoi >> compare_int_first;

				if (op_flag_first == "eq"){
					for ( it = currtable-> attrint_i[attr_tmp].equal_range(compare_int_first).first; 
							  it !=currtable-> attrint_i[attr_tmp].equal_range(compare_int_first).second; ++it ){
								  index_i.insert((*it).second);
							  }
				}else if (op_flag_first == "gt"){
					itlow = currtable-> attrint_i[attr_tmp].lower_bound(compare_int_first+1);
					for (it = itlow; it!=currtable-> attrint_i[attr_tmp].end();++it){
							index_i.insert((*it).second);
						}
					
				}else if (op_flag_first == "lt"){
					itup = currtable-> attrint_i[attr_tmp].upper_bound(compare_int_first-1);
					for (it = currtable-> attrint_i[attr_tmp].begin();it!=itup; ++it){
							index_i.insert((*it).second);
						}
				}else if (op_flag_first == "ne"){
					for(int i=0;i<currtable->rownum;i++){
						index_i.insert(i);
					}
					for ( it = currtable-> attrint_i[attr_tmp].equal_range(compare_int_first).first;
						it !=currtable-> attrint_i[attr_tmp].equal_range(compare_int_first).second; ++it ){
								  index_i.erase((*it).second);
						}
				}
				
			}
			else{
				if (op_flag_first == "eq"){
					//string tmp;
					//tmp = compare_alias_char_first[0];
					for ( it_var = currtable-> attrvar_i[attr_tmp].equal_range(compare_alias_char_first[0]).first; 
							  it_var !=currtable-> attrvar_i[attr_tmp].equal_range(compare_alias_char_first[0]).second; ++it_var )
						{
						  index_i.insert((*it_var).second);
						}
				}
				else if (op_flag_first == "ne"){
					for(int i=0;i<currtable->rownum;i++){
						index_i.insert(i);
					}
					for ( it_var = currtable-> attrvar_i[attr_tmp].equal_range(compare_alias_char_first[0]).first; 
							  it_var !=currtable-> attrvar_i[attr_tmp].equal_range(compare_alias_char_first[0]).second; ++it_var ){
						
							index_i.erase((*it_var).second);
					}
				}
			}
			
			// Print or COUNT 
			if (opt[0] == "COUNT"){
				cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
				cout << index_i.size()<<endl;
				return;
			}
			else if (opt[0] == "SUM"){
				if (target_size[0] == -1){
					int sum = 0;
					for (index_it=index_i.begin(); index_it!= index_i.end(); ++index_it){
						tmp_i = currtable->attrint[target_list[0]];
						sum = sum + tmp_i[*index_it];
					}
					cout << "SUM ("<< target_list[0]<<")"<<endl;
					cout << sum << endl;
					return;
				}else{
					cout << "ERROR:"<<endl;
					cout << "We cannot add string type"<<endl;
					return;
				}
			}
			else {
				//Doing the insertion function
				int target_number = target_list.size();
				int index_number = index_i.size();
				for (index_it=index_i.begin(); index_it!= index_i.end(); ++index_it){
					for (int i=0;i<target_number;i++){
						auto tmp = target_list[i];
						int value = target_size[i];
						if (value== -1){
							tmp_i = currtable->attrint[tmp];
							int tmp_index = *index_it;
							int_v.push_back(tmp_i[tmp_index]);
						}else {
							tmp_v = currtable->attrvar[tmp];
							int tmp_index = *index_it;
							str_v.push_back(tmp_v[tmp_index]);
						}
					}
					new_row(selecttable, int_v, str_v);
					for (int i=0;i<target_number;i++){
						int value = target_size[i];
						if (value == -1){
							int_v.pop_front();
						}else{
							str_v.pop_front();
						}
					}
				}
				print_table(selecttable);
				// Clear the element in the convert_stoi
				convert_stoi.str("");
				convert_stoi.clear();
				return;
			}
		}
		else if(check_where_num[0]==2){
			// Create Selected target table
			if (SEL_alias[0] == "*"){
				// get the table name (From_table_name[0]), finding the table's attrlist and attrsize
				target_list = currtable->attrname;
				int target_list_number = target_list.size();
				for (int in =0; in<target_list_number; in++){
					int value = currtable-> attrsize[target_list[in]];
					target_size.push_back(value);
				}
			}
			else if(SEL_target.size() == 0){
				int target_number = SEL_alias.size();
				for (int i=0;i<target_number;i++){
					auto tmp = SEL_alias[i];
					target_list.push_back(tmp);
					int value = currtable-> attrsize[tmp];
					target_size.push_back(value);
				}
			}
			else if (SEL_target[0] == "*"){
				target_list = currtable->attrname;
				int target_number = currtable->attrname.size();
				for (int i=0;i<target_number;i++){
					string tmp_combine = SEL_alias[0] +"."+ target_list[i];
					target_list_combine.push_back(tmp_combine);
					int value = currtable-> attrsize[target_list[i]];
					target_size.push_back(value);
				}
			}
			else if (SEL_target.size()!= 0 && SEL_target[0]!= "*"){
				int target_number = SEL_target.size();
				for (int i=0;i<target_number;i++){
					string tmp_combine = SEL_alias[i] +"."+ SEL_target[i];
					target_list.push_back(SEL_target[i]);
					target_list_combine.push_back(tmp_combine);
					int value = currtable-> attrsize[SEL_target[i]];
					target_size.push_back(value);
				}
			}
			
			struct Table *selecttable = new Table();
			
			if (target_list_combine.size()!=0){
				selecttable = new_table(target_list_combine,target_size);
			}else{
				selecttable = new_table(target_list,target_size);
			}

			// Finding the indexing value and Doing the where condition's value transform
			int target_list_number = target_list.size();
			string attr_tmp_fir = where_condition_first_alias[0];
			string attr_tmp_sec = where_condition_second_alias[0];
			int value_fir = currtable-> attrsize[attr_tmp_fir];
			int value_sec = currtable-> attrsize[attr_tmp_sec];
			multimap<int,int>::iterator it,itlow,itup;
			multimap<string,int>::iterator it_var;
			// convert string number to int number 
			if (value_fir== -1){
				convert_stoi << compare_alias_char_first[0];
				convert_stoi >> compare_int_first;
				convert_stoi.str("");
				convert_stoi.clear();
			}
			
			if(value_sec == -1){
				convert_stoi << compare_alias_char_second[0];
				convert_stoi >> compare_int_second;
				convert_stoi.str("");
				convert_stoi.clear();
			}
			if (compare_int_first == 0 || compare_int_second ==0){
				cout << "error: different type "<<endl;
				return;
			}
			// Finding the index value and push back to index_i
			if ( logical_op[0] == "OR"){
				// The reason why I choose "set" is that it can prohibit the same value to be inserted.
				set<int> index_i;
				set<int> :: iterator index_it;
				// find the indexing value which is corresponding to the first statement.
				if (value_fir== -1){
					if (op_flag_first == "eq"){
						for ( it = currtable-> attrint_i[attr_tmp_fir].equal_range(compare_int_first).first; 
							  it !=currtable-> attrint_i[attr_tmp_fir].equal_range(compare_int_first).second; ++it ){
								  index_i.insert((*it).second);
							  }
					}else if (op_flag_first == "gt"){
						itlow = currtable-> attrint_i[attr_tmp_fir].lower_bound(compare_int_first+1);
						for (it = itlow; it!=currtable-> attrint_i[attr_tmp_fir].end();++it){
							index_i.insert((*it).second);
						}
					}else if(op_flag_first == "lt") {
						itup = currtable-> attrint_i[attr_tmp_fir].upper_bound(compare_int_first-1);
						for (it = currtable-> attrint_i[attr_tmp_fir].begin();it!=itup; ++it){
							index_i.insert((*it).second);
						}
					}else if (op_flag_first == "ne"){
						for(int i=0;i<currtable->rownum;i++){
							index_i.insert(i);
						}
						for ( it = currtable-> attrint_i[attr_tmp_fir].equal_range(compare_int_first).first;
							it !=currtable-> attrint_i[attr_tmp_fir].equal_range(compare_int_first).second; ++it ){
									  index_i.erase((*it).second);
							}
					}
				}else{
					if (op_flag_first == "eq"){
						for ( it_var = currtable-> attrvar_i[attr_tmp_fir].equal_range(compare_alias_char_first[0]).first; 
							  it_var !=currtable-> attrvar_i[attr_tmp_fir].equal_range(compare_alias_char_first[0]).second; ++it_var ){
								  index_i.insert((*it_var).second);
							  }
					}else if (op_flag_first == "ne"){
						for(int i=0;i<currtable->rownum;i++){
							index_i.insert(i);
						}
						for ( it_var = currtable-> attrvar_i[attr_tmp_fir].equal_range(compare_alias_char_first[0]).first;
							it_var !=currtable-> attrvar_i[attr_tmp_fir].equal_range(compare_alias_char_first[0]).second; ++it_var ){
									  index_i.erase((*it_var).second);
							}
						
					}
				}
				// insert the value which is corresponding to the second statement.
				if (value_sec == -1){
					if (op_flag_second == "eq"){
						for ( it = currtable-> attrint_i[attr_tmp_sec].equal_range(compare_int_second).first; 
							  it !=currtable-> attrint_i[attr_tmp_sec].equal_range(compare_int_second).second; ++it ){
								  index_i.insert((*it).second);
							  }
					}else if (op_flag_second == "gt"){
						
						itlow = currtable-> attrint_i[attr_tmp_sec].lower_bound(compare_int_second+1);
						for (it = itlow; it!=currtable-> attrint_i[attr_tmp_sec].end();++it){
							index_i.insert((*it).second);
						}
					}else if(op_flag_second == "lt") {
						itup = currtable-> attrint_i[attr_tmp_sec].upper_bound(compare_int_second-1);
						for (it = currtable-> attrint_i[attr_tmp_sec].begin();it!=itup; ++it){
							index_i.insert((*it).second);
						}
					}else if (op_flag_second == "ne"){
						for(int i=0;i<currtable->rownum;i++){
							index_i.insert(i);
						}
						for ( it = currtable-> attrint_i[attr_tmp_sec].equal_range(compare_int_second).first;
							it !=currtable-> attrint_i[attr_tmp_sec].equal_range(compare_int_second).second; ++it ){
									  index_i.erase((*it).second);
							}
					}
				}else {
					if (op_flag_second == "eq"){
						for ( it_var = currtable-> attrvar_i[attr_tmp_sec].equal_range(compare_alias_char_second[0]).first; 
							  it_var !=currtable-> attrvar_i[attr_tmp_sec].equal_range(compare_alias_char_second[0]).second; ++it_var ){
								  index_i.insert((*it_var).second);
							  }
					}else if (op_flag_second == "ne"){
						for(int i=0;i<currtable->rownum;i++){
							index_i.insert(i);
						}
						for ( it_var = currtable-> attrvar_i[attr_tmp_sec].equal_range(compare_alias_char_second[0]).first; 
							  it_var !=currtable-> attrvar_i[attr_tmp_sec].equal_range(compare_alias_char_second[0]).second; ++it_var ){
								  index_i.erase((*it_var).second);
							  }
					}
				}
				
				// COUNT OR SUM or Print
				if (opt[0] == "COUNT"){
					cout << "COUNT("<< SEL_alias[0]<<")"<<endl;
					cout << index_i.size()<<endl;
					return;
				}
				else if (opt[0] == "SUM"){
					if (target_size[0] == -1){
						int sum = 0;
						for (index_it=index_i.begin(); index_it!= index_i.end(); ++index_it){
							tmp_i = currtable->attrint[target_list[0]];
							sum = sum + tmp_i[*index_it];
						}
						cout << "SUM ("<< SEL_alias[0]<<")"<<endl;
						cout << sum << endl;
						return;
					}else{
						cout << "ERROR:"<<endl;
						cout << "We cannot add string type"<<endl;
						return;
					}
					
				}
				else {
					// Insert the element
					int target_number = target_size.size();
					int index_number = index_i.size();
					for (index_it=index_i.begin(); index_it!= index_i.end(); ++index_it){
						for (int i=0;i<target_number;i++){
							auto tmp = target_list[i];
							int value = target_size[i];
							if (value== -1){
								tmp_i = currtable->attrint[tmp];
								int tmp_index = *index_it;
								int_v.push_back(tmp_i[tmp_index]);
							}else {
								tmp_v = currtable->attrvar[tmp];
								int tmp_index = *index_it;
								str_v.push_back(tmp_v[tmp_index]);
							}
						}
						new_row(selecttable, int_v, str_v);
						for (int i=0;i<target_number;i++){
							int value = target_size[i];
							if (value == -1){
								int_v.pop_front();
							}else{
								str_v.pop_front();
							}
						}
					}
				}
				
			}
			else if ( logical_op[0] == "AND"){
				set<int> index_i_tmp;
				set<int> index_i;
				set<int> :: iterator index_it;
				set<int> :: iterator it_test;
				// find the indexing value which is corresponding to the first statement.
				
				if (value_fir== -1){
					if (op_flag_first == "eq"){
						for ( it = currtable-> attrint_i[attr_tmp_fir].equal_range(compare_int_first).first; 
							  it !=currtable-> attrint_i[attr_tmp_fir].equal_range(compare_int_first).second; ++it ){
								  index_i_tmp.insert((*it).second);
							  }
					}else if (op_flag_first == "gt"){
						itlow = currtable-> attrint_i[attr_tmp_fir].lower_bound(compare_int_first+1);
						for (it = itlow; it!=currtable-> attrint_i[attr_tmp_fir].end();++it){
							index_i_tmp.insert((*it).second);
						}
					}else if (op_flag_first == "lt") {
						itup = currtable-> attrint_i[attr_tmp_fir].upper_bound(compare_int_first-1);
						for (it = currtable-> attrint_i[attr_tmp_fir].begin();it!=itup; ++it){
							index_i_tmp.insert((*it).second);
						}
					}else if (op_flag_first == "ne"){
						for(int i=0;i<currtable->rownum;i++){
							index_i_tmp.insert(i);
						}
						for ( it = currtable-> attrint_i[attr_tmp_fir].equal_range(compare_int_first).first;
							it !=currtable-> attrint_i[attr_tmp_fir].equal_range(compare_int_first).second; ++it ){
									  index_i_tmp.erase((*it).second);
							}
					}
				}else{
					if (op_flag_first == "eq"){
						for ( it_var = currtable-> attrvar_i[attr_tmp_fir].equal_range(compare_alias_char_first[0]).first; 
							  it_var !=currtable-> attrvar_i[attr_tmp_fir].equal_range(compare_alias_char_first[0]).second; ++it_var )
						{
							index_i_tmp.insert((*it_var).second);
						}
					}else if (op_flag_first == "ne"){
						for(int i=0;i<currtable->rownum;i++){
							index_i_tmp.insert(i);
						}
						
						for ( it_var = currtable-> attrvar_i[attr_tmp_fir].equal_range(compare_alias_char_first[0]).first; 
							  it_var !=currtable-> attrvar_i[attr_tmp_fir].equal_range(compare_alias_char_first[0]).second; ++it_var )
						{
							index_i_tmp.erase((*it_var).second);
						}
					}
				}
				//If we can find the indexing value which is corresponding to the second statement and is exist in index_i_tmp, 
				//we insert the value to insert_i.

				if (value_sec == -1){
					if (op_flag_second == "eq"){
						for ( it = currtable-> attrint_i[attr_tmp_sec].equal_range(compare_int_second).first; 
							  it !=currtable-> attrint_i[attr_tmp_sec].equal_range(compare_int_second).second; ++it ){
								  if (index_i_tmp.find((*it).second)!= index_i_tmp.end()){
									  index_i.insert((*it).second);
								  }
							  }
					}else if (op_flag_second == "gt"){
						itlow = currtable-> attrint_i[attr_tmp_sec].lower_bound(compare_int_second+1);
						for (it = itlow; it!=currtable-> attrint_i[attr_tmp_sec].end();++it){
							if (index_i_tmp.find((*it).second)!= index_i_tmp.end()){
								index_i.insert((*it).second);
							}
						}
					}else if (op_flag_second == "lt") {
						itup = currtable-> attrint_i[attr_tmp_sec].upper_bound(compare_int_second-1);
						for (it = currtable-> attrint_i[attr_tmp_sec].begin();it!=itup; ++it){
							if (index_i_tmp.find((*it).second)!= index_i_tmp.end()){
								index_i.insert((*it).second);
							}
						}
					}else if (op_flag_second == "ne"){
						for(int i=0;i<currtable->rownum;i++){
							index_i.insert(i);
						}
						
						for ( it = currtable-> attrint_i[attr_tmp_sec].equal_range(compare_int_second).first; 
							  it !=currtable-> attrint_i[attr_tmp_sec].equal_range(compare_int_second).second; ++it ){
								  if (index_i_tmp.find((*it).second)!= index_i_tmp.end()){
									  index_i.erase((*it).second);
									}
						}
					}
				}else{
					if (op_flag_second == "eq"){
						for ( it_var = currtable-> attrvar_i[attr_tmp_sec].equal_range(compare_alias_char_second[0]).first; 
							  it_var !=currtable-> attrvar_i[attr_tmp_sec].equal_range(compare_alias_char_second[0]).second; ++it_var )
						{
							if (index_i_tmp.find((*it_var).second) != index_i_tmp.end()){
								index_i.insert((*it_var).second);
							}
						}
					}if (op_flag_second == "ne"){
						for(int i=0;i<currtable->rownum;i++){
							index_i.insert(i);
						}
						for ( it_var = currtable-> attrvar_i[attr_tmp_sec].equal_range(compare_alias_char_second[0]).first; 
							  it_var !=currtable-> attrvar_i[attr_tmp_sec].equal_range(compare_alias_char_second[0]).second; ++it_var )	{
							if (index_i_tmp.find((*it_var).second) != index_i_tmp.end()){
								index_i.erase((*it_var).second);
							}
						}
					}
				}
				
				if (opt[0] == "COUNT"){
					cout << "COUNT("<< SEL_alias[0]<<")"<<endl;
					cout << index_i.size()<<endl;
					return;
				}
				else if (opt[0] == "SUM"){
					if (target_size[0] == -1){
						int sum = 0;
						for (index_it=index_i.begin(); index_it!= index_i.end(); ++index_it){
							tmp_i = currtable->attrint[target_list[0]];
							sum = sum + tmp_i[*index_it];
						}
						cout << "SUM ("<< SEL_alias[0]<<")"<<endl;
						cout << sum << endl;
						return;
					}else{
						cout << "ERROR:"<<endl;
						cout << "We cannot add string type"<<endl;
						return;
					}
					
				}
				else {
					// Print the table
					int target_number = target_size.size();
					int index_number = index_i.size();
					for (index_it=index_i.begin(); index_it!= index_i.end(); ++index_it){
						for (int i=0;i<target_number;i++){
							auto tmp = target_list[i];
							int value = target_size[i];
							if (value== -1){
								tmp_i = currtable->attrint[tmp];
								int tmp_index = *index_it;
								int_v.push_back(tmp_i[tmp_index]);
							}else {
								tmp_v = currtable->attrvar[tmp];
								int tmp_index = *index_it;
								str_v.push_back(tmp_v[tmp_index]);
							}
						}
						new_row(selecttable, int_v, str_v);
						for (int i=0;i<target_number;i++){
							int value = target_size[i];
							if (value == -1){
								int_v.pop_front();
							}else{
								str_v.pop_front();
							}
						}
					}
				}
			}
			
			print_table(selecttable);
			// Clear the element in the convert_stoi
			convert_stoi.str("");
			convert_stoi.clear();
			
		}
	}
	/*	There is two table which is selected and It is going to do inner join.
	*	First, we find the select attribute.
	*	Second, based on the inner join condition, we find the attribute which is corresponding to the condition and insert.
	*	Third, we print the table.
	*/
	else if(From_table_name.size() == 2){
		vector<int> inner_join_tmp_i;
		vector<string> inner_join_tmp_v;
		vector<int> compare_inner_join_i;
		vector<string> compare_inner_join_v;
		
		string tmp_alias = where_condition_first_alias[0];
		string which_table_wf = alias_name_to_table.find(tmp_alias)->second;
		currtable = tables[which_table_wf];
		string tmp_attr_fir = where_condition_first_attrname[0];
		int value_fir = currtable-> attrsize[tmp_attr_fir];
		string tmp_alias_sec = compare_alias_char_first[0];
		string which_table_ws = alias_name_to_table.find(tmp_alias_sec)->second;
		currtable_sec = tables[which_table_ws];
		string tmp_attr_sec = compare_attr_char_first[0];
		int value_sec = currtable_sec-> attrsize[tmp_attr_sec];
		
		
		
		set<string> attrname_tmp_fir(currtable->attrname.begin(),currtable->attrname.end());
		set<string> attrname_tmp_sec(currtable_sec->attrname.begin(),currtable_sec->attrname.end());
		// Searching the selecting attribute
		if(SEL_target.size() == 0){
			if (SEL_alias[0]=="*"){
				target_list = currtable->attrname;
				int target_number = currtable->attrname.size();
				for (int i=0;i<target_number;i++){
					int value = currtable-> attrsize[target_list[i]];
					target_size.push_back(value);
				}
			}
			else{
				int target_number = SEL_alias.size();
				// If the user doesn't define the alias, then I scan both of two tables and get the attr_name and attr_value
				for (int i=0;i<target_number;i++){
					auto tmp = SEL_alias[i];
					if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end() && attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
						cout << "the attribute: "<< tmp <<" “authorId” is ambiguous, as it appears in both table";
						return;
					}
					
					if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
						target_list.push_back(tmp);
						int value = currtable-> attrsize[tmp];
						target_size.push_back(value);
					}else if (attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
						target_list.push_back(tmp);
						int value = currtable_sec-> attrsize[tmp];
						target_size.push_back(value);
					}else{
						cout << "error: ";
						cout <<" The Selected element" << tmp <<" doesn't exist in the table"<<endl;
						return;
					}
				}
			}
			
		}
		//Means that users defines the alias
		else{
			if (SEL_target[0]=="*")
			{
				string which_table;
				which_table = alias_name_to_table.find(SEL_alias[0])->second;
				currtable_tmp = tables[which_table];
				target_list = currtable_tmp->attrname;
				int target_number = currtable_tmp->attrname.size();
				for (int i=0;i<target_number;i++){
					string tmp_combine = SEL_alias[0] +"."+ target_list[i];
					target_list_combine.push_back(tmp_combine);
					int value = currtable_tmp-> attrsize[target_list[i]];
					target_size.push_back(value);
				}
			}
			else{
				int SEL_alias_size = SEL_alias.size();
				for (int i=0;i<SEL_alias_size;i++){
					string tmp_alias = SEL_alias[i];
					if (alias_name_to_table.find(tmp_alias)!=alias_name_to_table.end()){
						string which_table;
						which_table = alias_name_to_table.find(tmp_alias)->second;
						currtable_tmp = tables[which_table];
						target_list.push_back(SEL_target[i]);
						string tmp_combine = SEL_alias[i] +"."+ SEL_target[i];
						target_list_combine.push_back(tmp_combine);
						int value = currtable_tmp-> attrsize[SEL_target[i]];
						target_size.push_back(value);
					}
					else if (alias_name_to_table.find(tmp_alias)==alias_name_to_table.end()){
						target_list_combine.push_back(tmp_alias);
						if (attrname_tmp_fir.find(tmp_alias)!=attrname_tmp_fir.end()){
							target_list.push_back(tmp_alias);
							int value = currtable-> attrsize[tmp_alias];
							target_size.push_back(value);
						}
						else if (attrname_tmp_sec.find(tmp_alias)!=attrname_tmp_sec.end()){
							target_list.push_back(tmp_alias);
							int value = currtable_sec-> attrsize[tmp_alias];
							target_size.push_back(value);
						}
					}
				}
			}
		}
		
		struct Table *selecttable = new Table();
		
		if (target_list_combine.size()!=0){
			selecttable = new_table(target_list_combine,target_size);
		}else{
			selecttable = new_table(target_list,target_size);
		}
		
		if (logical_op.size()==0){
			if (value_fir ==-1 && value_sec== -1){
				multimap<int,int>::iterator it;
				multimap<string,int>::iterator it_var;
				vector<int> index;
				vector<int>::iterator index_it;
				// In order to know which table mapping to another table, we compare each inner_join attribute.
				set<int> compare_inner_join_i_fir(currtable->attrint[tmp_attr_fir].begin(),currtable->attrint[tmp_attr_fir].end());
				set<int> compare_inner_join_i_sec(currtable_sec->attrint[tmp_attr_sec].begin(),currtable_sec->attrint[tmp_attr_sec].end());

				if (compare_inner_join_i_fir.size()!=currtable->attrint[tmp_attr_fir].size()){
					for(index_it =currtable->attrint[tmp_attr_fir].begin();index_it!= currtable->attrint[tmp_attr_fir].end();++index_it){
						it = currtable_sec-> attrint_i[tmp_attr_sec].find(*index_it);
						index.push_back((*it).second);
					}
					
					if (opt[0] == "COUNT"){
						cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
						cout << index.size()<<endl;
						return;
					}
					else if (opt[0] == "SUM"){
						if (target_size[0] == -1){
							int sum = 0;
							for (index_it=index.begin(); index_it!= index.end(); ++index_it){
								tmp_i = currtable->attrint[target_list[0]];
								sum = sum + tmp_i[*index_it];
							}
							cout << "SUM ("<< target_list[0]<<")"<<endl;
							cout << sum << endl;
							return;
						}else{
							cout << "ERROR:"<<endl;
							cout << "We cannot add string type"<<endl;
							return;
						}
					}
					else {
						for(int i =0;i<currtable->rownum;i++){
							for(int j =0;j<target_list.size();j++){
								string tmp = target_list[j];
								int value = target_size[j];
								if (value == -1){
									if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
										tmp_i = currtable->attrint[tmp];
										int_v.push_back(tmp_i[i]);
									}else{
										int tmp_index = index[i];
										tmp_i = currtable_sec->attrint[tmp];
										int_v.push_back(tmp_i[tmp_index]);
									}
									
								}else {
									if(attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
										tmp_v = currtable->attrvar[tmp];
										str_v.push_back(tmp_v[i]);
									}else{
										int tmp_index = index[i];
										tmp_v = currtable_sec->attrvar[tmp];
										str_v.push_back(tmp_v[tmp_index]);
									}
								}
							}
							new_row(selecttable, int_v, str_v);
							for (int j=0;j<target_list.size();j++){
								int value = target_size[j];
								if (value == -1){
									int_v.pop_front();
								}else{
									str_v.pop_front();
								}
							}
						}
						print_table(selecttable);
						return;
					}
					
					
				}else if (compare_inner_join_i_sec.size()!=currtable_sec->attrint[tmp_attr_sec].size()){
					for(index_it =currtable_sec->attrint[tmp_attr_sec].begin();index_it!= currtable_sec->attrint[tmp_attr_sec].end();++index_it){
						it = currtable-> attrint_i[tmp_attr_fir].find(*index_it);
						index.push_back((*it).second);
					}
					if (opt[0] == "COUNT"){
						cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
						cout << index.size()<<endl;
						return;
					}
					else if (opt[0] == "SUM"){
						if (target_size[0] == -1){
							int sum = 0;
							for (index_it=index.begin(); index_it!= index.end(); ++index_it){
								tmp_i = currtable->attrint[target_list[0]];
								sum = sum + tmp_i[*index_it];
							}
							cout << "SUM ("<< target_list[0]<<")"<<endl;
							cout << sum << endl;
							return;
						}else{
							cout << "ERROR:"<<endl;
							cout << "We cannot add string type"<<endl;
							return;
						}
					}
					else{
						for(int i =0;i<currtable_sec->rownum;i++){
							for(int j =0;j<target_list.size();j++){
								string tmp = target_list[j];
								int value = target_size[j];
								if (value == -1){
									if (attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
										tmp_i = currtable_sec->attrint[tmp];
										int_v.push_back(tmp_i[i]);
									}else{
										int tmp_index = index[i];
										tmp_i = currtable->attrint[tmp];
										int_v.push_back(tmp_i[tmp_index]);
									}
									
								}else {
									if(attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
										tmp_v = currtable_sec->attrvar[tmp];
										str_v.push_back(tmp_v[i]);
									}else{
										int tmp_index = index[i];
										tmp_v = currtable->attrvar[tmp];
										str_v.push_back(tmp_v[tmp_index]);
									}
								}
							}
							new_row(selecttable, int_v, str_v);
							for (int j=0;j<target_list.size();j++){
								int value = target_size[j];
								if (value == -1){
									int_v.pop_front();
								}else{
									str_v.pop_front();
								}
							}
						}
						print_table(selecttable);
						return;
					}
				}
			}
			else if (value_fir == 30 && value_sec == 30){
				multimap<string,int>::iterator it_var;
				vector<int> index;
				vector<int> ::iterator in;
				vector<string>::iterator index_it;
				// In order to know which table mapping to another table, we compare each inner_join attribute.
				set<string> compare_inner_join_v_fir(currtable->attrvar[tmp_attr_fir].begin(),currtable->attrvar[tmp_attr_fir].end());
				set<string> compare_inner_join_v_sec(currtable_sec->attrvar[tmp_attr_sec].begin(),currtable_sec->attrvar[tmp_attr_sec].end());
				
				if (compare_inner_join_v_fir.size()!=currtable->attrvar[tmp_attr_fir].size()){
					for(index_it =currtable->attrvar[tmp_attr_fir].begin();index_it!= currtable->attrvar[tmp_attr_fir].end();++index_it){
						it_var = currtable_sec-> attrvar_i[tmp_attr_sec].find(*index_it);
						index.push_back((*it_var).second);
					}
					if (opt[0] == "COUNT"){
						cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
						cout << index.size()<<endl;
						return;
					}
					else if (opt[0] == "SUM"){
						if (target_size[0] == -1){
							int sum = 0;
							for (in=index.begin(); in!= index.end(); ++in){
								tmp_i = currtable->attrint[target_list[0]];
								sum = sum + tmp_i[*in];
							}
							cout << "SUM ("<< target_list[0]<<")"<<endl;
							cout << sum << endl;
							return;
						}else{
							cout << "ERROR:"<<endl;
							cout << "We cannot add string type"<<endl;
							return;
						}
					}
					else{
						for(int i =0;i<currtable->rownum;i++){
							for(int j =0;j<target_list.size();j++){
								string tmp = target_list[j];
								int value = target_size[j];
								if (value == -1){
									if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
										tmp_i = currtable->attrint[tmp];
										int_v.push_back(tmp_i[i]);
									}else{
										int tmp_index = index[i];
										tmp_i = currtable_sec->attrint[tmp];
										int_v.push_back(tmp_i[tmp_index]);
									}
									
								}else {
									if(attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
										tmp_v = currtable->attrvar[tmp];
										str_v.push_back(tmp_v[i]);
									}else{
										int tmp_index = index[i];
										tmp_v = currtable_sec->attrvar[tmp];
										str_v.push_back(tmp_v[tmp_index]);
									}
								}
							}
							new_row(selecttable, int_v, str_v);
							for (int j=0;j<target_list.size();j++){
								int value = target_size[j];
								if (value == -1){
									int_v.pop_front();
								}else{
									str_v.pop_front();
								}
							}
						}
						print_table(selecttable);
						return;
					}
					
					
				}
				else if (compare_inner_join_v_sec.size()!=currtable_sec->attrvar[tmp_attr_sec].size()){
					for(index_it =currtable_sec->attrvar[tmp_attr_sec].begin();index_it!= currtable_sec->attrvar[tmp_attr_sec].end();++index_it){
						it_var = currtable-> attrvar_i[tmp_attr_fir].find(*index_it);
						index.push_back((*it_var).second);
					}
					
					if (opt[0] == "COUNT"){
						cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
						cout << index.size()<<endl;
						return;
					}
					else if (opt[0] == "SUM"){
						if (target_size[0] == -1){
							int sum = 0;
							for (in=index.begin(); in!= index.end(); ++in){
								tmp_i = currtable->attrint[target_list[0]];
								sum = sum + tmp_i[*in];
							}
							cout << "SUM ("<< target_list[0]<<")"<<endl;
							cout << sum << endl;
							return;
						}else{
							cout << "ERROR:"<<endl;
							cout << "We cannot add string type"<<endl;
							return;
						}
					}
					else{
						for(int i =0;i<currtable_sec->rownum;i++){
							for(int j =0;j<target_list.size();j++){
								string tmp = target_list[j];
								int value = target_size[j];
								if (value == -1){
									if (attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
										tmp_i = currtable_sec->attrint[tmp];
										int_v.push_back(tmp_i[i]);
									}else{
										int tmp_index = index[i];
										tmp_i = currtable->attrint[tmp];
										int_v.push_back(tmp_i[tmp_index]);
									}
									
								}else {
									if(attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
										tmp_v = currtable_sec->attrvar[tmp];
										str_v.push_back(tmp_v[i]);
									}else{
										int tmp_index = index[i];
										tmp_v = currtable->attrvar[tmp];
										str_v.push_back(tmp_v[tmp_index]);
									}
								}
							}
							new_row(selecttable, int_v, str_v);
							for (int j=0;j<target_list.size();j++){
								int value = target_size[j];
								if (value == -1){
									int_v.pop_front();
								}else{
									str_v.pop_front();
								}
							}
						}
						print_table(selecttable);
						return;
					}
				}
			}
			else{
				cout << "error: different types and cannot be compared."<<endl;
				return;
			}
			
			
		}
		/*	The flowchart for dealing with the case "AND" condition.
		*	First, we find the index which is fulfilled with the second where condition.
		*	Then, we pointed to the value of the inner joined attribute.
		*	Second, searching the other table with the same value and stored the index.
		*	Fourthe we printed it.
		*/
		else if(logical_op[0] == "AND"){
			
			set<int> compare_inner_join_i_fir(currtable->attrint[tmp_attr_fir].begin(),currtable->attrint[tmp_attr_fir].end());
			set<int> compare_inner_join_i_sec(currtable_sec->attrint[tmp_attr_sec].begin(),currtable_sec->attrint[tmp_attr_sec].end());
			
			set<string> compare_inner_join_v_fir(currtable->attrvar[tmp_attr_fir].begin(),currtable->attrvar[tmp_attr_fir].end());
			set<string> compare_inner_join_v_sec(currtable_sec->attrvar[tmp_attr_sec].begin(),currtable_sec->attrvar[tmp_attr_sec].end());
			
			set<int> index_i;
			set<int> :: iterator index_it;
			
			multimap<int,int>::iterator it,itlow,itup;
			multimap<string,int>::iterator it_var;
			
			int compare_int;
			// find the indexing value which is corresponding to the second where statement.
			string attr_tmp = where_condition_second_attrname[0];
			string which_table_tmp = alias_name_to_table.find(where_condition_second_alias[0])->second;
			if (which_table_tmp.compare(which_table_wf) == 0 ){
				int value = currtable->attrsize[attr_tmp];
				if (value== -1){
					convert_stoi << compare_alias_char_second[0];
					convert_stoi >> compare_int;
					convert_stoi.str("");
					convert_stoi.clear();
				} 
				
				
				if (value== -1){
					if (op_flag_second == "eq"){
						for ( it = currtable-> attrint_i[attr_tmp].equal_range(compare_int).first; 
							  it !=currtable-> attrint_i[attr_tmp].equal_range(compare_int).second; ++it ){
								  index_i.insert((*it).second);
							  }
					}else if (op_flag_second == "gt"){
						itlow = currtable-> attrint_i[attr_tmp].lower_bound(compare_int+1);
						for (it = itlow; it!=currtable-> attrint_i[attr_tmp].end();++it){
							index_i.insert((*it).second);
						}
					}else if (op_flag_second == "lt") {
						itup = currtable-> attrint_i[attr_tmp].upper_bound(compare_int-1);
						for (it = currtable-> attrint_i[attr_tmp].begin();it!=itup; ++it){
							index_i.insert((*it).second);
						}
					}else if (op_flag_second == "ne"){
						for(int i=0;i<currtable->rownum;i++){
							index_i.insert(i);
						}
						for ( it = currtable-> attrint_i[attr_tmp].equal_range(compare_int).first; 
							  it !=currtable-> attrint_i[attr_tmp].equal_range(compare_int).second; ++it ){
								  index_i.erase((*it).second);
						}
					}
				}else{
					if (op_flag_second == "eq"){
						for ( it_var = currtable-> attrvar_i[attr_tmp].equal_range(compare_alias_char_second[0]).first; 
							  it_var !=currtable-> attrvar_i[attr_tmp].equal_range(compare_alias_char_second[0]).second; ++it_var ){
								  index_i.insert((*it_var).second);
						}
					}else if (op_flag_second == "ne"){
						for(int i=0;i<currtable->rownum;i++){
							index_i.insert(i);
						}
						
						for ( it_var = currtable-> attrvar_i[attr_tmp].equal_range(compare_alias_char_second[0]).first; 
							  it_var !=currtable-> attrvar_i[attr_tmp].equal_range(compare_alias_char_second[0]).second; ++it_var ){
								  index_i.erase((*it_var).second);
						}
					}
				}
			}
			else if (which_table_tmp.compare(which_table_ws) == 0){
				int value = currtable_sec->attrsize[attr_tmp];
				if (value== -1){
					convert_stoi << compare_alias_char_second[0];
					convert_stoi >> compare_int;
					convert_stoi.str("");
					convert_stoi.clear();
				}
				// find the indexing value which is corresponding to the first statement.
				if (value== -1){
					if (op_flag_second == "eq"){
						for ( it = currtable_sec-> attrint_i[attr_tmp].equal_range(compare_int).first; 
							  it !=currtable_sec-> attrint_i[attr_tmp].equal_range(compare_int).second; ++it ){
								  index_i.insert((*it).second);
							  }
					}else if (op_flag_second == "gt"){
						itlow = currtable_sec-> attrint_i[attr_tmp].lower_bound(compare_int+1);
						for (it = itlow; it!=currtable_sec-> attrint_i[attr_tmp].end();++it){
							index_i.insert((*it).second);
						}
					}else if (op_flag_second == "lt"){
						itup = currtable_sec-> attrint_i[attr_tmp].upper_bound(compare_int-1);
						for (it = currtable_sec-> attrint_i[attr_tmp].begin();it!=itup; ++it){
							index_i.insert((*it).second);
						}
					}else if (op_flag_second == "ne"){
						for(int i=0;i<currtable_sec->rownum;i++){
							index_i.insert(i);
						}
						for ( it = currtable_sec-> attrint_i[attr_tmp].equal_range(compare_int).first; 
							  it !=currtable_sec-> attrint_i[attr_tmp].equal_range(compare_int).second; ++it ){
								  index_i.erase((*it).second);
							  }
					}
				}else{
					if (op_flag_second == "eq"){
						for ( it_var = currtable_sec-> attrvar_i[attr_tmp].equal_range(compare_alias_char_second[0]).first; 
							  it_var !=currtable_sec-> attrvar_i[attr_tmp].equal_range(compare_alias_char_second[0]).second; ++it_var ){
								  index_i.insert((*it_var).second);
						}
					}else if (op_flag_second == "ne"){
						for(int i=0;i<currtable_sec->rownum;i++){
							index_i.insert(i);
						}
						
						for ( it_var = currtable_sec-> attrvar_i[attr_tmp].equal_range(compare_alias_char_second[0]).first; 
							  it_var !=currtable_sec-> attrvar_i[attr_tmp].equal_range(compare_alias_char_second[0]).second; ++it_var ){
								  index_i.erase((*it_var).second);
						}
					}
				}
			}
				
			// we pointed the index to the value of the inner joined attribute.	
			if (value_fir == -1 && value_sec== -1){
				vector<int>	inner_join_tmp_i;
				vector<int>	compare_int_inner_join;
				vector<int>	::iterator	index_inner;
				vector<int>	index_combine;
				
				int tmp_index;
				int val;
				
				// It represents table_first mapping to table_sec
				if (compare_inner_join_i_fir.size()!=currtable->attrint[tmp_attr_fir].size()){
					if (which_table_tmp.compare(which_table_wf) == 0){
						inner_join_tmp_i = currtable->attrint[tmp_attr_fir];
						
						if (opt[0] == "COUNT"){
							cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
							cout << index_i.size()<<endl;
							return;
						}
						else if (opt[0] == "SUM"){
							if (target_size[0] == -1){
								int sum = 0;
								for (index_it=index_i.begin(); index_it!= index_i.end(); ++index_it){
									tmp_i = currtable->attrint[target_list[0]];
									sum = sum + tmp_i[*index_it];
								}
								cout << "SUM ("<< target_list[0]<<")"<<endl;
								cout << sum << endl;
								return;
							}else{
								cout << "ERROR:"<<endl;
								cout << "We cannot add string type"<<endl;
								return;
							}
						}
						else{
							// Print table
							for (index_it = index_i.begin();index_it != index_i.end(); ++index_it){
								for (int i=0;i<target_list.size();i++){
									string tmp = target_list[i];
									int value = target_size[i];
									if (value == -1){
										if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
											tmp_i = currtable->attrint[tmp];
											tmp_index = *index_it;
											int_v.push_back(tmp_i[tmp_index]);
										}else{
											tmp_index = inner_join_tmp_i[*index_it];
											it = currtable_sec-> attrint_i[tmp_attr_sec].find(tmp_index);
											tmp_i = currtable_sec->attrint[tmp];
											tmp_index = (*it).second;
											int_v.push_back(tmp_i[tmp_index]);
										}
									}else{
										if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
											tmp_v = currtable->attrvar[tmp];
											tmp_index = *index_it;
											str_v.push_back(tmp_v[tmp_index]);
										}else{
											tmp_index = inner_join_tmp_i[*index_it];
											it = currtable_sec-> attrint_i[tmp_attr_sec].find(tmp_index);
											tmp_v = currtable_sec->attrvar[tmp];
											tmp_index = (*it).second;
											str_v.push_back(tmp_v[tmp_index]);
										}
									}
								}
								new_row(selecttable, int_v, str_v);
								for (int i=0;i<target_list.size();i++){
									int value = target_size[i];
									if (value == -1){
										int_v.pop_front();
									}else{
										str_v.pop_front();
									}
								}
							}
							print_table(selecttable);
							return;
						}
					}
					else if (which_table_tmp.compare(which_table_ws) == 0){
						inner_join_tmp_i = currtable_sec->attrint[tmp_attr_sec];
						// 
						for (index_it = index_i.begin();index_it != index_i.end(); ++index_it){
							int tmp_i = *index_it;
							compare_int_inner_join.push_back(inner_join_tmp_i[tmp_i]);
						}
						
						for (int i=0;i<compare_int_inner_join.size();i++ ){
							val = compare_int_inner_join[i];
							for ( it = currtable-> attrint_i[tmp_attr_fir].equal_range(val).first; 
								it !=currtable-> attrint_i[tmp_attr_fir].equal_range(val).second; ++it ){
									index_combine.push_back((*it).second);
							}
						}
						if (opt[0] == "COUNT"){
							cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
							cout << index_combine.size()<<endl;
							return;
						}
						else if (opt[0] == "SUM"){
							if (target_size[0] == -1){
								int sum = 0;
								for (index_inner=index_combine.begin(); index_inner!= index_combine.end(); ++index_inner){
									tmp_i = currtable->attrint[target_list[0]];
									sum = sum + tmp_i[*index_inner];
								}
								cout << "SUM ("<< target_list[0]<<")"<<endl;
								cout << sum << endl;
								return;
							}else{
								cout << "ERROR:"<<endl;
								cout << "We cannot add string type"<<endl;
								return;
							}
						}
						else{
							for(index_inner =index_combine.begin();index_inner!= index_combine.end();++index_inner){
								for (int i=0;i<target_list.size();i++){								
									auto tmp = target_list[i];
									int value = target_size[i];
									if (value== -1){
										if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
											tmp_i = currtable->attrint[tmp];
											tmp_index = *index_inner;
											int_v.push_back(tmp_i[tmp_index]);
										}else{
											tmp_i = currtable->attrint[tmp_attr_fir];
											tmp_index = *index_inner;
											val = tmp_i[tmp_index];
											it = currtable_sec-> attrint_i[tmp_attr_sec].find(val);
											tmp_i = currtable_sec->attrint[tmp];
											tmp_index = (*it).second;
											int_v.push_back(tmp_i[tmp_index]);
										}
									}else {
										if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
											tmp_v = currtable->attrvar[tmp];
											tmp_index = *index_inner;
											str_v.push_back(tmp_v[tmp_index]);
										}else{
											tmp_i = currtable->attrint[tmp_attr_fir];
											tmp_index = *index_inner;
											val = tmp_i[tmp_index];
											it = currtable_sec-> attrint_i[tmp_attr_sec].find(val);
											tmp_v = currtable_sec->attrvar[tmp];
											tmp_index = (*it).second;
											str_v.push_back(tmp_v[tmp_index]);
										}
									}
									
								}
								new_row(selecttable, int_v, str_v);
								for (int i=0;i<target_list.size();i++){
									int value = target_size[i];
									if (value == -1){
										int_v.pop_front();
									}else{
										str_v.pop_front();
									}
								}
							}
							print_table(selecttable);
							return;
						}
					}
				}
				// It represents table_second mapping to table_fir
				else if (compare_inner_join_i_sec.size()!=currtable_sec->attrint[tmp_attr_sec].size()){
					if (which_table_tmp.compare(which_table_ws) == 0){
						inner_join_tmp_i = currtable_sec->attrint[tmp_attr_sec];
						
						if (opt[0] == "COUNT"){
							cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
							cout << index_i.size()<<endl;
							return;
						}
						else if (opt[0] == "SUM"){
							if (target_size[0] == -1){
								int sum = 0;
								for (index_it=index_i.begin(); index_it!= index_i.end(); ++index_it){
									tmp_i = currtable->attrint[target_list[0]];
									sum = sum + tmp_i[*index_it];
								}
								cout << "SUM ("<< target_list[0]<<")"<<endl;
								cout << sum << endl;
								return;
							}else{
								cout << "ERROR:"<<endl;
								cout << "We cannot add string type"<<endl;
								return;
							}
						}
						else{
							// Print table
							for (index_it = index_i.begin();index_it != index_i.end(); ++index_it){
								for (int i=0;i<target_list.size();i++){
									string tmp = target_list[i];
									int value = target_size[i];
									if (value == -1){
										if (attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
											tmp_i = currtable_sec->attrint[tmp];
											tmp_index = *index_it;
											int_v.push_back(tmp_i[tmp_index]);
										}else{
											tmp_index = inner_join_tmp_i[*index_it];
											it = currtable-> attrint_i[tmp_attr_fir].find(tmp_index);
											tmp_i = currtable->attrint[tmp];
											tmp_index = (*it).second;
											int_v.push_back(tmp_i[tmp_index]);
										}
									}else{
										if (attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
											tmp_v = currtable_sec->attrvar[tmp];
											tmp_index = *index_it;
											str_v.push_back(tmp_v[tmp_index]);
										}else{
											tmp_index = inner_join_tmp_i[*index_it];
											it = currtable-> attrint_i[tmp_attr_fir].find(tmp_index);
											tmp_v = currtable->attrvar[tmp];
											tmp_index = (*it).second;
											str_v.push_back(tmp_v[tmp_index]);
										}
									}
								}
								new_row(selecttable, int_v, str_v);
								for (int i=0;i<target_list.size();i++){
									int value = target_size[i];
									if (value == -1){
										int_v.pop_front();
									}else{
										str_v.pop_front();
									}
								}
							}
							print_table(selecttable);
							return;
						}
						
						
					}
					else if (which_table_tmp.compare(which_table_wf) == 0){
						inner_join_tmp_i = currtable->attrint[tmp_attr_fir];
						// 
						for (index_it = index_i.begin();index_it != index_i.end(); ++index_it){
							int tmp_i = *index_it;
							compare_int_inner_join.push_back(inner_join_tmp_i[tmp_i]);
						}
						
						for (int i=0;i<compare_int_inner_join.size();i++ ){
							val = compare_int_inner_join[i];
							for ( it = currtable_sec-> attrint_i[tmp_attr_fir].equal_range(val).first; 
								it !=currtable_sec-> attrint_i[tmp_attr_fir].equal_range(val).second; ++it ){
									index_combine.push_back((*it).second);
							}
						}
						if (opt[0] == "COUNT"){
							cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
							cout << index_combine.size()<<endl;
							return;
						}
						else if (opt[0] == "SUM"){
							if (target_size[0] == -1){
								int sum = 0;
								for (index_inner=index_combine.begin(); index_inner!= index_combine.end(); ++index_inner){
									tmp_i = currtable->attrint[target_list[0]];
									sum = sum + tmp_i[*index_inner];
								}
								cout << "SUM ("<< target_list[0]<<")"<<endl;
								cout << sum << endl;
								return;
							}else{
								cout << "ERROR:"<<endl;
								cout << "We cannot add string type"<<endl;
								return;
							}
						}
						else{
							// Print Table
							for(index_inner =index_combine.begin();index_inner!= index_combine.end();++index_inner){
								for (int i=0;i<target_list.size();i++){								
									auto tmp = target_list[i];
									int value = target_size[i];
									if (value== -1){
										if (attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
											tmp_i = currtable_sec->attrint[tmp];
											tmp_index = *index_inner;
											int_v.push_back(tmp_i[tmp_index]);
										}else{
											tmp_i = currtable_sec->attrint[tmp_attr_sec];
											tmp_index = *index_inner;
											val = tmp_i[tmp_index];
											it = currtable-> attrint_i[tmp_attr_fir].find(val);
											tmp_i = currtable->attrint[tmp];
											tmp_index = (*it).second;
											int_v.push_back(tmp_i[tmp_index]);
										}
									}else {
										if (attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
											tmp_v = currtable_sec->attrvar[tmp];
											tmp_index = *index_inner;
											str_v.push_back(tmp_v[tmp_index]);
										}else{
											tmp_i = currtable_sec->attrint[tmp_attr_sec];
											tmp_index = *index_inner;
											val = tmp_i[tmp_index];
											it = currtable-> attrint_i[tmp_attr_fir].find(val);
											tmp_v = currtable->attrvar[tmp];
											tmp_index = (*it).second;
											str_v.push_back(tmp_v[tmp_index]);
										}
									}
									
								}
								new_row(selecttable, int_v, str_v);
								for (int i=0;i<target_list.size();i++){
									int value = target_size[i];
									if (value == -1){
										int_v.pop_front();
									}else{
										str_v.pop_front();
									}
								}
							}
							print_table(selecttable);
							return;
						}
					}
				}
			}
			else if (value_fir == 30 && value_sec== 30){
				vector<string>	inner_join_tmp_v;
				vector<string>	compare_var_inner_join;
				vector<int>	::iterator	index_inner;
				vector<int>	index_combine;
				
				int tmp_index;
				string val;
				
				if (compare_inner_join_v_fir.size()!=currtable->attrvar[tmp_attr_fir].size()){
					if (which_table_tmp.compare(which_table_wf) == 0){
						inner_join_tmp_v = currtable->attrvar[tmp_attr_fir];
						
						if (opt[0] == "COUNT"){
							cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
							cout << index_i.size()<<endl;
							return;
						}
						else if (opt[0] == "SUM"){
							if (target_size[0] == -1){
								int sum = 0;
								for (index_it=index_i.begin(); index_it!= index_i.end(); ++index_it){
									tmp_i = currtable->attrint[target_list[0]];
									sum = sum + tmp_i[*index_it];
								}
								cout << "SUM ("<< target_list[0]<<")"<<endl;
								cout << sum << endl;
								return;
							}else{
								cout << "ERROR:"<<endl;
								cout << "We cannot add string type"<<endl;
								return;
							}
						}
						else{
							// Print table
							for (index_it = index_i.begin();index_it != index_i.end(); ++index_it){
								for (int i=0;i<target_list.size();i++){
									string tmp = target_list[i];
									int value = target_size[i];
									if (value == -1){
										if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
											tmp_i = currtable->attrint[tmp];
											tmp_index = *index_it;
											int_v.push_back(tmp_i[tmp_index]);
										}else{
											val = inner_join_tmp_v[*index_it];
											it_var = currtable_sec-> attrvar_i[tmp_attr_sec].find(val);
											tmp_i = currtable_sec->attrint[tmp];
											tmp_index = (*it_var).second;
											int_v.push_back(tmp_i[tmp_index]);
										}
									}else{
										if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
											tmp_v = currtable->attrvar[tmp];
											tmp_index = *index_it;
											str_v.push_back(tmp_v[tmp_index]);
										}else{
											val = inner_join_tmp_i[*index_it];
											it_var = currtable_sec-> attrvar_i[tmp_attr_sec].find(val);
											tmp_v = currtable_sec->attrvar[tmp];
											tmp_index = (*it_var).second;
											str_v.push_back(tmp_v[tmp_index]);
										}
									}
								}
								new_row(selecttable, int_v, str_v);
								for (int i=0;i<target_list.size();i++){
									int value = target_size[i];
									if (value == -1){
										int_v.pop_front();
									}else{
										str_v.pop_front();
									}
								}
							}
							print_table(selecttable);
							return;
						}
						
					}
					else if (which_table_tmp.compare(which_table_ws) == 0){
						inner_join_tmp_v = currtable_sec->attrvar[tmp_attr_sec];
						// 
						for (index_it = index_i.begin();index_it != index_i.end(); ++index_it){
							int tmp_i = *index_it;
							compare_var_inner_join.push_back(inner_join_tmp_v[tmp_i]);
						}
						
						for (int i=0;i<compare_var_inner_join.size();i++ ){
							val = compare_var_inner_join[i];
							for ( it_var = currtable-> attrvar_i[tmp_attr_fir].equal_range(val).first; 
								it_var !=currtable-> attrvar_i[tmp_attr_fir].equal_range(val).second; ++it_var ){
									index_combine.push_back((*it_var).second);
							}
						}
						
						if (opt[0] == "COUNT"){
							cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
							cout << index_combine.size()<<endl;
							return;
						}
						else if (opt[0] == "SUM"){
							if (target_size[0] == -1){
								int sum = 0;
								for (index_inner=index_combine.begin(); index_inner!= index_combine.end(); ++index_inner){
									tmp_i = currtable->attrint[target_list[0]];
									sum = sum + tmp_i[*index_inner];
								}
								cout << "SUM ("<< target_list[0]<<")"<<endl;
								cout << sum << endl;
								return;
							}else{
								cout << "ERROR:"<<endl;
								cout << "We cannot add string type"<<endl;
								return;
							}
						}
						else{
							// Print Table
							for(index_inner =index_combine.begin();index_inner!= index_combine.end();++index_inner){
								for (int i=0;i<target_list.size();i++){								
									string tmp = target_list[i];
									int value = target_size[i];
									if (value== -1){
										if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
											tmp_i = currtable->attrint[tmp];
											tmp_index = *index_inner;
											int_v.push_back(tmp_i[tmp_index]);
										}else{
											tmp_v = currtable->attrvar[tmp_attr_fir];
											tmp_index = *index_inner;
											val = tmp_v[tmp_index];
											it_var = currtable_sec-> attrvar_i[tmp_attr_sec].find(val);
											tmp_i = currtable_sec->attrint[tmp];
											tmp_index = (*it_var).second;
											int_v.push_back(tmp_i[tmp_index]);
										}
									}else {
										if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
											tmp_v = currtable->attrvar[tmp];
											tmp_index = *index_inner;
											str_v.push_back(tmp_v[tmp_index]);
										}else{
											tmp_v = currtable->attrvar[tmp_attr_fir];
											tmp_index = *index_inner;
											val = tmp_v[tmp_index];
											it_var = currtable_sec-> attrvar_i[tmp_attr_sec].find(val);
											tmp_v = currtable_sec->attrvar[tmp];
											tmp_index = (*it_var).second;
											str_v.push_back(tmp_v[tmp_index]);
										}
									}
									
								}
								new_row(selecttable, int_v, str_v);
								for (int i=0;i<target_list.size();i++){
									int value = target_size[i];
									if (value == -1){
										int_v.pop_front();
									}else{
										str_v.pop_front();
									}
								}
							}
							print_table(selecttable);
							return;
						}
					}
				}
				else if (compare_inner_join_v_sec.size()!=currtable_sec->attrvar[tmp_attr_sec].size()){
					if (which_table_tmp.compare(which_table_ws) == 0){
						inner_join_tmp_v = currtable_sec->attrvar[tmp_attr_fir];
						
						if (opt[0] == "COUNT"){
							cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
							cout << index_i.size()<<endl;
							return;
						}
						else if (opt[0] == "SUM"){
							if (target_size[0] == -1){
								int sum = 0;
								for (index_it=index_i.begin(); index_it!= index_i.end(); ++index_it){
									tmp_i = currtable->attrint[target_list[0]];
									sum = sum + tmp_i[*index_it];
								}
								cout << "SUM ("<< target_list[0]<<")"<<endl;
								cout << sum << endl;
								return;
							}else{
								cout << "ERROR:"<<endl;
								cout << "We cannot add string type"<<endl;
								return;
							}
						}
						else{
							// Print table
							for (index_it = index_i.begin();index_it != index_i.end(); ++index_it){
								for (int i=0;i<target_list.size();i++){
									string tmp = target_list[i];
									int value = target_size[i];
									if (value == -1){
										if (attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
											tmp_i = currtable_sec->attrint[tmp];
											tmp_index = *index_it;
											int_v.push_back(tmp_i[tmp_index]);
										}else{
											val = inner_join_tmp_v[*index_it];
											it_var = currtable-> attrvar_i[tmp_attr_fir].find(val);
											tmp_i = currtable->attrint[tmp];
											tmp_index = (*it_var).second;
											int_v.push_back(tmp_i[tmp_index]);
										}
									}else{
										if (attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
											tmp_v = currtable_sec->attrvar[tmp];
											tmp_index = *index_it;
											str_v.push_back(tmp_v[tmp_index]);
										}else{
											val = inner_join_tmp_i[*index_it];
											it_var = currtable-> attrvar_i[tmp_attr_fir].find(val);
											tmp_v = currtable->attrvar[tmp];
											tmp_index = (*it_var).second;
											str_v.push_back(tmp_v[tmp_index]);
										}
									}
								}
								new_row(selecttable, int_v, str_v);
								for (int i=0;i<target_list.size();i++){
									int value = target_size[i];
									if (value == -1){
										int_v.pop_front();
									}else{
										str_v.pop_front();
									}
								}
							}
							print_table(selecttable);
							return;
						}
					}
					else if (which_table_tmp.compare(which_table_wf) == 0){
						inner_join_tmp_v = currtable->attrvar[tmp_attr_fir];
						// print table
						for (index_it = index_i.begin();index_it != index_i.end(); ++index_it){
							int tmp_i = *index_it;
							compare_var_inner_join.push_back(inner_join_tmp_v[tmp_i]);
						}
					
						for (int i=0;i<compare_var_inner_join.size();i++ ){
							val = compare_var_inner_join[i];
							for ( it_var = currtable_sec-> attrvar_i[tmp_attr_sec].equal_range(val).first; 
								it_var !=currtable_sec-> attrvar_i[tmp_attr_sec].equal_range(val).second; ++it_var ){
									index_combine.push_back((*it_var).second);
							}
						}
						
						if (opt[0] == "COUNT"){
							cout << "COUNT ("<< SEL_alias[0]<<")"<<endl;
							cout << index_combine.size()<<endl;
							return;
						}
						else if (opt[0] == "SUM"){
							if (target_size[0] == -1){
								int sum = 0;
								for (index_inner=index_combine.begin(); index_inner!= index_combine.end(); ++index_inner){
									tmp_i = currtable->attrint[target_list[0]];
									sum = sum + tmp_i[*index_inner];
								}
								cout << "SUM ("<< target_list[0]<<")"<<endl;
								cout << sum << endl;
								return;
							}else{
								cout << "ERROR:"<<endl;
								cout << "We cannot add string type"<<endl;
								return;
							}
						}
						else{
							// Print Table
							for(index_inner =index_combine.begin();index_inner!= index_combine.end();++index_inner){
								for (int i=0;i<target_list.size();i++){								
									string tmp = target_list[i];
									int value = target_size[i];
									if (value== -1){
										if (attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
											tmp_i = currtable_sec->attrint[tmp];
											tmp_index = *index_inner;
											int_v.push_back(tmp_i[tmp_index]);
										}else{
											tmp_v = currtable_sec->attrvar[tmp_attr_sec];
											tmp_index = *index_inner;
											val = tmp_v[tmp_index];
											it_var = currtable-> attrvar_i[tmp_attr_fir].find(val);
											tmp_i = currtable->attrint[tmp];
											tmp_index = (*it_var).second;
											int_v.push_back(tmp_i[tmp_index]);
										}
									}else {
										if (attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
											tmp_v = currtable_sec->attrvar[tmp];
											tmp_index = *index_inner;
											str_v.push_back(tmp_v[tmp_index]);
										}else{
											tmp_v = currtable_sec->attrvar[tmp_attr_sec];
											tmp_index = *index_inner;
											val = tmp_v[tmp_index];
											it_var = currtable-> attrvar_i[tmp_attr_fir].find(val);
											tmp_v = currtable->attrvar[tmp];
											tmp_index = (*it_var).second;
											str_v.push_back(tmp_v[tmp_index]);
										}
									}
									
								}
								new_row(selecttable, int_v, str_v);
								for (int i=0;i<target_list.size();i++){
									int value = target_size[i];
									if (value == -1){
										int_v.pop_front();
									}else{
										str_v.pop_front();
									}
								}
							}
							print_table(selecttable);
							return;
						}
					}
				}
			}
			else{
				cout << "error: different types and cannot be compared."<<endl;
				return;					
			}
		}
	}
}
void select_h (lua_State* L)
{
	int command_num = lua_objlen(L,-1);

	vector<string> SEL_alias;   				// include '*'
	vector<string> SEL_target;					// all select target list
	vector<string> opt;							// function_opt = COUNT or SUM

	vector<string> From_table_name;
	vector<string> From_alias;
	/* The example for all variation: e.g WHERE authorId = 1 OR pages < 200;
	   check_where_num=2; where_condition_first_alias = authorId; op_flag_first = "eq";compare_int_first = 1;
	   logical_op = OR;
	   where_condition_second_alias = pages; second = "lt"; compare_int_second = 200
	*/
	vector<int>	check_where_num;
	vector<string> where_condition_first_alias; // it can be alias name, which occurs in this condition(where_condition_first_attrname is not empty), or attribute name.
	vector<string> where_condition_first_attrname;
	vector<string> where_condition_second_alias;
	vector<string> where_condition_second_attrname;
	vector<string> logical_op;
	string op_flag_first;
	string op_flag_second;
	vector<string> compare_alias_char_first;
	vector<string> compare_attr_char_first;
	vector<string> compare_alias_char_second;
	vector<string> compare_attr_char_second;
	int compare_int_first;
	int compare_int_second;
	// Convert the string to integer
	stringstream convert_stoi;

	// Extract the lua parser the command
	for (auto i=2;i<=command_num;i++){
		lua_rawgeti(L,-1,i);
		int check_elem = lua_objlen(L,-1);
		//distinguish from "SELECT"(i=2), "From"(i=3), and "Where" query(i=4)
		if (i==2) {
			for (auto j=1;j<=check_elem;j++){
				lua_rawgeti(L,-1,j);
				if (j>=2){
					int check_subelem = lua_objlen(L,-1);
					for (auto k=1;k<=check_subelem;k++) {
						if (k==1){
							lua_rawgeti(L,-1,k);
							SEL_alias.push_back(string(lua_tostring(L, -1)));
							lua_pop(L,1);
						} else{
							lua_rawgeti(L,-1,k);
							SEL_target.push_back(string(lua_tostring(L, -1)));
							lua_pop(L,1);
						}
					}
				} else {
					// opt = COUNT or SUM or attr or null
					opt.push_back(string(lua_tostring(L, -1)));
				}
				lua_pop(L,1);
			}
		} else if (i==3) {
			for (auto j=1;j<=check_elem;j++){
				lua_rawgeti(L,-1,j);
				int check_subelem = lua_objlen(L,-1);
				string tmp;
				for (auto k=1;k<=check_subelem;k++) {
					if (k==1){
						lua_rawgeti(L,-1,k);
						tmp = lua_tostring(L, -1);
						From_table_name.push_back(tmp);
						lua_pop(L,1);
					}
					From_alias.push_back(tmp);
					/* IF there is a true alias define, we erase the first element
					*  and then we push_back the true alias define.*/
					if (k == 2){
						From_alias.erase(From_alias.begin());
						lua_rawgeti(L,-1,k);
						From_alias.push_back(string(lua_tostring(L, -1)));
						lua_pop(L,1);
					}
				}
				lua_pop(L,1);
			}
		} else {
			for (auto j=1;j<=check_elem;j++){
				lua_rawgeti(L,-1,j);
				int check_subelem = lua_objlen(L,-1);
				check_where_num.push_back(check_elem);
				if (j==2){
					logical_op.push_back(string(lua_tostring(L, -1)));  // logical_op = "AND" or "OR"
				} else {
					for (auto k=1;k<=check_subelem;k++) {
						lua_rawgeti(L,-1,k);
						int check_alias = lua_objlen(L,-1);
						for (auto it=1;it<=check_alias;it++) {
							lua_rawgeti(L,-1,it);
							if (j ==1 && k==1 && it ==1){
								where_condition_first_alias.push_back(lua_tostring(L, -1));
								lua_pop(L,1);
							}else if (j == 1 && k==1 && it ==2){
								where_condition_first_attrname.push_back(lua_tostring(L, -1));
								lua_pop(L,1);
							}else if (j == 1 && k==2 && it == 1){
								op_flag_first = lua_tostring(L, -1);
								lua_pop(L,1);
							}else if (j == 1 && k==3 && it ==1){
								compare_alias_char_first.push_back(lua_tostring(L, -1));
								lua_pop(L,1);
							}else if (j == 1 && k==3 && it ==2){
								compare_attr_char_first.push_back(lua_tostring(L, -1));;
								lua_pop(L,1);
							}else if (j ==3 && k==1 && it ==1){
								where_condition_second_alias.push_back((lua_tostring(L, -1)));
								lua_pop(L,1);
							}else if (j==3 && k ==1 && it ==2){
								where_condition_second_attrname.push_back((lua_tostring(L, -1)));
								lua_pop(L,1);
							}else if (j==3 && k ==2 && it ==1){
								op_flag_second = lua_tostring(L, -1);
								lua_pop(L,1);
							}else if (j==3 && k ==3 && it ==1){
								compare_alias_char_second.push_back(lua_tostring(L, -1));
								lua_pop(L,1);
							}else if (j==3 && k ==3 && it ==2){
								compare_attr_char_second.push_back(lua_tostring(L, -1));
								lua_pop(L,1);
							}
						}
						lua_pop(L,1);
					}
				}
				lua_pop(L,1);
			}

		}
		lua_pop(L,1);
	}
	// define check_where_number
	int wii = check_where_num.empty();
	int lii = logical_op.empty();
	if(wii ==1){
		check_where_num.push_back(0);
	}else if(wii !=1 && lii!=1){
		check_where_num[0] = check_where_num[0] -1;
	}

	// mapping alias to table name
	map<string,string> alias_name_to_table;				//{key,value} = {alias, table_name};
	int table_number = From_table_name.size();
	int alias_number = From_alias.size();
	if (alias_number != 0){
		for (int i =0;i<table_number;i++){
			alias_name_to_table.insert({From_alias[i],From_table_name[i]});
		}
	}



	struct Table_h* currtable = nullptr;
	struct Table_h* currtable_sec = nullptr;

	if(tables_h.find(From_table_name[0]) != tables_h.end()) {
        currtable = tables_h[From_table_name[0]];
    }
	if (From_table_name.size() == 2){
		if(tables_h.find(From_table_name[1]) != tables_h.end()) {
			currtable_sec = tables_h[From_table_name[1]];
		}
	}
	vector<string> target_list;   			// name of target_list
	vector<string> target_list_combine;		// name with alias and target_list
	vector<int> target_size;	  			// size of target_list
    deque<string> str_v;
    deque<int> int_v;
	vector<int> tmp_i;									// in order to transform vector to deque
	vector<string> tmp_v;

	// There is only one table which is selected.
	if(From_table_name.size() == 1){
		if(check_where_num[0]==0){
			if (SEL_alias[0] == "*"){

				// get the table name (From_table_name[0]), finding the table's attrlist and attrsize
				target_list = currtable->attrname;
				int target_list_number = target_list.size();
				for (int in =0; in<target_list_number; in++){
					int value = currtable-> attrsize[target_list[in]];
					target_size.push_back(value);
				}
			}else{
				int target_number = SEL_alias.size();
				for (int i=0;i<target_number;i++){
					auto tmp = SEL_alias[i];
					target_list.push_back(tmp);
					int value = currtable-> attrsize[tmp];
					target_size.push_back(value);
				}
			}
			auto selecttable = new_table_h(target_list,target_size);
			int target_list_number = target_list.size();

			for(int i = 0; i < currtable->rownum; i++) {
				for(int in =0; in< target_list_number; in++){
					int value = target_size[in];
					if (value == -1){
						tmp_i = currtable->attrint[target_list[in]];
						int_v.push_back(tmp_i[i]);
					}else{
						tmp_v = currtable->attrvar[target_list[in]];
						str_v.push_back(tmp_v[i]);
					}
				}
				new_row_h(selecttable, int_v, str_v);
				for(int in =0; in< target_list_number; in++){
					int value = target_size[in];
					if (value == -1){
						int_v.pop_front();
					}else{
						str_v.pop_front();
					}
				}
			}
			print_table_h(selecttable);
		}
		else if (check_where_num[0]==1){
			// Create Selected target table
			if (SEL_alias[0] == "*"){
				// get the table name (From_table_name[0]), finding the table's attrlist and attrsize
				target_list = currtable->attrname;
				int target_list_number = target_list.size();
				for (int in =0; in<target_list_number; in++){
					int value = currtable-> attrsize[target_list[in]];
					target_size.push_back(value);
				}
			}else{
				int target_number = SEL_alias.size();
				for (int i=0;i<target_number;i++){
					auto tmp = SEL_alias[i];
					target_list.push_back(tmp);
					int value = currtable-> attrsize[tmp];
					target_size.push_back(value);
				}
			}
			auto selecttable = new_table_h(target_list,target_size);

			// Finding the index and Doing Where condition transform(char to int)
			int target_list_number = target_list.size();
			string attr_tmp = where_condition_first_alias[0];
			int value = currtable-> attrsize[attr_tmp];
			multimap<int,int>::iterator it,itlow,itup,eqit;
			multimap<string,int>::iterator it_var;
			vector<int> index_i;

			if (value== -1){
				// Converting the string to integer if the attribute is the integer
				convert_stoi << compare_alias_char_first[0];
				convert_stoi >> compare_int_first;

				if (op_flag_first == "eq"){
					for ( it = currtable-> attrint_i[attr_tmp].equal_range(compare_int_first).first;
							  it !=currtable-> attrint_i[attr_tmp].equal_range(compare_int_first).second; ++it ){
								  index_i.push_back((*it).second);
							  }
				}else if (op_flag_first == "gt"){
					itlow = currtable-> attrint_i[attr_tmp].lower_bound(compare_int_first+1);
					for (it = itlow; it!=currtable-> attrint_i[attr_tmp].end();++it){
							index_i.push_back((*it).second);
						}

				}else if (op_flag_first == "lt"){
					itup = currtable-> attrint_i[attr_tmp].upper_bound(compare_int_first-1);
					for (it = currtable-> attrint_i[attr_tmp].begin();it!=itup; ++it){
							index_i.push_back((*it).second);
						}
				}

			}
			else{
				if (op_flag_first == "eq"){
					//string tmp;
					//tmp = compare_alias_char_first[0];
					for ( it_var = currtable-> attrvar_i[attr_tmp].equal_range(compare_alias_char_first[0]).first;
							  it_var !=currtable-> attrvar_i[attr_tmp].equal_range(compare_alias_char_first[0]).second; ++it_var )
						{
						  index_i.push_back((*it_var).second);
						}
				}
			}
			//Doing the insertion function
			int target_number = SEL_alias.size();
			int index_number = index_i.size();
			for (int i=0;i<target_number;i++){
				for (int j=0; j< index_number; j++){
					auto tmp = SEL_alias[i];
					int value = target_size[i];
					if (value== -1){
						tmp_i = currtable->attrint[tmp];
						int tmp_index = index_i[j];
						int_v.push_back(tmp_i[tmp_index]);
					}else {
						tmp_v = currtable->attrvar[tmp];
						int tmp_index = index_i[j];
						str_v.push_back(tmp_v[tmp_index]);
					}
					new_row_h(selecttable, int_v, str_v);
					if (value == -1){
						int_v.pop_front();
					}else{
						str_v.pop_front();
					}
				}
			}
			print_table_h(selecttable);
			// Clear the element in the convert_stoi
			convert_stoi.str("");
			convert_stoi.clear();
		}
		else if(check_where_num[0]==2){
			// Create Selected target table
			if (SEL_alias[0] == "*"){
				// get the table name (From_table_name[0]), finding the table's attrlist and attrsize
				target_list = currtable->attrname;
				int target_list_number = target_list.size();
				for (int in =0; in<target_list_number; in++){
					int value = currtable-> attrsize[target_list[in]];
					target_size.push_back(value);
				}
			}
			else if(SEL_target.size() == 0){
				int target_number = SEL_alias.size();
				for (int i=0;i<target_number;i++){
					auto tmp = SEL_alias[i];
					target_list.push_back(tmp);
					int value = currtable-> attrsize[tmp];
					target_size.push_back(value);
				}
			}
			else if (SEL_target[0] == "*"){
				vector <string> tmp_attr;
				tmp_attr = currtable->attrname;
				int target_number = currtable->attrname.size();
				for (int i=0;i<target_number;i++){
					string tmp_combine = SEL_alias[0] +"."+ tmp_attr[i];
					target_list_combine.push_back(tmp_combine);
					int value = currtable-> attrsize[tmp_attr[i]];
					target_size.push_back(value);
				}
			}
			else if (SEL_target.size()!= 0 && SEL_target[0]!= "*"){
				int target_number = SEL_target.size();
				for (int i=0;i<target_number;i++){
					string tmp_combine = SEL_alias[i] +"."+ SEL_target[i];
					target_list.push_back(SEL_target[i]);
					target_list_combine.push_back(tmp_combine);
					int value = currtable-> attrsize[SEL_target[i]];
					target_size.push_back(value);
				}
			}
			// Wondering why this cannot compile
			struct Table_h *selecttable = new Table_h();

			if (target_list_combine.size()!=0){
				selecttable = new_table_h(target_list_combine,target_size);
			}else{
				selecttable = new_table_h(target_list,target_size);
			}

			// Finding the indexing value and Doing the where condition's value transform
			int target_list_number = target_list.size();
			string attr_tmp_fir = where_condition_first_alias[0];
			string attr_tmp_sec = where_condition_second_alias[0];
			int value_fir = currtable-> attrsize[attr_tmp_fir];
			int value_sec = currtable-> attrsize[attr_tmp_sec];
			multimap<int,int>::iterator it,itlow,itup;
			multimap<string,int>::iterator it_var;

			if (value_fir== -1){
				convert_stoi << compare_alias_char_first[0];
				convert_stoi >> compare_int_first;
				convert_stoi.str("");
				convert_stoi.clear();
			}

			if(value_sec == -1){
				convert_stoi << compare_alias_char_second[0];
				convert_stoi >> compare_int_second;
				convert_stoi.str("");
				convert_stoi.clear();
			}

			// Finding the index value and push back to index_i
			if ( logical_op[0] == "OR"){
				// The reason why I choose "set" is that it can prohibit the same value to be inserted.
				set<int> index_i;
				set<int> :: iterator index_it;
				// find the indexing value which is corresponding to the first statement.
				if (value_fir== -1){
					if (op_flag_first == "eq"){
						for ( it = currtable-> attrint_i[attr_tmp_fir].equal_range(compare_int_first).first;
							  it !=currtable-> attrint_i[attr_tmp_fir].equal_range(compare_int_first).second; ++it ){
								  index_i.insert((*it).second);
							  }
					}else if (op_flag_first == "gt"){
						itlow = currtable-> attrint_i[attr_tmp_fir].lower_bound(compare_int_first+1);
						for (it = itlow; it!=currtable-> attrint_i[attr_tmp_fir].end();++it){
							index_i.insert((*it).second);
						}
					}else {
						itup = currtable-> attrint_i[attr_tmp_fir].upper_bound(compare_int_first-1);
						for (it = currtable-> attrint_i[attr_tmp_fir].begin();it!=itup; ++it){
							index_i.insert((*it).second);
						}
					}
				}else{
					if (op_flag_first == "eq"){
						for ( it_var = currtable-> attrvar_i[attr_tmp_fir].equal_range(compare_alias_char_first[0]).first;
							  it_var !=currtable-> attrvar_i[attr_tmp_fir].equal_range(compare_alias_char_first[0]).second; ++it_var ){
								  index_i.insert((*it_var).second);
							  }
					}
				}
				// insert the value which is corresponding to the second statement.
				if (value_sec == -1){
					if (op_flag_second == "eq"){
						for ( it = currtable-> attrint_i[attr_tmp_sec].equal_range(compare_int_second).first;
							  it !=currtable-> attrint_i[attr_tmp_sec].equal_range(compare_int_second).second; ++it ){
								  index_i.insert((*it).second);
							  }
					}else if (op_flag_second == "gt"){

						itlow = currtable-> attrint_i[attr_tmp_sec].lower_bound(compare_int_second+1);
						for (it = itlow; it!=currtable-> attrint_i[attr_tmp_sec].end();++it){
							index_i.insert((*it).second);
						}
					}else {
						itup = currtable-> attrint_i[attr_tmp_sec].upper_bound(compare_int_second-1);
						for (it = currtable-> attrint_i[attr_tmp_sec].begin();it!=itup; ++it){
							index_i.insert((*it).second);
						}
					}
				}else {
					if (op_flag_second == "eq"){
						for ( it_var = currtable-> attrvar_i[attr_tmp_sec].equal_range(compare_alias_char_second[0]).first;
							  it_var !=currtable-> attrvar_i[attr_tmp_sec].equal_range(compare_alias_char_second[0]).second; ++it_var ){
								  index_i.insert((*it_var).second);
							  }
					}
				}
				// Insert the element
				int target_number = target_size.size();
				int index_number = index_i.size();
				for (index_it=index_i.begin(); index_it!= index_i.end(); ++index_it){
					for (int i=0;i<target_number;i++){
						auto tmp = target_list[i];
						int value = target_size[i];
						if (value== -1){
							tmp_i = currtable->attrint[tmp];
							int tmp_index = *index_it;
							int_v.push_back(tmp_i[tmp_index]);
						}else {
							tmp_v = currtable->attrvar[tmp];
							int tmp_index = *index_it;
							str_v.push_back(tmp_v[tmp_index]);
						}
					}
					new_row_h(selecttable, int_v, str_v);
					for (int i=0;i<target_number;i++){
						int value = target_size[i];
						if (value == -1){
							int_v.pop_front();
						}else{
							str_v.pop_front();
						}
					}
				}
			}
			else if ( logical_op[0] == "AND"){
				set<int> index_i_tmp;
				set<int> index_i;
				set<int> :: iterator index_it;
				set<int> :: iterator it_test;
				// find the indexing value which is corresponding to the first statement.

				if (value_fir== -1){
					if (op_flag_first == "eq"){
						for ( it = currtable-> attrint_i[attr_tmp_fir].equal_range(compare_int_first).first;
							  it !=currtable-> attrint_i[attr_tmp_fir].equal_range(compare_int_first).second; ++it ){
								  index_i_tmp.insert((*it).second);
							  }
					}else if (op_flag_first == "gt"){
						itlow = currtable-> attrint_i[attr_tmp_fir].lower_bound(compare_int_first+1);
						for (it = itlow; it!=currtable-> attrint_i[attr_tmp_fir].end();++it){
							index_i_tmp.insert((*it).second);
						}
					}else {
						itup = currtable-> attrint_i[attr_tmp_fir].upper_bound(compare_int_first-1);
						for (it = currtable-> attrint_i[attr_tmp_fir].begin();it!=itup; ++it){
							index_i_tmp.insert((*it).second);
						}
					}
				}else{
					if (op_flag_first == "eq"){
						for ( it_var = currtable-> attrvar_i[attr_tmp_fir].equal_range(compare_alias_char_first[0]).first;
							  it_var !=currtable-> attrvar_i[attr_tmp_fir].equal_range(compare_alias_char_first[0]).second; ++it_var )
						{
							index_i_tmp.insert((*it_var).second);
						}
					}
				}
				//If we can find the indexing value which is corresponding to the second statement and is exist in index_i_tmp,
				//we insert the value to insert_i.

				if (value_sec == -1){
					if (op_flag_second == "eq"){
						for ( it = currtable-> attrint_i[attr_tmp_sec].equal_range(compare_int_second).first;
							  it !=currtable-> attrint_i[attr_tmp_sec].equal_range(compare_int_second).second; ++it ){
								  if (index_i_tmp.find((*it).second)!= index_i_tmp.end()){
									  index_i.insert((*it).second);
								  }
							  }
					}else if (op_flag_second == "gt"){
						itlow = currtable-> attrint_i[attr_tmp_sec].lower_bound(compare_int_second+1);
						for (it = itlow; it!=currtable-> attrint_i[attr_tmp_sec].end();++it){
							if (index_i_tmp.find((*it).second)!= index_i_tmp.end()){
								index_i.insert((*it).second);
							}
						}
					}else {
						itup = currtable-> attrint_i[attr_tmp_sec].upper_bound(compare_int_second-1);
						for (it = currtable-> attrint_i[attr_tmp_sec].begin();it!=itup; ++it){
							if (index_i_tmp.find((*it).second)!= index_i_tmp.end()){
								index_i.insert((*it).second);
							}
						}
					}
				}else{
					if (op_flag_second == "eq"){
						for ( it_var = currtable-> attrvar_i[attr_tmp_sec].equal_range(compare_alias_char_second[0]).first;
							  it_var !=currtable-> attrvar_i[attr_tmp_sec].equal_range(compare_alias_char_second[0]).second; ++it_var )
						{
							if (index_i_tmp.find((*it_var).second) != index_i_tmp.end()){
								index_i.insert((*it_var).second);
							}
						}
					}
				}

				// Print the table
				int target_number = target_size.size();
				int index_number = index_i.size();
				for (index_it=index_i.begin(); index_it!= index_i.end(); ++index_it){
					for (int i=0;i<target_number;i++){
						auto tmp = target_list[i];
						int value = target_size[i];
						if (value== -1){
							tmp_i = currtable->attrint[tmp];
							int tmp_index = *index_it;
							int_v.push_back(tmp_i[tmp_index]);
						}else {
							tmp_v = currtable->attrvar[tmp];
							int tmp_index = *index_it;
							str_v.push_back(tmp_v[tmp_index]);
						}
					}
					new_row_h(selecttable, int_v, str_v);
					for (int i=0;i<target_number;i++){
						int value = target_size[i];
						if (value == -1){
							int_v.pop_front();
						}else{
							str_v.pop_front();
						}
					}
				}
			}

			print_table_h(selecttable);
			// Clear the element in the convert_stoi
			convert_stoi.str("");
			convert_stoi.clear();

		}
	}
	// There is two table which is selected.
	else if(From_table_name.size() == 2){
		if(SEL_target.size() == 0){
			set<string> attrname_tmp_fir(currtable->attrname.begin(),currtable->attrname.end());
			set<string> attrname_tmp_sec(currtable_sec->attrname.begin(),currtable_sec->attrname.end());
			int target_number = SEL_alias.size();
			for (int i=0;i<target_number;i++){
				auto tmp = SEL_alias[i];
				if (attrname_tmp_fir.find(tmp)!=attrname_tmp_fir.end()){
					target_list.push_back(tmp);
					int value = currtable-> attrsize[tmp];
					target_size.push_back(value);
				}
				else if (attrname_tmp_sec.find(tmp)!=attrname_tmp_sec.end()){
					target_list.push_back(tmp);
					int value = currtable_sec-> attrsize[tmp];
					target_size.push_back(value);
				}else
				{
					cout << "error: ";
					cout <<" The Selected element" << tmp <<" doesn't exist in the table"<<endl;
					return;
				}
			}
		}
		else if (From_alias.size()!=0){	//Means that users defines the alias
			int SEL_alias_size = SEL_alias.size();
			for (int i=0;i<SEL_alias_size;i++){
				string tmp = SEL_alias[i];
				//Separate the SEL_alias is true alias or target_name
				if (alias_name_to_table.find(tmp)==alias_name_to_table.end()){
					target_list_combine.push_back(tmp);
				}
			}


			/*
			vector <string> tmp_attr;
			tmp_attr = currtable->attrname;
			int target_number = currtable->attrname.size();
			for (int i=0;i<target_number;i++){
				string tmp_combine = SEL_alias[0] +"."+ tmp_attr[i];
				target_list_combine.push_back(tmp_combine);
				int value = currtable-> attrsize[tmp_attr[i]];
				target_size.push_back(value);
			}
			*/
		}
		else if (SEL_target.size()!= 0 && SEL_target[0]!= "*"){
			int target_number = SEL_target.size();
			for (int i=0;i<target_number;i++){
				string tmp_combine = SEL_alias[i] +"."+ SEL_target[i];
				target_list.push_back(SEL_target[i]);
				target_list_combine.push_back(tmp_combine);
				int value = currtable-> attrsize[SEL_target[i]];
				target_size.push_back(value);
			}
		}
		struct Table_h *selecttable = new Table_h();

		if (target_list_combine.size()!=0){
			selecttable = new_table_h(target_list_combine,target_size);
		}else{
			selecttable = new_table_h(target_list,target_size);
		}
	}
}



void print_tables()
{
    cout << endl << "Tables:" << endl;
    for(auto it = tables.begin(); it != tables.end(); ++it)
    {
        // Print table name
        cout << "[" << it->first << "]" << endl;
        auto table = it->second;
        // Print attributes
        for(auto str:table->attrname) {
            if (str == table->primkey)
                printf("%15s*(%2d)", str.c_str(), table->attrsize[str]);
            else
                printf("%16s(%2d)", str.c_str(), table->attrsize[str]);
        }
        cout << endl;
        // Print each row
        // Cache the types to make it efficient
        vector<int> attr_size;
        deque<vector<int>> attr_int;
        deque<vector<string>> attr_var;
        for(auto str:table->attrname) {
            attr_size.push_back(table->attrsize[str]);
            if (attr_size.back() == -1)
                attr_int.push_back(table->attrint[str]);
            else
                attr_var.push_back(table->attrvar[str]);
        }
        for(int i = 0; i < table->rownum; i++) {
            for(int j = 0; j < attr_size.size(); j++) {
                if (attr_size[j] == -1) {
                    printf("%20d", attr_int.front()[i]);
                    attr_int.push_back(attr_int.front());
                    attr_int.pop_front();
                } else {
                    printf("%20s", attr_var.front()[i].c_str());
                    attr_var.push_back(attr_var.front());
                    attr_var.pop_front();
                }
            }
            cout << endl;
        }
    }
}
void print_tables_h()
{
    cout << endl << "Tables:" << endl;
    for(auto it = tables_h.begin(); it != tables_h.end(); ++it)
    {
        // Print table name
        cout << "[" << it->first << "]" << endl;
        auto table = it->second;
        // Print attributes
        for(auto str:table->attrname) {
            if (str == table->primkey)
                printf("%15s*(%2d)", str.c_str(), table->attrsize[str]);
            else
                printf("%16s(%2d)", str.c_str(), table->attrsize[str]);
        }
        cout << endl;
        // Print each row
        // Cache the types to make it efficient
        vector<int> attr_size;
        deque<vector<int>> attr_int;
        deque<vector<string>> attr_var;
        for(auto str:table->attrname) {
            attr_size.push_back(table->attrsize[str]);
            if (attr_size.back() == -1)
                attr_int.push_back(table->attrint[str]);
            else
                attr_var.push_back(table->attrvar[str]);
        }
        for(int i = 0; i < table->rownum; i++) {
            for(int j = 0; j < attr_size.size(); j++) {
                if (attr_size[j] == -1) {
                    printf("%20d", attr_int.front()[i]);
                    attr_int.push_back(attr_int.front());
                    attr_int.pop_front();
                } else {
                    printf("%20s", attr_var.front()[i].c_str());
                    attr_var.push_back(attr_var.front());
                    attr_var.pop_front();
                }
            }
            cout << endl;
        }
    }
}
void save_tables_h(){
    char filename[]="saveFile.sql";
    fstream fp;
    fp.open(filename, ios::out);
    //fp<<"Hello HappyMan!!"<<endl;

    for(auto it = tables_h.begin(); it != tables_h.end(); ++it){
        vector<int> attr_size;
        deque<vector<int>> attr_int;
        deque<vector<string>> attr_var;
        fp<<"CREATE TABLE "<<it->first<<" (";
        auto table = it->second;
        int k=0;
        for(auto str:table->attrname) {

            if (str == table->primkey){
                if(table->attrsize[str]==-1)
                    fp<<str.c_str()<<" int PRIMARY KEY ";

                else
                    fp<<str.c_str()<<" varchar("<<table->attrsize[str]<<") PRIMARY KEY ";
            }

            else{
                if(table->attrsize[str]==-1)
                   fp<<str.c_str()<<" int ";
                else
                    fp<<str.c_str()<<" varchar("<<table->attrsize[str]<<") ";

            }
            k++;

            if(k==table->attrname.size())
                fp<<");"<<endl;
            else
                fp<<",";
        }
            for(auto str:table->attrname) {
            attr_size.push_back(table->attrsize[str]);
            if (attr_size.back() == -1)
                attr_int.push_back(table->attrint[str]);
            else
                attr_var.push_back(table->attrvar[str]);
        }

            for(int i = 0; i < table->rownum; i++) {

                    fp<<"INSERT INTO "<<it->first<<" VALUES (";
            for(int j = 0; j < attr_size.size(); j++) {

                if (attr_size[j] == -1) {
                    fp<<attr_int.front()[i];
                    attr_int.push_back(attr_int.front());
                    attr_int.pop_front();
                } else {
                    fp<<"\'"<<attr_var.front()[i].c_str()<<"\'";
                    attr_var.push_back(attr_var.front());
                    attr_var.pop_front();
                }
                if(j==attr_size.size()-1)
                    fp<<");"<<endl;
                else
                    fp<<",";
            }


        }

    }
    return;
}
void save_tables(){
    char filename[]="saveFile.sql";
    fstream fp;
    fp.open(filename, ios::out);
    //fp<<"Hello HappyMan!!"<<endl;

    for(auto it = tables.begin(); it != tables.end(); ++it){
        vector<int> attr_size;
        deque<vector<int>> attr_int;
        deque<vector<string>> attr_var;
        fp<<"CREATE TABLE "<<it->first<<" (";
        auto table = it->second;
        int k=0;
        for(auto str:table->attrname) {

            if (str == table->primkey){
                if(table->attrsize[str]==-1)
                    fp<<str.c_str()<<" int PRIMARY KEY ";

                else
                    fp<<str.c_str()<<" varchar("<<table->attrsize[str]<<") PRIMARY KEY ";
            }

            else{
                if(table->attrsize[str]==-1)
                   fp<<str.c_str()<<" int ";
                else
                    fp<<str.c_str()<<" varchar("<<table->attrsize[str]<<") ";

            }
            k++;

            if(k==table->attrname.size())
                fp<<");"<<endl;
            else
                fp<<",";
        }
            for(auto str:table->attrname) {
            attr_size.push_back(table->attrsize[str]);
            if (attr_size.back() == -1)
                attr_int.push_back(table->attrint[str]);
            else
                attr_var.push_back(table->attrvar[str]);
        }

            for(int i = 0; i < table->rownum; i++) {

                    fp<<"INSERT INTO "<<it->first<<" VALUES (";
            for(int j = 0; j < attr_size.size(); j++) {

                if (attr_size[j] == -1) {
                    fp<<attr_int.front()[i];
                    attr_int.push_back(attr_int.front());
                    attr_int.pop_front();
                } else {
                    fp<<"\'"<<attr_var.front()[i].c_str()<<"\'";
                    attr_var.push_back(attr_var.front());
                    attr_var.pop_front();
                }
                if(j==attr_size.size()-1)
                    fp<<");"<<endl;
                else
                    fp<<",";
            }


        }

    }
    return;
}
int main(int argc, char* argv[])
{
	clock_t start, finish;
	double time_length;
	char* inputfilename = nullptr;

    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <Input File>" << endl;
        return 0;

    } else {
        inputfilename = argv[1];
    }
    cout<< "enter 0 for tree, 1 for hash"<<endl;
    scanf("%d",&struct_type);
    if((struct_type!=0)&&(struct_type!=1)){
        cout<<"illegal input"<<endl;
        return 1;
    }
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    int status = luaL_loadfile(L, "src/parser.lua");
    if (status) {
        // If something went wrong, error message is at the top of
        // the stack
        cout << lua_tostring(L, -1) << endl;
        exit(1);
    }
    int result = lua_pcall(L, 0, 1, 0);
    if (result) {
        cout << lua_tostring(L, -1) << endl;
        lua_pop(L, 1);
    }

    lua_getglobal(L, "parseCommand");
    lua_pushnil(L);
    lua_pushstring(L, inputfilename);

    while(1) {
    start = clock();
        result = lua_pcall(L, 2, 1, 0);
        if (result) {
            cout << lua_tostring(L, -1) << endl;
            lua_pop(L, 1);
        } else {
            int command_num = lua_objlen(L, -1);
            for (auto i = 1; i <= command_num; i++) {
                // Get a command
                lua_rawgeti(L, -1, i);
                // Get operation
                lua_pushstring(L, "op");
                lua_rawget(L, -2);
                auto op = string(lua_tostring(L, -1));
                lua_pop(L, 1);

                if (op == "CREATE TABLE") {
                    cout << "Creating table...";
                    if(struct_type==0)
                        create_table(L);
                    else
                        create_table_h(L);
                } else if (op == "INSERT INTO") {
                    cout << "Inserting row...";
                    if(struct_type==0)
                        insert_into(L);
                    else
                        insert_into_h(L);

                } else if (op == "SELECT") {
                    // TODO
					//printf("SELECT is not yet implemented!\n");
					if(struct_type==0)
                        select(L);
                    else
                        select_h(L);
                       // cout<<"not implemented in hash mode"<<endl;
                } else {
                    cout << "Unknown operation " << op << endl;
                }
                // Pop command
                lua_pop(L, 1);
            }
            // Pop command list
            lua_pop(L, 1);
        }
        finish = clock();
        time_length = (double)(finish - start) / CLOCKS_PER_SEC;
        cout<<"Time used : "<<time_length<<" second."<<endl;
        string buf;
        std::getline (std::cin, buf);
        if(buf[0] == 'q') {
            return 0;
        }else if (buf[0] == 'i') {
			lua_getglobal(L, "parseCommand");
			lua_pushnil(L);
			std::getline (std::cin, buf);
			lua_pushstring(L, buf.c_str());

		}
		else if(buf[0] == 'p'&&buf[1] == 'r'&&buf[2] == 'i'&&buf[3] == 'n'&&buf[4] =='t'){
            if(struct_type==0)
                print_tables();
            else
                print_tables_h();
            lua_getglobal(L, "parseCommand");
            lua_pushnil(L);
		}
		else if(buf[0] == 's'&&buf[1] == 'a'&&buf[2] == 'v'&&buf[3] == 'e'){
            start = clock();
            cout << "Saving...";
            if(struct_type==0)
                save_tables();
            else
                save_tables_h();
            finish = clock();
            time_length = (double)(finish - start) / CLOCKS_PER_SEC;
            cout<<"Time used : "<<time_length<<" second."<<endl;
            lua_getglobal(L, "parseCommand");
            lua_pushnil(L);

		}
		else if(buf[0] == 'l'&&buf[1] == 'o'&&buf[2] == 'a'&&buf[3] == 'd'){

            cout << "Enter file name"<<endl;
            string filename;
            std::getline (std::cin, filename);
            start = clock();
            lua_getglobal(L, "parseCommand");
            lua_pushnil(L);
            lua_pushstring(L, filename.c_str());
            cout << "Loading...";
            finish = clock();
            time_length = (double)(finish - start) / CLOCKS_PER_SEC;
            cout<<"Time used : "<<time_length<<" second."<<endl;
		}
		else {
            lua_getglobal(L, "parseCommand");
            lua_pushstring(L, buf.c_str());
            lua_pushnil(L);
        }
    }

    lua_close(L);
}