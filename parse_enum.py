import sys


def parse_bitflags(path):
    out = {}
    current_enum = ""
    current_class = ""
    brace = 0
    with open(path, 'r') as f:
        lines = f.readlines()
        for line in lines:
            line = line.strip()
            if len(line) == 0:
                continue
            if "using" in line:
                continue

            if "class" in line and "enum" not in line and current_class == "":
                toks = line.split()
                for i in range(len(toks)):
                    if toks[i] == "class":
                        if toks[i + 1] != "SDLPP_EXPORT":
                            current_class = toks[i + 1]
                        else:
                            current_class = toks[i + 2]
                        break
            if current_class != "":
                if "{" in line:
                    brace = brace + 1
                if "}" in line:
                    brace = brace - 1
                    if brace == 0:
                        current_class = ""
            if "BEGIN_BITFLAGS" in line:
                pos = line.index("(")
                pos2 = line.index(")")
                current_enum = line[pos + 1:pos2].strip()
                if current_class != "":
                    current_enum = current_class + '::' + current_enum
                out[current_enum] = []
            else:
                if current_enum != "":
                    if "END_BITFLAGS" in line:
                        current_enum = ""
                    else:
                        if "FLAG" in line:
                            pos = line.index("(")
                            pos2 = line.index(")")
                            name = line[pos + 1:pos2].strip()
                            if len(name) > 0:
                                out[current_enum].append(name)
    return out


def parse_enum(path):
    out = {}
    current_enum = ""
    current_class = ""
    brace = 0
    with open(path, 'r') as f:
        lines = f.readlines()
        for line in lines:
            line = line.strip()
            if len(line) == 0:
                continue
            if "using" in line:
                continue
            if "class" in line and "enum" not in line and current_class == "":
                toks = line.split()
                for i in range(len(toks)):
                    if toks[i] == "class":
                        if toks[i + 1] != "SDLPP_EXPORT":
                            current_class = toks[i + 1]
                        else:
                            current_class = toks[i + 2]
                        break
            if current_class != "":
                if "{" in line:
                    brace = brace + 1
                if "}" in line:
                    brace = brace - 1
                    if brace == 0:
                        current_class = ""
            if "enum" in line:
                toks = line.split()
                for i in range(len(toks)):
                    if toks[i] != "enum" and toks[i] != "class" and toks[i] != "{":
                        current_enum = toks[i]
                        if current_class != "":
                            current_enum = current_class + '::' + current_enum
                        out[current_enum] = []
                        break
            else:
                if "}" in line:
                    current_enum = ""
                else:
                    if current_enum != "":
                        toks = line.split()
                        for i in range(len(toks)):
                            name = toks[i].strip()
                            if len(name) > 0:
                                out[current_enum].append(name)
                                break
    return out


def create_enum(enums, is_bf):
    for enum in enums.keys():
        vals = enums[enum]
        type = enum
        if is_bf:
            type = enum + "::flag_type"
        descr = enum.replace('::', '_')
        print(f'''
        namespace detail {{
             static inline constexpr std::array<{type}, {len(vals)}> s_vals_of_{descr} = {{
             {'\n\t\t\t'.join(f"{enum}::" + v + "," for v in vals)}
             }};       
        }}
        template <typename T>
        static inline constexpr const decltype(detail::s_vals_of_{descr})& 
        values(typename std::enable_if<std::is_same_v<{enum}, T>>::type* = nullptr) {{
            return detail::s_vals_of_{descr};
        }} 
        template <typename T>
        static inline constexpr auto  
        begin(typename std::enable_if<std::is_same_v<{enum}, T>>::type* = nullptr) {{
            return detail::s_vals_of_{descr}.begin();
        }} 
        template <typename T>
        static inline constexpr auto 
        end(typename std::enable_if<std::is_same_v<{enum}, T>>::type* = nullptr) {{
            return detail::s_vals_of_{descr}.end();
        }} 
        ''')


if __name__ == "__main__":
    pth = sys.argv[1]
    e = parse_enum(pth)
    e2 = parse_bitflags(pth)
    create_enum(e, False)
    create_enum(e2, True)

