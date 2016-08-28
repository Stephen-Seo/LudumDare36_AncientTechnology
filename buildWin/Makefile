SRC_ROOT = ../src
SFML_ROOT = /home/stephen/Downloads/SFML-2.4.0-source/buildWin64
CXX = x86_64-w64-mingw32-g++
CXXFLAGS = -c -std=c++14 -Wall -Wextra -O3 -D NDEBUG \
           -I${SFML_ROOT}/include \
           -I${SRC_ROOT} \
           -I${SRC_ROOT}/../SFML_GameEngine/src \
           -I${SRC_ROOT}/../EntityComponentMetaSystem/src \
           -I${SRC_ROOT}/../SFML_GameEngine/ResourcePacker/src \
           -mwindows

LINKER_FLAGS = -mwindows

LINKER_LIBS = ${SFML_ROOT}/bin/sfml-audio-2.dll \
              ${SFML_ROOT}/bin/sfml-network-2.dll \
              ${SFML_ROOT}/bin/sfml-graphics-2.dll \
              ${SFML_ROOT}/bin/sfml-window-2.dll \
              ${SFML_ROOT}/bin/sfml-system-2.dll

POST_LINKER_LIBS =

SOURCES = ${SRC_ROOT}/main.o \
          ${SRC_ROOT}/screens/GameScreen.o \
          ${SRC_ROOT}/screens/GameOverScreen.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/animatedSprite.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/connection.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/context.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/game.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/gui.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/musicPlayer.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/networkIdentifiers.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/resourceIdentifiers.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/resourceManager.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/sceneNode.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/soundPlayer.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/state.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/stateStack.o \
          ${SRC_ROOT}/../SFML_GameEngine/src/engine/utility.o \
          ${SRC_ROOT}/../SFML_GameEngine/ResourcePacker/src/ResourcePacker.o

all:    LudumDare36

LudumDare36: ${SOURCES}
		$(CXX) $(LINKER_FLAGS) -o LudumDare36.exe ${LINKER_LIBS} ${SOURCES} ${POST_LINKER_LIBS}
		touch LudumDare36

clean:	../src.clean ../src/screens.clean ../SFML_GameEngine/src/engine.clean ../SFML_GameEngine/ResourcePacker/src.clean
		rm -f LudumDare36.exe
		rm -f LudumDare36

%.clean:
		rm -f $*/*.o

%.o:		%.cpp
		$(CXX) $(CXXFLAGS) -o $*.o $*.cpp
