# Mistake 1
Created this test
``` Bash
run_test "Direct String Plus Quantifier" "go+l" "goooal" "goooal"
```
When asked was corrected to 
``` Bash
run_test "Direct String Plus Quantifier" "go+al" "goooal" "goooal"
```

# Mistake 2
Asking AI to debug problems it created when trying to add more features and making numerous mistakes to the point where I reset all changes to the last commit as it was not working at all

# Mistake 3
Several spelling mistakes with American vs Australian spellings of some functions and variables

# Mistake 4
When update /RegexApp.cpp, used target.type and target.value as a way to access the type and value of an argument and they were never made. 
Was changed to target for target.vaule and added logic for target.type