HEADERS += \
	taskbarprogress/cprogressbartaskbar.h \
	taskbarprogress/taskbarprogress/ctaskbarprogress.h

SOURCES += taskbarprogress/cprogressbartaskbar.cpp

win*{
	SOURCES += taskbarprogress/taskbarprogress/ctaskbarprogress_win.cpp \
}

mac*{
	SOURCES += taskbarprogress/taskbarprogress/ctaskbarprogress_mac.cpp \
}
