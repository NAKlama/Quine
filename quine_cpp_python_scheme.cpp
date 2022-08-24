/*
 * Multi-Language Quine
 * Author: Nina Alexandra Klama
 * Languages: C++11, Python 2.7, Scheme (Racket)
 *
 * Compile with: g++ -std=gnu++0x
 */
using namespace std;

#include <string>
#include <vector>
#include <map>
#include <tclap/CmdLine.h>

enum class Language {
  CPP,
  PYTHON,
  SCHEME
};

string version = "v1.1";

namespace func
{
  string escape(Language l, string s)
  {
    string out;
    for(char c : s)
    {
      switch(c)
      {
        case '\"':
          out += "\\\""; break;
        case '\\':
          out += "\\\\"; break;
        case '\'':
          out += "\\\'"; break;
        default:
          out += c;
      }
    }
    return out;
  }
}

class ReplaceObject
{
  public:
    virtual vector<string> retCode(Language lang) = 0;
    virtual string getReplString() = 0;
};

class ReplaceVectorString : public ReplaceObject
{
  private:
    vector<string>  *var;
    string          replName;
  public:
    ReplaceVectorString(string name, vector<string> *in) 
      : var(in), replName(name) {}
    string getReplString() { return replName; }  
    vector<string> retCode(Language lang)
    {
      auto ret = vector<string> ();

      for(string vLine : *var)
      {
        string outLine;
        outLine = "  \"";
        outLine += func::escape(lang, vLine);
        outLine += "\"";
        if(lang != Language::SCHEME)
          outLine += ",";
        ret.push_back(outLine);
      }
      if(lang != Language::SCHEME)
        ret.back() = ret.back().substr(0, ret.back().length()-1);
      return ret;
    }
};

class ReplaceVariableString : public ReplaceObject
{
  private:
    map<Language,string>  preString, postString;
    string                *var;
    string                replName;
  public:
    ReplaceVariableString(string name, string *v = nullptr) : var(v), replName(name) {}
    string getReplString() { return replName; }
    void setString(Language l, string pre, string post)
    { 
      preString.insert(pair<Language,string>(l,pre)); 
      postString.insert(pair<Language,string>(l,post)); 
    }
    vector<string> retCode(Language lang)
    {
      auto ret = vector<string> ();
      string out = "";
      out += preString[lang];
      out += func::escape(lang, *var);
      out += postString[lang];
      ret.push_back(out);
      return ret;
    }
};

class CodeObject
{
  private:
    map<Language,vector<string>*>       code;
    map<const string, ReplaceObject*>  *replacements;  
  public:
    CodeObject()
    {
      replacements = nullptr;
    }
    void addCode(Language lang, vector<string>* codeIn)
    { 
      code.insert( pair<Language,vector<string>*>(lang, codeIn) );
    }
    vector<string> returnCode(Language lang)
    {
      vector<string> out;
      auto lines = code[lang];
      for(string l : *lines)
      {
        if(replacements != nullptr)
        {
          bool found = false;
          for(auto m : *replacements)
          {
            if(!l.compare(m.first))
            {
              found = true;
              vector<string> repl = m.second->retCode(lang);
              for(auto retStr : repl)
                out.push_back(retStr);
            }
          }
          if(!found)
            out.push_back(l);
        }
        else
          out.push_back(l);
      }
      return out;
    }
    void addReplacement(ReplaceObject* ro)
    {
      string s = ro->getReplString();
      if(replacements == nullptr)
        replacements = new map<const string, ReplaceObject*>;
      replacements->insert(pair<const string, ReplaceObject*>(s, ro));
    }
    vector<string>* getCode(Language l)
    {
      vector<string> *ret = nullptr;
      for(auto m : code)
      {
        if(m.first == l)
          ret = m.second;
      }
      return ret;
    }
};

class Quine
{
  private:
    string      version;
    CodeObject  COPre;
    CodeObject  COClasses;
    CodeObject  COVar;
    CodeObject  COPost;

  public:
    Quine(string v) : version(v) {}
    void addLang(Language l, vector<string> *pre, vector<string> *classes, vector<string> *var, vector<string> *post)
    {
      COPre.addCode(l,      pre);
      COClasses.addCode(l,  classes);
      COVar.addCode(l,      var);
      COPost.addCode(l,     post);
    }
    void init()
    {
      auto *replVersion = new ReplaceVariableString("###VERSION###", &version);
      replVersion->setString(Language::CPP,    "string version = \"", "\";");
      replVersion->setString(Language::PYTHON, "version = \"", "\"");
      replVersion->setString(Language::SCHEME, "(define version \"", "\")");
      COPre.addReplacement(replVersion);

      auto replPreCPP          = new ReplaceVectorString("###strPreCPP###",        COPre.getCode(Language::CPP));
      auto replClassesCPP      = new ReplaceVectorString("###strClassesCPP###",    COClasses.getCode(Language::CPP));
      auto replVarCPP          = new ReplaceVectorString("###strVarCPP###",        COVar.getCode(Language::CPP));
      auto replPostCPP         = new ReplaceVectorString("###strPostCPP###",       COPost.getCode(Language::CPP));
      auto replPrePYTHON       = new ReplaceVectorString("###strPrePYTHON###",     COPre.getCode(Language::PYTHON));
      auto replClassesPYTHON   = new ReplaceVectorString("###strClassesPYTHON###", COClasses.getCode(Language::PYTHON));
      auto replVarPYTHON       = new ReplaceVectorString("###strVarPYTHON###",     COVar.getCode(Language::PYTHON));
      auto replPostPYTHON      = new ReplaceVectorString("###strPostPYTHON###",    COPost.getCode(Language::PYTHON));
      auto replPreSCHEME       = new ReplaceVectorString("###strPreSCHEME###",     COPre.getCode(Language::SCHEME));
      auto replClassesSCHEME   = new ReplaceVectorString("###strClassesSCHEME###", COClasses.getCode(Language::SCHEME));
      auto replVarSCHEME       = new ReplaceVectorString("###strVarSCHEME###",     COVar.getCode(Language::SCHEME));
      auto replPostSCHEME      = new ReplaceVectorString("###strPostSCHEME###",    COPost.getCode(Language::SCHEME));

      COVar.addReplacement(replPreCPP);
      COVar.addReplacement(replClassesCPP);
      COVar.addReplacement(replVarCPP);
      COVar.addReplacement(replPostCPP);
      COVar.addReplacement(replPrePYTHON);
      COVar.addReplacement(replClassesPYTHON);
      COVar.addReplacement(replVarPYTHON);
      COVar.addReplacement(replPostPYTHON);
      COVar.addReplacement(replPreSCHEME);
      COVar.addReplacement(replClassesSCHEME);
      COVar.addReplacement(replVarSCHEME);
      COVar.addReplacement(replPostSCHEME);
    }
    void print(Language l)
    {
      for(string c : COPre.returnCode(l))
        cout << c << endl;
      for(string c : COClasses.returnCode(l))
        cout << c << endl;
      for(string c : COVar.returnCode(l))
        cout << c << endl;
      for(string c : COPost.returnCode(l))
        cout << c << endl;
    }
};

vector<string> strPreCPP = {
  "/*",
  " * Multi-Language Quine",
  " * Author: Nina Alexandra Klama",
  " * Languages: C++11, Python 2.7, Scheme (Racket)",
  " *",
  " * Compile with: g++ -std=gnu++0x",
  " */",
  "using namespace std;",
  "",
  "#include <string>",
  "#include <vector>",
  "#include <map>",
  "#include <tclap/CmdLine.h>",
  "",
  "enum class Language {",
  "  CPP,",
  "  PYTHON,",
  "  SCHEME",
  "};",
  "",
  "###VERSION###",
  "",
  "namespace func",
  "{",
  "  string escape(Language l, string s)",
  "  {",
  "    string out;",
  "    for(char c : s)",
  "    {",
  "      switch(c)",
  "      {",
  "        case \'\\\"\':",
  "          out += \"\\\\\\\"\"; break;",
  "        case \'\\\\\':",
  "          out += \"\\\\\\\\\"; break;",
  "        case \'\\\'\':",
  "          out += \"\\\\\\\'\"; break;",
  "        default:",
  "          out += c;",
  "      }",
  "    }",
  "    return out;",
  "  }",
  "}",
  ""
};

vector<string> strClassesCPP = {
  "class ReplaceObject",
  "{",
  "  public:",
  "    virtual vector<string> retCode(Language lang) = 0;",
  "    virtual string getReplString() = 0;",
  "};",
  "",
  "class ReplaceVectorString : public ReplaceObject",
  "{",
  "  private:",
  "    vector<string>  *var;",
  "    string          replName;",
  "  public:",
  "    ReplaceVectorString(string name, vector<string> *in) ",
  "      : var(in), replName(name) {}",
  "    string getReplString() { return replName; }  ",
  "    vector<string> retCode(Language lang)",
  "    {",
  "      auto ret = vector<string> ();",
  "",
  "      for(string vLine : *var)",
  "      {",
  "        string outLine;",
  "        outLine = \"  \\\"\";",
  "        outLine += func::escape(lang, vLine);",
  "        outLine += \"\\\"\";",
  "        if(lang != Language::SCHEME)",
  "          outLine += \",\";",
  "        ret.push_back(outLine);",
  "      }",
  "      if(lang != Language::SCHEME)",
  "        ret.back() = ret.back().substr(0, ret.back().length()-1);",
  "      return ret;",
  "    }",
  "};",
  "",
  "class ReplaceVariableString : public ReplaceObject",
  "{",
  "  private:",
  "    map<Language,string>  preString, postString;",
  "    string                *var;",
  "    string                replName;",
  "  public:",
  "    ReplaceVariableString(string name, string *v = nullptr) : var(v), replName(name) {}",
  "    string getReplString() { return replName; }",
  "    void setString(Language l, string pre, string post)",
  "    { ",
  "      preString.insert(pair<Language,string>(l,pre)); ",
  "      postString.insert(pair<Language,string>(l,post)); ",
  "    }",
  "    vector<string> retCode(Language lang)",
  "    {",
  "      auto ret = vector<string> ();",
  "      string out = \"\";",
  "      out += preString[lang];",
  "      out += func::escape(lang, *var);",
  "      out += postString[lang];",
  "      ret.push_back(out);",
  "      return ret;",
  "    }",
  "};",
  "",
  "class CodeObject",
  "{",
  "  private:",
  "    map<Language,vector<string>*>       code;",
  "    map<const string, ReplaceObject*>  *replacements;  ",
  "  public:",
  "    CodeObject()",
  "    {",
  "      replacements = nullptr;",
  "    }",
  "    void addCode(Language lang, vector<string>* codeIn)",
  "    { ",
  "      code.insert( pair<Language,vector<string>*>(lang, codeIn) );",
  "    }",
  "    vector<string> returnCode(Language lang)",
  "    {",
  "      vector<string> out;",
  "      auto lines = code[lang];",
  "      for(string l : *lines)",
  "      {",
  "        if(replacements != nullptr)",
  "        {",
  "          bool found = false;",
  "          for(auto m : *replacements)",
  "          {",
  "            if(!l.compare(m.first))",
  "            {",
  "              found = true;",
  "              vector<string> repl = m.second->retCode(lang);",
  "              for(auto retStr : repl)",
  "                out.push_back(retStr);",
  "            }",
  "          }",
  "          if(!found)",
  "            out.push_back(l);",
  "        }",
  "        else",
  "          out.push_back(l);",
  "      }",
  "      return out;",
  "    }",
  "    void addReplacement(ReplaceObject* ro)",
  "    {",
  "      string s = ro->getReplString();",
  "      if(replacements == nullptr)",
  "        replacements = new map<const string, ReplaceObject*>;",
  "      replacements->insert(pair<const string, ReplaceObject*>(s, ro));",
  "    }",
  "    vector<string>* getCode(Language l)",
  "    {",
  "      vector<string> *ret = nullptr;",
  "      for(auto m : code)",
  "      {",
  "        if(m.first == l)",
  "          ret = m.second;",
  "      }",
  "      return ret;",
  "    }",
  "};",
  "",
  "class Quine",
  "{",
  "  private:",
  "    string      version;",
  "    CodeObject  COPre;",
  "    CodeObject  COClasses;",
  "    CodeObject  COVar;",
  "    CodeObject  COPost;",
  "",
  "  public:",
  "    Quine(string v) : version(v) {}",
  "    void addLang(Language l, vector<string> *pre, vector<string> *classes, vector<string> *var, vector<string> *post)",
  "    {",
  "      COPre.addCode(l,      pre);",
  "      COClasses.addCode(l,  classes);",
  "      COVar.addCode(l,      var);",
  "      COPost.addCode(l,     post);",
  "    }",
  "    void init()",
  "    {",
  "      auto *replVersion = new ReplaceVariableString(\"###VERSION###\", &version);",
  "      replVersion->setString(Language::CPP,    \"string version = \\\"\", \"\\\";\");",
  "      replVersion->setString(Language::PYTHON, \"version = \\\"\", \"\\\"\");",
  "      replVersion->setString(Language::SCHEME, \"(define version \\\"\", \"\\\")\");",
  "      COPre.addReplacement(replVersion);",
  "",
  "      auto replPreCPP          = new ReplaceVectorString(\"###strPreCPP###\",        COPre.getCode(Language::CPP));",
  "      auto replClassesCPP      = new ReplaceVectorString(\"###strClassesCPP###\",    COClasses.getCode(Language::CPP));",
  "      auto replVarCPP          = new ReplaceVectorString(\"###strVarCPP###\",        COVar.getCode(Language::CPP));",
  "      auto replPostCPP         = new ReplaceVectorString(\"###strPostCPP###\",       COPost.getCode(Language::CPP));",
  "      auto replPrePYTHON       = new ReplaceVectorString(\"###strPrePYTHON###\",     COPre.getCode(Language::PYTHON));",
  "      auto replClassesPYTHON   = new ReplaceVectorString(\"###strClassesPYTHON###\", COClasses.getCode(Language::PYTHON));",
  "      auto replVarPYTHON       = new ReplaceVectorString(\"###strVarPYTHON###\",     COVar.getCode(Language::PYTHON));",
  "      auto replPostPYTHON      = new ReplaceVectorString(\"###strPostPYTHON###\",    COPost.getCode(Language::PYTHON));",
  "      auto replPreSCHEME       = new ReplaceVectorString(\"###strPreSCHEME###\",     COPre.getCode(Language::SCHEME));",
  "      auto replClassesSCHEME   = new ReplaceVectorString(\"###strClassesSCHEME###\", COClasses.getCode(Language::SCHEME));",
  "      auto replVarSCHEME       = new ReplaceVectorString(\"###strVarSCHEME###\",     COVar.getCode(Language::SCHEME));",
  "      auto replPostSCHEME      = new ReplaceVectorString(\"###strPostSCHEME###\",    COPost.getCode(Language::SCHEME));",
  "",
  "      COVar.addReplacement(replPreCPP);",
  "      COVar.addReplacement(replClassesCPP);",
  "      COVar.addReplacement(replVarCPP);",
  "      COVar.addReplacement(replPostCPP);",
  "      COVar.addReplacement(replPrePYTHON);",
  "      COVar.addReplacement(replClassesPYTHON);",
  "      COVar.addReplacement(replVarPYTHON);",
  "      COVar.addReplacement(replPostPYTHON);",
  "      COVar.addReplacement(replPreSCHEME);",
  "      COVar.addReplacement(replClassesSCHEME);",
  "      COVar.addReplacement(replVarSCHEME);",
  "      COVar.addReplacement(replPostSCHEME);",
  "    }",
  "    void print(Language l)",
  "    {",
  "      for(string c : COPre.returnCode(l))",
  "        cout << c << endl;",
  "      for(string c : COClasses.returnCode(l))",
  "        cout << c << endl;",
  "      for(string c : COVar.returnCode(l))",
  "        cout << c << endl;",
  "      for(string c : COPost.returnCode(l))",
  "        cout << c << endl;",
  "    }",
  "};",
  ""
};

vector<string> strVarCPP = {
  "vector<string> strPreCPP = {",
  "###strPreCPP###",
  "};",
  "",
  "vector<string> strClassesCPP = {",
  "###strClassesCPP###",
  "};",
  "",
  "vector<string> strVarCPP = {",
  "###strVarCPP###",
  "};",
  "",
  "vector<string> strPostCPP = {",
  "###strPostCPP###",
  "};",
  "",
  "vector<string> strPrePYTHON = {",
  "###strPrePYTHON###",
  "};",
  "",
  "vector<string> strClassesPYTHON = {",
  "###strClassesPYTHON###",
  "};",
  "",
  "vector<string> strVarPYTHON = {",
  "###strVarPYTHON###",
  "};",
  "",
  "vector<string> strPostPYTHON = {",
  "###strPostPYTHON###",
  "};",
  "",
  "vector<string> strPreSCHEME = {",
  "###strPreSCHEME###",
  "};",
  "",
  "vector<string> strClassesSCHEME = {",
  "###strClassesSCHEME###",
  "};",
  "",
  "vector<string> strVarSCHEME = {",
  "###strVarSCHEME###",
  "};",
  "",
  "vector<string> strPostSCHEME = {",
  "###strPostSCHEME###",
  "};",
  ""
};

vector<string> strPostCPP = {
  "int main(int argc, char const *argv[])",
  "{",
  "  auto q = Quine(version);",
  "  q.addLang(Language::CPP, &strPreCPP, &strClassesCPP, &strVarCPP, &strPostCPP);",
  "  q.addLang(Language::PYTHON, &strPrePYTHON, &strClassesPYTHON, &strVarPYTHON, &strPostPYTHON);",
  "  q.addLang(Language::SCHEME, &strPreSCHEME, &strClassesSCHEME, &strVarSCHEME, &strPostSCHEME);",
  "  q.init();",
  "",
  "  Language lang;",
  "",
  "  try ",
  "  {",
  "    TCLAP::CmdLine cmd(\"Multi-Language Quine\", \' \', version);",
  "    TCLAP::SwitchArg lang_cpp(\"\", \"cpp\", \"Display C++11 Quine\");",
  "    TCLAP::SwitchArg lang_python(\"\", \"python\", \"Display Python 2.7 Quine\");",
  "    TCLAP::SwitchArg lang_scheme(\"\", \"scheme\", \"Display Scheme (Racket) Quine\");",
  "    vector<TCLAP::Arg*> xorList = {",
  "      &lang_cpp,",
  "      &lang_python,",
  "      &lang_scheme",
  "    };",
  "    cmd.xorAdd(xorList);",
  "    cmd.parse(argc, argv);",
  "",
  "    if(lang_cpp.getValue())",
  "      lang = Language::CPP;",
  "    else if(lang_python.getValue())",
  "      lang = Language::PYTHON;",
  "    else if(lang_scheme.getValue())",
  "      lang = Language::SCHEME;",
  "  }",
  "  catch (TCLAP::ArgException &e)",
  "  {",
  "    cerr << \"error: \" << e.error() << \" for arg \" << e.argId() << endl;",
  "  }",
  "",
  "  q.print(lang);",
  "",
  "  return 0;",
  "}",
  ""
};

vector<string> strPrePYTHON = {
  "#!/usr/bin/python",
  "#",
  "# Multi-Language Quine",
  "# Author: Nina Alexandra Klama",
  "# Languages: C++11, Python 2.7, Scheme (Racket)",
  "#",
  "",
  "import argparse",
  "",
  "###VERSION###",
  "",
  "",
  "def escape(lang, inStr):",
  "  out = \"\"",
  "  for c in inStr:",
  "    if c == \'\\\"\':",
  "      out += \"\\\\\\\"\"",
  "    elif c == \'\\\\\':",
  "      out += \"\\\\\\\\\"",
  "    elif c == \'\\\'\':",
  "      out += \"\\\\\\\'\"",
  "    else:",
  "      out += c",
  "  return out",
  "",
  ""
};

vector<string> strClassesPYTHON = {
  "class ReplaceObject:",
  "  pass",
  "",
  "",
  "class ReplaceVectorString (ReplaceObject):",
  "  def __init__(self, name, inVar):",
  "    self.replName    = name",
  "    self.var         = inVar",
  "",
  "  def getReplString(self):",
  "    return self.replName",
  "",
  "  def retCode(self, lang):",
  "    ret = []",
  "    for vLine in self.var:",
  "      outLine  = \"  \\\"\"",
  "      outLine += escape(lang, vLine)",
  "      outLine += \"\\\"\"",
  "      if(lang != \"SCHEME\"):",
  "        outLine += \",\"",
  "      ret.append(outLine)",
  "    if(lang != \"SCHEME\"):",
  "      ret[len(ret)-1] = ret[len(ret)-1][0:len(ret[len(ret)-1])-1]",
  "    return ret",
  "",
  "",
  "class ReplaceVariableString (ReplaceObject):",
  "  def __init__(self, name, var):",
  "    self.replName   = name",
  "    self.var        = var",
  "    self.strings    = []",
  "",
  "  def getReplString(self):",
  "    return self.replName",
  "",
  "  def setString(self, lang, pre, post):",
  "    self.strings.append({\'lang\': lang, \'pre\': pre, \'post\': post})",
  "",
  "  def retCode(self, lang):",
  "    ret = []",
  "    strings = None",
  "    for s in self.strings:",
  "      if s[\'lang\'] == lang:",
  "        strings = s",
  "    out  = strings[\'pre\']",
  "    out += escape(lang, self.var)",
  "    out += strings[\'post\']",
  "    ret.append(out)",
  "    return ret",
  "",
  "",
  "class CodeObject:",
  "  def __init__(self):",
  "    self.replacements = []",
  "    self.code         = []",
  "",
  "  def addCode(self, lang, codeIn):",
  "    self.code.append({\'lang\': lang, \'code\': codeIn})",
  "",
  "  def addReplacement(self, ro):",
  "    replStr = ro.getReplString()",
  "    self.replacements.append({\'str\': replStr, \'obj\': ro})",
  "",
  "  def returnCode(self, lang):",
  "    out = []",
  "    lines = None",
  "    for c in self.code:",
  "      if c[\'lang\'] == lang:",
  "        lines = c[\'code\']",
  "    for line in lines:",
  "      if len(self.replacements) > 0:",
  "        found = False",
  "        for repl in self.replacements:",
  "          if repl[\'str\'] == line:",
  "            found = True",
  "            for retStr in repl[\'obj\'].retCode(lang):",
  "              out.append(retStr)",
  "        if not found:",
  "          out.append(line)",
  "      else:",
  "        out.append(line)",
  "    return out",
  "",
  "  def getCode(self, lang):",
  "    ret = None",
  "    for c in self.code:",
  "      if c[\'lang\'] == lang:",
  "        ret = c[\'code\']",
  "    return ret",
  "",
  "",
  "class Quine:",
  "  def __init__(self, version):",
  "    self.version   = version",
  "    self.COPre     = CodeObject()",
  "    self.COClasses = CodeObject()",
  "    self.COVar     = CodeObject()",
  "    self.COPost    = CodeObject()",
  "",
  "  def addLang(self, lang, pre, classes, var, post):",
  "    self.COPre.addCode(lang, pre)",
  "    self.COClasses.addCode(lang, classes)",
  "    self.COVar.addCode(lang, var)",
  "    self.COPost.addCode(lang, post)",
  "",
  "  def init(self):",
  "    replVersion = ReplaceVariableString(\"###VERSION###\", self.version)",
  "    replVersion.setString(\"CPP\",    \"string version = \\\"\", \"\\\";\")",
  "    replVersion.setString(\"PYTHON\", \"version = \\\"\",\"\\\"\")",
  "    replVersion.setString(\"SCHEME\", \"(define version \\\"\",\"\\\")\")",
  "    self.COPre.addReplacement(replVersion)",
  "",
  "    replPreCPP        = ReplaceVectorString(\"###strPreCPP###\",        self.COPre.getCode(\"CPP\"))",
  "    replClassesCPP    = ReplaceVectorString(\"###strClassesCPP###\",    self.COClasses.getCode(\"CPP\"))",
  "    replVarCPP        = ReplaceVectorString(\"###strVarCPP###\",        self.COVar.getCode(\"CPP\"))",
  "    replPostCPP       = ReplaceVectorString(\"###strPostCPP###\",       self.COPost.getCode(\"CPP\"))",
  "    replPrePYTHON     = ReplaceVectorString(\"###strPrePYTHON###\",     self.COPre.getCode(\"PYTHON\"))",
  "    replClassesPYTHON = ReplaceVectorString(\"###strClassesPYTHON###\", self.COClasses.getCode(\"PYTHON\"))",
  "    replVarPYTHON     = ReplaceVectorString(\"###strVarPYTHON###\",     self.COVar.getCode(\"PYTHON\"))",
  "    replPostPYTHON    = ReplaceVectorString(\"###strPostPYTHON###\",    self.COPost.getCode(\"PYTHON\"))",
  "    replPreSCHEME     = ReplaceVectorString(\"###strPreSCHEME###\",     self.COPre.getCode(\"SCHEME\"))",
  "    replClassesSCHEME = ReplaceVectorString(\"###strClassesSCHEME###\", self.COClasses.getCode(\"SCHEME\"))",
  "    replVarSCHEME     = ReplaceVectorString(\"###strVarSCHEME###\",     self.COVar.getCode(\"SCHEME\"))",
  "    replPostSCHEME    = ReplaceVectorString(\"###strPostSCHEME###\",    self.COPost.getCode(\"SCHEME\"))",
  "",
  "    self.COVar.addReplacement(replPreCPP)",
  "    self.COVar.addReplacement(replClassesCPP)",
  "    self.COVar.addReplacement(replVarCPP)",
  "    self.COVar.addReplacement(replPostCPP)",
  "    self.COVar.addReplacement(replPrePYTHON)",
  "    self.COVar.addReplacement(replClassesPYTHON)",
  "    self.COVar.addReplacement(replVarPYTHON)",
  "    self.COVar.addReplacement(replPostPYTHON)",
  "    self.COVar.addReplacement(replPreSCHEME)",
  "    self.COVar.addReplacement(replClassesSCHEME)",
  "    self.COVar.addReplacement(replVarSCHEME)",
  "    self.COVar.addReplacement(replPostSCHEME)",
  "",
  "  def output(self, lang):",
  "    for line in self.COPre.returnCode(lang):",
  "      print line",
  "    for line in self.COClasses.returnCode(lang):",
  "      print line",
  "    for line in self.COVar.returnCode(lang):",
  "      print line",
  "    for line in self.COPost.returnCode(lang):",
  "      print line",
  "",
  "",
  "class ArgumentError(Exception):",
  "  pass",
  "",
  ""
};

vector<string> strVarPYTHON = {
  "strPreCPP = [",
  "###strPreCPP###",
  "  ]",
  "",
  "strClassesCPP = [",
  "###strClassesCPP###",
  "  ]",
  "",
  "strVarCPP = [",
  "###strVarCPP###",
  "  ]",
  "",
  "strPostCPP = [",
  "###strPostCPP###",
  "  ]",
  "",
  "strPrePYTHON = [",
  "###strPrePYTHON###",
  "  ]",
  "",
  "strClassesPYTHON = [",
  "###strClassesPYTHON###",
  "  ]",
  "",
  "strVarPYTHON = [",
  "###strVarPYTHON###",
  "  ]",
  "",
  "strPostPYTHON = [",
  "###strPostPYTHON###",
  "  ]",
  "",
  "strPreSCHEME = [",
  "###strPreSCHEME###",
  "  ]",
  "",
  "strClassesSCHEME = [",
  "###strClassesSCHEME###",
  "  ]",
  "",
  "strVarSCHEME = [",
  "###strVarSCHEME###",
  "  ]",
  "",
  "strPostSCHEME = [",
  "###strPostSCHEME###",
  "  ]",
  ""
};

vector<string> strPostPYTHON = {
  "if __name__ == \"__main__\":",
  "  q = Quine(version)",
  "  q.addLang(\"CPP\", strPreCPP, strClassesCPP, strVarCPP, strPostCPP)",
  "  q.addLang(\"PYTHON\", strPrePYTHON, strClassesPYTHON, strVarPYTHON, strPostPYTHON)",
  "  q.addLang(\"SCHEME\", strPreSCHEME, strClassesSCHEME, strVarSCHEME, strPostSCHEME)",
  "  q.init()",
  "",
  "  argParser = argparse.ArgumentParser(description=\"Multi-Language Quine %s\" % version)",
  "  argParser.add_argument(\'--cpp\',    action=\"store_true\", help=\"Display C++11 Quine\")",
  "  argParser.add_argument(\'--python\', action=\"store_true\", help=\"Display Python 2.7 Quine\")",
  "  argParser.add_argument(\'--scheme\', action=\"store_true\", help=\"Display Scheme (Racket) Quine\")",
  "  args = vars(argParser.parse_args())",
  "",
  "  argCount = 0",
  "  argOpts = [\'cpp\', \'python\', \'scheme\']",
  "  for aO in argOpts:",
  "    if args[aO]:",
  "      argCount += 1;",
  "  if argCount > 1:",
  "    raise ArgumentError(\"\\nERROR:Only specify one language please!\")",
  "",
  "  if args[\'cpp\']:",
  "    lang = \"CPP\"",
  "  elif args[\'scheme\']:",
  "    lang = \"SCHEME\"",
  "  else:",
  "    lang = \"PYTHON\"",
  "",
  "  q.output(lang)"
};

vector<string> strPreSCHEME = {
  "#! /usr/bin/racket",
  "#lang racket/base",
  "",
  "; ",
  "; Multi-Language Quine",
  "; Author: Nina Alexandra Klama",
  "; Languages: C++11, Python 2.7, Scheme (Racket)",
  "; ",
  "",
  "(require racket/cmdline)",
  "",
  "(define argv (current-command-line-arguments))",
  "",
  "(define (escapeChar inC)",
  "  (cond",
  "    [(char=? #\\\" inC) \"\\\\\\\"\"]",
  "    [(char=? #\\\\ inC) \"\\\\\\\\\"]",
  "    [(char=? #\\\' inC) \"\\\\\\\'\"]",
  "    [else (string inC)] ))",
  "",
  "(define (escape stringIn)",
  "  (apply string-append (map escapeChar (string->list stringIn))) )",
  "",
  "(define argc (vector-length argv))",
  "",
  "###VERSION###",
  ""
};

vector<string> strClassesSCHEME = {
  "(struct VersData (pre post) #:mutable)",
  "(struct Version (name ver langs) #:mutable)",
  "(struct CodeData (codeVect replVect replFunc) #:mutable)",
  "",
  "(define (replaceVersion line lang vers)",
  "  (let ((data (hash-ref (Version-langs vers) lang)))",
  "    (if (string=? line (Version-name vers))",
  "      (list (string-append (VersData-pre data) (eval (string->symbol (Version-ver vers)) ns) (VersData-post data)))",
  "      (list line) )))",
  "",
  "(define (quoteLinesFunc lang lines)",
  "  (for/list ([l lines])",
  "    (string-append",
  "      \"  \\\"\"",
  "      (escape l)",
  "      (if (string=? lang \"SCHEME\")",
  "        \"\\\"\"",
  "        \"\\\",\"))))",
  "",
  "(define (removeLastComma lines)",
  "  (let ((reverse-lines (reverse lines)))",
  "    (reverse",
  "      (append",
  "        (list",
  "          (substring",
  "            (car reverse-lines)",
  "            0",
  "            (- (string-length (car reverse-lines)) 1)))",
  "        (cdr reverse-lines) ))))",
  "",
  "(define (quoteLines lang lines)",
  "  (let ((outLines (quoteLinesFunc lang lines)))",
  "    (if (string=? lang \"SCHEME\")",
  "      outLines",
  "      (removeLastComma outLines) )))",
  "",
  "(define (noReplace line lang replList)",
  "    (list line) )",
  "",
  "(define (createCodeData langVect prefix func)",
  "  (CodeData",
  "    (make-hash",
  "      (for/list ([l langVect])",
  "        (list",
  "          l",
  "          (string-append prefix l))))",
  "    \'()     ; replVect",
  "    func) ) ; replFunc",
  "",
  "(define (addReplacer co replData)",
  "  (let ((prevReplVect (CodeData-replVect co)))",
  "    (set-CodeData-replVect! co",
  "      (append prevReplVect replData) )))",
  "",
  ""
};

vector<string> strVarSCHEME = {
  "(define-namespace-anchor a)",
  "(define ns (namespace-anchor->namespace a))",
  "",
  "(define strPreCPP (vector",
  "###strPreCPP###",
  "  ))",
  "",
  "(define strClassesCPP (vector",
  "###strClassesCPP###",
  "  ))",
  "",
  "(define strVarCPP (vector",
  "###strVarCPP###",
  "  ))",
  "",
  "(define strPostCPP (vector",
  "###strPostCPP###",
  "  ))",
  "",
  "(define strPrePYTHON (vector",
  "###strPrePYTHON###",
  "  ))",
  "",
  "(define strClassesPYTHON (vector",
  "###strClassesPYTHON###",
  "  ))",
  "",
  "(define strVarPYTHON (vector",
  "###strVarPYTHON###",
  "  ))",
  "",
  "(define strPostPYTHON (vector",
  "###strPostPYTHON###",
  "  ))",
  "",
  "(define strPreSCHEME (vector",
  "###strPreSCHEME###",
  "  ))",
  "",
  "(define strClassesSCHEME (vector",
  "###strClassesSCHEME###",
  "  ))",
  "",
  "(define strVarSCHEME (vector",
  "###strVarSCHEME###",
  "  ))",
  "",
  "(define strPostSCHEME (vector",
  "###strPostSCHEME###",
  "  ))",
  ""
};

vector<string> strPostSCHEME = {
  "(define (returnCode lang co)",
  "  (let ((code (eval (string->symbol (car (hash-ref (CodeData-codeVect co) lang))) ns)))",
  "    (for/list ([c code])",
  "      ((eval (string->symbol (CodeData-replFunc co)) ns)",
  "        c",
  "        lang",
  "        (CodeData-replVect co)))))",
  "",
  "(define (replaceVar line lang replList)",
  "  (let ((data (hash-ref replList line (void))))",
  "    (if (hash-has-key? replList line)",
  "      (quoteLines lang",
  "        (eval (string->symbol (hash-ref replList line)) ns))",
  "      (list line))))",
  "",
  "(define langs (list \"CPP\" \"PYTHON\" \"SCHEME\"))",
  "",
  "(define COPre     (createCodeData langs \"strPre\"     \"replaceVersion\"))",
  "(define COClasses (createCodeData langs \"strClasses\" \"noReplace\"))",
  "(define COVar     (createCodeData langs \"strVar\"     \"replaceVar\"))",
  "(define COPost    (createCodeData langs \"strPost\"    \"noReplace\"))",
  "",
  "(define versReplacer",
  "  (Version \"###VERSION###\" \"version\"",
  "    (hash",
  "      \"CPP\"    (VersData \"string version = \\\"\" \"\\\";\")",
  "      \"PYTHON\" (VersData \"version = \\\"\" \"\\\"\")",
  "      \"SCHEME\" (VersData \"(define version \\\"\" \"\\\")\") )))",
  "",
  "(define (replaceVars langs)",
  "  (define data",
  "    (map ",
  "      (lambda (l)",
  "        (list",
  "          (cons",
  "            (string-append \"###strPre\" l \"###\")",
  "            (string-append \"strPre\" l))",
  "          (cons",
  "            (string-append \"###strClasses\" l \"###\")",
  "            (string-append \"strClasses\" l))",
  "          (cons",
  "            (string-append \"###strVar\" l \"###\")",
  "            (string-append \"strVar\" l))",
  "          (cons",
  "            (string-append \"###strPost\" l \"###\")",
  "            (string-append \"strPost\" l))))",
  "      langs))",
  "  (define (iter in [out \'()])",
  "    (if (null? in)",
  "      out",
  "      (iter (cdr in) (append out (car in)))))",
  "  (make-hash",
  "    (iter data)))",
  "",
  "(define cmdLine",
  "  (hash",
  "    \"--cpp\"    \"CPP\"",
  "    \"--python\" \"PYTHON\"",
  "    \"--scheme\" \"SCHEME\"))",
  "",
  "(define language",
  "  (for/first ([c (vector->list argv)]",
  "              #:when (hash-has-key? cmdLine c))",
  "    (hash-ref cmdLine c)))",
  "",
  "(for ([c (vector->list argv)])",
  "  (when (string=? c \"--help\")",
  "    (display \"Multi-Language Quine \")",
  "    (display version)",
  "    (newline)(newline)",
  "    (display \"optional arguments:\")(newline)",
  "    (display \"  --cpp      Display C++11 Quine\")(newline)",
  "    (display \"  --python   Display Python 2.7 Quine\")(newline)",
  "    (display \"  --scheme   Display Scheme (Racket) Quine\")(newline)",
  "    (exit)))",
  "",
  "(addReplacer COPre versReplacer)",
  "(addReplacer COVar (replaceVars langs))",
  "",
  "(define (printLines lines)",
  "  (for ([lin lines])",
  "    (for ([l lin])",
  "      (display l)",
  "      (newline))))",
  "",
  "",
  "(printLines (returnCode language COPre))",
  "(newline)",
  "(printLines (returnCode language COClasses))",
  "(newline)",
  "(printLines (returnCode language COVar))",
  "(newline)",
  "(printLines (returnCode language COPost))",
  "(newline)",
  ""
};

int main(int argc, char const *argv[])
{
  auto q = Quine(version);
  q.addLang(Language::CPP, &strPreCPP, &strClassesCPP, &strVarCPP, &strPostCPP);
  q.addLang(Language::PYTHON, &strPrePYTHON, &strClassesPYTHON, &strVarPYTHON, &strPostPYTHON);
  q.addLang(Language::SCHEME, &strPreSCHEME, &strClassesSCHEME, &strVarSCHEME, &strPostSCHEME);
  q.init();

  Language lang;

  try 
  {
    TCLAP::CmdLine cmd("Multi-Language Quine", ' ', version);
    TCLAP::SwitchArg lang_cpp("", "cpp", "Display C++11 Quine");
    TCLAP::SwitchArg lang_python("", "python", "Display Python 2.7 Quine");
    TCLAP::SwitchArg lang_scheme("", "scheme", "Display Scheme (Racket) Quine");
    vector<TCLAP::Arg*> xorList = {
      &lang_cpp,
      &lang_python,
      &lang_scheme
    };
    cmd.xorAdd(xorList);
    cmd.parse(argc, argv);

    if(lang_cpp.getValue())
      lang = Language::CPP;
    else if(lang_python.getValue())
      lang = Language::PYTHON;
    else if(lang_scheme.getValue())
      lang = Language::SCHEME;
  }
  catch (TCLAP::ArgException &e)
  {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
  }

  q.print(lang);

  return 0;
}

