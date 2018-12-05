#include <cctype>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <sstream>

//  r3 - то, что возвращается из atom(), term(), expr() 
//  r2 - для доп операций
//  r0 - r3 для вызова функций
//  r4 - для функции

const uint mov_r0_r3 = 0xE1A00003;
const uint mov_r3_r0 = 0xE1A03000;
const uint mov_r3_value = 0xE3003000;
const uint ldr_r3_num = 0xE59F3000;
const uint mov_r2_neg1 = 0xE3E02000;

const uint sub_sp_4 = 0xE24DD004;
const uint add_sp_0 = 0xE28DD000;
const uint add_sp_4 = 0xE28DD004;
const uint mul_r3_r2 = 0xE0030293;
const uint add_r3_r2 = 0xE0833002;
const uint sub_r2_r3 = 0xE0423003;
const uint eor_r3_r2 = 0xE0233002;
const uint add_r3_1 = 0xE2833001;

const uint push_r4 = 0xE52DE004;
const uint str_r3_sp = 0xE58D3000;
const uint ldr_r0_sp = 0xE59D0000;
const uint ldr_r2_sp = 0xE59D2000;
const uint ldr_r3_sp = 0xE59D3000;
const uint ldr_r4_func = 0xE59F4000;
const uint blx_r4 = 0xE12FFF34;
const uint bx_lr = 0xE12FFF1E;
const uint pop_lr = 0xE49DE004;

extern "C" {
  int my_div(int a, int b);
  int my_mod(int a, int b);
  int my_inc(int a);
  int my_dec(int a);
  struct symbol_t {
    const char * name;
    void       * pointer;
  };
  void jit_compile_expression_to_arm(const char* expression, const symbol_t* externs, void* out_buffer);
}

enum TokenValue : char {
  NAME,
  NUMBER,
  END,
  PLUS = '+',
  MINUS = '-',
  MUL = '*',
  LB = '(',
  RB = ')',
  COM = ','
};

enum NumberValue : char {
  NUM0 = '0', NUM1 = '1', NUM2 = '2',
  NUM3 = '3', NUM4 = '4', NUM5 = '5',
  NUM6 = '6', NUM7 = '7', NUM8 = '8',
  NUM9 = '9',
};

TokenValue last_operation_token = NUMBER;
int number_value;                         //  last number value
std::string string_value;                 //  last string value
std::map<std::string, int*> table;        //  names table
std::vector<uint> result;                 //  asm codes
std::vector<std::pair<uint, uint>> big_numbers;  //  pair of offset and number that will be stored at the end
std::vector<std::pair<uint, std::string>> function_ptrs;  //  pair of offset for functions

void expr(std::istream*, bool);            //  expression with +, -
void term(std::istream*, bool);            //  term (multiplicity)
void atom(std::istream*, bool);            //  basic expressions (numbers, variables, functions)

TokenValue get_token(std::istream* input) {
  char ch;
  do {
    if (!input->get(ch)) {
      return last_operation_token = END;
    }
  } while (ch != '\n' && ch == ' ');

  switch (ch) {
    case 0:                                         // end of file
    case '\n':
      return last_operation_token = END;
    case MUL:
    case PLUS:
    case MINUS:
    case LB:
    case RB:
    case COM:
      return last_operation_token = TokenValue(ch); //  update operation
    case NUM0: 
    case NUM1: 
    case NUM2: 
    case NUM3: 
    case NUM4:
    case NUM5: 
    case NUM6: 
    case NUM7: 
    case NUM8: 
    case NUM9:
      //  read all number with first symbol (first put back it)
      input->putback(ch);
      *input >> number_value;
      return last_operation_token = NUMBER;
    default:
      if (isalpha(ch)) {
        string_value = ch;
        while (input->get(ch) && isalnum(ch)) {
          string_value.push_back(ch);
        }
        input->putback(ch);
        return last_operation_token = NAME;
      }
      return last_operation_token = END;
  }
}

void atom(std::istream* input, bool get) {
  if (get) {
    get_token(input);
  }

  switch (last_operation_token) {
    case NUMBER: {
      int v = number_value;
      get_token(input);
      if ((v >> 12) == 0) {
        result.push_back(mov_r3_value + v);   //  mov r3, #num
      } else {
        result.push_back(ldr_r3_num);       //  mov r3, .num, num declared later
        big_numbers.push_back(std::make_pair(result.size(), v));
      }
      return;
    }
    case NAME: {
      get_token(input);
      // function name
      if (last_operation_token == LB) {
        std::string func_name = string_value;
        int func_args = 0;
        int* v = table[func_name];
        do {
          result.push_back(sub_sp_4);     //  sub sp, sp, #4
          expr(input, true);
          func_args++;
          result.push_back(str_r3_sp);    //  str r3, [sp]
        } while (last_operation_token == COM);
        for (int i = 0; i < func_args; ++i) {
          result.push_back(ldr_r0_sp + (i << 12) + 4*(func_args - 1 - i));
        }
        result.push_back(ldr_r4_func);    //  ldr r4, .func_ptr
        function_ptrs.push_back(std::make_pair(result.size(), func_name));
        result.push_back(blx_r4);         //  blx r4
        result.push_back(mov_r3_r0);      //  mov r3, r0
        result.push_back(add_sp_0 + func_args * 4); // add sp, #4*fargc
        get_token(input);
      } else {
        int v = *table[string_value];
        result.push_back(ldr_r3_num);     //  ldr r3, .var_ptr
        big_numbers.push_back(std::make_pair(result.size(), v));
      }
      return;
    }
    case LB: {
      expr(input, true);
      get_token(input);
      return;
    }
    case MINUS: {
      atom(input, true);
      result.push_back(mov_r2_neg1);   //  mov r2, #-1
      result.push_back(eor_r3_r2);   //  eor r3, r2
      result.push_back(add_r3_1);   //  add r3, r3, #1
      return;
    }
    default:
      return;
  }
}

void term(std::istream* input, bool get) {
  result.push_back(sub_sp_4);   //  sub sp, sp, #4
  atom(input, get);
  result.push_back(str_r3_sp);   //  str r3, [sp]

  while (true) {
    switch (last_operation_token) {
      case MUL:
        atom(input, true);
        result.push_back(ldr_r2_sp);   //  ldr r2, [sp]
        result.push_back(mul_r3_r2);   //  mul r3, r3, r2
        result.push_back(str_r3_sp);   //  str r3, [sp]
        break;
      default:
        result.push_back(ldr_r3_sp);   //  ldr r3, [sp]
        result.push_back(add_sp_4);   //  add sp, sp, #4
        return;
    }
  }
}

void expr(std::istream* input, bool get) {
  result.push_back(sub_sp_4);   //  sub sp, sp, #4
  term(input, get);
  result.push_back(str_r3_sp);   //  str r3, [sp]

  while (true) {
    switch (last_operation_token) {
      case PLUS:
        term(input, true);
        result.push_back(ldr_r2_sp);   //  ldr r2, [sp]
        result.push_back(add_r3_r2);   //  add r3, r3, r2
        result.push_back(str_r3_sp);   //  str r3, [sp]
        break;
      case MINUS:
        term(input, true);
        result.push_back(ldr_r2_sp);   //  ldr r2, [sp]
        result.push_back(sub_r2_r3);   //  sub r3, r2, r3
        result.push_back(str_r3_sp);   //  str r3, [sp]
        break;
      default:
        result.push_back(ldr_r3_sp);   //  ldr r3, [sp]
        result.push_back(add_sp_4);   //  add sp, sp, #4
        return;
    }
  }
}

extern "C" void
jit_compile_expression_to_arm(const char* expression, const symbol_t* externs, void* out_buffer) {
  result.push_back(push_r4);   //  push {r4}, same as str r4, [sp, #-4]!
  result.push_back(sub_sp_4);   //  sub sp, sp, #4
  const symbol_t* data = externs;
  int it = 0;
  while (data[it].name != NULL) {
    int *ptr = (int*)data[it].pointer;
    table[data[it].name] = ptr;
    ++it;
  }
  std::string s (expression);
  std::istringstream istr(s);
  std::istream* input = &istr; // stream pointer
  while (*input) {
    get_token(input);
    if (last_operation_token == END) {
      break;
    }
    expr(input, false);
  }

  result.push_back(mov_r0_r3);     //  mov r0, r3
  result.push_back(add_sp_4);     //  add sp, sp, #4
  result.push_back(pop_lr);     //  pop  {lr}, same as ldr [sp], #4
  result.push_back(bx_lr);     //  bx  lr

  //  execute functions
  for (int i = 0; i < function_ptrs.size(); ++i) {
    result.push_back((int)table[function_ptrs[i].second]); //  function ptr
    result[function_ptrs[i].first - 1] += (result.size() - function_ptrs[i].first - 2)*4;
  }

  //  set big numbers
  for (int i = 0; i < big_numbers.size(); i++) {
    result.push_back(big_numbers[i].second);    //  number
    result[big_numbers[i].first - 1] += (result.size() - big_numbers[i].first - 2)*4;   //  address offset
  }

  uint *res_buff = (uint*)out_buffer;
  for (int i = 0; i < result.size(); ++i) {
    res_buff[i] = result[i];
  }
}