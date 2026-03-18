###############################################################################
######                            PROPERTIES                             ######
###############################################################################

CXX			= g++
RM			= rm -rf
CXXFLAGS	= -Wall -Wextra -Werror -MD -MP -g
MAKEFLAGS	= -j$(nproc) --no-print-directory
INCLUDES	= -I incs/

NAME		= rubik

SRCS_DIR	= srcs
SRCS		= $(filter-out $(SRCS_DIR)/main_%_test.cpp,$(wildcard $(SRCS_DIR)/*.cpp))

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
		$(RM) $(OBJS_DIR) $(NAME)

re		:
		$(RM) $(OBJS_DIR) $(NAME)
		$(MAKE) all

run		:
		$(MAKE) re
		./$(NAME)

test_move	:
		$(CXX) $(CXXFLAGS) $(INCLUDES) -o test_move \
			srcs/main_move_engine_test.cpp srcs/move.cpp

test_prune	:
		$(CXX) $(CXXFLAGS) $(INCLUDES) -o test_prune \
			srcs/main_prune_test.cpp srcs/thistlethwaite.cpp srcs/move.cpp srcs/prune.cpp

test_phase1	:
		$(CXX) $(CXXFLAGS) $(INCLUDES) -o test_phase1 \
			srcs/main_phase1_test.cpp srcs/thistlethwaite.cpp srcs/move.cpp srcs/prune.cpp srcs/phase_solver.cpp

v		:
		$(MAKE) visualizer

visualizer	:
		node --watch --watch-path=server.js --watch-path=visualizer server.js

-include $(DEPS)

.PHONY: all clean fclean bonus re run visualizer v test_move test_prune test_phase1