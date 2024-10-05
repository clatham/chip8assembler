#include <cstdio>
#include <fstream>
#include <map>
#include <string>
#include <vector>


enum InstructionEnum
{
    INST_DEFINEBYTE,
    INST_DEFINEWORD,

    INST_CLS,
    INST_RET,
    INST_JP_ADDR,
    INST_CALL_ADDR,
    INST_SE_VX_NN,
    INST_SNE_VX_NN,
    INST_SE_VX_VY,
    INST_LD_VX_NN,
    INST_ADD_VX_NN,
    INST_LD_VX_VY,
    INST_OR_VX_VY,
    INST_AND_VX_VY,
    INST_XOR_VX_VY,
    INST_ADD_VX_VY,
    INST_SUB_VX_VY,
    INST_SHR_VX_VY,
    INST_SUBN_VX_VY,
    INST_SHL_VX_VY,
    INST_SNE_VX_VY,
    INST_LD_I_ADDR,
    INST_JP_V0_ADDR,
    INST_RND_VX_NN,
    INST_DRW_VX_VY_N,
    INST_SKP_VX,
    INST_SKNP_VX,
    INST_LD_VX_DT,
    INST_LD_VX_N,
    INST_LD_DT_VX,
    INST_LD_ST_VX,
    INST_ADD_I_VX,
    INST_LD_F_VX,
    INST_LD_B_VX,
    INST_LD_I_VX,
    INST_LD_VX_I
};


enum RegisterEnum
{
    REG_V0,
    REG_V1,
    REG_V2,
    REG_V3,
    REG_V4,
    REG_V5,
    REG_V6,
    REG_V7,
    REG_V8,
    REG_V9,
    REG_VA,
    REG_VB,
    REG_VC,
    REG_VD,
    REG_VE,
    REG_VF,
    REG_B,
    REG_DT,
    REG_F,
    REG_I,
    REG_I_INDIRECT,
    REG_K,
    REG_ST
};


struct Statement
{
    uint8_t instruction;
    uint8_t size;
    uint16_t offset;
    
    uint16_t value;
    
    uint8_t x;
    uint8_t y;
    uint8_t n;
    uint8_t nn;
    std::string address;
};


struct Token
{
    int column;
    std::string text;
};


// global variables
std::map<std::string, int> g_symbolTable;
std::vector<Statement> g_statements;
int g_lineNumber;


//
//
//

std::string trim(const std::string& text)
{
    size_t first = text.find_first_not_of(" \f\n\r\t\v");
    size_t last = text.find_last_not_of(" \f\n\r\t\v");

    return std::string(text.begin() + first, text.begin() + last + 1);
}


//
//
//

bool parseInteger(std::string& text, int& result, int maxValue = 0)
{
    text = trim(text);
    
    if(text[0] == '$')
        result = (int) strtol(text.data() + 1, NULL, 16);
    else if(text[0] == '%')
        result = (int) strtol(text.data() + 1, NULL, 2);
    else
        result = (int) strtol(text.data(), NULL, 0);

    if(maxValue  &&  result > maxValue)
        return false;
    
    return true;
}


//
//
//

bool parseRegister(std::string& text, int& result)
{
    text = trim(text);
    
    if(text.compare("b") == 0)
        result = REG_B;
    else if(text.compare("dt") == 0)
        result = REG_DT;
    else if(text.compare("f") == 0)
        result = REG_F;
    else if(text.compare("i") == 0)
        result = REG_I;
    else if(text.compare("[i]") == 0)
        result = REG_I_INDIRECT;
    else if(text.compare("k") == 0)
        result = REG_K;
    else if(text.compare("st") == 0)
        result = REG_ST;
    else if(text.compare("v0") == 0)
        result = REG_V0;
    else if(text.compare("v1") == 0)
        result = REG_V1;
    else if(text.compare("v2") == 0)
        result = REG_V2;
    else if(text.compare("v3") == 0)
        result = REG_V3;
    else if(text.compare("v4") == 0)
        result = REG_V4;
    else if(text.compare("v5") == 0)
        result = REG_V5;
    else if(text.compare("v6") == 0)
        result = REG_V6;
    else if(text.compare("v7") == 0)
        result = REG_V7;
    else if(text.compare("v8") == 0)
        result = REG_V8;
    else if(text.compare("v9") == 0)
        result = REG_V9;
    else if(text.compare("va") == 0)
        result = REG_VA;
    else if(text.compare("vb") == 0)
        result = REG_VB;
    else if(text.compare("vc") == 0)
        result = REG_VC;
    else if(text.compare("vd") == 0)
        result = REG_VD;
    else if(text.compare("ve") == 0)
        result = REG_VE;
    else if(text.compare("vf") == 0)
        result = REG_VF;
    else
        return false;
    
    return true;
}


//
//
//

std::vector<Token> split(const std::string& line)
{
    std::vector<Token> tokens;
    Token token = { -1, "" };
    int column = 0;
    
    while(column < line.size()  &&  line[column] != ';')
    {
        if(line[column] == ':')
        {
            if(token.text.empty())
            {
                fprintf(stderr, "%d, %d:  label name must preceed colon\n", g_lineNumber, column);
                exit(1);
            }
            else
            {
                token.text += ":";

                tokens.push_back(token);
                
                token.column = -1;
                token.text.clear();
            }
        }
        else if(isspace(line[column])  ||  line[column] == ',')
        {
            if(!token.text.empty())
            {
                tokens.push_back(token);
                
                token.column = -1;
                token.text.clear();
            }
        }
        else
        {
            if(token.column == -1)
                token.column = column;

            token.text += std::tolower(line[column]);
        }

        ++column;
    }

    if(!token.text.empty())
    {
        tokens.push_back(token);
    }

    return tokens;
}


//
//
//

bool readInput(const std::string& inputFilename)
{
    // open input file
    std::ifstream inputFile;
    inputFile.open(inputFilename);

    if(!inputFile)
    {
        fprintf(stderr, "error opening input file \"%s\"\n", inputFilename.c_str());
        return false;
    }
    
    
    // initialize some important variables
    g_lineNumber = 1;
    uint16_t offset = 0x0200;  // address where chip-8 files are loaded
    Statement statement;
    
    
    // read file line-by-line
    std::string line;
    
    while(getline(inputFile, line))
    {
        auto tokens = split(line);

        if(tokens.empty())
        {
            ++g_lineNumber;
            continue;
        }
        

        // handle optional label
        if(tokens[0].text[tokens[0].text.size() - 1] == ':')
        {
            tokens[0].text.pop_back();
            g_symbolTable[tokens[0].text] = offset;
            
            if(tokens.size() < 2)
                continue;
            else
                tokens = std::vector<Token>(tokens.begin() + 1, tokens.end());
        }
        
        
        // handle '.org' directive
        if(tokens[0].text.compare(".org") == 0)
        {
            int origin;
            
            if(tokens.size() != 2  ||  !parseInteger(tokens[1].text, origin, 0xffff))
            {
                fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to '.org'\n", g_lineNumber);
                return false;
            }

            offset = origin;
        }
        
        // handle '.byte' directive
        else if(tokens[0].text.compare(".byte") == 0)
        {
            if(tokens.size() < 2)
            {
                fprintf(stderr, "line %d:  missing argument to '.byte'\n", g_lineNumber);
                return false;
            }

            for(int index = 1; index < tokens.size(); ++index)
            {
                int byte;
                
                if(!parseInteger(tokens[index].text, byte, 0xff))
                {
                    fprintf(stderr, "line %d, col %d:  invalid argument to '.byte'\n", g_lineNumber, tokens[index].column);
                    return false;
                }
                
                
                statement.instruction = INST_DEFINEBYTE;
                statement.offset = offset;
                statement.size = 1;
                statement.value = byte;
                
                g_statements.push_back(statement);
                
                offset += statement.size;
            }
        }
        
        // handle '.word' directive
        else if(tokens[0].text.compare(".word") == 0)
        {
            if(tokens.size() < 2)
            {
                fprintf(stderr, "line %d:  missing argument to '.word'\n", g_lineNumber);
                return false;
            }

            for(int index = 1; index < tokens.size(); ++index)
            {
                int word;
                
                if(!parseInteger(tokens[index].text, word, 0xffff))
                {
                    fprintf(stderr, "line %d, col %d:  invalid argument to '.word'\n", g_lineNumber, tokens[index].column);
                    return false;
                }
                
                
                statement.instruction = INST_DEFINEWORD;
                statement.offset = offset;
                statement.size = 2;
                statement.value = word;
                
                g_statements.push_back(statement);
                
                offset += statement.size;
            }
        }

        // handle 'add vx, nn' or 'add vx, vy' or 'add i, vx' instruction
        else if(tokens[0].text.compare("add") == 0)
        {
            int reg1, reg2, byte;
            
            if(parseRegister(tokens[2].text, reg2))
            {
                if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1)  ||  reg2 < REG_V0  ||  reg2 > REG_VF)
                {
                    fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'add'\n", g_lineNumber);
                    return false;
                }
                
                
                if(reg1 == REG_I)
                {
                    statement.instruction = INST_ADD_I_VX;
                    statement.offset = offset;
                    statement.size = 2;
                    statement.x = reg2;
                    
                    g_statements.push_back(statement);
                    
                    offset += statement.size;
                }
                else if(reg1 >= REG_V0  &&  reg1 <= REG_VF)
                {
                    statement.instruction = INST_ADD_VX_VY;
                    statement.offset = offset;
                    statement.size = 2;
                    statement.x = reg1;
                    statement.y = reg2;
                    
                    g_statements.push_back(statement);
                    
                    offset += statement.size;
                }
                else
                {
                    fprintf(stderr, "line %d:  invalid argument to 'add'\n", g_lineNumber);
                    return false;
                }
            }
            else
            {
                if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1)  ||  !parseInteger(tokens[2].text, byte, 0xff))
                {
                    fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'add'\n", g_lineNumber);
                    return false;
                }
                
                
                statement.instruction = INST_ADD_VX_NN;
                statement.offset = offset;
                statement.size = 2;
                statement.x = reg1;
                statement.nn = byte;
                
                g_statements.push_back(statement);
                
                offset += statement.size;
            }
        }

        // handle 'and vx, vy' instruction
        else if(tokens[0].text.compare("and") == 0)
        {
            int reg1, reg2;
            
            if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1)  ||  !parseRegister(tokens[2].text, reg2))
            {
                fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'and'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_AND_VX_VY;
            statement.offset = offset;
            statement.size = 2;
            statement.x = reg1;
            statement.y = reg2;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'call addr' instruction
        else if(tokens[0].text.compare("call") == 0)
        {
            if(tokens.size() != 2)
            {
                fprintf(stderr, "line %d:  missing or unexpected argument to 'call'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_CALL_ADDR;
            statement.offset = offset;
            statement.size = 2;
            statement.address = tokens[1].text;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'cls' instruction
        else if(tokens[0].text.compare("cls") == 0)
        {
            if(tokens.size() != 1)
            {
                fprintf(stderr, "line %d:  unexpected argument to 'cls'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_CLS;
            statement.offset = offset;
            statement.size = 2;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'drw vx, vy, n' instruction
        else if(tokens[0].text.compare("drw") == 0)
        {
            int reg1, reg2, nibble;
            
            if(tokens.size() != 4  ||  !parseRegister(tokens[1].text, reg1)  ||  !parseRegister(tokens[2].text, reg2)  ||  !parseInteger(tokens[3].text, nibble, 0xf))
            {
                fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'drw'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_DRW_VX_VY_N;
            statement.offset = offset;
            statement.size = 2;
            statement.x = reg1;
            statement.y = reg2;
            statement.n = nibble;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'jp addr' or 'jp v0, addr' instruction
        else if(tokens[0].text.compare("jp") == 0)
        {
            int reg1;

            if(parseRegister(tokens[1].text, reg1)  &&  reg1 == REG_V0)
            {
                if(tokens.size() != 3)
                {
                    fprintf(stderr, "line %d:  missing or unexpected argument to 'jp'\n", g_lineNumber);
                    return false;
                }
                
                
                statement.instruction = INST_JP_V0_ADDR;
                statement.offset = offset;
                statement.size = 2;
                statement.address = tokens[1].text;
                
                g_statements.push_back(statement);
                
                offset += statement.size;
            }
            else
            {
                if(tokens.size() != 2)
                {
                    fprintf(stderr, "line %d:  missing or unexpected argument to 'jp'\n", g_lineNumber);
                    return false;
                }
                
                
                statement.instruction = INST_JP_ADDR;
                statement.offset = offset;
                statement.size = 2;
                statement.address = tokens[1].text;
                
                g_statements.push_back(statement);
                
                offset += statement.size;
            }
        }

        // handle various 'ld' instruction
        else if(tokens[0].text.compare("ld") == 0)
        {
            int reg1, reg2, byte;

            if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1))
            {
                fprintf(stderr, "line %d:  missing or unexpected argument to 'ld'\n", g_lineNumber);
                return false;
            }

            if(reg1 >= REG_V0  &&  reg1 <= REG_VF)
            {
                if(!parseRegister(tokens[2].text, reg2))
                {
                    if(!parseInteger(tokens[2].text, byte, 0xff))
                    {
                        fprintf(stderr, "line %d:  invalid argument to 'ld'\n", g_lineNumber);
                        return false;
                    }


                    statement.instruction = INST_LD_VX_NN;
                    statement.offset = offset;
                    statement.size = 2;
                    statement.x = reg1;
                    statement.nn = byte;
                    
                    g_statements.push_back(statement);
                    
                    offset += statement.size;
                }
                else
                {
                    switch(reg2)
                    {
                        case REG_DT:  statement.instruction = INST_LD_VX_DT;  break;
                        case REG_I_INDIRECT:  statement.instruction = INST_LD_VX_I;  break;
                        case REG_K:   statement.instruction = INST_LD_VX_N;  break;
                        case REG_V0:
                        case REG_V1:
                        case REG_V2:
                        case REG_V3:
                        case REG_V4:
                        case REG_V5:
                        case REG_V6:
                        case REG_V7:
                        case REG_V8:
                        case REG_V9:
                        case REG_VA:
                        case REG_VB:
                        case REG_VC:
                        case REG_VD:
                        case REG_VE:
                        case REG_VF:  statement.instruction = INST_LD_VX_VY;  break;
                        default:
                            fprintf(stderr, "line %d:  invalid argument to 'ld'\n", g_lineNumber);
                            return false;
                    }
                    
                    statement.offset = offset;
                    statement.size = 2;
                    statement.x = reg1;
                    statement.y = reg2;
                    
                    g_statements.push_back(statement);
                    
                    offset += statement.size;
                }
            }
            else if(reg1 == REG_I)
            {
                statement.instruction = INST_LD_I_ADDR;
                statement.offset = offset;
                statement.size = 2;
                statement.address = tokens[2].text;
                
                g_statements.push_back(statement);
                
                offset += statement.size;
            }
            else
            {
                if(!parseRegister(tokens[2].text, reg2)  ||  reg2 < REG_V0  ||  reg2 > REG_VF)
                {
                    fprintf(stderr, "line %d:  invalid argument to 'ld'\n", g_lineNumber);
                    return false;
                }
                
                
                switch(reg1)
                {
                    case REG_B:   statement.instruction = INST_LD_B_VX;   break;
                    case REG_DT:  statement.instruction = INST_LD_DT_VX;  break;
                    case REG_F:   statement.instruction = INST_LD_F_VX;   break;
                    case REG_I_INDIRECT:  statement.instruction = INST_LD_I_VX;  break;
                    case REG_ST:  statement.instruction = INST_LD_ST_VX;  break;
                    default:
                        fprintf(stderr, "line %d:  invalid argument to 'ld'\n", g_lineNumber);
                        return false;
                }
                
                statement.offset = offset;
                statement.size = 2;
                statement.x = reg2;
                
                g_statements.push_back(statement);
                
                offset += statement.size;
            }
        }

        // handle 'or vx, vy' instruction
        else if(tokens[0].text.compare("or") == 0)
        {
            int reg1, reg2;
            
            if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1)  ||  !parseRegister(tokens[2].text, reg2))
            {
                fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'or'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_OR_VX_VY;
            statement.offset = offset;
            statement.size = 2;
            statement.x = reg1;
            statement.y = reg2;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'ret' instruction
        else if(tokens[0].text.compare("ret") == 0)
        {
            if(tokens.size() != 1)
            {
                fprintf(stderr, "line %d:  unexpected argument to 'ret'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_RET;
            statement.offset = offset;
            statement.size = 2;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'rnd vx, nn' instruction
        else if(tokens[0].text.compare("rnd") == 0)
        {
            int reg1, byte;
            
            if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1)  ||  !parseInteger(tokens[2].text, byte, 0xff))
            {
                fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'rnd'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_RND_VX_NN;
            statement.offset = offset;
            statement.size = 2;
            statement.x = reg1;
            statement.nn = byte;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'se vx, nn' or 'se vx, vy' instruction
        else if(tokens[0].text.compare("se") == 0)
        {
            int reg1, reg2, byte;
            
            if(parseRegister(tokens[2].text, reg2))
            {
                if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1))
                {
                    fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'se'\n", g_lineNumber);
                    return false;
                }
                
                
                statement.instruction = INST_SE_VX_VY;
                statement.offset = offset;
                statement.size = 2;
                statement.x = reg1;
                statement.y = reg2;
                
                g_statements.push_back(statement);
                
                offset += statement.size;
            }
            else
            {
                if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1)  ||  !parseInteger(tokens[2].text, byte, 0xff))
                {
                    fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'se'\n", g_lineNumber);
                    return false;
                }
                
                
                statement.instruction = INST_SE_VX_NN;
                statement.offset = offset;
                statement.size = 2;
                statement.x = reg1;
                statement.nn = byte;
                
                g_statements.push_back(statement);
                
                offset += statement.size;
            }
        }

        // handle 'shl vx' instruction
        else if(tokens[0].text.compare("shl") == 0)
        {
            int reg1;
            
            if(tokens.size() != 2  ||  !parseRegister(tokens[1].text, reg1))
            {
                fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'shl'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_SHL_VX_VY;
            statement.offset = offset;
            statement.size = 2;
            statement.x = reg1;
            statement.y = 0;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'shr vx' instruction
        else if(tokens[0].text.compare("shr") == 0)
        {
            int reg1;
            
            if(tokens.size() != 2  ||  !parseRegister(tokens[1].text, reg1))
            {
                fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'shr'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_SHR_VX_VY;
            statement.offset = offset;
            statement.size = 2;
            statement.x = reg1;
            statement.y = 0;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'sne vx, nn' or 'sne vx, vy' instruction
        else if(tokens[0].text.compare("sne") == 0)
        {
            int reg1, reg2, byte;
            
            if(parseRegister(tokens[2].text, reg2))
            {
                if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1))
                {
                    fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'sne'\n", g_lineNumber);
                    return false;
                }
                
                
                statement.instruction = INST_SNE_VX_VY;
                statement.offset = offset;
                statement.size = 2;
                statement.x = reg1;
                statement.y = reg2;
                
                g_statements.push_back(statement);
                
                offset += statement.size;
            }
            else
            {
                if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1)  ||  !parseInteger(tokens[2].text, byte, 0xff))
                {
                    fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'sne'\n", g_lineNumber);
                    return false;
                }
                
                
                statement.instruction = INST_SNE_VX_NN;
                statement.offset = offset;
                statement.size = 2;
                statement.x = reg1;
                statement.nn = byte;
                
                g_statements.push_back(statement);
                
                offset += statement.size;
            }
        }

        // handle 'sknp vx' instruction
        else if(tokens[0].text.compare("sknp") == 0)
        {
            int reg1;

            if(tokens.size() != 2  ||  !parseRegister(tokens[1].text, reg1))
            {
                fprintf(stderr, "line %d:  missing or unexpected argument to 'sknp'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_SKNP_VX;
            statement.offset = offset;
            statement.size = 2;
            statement.x = reg1;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'skp vx' instruction
        else if(tokens[0].text.compare("skp") == 0)
        {
            int reg1;

            if(tokens.size() != 2  ||  !parseRegister(tokens[1].text, reg1))
            {
                fprintf(stderr, "line %d:  missing or unexpected argument to 'skp'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_SKP_VX;
            statement.offset = offset;
            statement.size = 2;
            statement.x = reg1;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'sub vx, vy' instruction
        else if(tokens[0].text.compare("sub") == 0)
        {
            int reg1, reg2;
            
            if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1)  ||  !parseRegister(tokens[2].text, reg2))
            {
                fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'sub'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_SUB_VX_VY;
            statement.offset = offset;
            statement.size = 2;
            statement.x = reg1;
            statement.y = reg2;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'subn vx, vy' instruction
        else if(tokens[0].text.compare("subn") == 0)
        {
            int reg1, reg2;
            
            if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1)  ||  !parseRegister(tokens[2].text, reg2))
            {
                fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'subn'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_SUBN_VX_VY;
            statement.offset = offset;
            statement.size = 2;
            statement.x = reg1;
            statement.y = reg2;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle 'xor vx, vy' instruction
        else if(tokens[0].text.compare("xor") == 0)
        {
            int reg1, reg2;
            
            if(tokens.size() != 3  ||  !parseRegister(tokens[1].text, reg1)  ||  !parseRegister(tokens[2].text, reg2))
            {
                fprintf(stderr, "line %d:  missing, unexpected, or invalid argument(s) to 'xor'\n", g_lineNumber);
                return false;
            }
            
            
            statement.instruction = INST_XOR_VX_VY;
            statement.offset = offset;
            statement.size = 2;
            statement.x = reg1;
            statement.y = reg2;
            
            g_statements.push_back(statement);
            
            offset += statement.size;
        }

        // handle unknown instructions
        else
        {
            fprintf(stderr, "line %d:  unknown instruction %s\n", g_lineNumber, tokens[0].text.c_str());
            return false;
        }

        
        ++g_lineNumber;
    }
    
    
    return true;
}


//
//
//

bool writeOutput(const std::string& outputFilename)
{
    // open output file
    std::ofstream outputFile;
    outputFile.open(outputFilename, std::ios::binary);

    if(!outputFile)
    {
        fprintf(stderr, "error opening output file \"%s\"\n", outputFilename.c_str());
        return false;
    }
    
    
    uint8_t byte;
    uint16_t word;
    int address;
    std::vector<Statement>::iterator statementItor;
    std::map<std::string, int>::iterator symbolItor;
    
    for(statementItor = g_statements.begin(); statementItor != g_statements.end(); ++statementItor)
    {
        switch(statementItor->instruction)
        {
            case INST_DEFINEBYTE:
                byte = statementItor->value;
                
                outputFile.write((char *) &byte, 1);
                break;
            
            case INST_DEFINEWORD:
                word = statementItor->value;
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_CLS:
                word = 0x00e0;
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_RET:
                word = 0x00ee;
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_JP_ADDR:
                if((symbolItor = g_symbolTable.find(statementItor->address)) != g_symbolTable.end())
                    word = 0x1000 | symbolItor->second;
                else if(parseInteger(statementItor->address, address, 0xfff))
                    word = 0x1000 | address;
                
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_CALL_ADDR:
                if((symbolItor = g_symbolTable.find(statementItor->address)) != g_symbolTable.end())
                    word = 0x2000 | symbolItor->second;
                else if(parseInteger(statementItor->address, address, 0xfff))
                    word = 0x2000 | address;
                
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_SE_VX_NN:
                word = 0x3000 | (statementItor->x << 8) | statementItor->nn;
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_SNE_VX_NN:
                word = 0x4000 | (statementItor->x << 8) | statementItor->nn;
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_SE_VX_VY:
                word = 0x5000 | (statementItor->x << 8) | (statementItor->y << 4);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_LD_VX_NN:
                word = 0x6000 | (statementItor->x << 8) | statementItor->nn;
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_ADD_VX_NN:
                word = 0x7000 | (statementItor->x << 8) | statementItor->nn;
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_LD_VX_VY:
                word = 0x8000 | (statementItor->x << 8) | (statementItor->y << 4);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_OR_VX_VY:
                word = 0x8001 | (statementItor->x << 8) | (statementItor->y << 4);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_AND_VX_VY:
                word = 0x8002 | (statementItor->x << 8) | (statementItor->y << 4);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_XOR_VX_VY:
                word = 0x8003 | (statementItor->x << 8) | (statementItor->y << 4);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_ADD_VX_VY:
                word = 0x8004 | (statementItor->x << 8) | (statementItor->y << 4);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_SUB_VX_VY:
                word = 0x8005 | (statementItor->x << 8) | (statementItor->y << 4);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_SHR_VX_VY:
                word = 0x8006 | (statementItor->x << 8) | (statementItor->y << 4);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_SUBN_VX_VY:
                word = 0x8007 | (statementItor->x << 8) | (statementItor->y << 4);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_SHL_VX_VY:
                word = 0x800e | (statementItor->x << 8) | (statementItor->y << 4);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_SNE_VX_VY:
                word = 0x9000 | (statementItor->x << 8) | (statementItor->y << 4);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_LD_I_ADDR:
                if((symbolItor = g_symbolTable.find(statementItor->address)) != g_symbolTable.end())
                    word = 0xa000 | symbolItor->second;
                else if(parseInteger(statementItor->address, address, 0xfff))
                    word = 0xa000 | address;
                
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_JP_V0_ADDR:
                if((symbolItor = g_symbolTable.find(statementItor->address)) != g_symbolTable.end())
                    word = 0xb000 | symbolItor->second;
                else if(parseInteger(statementItor->address, address, 0xfff))
                    word = 0xb000 | address;
                
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_RND_VX_NN:
                word = 0xc000 | (statementItor->x << 8) | statementItor->nn;
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_DRW_VX_VY_N:
                word = 0xd000 | (statementItor->x << 8) | (statementItor->y << 4) | statementItor->n;
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_SKP_VX:
                word = 0xe09e | (statementItor->x << 8);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_SKNP_VX:
                word = 0xe0a1 | (statementItor->x << 8);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_LD_VX_DT:
                word = 0xf007 | (statementItor->x << 8);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_LD_VX_N:
                word = 0xf00a | (statementItor->x << 8);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_LD_DT_VX:
                word = 0xf015 | (statementItor->x << 8);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_LD_ST_VX:
                word = 0xf018 | (statementItor->x << 8);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_ADD_I_VX:
                word = 0xf01e | (statementItor->x << 8);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_LD_F_VX:
                word = 0xf029 | (statementItor->x << 8);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_LD_B_VX:
                word = 0xf033 | (statementItor->x << 8);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_LD_I_VX:
                word = 0xf055 | (statementItor->x << 8);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;
            
            case INST_LD_VX_I:
                word = 0xf065 | (statementItor->x << 8);
                outputFile.write((char *) &word + 1, 1);
                outputFile.write((char *) &word, 1);
                break;

            default:
                fprintf(stderr, "unexpected instruction %d\n", statementItor->instruction);
                return false;
        }
    }
    
    return true;
}


//
//
//

int main(int argc,char *argv[])
{
    // ensure we got a filename
    if(argc != 2)
    {
        fprintf(stderr, "\nusage:  chip8asm <filename>\n");
        return 1;
    }

    std::string inputFilename(argv[1]);
    
    
    // figure out the output filename
    std::string outputFilename(argv[1]);

    if(outputFilename.compare(outputFilename.size() - 2, 2, ".s") == 0  ||
        outputFilename.compare(outputFilename.size() - 2, 2, ".S") == 0)
    {
        outputFilename.resize(outputFilename.size() - 2);
    }

    outputFilename += ".ch8";

    
    // parse statements from input file
    if(!readInput(inputFilename))
    {
        fprintf(stderr, "error reading input file\n");
        return 1;
    }


    // write output file
    if(!writeOutput(outputFilename))
    {
        fprintf(stderr, "error writing output file\n");
        return 1;
    }


    return 0;
}
