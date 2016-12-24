# Define executable name
 
BIN = bin/QuantServer

############### List All the .cpp files here....################

SRCS = 	main.cpp \
	FillMsgStructs.cpp \
	NQTV.cpp \
	NQTVDlg.cpp \
	Util.cpp \
	ReceiveITCH.cpp \
	Logger.cpp \
	ReceiveFeed.cpp \
	ParseFeed.cpp \
	BuildBook.cpp \
	SaveToDB.cpp \
	SaveToDisk.cpp \
	PlayBack.cpp \
	NasdTestFile.cpp \
	Distributor.cpp\
	QuantQueue.cpp \
	OrdersMap.cpp \
	TickDataMap.cpp \
	GUI/QtxGui.cpp \
	GUI/dialogs/configdialog.cpp \
	GUI/dialogs/configpages.cpp \
	GUI/dialogs/connDialog.cpp \
	GUI/widgets/perfWidget/perfwidget.cpp \
	GUI/widgets/perfWidget/dataQuery/cpudataquery.cpp \
	GUI/widgets/perfWidget/dataQuery/dataquery.cpp \
	GUI/widgets/perfWidget/dataQuery/queryresource.cpp \
	GUI/widgets/perfWidget/dataQuery/ramdataquery.cpp \
	GUI/widgets/perfWidget/painters/barspainter.cpp \
	GUI/widgets/perfWidget/painters/graphmulticolorpainter.cpp \
	GUI/widgets/perfWidget/painters/graphpainter.cpp \
	GUI/widgets/perfWidget/painters/painterutils.cpp \
	GUI/widgets/perfWidget/painters/performancepainter.cpp \
	GUI/widgets/perfWidget/widgets/cpuwidget.cpp \
	GUI/widgets/perfWidget/widgets/performancewidget.cpp \
	GUI/widgets/perfWidget/widgets/ramwidget.cpp \
	GUI/widgets/digitalclock/digitalclock.cpp \
	GUI/widgets/resourcesWidget/memoryconversion.cpp \
	GUI/widgets/resourcesWidget/resourcesworker.cpp \
	GUI/widgets/resourcesWidget/workerthread.cpp \
	GUI/widgets/resourcesWidget/resourcesWidget.cpp \
	GUI/moc_QtxGui.cpp \
	GUI/moc_connDialog.cpp \
	GUI/moc_configdialog.cpp \
	GUI/moc_perfwidget.cpp \
	GUI/moc_cpuwidget.cpp \
	GUI/moc_performancewidget.cpp \
	GUI/moc_digitalclock.cpp \
	GUI/moc_resourcesworker.cpp \
	GUI/moc_resourcesWidget.cpp \
	GUI/qrc_resources.cpp


############### List all the includes paths here....############
#INCLUDES = -I/home/amro/projects/QuantServer  -I../Include   -I../Common 
############### List all header file paths here...##############
# Define header file paths
INCPATH = -I/usr/include/mysql -I./Common -I./Include -I/usr/lib64/qt4/mkspecs/linux-g++ -I. -I/usr/include/QtCore -I/usr/include/QtGui -I/usr/include -IGUI/ -IGUI/widgets/perfWidget -IGUI/widgets/perfWidget/widgets -IGUI/widgets/perfWidget/dataQuery -IGUI/widgets/perfWidget/painters -IGUI/widgets/digitalclock -IGUI/widgets/resourcesWidget
# Define the -L library path(s)
LDFLAGS = -lrt -L/usr/lib64/mysql -lmysqlclient -lQtCore -lQtGui
# LDFLAGS = -lrt 
###################### Define the -l library name(s)  ##########
LIBS = -lpthread -lprocps
#-lQtCore -lQtGui
#
#
#
#################################################################################################################
#			Only in special cases should anything be edited below this line 		     	#	
#														#
#################################################################################################################

OBJS      = $(SRCS:.cpp=.o)
#CXXFLAGS  = -Wall -ansi -pedantic -ggdb  -std=c++11
CXXFLAGS  = -Wall -ansi -pedantic -g -std=c++11
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

# qt meta object compiler (moc)
# moc%.cpp: %.h
# 	moc $< -o $@

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
