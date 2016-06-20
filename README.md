# Areenajabat
2016 kesäpeliprojekti

```cpp
// Sulkujen asettaminen omalle riville
int main()
{
}

// funktiot lowerCamelCase()
void doSomething()
{
}

// sisäkkäiset preprosessorit
#if 1
#   if LINUX
#     include <sys/stat.h>
#   endif
#endif

// luokat ja jäsenet
class UpperCamelCase
{
  // publicit ylhäällä
public:
  void function();
// privatet alimpana ja muuttujat
private:
  void privateFunc();
  int m_member;
  std::string m_name;
};

// namespace
namespace arena
{
  // kaikki paska tän sisään
}

// vakiot, UpperCamel
const int MaxMornings = 42;

struct Constant
{
  enum Enum
  {
    Constant,
    Value
  };
};

// .cpp/.h sisäiset globaalit/staattiset
int g_globalInt = 4;
static int s_staticInt = 5;
```
