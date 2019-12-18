##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=udp2mqtt
ConfigurationName      :=Debug
WorkspacePath          :=/Users/lieven/workspace
ProjectPath            :=/Users/lieven/workspace/udp2mqtt
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Lieven Merckx
Date                   :=18/12/2019
CodeLitePath           :="/Users/lieven/Library/Application Support/CodeLite"
LinkerName             :=/usr/bin/llvm-g++
SharedObjectLinkerName :=/usr/bin/llvm-g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="udp2mqtt.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -l:libpaho-mqtt3c.a
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../Common $(IncludeSwitch)../ArduinoJson/src $(IncludeSwitch)../paho.mqtt.c/src 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)Common $(LibrarySwitch)pthread 
ArLibs                 :=  "Common" "pthread" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../Common/Debug $(LibraryPathSwitch)../paho.mqtt.c/build/output 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/llvm-ar rcu
CXX      := /usr/bin/llvm-g++
CC       := /usr/bin/llvm-gcc
CXXFLAGS :=  -g -O0 -std=c++11 $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/llvm-as


##
## User defined environment variables
##
CodeLiteDir:=/Applications/codelite.app/Contents/SharedSupport/
PATH:=/Users/lieven/esp/esp-idf/components/esptool_py/esptool:/Users/lieven/esp/esp-idf/components/espcoredump:/Users/lieven/esp/esp-idf/components/partition_table/:/Users/lieven/.espressif/tools/xtensa-esp32-elf/esp-2019r2-8.2.0/xtensa-esp32-elf/bin:/Users/lieven/.espressif/tools/xtensa-esp32s2-elf/esp-2019r2-8.2.0/xtensa-esp32s2-elf/bin:/Users/lieven/.espressif/tools/esp32ulp-elf/2.28.51.20170517/esp32ulp-elf-binutils/bin:/Users/lieven/.espressif/tools/openocd-esp32/v0.10.0-esp32-20190708/openocd-esp32/bin:/Users/lieven/.espressif/python_env/idf4.1_py2.7_env/bin:/Users/lieven/esp/esp-idf/components/esptool_py/esptool:/Users/lieven/esp/esp-idf/components/espcoredump:/Users/lieven/esp/esp-idf/components/partition_table/:/Users/lieven/esp/esp-idf/tools/:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/opt/X11/bin
Objects0=$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IntermediateDirectory)/Sys.cpp$(ObjectSuffix) $(IntermediateDirectory)/Timer.cpp$(ObjectSuffix) $(IntermediateDirectory)/Udp.cpp$(ObjectSuffix) $(IntermediateDirectory)/Udp2Mqtt.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ./Debug || $(MakeDirCommand) ./Debug


$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main.cpp$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/udp2mqtt/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.cpp$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/main.cpp$(DependSuffix) -MM main.cpp

$(IntermediateDirectory)/main.cpp$(PreprocessSuffix): main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.cpp$(PreprocessSuffix) main.cpp

$(IntermediateDirectory)/Sys.cpp$(ObjectSuffix): Sys.cpp $(IntermediateDirectory)/Sys.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/udp2mqtt/Sys.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Sys.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Sys.cpp$(DependSuffix): Sys.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Sys.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Sys.cpp$(DependSuffix) -MM Sys.cpp

$(IntermediateDirectory)/Sys.cpp$(PreprocessSuffix): Sys.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Sys.cpp$(PreprocessSuffix) Sys.cpp

$(IntermediateDirectory)/Timer.cpp$(ObjectSuffix): Timer.cpp $(IntermediateDirectory)/Timer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/udp2mqtt/Timer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Timer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Timer.cpp$(DependSuffix): Timer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Timer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Timer.cpp$(DependSuffix) -MM Timer.cpp

$(IntermediateDirectory)/Timer.cpp$(PreprocessSuffix): Timer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Timer.cpp$(PreprocessSuffix) Timer.cpp

$(IntermediateDirectory)/Udp.cpp$(ObjectSuffix): Udp.cpp $(IntermediateDirectory)/Udp.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/udp2mqtt/Udp.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Udp.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Udp.cpp$(DependSuffix): Udp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Udp.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Udp.cpp$(DependSuffix) -MM Udp.cpp

$(IntermediateDirectory)/Udp.cpp$(PreprocessSuffix): Udp.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Udp.cpp$(PreprocessSuffix) Udp.cpp

$(IntermediateDirectory)/Udp2Mqtt.cpp$(ObjectSuffix): Udp2Mqtt.cpp $(IntermediateDirectory)/Udp2Mqtt.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/udp2mqtt/Udp2Mqtt.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Udp2Mqtt.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Udp2Mqtt.cpp$(DependSuffix): Udp2Mqtt.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Udp2Mqtt.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Udp2Mqtt.cpp$(DependSuffix) -MM Udp2Mqtt.cpp

$(IntermediateDirectory)/Udp2Mqtt.cpp$(PreprocessSuffix): Udp2Mqtt.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Udp2Mqtt.cpp$(PreprocessSuffix) Udp2Mqtt.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


