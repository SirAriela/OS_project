# ---------- Build Configuration ----------
SHELL := /bin/bash
CXX      ?= g++
CXXFLAGS ?= -Wall -Wextra -Wpedantic -std=c++17 -g -O0

# Target definitions
SERVER_TARGET := lf_server
CLIENT_TARGET := tcp_client
TEST_TARGET   := test_algorithms
BINARIES      := $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_TARGET)

# Source file definitions
SERVER_SOURCES := lf_server.cpp graph.cpp point.cpp graph_algorithms.cpp
CLIENT_SOURCES := client.cpp
TEST_SOURCES   := test_algorithms.cpp graph.cpp point.cpp graph_algorithms.cpp

# ---------- Build Rules ----------
all: $(BINARIES)

$(SERVER_TARGET): $(SERVER_SOURCES:.cpp=.o)
	$(CXX) $^ -o $@ -pthread

$(CLIENT_TARGET): $(CLIENT_SOURCES:.cpp=.o)
	$(CXX) $^ -o $@

$(TEST_TARGET): $(TEST_SOURCES:.cpp=.o)
	$(CXX) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BINARIES) valgrind.*.log *.log *.gcov *.gcda *.gcno

# ---------- Full Coverage Testing ----------
# Coverage flags
COVERAGE_CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -g -O0 --coverage

# Full coverage test including server functionality
coverage: coverage-clean coverage-build coverage-run coverage-report

coverage-clean:
	@echo "Cleaning previous build and coverage files..."
	rm -f *.o $(BINARIES) *.gcov *.gcda *.gcno

coverage-build:
	@echo "Building with coverage flags..."
	g++ $(COVERAGE_CXXFLAGS) -c graph.cpp -o graph.o
	g++ $(COVERAGE_CXXFLAGS) -c point.cpp -o point.o
	g++ $(COVERAGE_CXXFLAGS) -c graph_algorithms.cpp -o graph_algorithms.o
	g++ $(COVERAGE_CXXFLAGS) -c test_algorithms.cpp -o test_algorithms.o
	g++ $(COVERAGE_CXXFLAGS) -c lf_server.cpp -o lf_server.o
	g++ $(COVERAGE_CXXFLAGS) -c client.cpp -o client.o
	g++ $(COVERAGE_CXXFLAGS) graph.o point.o graph_algorithms.o test_algorithms.o -o test_algorithms
	g++ $(COVERAGE_CXXFLAGS) lf_server.o graph.o point.o graph_algorithms.o -o lf_server -pthread
	g++ $(COVERAGE_CXXFLAGS) client.o -o tcp_client
	chmod +x coverage_test.sh 
	
coverage-run:
	@echo "Running algorithm tests to generate coverage data..."
	./test_algorithms
	@echo "Testing server functionality for coverage..."
	@echo "Starting server on port 8080..."
	@./lf_server 8080 & srv=$$!; \
	sleep 3; \
	echo "Sending test requests to generate server coverage..."; \
	echo "-e 6 -v 5 -s 15" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "-e 8 -v 6 -s 20" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "-e 10 -v 7 -s 25" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "-e 12 -v 8 -s 30" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "-e 4 -v 3 -s 10" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "-e 15 -v 10 -s 50" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "Testing invalid requests for error handling coverage..."; \
	echo "invalid_request" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "-e -5 -v 3 -s 10" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "-e 5 -v 10 -s 10" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "Testing edge cases for better coverage..."; \
	echo "-e 1 -v 2 -s 1" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "-e 20 -v 5 -s 100" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "-e 0 -v 3 -s 5" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "-e 5 -v 0 -s 5" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "-e 5 -v 3 -s -5" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "-e 5 -v 3 -s 5 -extra_param" | ./tcp_client 127.0.0.1 8080; \
	sleep 1; \
	echo "Testing concurrent requests for thread coverage..."; \
	echo "-e 6 -v 4 -s 10" | ./tcp_client 127.0.0.1 8080 & \
	echo "-e 7 -v 5 -s 15" | ./tcp_client 127.0.0.1 8080 & \
	echo "-e 8 -v 6 -s 20" | ./tcp_client 127.0.0.1 8080 & \
	sleep 3; \
	echo "Stopping server..."; \
	kill -INT $$srv 2>/dev/null || true; \
	wait $$srv 2>/dev/null || true; \
	echo "Server tests completed!"

coverage-report:
	@echo "Generating coverage reports for YOUR source files only..."
	@echo "========================================"
	@for src_file in graph.cpp point.cpp graph_algorithms.cpp lf_server.cpp client.cpp; do \
		if [ -f "$$src_file" ]; then \
			echo "Processing coverage for $$src_file "; \
			gcov -b -c "$$src_file" >/dev/null 2>&1; \
			if [ -f "$${src_file}.gcov" ]; then \
				echo "Coverage for $$src_file:"; \
				echo "----------------------------------------"; \
				total_lines=0; \
				covered_lines=0; \
				while IFS=: read -r count _ rest; do \
					count="$${count#"$${count%%[![:space:]]*}"}"; \
					if [ -n "$$count" ] && [ "$$count" != "-" ]; then \
						if [ "$$count" = "#####" ]; then \
							total_lines=$$((total_lines + 1)); \
						elif echo "$$count" | grep -q '^[0-9]\+$$'; then \
							total_lines=$$((total_lines + 1)); \
							if [ "$$count" -gt 0 ]; then \
								covered_lines=$$((covered_lines + 1)); \
							fi; \
						fi; \
					fi; \
				done < "$${src_file}.gcov"; \
				if [ "$$total_lines" -gt 0 ]; then \
					pct=$$(( (covered_lines*100 + total_lines/2) / total_lines )); \
					echo "Lines executed: $$pct% ($$covered_lines/$$total_lines)"; \
					echo "File: $${src_file}.gcov created"; \
				else \
					echo "No executable lines found"; \
				fi; \
				echo; \
			else \
				echo "No coverage data generated for $$src_file"; \
			fi; \
		fi; \
	done
	@echo "========================================"
	@echo "FULL COVERAGE TEST COMPLETE"
	@echo "========================================"
	@echo "Coverage files created for YOUR source files only:"
	@for src_file in graph.cpp point.cpp graph_algorithms.cpp lf_server.cpp client.cpp; do \
		if [ -f "$${src_file}.gcov" ]; then \
			echo "  ✓ $${src_file}.gcov"; \
		fi; \
	done
	@echo
	@echo "External library coverage files have been filtered out."
	@echo "Only YOUR source code coverage is shown above."
	@echo
	@echo "Coverage summary:"
	@echo "  - graph.cpp.gcov - Graph implementation coverage"
	@echo "  - point.cpp.gcov - Point class coverage"
	@echo "  - graph_algorithms.cpp.gcov - Algorithm implementations coverage"
	@echo "  - lf_server.cpp.gcov - Server implementation coverage"
	@echo "  - client.cpp.gcov - Client implementation coverage"

# Convenience targets
server: $(SERVER_TARGET)
client: $(CLIENT_TARGET)
test:   $(TEST_TARGET)

# ---------- Valgrind Configuration ----------
VG_MEMCHECK_FLAGS ?= --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose
VG_HELGRIND_FLAGS ?= --tool=helgrind --verbose
VG_CACHEGRIND_FLAGS ?= --tool=cachegrind --verbose

# Test configuration
HOST     ?= 127.0.0.1
PORT     ?= 8080
REQUEST  ?= -e 8 -v 6 -s 42

# ---------- Valgrind Analysis ----------
# Memory check analysis
valgrind-memcheck: $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "Starting memcheck analysis on $(HOST):$(PORT)"
	@valgrind $(VG_MEMCHECK_FLAGS) --log-file=valgrind.memcheck.log ./$(SERVER_TARGET) $(PORT) & srv=$$!; \
	sleep 2; \
	echo "Sending test request: $(REQUEST)"; \
	(echo "$(REQUEST)" | ./$(CLIENT_TARGET) $(HOST) $(PORT)) || true; \
	sleep 1; \
	if kill -0 $$srv 2>/dev/null; then \
		echo "Terminating server"; \
		kill -INT $$srv; \
		wait $$srv 2>/dev/null || true; \
	else \
		echo "Server already terminated"; \
	fi; \
	echo "Memcheck analysis complete. Log: valgrind.memcheck.log"

# Thread analysis for race conditions and deadlocks
valgrind-helgrind: $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "Starting helgrind analysis on $(HOST):$(PORT)"
	@valgrind $(VG_HELGRIND_FLAGS) --log-file=valgrind.helgrind.log ./$(SERVER_TARGET) $(PORT) & srv=$$!; \
	sleep 2; \
	echo "Sending test request: $(REQUEST)"; \
	(echo "$(REQUEST)" | ./$(CLIENT_TARGET) $(HOST) $(PORT)) || true; \
	sleep 1; \
	if kill -0 $$srv 2>/dev/null; then \
		echo "Terminating server"; \
		kill -INT $$srv; \
		wait $$srv 2>/dev/null || true; \
	else \
		echo "Server already terminated"; \
	fi; \
	echo "Helgrind analysis complete. Log: valgrind.helgrind.log"

# Cache performance analysis
valgrind-cachegrind: $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "Starting cachegrind analysis on $(HOST):$(PORT)"
	@valgrind $(VG_CACHEGRIND_FLAGS) --log-file=valgrind.cachegrind.log ./$(SERVER_TARGET) $(PORT) & srv=$$!; \
	sleep 2; \
	echo "Sending test request: $(REQUEST)"; \
	(echo "$(REQUEST)" | ./$(CLIENT_TARGET) $(HOST) $(PORT)) || true; \
	sleep 1; \
	if kill -0 $$srv 2>/dev/null; then \
		echo "Terminating server"; \
		kill -INT $$srv; \
		wait $$srv 2>/dev/null || true; \
	else \
		echo "Server already terminated"; \
	fi; \
	echo "Cachegrind analysis complete. Log: valgrind.cachegrind.log"

# Complete section 10 analysis - all three required tools
valgrind-section10: valgrind-memcheck valgrind-helgrind valgrind-cachegrind
	@echo "=== SECTION 10 VALGRIND ANALYSIS COMPLETE ==="
	@echo "Generated logs:"
	@echo "  - valgrind.memcheck.log (memory analysis)"
	@echo "  - valgrind.helgrind.log (thread analysis)" 
	@echo "  - valgrind.cachegrind.log (cache analysis)"
	@echo "=== ALL THREE ANALYSES REQUIRED FOR SECTION 10 ==="

# Run all valgrind analyses (alias)
valgrind-all: valgrind-section10

# Default valgrind target - now covers complete section 10
valgrind: valgrind-section10

# Convenience targets
valgrind-server: $(SERVER_TARGET)
	valgrind $(VG_MEMCHECK_FLAGS) --log-file=valgrind.server.log ./$(SERVER_TARGET) $(PORT)

# Run test suite under valgrind
valgrind-test: $(TEST_TARGET)
	valgrind $(VG_MEMCHECK_FLAGS) --log-file=valgrind.test.log ./$(TEST_TARGET)
	@echo "Test analysis complete. Log: valgrind.test.log"

# Send request to running server
request: $(CLIENT_TARGET)
	@echo "Sending request: $(REQUEST) to $(HOST):$(PORT)"
	@(echo "$(REQUEST)" | ./$(CLIENT_TARGET) $(HOST) $(PORT)) || echo "Connection failed"

# ---------- Multi-client Testing ----------
# Test with multiple concurrent clients
test-concurrent: $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "Starting server for concurrent test"
	@./$(SERVER_TARGET) $(PORT) & srv=$$!; \
	sleep 2; \
	echo "Sending 3 concurrent requests"; \
	(echo "-e 5 -v 4 -s 10" | ./$(CLIENT_TARGET) $(HOST) $(PORT)) & \
	(echo "-e 8 -v 6 -s 20" | ./$(CLIENT_TARGET) $(HOST) $(PORT)) & \
	(echo "-e 12 -v 8 -s 30" | ./$(CLIENT_TARGET) $(HOST) $(PORT)) & \
	wait; \
	sleep 1; \
	if kill -0 $$srv 2>/dev/null; then \
		kill -INT $$srv; \
		wait $$srv 2>/dev/null || true; \
	fi; \
	echo "Concurrent test complete"

# Test concurrent clients under valgrind
valgrind-concurrent: $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "Starting concurrent test under valgrind"
	@valgrind $(VG_MEMCHECK_FLAGS) --log-file=valgrind.concurrent.log ./$(SERVER_TARGET) $(PORT) & srv=$$!; \
	sleep 3; \
	echo "Sending 3 concurrent requests under analysis"; \
	(echo "-e 5 -v 4 -s 10" | ./$(CLIENT_TARGET) $(HOST) $(PORT)) & \
	(echo "-e 8 -v 6 -s 20" | ./$(CLIENT_TARGET) $(HOST) $(PORT)) & \
	(echo "-e 12 -v 8 -s 30" | ./$(CLIENT_TARGET) $(HOST) $(PORT)) & \
	wait; \
	sleep 2; \
	if kill -0 $$srv 2>/dev/null; then \
		kill -INT $$srv; \
		wait $$srv 2>/dev/null || true; \
	fi; \
	echo "Concurrent valgrind analysis complete. Log: valgrind.concurrent.log"

# ---------- Phony Targets ----------
.PHONY: all clean server client test \
        valgrind valgrind-memcheck valgrind-helgrind valgrind-cachegrind valgrind-all \
        valgrind-server valgrind-test valgrind-concurrent \
        request test-concurrent coverage coverage-clean coverage-build coverage-run coverage-report