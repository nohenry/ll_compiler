native void write_int(int64 i);
native void write_string(string s);
native string read_entire_file(string filepath);

struct Lexer {
    string source;
    uint64 cursor;
}

bool has_next(Lexer* lexer) {
    while lexer.cursor < lexer.source.length {
        char a = lexer.source[lexer.cursor];
        if (a != 0x20 && a != 0xa && a != 0xd) break;

        lexer.cursor += 1;
    }
    return lexer.cursor < lexer.source.length;
}

int64 next(Lexer* lexer) {
    lexer.has_next();

    bool negative = false;
    if lexer.source[lexer.cursor] == 0x4c {
        negative = true;
    } else if lexer.source[lexer.cursor] == 0x52 {
        negative = false;
    }
    lexer.cursor += 1;

    int64 result = 0;
    while lexer.source[lexer.cursor] >= 48 && lexer.source[lexer.cursor] <= 57 {
        result *= 10;
        result += cast(int64)lexer.source[lexer.cursor] - 48;
        lexer.cursor += 1;
    }

    if negative {
        return -result;
    } else {
        return result;
    }
}

void main() {
    Lexer lexer;
    lexer.cursor = 0;
    lexer.source = read_entire_file("tests/aoc/day1.txt");

    int64 cursor = 50;
    int64 pwd = 0;
    while lexer.has_next() {
        int64 b = lexer.next();
        int64 a = (cursor + b);

        if a <= 0 {
            if cursor == 0 {
                pwd += (-a / 100);
            } else {
                pwd += (-a / 100) + 1;
            }
        } else {
            pwd += (a / 100);
        }

        if b < 0  cursor = a % 100 + 100
        else      cursor = a;
        cursor %= 100;

    }
    write_int(pwd);
}