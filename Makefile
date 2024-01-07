SHELL := /bin/bash
.DEFAULT_GOAL := help
.PHONY: help build test clean debug

f_debug = build_debug
app_targets = PyClassifiers
test_targets = unit_tests_pyclassifiers
n_procs = -j 16

define ClearTests
	@for t in $(test_targets); do \
		if [ -f $(f_debug)/tests/$$t ]; then \
			echo ">>> Cleaning $$t..." ; \
			rm -f $(f_debug)/tests/$$t ; \
		fi ; \
	done
	@nfiles="$(find . -name "*.gcda" -print0)" ; \
	if test "${nfiles}" != "" ; then \
		find . -name "*.gcda" -print0 | xargs -0 rm 2>/dev/null ;\
	fi ; 
endef


clean: ## Clean the tests info
	@echo ">>> Cleaning Debug PyClassifiers tests...";
	$(call ClearTests)
	@echo ">>> Done";

build: ## Build a version of the project
	@echo ">>> Building Debug PyClassifiers...";
	@if [ -d ./$(f_debug) ]; then rm -rf ./$(f_debug); fi
	@mkdir $(f_debug); 
	@cmake -S . -B $(f_debug) -D CMAKE_BUILD_TYPE=Debug -D ENABLE_TESTING=ON -D CODE_COVERAGE=ON
	@cmake --build $(f_debug) $(n_procs)
	@echo ">>> Done";	

opt = ""
test: ## Run tests (opt="-s") to verbose output the tests, (opt="-c='Test Maximum Spanning Tree'") to run only that section
	@echo ">>> Running PyClassifiers & Platform tests...";
	@$(MAKE) clean
	@cmake --build $(f_debug) -t $(test_targets) $(n_procs)
	@for t in $(test_targets); do \
		if [ -f $(f_debug)/tests/$$t ]; then \
			cd $(f_debug)/tests ; \
			./$$t $(opt) ; \
		fi ; \
	done
	@echo ">>> Done";

coverage: ## Run tests and generate coverage report (build/index.html)
	@echo ">>> Building tests with coverage...";
	@$(MAKE) test
	@cd $(f_debug) ; \
	gcovr --config ../gcovr.cfg tests ;
	@echo ">>> Done";	


help: ## Show help message
	@IFS=$$'\n' ; \
	help_lines=(`fgrep -h "##" $(MAKEFILE_LIST) | fgrep -v fgrep | sed -e 's/\\$$//' | sed -e 's/##/:/'`); \
	printf "%s\n\n" "Usage: make [task]"; \
	printf "%-20s %s\n" "task" "help" ; \
	printf "%-20s %s\n" "------" "----" ; \
	for help_line in $${help_lines[@]}; do \
		IFS=$$':' ; \
		help_split=($$help_line) ; \
		help_command=`echo $${help_split[0]} | sed -e 's/^ *//' -e 's/ *$$//'` ; \
		help_info=`echo $${help_split[2]} | sed -e 's/^ *//' -e 's/ *$$//'` ; \
		printf '\033[36m'; \
		printf "%-20s %s" $$help_command ; \
		printf '\033[0m'; \
		printf "%s\n" $$help_info; \
	done
