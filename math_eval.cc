

#include <cctype>
#include <string>
#include <iostream>
#include <stack>
#include <unordered_map>
using namespace std;

unordered_map<char, int> op_pri = {
  {'+',1},
  {'-', 1},
  {'*',2}
};

int cal_one(stack<char>& ops, stack<int>& nums) {
  int b = nums.top(); 
  nums.pop();
  int a = nums.top();
  nums.pop();

  char c  = ops.top();
  ops.pop();
  int num = 0;
  if(c == '+') {
    num = a + b;

  } else if(c == '-') {
    num = a - b;
  } else if(c == '*') {
    num = a * b;
  }

  return num;
}
int solve(std::string s) {
  stack<char> ops;
  stack<int> nums;

  nums.push(0);
  for(int i=0; i < s.length(); i++) {
    char c= s[i];

    if(c == '(') {
      ops.push(c);
    } else if(c == ')') {
      while(1) {
        if(ops.top() != '(') {
          int res = cal_one(ops, nums);
          nums.push(res);
        } else {
          ops.pop();
          break;
        }
      }
    } else {
      // digit
      if(isdigit(c)) {
        int j = i;
        int cur_num = 0;
        while(j < s.length() && isdigit(s[j])) {
          cur_num = cur_num * 10 + s[j]-'0';
          j++;
        }
        i = j-1;
        nums.push(cur_num);
      } else {

        // + - *
        if(i > 0 && (s[i-1] == '(' || s[i-1] == '-' || s[i-1]=='+')) {
          nums.push(0);
        }
        // This ops.top() != '(' is important. (expr) is highest priority
        while(!ops.empty() && ops.top() != '(') {
          if(op_pri[ops.top()] >= op_pri[c]  ) {
            int res = cal_one(ops, nums);
            nums.push(res);
          } else {
            break;
          }
        }
        ops.push(c);
      }

    }
  }
  while(!ops.empty()) {
    int res = cal_one(ops, nums);
    nums.push(res);
  }

  return nums.top();
  return 0;

}
int main(int argc, char* argv[]) {


  string s(argv[1]);


  cout << s << endl;

  cout << solve(s) << endl;

  return 0;


}
