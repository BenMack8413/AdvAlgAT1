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