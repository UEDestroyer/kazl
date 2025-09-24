#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <any>
#include <memory>
#include <utility>
#include <sstream>
#include <type_traits>

// -------- macros --------
#define REFLECTABLE(...) \
    std::map<std::string, Let> to_map() const { \
        return { __VA_ARGS__ }; \
    }



template<typename T>
class BaseClass;
class Let;

// -------- range ---------
class range : public std::vector<int> {
public:
    range(int to) {
        for (int i = 0;i<to;i++) {
            this->push_back(i);
        }
    }

    range(int from,int to) {
        for (int i = from;i<to;i++) {
            this->push_back(i);
        }
    }
    range(int from,int to,int inter) {
        for (int i = from;i<to;i+=inter) {
            this->push_back(i);
        }
    }
};
//-----list------
class List : public std::vector<Let> {
public:

};


// -------- Sout --------
class Sout {
    std::ostringstream ss;
public:

    template<typename T>
    Sout& operator<<(const T& value) {
        // если объект наследник BaseClass
        if constexpr (std::is_base_of_v<BaseClass<std::decay_t<T>>, T>) {
            ss << "{";
            const auto& fields = value.get_map();
            for (const auto& [key, val] : fields) {
                ss << key << ": " << Sout{} << val << ", ";
            }
            std::string s = ss.str();
            if (s.size() > 2) s[s.size() - 2] = '}'; // закрываем скобку
            ss.str(""); ss.clear();
            ss << s;
        }
        // обычные типы
        else ss << value;
        return *this;
    }



    std::string str() const { return ss.str(); }
    operator std::string() const {
        return ss.str();
    }
};

// -------- Let --------
template<typename T, typename = void>
struct is_streamable : std::false_type {};

template<typename T>
struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<const T&>())>>
    : std::true_type {};

class Let {
    struct Base {
        virtual ~Base() = default;
        virtual void print(std::ostream& os) const = 0;
        virtual std::shared_ptr<Base> clone() const = 0;
        virtual std::string toString() const = 0;
        virtual double toDouble(bool& ok) const = 0;
    };

    template<typename T>
    struct Holder : Base {
        T value;
        Holder(T v) : value(std::move(v)) {}

        void print(std::ostream& os) const override {
            if constexpr (is_streamable<T>::value) {
                os << value;
            } else {
                os << "[unprintable type]";
            }
        }

        std::shared_ptr<Base> clone() const override {
            return std::make_shared<Holder<T>>(value);
        }

        std::string toString() const override {
            if constexpr (std::is_convertible_v<T, std::string>)
                return value;
            else if constexpr (std::is_arithmetic_v<T>)
                return std::to_string(value);
            else
                return "[unstringable]";
        }

        double toDouble(bool& ok) const override {
            if constexpr (std::is_arithmetic_v<T>) {
                ok = true;
                return static_cast<double>(value);
            } else if constexpr (std::is_convertible_v<T, std::string>) {
                try {
                    return std::stod(value);
                } catch (...) {
                    ok = false;
                    return 0;
                }
            } else {
                ok = false;
                return 0;
            }
        }
    };

    std::shared_ptr<Base> ptr;

public:
    template<typename T>
    Let(T v) : ptr(std::make_shared<Holder<T>>(std::move(v))) {}

    Let(const Let& other) : ptr(other.ptr ? other.ptr->clone() : nullptr) {}
    Let& operator=(const Let& other) {
        if (this != &other) {
            ptr = other.ptr ? other.ptr->clone() : nullptr;
        }
        return *this;
    }

    // печать
    friend std::ostream& operator<<(std::ostream& os, const Let& obj) {
        if (obj.ptr) obj.ptr->print(os);
        else os << "null";
        return os;
    }

    // ================= СРАВНЕНИЯ =================
    friend bool operator==(const Let& a, const Let& b) {
        if (!a.ptr || !b.ptr) return false;
        bool okA=false, okB=false;
        double da = a.ptr->toDouble(okA);
        double db = b.ptr->toDouble(okB);
        if (okA && okB) return da == db; // числовое сравнение
        return a.ptr->toString() == b.ptr->toString(); // строковое
    }

    friend bool operator!=(const Let& a, const Let& b) {
        return !(a == b);
    }

    friend bool operator<(const Let& a, const Let& b) {
        bool okA=false, okB=false;
        double da = a.ptr->toDouble(okA);
        double db = b.ptr->toDouble(okB);
        if (okA && okB) return da < db;
        return a.ptr->toString() < b.ptr->toString();
    }

    friend bool operator>(const Let& a, const Let& b) {
        return b < a;
    }

    friend bool operator<=(const Let& a, const Let& b) {
        return !(a > b);
    }

    friend bool operator>=(const Let& a, const Let& b) {
        return !(a < b);
    }

    // ================= АРИФМЕТИКА =================
    friend Let operator+(const Let& a, const Let& b) {
        bool okA=false, okB=false;
        double da = a.ptr->toDouble(okA);
        double db = b.ptr->toDouble(okB);
        if (okA && okB) return Let(da + db);
        return Let(a.ptr->toString() + b.ptr->toString());
    }

    friend Let operator-(const Let& a, const Let& b) {
        bool okA=false, okB=false;
        double da = a.ptr->toDouble(okA);
        double db = b.ptr->toDouble(okB);
        if (okA && okB) return Let(da - db);
        throw std::runtime_error("Unsupported types for -");
    }

    friend Let operator*(const Let& a, const Let& b) {
        bool okA=false, okB=false;
        double da = a.ptr->toDouble(okA);
        double db = b.ptr->toDouble(okB);
        if (okA && okB) return Let(da * db);
        throw std::runtime_error("Unsupported types for *");
    }

    friend Let operator/(const Let& a, const Let& b) {
        bool okA=false, okB=false;
        double da = a.ptr->toDouble(okA);
        double db = b.ptr->toDouble(okB);
        if (okA && okB) {
            if (db == 0) throw std::runtime_error("Division by zero");
            return Let(da / db);
        }
        throw std::runtime_error("Unsupported types for /");
    }

    // префикс ++x
    friend Let& operator++(Let& a) {
        a = a + Let(1);
        return a;
    }

    // постфикс x++
    friend Let operator++(Let& a, int) {
        Let old = a;
        a = a + Let(1);
        return old;
    }

    friend Let& operator--(Let& a) {
        a = a - Let(1);
        return a;
    }
    friend Let operator--(Let& a, int) {
        Let old = a;
        a = a - Let(1);
        return old;
    }
    // ================= get<T>() =================
    template<typename T>
    T& get() {
        auto* h = dynamic_cast<Holder<T>*>(ptr.get());
        if (!h) throw std::bad_cast();
        return h->value;
    }

    template<typename T>
    const T& get() const {
        auto* h = dynamic_cast<const Holder<T>*>(ptr.get());
        if (!h) throw std::bad_cast();
        return h->value;
    }



    // авто-конверсия
    template<typename T>
    operator T&() { return get<T>(); }

    template<typename T>
    operator const T&() const { return get<T>(); }
};

// -------- BaseClass --------
template<typename Derived>
class BaseClass {
protected:
    std::map<std::string, Let> fields;

    const std::map<std::string, Let>& get_map() const {
        return fields;
    }

public:
    BaseClass() = default;
    virtual ~BaseClass() = default;

    // Пусть Derived сам вызовет init_fields()
    void init_fields() {
        fields = static_cast<const Derived*>(this)->to_map();
    }

    friend std::ostream& operator<<(std::ostream& os, const BaseClass<Derived>& obj) {
        const auto& fields = obj.get_map();
        std::string res = "{";
        for (const auto& [key, value] : fields) {
            res += key + ": " + static_cast<std::string>(Sout{} << value) + ", ";
        }
        res[res.size() -2] = '}';
        return os << res;
    }
};

//functions
// -------- print --------
inline bool print(const Let& val, const std::string& end = "\n") {
    std::cout << val << end;
    return true;
}
//read-write


bool write_file(const std::string& filename, const Let& data) {
    std::ofstream ofs(filename, std::ios::out | std::ios::trunc);
    if (!ofs) return false;
    ofs << data;
    return true;
}

bool write_file(const std::string& filename, const std::string data) {
    std::ofstream ofs(filename, std::ios::out | std::ios::trunc);
    if (!ofs) return false;
    ofs << data;
    return true;
}



// read_file: считать весь файл в строку
inline std::string read_file(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::in);
    if (!ifs) return "";
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));
}
inline Let read_file_let(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::in);
    if (!ifs) return Let("");  // явный возврат Let от строки
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    return Let(content);
}


bool replace_all(std::string s,std::string from,std::string to) {
    while (s.find(from)!=std::string::npos) {
        s.replace(s.find(from),from.size(),to);
    }
    return true;
}