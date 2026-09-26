Created this test
``` Bash
run_test "Direct String Plus Quantifier" "go+l" "goooal" "goooal"
```
When asked was corrected to 
``` Bash
run_test "Direct String Plus Quantifier" "go+al" "goooal" "goooal"
```