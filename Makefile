f_release = build_release
f_debug = build_debug
app_targets = PyClassifiers
test_targets = unit_tests_pyclassifiers 

# Set the number of parallel jobs to the number of available processors minus 7
CPUS := $(shell getconf _NPROCESSORS_ONLN 2>/dev/null \
                 || nproc --all 2>/dev/null \
                 || sysctl -n hw.ncpu)

# --- Your desired job count: CPUs – 7, but never less than 1 --------------
JOBS := $(shell n=$(CPUS); [ $${n} -gt 7 ] && echo $$((n-7)) || echo 1)

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

define build_target
	@echo ">>> Building the project for $(1)..."
	@if [ -d $(2) ]; then rm -fr $(2); fi
	@conan install . --build=missing -of $(2) -s build_type=$(1)
	@cmake -S . -B $(2) -DCMAKE_TOOLCHAIN_FILE=$(2)/build/$(1)/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=$(1) -D$(3)
	@echo ">>> Will build using $(JOBS) parallel jobs"
	echo ">>> Done"
endef

define compile_target
	@echo ">>> Compiling for $(1)..."
		if [ "$(3)" != "" ]; then \
		target="-t$(3)"; \
	else \
		target=""; \
	fi
	@cmake --build $(2) --config $(1) --parallel $(JOBS) $(target)
	@echo ">>> Done"
endef

setup: ## Install dependencies for tests and coverage
	@if [ "$(shell uname)" = "Darwin" ]; then \
		brew install gcovr; \
		brew install lcov; \
	fi
	@if [ "$(shell uname)" = "Linux" ]; then \
		pip install gcovr; \
	fi

dependency: ## Create a dependency graph diagram of the project (build/dependency.png)
	@echo ">>> Creating dependency graph diagram of the project...";
	$(MAKE) debug
	cd $(f_debug) && cmake .. --graphviz=dependency.dot && dot -Tpng dependency.dot -o dependency.png

buildd: ## Build the debug targets
	@$(call compile_target,"Debug","$(f_debug)")

buildr: ## Build the release targets
	@$(call compile_target,"Release","$(f_release)")

clean: ## Clean the tests info
	@echo ">>> Cleaning Debug PyClassifiers tests...";
	$(call ClearTests)
	@echo ">>> Done";

prefix = "/usr/local"
install: ## Install library
	@echo ">>> Installing PyClassifiers...";
	@cmake --install $(f_release) --prefix $(prefix)
	@echo ">>> Done";

debug: ## Build a debug version of the project with Conan
	@$(call build_target,"Debug","$(f_debug)", "ENABLE_TESTING=ON")

release: ## Build a Release version of the project with Conan
	@$(call build_target,"Release","$(f_release)", "ENABLE_TESTING=OFF")

opt = ""
test: ## Run tests (opt="-s") to verbose output the tests, (opt="-c='Test Maximum Spanning Tree'") to run only that section
	@echo ">>> Running PyClassifiers tests...";
	@$(MAKE) clean
	@cmake --build $(f_debug) -t $(test_targets) --parallel
	@for t in $(test_targets); do \
		if [ -f $(f_debug)/tests/$$t ]; then \
			cd $(f_debug)/tests ; \
			./$$t $(opt) ; \
		fi ; \
	done
	@echo ">>> Done";

coverage: ## Run tests and generate coverage report (build/index.html)
	@echo ">>> Building tests with coverage..."
	@$(MAKE) test
	@gcovr $(f_debug)/tests
	@echo ">>> Done";	

# Conan package manager targets
# =============================

conan-create: ## Create Conan package
	@echo ">>> Creating Conan package..."
	@echo ">>> Creating Release build..."
	@conan create . --build=missing -tf "" -s:a build_type=Release
	@echo ">>> Creating Debug build..."
	@conan create . --build=missing -tf "" -s:a build_type=Debug -o "&:enable_testing=False"
	@echo ">>> Done"

conan-clean: ## Clean Conan cache and build folders
	@echo ">>> Cleaning Conan cache and build folders..."
	@conan remove "*" --confirm
	@conan cache clean
	@if test -d "$(f_release)" ; then rm -rf "$(f_release)"; fi
	@if test -d "$(f_debug)" ; then rm -rf "$(f_debug)"; fi
	@echo ">>> Done"

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
