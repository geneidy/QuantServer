# Define executable name
 
BIN = bin/QuantServer

############### List All the .cpp files here....################
##### All Header files are included in the .cpp and compiled accordingly #######

SRCS = 	main.cpp \
	FillMsgStructs.cpp \
	NQTV.cpp \
	NQTVDlg.cpp \
	Util.cpp \
	ReceiveITCH.cpp \
	Logger.cpp \
	ReceiveFeed.cpp \
	ParseFeed.cpp \
        BuildBookLL.cpp\
	SaveToDB.cpp \
	SaveToDisk.cpp \
	PlayBack.cpp \
	NasdTestFile.cpp \
	Distributor.cpp\
	QuantQueue.cpp \
	OrdersMap.cpp \
	TickDataMap.cpp\
	QSettings.cpp\
	StatsPerSec.cpp\
	SaveOrdersToDisc.cpp

############### List all the includes paths here....############
#INCLUDES = -I/home/amro/projects/QuantServer  -I../Include   -I../Common 
############### List all header file paths here...##############
# Define header file paths
INCPATH = -I./Common -I./Include
# Define the -L library path(s)
LDFLAGS = -lrt 
# LDFLAGS = -lrt 
###################### Define the -l library name(s)  ##########
LIBS = -lpthread
#
#
#
#################################################################################################################
#			Only in special cases should anything be edited below this line 		     	#	
#														#
#################################################################################################################

OBJS      = $(SRCS:.cpp=.o)
CXXFLAGS  = -Wall -ansi -pedantic -g -std=c++11
#CXXFLAGS  = -Wall -ansi -pedantic -std=c++11
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
	@$(CXX) -c $(CXXFLAGS) $(INCPATH) "$<" -o "$@"


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
