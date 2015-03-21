# Define executable name
 
BIN = QuantServer

############### List All the .cpp files here....################
SRCS = main.cpp FillMsgStructs.cpp NQTV.cpp NQTVDlg.cpp Util.cpp ReceiveITCH.cpp DBLayer.cpp
############### List all the includes paths here....############
#INCLUDES = -I/home/amro/projects/QuantServer  -I../Include   -I../Common 
############### List all header file paths here...##############
# Define header file paths
#INCPATH = -I./
INCPATH = -I/usr/include/mysql -I/home/amro/projects/QuantServer  -I./Include   -I./Common   -I./
# Define the -L library path(s)
LDFLAGS = -lrt -L/usr/lib64/mysql -lmysqlclient

###################### Define the -l library name(s)  ##########
LIBS =
#
#
#
#################################################################################################################
#			Only in special cases should anything be edited below this line 		     	#	
#														#
#################################################################################################################
#OBJS      = $(CPP_SRCS:.cpp=.o)
OBJS      = $(SRCS:.cpp=.o)
CXXFLAGS  = -Wall -ansi -pedantic -g
DEP_FILE  = .depend

.PHONY = all clean distclean

# Main entry point
#
all: depend $(BIN)


# For linking object file(s) to produce the executable
#
$(BIN): $(OBJS)
	@echo Linking $@
	@$(CXX) $^ $(LDFLAGS) $(LIBS) -o $@


# For compiling source file(s)
#
.cpp.o:
	@echo Compiling $<
	@$(CXX) -c $(CXXFLAGS) $(INCPATH) $<


# For cleaning up the project
#
clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(BIN)
	$(RM) $(DEP_FILE)

# For determining source file dependencies
#
depend: $(DEP_FILE)
	@touch $(DEP_FILE)

$(DEP_FILE):
	@echo Generating dependencies in $@
	@-$(CXX) -E -MM $(CXXFLAGS) $(INCPATH) $(SRCS) >> $(DEP_FILE)

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
ifeq (,$(findstring distclean,$(MAKECMDGOALS)))
-include $(DEP_FILE)
endif
endif