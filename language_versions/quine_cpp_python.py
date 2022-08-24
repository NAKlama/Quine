#!/usr/bin/python
#
# Multi-Language Quine
# Author: Nina Alexandra Klama
# Languages: C++11, Python
#

import argparse
import string

version = "v1.0"


def escape(lang, inStr):
  out = ""
  for c in inStr:
    if c == '\"':
      out += "\\\""
    elif c == '\\':
      out += "\\\\"
    elif c == '\'':
      out += "\\\'"
    else:
      out += c
  return out


class ReplaceObject:
  pass


class ReplaceVectorString (ReplaceObject):
  def __init__(self, name, inVar):
    self.replName    = name
    self.var         = inVar

  def getReplString(self):
    return self.replName

  def retCode(self, lang):
    ret = []
    for vLine in self.var:
      outLine  = "  \""
      outLine += escape(lang, vLine)
      outLine += "\","
      ret.append(outLine)
    ret[len(ret)-1] = ret[len(ret)-1][0:len(ret[len(ret)-1])-1]
    return ret


class ReplaceVariableString (ReplaceObject):
  def __init__(self, name, var):
    self.replName   = name
    self.var        = var
    self.strings    = []

  def getReplString(self):
    return self.replName

  def setString(self, lang, pre, post):
    self.strings.append({'lang': lang, 'pre': pre, 'post': post})

  def retCode(self, lang):
    ret = []
    strings = None
    for s in self.strings:
      if s['lang'] == lang:
        strings = s
    out  = strings['pre']
    out += escape(lang, self.var)
    out += strings['post']
    ret.append(out)
    return ret


class CodeObject:
  def __init__(self):
    self.replacements = []
    self.code         = []

  def addCode(self, lang, codeIn):
    self.code.append({'lang': lang, 'code': codeIn})

  def addReplacement(self, ro):
    replStr = ro.getReplString()
    self.replacements.append({'str': replStr, 'obj': ro})

  def returnCode(self, lang):
    out = []
    lines = None
    for c in self.code:
      if c['lang'] == lang:
        lines = c['code']
    for line in lines:
      if len(self.replacements) > 0:
        found = False
        for repl in self.replacements:
          if repl['str'] == line:
            found = True
            for retStr in repl['obj'].retCode(lang):
              out.append(retStr)
        if not found:
          out.append(line)
      else:
        out.append(line)
    return out

  def getCode(self, lang):
    ret = None
    for c in self.code:
      if c['lang'] == lang:
        ret = c['code']
    return ret


class Quine:
  def __init__(self, version):
    self.version   = version
    self.COPre     = CodeObject()
    self.COClasses = CodeObject()
    self.COVar     = CodeObject()
    self.COPost    = CodeObject()

  def addLang(self, lang, pre, classes, var, post):
    self.COPre.addCode(lang, pre)
    self.COClasses.addCode(lang, classes)
    self.COVar.addCode(lang, var)
    self.COPost.addCode(lang, post)

  def init(self):
    replVersion = ReplaceVariableString("###VERSION###", self.version)
    replVersion.setString("CPP",    "string version = \"", "\";")
    replVersion.setString("PYTHON", "version = \"","\"")
    self.COPre.addReplacement(replVersion)

    replPreCPP        = ReplaceVectorString("###strPreCPP###",        self.COPre.getCode("CPP"))
    replClassesCPP    = ReplaceVectorString("###strClassesCPP###",    self.COClasses.getCode("CPP"))
    replVarCPP        = ReplaceVectorString("###strVarCPP###",        self.COVar.getCode("CPP"))
    replPostCPP       = ReplaceVectorString("###strPostCPP###",       self.COPost.getCode("CPP"))
    replPrePYTHON     = ReplaceVectorString("###strPrePYTHON###",     self.COPre.getCode("PYTHON"))
    replClassesPYTHON = ReplaceVectorString("###strClassesPYTHON###", self.COClasses.getCode("PYTHON"))
    replVarPYTHON     = ReplaceVectorString("###strVarPYTHON###",     self.COVar.getCode("PYTHON"))
    replPostPYTHON    = ReplaceVectorString("###strPostPYTHON###",    self.COPost.getCode("PYTHON"))

    self.COVar.addReplacement(replPreCPP)
    self.COVar.addReplacement(replClassesCPP)
    self.COVar.addReplacement(replVarCPP)
    self.COVar.addReplacement(replPostCPP)
    self.COVar.addReplacement(replPrePYTHON)
    self.COVar.addReplacement(replClassesPYTHON)
    self.COVar.addReplacement(replVarPYTHON)
    self.COVar.addReplacement(replPostPYTHON)

  def output(self, lang):
    for line in self.COPre.returnCode(lang):
      print line
    for line in self.COClasses.returnCode(lang):
      print line
    for line in self.COVar.returnCode(lang):
      print line
    for line in self.COPost.returnCode(lang):
      print line


class ArgumentError(Exception):
  pass


strPreCPP = [
  "/*",
  " * Multi-Language Quine",
  " * Author: Nina Alexandra Klama",
  " * Languages: C++11, Python",
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
  "  PYTHON",
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
  ]

strClassesCPP = [
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
  "        outLine += \"\\\",\";",
  "        ret.push_back(outLine);",
  "      }",
  "      ret.back() = ret.back().substr(0, ret.back().length()-1);",
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
  "      COPre.addReplacement(replVersion);",
  "",
  "      auto replPreCPP          = new ReplaceVectorString(\"###strPreCPP###\",        COPre.getCode(Language::CPP));",
  "      auto replClassesCPP      = new ReplaceVectorString(\"###strClassesCPP###\",    COClasses.getCode(Language::CPP));",
  "      auto replVarCPP          = new ReplaceVectorString(\"###strVarCPP###\",        COVar.getCode(Language::CPP));",
  "      auto replPostCPP         = new ReplaceVectorString(\"###strPostCPP###\",       COPost.getCode(Language::CPP));",
  "      auto *replPrePYTHON      = new ReplaceVectorString(\"###strPrePYTHON###\",     COPre.getCode(Language::PYTHON));",
  "      auto *replClassesPYTHON  = new ReplaceVectorString(\"###strClassesPYTHON###\", COClasses.getCode(Language::PYTHON));",
  "      auto *replVarPYTHON      = new ReplaceVectorString(\"###strVarPYTHON###\",     COVar.getCode(Language::PYTHON));",
  "      auto *replPostPYTHON     = new ReplaceVectorString(\"###strPostPYTHON###\",    COPost.getCode(Language::PYTHON));",
  "",
  "      COVar.addReplacement(replPreCPP);",
  "      COVar.addReplacement(replClassesCPP);",
  "      COVar.addReplacement(replVarCPP);",
  "      COVar.addReplacement(replPostCPP);",
  "      COVar.addReplacement(replPrePYTHON);",
  "      COVar.addReplacement(replClassesPYTHON);",
  "      COVar.addReplacement(replVarPYTHON);",
  "      COVar.addReplacement(replPostPYTHON);",
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
  ]

strVarCPP = [
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
  ""
  ]

strPostCPP = [
  "int main(int argc, char const *argv[])",
  "{",
  "  auto q = Quine(version);",
  "  q.addLang(Language::CPP, &strPreCPP, &strClassesCPP, &strVarCPP, &strPostCPP);",
  "  q.addLang(Language::PYTHON, &strPrePYTHON, &strClassesPYTHON, &strVarPYTHON, &strPostPYTHON);",
  "  q.init();",
  "",
  "  Language lang;",
  "",
  "  try ",
  "  {",
  "    TCLAP::CmdLine cmd(\"Multi-Language Quine\", \' \', version);",
  "    TCLAP::SwitchArg lang_cpp(\"\", \"cpp\", \"Display C++11 Quine\");",
  "    TCLAP::SwitchArg lang_python(\"\", \"python\", \"Display Python 2.7 Quine\");",
  "    vector<TCLAP::Arg*> xorList = {",
  "      &lang_cpp,",
  "      &lang_python",
  "    };",
  "    cmd.xorAdd(xorList);",
  "    cmd.parse(argc, argv);",
  "",
  "    if(lang_cpp.getValue())",
  "      lang = Language::CPP;",
  "    else if(lang_python.getValue())",
  "      lang = Language::PYTHON;",
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
  ]

strPrePYTHON = [
  "#!/usr/bin/python",
  "#",
  "# Multi-Language Quine",
  "# Author: Nina Alexandra Klama",
  "# Languages: C++11, Python",
  "#",
  "",
  "import argparse",
  "import string",
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
  ]

strClassesPYTHON = [
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
  "      outLine += \"\\\",\"",
  "      ret.append(outLine)",
  "    ret[len(ret)-1] = ret[len(ret)-1][0:len(ret[len(ret)-1])-1]",
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
  "",
  "    self.COVar.addReplacement(replPreCPP)",
  "    self.COVar.addReplacement(replClassesCPP)",
  "    self.COVar.addReplacement(replVarCPP)",
  "    self.COVar.addReplacement(replPostCPP)",
  "    self.COVar.addReplacement(replPrePYTHON)",
  "    self.COVar.addReplacement(replClassesPYTHON)",
  "    self.COVar.addReplacement(replVarPYTHON)",
  "    self.COVar.addReplacement(replPostPYTHON)",
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
  ]

strVarPYTHON = [
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
  ""
  ]

strPostPYTHON = [
  "if __name__ == \"__main__\":",
  "  q = Quine(version)",
  "  q.addLang(\"CPP\", strPreCPP, strClassesCPP, strVarCPP, strPostCPP)",
  "  q.addLang(\"PYTHON\", strPrePYTHON, strClassesPYTHON, strVarPYTHON, strPostPYTHON)",
  "  q.init()",
  "",
  "  argParser = argparse.ArgumentParser(description=\"Multi-Language Quine %s\" % version)",
  "  argParser.add_argument(\'--cpp\',    action=\"store_true\", help=\"Display C++11 Quine\")",
  "  argParser.add_argument(\'--python\', action=\"store_true\", help=\"Display Python 2.7 Quine\")",
  "  args = vars(argParser.parse_args())",
  "",
  "  if args[\'cpp\'] and args[\'python\']:",
  "    raise ArgumentError(\"\\nERROR:Only specify one language please!\")",
  "",
  "  if args[\'cpp\']:",
  "    lang = \"CPP\"",
  "  else:",
  "    lang = \"PYTHON\"",
  "",
  "  q.output(lang)"
  ]

if __name__ == "__main__":
  q = Quine(version)
  q.addLang("CPP", strPreCPP, strClassesCPP, strVarCPP, strPostCPP)
  q.addLang("PYTHON", strPrePYTHON, strClassesPYTHON, strVarPYTHON, strPostPYTHON)
  q.init()

  argParser = argparse.ArgumentParser(description="Multi-Language Quine %s" % version)
  argParser.add_argument('--cpp',    action="store_true", help="Display C++11 Quine")
  argParser.add_argument('--python', action="store_true", help="Display Python 2.7 Quine")
  args = vars(argParser.parse_args())

  if args['cpp'] and args['python']:
    raise ArgumentError("\nERROR:Only specify one language please!")

  if args['cpp']:
    lang = "CPP"
  else:
    lang = "PYTHON"

  q.output(lang)
