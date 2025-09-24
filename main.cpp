#include "staticlib.h"
using namespace std;

std::vector<char> spec = {
    '{', '}', '(', ')',   // скобки
    '[', ']',             // массивы
    ';', ',', '.',        // разделители
    '+', '-', '*', '/', '%', // арифметика
    '<', '>', '=', '!', '&', '|', '^', '~', // операторы сравнения/логики/побитовые
    ':', '?',             // тернарный оператор и метки
    '"' , '\'', '\\'      // строковые/символьные литералы и экранирование
};
std::vector<char> letSpec = {'}',')'};


bool isSpec(char c, const std::vector<char>& spec) {
    return std::find(spec.begin(), spec.end(), c) != spec.end();
}


std::string python_to_cpp(const std::string& code) {
    std::istringstream iss(code);
    std::ostringstream oss;
    std::string line;
    std::vector<int> indent_stack;
    indent_stack.push_back(0);
    while (std::getline(iss, line)) {

        if (line.empty()) {
            oss << "\n";
            continue;
        }

        int curr_indent = line.find_first_not_of(" \t");
        std::string trimmed = (curr_indent == std::string::npos) ? "" : line.substr(curr_indent);

        // Закрываем блоки, если отступ уменьшился
        while (curr_indent < indent_stack.back()) {
            indent_stack.pop_back();
            oss << std::string(indent_stack.back(), ' ') << "}\n";
        }

        if (trimmed.empty()) {
            oss << line << "\n";
            continue;
        }

        // Обработка классов
        if (trimmed.find("class ") == 0 && trimmed.back() == ':') {
            trimmed.back() = ' ';
            oss << std::string(curr_indent, ' ') << trimmed << "{\n";
            indent_stack.push_back(curr_indent + 4);
        }
        // Обработка функций/методов
        else if (trimmed.find("def ") == 0 && trimmed.back() == ':') {
            trimmed.back() = ' ';
            oss << std::string(curr_indent, ' ') << trimmed << "{\n";
            indent_stack.push_back(curr_indent + 4);
        }
        // Обработка if
        else if (trimmed.find("if ") == 0 && trimmed.back() == ':') {
            trimmed.back() = ' ';
            oss << std::string(curr_indent, ' ') << "if (" << trimmed.substr(3) << ") {\n";
            indent_stack.push_back(curr_indent + 4);
        }
        // Обработка while
        else if (trimmed.find("while ") == 0 && trimmed.back() == ':') {
            trimmed.back() = ' ';
            oss << std::string(curr_indent, ' ') << "while (" << trimmed.substr(6) << ") {\n";
            indent_stack.push_back(curr_indent + 4);
        }
        // Обработка for
        else if (trimmed.find("for ") == 0 && trimmed.back() == ':') {
            trimmed.pop_back();
            size_t in_pos = trimmed.find(" in ");
            if (in_pos != std::string::npos && in_pos > 4) {
                std::string var = trimmed.substr(4, in_pos - 4);
                std::string iterable = trimmed.substr(in_pos + 4);
                oss << std::string(curr_indent, ' ') << "for (auto " << var << " : " << iterable << ") {\n";
            } else {
                // если "in" нет — просто помещаем содержимое в скобки
                oss << std::string(curr_indent, ' ') << "for (" << trimmed.substr(4) << ") {\n";
            }
            indent_stack.push_back(curr_indent + 4);
        }

        // Остальное оставляем без изменений
        else {
            oss << line << "\n";
        }
    }

    // Закрываем оставшиеся блоки
    while (indent_stack.size() > 1) {
        indent_stack.pop_back();
        oss << std::string(indent_stack.back(), ' ') << "}\n";
    }

    return oss.str();
}


string to_CPP(const string &s) {
    string header = "#include \"/mnt/500GB/kazl/staticlib.h\"\n\n";
    string functionsVal;  // только include / using
    string body;      // всё остальное

    map<int,int> funcval;
    int tmpFuncValue[2];

    vector<string> classes = {};
    bool classFound = false;
    bool nowClassFound = false;
    vector<string> nowClassField= {};

    bool functionFind = false;
    string funcTab="";

    std::istringstream iss(s);
    std::string line;

    long long linee = 0;

    while (std::getline(iss, line)) {
        if (line.empty()) continue;

        if (line.find("import ")!=string::npos) {
            line.replace(line.find("import "),7,"#include \"");
            linee--;
            line+="\"";
        }

        // убираем пробелы слева и справа
        std::string trimmed = line;


        size_t pos = trimmed.find("def __init__");
        if (pos != std::string::npos && !classes.empty() && classFound) {
            // берём имя класса
            std::string className = classes.back();
            // заменяем всю строку "def __init__" на "ClassName"
            trimmed.replace(pos, 12, className);
            // теперь строка выглядит как "Test(int a) {"
        }

        pos = trimmed.find("def");
        if (pos != std::string::npos) {
            if (!classFound) {
                tmpFuncValue[0]=linee;
            }
            trimmed.replace(pos, 3, "Let");
            functionFind = true;
            funcTab = trimmed.substr(0, pos);
        }

        // безопасная проверка закрывающей скобки
        size_t closeBrace = trimmed.find("}");
        if (functionFind && closeBrace != std::string::npos &&
            funcTab == trimmed.substr(0, closeBrace))
        {
            if (!classFound) {
                linee+=1;
                tmpFuncValue[1]=linee;
                funcval.insert(std::pair<int,int>(tmpFuncValue[0],tmpFuncValue[1]));
            }
            trimmed.replace(closeBrace, 1, funcTab + "\treturn Let(0);\n\t}");
            functionFind = false;
        }


        int classIndex = line.find("class");
        if (classIndex != std::string::npos) {
            size_t openParen = line.find('(', classIndex);
            std::string className;
            if (openParen != std::string::npos) {
                className = line.substr(classIndex + 6, openParen - (classIndex + 6)); // 6 = длина "class "
            } else {
                className = line.substr(classIndex + 6);
            }
            className.erase(std::remove_if(className.begin(), className.end(), ::isspace), className.end());
            classes.push_back(className);
            nowClassFound = true;
            classFound=true;
            tmpFuncValue[0] = linee;
        }



        // include / using
        if (trimmed.rfind("#include", 0) == 0 ||
            trimmed.rfind("using", 0) == 0)
        {
            functionsVal += line + "\n";
        } else {
            // проверка: нужно ли добавлять ;
            bool needsSemicolon =
                trimmed.find('{') == std::string::npos &&   // не начинаем блок
                trimmed.find('}') == std::string::npos &&   // не закрываем блок
                !trimmed.empty() &&
                trimmed.back() != ';';
            if (classFound && trimmed[0]=='}') {
                classFound = false;
                needsSemicolon = true;

                string res = "";
                for (string i :nowClassField) {
                    res += "{\""+i + "\", "+i+"},";
                }
                res[res.length()-1] = ' ';


                trimmed="   REFLECTABLE({" +res+ "})\t\n\t" + trimmed;
                nowClassField={};
                linee++;
                tmpFuncValue[1]=linee;
                funcval.insert(std::pair<int,int>(tmpFuncValue[0],tmpFuncValue[1]));
            }

            if (trimmed.find('=')!= std::string::npos && trimmed.find("this")>trimmed.find('=') && trimmed.find(" ")!=string::npos &&trimmed[(trimmed.find(" ")+1)]=='=') {
                int pos = trimmed.find('=');

                // идём влево от '=' до ближайшего пробела или спецсимвола
                int start = pos - 1;
                while (start >= 0 && !isspace(trimmed[start]) && !isSpec(trimmed[start], spec)) {
                    start--;
                }
                start++; // встали на начало имени

                string before = trimmed.substr(0, start-2);
                string after  = trimmed.substr(start-2);



                trimmed = before + "Let " + after;
                if (classFound) {
                    nowClassField.push_back(after.substr(0, after.find('=')));
                }


            }
            if (nowClassFound) {
                size_t openParen = trimmed.find('(');
                size_t closeParen = trimmed.find(')');
                if (openParen != std::string::npos && closeParen != std::string::npos) {
                    std::string inside = trimmed.substr(openParen + 1, closeParen - openParen - 1);
                    if (inside.empty() || inside.find_first_not_of(" \t") == std::string::npos || inside==" ") {
                        // пустые скобки → заменяем на : public BaseClass
                        trimmed = trimmed.substr(0, openParen) + " : public BaseClass<" + classes.back() +"> " + trimmed.substr(closeParen + 1);
                        nowClassFound = false;
                    }
                }
            }

            body += "\t" + trimmed + (needsSemicolon ? ";" : "") + "\n";

        }
        linee++;
    }

    // Разделяем body на строки
    vector<string> bodyLines;
    istringstream bodyStream(body);
    string tmpLine;
    while (getline(bodyStream, tmpLine)) {
        bodyLines.push_back(tmpLine);
    }

    string members;    // переменные и методы класса Main
    string finalBody;  // тело sub()

    for (size_t i = 0; i < bodyLines.size(); i++) {
        bool isFuncLine = false;

        // Проверяем, принадлежит ли текущая линия какой-либо функции из funcval
        for (auto &kv : funcval) {
            if ((int)i >= kv.first && (int)i <= kv.second) {
                cout<<kv.first<<"="<<kv.second<<"\n";
                isFuncLine = true;
                break;
            }
        }

        if (isFuncLine) {
            // Функции оставляем в теле sub(), добавляем одну табуляцию
            finalBody += "" + bodyLines[i] + "\n";
            cout<<isFuncLine<<bodyLines[i]<<"\n";
        } else {
            // Всё остальное переносим в members с одной табуляцией
            cout<<isFuncLine<<bodyLines[i]<<"\n";
            members += "\t" + bodyLines[i] + "\n";
        }
    }

    swap(members,finalBody);
    // Теперь members и finalBody можно вставлять в финальный код
    string result = header + functionsVal + "\n"
                    "class Main{\npublic:\n" + members + "\n"
                    "    int sub(){\n" + finalBody + "        return 0;\n    }\n"
                    "};\n\n"
                    "int main(){\n"
                    "    Main m;\n"
                    "    m.sub();\n"
                    "    return 0;\n}";

    return result;
}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "usage: kazl <file.kazl>";
        return 1;
    }

    cout<<argv[1]<<endl;

    string sourceCode = read_file(argv[1]);   // читаем содержимое kazl-файла
    string generatedCode = to_CPP(python_to_cpp(sourceCode)); // конвертим в C++

    string baseName = argv[1];
    if (baseName.find(".kazl") != string::npos) {
        baseName = baseName.substr(0, baseName.find(".kazl")); // убираем расширение
    }

    string cppFile = baseName + ".cpp";
    string execFile = baseName + ".exec";

    write_file(cppFile, generatedCode);

    string cmd = "g++ -std=c++20 " + cppFile + " -o " + execFile;
    system(cmd.c_str());

    return 0;
}
