#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <regex>
#include <map>
#include <vector>

using namespace std;



regex assignment("(^|[^=])=($|[^=])");
regex parenbool("^(\\((\\S|\\s)+(==|!=|<=|>=|<|>)(\\S|\\s)+\\))$");
regex normbool("^((\\S|\\s)+(==|!=|<=|>=|<|>)(\\S|\\s)+)$");
regex addoperator("([\\+])");
regex islist("^\\[((\\d*)|([a-zA-Z_$][a-zA-Z_$0-9]*))+(?:,\\s*((\\d*)|([a-zA-Z_$][a-zA-Z_$0-9]*))+)*\\]$");
regex isvar("^[a-zA-Z_$][a-zA-Z_$0-9]*$");
regex isnum("^[0-9]+$");
regex isvarindex("^[a-zA-Z_$][a-zA-Z_$0-9]*\\[((\\d)|([a-zA-Z_$][a-zA-Z_$0-9]*))+\\]$");
regex listcomprehension("^[a-zA-Z_$][a-zA-Z_$0-9]*\\[((\\d)*|([a-zA-Z_$][a-zA-Z_$0-9]*))*:((\\d)*|([a-zA-Z_$][a-zA-Z_$0-9]*))\\]$");
regex isstring("^\\\"[\\w\\W]*\\\"$");
regex printing("^(print)");
regex length("^(len)");
regex comment("(^#.*)");
regex ifopen("^(if )");
regex elseopen("^(else:)");
regex indent("^(    )+");
regex def("^(def)");
regex isfunccall("^(\\S+)\\s*\\(\\S*\\)$");
regex returncall("(return )");

map<string, string> varmap; // This map will store our variables in use
map<string, list<string>> funcmap; // This map will store all of our functions
int line_pos = 0;
smatch m;

string trim(string line);
pair<string, int> parsevarindex(map<string, string> varmap, string line);
vector<int> stringtoIntArray(map<string, string> varmap, string arr);
string intarraytoString(vector<int> arr);
string AssignListCompFunc(map<string, string> varmap, string line, vector<int> assign);
string RetrieveListCompFunc(map<string, string> varmap, string line);
string formatListPrinting(string arr);
void print(map<string, string> varmap, string line);
string len(map<string, string> varmap, string line);
string addint(map<string, string> varmap, list<string> vars);
string addlist(map<string, string> varmap, list<string> vars);
map<string, string> assignVar(map<string, string> varmap, string line);
map<string, list<string>> createFunc(map<string, list<string>> funcmap, string line, list<string> funclines);
string addOperate(map<string, string> varmap, string line);
int ifevaluate(map<string, string> varmap, string if_line);
string funcCall(map<string, string> varmap, string line);
string returnFunc(map<string, string> varmap, string returnline);
string PassLines(map<string, string> varmap, list<string> lines, bool infunc);

string trim(string line) // trim any whitespace at the start or end of a string
{
    if (line != "")
    {
        int first = line.find_first_not_of(' ');
        int last = line.find_last_not_of(' ');
        return line.substr(first, (last - first + 1));
    }
    else
    {
        return "";
    }

}


// when we encounter something like l2[3] we split it into the variable name and its index
// for l2[3] that would be l2 and 3
pair<string, int> parsevarindex(map<string, string> varmap, string line)
{
    pair<string, int> result;
    int first = line.find_first_of('[') + 1;
    int last = line.find_last_of(']') - 1;
    result.first = line.substr(0, first - 1);
    result.first = trim(result.first);
    string secondres = line.substr(first, last - first + 1);
    if (regex_search(secondres, m, isvar))
    {
        if (varmap.count(secondres))
        {
            if (regex_search(varmap[secondres], m, islist))
            {
                cout << "List cannot be an index value" << endl;
                cout << "Line: " << line_pos << endl;
                cout << line << endl;
                cout << secondres << endl;
                exit(1);
            }
            else
            {
                result.second = stoi(varmap[secondres]);
            }

        }
        else
        {
            cout << "Variable as index does not exist" << endl;
            cout << "Line: " << line_pos << endl;
            cout << line << endl;
            cout << secondres << endl;
            exit(1);
        }
    }
    else if(regex_search(secondres, m, isnum))
    {
        result.second = stoi(secondres);
    }
    else
    {
        cout << "Invalid data type as index" << endl;
        cout << "Line: " << line_pos << endl;
        cout << line << endl;
        cout << secondres << endl;
        exit(1);
    }


    return result;

}


//This is to convert any string that looks like [1,2,3,4,5] into a vector that is more easily manipulable
vector<int> stringtoIntArray(map<string, string> varmap, string arr)
{
    vector<int> intarray;
    string holder = "";
    if (arr == "[]")
    {
        return intarray;
    }
    for (int k = 0; k < arr.length(); k++)
    {
        if (arr[k] == ',' || arr[k] == ']')
        {
            if (regex_search(holder, m, isnum))
            {
                intarray.push_back(stoi(holder));
                holder = "";
            }
            else if(regex_search(holder, m, isvar))
            {
                if (varmap.count(holder))
                {
                    intarray.push_back(stoi(varmap[holder]));
                    holder = "";
                }
                else
                {
                    cout << "Variable does not exist" << endl;
                    cout << "Line: " << line_pos << endl;
                    cout << arr << endl;
                    cout << holder << endl;
                    exit(1);
                }
            }
            else if (regex_search(holder, m, isvarindex))
            {
                pair<string, int> vname_vindex = parsevarindex(varmap, holder);
                if (varmap.count(vname_vindex.first))
                {
                    vector<int> intarray = stringtoIntArray(varmap, varmap[vname_vindex.first]);
                    if (intarray.size() > vname_vindex.second)
                    {
                        intarray.push_back(intarray.at(vname_vindex.second));
                    }
                    else
                    {
                        cout << "List out of bounds exception" << endl;
                        cout << "Line: " << line_pos << endl;
                        cout << arr << endl;
                        cout << holder << endl;
                        exit(1);
                    }
                }
                else
                {
                    cout << "Variable does not exist" << endl;
                    cout << "Line: " << line_pos << endl;
                    cout << arr << endl;
                    cout << holder << endl;
                    exit(1);
                }
                
            }
            else
            {
                cout << "Found an Invalid Character" << endl;
                cout << "Line: " << line_pos << endl;
                cout << arr << endl;
                exit(1);
            }
        }
        else if (arr[k] == ' ' || arr[k] == '[')
        {
            // do nothing
        }
        else
        {
            holder += arr[k];
        }
    }
    return intarray;
}

// This converts any vector back into its string format of [1,2,3,4,5]
string intarraytoString(vector<int> arr)
{
    string strarray = "[";
    int k;
    for (k = 0; k < arr.size(); k++)
    {
        strarray += to_string(arr.at(k)) + ",";
    }
    if (arr.size() == 0)
    {
        strarray += "]";
    }
    else
    {
        strarray.pop_back();
        strarray += "]";
    }


    return strarray;
}

// This function is responsible in assigning variables to list comp
string AssignListCompFunc(map<string, string> varmap, string line, vector<int> assign)
{
    string result;
    int first = line.find_first_of('[') + 1;
    int last = line.find_last_of(']') - 1;
    string element = line.substr(0, first - 1);
    string idxs = line.substr(first, last - first + 1);

    int colon = idxs.find_first_of(':');

    vector<int> elementarray;

    if (varmap.count(element))
    {
        elementarray = stringtoIntArray(varmap, varmap[element]);
    }
    else
    {
        cout << "Variable does not exist" << endl;
        cout << "Line: " << line_pos << endl;
        cout << line << endl;
        cout << element << endl;
        exit(1);
    }

    string start = idxs.substr(0, colon);
    string end = idxs.substr(colon + 1, idxs.size() - colon);
    start = trim(start);
    end = trim(start);

    int startidx;
    int endidx;

    if (regex_search(start, m, isvar))
    {
        if (varmap.count(start))
        {
            if (regex_search(varmap[start], m, islist))
            {
                cout << "List cannot be an index value" << endl;
                cout << "Line: " << line_pos << endl;
                cout << line << endl;
                cout << start << endl;
                exit(1);
            }
            else
            {
                startidx = stoi(varmap[start]);
            }

        }
        else
        {
            cout << "Variable does not exist" << endl;
            cout << "Line: " << line_pos << endl;
            cout << line << endl;
            cout << start << endl;
            exit(1);
        }
    }
    else if (regex_search(start, m, isnum))
    {
        startidx = stoi(start);
    }
    else if (start == "")
    {
        startidx = 0;
    }
    else
    {
        cout << "Invalid data type" << endl;
        cout << "Line: " << line_pos << endl;
        cout << line << endl;
        cout << start << endl;
        exit(1);
    }

    if (regex_search(end, m, isvar))
    {
        if (varmap.count(end))
        {
            if (regex_search(varmap[end], m, islist))
            {
                cout << "List cannot be an index value" << endl;
                cout << "Line: " << line_pos << endl;
                cout << line << endl;
                cout << end << endl;
                exit(1);
            }
            else
            {
                endidx = stoi(varmap[end]);
            }

        }
        else
        {
            cout << "Variable does not exist" << endl;
            cout << "Line: " << line_pos << endl;
            cout << line << endl;
            cout << end << endl;
            exit(1);
        }
    }
    else if (regex_search(end, m, isnum))
    {
        endidx = stoi(end);
    }
    else if (end == "")
    {
        endidx = elementarray.size() - 1;
    }
    else
    {
        cout << "Invalid data type" << endl;
        cout << "Line: " << line_pos << endl;
        cout << line << endl;
        cout << end << endl;
        exit(1);
    }

    int i;
    for (int k = startidx, i = 0; k <= endidx; k++, i++)
    {
        elementarray.at(k) = assign.at(i);
    }

    result = intarraytoString(elementarray);

    return result;
}

//This will return a value when a list contains :
string RetrieveListCompFunc(map<string, string> varmap, string line)
{
    string result;
    vector<int> resultarray;
    int first = line.find_first_of('[') + 1;
    int last = line.find_last_of(']') - 1;
    string element = line.substr(0, first - 1);
    string idxs = line.substr(first, last - first + 1);

    int colon = idxs.find_first_of(':');
    vector<int> elementarray;

    if (varmap.count(element))
    {
        elementarray = stringtoIntArray(varmap, varmap[element]);
    }
    else
    {
        cout << "Variable does not exist" << endl;
        cout << "Line: " << line_pos << endl;
        cout << line << endl;
        cout << element << endl;
        exit(1);
    }


    string start = idxs.substr(0, colon);
    string end = idxs.substr(colon + 1, idxs.size() - colon);
    start = trim(start);
    end = trim(start);

    int startidx;
    int endidx;

    if (regex_search(start, m, isvar))
    {
        if (varmap.count(start))
        {
            if (regex_search(varmap[start], m, islist))
            {
                cout << "List cannot be an index value" << endl;
                cout << "Line: " << line_pos << endl;
                cout << line << endl;
                cout << start << endl;
                exit(1);
            }
            else
            {
                startidx = stoi(varmap[start]);
            }

        }
        else
        {
            cout << "Variable does not exist" << endl;
            cout << "Line: " << line_pos << endl;
            cout << line << endl;
            cout << start << endl;
            exit(1);
        }
    }
    else if (regex_search(start, m, isnum))
    {
        startidx = stoi(start);
    }
    else if (start == "")
    {
        startidx = 0;
    }
    else
    {
        cout << "Invalid data type" << endl;
        cout << "Line: " << line_pos << endl;
        cout << line << endl;
        cout << start << endl;
        exit(1);
    }

    if (regex_search(end, m, isvar))
    {
        if (varmap.count(end))
        {
            if (regex_search(varmap[end], m, islist))
            {
                cout << "List cannot be an index value" << endl;
                cout << "Line: " << line_pos << endl;
                cout << line << endl;
                cout << end << endl;
                exit(1);
            }
            else
            {
                endidx = stoi(varmap[end]);
            }

        }
        else
        {
            cout << "Variable does not exist" << endl;
            cout << "Line: " << line_pos << endl;
            cout << line << endl;
            cout << end << endl;
            exit(1);
        }
    }
    else if (regex_search(end, m, isnum))
    {
        endidx = stoi(end);
    }
    else if (end == "")
    {
        endidx = elementarray.size() - 1;
    }
    else
    {
        cout << "Invalid data type" << endl;
        cout << "Line: " << line_pos << endl;
        cout << line << endl;
        cout << end << endl;
        exit(1);
    }

    for (int k = startidx; k <= endidx; k++)
    {
        resultarray.push_back(elementarray.at(k));
    }

    result = intarraytoString(resultarray);

    return result;

}

// Format a list properly for printing
string formatListPrinting(string arr)
{
    string holder = "";
    for (int k = 0; k < arr.length(); k++)
    {
        if (arr[k] == ',')
        {
            holder += ", ";
        }
        else if (arr[k] == ' ')
        {
            // do nothing
        }
        else if (arr[k] >= 48 && arr[k] <= 57)
        {
            holder += arr[k];
        }
        else if (arr[k] == '[' || arr[k] == ']')
        {
            holder += arr[k];
        }
        else
        {
            cout << "Found an Invalid Character" << endl;
            cout << "Line: " << line_pos << endl;
            cout << arr << endl;
            exit(1);
        }
    }
    return holder;
}


// Handling print() statements
void print(map<string, string> varmap, string line)
{
    //find the list of all arguments of the print call
    int first = line.find_first_of('(') + 1;
    int last = line.find_last_of(')') - 1;
    string args = line.substr(first, (last - first + 1));
    list<string> arglist;
    //cout << args << endl;
    string arg;
    int comma = args.find(",");
    if (comma == -1)
    {
        args = trim(args);
        arglist.push_back(args);
    }
    while (comma != -1)
    {
        arg = args.substr(0, comma);
        arg = trim(arg);
        arglist.push_back(arg);
        args = args.substr(comma + 1, args.length() - comma);
        comma = args.find(",");
        if (comma == -1)
        {
            args = trim(args);
            arglist.push_back(args);
        }
    }

    //parse through each arguement and append how each type should be represented
    string result = "";
    for (auto const& i : arglist)
    {
        if (regex_search(i, m, isvar))
        {
            if (varmap.count(i))
            {
                if (regex_search(varmap[i], m, islist))
                {
                    result += formatListPrinting(varmap[i]) + " ";
                }
                else
                {
                    result += varmap[i] + " ";
                }
                
            }
            else
            {
                cout << "Variable does not exist" << endl;
                cout << "Line: " << line_pos << endl;
                cout << line << endl;
                cout << i << endl;
                exit(1);
            }
        }
        else if (regex_search(i, m, isvarindex))
        {
            pair<string, int> vname_vindex = parsevarindex(varmap, i);
            if (varmap.count(vname_vindex.first))
            {
                vector<int> intarray = stringtoIntArray(varmap, varmap[vname_vindex.first]);
                if (intarray.size() > vname_vindex.second)
                {
                    result += to_string(intarray.at(vname_vindex.second)) + " ";
                }
                else
                {
                    cout << "List out of bounds exception" << endl;
                    cout << "Line: " << line_pos << endl;
                    cout << line << endl;
                    cout << i << endl;
                    exit(1);
                }

            }
            else
            {
                cout << "Variable does not exist" << endl;
                cout << "Line: " << line_pos << endl;
                cout << line << endl;
                cout << i << endl;
                exit(1);
            }
        }
        else if (regex_search(i, m, isnum))
        {
            result += i + " ";
        }
        else if (regex_search(i, m, isstring))
        {
            result += i.substr(1, i.length() - 2) + " ";
        }
        else if (regex_search(i, m, isfunccall))
        {
            result += funcCall(varmap, i);
        }
        else if (regex_search(i, m, listcomprehension))
        {
            result += RetrieveListCompFunc(varmap, i);
        }
    }
    // remove trailing whitespace
    result.pop_back();
    cout << result << endl;


}

string len(map<string, string> varmap, string line)
{
    int first = line.find_first_of('(') + 1;
    int last = line.find_last_of(')') - 1;
    string arg = line.substr(first, (last - first + 1));
    trim(arg);
    vector<int> listarr;
    if (regex_search(arg, m, islist))
    {
        listarr = stringtoIntArray(varmap, arg);
    }
    else if (regex_search(arg, m, isvar))
    {
        if (varmap.count(arg))
        {
            if (regex_search(varmap[arg], m, islist))
            {
                listarr = stringtoIntArray(varmap, varmap[arg]);
            }
            else
            {
                cout << "Invalid type use lists for len()" << endl;
                cout << line_pos << endl;
                cout << line << endl;
                cout << arg << endl;
                exit(1);
            }
        }
        else
        {
            cout << "Variable does not exist" << endl;
            cout << line_pos << endl;
            cout << line << endl;
            cout << arg << endl;
            exit(1);
        }
        
    }
    else
    {
        cout << "Invalid type use lists for len()" << endl;
        cout << line_pos << endl;
        cout << line << endl;
        cout << arg << endl;
        exit(1);
    }

    return to_string(listarr.size());
}

// function that adds ints together from a set of variables
string addint(map<string, string> varmap, list<string> vars)
{
    int result = 0;
    for (auto const& i : vars)
    {
        // if the variable is in varmap then we need to use that value
        if (regex_search(i, m, isvar))
        {
            result += stoi(varmap[i]);
        }
        // if we are asked to take a variable from a list we need to get that variable
        else if (regex_search(i, m, isvarindex))
        {
            pair<string, int> vname_vindex = parsevarindex(varmap, i);
            vector<int> intarray = stringtoIntArray(varmap, varmap[vname_vindex.first]);
            result += (intarray.at(vname_vindex.second));
        }
        else
        {
            result += stoi(i);
        }
    }

    return to_string(result);
}

// function that adds or combines lists together from a set of lists
string addlist(map<string, string> varmap, list<string> vars)
{
    // result will just combine all lists as strings 
    // we do a second pass over properly formatting everything 
    string result;
    string formated_result = "";
    for (auto const& i : vars)
    {
        if (i == "[]")
        {
            result += "";
        }
        else if (regex_search(i, m, isvar))
        {
            if (varmap[i] == "[]")
            {
                result += "";
            }
            else
            {
                result += varmap[i];
            }         
        }
        else if (regex_search(i, m, listcomprehension))
        {
            result += RetrieveListCompFunc(varmap, i);
        }
        else if (regex_search(i, m, islist))
        {
            result += intarraytoString(stringtoIntArray(varmap, i));
        }
        else 
        {
            result += i;
        }
    }
    formated_result += '[';
    for (int k = 1; k < result.size() - 1; k++)
    {
        if (result[k] == ']')
        {
            formated_result += ',';
        }
        else if (result[k] == '[')
        {
            //do nothing
        }
        else
        {
            formated_result += result[k];
        }
    }
    formated_result += ']';
    return formated_result;
}

// function splits up all variables that need to be added together aswell as determine if it is list or int addition
// also checks the validity of additions
string addOperate(map<string, string> varmap, string line)
{
    // this will split up the add statement into each of its individual variables
    // ex 1 + 2 + l3 + 4
    // vars = {1, 2, x, 4}
    string addline = line;
    string result;
    list<string> vars;
    string var;
    int plus = line.find("+");
    bool listflag = true;
    bool intflag = true;
    while (plus != -1)
    {
        var = addline.substr(0, plus);
        var = trim(var);
        vars.push_back(var);
        addline = addline.substr(plus + 1, addline.length() - plus);
        plus = addline.find("+");
        if (plus == -1)
        {
            addline = trim(addline);
            vars.push_back(addline);
        }
    }
    // iterate through our variables and see if we are trying to add ints or lists together
    list<string>::iterator iter;
    string i;
    for (iter = vars.begin(); iter != vars.end(); iter++)
    {
        i = *iter;
        if (regex_search(i, m, isvar))
        {
            if (varmap.count(i))
            {
                if (regex_search(varmap[i], m, islist))
                {
                    intflag = false;
                }
                else if (regex_search(varmap[i], m, isnum))
                {
                    listflag = false;
                }
            }
            else
            {
                cout << "Variable does not exist" << endl;
                cout << "Line: " << line_pos << endl;
                cout << line << endl;
                cout << i << endl;
                exit(1);
            }
        }
        else if (regex_search(i, m, isvarindex))
        {
            pair<string, int> vname_vindex = parsevarindex(varmap, i);
            if (varmap.count(vname_vindex.first))
            {
                vector<int> intarray = stringtoIntArray(varmap,varmap[vname_vindex.first]);
                if (intarray.size() > vname_vindex.second)
                {
                    listflag = false;
                }
                else
                {
                    cout << "List out of bounds exception" << endl;
                    cout << "Line: " << line_pos << endl;
                    cout << line << endl;
                    cout << i << endl;
                    exit(1);
                }

            }
            else
            {
                cout << "Variable does not exist" << endl;
                cout << "Line: " << line_pos << endl;
                cout << line << endl;
                cout << i << endl;
                exit(1);
            }

        }
        else if (regex_search(i, m, islist))
        {
            intflag = false;
        }
        else if (regex_search(i, m, listcomprehension))
        {
            intflag = false;
        }
        else if (regex_search(i, m, isnum))
        {
            listflag = false;
        }
        else if (regex_search(i, m, isfunccall))
        {
            *iter = funcCall(varmap, i);
            if (regex_search(*iter, m, isnum))
            {
                listflag = false;
            }
            else if (regex_search(*iter, m, islist))
            {
                intflag = false;
            }

        }
        else
        {
            cout << "Unrecognized Variable Type" << endl;
            cout << "Line: " << line_pos << endl;
            cout << line << endl;
            cout << i << endl;
            exit(1);
        }
    }

    if (intflag)
    {
        result = addint(varmap, vars);
    }
    else if (listflag)
    {
        result = addlist(varmap, vars);
    }
    else
    {
        cout << "Type mismatch cannot add a list to an int" << endl;
        cout << "Line: " << line_pos << endl;
        cout << line << endl;
        exit(1);
    }


    return result;

}


// function assigns variables values and passes to addition if necessary
map<string, string> assignVar(map<string, string> varmap, string line)
{
    map<string, string> result_varmap = varmap; // this resulting var map is a shallow copy of the original map that we will mutate and return
    string result; // this value will hold the result int or array that needs to be assigned into the varmap

    int eq = line.find("=");

    // break the line into its variable and value the variable should equal to
    string varname = line.substr(0, eq);
    varname = trim(varname);
    string value = line.substr(eq + 1, line.length() - eq);
    value = trim(value);

    if (regex_search(value, m, addoperator)) // if the value contains an add opperation then perfrom that operation
    {
        result = addOperate(varmap, value);
    }
    else if (regex_search(value, m, isvar)) // if the value is a variable that already exists in the map then we can assign it
    {
        if (varmap.count(value))
        {
            result = varmap[value];
        }
        else
        {
            cout << "Variable does not exist" << endl;
            cout << "Line: " << line_pos << endl;
            cout << value << endl;
            exit(1);
        }
    }
    else if (regex_search(value, m, isvarindex)) // if the value is something like l1[1] check if it exists
    {
        pair<string, int> vname_vindex = parsevarindex(varmap, value);
        if (varmap.count(vname_vindex.first))
        {
            vector<int> intarray = stringtoIntArray(varmap, varmap[vname_vindex.first]);
            if (intarray.size() > vname_vindex.second)
            {
                result = to_string(intarray.at(vname_vindex.second));
            }
            else
            {
                cout << "List out of bounds exception" << endl;
                cout << "Line: " << line_pos << endl;
                cout << line << endl;
                cout << value << endl;
                exit(1);
            }
        }
        else
        {
            cout << "Variable does not exist" << endl;
            cout << "Line: " << line_pos << endl;
            cout << line << endl;
            cout << value << endl;
            exit(1);
        }
    }
    else if (regex_search(value, m, listcomprehension)) // if the value is something with list comprehension
    {
        result = RetrieveListCompFunc(varmap, value);
    }
    else if (regex_search(value, m, isnum))// if the value is an int then we can just assign it
    {
        result = value;
    }
    else if (regex_search(value, m, islist)) // if the value is a list then we can assign it
    {
        result = intarraytoString(stringtoIntArray(varmap, value));
    }
    else if (regex_search(value, m, length))
    {
        result = len(varmap, value);
    }
    else if (regex_search(value, m, isfunccall))
    {
        result = funcCall(varmap, value);
    }

    if (regex_search(varname, m, isvarindex)) // if the value we are assigning to is an index in a list check if the spot exists
    {
        pair<string, int> vname_vindex = parsevarindex(varmap, varname);
        if (varmap.count(vname_vindex.first))
        {
            vector<int> intarray = stringtoIntArray(varmap, varmap[vname_vindex.first]);
            if (intarray.size() > vname_vindex.second)
            {
                intarray.at(vname_vindex.second) = stoi(result);
                result = intarraytoString(intarray);
            }
            else
            {
                cout << "List out of bounds exception" << endl;
                cout << "Line: " << line_pos << endl;
                cout << line << endl;
                cout << varname << endl;
                exit(1);
            }
        }
        else
        {
            cout << "Variable does not exist" << endl;
            cout << "Line: " << line_pos << endl;
            cout << line << endl;
            cout << varname << endl;
            exit(1);
        }
        result_varmap[vname_vindex.first] = result;
    }
    else if(regex_search(varname, m ,listcomprehension)) // if we are assigning to a list that has list comp it
    {
        result = AssignListCompFunc(varmap, varname, stringtoIntArray(varmap, result));
        result_varmap[varname.substr(0, varname.find_first_of('['))] = result;
    }
    else if (regex_search(varname, m, isvar))
    {
        result_varmap[varname] = result;
    }
    else
    {
        cout << "Cannot assign value to variable type" << endl;
        cout << "Line: " << line_pos << endl;
        cout << line << endl;
        cout << varname << endl;
        exit(1);

    }



    return result_varmap;
}

// This function will create a funcmap entry of a python function
map<string, list<string>> createFunc(map<string, list<string>> funcmap, string line, list<string> funclines)
{
    map<string, list<string>> result_funcmap = funcmap;
    string definition = trim(line);
    definition = definition.substr(4, definition.size() - 4);
    int open_paren = definition.find_first_of("(");
    int closing_paren = definition.find_first_of(")");

    string funcname = definition.substr(0, open_paren);
    string args = "ARGS: " + definition.substr(open_paren + 1, closing_paren - open_paren - 1);
    funclines.push_front(args);


    result_funcmap[funcname] = funclines;

    return result_funcmap;
}


// pass an if statement and check its conditional and return if its true (1) or false (0) or invalid(-1)
int ifevaluate(map<string, string> varmap, string if_line)
{
    string line = trim(if_line);
    line = line.substr(2, line.size() - 2); // remove if at the start of statement
    line = trim(line);
    string vars[2];
    int cond = -1;     // 0: ==
                       // 1: !=
                       // 2: <=
                       // 3: >=
                       // 4: <
                       // 5: >

    if (line.at(line.size() - 1) != ':') // if ":" is not present at the end it is invalid if it exists remove it and continue
    {
        cout << "If statement improperly formatted" << endl;
        cout << "Line: " << line_pos << endl;
        cout << if_line << endl;
        exit(1);
    }
    else
    {
        line = line.substr(0, line.size() - 1);
    }

    if (regex_search(line, m, parenbool) || regex_search(line, m, normbool))
    {
        int first;
        line = trim(line);
        if (regex_search(line, m, parenbool))
        { 
            line = line.substr(1, line.size() - 2); // remove the parenthesis
            int first;
        }

        // determine what our comparator is
        if (line.find_first_of('!') != -1)
        {
            first = line.find_first_of('!');
            cond = 1;
        }
        else if (line.find_first_of('<') != -1)
        {
            first = line.find_first_of('<');
            if (line.at(first + 1) != '=')
            {
                cond = 4;
            }
            else
            {
                cond = 2;
            }

        }
        else if (line.find_first_of('>') != -1)
        {
            first = line.find_first_of('>');
            if (line.at(first + 1) != '=')
            {
                cond = 5;
            }
            else
            {
                cond = 3;
            }
        }
        else
        {
            first = line.find_first_of('=');
            cond = 0;
        }

        // find our values to compare and get what they are
        vars[0] = trim(line.substr(0, first));
        int last = first + 1;
        if (cond != 5 && cond != 4)
        {
            last++;
        }
        vars[1] = trim(line.substr(last, line.size() - last + 1));

        
        if (regex_search(vars[0], m, isvar))
        {
            if (varmap.count(vars[0]))
            {
                vars[0] = varmap[vars[0]];
            }
            else
            {
                cout << "Variable does not exist" << endl;
                cout << "Line: " << line_pos << endl;
                cout << if_line << endl;
                cout << vars[0] << endl;
                exit(1);
            }

        }
        else if (regex_search(vars[0], m, isnum))
        {
            // do nothing
        }
        else if (regex_search(vars[0], m, isvarindex))
        {
            pair<string, int> vname_vindex = parsevarindex(varmap, vars[0]);
            if (varmap.count(vname_vindex.first))
            {
                vector<int> intarray = stringtoIntArray(varmap, varmap[vname_vindex.first]);
                if (intarray.size() > vname_vindex.second)
                {
                    vars[0] = to_string(intarray.at(vname_vindex.second));
                }
                else
                {
                    cout << "List out of bounds exception" << endl;
                    cout << "Line: " << line_pos << endl;
                    cout << if_line << endl;
                    cout << vars[0] << endl;
                    exit(1);
                }

            }
            else
            {
                cout << "Variable does not exist" << endl;
                cout << "Line: " << line_pos << endl;
                cout << if_line << endl;
                cout << vars[0] << endl;
                exit(1);
            }
        }
        else if (regex_search(vars[0], m, isfunccall))
        {
            vars[0] = funcCall(varmap, vars[0]);
        }
        else
        {
            cout << "Invalid declaration" << endl;
            cout << "Line: " << line_pos << endl;
            cout << if_line << endl;
            cout << vars[0] << endl;
            exit(1);
        }

        if (regex_search(vars[1], m, isvar))
        {
            if (varmap.count(vars[1]))
            {
                vars[1] = varmap[vars[1]];
            }
            else
            {
                cout << "Variable does not exist" << endl;
                cout << "Line: " << line_pos << endl;
                cout << if_line << endl;
                cout << vars[0] << endl;
                exit(1);
            }

        }
        else if (regex_search(vars[1], m, isnum))
        {
            // do nothing
        }
        else if (regex_search(vars[1], m, isvarindex))
        {
            pair<string, int> vname_vindex = parsevarindex(varmap, vars[1]);
            if (varmap.count(vname_vindex.first))
            {
                vector<int> intarray = stringtoIntArray(varmap, varmap[vname_vindex.first]);
                if (intarray.size() > vname_vindex.second)
                {
                    vars[1] = to_string(intarray.at(vname_vindex.second));
                }
                else
                {
                    cout << "List out of bounds exception" << endl;
                    cout << "Line: " << line_pos << endl;
                    cout << if_line << endl;
                    cout << vars[1] << endl;
                    exit(1);
                }

            }
            else
            {
                cout << "Variable does not exist" << endl;
                cout << "Line: " << line_pos << endl;
                cout << if_line << endl;
                cout << vars[1] << endl;
                exit(1);
            }
        }
        else if (regex_search(vars[1], m, isfunccall))
        {
            vars[1] = funcCall(varmap, vars[1]);
        }
        else
        {
            cout << "Invalid declaration" << endl;
            cout << "Line: " << line_pos << endl;
            cout << if_line << endl;
            cout << vars[1] << endl;

            exit(1);
        }
    }
    else
    {
        cout << "If statement improperly formatted or Conditional is missing" << endl;
        cout << "Line: " << line_pos << endl;
        cout << if_line << endl;
        exit(1);
    }


        

    if (cond == 0) // ==
    {
        if (vars[0].compare(vars[1]) == 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (cond == 1) // !=
    {
        if (vars[0].compare(vars[1]) != 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (cond == 2) // <=
    {
        int v1 = stoi(vars[0]);
        int v2 = stoi(vars[1]);
        if (v1 <= v2)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (cond == 3) // >=
    {
        int v1 = stoi(vars[0]);
        int v2 = stoi(vars[1]);
        if (v1 >= v2)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (cond == 4) // <
    {
        int v1 = stoi(vars[0]);
        int v2 = stoi(vars[1]);
        if (v1 < v2)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (cond == 5) // >
    {
        int v1 = stoi(vars[0]);
        int v2 = stoi(vars[1]);
        if (v1 > v2)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return -1;


}

// Whenever we call a function pass the args over to the function in its own varmap then run its lines
string funcCall(map<string, string> varmap, string funcline)
{
    map<string, string> func_varmap; // create a var map only in scope of the function call
    int open_paren = funcline.find_first_of("(");
    int closing_paren = funcline.find_last_of(")");

    string funcname = funcline.substr(0, open_paren);
    string result;

    // Check if the function exists and get the args and assign them to our func_varmap
    if (funcmap.count(funcname))
    {
        string args = "ARGS: " + funcline.substr(open_paren + 1, closing_paren - open_paren - 1);
        string funcargs = *funcmap[funcname].begin();
        args = args.substr(6, args.size() - 6);
        funcargs = funcargs.substr(6, funcargs.size() - 6);
        string funcarg;
        string arg;
        int argcomma = args.find(",");
        int funcargcomma = funcargs.find(",");
        list<string> arglist;
        list<string> funcarglist;
        if (args == "" && funcargs == "")
        {
            // do nothing
        }
        else if (argcomma == -1 && funcargcomma == -1)
        {
            args = trim(args);
            funcargs = trim(funcargs);
            arglist.push_back(args);
            funcarglist.push_back(funcargs);
        }
        while (argcomma != -1 && funcargcomma != -1)
        {
            arg = args.substr(0, argcomma);
            arg = trim(arg);
            funcarg = funcargs.substr(0, funcargcomma);
            funcarg = trim(funcarg);

            arglist.push_back(arg);
            funcarglist.push_back(funcarg);


            args = args.substr(argcomma + 1, args.length() - argcomma);
            argcomma = args.find(",");
            funcargs = funcargs.substr(funcargcomma + 1, funcargs.length() - funcargcomma);
            funcargcomma = funcargs.find(",");

            if (argcomma == -1 && funcargcomma == -1)
            {
                args = trim(args);
                funcargs = trim(funcargs);
                arglist.push_back(args);
                funcarglist.push_back(funcargs);

            }
        }
        if (argcomma != funcargcomma)
        {
            cout << "Incorrect number of arguments in Function Call" << endl;
            cout << "Line: " << line_pos << endl;
            cout << funcline << endl;
            exit(1);
        }
        

        list<string>::iterator i;
        for (i = arglist.begin(); i != arglist.end(); i++)
        {
            arg = *i;

            if (regex_search(arg, m, isvar))
            {
                if (varmap.count(arg))
                {
                    *i = varmap[arg];
                }
                else
                {
                    cout << "Variable does not exist" << endl;
                    cout << "Line: " << line_pos << endl;
                    cout << funcline << endl;
                    cout << arg << endl;
                    exit(1);
                }
            }
            else if (regex_search(arg, m, isvarindex))
            {
                pair<string, int> vname_vindex = parsevarindex(varmap, arg);
                if (varmap.count(vname_vindex.first))
                {
                    vector<int> intarray = stringtoIntArray(varmap, varmap[vname_vindex.first]);
                    if (intarray.size() > vname_vindex.second)
                    {
                        *i = to_string(intarray.at(vname_vindex.second));
                    }
                    else
                    {
                        cout << "List out of bounds exception" << endl;
                        cout << "Line: " << line_pos << endl;
                        cout << funcline << endl;
                        cout << arg << endl;
                        exit(1);
                    }
                }
                else
                {
                    cout << "Variable does not exist" << endl;
                    cout << "Line: " << line_pos << endl;
                    cout << funcline << endl;
                    cout << arg << endl;
                    exit(1);
                }
            }
            else if (regex_search(arg, m, isnum))// if the value is an int then we can just assign it
            {
                // do nothing
            }
            else if (regex_search(arg, m, islist)) // if the value is a list then we can assign it
            {
                // do nothing
            }
            else if (regex_search(arg, m, isfunccall))
            {
                *i = funcCall(varmap, arg);
            }
            else
            {
                cout << "Invalid Type (Use ints and lists only)" << endl;
                cout << "Line: " << line_pos << endl;
                cout << funcline << endl;
                cout << arg << endl;
                exit(1);
            }

        }
        list<string>::iterator a;
        list<string>::iterator b;
        for (a = arglist.begin(), b = funcarglist.begin(); a != arglist.end(); a++, b++)
        {
            string assignment = *b + " = " + *a;
            func_varmap = assignVar(func_varmap, assignment);

        }
        list<string>::iterator it;
        it = funcmap[funcname].begin();
        it++;
        list<string> lines(it, funcmap[funcname].end());
        result = PassLines(func_varmap, lines, true);

    }
    else
    {
        cout << "Function is not defined" << endl;
        cout << "Line: " << line_pos << endl;
        cout << funcline << endl;
        exit(1);
    }



    return result;
}

// When we need to return a value from a function
string returnFunc(map<string, string> varmap, string returnline)
{
    string line = returnline;
    int pos = line.find_first_of(" ");
    string returnobj = line.substr(pos, line.size() - pos);
    returnobj = trim(returnobj);

    if (regex_search(returnobj, m, isvar))
    {
        if (varmap.count(returnobj))
        {
            returnobj = varmap[returnobj];
        }
        else
        {
            cout << "Variable does not exist" << endl;
            cout << "Line: " << line_pos << endl;
            cout << returnline << endl;
            cout << returnobj << endl;
            exit(1);
        }
    }
    else if (regex_search(returnobj, m, isvarindex))
    {
        pair<string, int> vname_vindex = parsevarindex(varmap, returnobj);
        if (varmap.count(vname_vindex.first))
        {
            vector<int> intarray = stringtoIntArray(varmap, varmap[vname_vindex.first]);
            if (intarray.size() > vname_vindex.second)
            {
                returnobj = to_string(intarray.at(vname_vindex.second));
            }
            else
            {
                cout << "List out of bounds exception" << endl;
                cout << "Line: " << line_pos << endl;
                cout << returnline << endl;
                cout << returnobj << endl;
                exit(1);
            }
        }
        else
        {
            cout << "Variable does not exist" << endl;
            cout << "Line: " << line_pos << endl;
            cout << returnline << endl;
            cout << returnobj << endl;
            exit(1);
        }
    }
    else if (regex_search(returnobj, m, isnum))// if the value is an int then we can just pass it
    {
        // do nothing
    }
    else if (regex_search(returnobj, m, islist)) // if the value is a list then we can pass it
    {
        // do nothing
    }
    else if (regex_search(returnobj, m, addoperator))
    {
        returnobj = addOperate(varmap, returnobj);
    }
    else
    {
        cout << "Invalid Type (Use ints and lists only)" << endl;
        cout << "Line: " << line_pos << endl;
        cout << returnline << endl;
        cout << returnobj << endl;
        exit(1);
    }

    return returnobj;
    
}


string PassLines(map<string, string> varmap, list<string> lines, bool infunc)
{
    list<string> pylines = lines;
    string currLine;
    int expected_indent = 0; // this will tell us what the expected indent of the next line should be
    bool ifflag = false; // this is used to determine whether or not we should ignore the next lines of indented code
    bool oneblockrule = false; // if and else statements must have one block of indented code
    int ifresult = -1;
    vector<int> ifresults; // hold our if results


    list<string>::iterator i;
    for (i = pylines.begin(); i != pylines.end(); i++)
    {
        currLine = *i;
        if (!infunc) // We don't want to update our line pos while inside a function we can tell the user what is wrong in the lines
        {
            line_pos += 1;
        }

        int indent_pos = currLine.find_first_not_of(' ');
        if (regex_search(currLine, m, indent))
        {
            if (indent_pos == -1) // line is blank with indents
            {
                currLine = "";
            }
            else if (indent_pos % 4 != 0) // spacing is not by 4s
            {
                cout << "Invalid Spacing" << endl;
                cout << "Line: " << line_pos << endl;
                cout << currLine << endl;
                exit(1);
            }
            else if (expected_indent < indent_pos / 4)
            {
                cout << "Unexpected Indent" << endl;
                cout << "Line: " << line_pos << endl;
                cout << currLine << endl;
                exit(1);
            }
            else if (expected_indent > indent_pos / 4 && oneblockrule == true) // if and else statements require at least one line of code
            {
                cout << "Missing Indent Block" << endl;
                cout << "Line: " << line_pos << endl;
                cout << currLine << endl;
                exit(1);
            }
            else if (ifflag == false && indent_pos / 4 >= ifresults.size()) // ifflag is false then ignore lines but check for one block rules
            {
                currLine = trim(currLine);
                if (regex_search(currLine, m, ifopen) || regex_search(currLine, m, elseopen))
                {
                    expected_indent = indent_pos / 4 + 1;
                    oneblockrule = true;
                }
                else
                {
                    oneblockrule = false;
                }
                currLine = "";
            }
            else if (expected_indent > indent_pos / 4 && oneblockrule == false)
            {
                currLine = trim(currLine);
                if (currLine != "" && ifresults.size() > 0)
                {
                    if (regex_search(currLine, m, elseopen))
                    {
                        ifresults.erase(ifresults.begin() + (indent_pos / 4 + 1), ifresults.end());
                    }
                    else
                    {
                        ifresults.erase(ifresults.begin() + (indent_pos / 4), ifresults.end());
                    }
                }
                expected_indent = indent_pos / 4;

            }
            else
            {
                currLine = trim(currLine);
                oneblockrule = false;
                
            }

        }
        else if (expected_indent > 0 && oneblockrule == false)
        {
            currLine = trim(currLine);
            if (currLine != "" && ifresults.size() > 0)
            {
                if (regex_search(currLine, m, elseopen))
                {
                    ifresults.erase(ifresults.begin() + (indent_pos / 4 + 1), ifresults.end());
                }
                else
                {
                    ifresults.erase(ifresults.begin() + (indent_pos / 4), ifresults.end());
                }
            }
            expected_indent = indent_pos / 4;

        }
        else if(expected_indent > 0 && oneblockrule == true)
        {
            cout << "Missing Indent Block" << endl;
            cout << "Line: " << line_pos << endl;
            cout << currLine << endl;
            exit(1);
        }





        if (regex_search(currLine, m, ifopen))
        {
            ifresult = ifevaluate(varmap, currLine);
            ifresults.push_back(ifresult);
            if (ifresult == -1)
            {
                exit(1);
            }
            else if (ifresult == 0)
            {
                ifflag = false;

            }
            else
            {
                ifflag = true;
            }
            oneblockrule = true;
            expected_indent += 1;

        }
        else if (regex_search(currLine, m, elseopen))
        {
            ifresult = ifresults.back();
            if (ifresult == -1)
            {
                exit(1);
            }
            else if (ifresult == 0)
            {
                ifflag = true;
            }
            else if (ifresult == 1)
            {
                ifflag = false;
            }
            oneblockrule = true;
            expected_indent += 1;
        }
        else if (regex_search(currLine, m, comment))
        {
            // do nothing
        }
        else if (regex_search(currLine, m, printing))
        {
            print(varmap, currLine);
        }
        else if (regex_search(currLine, m, length))
        {
            len(varmap, currLine);
        }
        else if (regex_search(currLine, m, assignment)) // if a line contains a singular = sign then its an assignment operation
        {
            varmap = assignVar(varmap, currLine);
        }
        else if (regex_search(currLine, m, def))
        {
            expected_indent++;
            oneblockrule = true;
            list<string> funclines;
            i++;
            line_pos += 1;
            string templine = *i;
            int func_indent_pos = templine.find_first_not_of(' ');
            while (expected_indent <= func_indent_pos / 4 || func_indent_pos == -1)
            {
                if (func_indent_pos != -1)
                {
                    templine = templine.substr(4, templine.size() - 4);
                    funclines.push_back(templine);
                    line_pos += 1;
                    i++;
                    templine = *i;
                    func_indent_pos = templine.find_first_not_of(' ');
                    oneblockrule = false;
                }
                else
                {
                    line_pos += 1;
                    i++;
                    templine = *i;
                    func_indent_pos = templine.find_first_not_of(' ');
                }
            }
            if(expected_indent < func_indent_pos / 4 && oneblockrule == true)
            {
                cout << "Missing Indent Block" << endl;
                cout << "Line: " << line_pos << endl;
                cout << currLine << endl;
                exit(1);
            }
            else
            {
                expected_indent = func_indent_pos / 4;
            }
            i--;
            line_pos -= 1;

            
            
            funcmap = createFunc(funcmap, currLine, funclines);
        }
        else if (regex_search(currLine, m, isfunccall))
        {
            funcCall(varmap, currLine);
        }
        else if (regex_search(currLine, m, returncall))
        {
            return returnFunc(varmap, currLine);


            //Needs to be completed
        }
        else if (trim(currLine) == "")
        {
            // do nothing its a blank line
        }
        else if (currLine == "END")
        {
            // do nothing program will stop on its own

        }
        else
        {
            cout << "Unrecognized Line" << endl;
            cout << "Line: " << line_pos << endl;
            cout << currLine << endl;
            exit(1);
        }

    }

    return "";







}



int main(int argc, char** argv)
{

    if (argc < 2)
    {
        cout << "Too few arguments." << endl;
        cout << "Cmdline should have the format of ./minipython test1.py" << endl;
        return 1;
    }
    else if (argc > 2)
    {
        cout << "Too many arguments." << endl;
        cout << "Cmdline should have the format of ./minipython test1.py" << endl;
        return 1;
    }

    string inputFile = argv[1];


    ifstream ifs(inputFile);

    if (!ifs) // checks if inputfile has not been opened by the ifs
    {
        cout << "Input file " << inputFile << " was not found in the programs directory." << endl;
        cout << "Make sure file's name is correctly inputed and exists in the same directory as program" << endl;
        return 1;
    }

    string ln;
    list<string> pylines;
    while (getline(ifs, ln))
    {
        pylines.push_back(ln);
    }
    pylines.push_back("END");
    PassLines(varmap, pylines, false);

}