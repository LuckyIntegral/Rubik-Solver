###############################################################################
######                            PROPERTIES                             ######
###############################################################################

CXX			= g++
RM			= rm -rf
OPTFLAGS	= -O3 -DNDEBUG -march=native
CXXFLAGS	= -Wall -Wextra -Werror -MD -MP $(OPTFLAGS)
MAKEFLAGS	= -j$(nproc) --no-print-directory
INCLUDES	= -I incs/

NAME		= rubik

VENV		= venv
VENV_PY		= $(VENV)/bin/python3
VENV_PIP	= $(VENV)/bin/pip

SRCS_DIR	= srcs
SRCS		= $(filter-out $(SRCS_DIR)/main_test.cpp $(SRCS_DIR)/main_test_performance.cpp,$(wildcard $(SRCS_DIR)/*.cpp))

OBJS_DIR	= objs
OBJS		= $(addprefix $(OBJS_DIR)/, $(SRCS:${SRCS_DIR}/%.cpp=%.o))
DEPS		= $(addprefix $(OBJS_DIR)/, $(SRCS:${SRCS_DIR}/%.cpp=%.d))

###############################################################################
######                               RULES                               ######
###############################################################################

all		: $(NAME)

$(NAME)	: ${OBJS}
		$(CXX) -o $@ $^

${OBJS_DIR}/%.o	: ${SRCS_DIR}/%.cpp
		@mkdir -p $(dir $@)
		${CXX} ${CXXFLAGS} -c $< -o $@ ${INCLUDES}

clean	:
		$(RM) $(OBJS_DIR)

fclean	:
		$(RM) $(OBJS_DIR) $(NAME) test test.d test_performance

re		:
		$(RM) $(OBJS_DIR) $(NAME)
		$(MAKE) all

run		:
		$(MAKE) re
		./$(NAME)

# Python visualizer: creates venv + installs deps when needed, then runs
$(VENV)/bin/python3:
		python3 -m venv $(VENV)

$(VENV)/.deps_installed: requirements.txt $(VENV)/bin/python3
		$(VENV_PIP) install -r requirements.txt
		@touch $(VENV)/.deps_installed

v		: $(VENV)/.deps_installed
		$(VENV_PY) visualizer/main.py

clean-venv	:
		$(RM) $(VENV)

test		:
		$(CXX) $(CXXFLAGS) $(INCLUDES) -o test \
			srcs/main_test.cpp srcs/thistlethwaite.cpp srcs/move.cpp srcs/prune.cpp srcs/solver.cpp srcs/heuristics.cpp srcs/encode_tables.cpp

test_performance	:
		$(CXX) $(CXXFLAGS) $(INCLUDES) -o test_performance \
			srcs/main_test_performance.cpp srcs/thistlethwaite.cpp srcs/move.cpp srcs/prune.cpp srcs/solver.cpp srcs/heuristics.cpp srcs/encode_tables.cpp

-include $(DEPS)

.PHONY: all clean fclean re run v clean-venv test test_performance