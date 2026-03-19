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
		$(RM) $(OBJS_DIR) $(NAME) test test.d

re		:
		$(RM) $(OBJS_DIR) $(NAME)
		$(MAKE) all

run		:
		$(MAKE) re
		./$(NAME)

test		:
		$(CXX) $(CXXFLAGS) $(INCLUDES) -o test \
			srcs/main_phase_test.cpp srcs/thistlethwaite.cpp srcs/move.cpp srcs/prune.cpp srcs/phase_solver.cpp srcs/heuristics.cpp
		./test

-include $(DEPS)

.PHONY: all clean fclean re run test