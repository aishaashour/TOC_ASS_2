/// Student1: Aisha Ibrahim Abdulsalam ID: 20221092
/// Student2: Shaza Ahmed Mohamed ID: 20221079
/// Student3: Rania Raafat Edwa ID: 20221055
// Extended BNF (EBNF) Grammar:
// expr    -> factor { '.' factor }              left associative (product)   -->lowest precedence
// factor  -> primary { '^' '-' digit }          postfix inverse applied n times
// primary -> '(' expr ')' | 'e' | var          -->heighest precedence
// var     -> 'a'..'z'  (single lowercase letter, 'e' is reserved for identity)
// --------------------------
// Reduction Rules:
// R1: e.x->x
// R2: x.e->x
// R3: x^-1.x->e
// R4: x.x^-1->e
// R5: e^-1->e
// R6: x^-1^-1->x
// R7: y^-1.(y.z)->z
// R8: y.(y^-1.z)->z
// R9: (x.y).z->x.(y.z)
// R10: (x.y)^-1->y^-1.x^-1
// -------------------------
//tests
// 1: ((x.y^-1).z)^-1
// 2: x
// 3: x^-3
// 4: ((x.y).(z.t))^-1
// 5: x^-1^-1
// 6: (x.y^-2)^-3
// 7: ((x.y)^-1)^-1
// 8: x^-1.x
// 9: ((a.b).(c.(d.e^-1)))^-1
// 10: a^-1.b^-1.(c.d^-1)^-1
// 11: (x.y).z
// 12: x.(y.z)
// 13: (x.y)^-1
// 14: (z^-1.y).x^-1
// 15: z^-1.(x.y^-1)^-1
// 16: x.y.z
// 17: x^-1.y^-1.z^-1
// 18: (((a.b).c).d)^-1
// 19: a^-1^-1^-1
// 20: z^-1.(y.x^-1)
// ---------------------
#include <cstdio>
#include <cstdlib>
#include <cstring>
struct InFile
{
    FILE* file;
    InFile(const char* str) {file=fopen(str,"r");}
    ~InFile(){if(file) fclose(file);}
    char GetNextTokenChar()
    {
        int ch = fgetc(file);
        if(ch==EOF)return 0;
        return ch;
    }
};
struct OutFile
{
    FILE* file;
    OutFile(const char* str){file=0;if(str)file=fopen(str,"w");}
    ~OutFile(){if(file)fclose(file);}
    void Out(const char* s)
    {
        fprintf(file,"%s\n",s);fflush(file);
    }
};
struct CompilerInfo
{
    InFile in_file;
    OutFile out_file;
    OutFile debug_file;
    CompilerInfo(const char* in_str,const char* out_str,const char* debug_str): in_file(in_str),out_file(out_str),debug_file(debug_str){}
};
#define MAX_TOKEN_LEN 40
enum TokenType
{
    ID,IDENTITY,DOT,CARET,MINUS,ONE,LEFT_PAREN,RIGHT_PAREN,ERROR,ENDFILE
};
const char* TokenTypeStr[]=
{
    "ID","Identity","Dot","Caret","Minus","One","LeftParen","RightParen","Error","EndFile"
};
struct Token
{
    TokenType type;
    char ch;
    Token(){ch=0;type=ERROR;}
    Token(TokenType _type,char _ch){type=_type;ch=_ch;}
};
const Token symbolic_tokens[]=
{
    Token(ID,0),Token(IDENTITY,'e'),Token(DOT,'.'),
    Token(CARET,'^'),Token(MINUS,'-'),
    Token(LEFT_PAREN,'('),Token(RIGHT_PAREN,')')
};
const int num_symbolic_tokens=sizeof(symbolic_tokens)/sizeof(symbolic_tokens[0]);
void GetNextToken(CompilerInfo* pci,Token* ptoken)      //reads characters 1 by 1
{
    ptoken->type=ERROR;ptoken->ch=0;char s;
    while(true)
    {
        s=pci->in_file.GetNextTokenChar();
        if(!s||(s!=' '&&s!='\t'&&s!='\n'&&s!='\r'))break;
    }
    if(!s){ptoken->type=ENDFILE;ptoken->ch=0;return;}
    int i;
    for(i=0;i<num_symbolic_tokens;i++)
    {
        if(s==symbolic_tokens[i].ch){ptoken->type=symbolic_tokens[i].type;ptoken->ch=s;return;}
    }
    if(s>='1'&&s<='9'){ptoken->type=ONE;ptoken->ch=s;return;}
    if(s>='a'&&s<='z'){ptoken->type=ID;ptoken->ch=s;return;}
}
enum NodeKind
{
    PRODUCT_NODE,INVERSE_NODE,ID_NODE,IDENTITY_NODE
};
const char* NodeKindStr[]=
{
    "product","inverse","ID","identity"
};
#define MAX_CHILDREN 2
struct TreeNode
{
    TreeNode* child[MAX_CHILDREN];
    NodeKind node_kind;char id;
    TreeNode(){int i;for(i=0;i<MAX_CHILDREN;i++)child[i]=0;id=0;}
};
struct ParseInfo{Token next_token;};
void Match(CompilerInfo* pci,ParseInfo* ppi,TokenType expected_token_type)
{
    pci->debug_file.Out("Start Match");
    if(ppi->next_token.type!=expected_token_type)throw 0;
    GetNextToken(pci,&ppi->next_token);
    fprintf(pci->debug_file.file,"%c (%s)\n",ppi->next_token.ch,TokenTypeStr[ppi->next_token.type]);
    fflush(pci->debug_file.file);
}
TreeNode* Expr(CompilerInfo*,ParseInfo*);
TreeNode* Primary(CompilerInfo* pci,ParseInfo* ppi)     //paranthes and e or id
{
    pci->debug_file.Out("Start Primary");
    if(ppi->next_token.type==IDENTITY)
    {
        TreeNode* tree=new TreeNode;
        tree->node_kind=IDENTITY_NODE;
        tree->id=ppi->next_token.ch;
        Match(pci,ppi,IDENTITY);
        pci->debug_file.Out("End Primary");return tree;
    }
    if(ppi->next_token.type==ID)
    {
        TreeNode* tree=new TreeNode;
        tree->node_kind=ID_NODE;
        tree->id=ppi->next_token.ch;
        Match(pci,ppi,ID);
        pci->debug_file.Out("End Primary");return tree;
    }
    if(ppi->next_token.type==LEFT_PAREN)
    {
        Match(pci,ppi,LEFT_PAREN);
        TreeNode* tree=Expr(pci,ppi);
        Match(pci,ppi,RIGHT_PAREN);
        pci->debug_file.Out("End Primary");return tree;
    }
    throw 0;
}
TreeNode* Factor(CompilerInfo* pci, ParseInfo* ppi)     //inverse operator
{
    pci->debug_file.Out("Start Factor");
    TreeNode* tree=Primary(pci, ppi);
    while(ppi->next_token.type==CARET)
    {
        Match(pci,ppi,CARET);
        Match(pci,ppi,MINUS);
        if(ppi->next_token.type!=ONE)throw 0;
        int n=ppi->next_token.ch-'0';
        GetNextToken(pci,&ppi->next_token);
        for(int i=0;i<n;i++)
        {
            TreeNode* new_tree=new TreeNode;new_tree->node_kind=INVERSE_NODE;new_tree->child[0]=tree;tree = new_tree;
        }
    }
    pci->debug_file.Out("End Factor");
    return tree;
}
TreeNode* Expr(CompilerInfo* pci,ParseInfo* ppi)     //product operator
{
    pci->debug_file.Out("Start Expr");
    TreeNode* tree=Factor(pci,ppi);
    while(ppi->next_token.type==DOT)
    {
        TreeNode* new_tree=new TreeNode;
        new_tree->node_kind=PRODUCT_NODE;
        new_tree->child[0]=tree;
        Match(pci,ppi,DOT);
        new_tree->child[1]=Factor(pci, ppi);
        tree=new_tree;
    }
    pci->debug_file.Out("End Expr");return tree;
}
TreeNode* Parse(CompilerInfo* pci)
{
    ParseInfo parse_info;
    GetNextToken(pci,&parse_info.next_token);
    TreeNode* syntax_tree=Expr(pci,&parse_info);
    if(parse_info.next_token.type!=ENDFILE)pci->debug_file.Out("Error: input not fully consumed");
    return syntax_tree;
}
TreeNode* NewNode(NodeKind kind,char id_ch,TreeNode* c0,TreeNode* c1)
{
    TreeNode* t=new TreeNode;
    t->node_kind=kind;t->id=id_ch;t->child[0]=c0;t->child[1]=c1;return t;
}
TreeNode* CopyTree(TreeNode* node)      //deep copy
{
    if(!node)return 0;
    TreeNode* t=new TreeNode;
    t->node_kind=node->node_kind;t->id=node->id;
    t->child[0]=CopyTree(node->child[0]);t->child[1]=CopyTree(node->child[1]);
    return t;
}
void DestroyTree(TreeNode* node)
{
    if(!node)return;int i;
    for(i=0;i<MAX_CHILDREN;i++)DestroyTree(node->child[i]);
    delete node;
}
bool TreesEqual(TreeNode* a,TreeNode* b)        //check if 2 trees equal
{
    if(!a&&!b)return true;  //both are null
    if(!a||!b)return false;    //one null and one not
    if(a->node_kind!=b->node_kind)return false;     //node type differs
    if(a->id!=b->id)return false;           //not the same id (variable)
    return TreesEqual(a->child[0],b->child[0])&&TreesEqual(a->child[1],b->child[1]);    //recursively compare left and right child
}
void PrintTreeHelper(TreeNode* node,char* prefix,int is_root,OutFile* out)
{
    if(!node)return;    //base case: stop recursion if node is null
    char line[256];line[0]='\0';
    strcat(line,prefix);if(!is_root)strcat(line,"|--"); //tree branch for child nodes
    switch(node->node_kind)
    {
        case PRODUCT_NODE:strcat(line,"product");break;     //print product for . nodes
        case INVERSE_NODE:strcat(line,"inverse");break;
        case ID_NODE:{char tmp[2];tmp[0]=node->id;tmp[1]='\0';strcat(line,tmp);}break;      //convert character to string
        case IDENTITY_NODE:strcat(line,"e");break;      //print identity element
    }
    printf("%s\n",line);fflush(NULL);       //print line
    if(out&&out->file){fprintf(out->file,"%s\n",line);fflush(out->file);}
    int prefix_len=strlen(prefix);
    char* child_prefix=new char[prefix_len+10];
    strcpy(child_prefix,prefix);
    if(!is_root)strcat(child_prefix,"   ");
    int i;
    for(i=0;i<MAX_CHILDREN;i++)
    {if(node->child[i])PrintTreeHelper(node->child[i],child_prefix,0, out);}delete[] child_prefix;
}
void PrintTree(TreeNode* node,OutFile* out)
{
    char prefix[4];     //empty initial prefix
    prefix[0]='\0';
    PrintTreeHelper(node,prefix,1,out);
}
bool NeedsParens(TreeNode* node)
{
    if(!node)return false;
    return node->node_kind==PRODUCT_NODE;   //if product node , it needs parentheses
}
void TreeToExpr(TreeNode* node,char* buf)
{
    if(!node)return;
    switch(node->node_kind)
    {
        case ID_NODE: {
            char tmp[2];
            tmp[0]=node->id;
            tmp[1]='\0';
            strcat(buf,tmp);
            break;
        }
        case IDENTITY_NODE: {
            strcat(buf,"e");
            break;
        }
        case INVERSE_NODE:{
            bool parens=NeedsParens(node->child[0]);
            if(parens)strcat(buf,"(");
            TreeToExpr(node->child[0],buf);
            if(parens)strcat(buf,")");
            strcat(buf,"^-1");
            break;
        }
        case PRODUCT_NODE:{bool lp=(node->child[0]&&node->child[0]->node_kind==PRODUCT_NODE);
            bool rp=(node->child[1]&&node->child[1]->node_kind==PRODUCT_NODE);
            if(lp)strcat(buf,"(");TreeToExpr(node->child[0],buf);
            if(lp)strcat(buf,")");strcat(buf,".");if(rp)strcat(buf,"(");TreeToExpr(node->child[1],buf);
            if(rp)strcat(buf,")");break;}
    }
}
void PrintExpr(TreeNode* node,OutFile* out)
{
    char buf[1024];buf[0]='\0';TreeToExpr(node,buf);printf("%s\n",buf);fflush(NULL);
    if(out && out->file){fprintf(out->file,"%s\n",buf);fflush(out->file);}
}
TreeNode* TryR5(TreeNode* node,bool* changed)       //e^-1->e
{
    if(node->node_kind==INVERSE_NODE&&node->child[0]&&node->child[0]->node_kind==IDENTITY_NODE)
    {TreeNode* result=NewNode(IDENTITY_NODE,'e',0,0);
        *changed=true;
        return result;
    }
    return 0;
}
TreeNode* TryR6(TreeNode* node,bool* changed)       //x^-1^-1 -> x
{
    if(node->node_kind==INVERSE_NODE&&node->child[0]&&node->child[0]->node_kind==INVERSE_NODE)
    {TreeNode* result=CopyTree(node->child[0]->child[0]);       //copy original x
        *changed=true;
        return result;
    }return 0;
}
TreeNode* TryR10(TreeNode* node,bool* changed)      //(x.y)^-1 -> y^-1.x^-1
{
    if(node->node_kind==INVERSE_NODE&&node->child[0]&&node->child[0]->node_kind==PRODUCT_NODE)
    {
        TreeNode* x=CopyTree(node->child[0]->child[0]);     //copy left child
        TreeNode* y=CopyTree(node->child[0]->child[1]);     //copy right child
        TreeNode* yi=NewNode(INVERSE_NODE,0,y,0);       //inverse node y^-1
        TreeNode* xi=NewNode(INVERSE_NODE,0,x,0);
        TreeNode* result=NewNode(PRODUCT_NODE,0,yi,xi);     //y^-1.x^-1
        *changed=true;
        return result;
    }return 0;
}
TreeNode* TryR1(TreeNode* node,bool* changed)       //e.x -> x
{
    if(node->node_kind==PRODUCT_NODE&&node->child[0]&&node->child[0]->node_kind==IDENTITY_NODE)     //left child is e
    {TreeNode* result=CopyTree(node->child[1]); //return the right child
        *changed=true;
        return result;
    }return 0;
}
TreeNode* TryR2(TreeNode* node,bool* changed)       //x.e->x
{
    if(node->node_kind==PRODUCT_NODE&&node->child[1]&&node->child[1]->node_kind==IDENTITY_NODE)
    {TreeNode* result=CopyTree(node->child[0]);
        *changed=true;
        return result;
    }return 0;
}
TreeNode* TryR3(TreeNode* node,bool* changed)       //x^-1.x->e
{
    if(node->node_kind==PRODUCT_NODE&&node->child[0]&&node->child[0]->node_kind==INVERSE_NODE&&node->child[1])
    {
        if(TreesEqual(node->child[0]->child[0],node->child[1]))     //both expressions are the same
        {TreeNode* result=NewNode(IDENTITY_NODE,'e',0,0);       //result is e
            *changed=true;return result;}}return 0;
}
TreeNode* TryR4(TreeNode* node,bool* changed)       //x.x^-1->e
{
    if(node->node_kind==PRODUCT_NODE&&node->child[1]&&node->child[1]->node_kind==INVERSE_NODE&&node->child[0])
    {
        if(TreesEqual(node->child[0],node->child[1]->child[0]))
        {TreeNode* result=NewNode(IDENTITY_NODE,'e',0,0);
            *changed=true;
            return result;}
    }
    return 0;
}
TreeNode* TryR7(TreeNode* node,bool* changed)       //y^-1.(y.z) -> z
{
    if(node->node_kind==PRODUCT_NODE&&node->child[0]&&node->child[0]->node_kind==INVERSE_NODE&&
    node->child[1]&&node->child[1]->node_kind==PRODUCT_NODE)
    {
        TreeNode* y=node->child[0]->child[0];TreeNode* yz=node->child[1];
        if(TreesEqual(y,yz->child[0])) {        //first product operand equals y.
            TreeNode* result=CopyTree(yz->child[1]);*changed=true;
            return result;
        }
    }return 0;
}
TreeNode* TryR8(TreeNode* node,bool* changed)       //y.(y^-1.z) -> z
{
    if(node->node_kind==PRODUCT_NODE&&node->child[0]&&node->child[1] && node->child[1]->node_kind==PRODUCT_NODE&&
    node->child[1]->child[0]&&node->child[1]->child[0]->node_kind==INVERSE_NODE)
    {
        TreeNode* y=node->child[0];TreeNode* yz=node->child[1];
        if(TreesEqual(y,yz->child[0]->child[0]))
        {TreeNode* result=CopyTree(yz->child[1]);*changed=true;return result;}}return 0;
}
TreeNode* TryR9(TreeNode* node,bool* changed)       //(x.y).z->x.(y.z)
{
    if(node->node_kind==PRODUCT_NODE&&node->child[0]&&node->child[0]->node_kind==PRODUCT_NODE)
    {
        TreeNode* x=CopyTree(node->child[0]->child[0]);     //extract x
        TreeNode* y=CopyTree(node->child[0]->child[1]);     //y
        TreeNode* z=CopyTree(node->child[1]);               //z
        TreeNode* yz=NewNode(PRODUCT_NODE,0,y,z);       //(y.z)
        TreeNode* result=NewNode(PRODUCT_NODE,0,x,yz);      //x.(y.z)
        *changed=true;
        return result;
    }return 0;
}
bool ApplyOneRule(TreeNode* node,TreeNode** replacement)
{
    if(!node)return false;      //node is null
    bool dummy=false;
    TreeNode* result=0;
    if(!result&&node->node_kind==INVERSE_NODE)result=TryR5(node,&dummy);
    if(!result&&node->node_kind==INVERSE_NODE)result=TryR6(node,&dummy);
    if(!result&&node->node_kind==INVERSE_NODE)result=TryR10(node,&dummy);
    if(!result&&node->node_kind==PRODUCT_NODE)result=TryR1(node,&dummy);
    if(!result&&node->node_kind==PRODUCT_NODE)result=TryR2(node,&dummy);
    if(!result&&node->node_kind==PRODUCT_NODE)result=TryR3(node,&dummy);
    if(!result&&node->node_kind==PRODUCT_NODE)result=TryR4(node,&dummy);
    if(!result&&node->node_kind==PRODUCT_NODE)result=TryR7(node,&dummy);
    if(!result&&node->node_kind==PRODUCT_NODE)result=TryR8(node,&dummy);
    if(!result&&node->node_kind==PRODUCT_NODE)result=TryR9(node,&dummy);
    if(result){*replacement=result;
        return true;        //reduction occurred
    }
    TreeNode* child_replacement=0;
    if(ApplyOneRule(node->child[0],&child_replacement))     //left tree reduction
    {
        if(child_replacement){
            DestroyTree(node->child[0]);
            node->child[0]=child_replacement;}return true;
    }
    if(ApplyOneRule(node->child[1],&child_replacement))     //right tree reduction
    {
        if(child_replacement){
            DestroyTree(node->child[1]);        //delete old subtree
            node->child[1]=child_replacement;       //replace with simplified tree
        }return true;
    }return false;
}
TreeNode* Reduce(TreeNode* tree,OutFile* out)
{
    while(true)
    {
        TreeNode* replacement=0;
        if(!ApplyOneRule(tree,&replacement))    //stop when no rules applied
            break;
        if(replacement){DestroyTree(tree);tree=replacement;}
        printf("\n");fflush(NULL);
        if(out && out->file) {
            fprintf(out->file,"\n");fflush(out->file);
        }
        PrintTree(tree,out);        //print updated tree
        PrintExpr(tree,out);        //print updated expression
    }return tree;
}
void StartCompiler(CompilerInfo* pci)
{
    TreeNode* syntax_tree=Parse(pci);
    printf("Parse Tree:\n");fflush(NULL);
    if(pci->out_file.file){fprintf(pci->out_file.file,"Parse Tree:\n");fflush(pci->out_file.file);}
    PrintTree(syntax_tree,&pci->out_file);
    printf("\nReducing to normal form:\n");fflush(NULL);
    if(pci->out_file.file){fprintf(pci->out_file.file,"\nReducing to normal form:\n");fflush(pci->out_file.file);}
    syntax_tree=Reduce(syntax_tree,&pci->out_file);
    printf("\nNormal Form:\n");fflush(NULL);
    if(pci->out_file.file){fprintf(pci->out_file.file,"\nNormal Form:\n");fflush(pci->out_file.file);}
    PrintTree(syntax_tree,&pci->out_file);
    PrintExpr(syntax_tree,&pci->out_file);
    printf("-----------------------------\n");fflush(NULL);
    if(pci->out_file.file){fprintf(pci->out_file.file,"--------------------------\n");fflush(pci->out_file.file);}
    DestroyTree(syntax_tree);
}
int main()
{
    printf("Start main()\n");fflush(NULL);
    CompilerInfo compiler_info("input.txt","output.txt","debug.txt");
    StartCompiler(&compiler_info);
    printf("End main()\n");fflush(NULL);
    return 0;
}