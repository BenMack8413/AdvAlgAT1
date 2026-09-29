#!/bin/bash
mkdir -p tests

# 1. Standard Single Line Test File
cat << 'EOF' > tests/test_single_line.txt
The quick brown fox jumps over 123 lazy dogs.
EOF

# 2. Standard Multi-Line Test File
cat << 'EOF' > tests/test_multi_line.txt
Header line: system initialization
ERROR: Database connection failed on port 5432
INFO: Retrying connection...
ERROR: Timeout reached after 3000ms
Footer line: process terminated
EOF

# 3. Empty Test File
touch tests/test_empty.txt

# 4. Structured Log File
cat << 'EOF' > tests/test_logs.txt
2026-09-29 10:00:01 [INFO] User logged in from 192.168.1.50
2026-09-29 10:02:15 [WARN] High memory usage: 88%
2026-09-29 10:05:30 [INFO] User logged out from 10.0.0.1
EOF

# 5. Long Line Stress Test File (10,000 'a's + target + 10,000 'b's)
LONG_PREFIX=$(printf 'a%.0s' {1..10000})
LONG_SUFFIX=$(printf 'b%.0s' {1..10000})
echo "${LONG_PREFIX}STRESS_TARGET_MATCH${LONG_SUFFIX}" > tests/test_long_line.txt

# 6. Whitespace & Blank Lines File
cat << 'EOF' > tests/test_whitespace.txt


   
	line with leading tab
trailing space   

EOF

# 7. Symbol-Heavy Configuration File
cat << 'EOF' > tests/test_config.ini
[database]
server_host=127.0.0.1:8080
db_pass="P@ssw0rd!#123"
enabled=true # main flag
EOF

# 8. Multiple Matches / Overlaps File
cat << 'EOF' > tests/test_multiple_matches.txt
cat dog cat bird cat
EOF

# 9. Missing Newline at EOF File
printf "first line\nsecond line without newline" > tests/test_no_eof_newline.txt