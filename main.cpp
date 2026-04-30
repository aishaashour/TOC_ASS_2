/// Student1: Aisha Ibrahim Abdulsalam    ID: 20221092
/// Student2: Shaza Ahmed Mohamed         ID: 20221079
/// Student3: Rania Raafat Edwar          ID: 20221055
// Group Expression Recursive Descent Parser
// Based on expressions from James Hein Book, Pages 459-460
// Extended BNF (EBNF) Grammar:
// expr    -> factor { '.' factor }         left associative (product)
// factor  -> primary { '^' '-' '1' }       postfix inverse
// primary -> '(' expr ')' | 'e' | var
// var     -> 'a'..'z'
// -------------------------------------------------
// Reduction Rules (from James Hein Book, Page 460):
// R1:  e . x          -> x
// R2:  x . e          -> x
// R3:  x^-1 . x       -> e
// R4:  x . x^-1       -> e
// R5:  e^-1           -> e
// R6:  x^-1^-1        -> x
// R7:  y^-1 . (y . z) -> z
// R8:  y . (y^-1 . z) -> z
// R9:  (x . y) . z    -> x . (y . z)
// R10: (x . y)^-1     -> y^-1 . x^-1
// ---------------------------------------------------
// Test  1: ((x.y^-1).z)^-1
// Test  2: x
// Test  3: e
// Test  4: ((x.y).(z.t))^-1
// Test  5: x^-1^-1
// Test  6: e.x
// Test  7: ((x.y)^-1)^-1
// Test  8: x^-1.x
// Test  9: ((a.b).(c.(d.e^-1)))^-1
// Test 10: a^-1.b^-1.(c.d^-1)^-1
// Test 11: (x.y).z
// Test 12: x.(y.z)
// Test 13: (x.y)^-1
// Test 14: (z^-1.y).x^-1
// Test 15: z^-1.(x.y^-1)^-1
// Test 16: x.y.z
// Test 17: x^-1.y^-1.z^-1
// Test 18: (((a.b).c).d)^-1
// Test 19: a^-1^-1^-1
// Test 20: z^-1.(y.x^-1)
// ------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <cstring>

////////////////////////////////////////////////////////////////////////
// Input and Output ////////////////////////////////////////////////////

struct InFile
{
    FILE* file;

    InFile(const char* str) {file=fopen(str, "r");}
    ~InFile() {if(file) fclose(file);}

    char GetNextTokenChar()
    {
        int ch = fgetc(file);
        if(ch==EOF)
            return 0;
        return ch;
    }
};

struct OutFile
{
    FILE* file;

    OutFile(const char* str) {file=0; if(str) file=fopen(str, "w");}
    ~OutFile() {if(file) fclose(file);}

    void Out(const char* s)
    {
        fprintf(file, "%s\n", s); fflush(file);
    }
};

///////////////////////////////////////////////////////////////////////
// Compiler Parameters ////////////////////////////////////////////////

struct CompilerInfo
{
    InFile  in_file;
    OutFile out_file;
    OutFile debug_file;

    CompilerInfo(const char* in_str, const char* out_str, const char* debug_str)
                : in_file(in_str), out_file(out_str), debug_file(debug_str)
    {
    }
};

//////////////////////////////////////////////////////////////////////
// Scanner ///////////////////////////////////////////////////////////

#define MAX_TOKEN_LEN 40

enum TokenType
{
    ID, IDENTITY,
    DOT, CARET, MINUS, ONE,
    LEFT_PAREN, RIGHT_PAREN,
    ERROR, ENDFILE
};

// Used for debugging only /////////////////////////////////////////
const char* TokenTypeStr[]=
{
    "ID", "Identity",
    "Dot", "Caret", "Minus", "One",
    "LeftParen", "RightParen",
    "Error", "EndFile"
};

struct Token
{
    TokenType type;
    char      ch;

    Token() {ch=0; type=ERROR;}
    Token(TokenType _type, char _ch) {type=_type; ch=_ch;}
};

const Token symbolic_tokens[]=
{
    Token(ID,          0  ), // not used directly
    Token(IDENTITY,    'e'),
    Token(DOT,         '.'),
    Token(CARET,       '^'),
    Token(MINUS,       '-'),
    Token(ONE,         '1'),
    Token(LEFT_PAREN,  '('),
    Token(RIGHT_PAREN, ')')
};

const int num_symbolic_tokens=sizeof(symbolic_tokens)/sizeof(symbolic_tokens[0]);

void GetNextToken(CompilerInfo* pci, Token* ptoken) // Reads next token from a file
{
    ptoken->type=ERROR;
    ptoken->ch  =0;

    char s;
    while(true)   // skip whitespace and newlines
    {
        s = pci->in_file.GetNextTokenChar();
        if(!s || (s!=' ' && s!='\t' && s!='\n' && s!='\r'))
            break;
    }

    if(!s)
    {
        ptoken->type=ENDFILE;
        ptoken->ch  =0;
        return;
    }

    int i;
    for(i=0;i<num_symbolic_tokens;i++)
    {
        if(s==symbolic_tokens[i].ch)
        {
            ptoken->type = symbolic_tokens[i].type;
            ptoken->ch   =s;
            return;
        }
    }

    if(s>='a' && s<='z')
    {
        ptoken->type=ID;
        ptoken->ch  =s;
        return;
    }
}

/////////////////////////////////////////////////////////////////
// Parser ///////////////////////////////////////////////////////
/// top-down parsing

enum NodeKind
{
    PRODUCT_NODE,
    INVERSE_NODE,
    ID_NODE,
    IDENTITY_NODE
};

// Used for debugging only ///////////////////////////////////////
const char* NodeKindStr[]=
{
    "product",
    "inverse",
    "ID",
    "identity"
};

#define MAX_CHILDREN 2

struct TreeNode
{
    TreeNode* child[MAX_CHILDREN];
    NodeKind  node_kind;
    char      id;   // variable name

    TreeNode()
    {
        int i;
        for(i=0;i<MAX_CHILDREN;i++)
            child[i]=0;
        id=0;
    }
};

struct ParseInfo  // stores the current token
{
    Token next_token;
};

void Match(CompilerInfo* pci, ParseInfo* ppi, TokenType expected_token_type) // Consumes expected token
{
    pci->debug_file.Out("Start Match");

    if(ppi->next_token.type!=expected_token_type)
        throw 0;
    GetNextToken(pci, &ppi->next_token);

    fprintf(pci->debug_file.file, "%c (%s)\n",
            ppi->next_token.ch, TokenTypeStr[ppi->next_token.type]);
    fflush(pci->debug_file.file);
}

TreeNode* Expr(CompilerInfo*, ParseInfo*);

///////////////////////////////////////////////////////////////////////
// primary -> '(' expr ')' | 'e' | var ////////////////////////////////

TreeNode* Primary(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start Primary");

    if(ppi->next_token.type==IDENTITY)
    {
        TreeNode* tree=new TreeNode;
        tree->node_kind=IDENTITY_NODE;
        tree->id=ppi->next_token.ch;
        Match(pci, ppi, IDENTITY);

        pci->debug_file.Out("End Primary");
        return tree;
    }

    if(ppi->next_token.type==ID)
    {
        TreeNode* tree=new TreeNode;
        tree->node_kind=ID_NODE;
        tree->id=ppi->next_token.ch;
        Match(pci, ppi, ID);

        pci->debug_file.Out("End Primary");
        return tree;
    }

    if(ppi->next_token.type==LEFT_PAREN)
    {
        Match(pci, ppi, LEFT_PAREN);
        TreeNode* tree=Expr(pci, ppi);
        Match(pci, ppi, RIGHT_PAREN);

        pci->debug_file.Out("End Primary");
        return tree;
    }

    throw 0;
    return 0;
}

//////////////////////////////////////////////////////////////////////
// factor -> primary { '^' '-' '1' }   (postfix inverse, stackable) //

TreeNode* Factor(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start Factor");

    TreeNode* tree=Primary(pci, ppi);

    while(ppi->next_token.type==CARET)
    {
        Match(pci, ppi, CARET);
        Match(pci, ppi, MINUS);
        Match(pci, ppi, ONE);

        TreeNode* new_tree=new TreeNode;
        new_tree->node_kind=INVERSE_NODE;
        new_tree->child[0] =tree;

        tree=new_tree;
    }

    pci->debug_file.Out("End Factor");
    return tree;
}

///////////////////////////////////////////////////////////////////
// expr -> factor { '.' factor }    (left associative) ////////////

TreeNode* Expr(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start Expr");

    TreeNode* tree=Factor(pci, ppi);

    while(ppi->next_token.type==DOT)
    {
        TreeNode* new_tree=new TreeNode;
        new_tree->node_kind=PRODUCT_NODE;

        new_tree->child[0]=tree;
        Match(pci, ppi, DOT);
        new_tree->child[1]=Factor(pci, ppi);
        tree=new_tree;
    }

    pci->debug_file.Out("End Expr");
    return tree;
}

//////////////////////////////////////////////////////////////////
// Parse /////////////////////////////////////////////////////////

TreeNode* Parse(CompilerInfo* pci)
{
    ParseInfo parse_info;
    GetNextToken(pci, &parse_info.next_token);

    TreeNode* syntax_tree = Expr(pci, &parse_info);

    if(parse_info.next_token.type!=ENDFILE)
        pci->debug_file.Out("Error: input not fully consumed");

    return syntax_tree;
}

/////////////////////////////////////////////////////////////////////
// Tree Utilities ///////////////////////////////////////////////////

TreeNode* NewNode(NodeKind kind, char id_ch, TreeNode* c0, TreeNode* c1)
{
    TreeNode* t=new TreeNode;
    t->node_kind=kind;
    t->id=id_ch;
    t->child[0]=c0;
    t->child[1]=c1;
    return t;
}

TreeNode* CopyTree(TreeNode* node)
{
    if(!node) return 0;
    TreeNode* t=new TreeNode;
    t->node_kind=node->node_kind;
    t->id=node->id;
    t->child[0]=CopyTree(node->child[0]);
    t->child[1]=CopyTree(node->child[1]);
    return t;
}

void DestroyTree(TreeNode* node)
{
    if(!node) return;
    int i;
    for(i=0;i<MAX_CHILDREN;i++)
        DestroyTree(node->child[i]);
    delete node;
}

bool TreesEqual(TreeNode* a, TreeNode* b) // structural equality check
{
    if(!a && !b) return true;
    if(!a || !b) return false;
    if(a->node_kind!=b->node_kind) return false;
    if(a->id!=b->id) return false;
    return TreesEqual(a->child[0], b->child[0]) && TreesEqual(a->child[1], b->child[1]);
}

////////////////////////////////////////////////////////////////////////////////////
// Print Tree //////////////////////////////////////////////////////////////////////

void PrintTreeHelper(TreeNode* node, char* prefix, int is_root, OutFile* out)
{
    if(!node) return;

    char line[256];
    line[0]='\0';

    strcat(line, prefix);
    if(!is_root)
        strcat(line, "|--");

    switch(node->node_kind)
    {
        case PRODUCT_NODE:  strcat(line, "product");                                             break;
        case INVERSE_NODE:  strcat(line, "inverse");                                             break;
        case ID_NODE:       { char tmp[2]; tmp[0]=node->id; tmp[1]='\0'; strcat(line, tmp); }   break;
        case IDENTITY_NODE: strcat(line, "e");                                                   break;
    }

    printf("%s\n", line); fflush(NULL);
    if(out && out->file) { fprintf(out->file, "%s\n", line); fflush(out->file); }

    int   prefix_len  = strlen(prefix);
    char* child_prefix=new char[prefix_len+10];
    strcpy(child_prefix, prefix);

    if(!is_root) strcat(child_prefix, "   ");

    int i;
    for(i=0;i<MAX_CHILDREN;i++)
    {
        if(node->child[i])
            PrintTreeHelper(node->child[i], child_prefix, 0, out);
    }

    delete[] child_prefix;
}

void PrintTree(TreeNode* node, OutFile* out)
{
    char prefix[4];
    prefix[0]='\0';
    PrintTreeHelper(node, prefix, 1, out);
}

////////////////////////////////////////////////////////////////////////////////////
// Tree to Expression //////////////////////////////////////////////////////////////

// Returns true if node needs parentheses when used as child of a product
bool NeedsParens(TreeNode* node)
{
    if(!node) return false;
    return node->node_kind==PRODUCT_NODE;
}

void TreeToExpr(TreeNode* node, char* buf) // writes expression string into buf
{
    if(!node) return;

    switch(node->node_kind)
    {
        case ID_NODE:
        {
            char tmp[2]; tmp[0]=node->id; tmp[1]='\0';
            strcat(buf, tmp);
            break;
        }
        case IDENTITY_NODE:
        {
            strcat(buf, "e");
            break;
        }
        case INVERSE_NODE:
        {
            bool parens = NeedsParens(node->child[0]);
            if(parens) strcat(buf, "(");
            TreeToExpr(node->child[0], buf);
            if(parens) strcat(buf, ")");
            strcat(buf, "^-1");
            break;
        }
        case PRODUCT_NODE:
        {
            bool lp = (node->child[0] && node->child[0]->node_kind==PRODUCT_NODE);
            bool rp = (node->child[1] && node->child[1]->node_kind==PRODUCT_NODE);
            // left child: never needs parens (left-assoc), right child needs parens if product
            if(lp) strcat(buf, "(");
            TreeToExpr(node->child[0], buf);
            if(lp) strcat(buf, ")");
            strcat(buf, ".");
            if(rp) strcat(buf, "(");
            TreeToExpr(node->child[1], buf);
            if(rp) strcat(buf, ")");
            break;
        }
    }
}

void PrintExpr(TreeNode* node, OutFile* out)
{
    char buf[1024];
    buf[0]='\0';
    TreeToExpr(node, buf);
    printf("%s\n", buf); fflush(NULL);
    if(out && out->file) { fprintf(out->file, "%s\n", buf); fflush(out->file); }
}

///////////////////////////////////////////////////////////////////////////////////
// Reduction Rules////////////////////////////////////////////////////////////////
// Each rule returns a new tree if it applied, or 0 if it did not apply.
// The caller is responsible for freeing old nodes and updating the pointer.


// R5: e^-1 -> e
TreeNode* TryR5(TreeNode* node, bool* changed)
{
    if(node->node_kind==INVERSE_NODE &&
       node->child[0] && node->child[0]->node_kind==IDENTITY_NODE)
    {
        TreeNode* result=NewNode(IDENTITY_NODE, 'e', 0, 0);
        *changed=true;
        return result;
    }
    return 0;
}

// R6: x^-1^-1 -> x
TreeNode* TryR6(TreeNode* node, bool* changed)
{
    if(node->node_kind==INVERSE_NODE &&
       node->child[0] && node->child[0]->node_kind==INVERSE_NODE)
    {
        TreeNode* result=CopyTree(node->child[0]->child[0]);
        *changed=true;
        return result;
    }
    return 0;
}

// R10: (x . y)^-1 -> y^-1 . x^-1
TreeNode* TryR10(TreeNode* node, bool* changed)
{
    if(node->node_kind==INVERSE_NODE &&
       node->child[0] && node->child[0]->node_kind==PRODUCT_NODE)
    {
        TreeNode* x=CopyTree(node->child[0]->child[0]);
        TreeNode* y=CopyTree(node->child[0]->child[1]);
        TreeNode* yi=NewNode(INVERSE_NODE, 0, y, 0);
        TreeNode* xi=NewNode(INVERSE_NODE, 0, x, 0);
        TreeNode* result=NewNode(PRODUCT_NODE, 0, yi, xi);
        *changed=true;
        return result;
    }
    return 0;
}

// R1: e . x -> x
TreeNode* TryR1(TreeNode* node, bool* changed)
{
    if(node->node_kind==PRODUCT_NODE &&
       node->child[0] && node->child[0]->node_kind==IDENTITY_NODE)
    {
        TreeNode* result=CopyTree(node->child[1]);
        *changed=true;
        return result;
    }
    return 0;
}

// R2: x . e -> x
TreeNode* TryR2(TreeNode* node, bool* changed)
{
    if(node->node_kind==PRODUCT_NODE &&
       node->child[1] && node->child[1]->node_kind==IDENTITY_NODE)
    {
        TreeNode* result=CopyTree(node->child[0]);
        *changed=true;
        return result;
    }
    return 0;
}

// R3: x^-1 . x -> e
TreeNode* TryR3(TreeNode* node, bool* changed)
{
    if(node->node_kind==PRODUCT_NODE &&
       node->child[0] && node->child[0]->node_kind==INVERSE_NODE &&
       node->child[1])
    {
        if(TreesEqual(node->child[0]->child[0], node->child[1]))
        {
            TreeNode* result=NewNode(IDENTITY_NODE, 'e', 0, 0);
            *changed=true;
            return result;
        }
    }
    return 0;
}

// R4: x . x^-1 -> e
TreeNode* TryR4(TreeNode* node, bool* changed)
{
    if(node->node_kind==PRODUCT_NODE &&
       node->child[1] && node->child[1]->node_kind==INVERSE_NODE &&
       node->child[0])
    {
        if(TreesEqual(node->child[0], node->child[1]->child[0]))
        {
            TreeNode* result=NewNode(IDENTITY_NODE, 'e', 0, 0);
            *changed=true;
            return result;
        }
    }
    return 0;
}

// R7: y^-1 . (y . z) -> z
TreeNode* TryR7(TreeNode* node, bool* changed)
{
    if(node->node_kind==PRODUCT_NODE &&
       node->child[0] && node->child[0]->node_kind==INVERSE_NODE &&
       node->child[1] && node->child[1]->node_kind==PRODUCT_NODE)
    {
        TreeNode* y  = node->child[0]->child[0];
        TreeNode* yz = node->child[1];
        if(TreesEqual(y, yz->child[0]))
        {
            TreeNode* result=CopyTree(yz->child[1]);
            *changed=true;
            return result;
        }
    }
    return 0;
}

// R8: y . (y^-1 . z) -> z
TreeNode* TryR8(TreeNode* node, bool* changed)
{
    if(node->node_kind==PRODUCT_NODE &&
       node->child[0] &&
       node->child[1] && node->child[1]->node_kind==PRODUCT_NODE &&
       node->child[1]->child[0] && node->child[1]->child[0]->node_kind==INVERSE_NODE)
    {
        TreeNode* y  = node->child[0];
        TreeNode* yz = node->child[1];
        if(TreesEqual(y, yz->child[0]->child[0]))
        {
            TreeNode* result=CopyTree(yz->child[1]);
            *changed=true;
            return result;
        }
    }
    return 0;
}

// R9: (x . y) . z -> x . (y . z)
TreeNode* TryR9(TreeNode* node, bool* changed)
{
    if(node->node_kind==PRODUCT_NODE &&
       node->child[0] && node->child[0]->node_kind==PRODUCT_NODE)
    {
        TreeNode* x=CopyTree(node->child[0]->child[0]);
        TreeNode* y=CopyTree(node->child[0]->child[1]);
        TreeNode* z=CopyTree(node->child[1]);
        TreeNode* yz=NewNode(PRODUCT_NODE, 0, y, z);
        TreeNode* result=NewNode(PRODUCT_NODE, 0, x, yz);
        *changed=true;
        return result;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////

// Applies a single pass of all rules to this node (children first)
// in depth-first (left child, right child, then self) order.
bool ApplyOneRule(TreeNode* node, TreeNode** replacement)
{
    if(!node) return false;

    // --- try rules on THIS node first (top-down order) ---
    bool dummy=false;
    TreeNode* result=0;

    // Inverse rules: R5, R6, R10
    if(!result && node->node_kind==INVERSE_NODE) result=TryR5 (node, &dummy);
    if(!result && node->node_kind==INVERSE_NODE) result=TryR6 (node, &dummy);
    if(!result && node->node_kind==INVERSE_NODE) result=TryR10(node, &dummy);

    // Product rules: R1..R4, R7, R8, R9
    if(!result && node->node_kind==PRODUCT_NODE) result=TryR1(node, &dummy);
    if(!result && node->node_kind==PRODUCT_NODE) result=TryR2(node, &dummy);
    if(!result && node->node_kind==PRODUCT_NODE) result=TryR3(node, &dummy);
    if(!result && node->node_kind==PRODUCT_NODE) result=TryR4(node, &dummy);
    if(!result && node->node_kind==PRODUCT_NODE) result=TryR7(node, &dummy);
    if(!result && node->node_kind==PRODUCT_NODE) result=TryR8(node, &dummy);
    if(!result && node->node_kind==PRODUCT_NODE) result=TryR9(node, &dummy);

    if(result)
    {
        *replacement=result;
        return true;
    }

    // --- no rule matched here; recurse into left child ---
    TreeNode* child_replacement=0;
    if(ApplyOneRule(node->child[0], &child_replacement))
    {
        if(child_replacement)
        {
            DestroyTree(node->child[0]);
            node->child[0]=child_replacement;
        }
        return true;
    }

    // --- recurse into right child ---
    if(ApplyOneRule(node->child[1], &child_replacement))
    {
        if(child_replacement)
        {
            DestroyTree(node->child[1]);
            node->child[1]=child_replacement;
        }
        return true;
    }

    return false;
}

// Reduce tree to normal form.
// Prints the tree and expression after EVERY individual rule application.
TreeNode* Reduce(TreeNode* tree, OutFile* out)
{
    while(true)
    {
        TreeNode* replacement=0;
        if(!ApplyOneRule(tree, &replacement))
            break;  // no rule matched anywhere: normal form reached

        if(replacement)
        {
            DestroyTree(tree);
            tree=replacement;
        }

        printf("\n"); fflush(NULL);
        if(out && out->file) { fprintf(out->file, "\n"); fflush(out->file); }

        PrintTree(tree, out);
        PrintExpr(tree, out);
    }
    return tree;
}

////////////////////////////////////////////////////////////////////////////////////
// Compiler ////////////////////////////////////////////////////////////////////////

void StartCompiler(CompilerInfo* pci)
{
    TreeNode* syntax_tree=Parse(pci);

    printf("Parse Tree:\n"); fflush(NULL);
    if(pci->out_file.file) { fprintf(pci->out_file.file, "Parse Tree:\n"); fflush(pci->out_file.file); }

    PrintTree(syntax_tree, &pci->out_file);

    printf("\nReducing to normal form:\n"); fflush(NULL);
    if(pci->out_file.file) { fprintf(pci->out_file.file, "\nReducing to normal form:\n"); fflush(pci->out_file.file); }

    syntax_tree=Reduce(syntax_tree, &pci->out_file);

    printf("\nNormal Form:\n"); fflush(NULL);
    if(pci->out_file.file) { fprintf(pci->out_file.file, "\nNormal Form:\n"); fflush(pci->out_file.file); }

    PrintTree(syntax_tree, &pci->out_file);
    PrintExpr(syntax_tree, &pci->out_file);

    printf("---------------------------------\n"); fflush(NULL);
    if(pci->out_file.file)
    {
        fprintf(pci->out_file.file, "---------------------------------\n");
        fflush(pci->out_file.file);
    }

    DestroyTree(syntax_tree);
}

////////////////////////////////////////////////////////////////////////////////////
// Main ////////////////////////////////////////////////////////////////////////////

int main()
{
    printf("Start main()\n"); fflush(NULL);
    CompilerInfo compiler_info("input.txt", "output.txt", "debug.txt");

    StartCompiler(&compiler_info);

    printf("End main()\n"); fflush(NULL);
    return 0;
}