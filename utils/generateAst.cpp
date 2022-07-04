// This script will be used to generate the class template for visitor design pattern
// Uses std::shared_ptr for smart pointer memory management
// C++ does not allow templates in abstract classes, we use std::any and cast it later on

#include<iostream>
#include<algorithm>
#include<string>
#include<utility>
#include<cctype>  // std::tolower, std::isspace
#include<vector>
#include<fstream>
#include<sstream>

std::vector<std::string> split(std::string str, const std::string& delim){ 
    std::vector<std::string> tokens;
    std::string::size_type start = 0; // find on str return size_type and not iterator
    std::string::size_type end = str.find(delim);

    // Find the delimiter in string and add tokens to the vector
    while(end != std::string::npos){
        tokens.push_back(str.substr(start,end-start));
        start = end + delim.size();
        end = str.find(delim,start);
    }

    tokens.push_back(str.substr(start,end-start));

    return tokens;
}

std::string trim(std::string str){
    // Find first non-whitespace from left
    // Find first non-whitespace from right

    auto isspace = [] (auto c) { return std::isspace(c);}; // Lambda function

    std::string::iterator start = std::find_if_not(str.begin(), str.end(), isspace);

    // .base() converts a reverse iterator into the corresponding forward iterator
    std::string::iterator end = std::find_if_not(str.rbegin(), str.rend(), isspace).base();

    return {start,end}; // Returns and initialized std::string(start,end) object
}

std::string toLower(std::string str){
    std::string out;

    for(auto x : str){
        out.push_back(std::tolower(x));
    }

    return out;
}

// To convert * -> shared_ptr for auto memory management
std::string fix_pointer(std::string field){
    std::string type = trim(split(field," ")[0]);
    std::string name = trim(split(field," ")[1]);
    std::ostringstream out;
    // If its a pointer to base class, convert to shared_ptr
    if(type.back() == '*'){
        type = type.substr(0,type.end() - type.begin() - 1);
        out << "std::shared_ptr<" << type << ">";
    } else {
        out << type;
    }

    out << " " << name;

    return out.str();
}

// Define the base visitor class
// This will map the desired function to the correct type
void defineVisitor(std::ofstream& writer, std::string baseName, const std::vector<std::string>& types){
    writer << "struct " << baseName << "Visitor {\n";

    for(std::string type : types){
        std::string typeName = trim(split(type,": ")[0]);
        writer << "virtual std::any visit" << typeName << baseName <<"(std::shared_ptr<" << typeName << "> " << toLower(baseName) << ") = 0;\n";
    }
    // Define virtual destructor
    writer << "virtual ~" << baseName << "Visitor() = default;\n" << "};\n";
}

// Defining each type
void defineType(std::ofstream& writer, std::string baseName, std::string className, std::string fieldList){
    writer << "struct " << className << ": " << baseName << ", public std::enable_shared_from_this<" << className << "> {\n";

    // Constructor
    writer << "  " << className << "(";

    std::vector<std::string> fields = split(fieldList, ", ");
    writer << fix_pointer(fields[0]) ;

    for(int i = 1; i < fields.size() ; ++i){
        writer << ", " << fix_pointer(fields[i]);
    }

    writer << ")\n" << "  : ";

    // Store parameters in fields.
    std::string name = split(fields[0], " ")[1];
    writer << name << "{std::move(" << name << ")}";

    for (int i = 1; i < fields.size(); ++i) {
        name = split(fields[i], " ")[1];
        writer << ", " << name << "{std::move(" << name << ")}";
    }

    writer << "\n"
            << "  {}\n";

    // Visitor pattern.
    writer << "\n"
                "  std::any accept(" << baseName << "Visitor& visitor)"
                    " override {\n"
                "    return visitor.visit" << className << baseName <<
                    "(shared_from_this());\n"
                "  }\n";

    // Fields.
    writer << "\n";
    for(std::string field : fields){
        writer << "  const " << fix_pointer(field) << ";\n";
    } 

    writer << "};\n\n";
}

void defineAst(const std::string& output_dir,const std::string& baseName,const std::vector<std::string>& types){
    std::string path = output_dir + "/" + baseName + ".h";
    std::ofstream writer{path};

    writer << "#pragma once\n"
                "\n"
                "#include <any>\n"
                "#include <memory>\n"
                "#include <utility>  // std::move\n"
                "#include <vector>\n"
                "#include \"../scanner/token.h\"\n"
                "\n";

    for (auto type : types){
        writer << "struct " << trim(split(type,":")[0]) <<";\n";
    }

    // The visitor class (Abstract)
    writer<<'\n';
    defineVisitor(writer, baseName, types);

    // The base class
    writer<<'\n' << "struct " << baseName << "\n" <<"{\n"
           " virtual std::any accept(" << baseName << "Visitor& visitor) = 0;\n" << "};\n\n";

    // Define all types
    for(std::string type : types){
        std::string className = trim(split(type,": ")[0]);
        std::string fields = trim(split(type,": ")[1]);
        defineType(writer,baseName, className, fields);
    }
}

int main(int argc, char** argv){
    
    if(argc != 2){
        std::cout<<"Usage: generateAst.cpp <output_dir>\n";
        std::exit(64);
    }
    
    std::string output_dir = argv[1];
    
    // (out_dir,base_name,types)
    defineAst(output_dir, "Expr", {
        "Assign   : Token name, Expr* value",
        "Binary   : Expr* left, Token op, Expr* right",
        "Unary    : Token op, Expr* right",
        "Literal  : std::any value",
        "Grouping : Expr* expression",
        "Ternary  : Expr* left, Expr* middle, Expr* right, Token leftOp, Token middleOp",
        "Variable : Token name"
    });

    // defineAst(output_dir, "Stmt", {
    //     "Expression : Expr* expression",
    //     "Print      : Expr* expression",
    //     "Var        : Token name, Expr* initializer",
    // }
    // );
    return 0;
}