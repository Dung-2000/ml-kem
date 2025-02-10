MYTEST_BUILD_DIR := $(BUILD_DIR)/mytest

MYTEST_DIR := mytest
MYTEST_SOURCES := $(wildcard $(MYTEST_DIR)/*.cpp)
MYTEST_HEADERS := $(wildcard $(MYTEST_DIR)/*.hpp)
MYTEST_EXECS := $(addprefix $(MYTEST_BUILD_DIR)/, $(notdir $(MYTEST_SOURCES:.cpp=.exe)))

MYTEST_SOURCES := $(wildcard $(MYTEST_DIR)/*.cpp)
MYDEBUG_BUILD_DIR := $(MYTEST_BUILD_DIR)/debug
MYTEST_BINARY := $(MYTEST_BUILD_DIR)/mytest.out
MYTEST_OBJECTS := $(addprefix $(MYTEST_BUILD_DIR)/, $(notdir $(patsubst %.cpp,%.o,$(MYTEST_SOURCES))))

$(MYTEST_BUILD_DIR):
	mkdir -p $@

$(MYTEST_BUILD_DIR)/%.exe: $(MYTEST_DIR)/%.cpp $(MYTEST_BUILD_DIR)
	$(CXX) $(CXX_DEFS) $(CXX_FLAGS) $(WARN_FLAGS) $(DEBUG_FLAGS) $(I_FLAGS) $(DEP_IFLAGS) $< -o $@

mytest: $(MYTEST_EXECS) ## Build and run mytest program, using the ml-ken as API lib
	$(foreach exec,$^,./$(exec);)

$(MYDEBUG_BUILD_DIR):
	mkdir -p $@

$(MYDEBUG_BUILD_DIR)/%.o: $(MYTEST_DIR)/%.cpp $(MYDEBUG_BUILD_DIR) $(SHA3_INC_DIR) $(ASCON_INC_DIR) $(SUBTLE_INC_DIR)
	$(CXX) $(CXX_FLAGS) $(WARN_FLAGS) $(DEBUG_FLAGS) $(I_FLAGS) $(DEP_IFLAGS) -c $< -o $@

$(MYTEST_BINARY): $(MYTEST_OBJECTS)
	$(CXX) $(DEBUG_FLAGS) $^ $(LINK_OPT_FLAGS) -o $@
