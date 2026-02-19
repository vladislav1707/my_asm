#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <bitset>

using namespace std;

vector<vector<string>> mnemonic_table = {
    // логические и арифметические команды (8 бит)
    {"OR", "01000000"},
    {"NOR", "01000001"},
    {"AND", "01000010"},
    {"NAND", "01000011"},
    {"ADD", "01000100"},
    {"SUB", "01000101"},
    {"XOR", "01000110"},
    {"SHR", "01000111"},

    // команда IMM (2 бита)
    {"IMM", "00"},

    // команда MOV (2 бита)
    {"MOV", "10"},

    // регистры и порты (3 бита)
    {"R0", "000"},
    {"R1", "001"},
    {"R2", "010"},
    {"R3", "011"},
    {"R4", "100"},
    {"R5", "101"},
    {"INP1", "110"},
    {"OUT1", "110"},
    {"INP2", "111"},
    {"OUT2", "111"},

    // команды условных переходов (3 бита)
    {"JMP", "11000000"},
    {"JEQ", "11000001"},
    {"JZ", "11000001"},
    {"JNE", "11000010"},
    {"JNS", "11000010"},
    {"JLT", "11000011"},
    {"JLE", "11000100"},
    {"JGT", "11000101"},
    {"JGE", "11000110"},
    {"NOP", "11000111"}};

// вроде функция уже не нужна но я не уверен
// string trim(const string &s)
//{
//    size_t start = s.find_first_not_of(" \t\r\n"); // первый символ который не \t, \r, или \n
//    if (start == string::npos)                     // если не нашел нормальных символов то строка только из пробелов
//    {
//        return ""; // строка состоит только из пробелов
//    }
//    size_t end = s.find_last_not_of(" \t\r\n"); // последний символ который не \t, \r, или \n
//    return s.substr(start, end - start + 1);    // пробелы отступы и тд вначале строки удаляются
//}

string str_to_lower(string s)
{
    string result = s;
    transform(result.begin(), result.end(), result.begin(), // transform из algorithm, читает с начала и до конца, и записывает туда же, в конце лямбда выражение
              [](unsigned char c)
              { return static_cast<unsigned char>(tolower(c)); }); // static_cast преобразует char в беззнаковый
    return result;
}

bool is_num(string s)
{
    size_t start = 0; // определяет где начнется цикл

    if (s[0] == '-') // минус вначале можно
    {
        if (s.length() == 1) // если после минуса ничего нет то не число
        {
            return false;
        }
        start = 1; // если минус был, начало цикла проверки на буквы будет после минуса
    }

    for (size_t i = start; i < s.length(); ++i) // если минус есть то его не учитывать
    {
        // если 1 из символов не цифра то выдать false
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
        {
            return false;
        }
    }

    return true;
}

string dec_to_bin(int dec)
{
    if (dec < -32 || dec > 31) // при выходе за диапазон выдается ошибка
    {
        throw out_of_range("ERROR: Value " + to_string(dec) + " out of 6-bit signed range (-32..31)");
    }
    bitset<6> result = dec;    // превращение в двоичное число
    return result.to_string(); // вывод ввиде строки
}

int main(int argc, char **args)
{
    // myasm вход выход формат(hex, bin, dec)
    // пример:
    // my_asm counter.txt counter_bin.txt bin
    if (argc != 4)
    {
        cerr << "ERROR: Incorrect number of arguments";
        return 1;
    }

    // переменные с аргументами
    string input_file = args[1];
    string output_file = args[2];
    string format = args[3]; // hex, bin, dec

    if (format != "hex" && format != "bin" && format != "dec")
    {
        cerr << "ERROR: Incorrect format";
        return 1;
    }

    ifstream in;  // входной поток
    ofstream out; // выходной поток

    in.open(input_file);
    out.open(output_file);
    if (!in)
    {
        cerr << "ERROR: Failed to open input file" << output_file << "\n";
        return 1;
    }
    if (!out)
    {
        cerr << "ERROR: Failed to open output file " << output_file << "\n";
        return 1;
    }

    vector<string> lines; // пример: ["IMM 1", "MOV R0 R1", "label cycle", "ADD", "MOV R3 R2", "IMM cycle", "JMP"]
    string line;

    while (getline(in, line)) // line получает свое значение тут, цикл идет пока значение получается
    {
        size_t commentPos = line.find('#');
        if (commentPos != string::npos) // если есть # то:
        {
            line = line.substr(0, commentPos); // оставить только от начала строки заканчивая началом комментария(комментарий обрезан и # тоже)
        }
        if (!line.empty()) // если строка не пустая
        {
            lines.push_back(line); // добавить строку в lines
        };
    }

    vector<vector<string>> parsed_program; // пример: [["IMM", "1"], ["MOV", "R0", "R1"], ...]

    for (size_t i = 0; i < lines.size(); i++)
    {
        stringstream sstream(lines[i]); // stringstream для каждого элемента lines
        string token;                   // полученная команда или аргумент

        parsed_program.push_back(vector<string>());
        while (sstream >> token) // проверяем есть ли еще слова и записываем слово из sstream в переменную word
        {
            parsed_program[i].push_back(token); // команда или аргумент в конец вектора с командой который находится в векторе с командами
        }
    }

    vector<vector<string>> labels; // вектор с метками, но тут так же будут записаны и константы

    for (size_t i = 0; i < parsed_program.size();)
    {
        if (!parsed_program[i].empty() && str_to_lower(parsed_program[i][0]) == "label") // если строка не пуста и равна "label"
        {
            if (parsed_program[i].size() < 2) // если недостаточно аргументов у label выдать ошибку
            {
                cerr << "ERROR: Label without name at line: " << i + 1 << endl;
                return 1;
            }
            string label_name = parsed_program[i][1];         // имя метки
            labels.push_back({label_name, to_string(i)});     // добавить в вектор вектор с именем и значением метки
            parsed_program.erase(parsed_program.begin() + i); // удалить прочтенный label из parsed_program
        }
        else if (!parsed_program[i].empty() && str_to_lower(parsed_program[i][0]) == "const") // если строка не пуста и равна "const"
        {
            if (parsed_program[i].size() < 3) // если недостаточно аргументов у const выдать ошибку
            {
                cerr << "ERROR: Const without name at line: " << i << endl;
                return 1;
            }
            string const_name = parsed_program[i][1];             // имя константы
            labels.push_back({const_name, parsed_program[i][2]}); // добавить в вектор вектор с именем и значением константы
            parsed_program.erase(parsed_program.begin() + i);     // удалить прочтенную константу из parsed_program
        }
        else
        {
            // Если это не метка и не константа, просто переходим к следующей строке.
            i++;
        }
    }

    for (size_t i = 0; i < parsed_program.size(); i++) // пройтись по командам
    {
        for (size_t j = 0; j < parsed_program[i].size(); j++) // пройтись по каждому элементу команды
        {
            for (size_t k = 0; k < labels.size(); k++) // пройтись по каждому элементу labels и проверить нет ли совпадения
            {
                // сравнить регистронезависимо все аргументы команды и название всех меток(названия находятся в 0 элементе вектора)
                // проверить то что команда не пустая
                if (!parsed_program[i].empty() && str_to_lower(parsed_program[i][j]) == str_to_lower(labels[k][0]))
                {
                    parsed_program[i][j] = str_to_lower(labels[k][1]); // если совпадение есть то заменить на значение константы/метки
                }
            }
        }
    }

    vector<string> binary_code;

    for (int i = 0; i < parsed_program.size(); i++) // для каждой команды
    {
        binary_code.push_back("");
        for (int j = 0; j < parsed_program[i].size(); j++) // для каждого элемента команды
        {
            bool found = false;
            for (int k = 0; k < mnemonic_table.size(); k++) // для каждой мнемоники
            {
                if (str_to_lower(parsed_program[i][j]) == str_to_lower(mnemonic_table[k][0])) // если команда/аргумент = обозначению
                {
                    binary_code[i].append(mnemonic_table[k][1]); // добавить значение мнемоники в двоичном коде
                    found = true;
                    break;
                }
            }
            if (found)
            {
                // обработано, не надо делать ничего
            }
            else if (is_num(parsed_program[i][j])) // если число
            {
                try
                {
                    int num = stoi(parsed_program[i][j]);
                    binary_code[i].append(dec_to_bin(num)); // dec_to_bin уже проверяет диапазон
                }
                catch (const out_of_range &e)
                {
                    cerr << e.what() << " at line " << i + 1 << endl;
                    return 1;
                }
            }
            else // если не номер и мнемоника не найдена вывести ошибку
            {
                cerr << "ERROR: Mnemonic not found";
                return 1;
            }
        }
        if (binary_code[i].length() != 8) // при неверной длине команды
        {
            cerr << "ERROR: Invalid command length at line: " << i + 1 << endl;
            return 1;
        }
    }
    if (binary_code.size() > 256) // обработка неразрешенного размера
    {
        cerr << "ERROR: Code size(" << binary_code.size() << " bytes) is larger than allowed(256 bytes)";
        return 1;
    }

    // запись в файл в выбранном формате(bin, dec, hex) с заполнением пустого пространства NOP(холостым ходом)

    bool is_success = false;
    if (format == "bin") // если выбран двоичный код
    {
        if (!out.is_open()) // если не открылось то выдать ошибку
        {
            cerr << "ERROR: Unable to open or create output file";
            return 1;
        }
        for (int i = 0; i < binary_code.size(); i++) // записать в файл двоичный код
        {
            out << binary_code[i] << "\n"; // писать каждую команду с новой строки
        }
        // заполнить остальную память NOP(холостой ход)
        for (int i = binary_code.size() + 1; i < 256 - binary_code.size(); i++)
        {
            out << "11000111" << "\n";
        }
        is_success = true;
    }
    if (format == "dec") // если выбран десятичный код
    {
        if (!out.is_open()) // если не открылось то выдать ошибку
        {
            cerr << "ERROR: Unable to open or create output file";
            return 1;
        }
        for (int i = 0; i < binary_code.size(); i++) // записать в файл десятичный код
        {
            // Преобразуем двоичную строку в десятичное число
            std::bitset<8> byte(binary_code[i]); // преобразовать string с двоичным кодом в bitset из 1 байта(8 бит)
            out << byte.to_ulong() << "\n";      // преобразовать байт в unsigned long(беззнаковый длинный) и писать в файл
        }
        // заполнить остальную память NOP(холостой ход)
        for (int i = binary_code.size() + 1; i < 256 - binary_code.size(); i++)
        {
            out << "199" << "\n";
        }
        is_success = true;
    }

    // после успешной записи закрыть файлы
    out.close();
    in.close();
    if (is_success)
    {
        std::cout << "File has been written" << std::endl;
    }
    return 0;

    // TODO: надо добавить заполнение NOP до 256 байт, а еще новые форматы: dec и hex
}