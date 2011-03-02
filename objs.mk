
SHARE_OBJS= \
	src/dc_exec.o \
	src/dc_vmu.o \
	src/swapth.o \
	src/elements.o \
	src/profiler.o \
	src/share/vec3.o \
	src/share/image.o \
	src/share/part.o \
	src/share/back.o \
	src/share/geom.o \
	src/share/gui.o \
	src/share/config.o \
	src/share/binary.o \
	src/share/state.o \
	src/share/audio.o

BALL_OBJS= \
	src/share/solid.o \
	src/ball/hud.o \
	src/ball/game.o \
	src/ball/level.o \
	src/ball/set.o \
	src/ball/demo.o \
	src/ball/util.o \
	src/ball/st_conf.o \
	src/ball/st_demo.o \
	src/ball/st_save.o \
	src/ball/st_fail.o \
	src/ball/st_goal.o \
	src/ball/st_done.o \
	src/ball/st_level.o \
	src/ball/st_over.o \
	src/ball/st_play.o \
	src/ball/st_set.o \
	src/ball/st_start.o \
	src/ball/st_title.o \
	src/ball/main.o 

PUTT_OBJS= \
	src/share/solid_putt.o \
	src/putt/hud.o \
	src/putt/game.o \
	src/putt/hole.o \
	src/putt/course.o \
	src/putt/st_all.o \
	src/putt/st_conf.o \
	src/putt/main.o


MAPC_OBJS= \
	src/share/solid_putt.o \
	src/share/mapc.o 


src/share/solid_putt.o: src/share/solid.cpp

