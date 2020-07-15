#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stack>

using namespace std;

int counter = 0;
int ifvar;

map <string, int> VarTable;
map <string, int> lables;

enum LEXEMTYPE {
    NONE,
    NUMBER,
    OPER,
    VARIABLE,
};

enum OPERATOR {
	IF, THEN, ELSE, ENDIF,
	WHILE, ENDWHILE,
	GOTO,
    ASSIGN,
    COLON,
    OR,
    AND,
    BITOR,
    XOR,
    BITAND,
    EQ, NEQ,
    SHL, SHR,
    LEQ, LT, GEQ, GT,
    PLUS, MINUS,
    MULTIPLY, DIV, MOD,
    LBRACKET, RBRACKET,
};

int PRIORITY[] = {
    -4, -4, -4, -4,
    -4, -4,
    -3,
    -2,
    -1,
    0,
    1,
    2,
    3,
    4,
    5, 5,
    6, 6,
    7, 7, 7, 7,
    8, 8,
    9, 9, 9,
    10, 10,
};

string OPERTEXT [] = {
   "if",
   "then",
   "else",
   "endif",
   "while",
   "endwhile",
   "goto",
   ":=",
   ":",
   "or",
   "and",
   "|",
   "^",
   "&",
   "==", "!=",
   "<<", ">>",
   "<=", "<", ">=", ">",
   "+", "-",
   "*", "/", "%",
   "(", ")",
};

class Lexem {
	LEXEMTYPE type;
public:
	Lexem() {};
	~Lexem() {};
	void setType(LEXEMTYPE t) {
		type = t;
	}
	LEXEMTYPE getType() {
		return type;
	}
	virtual void print() {
		cout << "Lexem" << endl;
	}
};

class Number: public Lexem {
	int value;
public:
	Number();
	int getValue() {
		return value;
	}
	Number(int val) {
		setType(NUMBER);
		value = val;
	}
	void print() {
		cout << value;
	}
};


class Variable: public Lexem {
	int value;
	string name;
public:
	int getValue() {
		return VarTable[name];
	}
	string getname() {
		return name;
	}

	Variable(string name1) {
		setType(VARIABLE);
		name = name1;
	}

	Variable(string name1, int n) {
		name = name1;
		value = n;
		VarTable[name] = value;
	}

	void setValue(int value1) {
		value = value1;
		VarTable[name] = value;
	}
	void print() {
		cout << name;
	}
	int inLabelTable() {
		return lables[name];
	}
};

class Oper: public Lexem {
	OPERATOR opertype;
	string out;
public:
	Oper() {};
	~Oper() {};
	OPERATOR get_op() {
		return opertype;
	}
	Oper(int n) {
		setType(OPER);
		opertype = OPERATOR(n);
		out = OPERTEXT[n];
	}
	int getPriority() {
		return PRIORITY[opertype];
	}
	void print() {
		cout << out;
	}
};


class Goto: public Oper {
	int row = -1;
	int operation;
public:
	Goto(int n) : Oper(n) {
		row = -1;
		operation = n;
	}
	void setRow(int n) {
		row = n;
	}
	void setRow(const string &label) {
		row = lables[label];
	}
	int getrow() {
		return row;
	}
	void print() {
		cout << OPERTEXT[operation];
	}
};

Lexem *get_number(string codeline, string::iterator &pointer) {
	if (!isdigit(*pointer)) {
		return nullptr;
	}
	int val = 0;
	while((pointer != codeline.end()) && isdigit(*pointer)) {
		counter++;
		val *= 10;
		val += (*pointer) - '0';
		pointer++;
	}
	Lexem *ptr = new Number(val);
	return ptr;
}

Lexem *get_var(string codeline, string::iterator &pointer) {
	if (!isalpha(*pointer)) {
		return nullptr;
	}
	string str;
	while ((pointer != codeline.end()) && isalpha(*pointer)) {
		counter++;
		str += (*pointer);
		pointer++;
	}
	Lexem *ptr = new Variable(str);
	return ptr;
}

Lexem *get_operator(string codeline, string::iterator &ptr) {
    if(isdigit(*ptr)) {
        return nullptr;
    }
    Lexem *operation = nullptr;
    int n = sizeof(OPERTEXT) / sizeof(string);
    int op;
    for (op = 0; op < n; op++) {
        int len = OPERTEXT[op].size();
        if (OPERTEXT[op] == codeline.substr(counter, len)) {
            ptr += OPERTEXT[op].size();
            counter += OPERTEXT[op].size();
            if (op == GOTO || op == IF || op == ELSE || op == WHILE || op == ENDWHILE) {
            	return new Goto(op);
            }
           	return new Oper(op);
        }
    }
    return nullptr;
}

void initJumps(vector <vector <Lexem *>> infix) {
	stack <Goto *> stackIfElse;
	for (int row = 0; row < (int)infix.size(); row++) {
		for (int i = 0; i < (int)infix[row].size(); i++) {
            if(infix[row][i] == nullptr) {
                continue;
            } else if (infix[row][i] -> getType() == OPER) {
				Oper *lexemoper = (Oper *)infix[row][i];
				if (lexemoper -> get_op() == IF) {
					stackIfElse.push((Goto *)lexemoper);
				}
				if (lexemoper -> get_op() == ELSE) {
					stackIfElse.top() -> setRow(row + 1);
					stackIfElse.pop();
					stackIfElse.push((Goto *)lexemoper);
				}
				if (lexemoper -> get_op() == ENDIF) {
					stackIfElse.top() -> setRow(row + 1);
					stackIfElse.pop();
				}
			}
		}
	}
	stack <Goto *> stackWhile;
	for (int row = 0; row < (int)infix.size(); row++) {
		for (int i = 0; i < (int)infix[row].size(); i++) {
            if(infix[row][i] == nullptr) {
                continue;
            } else if (infix[row][i] -> getType() == OPER) {
				Oper *lexemoper = (Oper *)infix[row][i];
				if (lexemoper -> get_op() == WHILE) {
					Goto *lexemgoto = (Goto *)lexemoper;
					lexemgoto -> setRow(row);
					stackWhile.push(lexemgoto);
				}
				if (lexemoper -> get_op() == ENDWHILE) {
					Goto *lexemgoto = (Goto *)lexemoper;
					lexemgoto -> setRow(stackWhile.top() -> getrow());
					stackWhile.top() -> setRow(row + 1);
					stackWhile.pop();
				}
			}
		}
	}
}


vector <Lexem *> parseLexem(string codeline) {
	vector <Lexem *> infix;
	Lexem * ptr = NULL;
	for(auto pointer = codeline.begin(); pointer != codeline.end();) {
		if ((*pointer) == ' ' || (*pointer) == '	') {
			counter++;
			pointer++;
			continue;
		}
		ptr = get_number(codeline, pointer);
		if (ptr != nullptr) {
			infix.push_back(ptr);
			continue;
		}
		ptr = get_operator(codeline, pointer);
		if (ptr != nullptr) {
			infix.push_back(ptr);
			continue;
		}
		ptr = get_var(codeline, pointer);
        if (ptr != nullptr) {
            infix.push_back(ptr);
            continue;
        }
	}
	return infix;
}

void print(vector <vector <Lexem *>> array) {
	for(auto ptr : array) {
		for (auto qwe : ptr) {
			qwe -> print();
			cout << " ";
		}
		cout << "" << endl;
	}
}


void initLables(vector <Lexem *> &infix, int row) {
	for(int i = 1; i < (int)infix.size(); i++) {
		if (infix[i - 1] -> getType() == VARIABLE && infix[i] -> getType() == OPER) {
			Variable *lexemvar = (Variable *)infix[i - 1];
			Oper *lexemop = (Oper *)infix[i];
			if (lexemop -> get_op() == COLON) {
				lables[lexemvar -> getname()] = row;
				delete infix[i - 1];
				delete infix[i];
				infix[i - 1] = nullptr;
				infix[i] = nullptr;
				i++;
			}
		}
	}
}

void joinGotoAndLabel(Variable *lexemvar, stack <Lexem *> &stack){
	if (((Oper *)stack.top()) -> get_op() == GOTO) {
		Goto *lexemgoto = (Goto *)stack.top();
		lexemgoto -> setRow(lexemvar -> getname());
	}
}

vector <Lexem *> buildPostfix(vector <Lexem *> infix) {
	int count = 0;
	stack <Lexem *> st;
	vector <Lexem *> postfix;
	Oper *q;
	for (auto ptr : infix) {
		if (ptr == nullptr) {
			continue;
		}
		if (ptr -> getType() == VARIABLE) {
			Variable *lexemvar = (Variable *)ptr;
			if (lexemvar -> inLabelTable()){
				joinGotoAndLabel(lexemvar, st);
			} else {
				postfix.push_back(ptr);
			}
		} else if (ptr -> getType() == NUMBER) {
			if (count != 0) {
				count--;
			}
			postfix.push_back(ptr);
		} else if (ptr -> getType() == OPER) {
			q = (Oper *) ptr;
			if (q -> get_op() == ENDIF || q -> get_op() == THEN) {
				continue;
			}
			if (q -> get_op() == RBRACKET) {
                if (!st.empty()) {
                    while((!st.empty()) && ((Oper *)st.top()) -> get_op() != LBRACKET) {
                        postfix.push_back(st.top());
                        st.pop();
                    }
                }
                if(!st.empty()) {
                    st.pop();
                }
                continue;
            } else if(q -> get_op() != LBRACKET) {
                while(!(st.empty()) &&
                        ((Oper *)st.top()) -> get_op() != LBRACKET &&
                        (q -> getPriority()) < ((Oper *)st.top()) -> getPriority()) {
		                    postfix.push_back(st.top());
		                    st.pop();
		                }
	            }
		        st.push(ptr);
        }
	}
	while(!st.empty()) {
        postfix.push_back(st.top());
        st.pop();
    }
    return postfix;
};

Lexem *calc(Lexem *left, Lexem *right, Lexem *op) {
	Lexem *ptr;
	Oper *q;
	int n;
	q = (Oper *) op;
	int l, r, value;
	l = ((Number *) left) -> getValue();
	r = ((Number *) right) -> getValue();
	if (q -> get_op() == ASSIGN) {
        if(right -> getType() == VARIABLE) {
            value = ((Variable *)right) -> getValue();
        }	else if (right -> getType() == NUMBER){
            value = ((Number *)right) -> getValue();
        }
        if (left -> getType() == VARIABLE) {
 	        ((Variable *)left) -> setValue(value);
        }
        ptr = new Number(value);
    	ifvar = 1;
        return ptr;
    }
    if(left -> getType() == VARIABLE) {
        l = ((Variable *)left) -> getValue();
    } else if (left -> getType() == NUMBER){
        l = ((Number *)left) -> getValue();
	}
    if(right -> getType() == VARIABLE) {
        r = ((Variable *)right) -> getValue();
    } else if (right -> getType() == NUMBER){
        r = ((Number *)right) -> getValue();
    }
    if (q -> get_op() == PLUS) {
		ptr = new Number(l + r);
	} else if (q -> get_op() == MINUS) {
		ptr = new Number(l - r);
	} else if (q -> get_op() == MULTIPLY) {
		ptr = new Number(l * r);
	} else if (q -> get_op() == OR) {
		ptr = new Number(l || r);
	} else if (q -> get_op() == AND) {
		ptr = new Number(l && r);
	} else if (q -> get_op() == BITOR) {
		ptr = new Number(l | r);
	} else if (q -> get_op() == XOR) {
		ptr = new Number(l ^ r);
	} else if (q -> get_op() == BITAND) {
		ptr = new Number(l & r);
	} else if (q -> get_op() == EQ) {
		ptr = new Number(l == r);
	} else if (q -> get_op() == NEQ) {
		ptr = new Number(l != r);
	} else if (q -> get_op() == LEQ) {
		ptr = new Number(l <= r);
	} else if (q -> get_op() == LT) {
		ptr = new Number(l < r);
	} else if (q -> get_op() == GEQ) {
		ptr = new Number(l >= r);
	} else if (q -> get_op() == GT) {
		ptr = new Number(l > r);
	} else if (q -> get_op() == SHL) {
		ptr = new Number(l << r);
	} else if (q -> get_op() == SHR) {
		ptr = new Number(l >> r);
	} else if (q -> get_op() == MOD) {
		ptr = new Number(l % r);
	}
	return ptr;
}

int evaluatePostfix(vector <Lexem *> postfix, int row) {
	stack <Lexem *> st;
	Lexem * ans;
	int n;
	for (auto ptr : postfix) {
		ifvar = 0;
		if (ptr -> getType() == NUMBER || ptr -> getType() == VARIABLE) {
			st.push(ptr);
		} else {
			if (((Oper *)ptr) -> get_op() == GOTO || ((Oper *)ptr) -> get_op() == ELSE || ((Oper *)ptr) -> get_op() == ENDWHILE) {
				Goto *lexemgoto = (Goto *)ptr;
				return lexemgoto -> getrow();
			} else if (((Oper *)ptr) -> get_op() == IF || ((Oper *)ptr) -> get_op() == WHILE) {
				Goto *lexemgoto = (Goto *)ptr;
				int rvalue = ((Number *)st.top()) -> getValue();
				st.pop();
				if (!rvalue) {
					return lexemgoto -> getrow();
				}
			} else {
				Lexem * left, *right;
				right = st.top();
				st.pop();
				left = st.top();
				st.pop();
				ans = calc(left, right, ptr);
				st.push(ans);
			}
		}
	}
	if (!st.empty()) {
		if(st.top() -> getType() == VARIABLE) {
			n = ((Variable *)st.top()) -> getValue();
			st.pop();
			cout << n << endl;
			return row + 1;
		}
		n = ((Number *)st.top()) -> getValue();
		st.pop();
		if (ifvar == 0) {
			cout << n << endl;
		}
	}
	return row + 1;
}

void delet(vector <vector <Lexem *>> &v) {
    for(auto ptr: v) {
    	for (auto qwe : ptr) {
        	delete qwe;
    	}
    }
}


int main() {
	string codeline ;
	vector <vector <Lexem *>> infix, postfix;
	while(getline(cin, codeline)) {
		if (codeline == "exit" || codeline == "quit") {
			break;
		}
		counter = 0;
		infix.push_back(parseLexem(codeline));
	}
	for (int row = 0; row < (int)infix.size(); ++row) {
		initLables(infix[row], row);
	}
	initJumps(infix);
	for(auto &inf : infix) {
		postfix.push_back(buildPostfix(inf));
	}
	print(postfix);
	int row = 0;
	while (0 <= row && row < (int)postfix.size()) {
		row = evaluatePostfix(postfix[row], row);
	}
	return 0;
}