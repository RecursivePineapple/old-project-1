
import yaml
import os

inpath = os.path.abspath("../error_codes.yml")
header_outpath = os.path.abspath("src/Common/ErrorCodes.hpp")
cpp_outpath = os.path.abspath("src/Common/ErrorCodes.cpp")

header_include = "Common/ErrorCodes.hpp"

N = 4

def run():
    with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), inpath), "r") as infile:
        data = yaml.load(infile, Loader=yaml.CSafeLoader)
    
    with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), header_outpath), "w") as outfile:
        outfile.writelines([
            "#pragma once\n",
            "\n",
            "namespace error {\n",
            "constexpr int Success = 0;\n",
        ])
        for i, (module, codes) in enumerate(data.items()):
            outfile.writelines([
                "struct %s {\n" % module,
                *["    static constexpr int %s = %d;\n" % (error, (i+1) * pow(10, N) + ei) for ei, (error, _) in enumerate(codes.items())],
                "};\n\n"
            ])
        
        outfile.writelines([
            "const char* GetErrorString(int code);\n"
            "}\n"
        ])
    
    with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), cpp_outpath), "w") as outfile:
        outfile.writelines([
            "#pragma target server\n",
            "\n",
            "#include \"%s\"\n" % header_include,
            "\n",
            "namespace error {\n",
            "\n",
            "const char* GetErrorString(int code)\n",
            "{\n",
            "    switch(code)\n",
            "    {\n",
            "        default: {\n",
            "            return nullptr;\n",
            "        }\n",
            "        case ::error::Success: {\n",
            "            return \"Success\";\n",
            "        }\n",
        ])
        for i, (module, codes) in enumerate(data.items()):
            for error, error_text in codes.items():
                outfile.writelines([
                    "        case ::error::%s::%s: {\n" % (module, error),
                    "            return \"%s\";\n" % error_text,
                    "        }\n",
                ])

        outfile.writelines([
            "    }\n",
            "}\n",
            "}\n"
        ])

if __name__ == "__main__":
    run()
