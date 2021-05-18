#include "parser.h"

void require(Text* a, char symbol);
double executeUnaryOperation(double value, size_t op_code);
void SyntaxError(Text* a, const char* error);
Node* createNode(char type, Value v, Node* left_child, Node* right_child);

char getChar(Text* a, int offs);
Node* getProg(Text* a);
Node* getStat(Text* a, DynamicArray* variables, size_t tab_count);
Node* getStat(Text* a, DynamicArray* variables, size_t tab_count);
Node* getLoop(Text* a, DynamicArray* variables, size_t tab_count);
Node* getCond(Text* a, size_t tab_count);
Node* getJump(Text* a);
Node* getFdec(Text* a, size_t tab_count);
Node* getAssg(Text* a, DynamicArray* variables);
Node* getExpr(Text* a);
Node* getSimp(Text* a);
Node* getTerm(Text* a);
Node* getPrim(Text* a);
Node* getCall(Text* a);
void destroySingleNode(Node* node);
Node* getBlck(Text* a, DynamicArray* variables, size_t tabs_expected);
char* getVar(Text* a);
Node* getNum(Text* a);
Node* newTermSimp(unsigned char math_number);
Node* createVarDefinition(char* var_name, Node* expr, int var_number);
Node* createCall(char* func_name);
Node* createCallArg(Node* expression);
Node* newCompare(unsigned char comp_number);
Node* createConst(double value);
Node* createAssg(char* var_name, Node* expr);


//#define CONST(x) createNode(TYPE_CONST, {.const_value=x}, NULL, NULL)
//
// #define ADD(x, y) createNode(TYPE_BIN_OP, {.op_value=OP_PLUS}, x, y)
//
// #define SUB(x, y) createNode(TYPE_BIN_OP, {.op_value=OP_MINUS}, x, y)
//
// #define MUL(x, y) createNode(TYPE_BIN_OP, {.op_value=OP_MUL}, x, y)
//
// #define DIV(x, y) createNode(TYPE_BIN_OP, {.op_value=OP_DIV}, x, y)
//
// #define POW(x, y) createNode(TYPE_BIN_OP, {.op_value=OP_POW}, x, y)
//
// #define SIN(x) createNode(TYPE_UN_OP, {.op_value=OP_SIN}, x, NULL)
//
// #define COS(x) createNode(TYPE_UN_OP, {.op_value=OP_COS}, x, NULL)
//
// #define TAN(x) createNode(TYPE_UN_OP, {.op_value=OP_TAN}, x, NULL)
//
// #define CTG(x) createNode(TYPE_UN_OP, {.op_value=OP_CTG}, x, NULL)
//
// #define LN(x) createNode(TYPE_UN_OP, {.op_value=OP_LN}, x, NULL)
//
// #define SQRT(x) createNode(TYPE_UN_OP, {.op_value=OP_SQRT}, x, NULL)

/*const char* unary_operations[UNARYOPSCOUNT] = {
    "sin",
    "cos",
    "tg",
    "ctg",
    "sqrt"
};*/

Node* createNode(char type, Value v, Node* left_child, Node* right_child) {
    Node* new_node = (Node*)calloc(1, sizeof(Node));
    new_node->type = type;
    new_node->value = v;
    new_node->left = left_child;
    new_node->right = right_child;
    return new_node;
}

Node* parseString(char* s) {
    Text* a = (Text*)calloc(1, sizeof(Text));
    a->s = s;
    Node* res = getProg(a);
    return res;
}

void require(Text* a, char symbol) {
    //printf("requiring symbol: %c %d\n", symbol, symbol);
    if (a->s[a->p] != symbol) {
        SyntaxError(a, "wrong symbol");
    }
}

// double executeUnaryOperation(double value, size_t op_code) {
//     switch (op_code) {
//         case OP_SIN: return sin(value);
//         case OP_COS: return cos(value);
//         case OP_TAN: return tan(value);
//         case OP_CTG: return 1/tan(value);
//         case OP_SQRT: return sqrt(value);
//         default: assert(!"Wrong operation");
//     }
// }

Node* createFdec(char* func_name) {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = D_TYPE;
    result->right = (Node*)calloc(1, sizeof(Node));
    result->right->type = ID_TYPE;
    result->right->value.name = func_name;
    return result;
}

Node* createDefArg(char* arg_name) {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = ID_TYPE;
    result->value.name = arg_name;
    return result;
}

void setOffset(Text* a, size_t new_ofs) {
    a->p = new_ofs;
}

size_t nextToken(Text* a, size_t offs) {
    while (getChar(a, offs) == ' ') {
        offs++;
    }
    a->p += offs;
    return offs;
}

bool isFdec(char* s) {
    return strncmp(s, "def", 3);
}

bool isCond(char* s) {
    return strncmp(s, "if", 2);
}

bool isLoop(char* s) {
    return strncmp(s, "while", 5);
}

bool isJump(char* s) {
    return strncmp(s, "return", 6);
}

bool isUpper(char c) {
    return 'A' <= c && c <= 'Z';
}

bool isLower(char c) {
    return 'a' <= c && c <= 'z';
}

bool isNum(char c) {
    return '0' <= c && c <= '9';
}

bool isVarStart(char c) {
    return isUpper(c) || isLower(c) || c == '_';
}

bool isVarChar(char c) {
    return isUpper(c) || isLower(c) || isNum(c) || c == '_';
}

char getChar(Text* a, int offs) {
    return *(a->s + a->p + offs);
}

size_t countTabs(Text* a) {
    size_t result = 0;
    while (getChar(a, 0) == ' ' || getChar(a, 0) == '\t') {
        if (getChar(a, 0) == ' ') {
            result++;
        } else if (getChar(a, 0) == '\t') {
            result += 4;
        }
        a->p++;
    }
    return result;
}

Node* newBlock() {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = COMP_TYPE;
    return result;
}

Node* getBlck(Text* a, DynamicArray* variables, size_t tabs_expected) {
    Node* current_block = newBlock();
    Node* current_leaf = current_block;
    //DynamicArray* variables = newDynamicArray();
    size_t last_offs = a->p;
    size_t tab_count = countTabs(a);
    if (tabs_expected != tab_count) {
        printf("Wrong tabs amount, expected %d\n", tabs_expected);
        SyntaxError(a, "wrong amount of tabs!");
    }
    // printf("yay getting block with %u tabs\n", tab_count);
    a->p = last_offs;
    while (true) {
        size_t cur_offs = a->p;
        size_t tabs = countTabs(a);
        // printf("tabs are %zu (ptr is %zu)\n", tabs, cur_offs);
        if (tabs > tab_count) {
            SyntaxError(a, "wrong amount of tabs!");
        }
        if (tabs < tab_count) {
            a->p = cur_offs;
            // printf("yeee tabs broke and its %zu at pointer %zu\n", tabs, a->p);
            return current_block;
        }
        Node* statement = getStat(a, variables, tabs);
        //a->p++;
        if (statement->type != BLANK_LINE_TYPE) {
            current_leaf->right = statement;
            current_leaf = statement;
        } else {
            destroySingleNode(statement);
        }
        //nextToken(a, 0);
        if (getChar(a, 0) == '\0') {
            break;
        }
        printf("yeah block with var cnt %d\n", variables->size);
    }
    //printf("YEEEEEEEEEEEEEEEEEEEEEEEEEEE block with %u tabs got\n", tab_count);
    return current_block;
}

Node* getProg(Text* a) {
    DynamicArray* outside_vars = newDynamicArray();
    Node* block = getBlck(a, outside_vars, 0);
    block->value.variables = outside_vars;
    return block;
}

Node* newStat() {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = STAT_TYPE;
    return result;
}

Node* createLoop() {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = LOOP_TYPE;
    return result;
}

Node* createCond() {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = COND_TYPE;
    result->right = (Node*)calloc(1, sizeof(Node));
    result->right->type = IFEL_TYPE;
    return result;
}

Node* getLoop(Text* a, DynamicArray* variables, size_t tab_count) {
    size_t last_offs = a->p;
    if (strncmp(a->s + a->p, "while", 5) == 0) {
        nextToken(a, 5);
        Node* condition = getExpr(a);
        if (condition == NULL) {
            SyntaxError(a, "condition expected");
        }
        require(a, ':');
        nextToken(a, 1);
        require(a, '\n');
        a->p++;
        Node* block = getBlck(a, variables, tab_count + 4);
        if (block == NULL) {
            SyntaxError(a, "block expected");
        }
        Node* cur_ans = createLoop();
        cur_ans->left = condition;
        cur_ans->right = block;
        return cur_ans;
    }
    return NULL;
}

bool hasElse(Text* a) {
    size_t offs = 0;
    while (true) {
        //printf("%d %d ", *(a->s + a->p + offs), isspace(*(a->s + a->p + offs)));
        if (*(a->s + a->p + offs) == '\0') {
            // printf("\n");
            return false;
        }
        if (isspace(*(a->s + a->p + offs)) != 0) {
            //printf("its not space!! ");
            offs++;
            continue;
        }
        if (strncmp(a->s + a->p + offs, "else", 4) == 0) {
            // printf("\n");
            return true;
        } else {
            // printf("\n");
            return false;
        }
    }
}

Node* getCond(Text* a, size_t tab_count, DynamicArray* variables) {
    //printf("trying to find a condition... %zu %c\n", a->p, getChar(a, 0));
    size_t last_offs = a->p;
    if (strncmp(a->s + a->p, "if", 2) == 0) {
        //printf("it is condition (if)\n");
        nextToken(a, 2);
        Node* condition = getExpr(a);
        if (condition == NULL) {
            SyntaxError(a, "condition expected");
        }
        require(a, ':');
        nextToken(a, 1);
        require(a, '\n');
        a->p++;
        Node* true_block = getBlck(a, variables, tab_count + 4);
        if (true_block == NULL) {
            SyntaxError(a, "block expected");
        }
        Node* cur_ans = createCond();
        cur_ans->left = condition;
        cur_ans->right->left = true_block;
        if (hasElse(a)) {
            //printf("even with else (i think)\n");
            size_t cur_tabs = countTabs(a);
            if (cur_tabs == tab_count) {
                nextToken(a, 4);
                require(a, ':');
                nextToken(a, 1);
                require(a, '\n');
                a->p++;
                Node* false_block = getBlck(a, variables, tab_count + 4);
                cur_ans->right->right = false_block;
            }
        }
        return cur_ans;
    }
    return NULL;
}

Node* createJump() {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = JUMP_TYPE;
    return result;
}

Node* getJump(Text* a) {
    if (strncmp(a->s + a->p, "return", 6) == 0) {
        printf("ye looks like return\n");
        nextToken(a, 6);
        Node* ret_expr = getExpr(a);
        printf("%p\n", ret_expr);
        Node* result = createJump();
        if (ret_expr != NULL) {
            //SyntaxError(a, "expression needed");
            result->right = ret_expr;
        }
        return result;
    }
    return NULL;
}

Node* getStat(Text* a, DynamicArray* variables, size_t tab_count) {
    //printf("yay getting statement\n");
    Node* stat = newStat();
    Node* res = NULL;
    res = getLoop(a, variables, tab_count);
    if (res != NULL) {
        stat->left = res;
        return stat;
    }
    // printf("not loop...\n");

    res = getCond(a, tab_count, variables);
    if (res != NULL) {
        stat->left = res;
        return stat;
    }
    // printf("not cond...\n");

    res = getJump(a);
    if (res != NULL) {
        require(a, '\n');
        a->p++;
        stat->left = res;
        return stat;
    }
    // printf("not jump...\n");

    res = getFdec(a, tab_count);
    if (res != NULL) {
        stat->left = res;
        return stat;
    }
    // printf("not fdec...\n");

    res = getAssg(a, variables);
    if (res != NULL) {
        require(a, '\n');
        //nextToken(a, 1);
        a->p++;
        //printf("okay, statement is actually assignation %zu %c\n", a->p, getChar(a, 0));
        stat->left = res;
        return stat;
    }
    //printf("not assg...\n");

    res = getExpr(a);
    if (res != NULL) {
        require(a, '\n');
        a->p++;
        stat->left = res;
        return stat;
    }
    //printf("not even assg... its nothing statement!\n");
    // its a blank line!!
    nextToken(a, 0);
    require(a, '\n');
    a->p++;
    stat->type = BLANK_LINE_TYPE;
    //destroySingleNode(stat);
    return stat;
}

int getCompSignNum(Text* a) {
    if (strncmp(a->s + a->p, "==", 2) == 0) {
        return OP_EQUAL;
    }
    if (strncmp(a->s + a->p, "<=", 2) == 0) {
        return OP_LOE;
    }
    if (strncmp(a->s + a->p, ">=", 2) == 0) {
        return OP_GOE;
    }
    if (strncmp(a->s + a->p, "!=", 2) == 0) {
        return OP_NE;
    }
    if (getChar(a, 0) == '<') {
        return OP_LESS;
    }
    if (getChar(a, 0) == '>') {
        return OP_GREATER;
    }
    return -1;
}

const char* getSignChar(char number) {
    switch (number) {
        case OP_EQUAL   : return "==";
        case OP_LOE     : return "<=";
        case OP_GOE     : return ">=";
        case OP_NE      : return "!=";
        case OP_LESS    : return "<";
        case OP_GREATER : return ">";
        case OP_ADD     : return "+";
        case OP_SUB     : return "-";
        case OP_MUL     : return "*";
        case OP_DIV     : return "/";
        default : return "wrong sign";
    }
}

int getSimpOpNum(Text* a) {
    if (getChar(a, 0) == '+') {
        return OP_ADD;
    }
    if (getChar(a, 0) == '-') {
        return OP_SUB;
    }
    return -1;
}

int getTermOpNum(Text* a) {
    if (getChar(a, 0) == '*') {
        return OP_MUL;
    }
    if (getChar(a, 0) == '/') {
        return OP_DIV;
    }
    return -1;
}

Node* getVarNode(Text* a){
    char* var_name = getVar(a);
    if (var_name == NULL) {
        return NULL;
    }
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = ID_TYPE;
    result->value.name = var_name;
    return result;
}

int CompareVarUsage(const void* var1, const void* var2) {
    Variable* v1 = (Variable*)var1;
    Variable* v2 = (Variable*)var2;
    return (v2->usage - v2->usage);
}

Node* getFdec(Text* a, size_t tab_count) {
    size_t last_offs = a->p;
    if (strncmp(a->s + a->p, "def", 3) == 0) {
        // printf("getting fdec\n");
        nextToken(a, 3);
        // printf("yeah!! %c\n", getChar(a, 0));
        char* func_name = getVar(a);
        // printf("yeah!! %s\n", func_name);
        if (func_name == NULL) {
            SyntaxError(a, "expected function name");
        }
        Node* result = createFdec(func_name);
        require(a, '(');
        nextToken(a, 1);
        //printf("yay it was (\n");
        DynamicArray* func_variables = newDynamicArray();
        result->value.variables = func_variables;
        //printf("yeah its really a %p\n", func_variables);
        assert(func_variables != NULL);
        //printf("asserted ok!!\n");
        if (getChar(a, 0) == ')') {
            nextToken(a, 1);
            require(a, ':');
            nextToken(a, 1);
            require(a, '\n');
            a->p++;
            Node* block = getBlck(a, func_variables, tab_count + 4);
            result->value.variables = func_variables;
            result->right->left = block;
            return result;
            return result;
        }
        //printf("if was ok!!\n");
        char* arg_name = getVar(a);
        if (arg_name == NULL) {
            SyntaxError(a, "expected arg name");
        }
        DApushBack(result->value.variables, {arg_name, 1, 0, true});
        Node* cur_arg = createDefArg(arg_name);
        result->right->right = cur_arg;
        while (getChar(a, 0) == ',') {
            nextToken(a, 1);
            arg_name = getVar(a);
            DApushBack(result->value.variables, {arg_name, 1, 0, true});
            if (arg_name == NULL) {
                SyntaxError(a, "expected arg name");
            }
            cur_arg->right = createDefArg(arg_name);
        }
        require(a, ')');
        nextToken(a, 1);
        require(a, ':');
        nextToken(a, 1);
        require(a, '\n');
        a->p++;
        func_variables->arg_cnt = func_variables->size;
        Node* block = getBlck(a, func_variables, tab_count + 4);
        result->value.variables = func_variables;
        result->right->left = block;
        qsort(func_variables->array, func_variables->size, sizeof(Variable), CompareVarUsage);
        return result;
    }
    return NULL;
}

Node* getAssg(Text* a, DynamicArray* variables) {
    size_t last_offs = a->p;
    char* var_name = getVar(a);
    if (getChar(a, 0) == '=') {
        //printf("yay getting assg\n");
        //printf("variable is %s (at ptr %zu)\n", var_name, last_offs);
        nextToken(a, 1);
        Node* expr = getExpr(a);
        if (expr == NULL) {
            SyntaxError(a, "expression expected");
        }
        //printf("assignation of %zu got expr!\n", last_offs);
        int var_number = DAfind(variables, var_name);
        printf("find result: %d\n", var_number);
        if (var_number != -1) {
            //printf("finally, assg and not decl\n");
            variables->array[var_number].usage++;
            return createAssg(var_name, expr);
        } else {
            DApushBack(variables, {var_name, 1, 0, false});
            printf("-------------------- var %s %p\n", var_name, var_name);
            return createVarDefinition(var_name, expr, variables->size - 1);
        }
    }
    a->p = last_offs;
    return NULL;
}

Node* getCall(Text* a) {
    size_t last_offs = a->p;
    char* func_name = getVar(a);
    if (func_name != NULL) {
        Node* func = createCall(func_name);
        if (getChar(a, 0) == '(') {
            nextToken(a, 1);
            if (getChar(a, 0) == ')') {
                nextToken(a, 1);
                return func;
            }
            Node* cur_expr = getExpr(a);
            func->right = createCallArg(cur_expr);
            Node* cur_arg = func->right;
            while (getChar(a, 0) == ',') {
                nextToken(a, 1);
                cur_expr = getExpr(a);
                if (cur_expr == NULL) {
                    SyntaxError(a, "Expr expected");
                }
                cur_arg->right = createCallArg(cur_expr);
                cur_arg = cur_arg->right;
            }
            require(a, ')');
            nextToken(a, 1);
            return func;
        }
    }
    a->p = last_offs;
    return NULL;
}

Node* getPrim(Text* a) {
    printf("getting prim\n");
    //printf("pointer is %d %c\n", a->p, getChar(a, 0));
    if (getChar(a, 0) == '(') {
        //printf("yay from prim to expr\n");
        nextToken(a, 1);
        Node* res = getExpr(a);
        require(a, ')');
        nextToken(a, 1);
        return res;
    }
    //printf("pointer is %d %c\n", a->p, getChar(a, 0));
    Node* res = getCall(a);
    if (res != NULL) {
        printf("its call!!\n");
        return res;
    }
    //printf("pointer is %d %c\n", a->p, getChar(a, 0));
    res = getVarNode(a);
    if (res != NULL) {
        printf("its var!!\n");
        return res;
    }
    //printf("pointer is %d %c\n", a->p, getChar(a, 0));
    res = getNum(a);
    if (res != NULL) {
        printf("its num!!\n");
        return res;
    }
    //printf("pointer is %d %c and its just nothing!\n", a->p, getChar(a, 0));
    return NULL;
    //SyntaxError(a, "expected (Expr) or call or var or num");
}

Node* getTerm(Text* a) {
    printf("getting term\n");
    size_t last_offs = a->p;
    Node* start_simp = getPrim(a);
    printf("term::prim got %p\n", start_simp);
    //nextToken(a, 0);
    int res = 0;
    while (true) {
        nextToken(a, 0);
        if ((res = getTermOpNum(a)) != -1) {
            nextToken(a, 1);
            Node* new_start = newTermSimp(res);
            new_start->left = start_simp;
            Node* right = getPrim(a);
            if (right == NULL) {
                SyntaxError(a, "no simple statement found");
            }
            new_start->right = right;
            start_simp = new_start;
        } else {
            break;
        }
    }
    //printf("exiting term\n");
    return start_simp;
}

Node* getSimp(Text* a) {
    printf("getting simp\n");
    size_t last_offs = a->p;
    Node* start_simp = getTerm(a);
    printf("%p term\n", start_simp);
    //nextToken(a, 0);
    int res = 0;
    while (true) {
        nextToken(a, 0);
        if ((res = getSimpOpNum(a)) != -1) {
            //printf("bruh res is %d\n", res);
            nextToken(a, 1);
            Node* new_start = newTermSimp(res);
            new_start->left = start_simp;
            Node* right = getTerm(a);
            if (right == NULL) {
                SyntaxError(a, "no simple statement found");
            }
            new_start->right = right;
            start_simp = new_start;
        } else {
            break;
        }
    }
    //printf("exiting simp\n");
    return start_simp;
}

Node* getExpr(Text* a) {
    printf("getting expr\n");
    size_t last_offs = a->p;
    Node* start_statement = getSimp(a);
    printf("%p\n", start_statement);
    //nextToken(a, 0);
    int res = 0;
    while (true) {
        nextToken(a, 0);
        if ((res = getCompSignNum(a)) != -1) {
            if (res == OP_LESS || res == OP_GREATER) {
                nextToken(a, 1);
            } else {
                nextToken(a, 2);
            }
            Node* new_start = newCompare(res);
            new_start->left = start_statement;
            Node* right = getSimp(a);
            if (right == NULL) {
                SyntaxError(a, "no simple statement found");
            }
            new_start->right = right;
            start_statement = new_start;
        } else {
            break;
        }
    }
    //printf("exiting expr\n");
    return start_statement;
}

char* getVar(Text* a) {
    size_t last_offs = a->p;
    size_t length = 0;
    if (isVarStart(getChar(a, 0))) {
        a->p++;
        length++;
        while (isVarChar(getChar(a, 0))) {
            length++;
            a->p++;
        }
        char* var_name = (char*)calloc(length + 1, sizeof(char));
        strncpy(var_name, a->s + last_offs, length);
        nextToken(a, 0);
        return var_name;
    } else {
        return NULL;
    }
}

Node* getNum(Text* a) {
    int64_t x = 0;
    size_t size = 0;
    int res = sscanf(a->s + a->p, "%lld%n\n", &x, &size);
    if (res == 0 || size == 0) {
        return NULL;
        //SyntaxError(a, "not a double");
    }
    nextToken(a, size);
    //printf("num got!!\n");
    return createConst(x);
}

Node* createConst(double value) {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = NUMB_TYPE;
    result->value.num = value;
    return result;
}

void destroySingleNode(Node* node) {
    free(node);
}

Node* newCompare(unsigned char comp_number) {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = MATH_TYPE;
    result->value.math = comp_number;
    return result;
}

Node* newTermSimp(unsigned char math_number) {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = MATH_TYPE;
    result->value.math = math_number;
    return result;
}

Node* createCallArg(Node* expression) {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = ARG_TYPE;
    result->left = expression;
    return result;
}

Node* createCall(char* func_name) {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = CALL_TYPE;
    result->left = (Node*)calloc(1, sizeof(Node));
    result->left->type = ID_TYPE;
    result->left->value.name = func_name;
    return result;
}

Node* createVarDefinition(char* var_name, Node* expr, int var_number) {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = DECL_TYPE;
    result->value.var_number = var_number;
    result->left = (Node*)calloc(1, sizeof(Node));
    result->left->type = ID_TYPE;
    result->left->value.name = var_name;
    printf("eeeee, %p\n", result->left->value.name);
    result->right = expr;
    return result;
}

Node* createAssg(char* var_name, Node* expr) {
    Node* result = (Node*)calloc(1, sizeof(Node));
    result->type = ASSG_TYPE;
    result->left = (Node*)calloc(1, sizeof(Node));
    result->left->type = ID_TYPE;
    result->left->value.name = var_name;
    result->right = expr;
    return result;
}

void printToEndline(char* s) {
    while (*s != '\n') {
        putchar(*s);
        ++s;
    }
}

void SyntaxError(Text* a, const char* error) {
    size_t start = a->p;
    bool changed = false;
    while (start > 0 && *(a->s + start) != '\n') {
        --start;
        changed = true;
    }
    if (changed) {
        ++start;
    }
    size_t end = a->p;
    while (*(a->s + end) != '\0' && *(a->s + end) != '\n') {
        ++end;
    }
    printf("%.*s\n", end - start, (a->s + start));
    for (size_t i = start; i < a->p; ++i) {
        printf(" ");
    }
    printf("^\n");
    printf("Error! index: %u (symbol %c)\n", a->p, a->s[a->p]);
    printf("%s\n", error);
    assert(!"OK");
}

void graphWriteNodeLight(Node* node, FILE* file) {
    assert(node);
    assert(file);
    //printf("yeah! type is %d\n", node->type);
    fprintf(file, "\tel%p [style=filled, fillcolor=\"#adffbf\", label=\"", node);
    if (node->type == D_TYPE) {
        fprintf(file, "D_TYPE");
    } else if (node->type == DECL_TYPE) {
        fprintf(file, "DECL_TYPE");
    } else if (node->type == ID_TYPE) {
        fprintf(file, "ID_TYPE | %s", node->value.name);
    } else if (node->type == ARG_TYPE) {
        fprintf(file, "ARG_TYPE");
    } else if (node->type == COMP_TYPE) {
        fprintf(file, "COMP_TYPE");
    } else if (node->type == STAT_TYPE) {
        fprintf(file, "STAT_TYPE");
    } else if (node->type == COND_TYPE) {
        fprintf(file, "COND_TYPE");
    } else if (node->type == IFEL_TYPE) {
        fprintf(file, "IFEL_TYPE");
    } else if (node->type == LOOP_TYPE) {
        fprintf(file, "LOOP_TYPE");
    } else if (node->type == ASSG_TYPE) {
        fprintf(file, "ASSG_TYPE");
    } else if (node->type == CALL_TYPE) {
        fprintf(file, "CALL_TYPE");
    } else if (node->type == JUMP_TYPE) {
        fprintf(file, "JUMP_TYPE");
    } else if (node->type == MATH_TYPE) {
        fprintf(file, "MATH_TYPE | %s", getSignChar(node->value.math));
    } else if (node->type == NUMB_TYPE) {
        fprintf(file, "NUMB_TYPE | %lld",node->value.num);
    }
    fprintf(file, "\"];\n");
    if (node->left != NULL) {
        graphWriteNodeLight(node->left, file);
        fprintf(file, "el%p -> el%p\n", node, node->left);
    }
    if (node->right != NULL) {
        graphWriteNodeLight(node->right, file);
        fprintf(file, "el%p -> el%p\n", node, node->right);
    }
}

void graphDump(Node* root) {
    //printf("%p\n", root);
    assert(root);
    //printf("yeah! root type is %d\n", root->type);
    FILE* output = NULL;
    open_file(&output, "graph.txt", "w");
    fprintf(output, "digraph structs {\n\trankdir=HR;\n");
    graphWriteNodeLight(root, output);
    fprintf(output, "}\n");
    fclose(output);
    system("dot -Tsvg graph.txt > graph.svg");
}
